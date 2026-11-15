#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file_handler.h"
#include "../include/gui.h"
#include "../include/seat_allocation.h"
#include "../include/driver.h"
#include "../include/student.h"

#define FILE_BUS_STATUS_PATH "data/bus_status.dat"

typedef struct {
    char bus_id[20];
    char status[20];
} BusStatusRecord;

typedef struct {
    GWeakRef window;
} DashboardTimer;

static GtkWidget *get_window_widget(GtkWindow *window, const char *key);

static void show_ui_message(GtkWindow *parent, const char *title,
                            const char *message)
{
    GtkWidget *dialog = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *label = gtk_label_new(message);
    GtkWidget *button = gtk_button_new_with_label("OK");

    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 380, 150);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_set_margin_top(box, 18);
    gtk_widget_set_margin_bottom(box, 18);
    gtk_widget_set_margin_start(box, 18);
    gtk_widget_set_margin_end(box, 18);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), button);
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    g_signal_connect_swapped(button, "clicked",
                             G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

static int get_bus_status(const char *bus_id, char status[20])
{
    BusStatusRecord record = {{0}, {0}};
    FileStatus result = file_find_record(
        FILE_BUS_STATUS_PATH, bus_id, &record, sizeof(record),
        offsetof(BusStatusRecord, bus_id), sizeof(record.bus_id));

    if (result == FILE_STATUS_NOT_FOUND)
    {
        snprintf(status, 20, "Active");
        return 1;
    }
    if (result != FILE_STATUS_OK ||
        memchr(record.status, '\0', sizeof(record.status)) == NULL)
        return 0;
    snprintf(status, 20, "%s", record.status);
    return 1;
}

static FileStatus save_bus_status(const char *bus_id, const char *status)
{
    BusStatusRecord record = {{0}, {0}};
    FileStatus result;

    snprintf(record.bus_id, sizeof(record.bus_id), "%s", bus_id);
    snprintf(record.status, sizeof(record.status), "%s", status);
    result = file_find_record(FILE_BUS_STATUS_PATH, bus_id, &record,
                              sizeof(record),
                              offsetof(BusStatusRecord, bus_id),
                              sizeof(record.bus_id));
    if (result == FILE_STATUS_NOT_FOUND)
        return file_save_record(FILE_BUS_STATUS_PATH, &record, sizeof(record),
                                offsetof(BusStatusRecord, bus_id),
                                sizeof(record.bus_id));
    if (result != FILE_STATUS_OK)
        return result;

    snprintf(record.status, sizeof(record.status), "%s", status);
    return file_update_record(FILE_BUS_STATUS_PATH, &record, sizeof(record),
                              offsetof(BusStatusRecord, bus_id),
                              sizeof(record.bus_id));
}

static guint selected_index(GtkDropDown *dropdown)
{
    guint selected = gtk_drop_down_get_selected(dropdown);
    return selected == GTK_INVALID_LIST_POSITION ? G_MAXUINT : selected;
}

static Bus *selected_bus(GtkWindow *window)
{
    GtkWidget *dropdown = get_window_widget(window, "bus-dropdown");
    GPtrArray *buses;
    guint index;

    if (dropdown == NULL)
        return NULL;
    index = selected_index(GTK_DROP_DOWN(dropdown));
    buses = (GPtrArray *)g_object_get_data(G_OBJECT(dropdown), "bus-records");
    if (buses == NULL || index >= buses->len)
        return NULL;
    return (Bus *)g_ptr_array_index(buses, index);
}

static const char *selected_schedule(GtkWindow *window)
{
    GtkWidget *dropdown = get_window_widget(window, "schedule-dropdown");
    GPtrArray *schedules;
    guint index;

    if (dropdown == NULL)
        return NULL;
    index = selected_index(GTK_DROP_DOWN(dropdown));
    schedules = (GPtrArray *)g_object_get_data(G_OBJECT(dropdown),
                                               "schedule-ids");
    if (schedules == NULL || index >= schedules->len)
        return NULL;
    return (const char *)g_ptr_array_index(schedules, index);
}

static void append_table_row(GtkListBox *list, const char *bus,
                             const char *seat, const char *student,
                             const char *status, const char *preference)
{
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *grid = gtk_grid_new();
    const char *values[] = {bus, seat, student, status, preference};
    int column;

    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_widget_set_margin_top(grid, 7);
    gtk_widget_set_margin_bottom(grid, 7);
    gtk_widget_set_margin_start(grid, 8);
    gtk_widget_set_margin_end(grid, 8);
    for (column = 0; column < 5; ++column)
    {
        GtkWidget *label = gtk_label_new(values[column]);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_grid_attach(GTK_GRID(grid), label, column, 0, 1, 1);
    }
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), grid);
    gtk_list_box_append(list, row);
}

static void refresh_seat_table(GtkWindow *window);
static void populate_schedules(GtkWindow *window);

static void populate_buses(GtkWindow *window, const char *select_bus_id)
{
    GtkWidget *dropdown = get_window_widget(window, "bus-dropdown");
    GtkStringList *strings = gtk_string_list_new(NULL);
    GPtrArray *buses = g_ptr_array_new_with_free_func(g_free);
    void *records = NULL;
    size_t count = 0;
    size_t index;
    guint selected = GTK_INVALID_LIST_POSITION;
    FileStatus result = file_read_records(FILE_BUSES_PATH, &records,
                                          sizeof(Bus), &count);

    if (result != FILE_STATUS_OK && result != FILE_STATUS_NOT_FOUND)
    {
        g_ptr_array_unref(buses);
        g_object_unref(strings);
        show_ui_message(window, "Bus data error",
                        file_status_message(result));
        return;
    }

    for (index = 0; index < count; ++index)
    {
        Bus *records_as_buses = (Bus *)records;
        Bus *bus = &records_as_buses[index];
        char status[20];
        char *label;
        Bus *copy;

        if (memchr(bus->bus_id, '\0', sizeof(bus->bus_id)) == NULL ||
            memchr(bus->bus_number, '\0', sizeof(bus->bus_number)) == NULL ||
            bus->capacity <= 0)
        {
            free(records);
            g_ptr_array_unref(buses);
            g_object_unref(strings);
            show_ui_message(window, "Bus data error",
                            "A bus record has an invalid ID, number, or "
                            "seat capacity.");
            return;
        }
        if (!get_bus_status(bus->bus_id, status))
        {
            free(records);
            g_ptr_array_unref(buses);
            g_object_unref(strings);
            show_ui_message(window, "Bus data error",
                            "Could not read the bus status records.");
            return;
        }
        if (strcmp(status, "Active") != 0)
            continue;

        label = g_strdup_printf("Bus %s | %s | %d seats", bus->bus_id,
                                bus->bus_number, bus->capacity);
        gtk_string_list_append(strings, label);
        g_free(label);
        copy = g_new(Bus, 1);
        *copy = *bus;
        g_ptr_array_add(buses, copy);
        if (select_bus_id != NULL &&
            strcmp(select_bus_id, bus->bus_id) == 0)
            selected = buses->len - 1;
    }
    free(records);

    g_object_set_data_full(G_OBJECT(dropdown), "bus-records", buses,
                           (GDestroyNotify)g_ptr_array_unref);
    gtk_drop_down_set_model(GTK_DROP_DOWN(dropdown), G_LIST_MODEL(strings));
    g_object_unref(strings);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown),
                               selected != GTK_INVALID_LIST_POSITION
                                   ? selected
                                   : (buses->len > 0 ? 0
                                                     : GTK_INVALID_LIST_POSITION));
    populate_schedules(window);
}

static void populate_schedules(GtkWindow *window)
{
    GtkWidget *dropdown = get_window_widget(window, "schedule-dropdown");
    Bus *bus = selected_bus(window);
    GtkStringList *strings = gtk_string_list_new(NULL);
    GPtrArray *ids = g_ptr_array_new_with_free_func(g_free);
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus result;

    if (bus != NULL)
    {
        result = file_read_records(FILE_SCHEDULES_PATH, &records,
                                   sizeof(Schedule), &count);
        if (result != FILE_STATUS_OK && result != FILE_STATUS_NOT_FOUND)
        {
            g_ptr_array_unref(ids);
            g_object_unref(strings);
            show_ui_message(window, "Schedule data error",
                            file_status_message(result));
            return;
        }
        for (index = 0; index < count; ++index)
        {
            Schedule *schedules = (Schedule *)records;
            Schedule *schedule = &schedules[index];
            char *label;

            if (memchr(schedule->schedule_id, '\0',
                       sizeof(schedule->schedule_id)) == NULL ||
                memchr(schedule->bus_id, '\0',
                       sizeof(schedule->bus_id)) == NULL)
            {
                free(records);
                g_ptr_array_unref(ids);
                g_object_unref(strings);
                show_ui_message(window, "Schedule data error",
                                "A schedule record contains invalid data.");
                return;
            }
            if (strcmp(schedule->bus_id, bus->bus_id) != 0)
                continue;
            label = g_strdup_printf("%s | %s", schedule->schedule_id,
                                    schedule->departure_time);
            gtk_string_list_append(strings, label);
            g_free(label);
            g_ptr_array_add(ids, g_strdup(schedule->schedule_id));
        }
        free(records);
    }

    g_object_set_data_full(G_OBJECT(dropdown), "schedule-ids", ids,
                           (GDestroyNotify)g_ptr_array_unref);
    gtk_drop_down_set_model(GTK_DROP_DOWN(dropdown), G_LIST_MODEL(strings));
    g_object_unref(strings);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown),
                               ids->len > 0 ? 0 : GTK_INVALID_LIST_POSITION);
    refresh_seat_table(window);
}

static void refresh_seat_table(GtkWindow *window)
{
    GtkWidget *list_widget = get_window_widget(window, "seat-list");
    GtkListBox *list = GTK_LIST_BOX(list_widget);
    GtkWidget *child = gtk_widget_get_first_child(list_widget);
    Bus *bus = selected_bus(window);
    const char *schedule_id = selected_schedule(window);
    GtkWidget *occupancy = get_window_widget(window, "occupancy-label");
    void *booking_records = NULL;
    void *wait_records = NULL;
    void *seat_records = NULL;
    size_t booking_count = 0;
    size_t waiting_count = 0;
    size_t seat_record_count = 0;
    Booking *bookings;
    SeatWaitingListEntry *waiting;
    SeatWaitingListEntry *seats;
    SeatAvailability availability;
    FileStatus result;
    int seat;
    size_t index;
    size_t position = 0;

    while (child != NULL)
    {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(list, child);
        child = next;
    }
    if (bus == NULL || schedule_id == NULL)
    {
        gtk_label_set_text(GTK_LABEL(occupancy), "Occupancy unavailable");
        return;
    }
    if (!get_seat_availability(schedule_id, &availability))
    {
        gtk_label_set_text(GTK_LABEL(occupancy), "Occupancy unavailable");
        return;
    }
    {
        char *text = g_strdup_printf(
            "Occupancy: %d / %d seats | %d available",
            availability.occupied_seats, availability.total_seats,
            availability.available_seats);
        gtk_label_set_text(GTK_LABEL(occupancy), text);
        g_free(text);
    }

    result = file_read_records(FILE_BOOKINGS_PATH, &booking_records,
                               sizeof(Booking), &booking_count);
    if (result == FILE_STATUS_NOT_FOUND)
    {
        booking_records = NULL;
        booking_count = 0;
        result = FILE_STATUS_OK;
    }
    if (result != FILE_STATUS_OK)
    {
        show_ui_message(window, "Booking data error",
                        file_status_message(result));
        return;
    }
    result = file_read_records(FILE_WAITLIST_PATH, &wait_records,
                               sizeof(SeatWaitingListEntry), &waiting_count);
    if (result == FILE_STATUS_NOT_FOUND)
    {
        wait_records = NULL;
        waiting_count = 0;
        result = FILE_STATUS_OK;
    }
    if (result != FILE_STATUS_OK)
    {
        free(booking_records);
        show_ui_message(window, "Waiting-list data error",
                        file_status_message(result));
        return;
    }
    result = file_read_records(FILE_SEATS_PATH, &seat_records,
                               sizeof(SeatWaitingListEntry),
                               &seat_record_count);
    if (result == FILE_STATUS_NOT_FOUND)
    {
        seat_records = NULL;
        seat_record_count = 0;
        result = FILE_STATUS_OK;
    }
    if (result != FILE_STATUS_OK)
    {
        free(booking_records);
        free(wait_records);
        show_ui_message(window, "Seat data error",
                        file_status_message(result));
        return;
    }

    bookings = (Booking *)booking_records;
    waiting = (SeatWaitingListEntry *)wait_records;
    seats = (SeatWaitingListEntry *)seat_records;
    for (seat = 1; seat <= bus->capacity; ++seat)
    {
        const char *student_id = "";
        const char *status = "Available";
        const char *seat_preference = "--";
        for (index = 0; index < booking_count; ++index)
        {
            if (strcmp(bookings[index].schedule_id, schedule_id) == 0 &&
                bookings[index].seat_number == seat)
            {
                student_id = bookings[index].student_id;
                status = "Booked";
                seat_preference = "Any";
                {
                    size_t seat_index;
                    for (seat_index = 0; seat_index < seat_record_count;
                         ++seat_index)
                    {
                        if (strcmp(seats[seat_index].booking.student_id,
                                   bookings[index].student_id) == 0 &&
                            strcmp(seats[seat_index].booking.schedule_id,
                                   schedule_id) == 0 &&
                            seats[seat_index].booking.seat_number == seat)
                        {
                            seat_preference =
                                seats[seat_index].preference ==
                                        SEAT_PREFERENCE_WINDOW
                                    ? "Window"
                                    : (seats[seat_index].preference ==
                                               SEAT_PREFERENCE_FRONT
                                           ? "Front"
                                           : "Any");
                            break;
                        }
                    }
                }
                break;
            }
        }
        char *seat_text = g_strdup_printf("%d", seat);
        append_table_row(list, bus->bus_number, seat_text, student_id, status,
                         seat_preference);
        g_free(seat_text);
    }
    for (index = 0; index < waiting_count; ++index)
    {
        const char *preference;
        char *queue_position;
        if (strcmp(waiting[index].booking.schedule_id, schedule_id) != 0)
            continue;
        ++position;
        preference = waiting[index].preference == SEAT_PREFERENCE_WINDOW
                        ? "Window"
                        : (waiting[index].preference == SEAT_PREFERENCE_FRONT
                               ? "Front"
                               : "Any");
        queue_position = g_strdup_printf("Waiting #%lu",
                                         (unsigned long)position);
        append_table_row(list, bus->bus_number, queue_position,
                         waiting[index].booking.student_id, queue_position,
                         preference);
        g_free(queue_position);
    }
    free(booking_records);
    free(wait_records);
    free(seat_records);
}

static void bus_selection_changed(GObject *object, GParamSpec *spec,
                                  gpointer data)
{
    (void)object;
    (void)spec;
    populate_schedules(GTK_WINDOW(data));
}

static void schedule_selection_changed(GObject *object, GParamSpec *spec,
                                       gpointer data)
{
    (void)object;
    (void)spec;
    refresh_seat_table(GTK_WINDOW(data));
}

static void refresh_seats_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    populate_buses(GTK_WINDOW(data), NULL);
}

static void book_seat_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *student_entry = get_window_widget(window, "student-id-entry");
    GtkWidget *preference = get_window_widget(window, "preference-dropdown");
    const char *student_id =
        gtk_editable_get_text(GTK_EDITABLE(student_entry));
    const char *schedule_id = selected_schedule(window);
    Student student;
    Booking booking = {0};
    guint pref = selected_index(GTK_DROP_DOWN(preference));
    SeatAllocationResult result;

    (void)button;
    if (student_id[0] == '\0' || strlen(student_id) >= sizeof(student.student_id))
    {
        show_ui_message(window, "Invalid Student ID",
                        "Enter a valid student ID before booking.");
        return;
    }
    if (schedule_id == NULL)
    {
        show_ui_message(window, "No schedule selected",
                        "The selected bus has no available schedule.");
        return;
    }
    if (file_find_record(FILE_STUDENTS_PATH, student_id, &student,
                         sizeof(student), offsetof(Student, student_id),
                         sizeof(student.student_id)) != FILE_STATUS_OK)
    {
        show_ui_message(window, "Invalid Student ID",
                        "That student ID is not in the BusMate student data.");
        return;
    }
    snprintf(booking.student_id, sizeof(booking.student_id), "%s", student_id);
    snprintf(booking.schedule_id, sizeof(booking.schedule_id), "%s",
             schedule_id);
    result = allocate_seat_with_preference(&booking, (SeatPreference)pref);
    if (result == SEAT_ALLOCATION_BOOKED)
    {
        char *message = g_strdup_printf(
            "Seat %d was allocated to student %s.", booking.seat_number,
            student_id);
        show_ui_message(window, "Seat booked", message);
        g_free(message);
    }
    else if (result == SEAT_ALLOCATION_WAITLISTED)
        show_ui_message(window, "Added to waiting list",
                        "The bus is full. The student was added to the FIFO "
                        "waiting list.");
    else if (result == SEAT_ALLOCATION_DUPLICATE)
        show_ui_message(window, "Duplicate booking",
                        "This student already has an active booking or a "
                        "waiting-list entry for this schedule.");
    else
        show_ui_message(window, "Booking failed",
                        "The seat could not be allocated. Check the schedule "
                        "and data files.");
    refresh_seat_table(window);
}

static void cancel_seat_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *student_entry = get_window_widget(window, "student-id-entry");
    const char *student_id =
        gtk_editable_get_text(GTK_EDITABLE(student_entry));
    const char *schedule_id = selected_schedule(window);

    (void)button;
    if (student_id[0] == '\0' || strlen(student_id) >= 20)
    {
        show_ui_message(window, "Invalid Student ID",
                        "Enter a valid student ID before cancelling.");
        return;
    }
    if (schedule_id == NULL)
    {
        show_ui_message(window, "No schedule selected",
                        "Select a bus schedule before cancelling.");
        return;
    }
    if (!cancel_seat(student_id, schedule_id))
        show_ui_message(window, "Cancellation failed",
                        "No matching booking or waitlist entry was found, or "
                        "the shared data could not be updated.");
    else
        show_ui_message(window, "Cancellation complete",
                        "The request was cancelled. If a booked seat was "
                        "freed, the first waiting student was promoted.");
    refresh_seat_table(window);
}
static GtkWidget *get_window_widget(GtkWindow *window, const char *key)
{
    return GTK_WIDGET(g_object_get_data(G_OBJECT(window), key));
}

static const char *get_entry_text(GtkWindow *window, const char *key)
{
    GtkWidget *entry = get_window_widget(window, key);
    return gtk_editable_get_text(GTK_EDITABLE(entry));
}

static void show_report(GtkWindow *window, const char *message)
{
    GtkTextBuffer *buffer =
        GTK_TEXT_BUFFER(g_object_get_data(G_OBJECT(window), "report-buffer"));
    gtk_text_buffer_set_text(buffer, message, -1);
}

static int valid_record_id(const char *id, size_t capacity)
{
    return id[0] != '\0' && memchr(id, '\0', capacity) != NULL;
}

static int valid_input_id(const char *id)
{
    return id != NULL && id[0] != '\0' && strlen(id) < 20;
}

static void refresh_dashboard(GtkWindow *window, const char *message);

static int read_student_details(GtkWindow *window, Student *student)
{
    const char *student_id = get_entry_text(window, "student-entry");
    const char *name = get_entry_text(window, "student-name-entry");
    const char *password = get_entry_text(window, "student-password-entry");

    if (!valid_input_id(student_id) || name[0] == '\0' ||
        strlen(name) >= sizeof(student->name) || password[0] == '\0' ||
        strlen(password) >= sizeof(student->password))
        return 0;

    memset(student, 0, sizeof(*student));
    snprintf(student->student_id, sizeof(student->student_id), "%s",
             student_id);
    snprintf(student->name, sizeof(student->name), "%s", name);
    snprintf(student->password, sizeof(student->password), "%s", password);
    return 1;
}

static int save_student(GtkWindow *window, const Student *student)
{
    Student existing;
    FileStatus status;

    status = file_save_record(FILE_STUDENTS_PATH, student, sizeof(*student),
                              offsetof(Student, student_id),
                              sizeof(student->student_id));
    if (status == FILE_STATUS_ALREADY_EXISTS)
    {
        status = file_find_record(FILE_STUDENTS_PATH, student->student_id,
                                  &existing, sizeof(existing),
                                  offsetof(Student, student_id),
                                  sizeof(existing.student_id));
        if (status == FILE_STATUS_OK &&
            memchr(existing.name, '\0', sizeof(existing.name)) != NULL &&
            memchr(existing.password, '\0', sizeof(existing.password)) != NULL &&
            strcmp(existing.name, student->name) == 0 &&
            strcmp(existing.password, student->password) == 0)
            return 1;
        refresh_dashboard(
            window,
            "That student ID is already registered with different details; "
            "existing data was not changed.");
        return 0;
    }
    if (status != FILE_STATUS_OK)
    {
        char message[160];
        snprintf(message, sizeof(message), "Could not save student details: %s.",
                 file_status_message(status));
        refresh_dashboard(window, message);
        return 0;
    }

    return 1;
}

static int verify_student_details(GtkWindow *window, Student *student)
{
    Student saved;
    FileStatus status;

    status = file_find_record(FILE_STUDENTS_PATH, student->student_id, &saved,
                              sizeof(saved), offsetof(Student, student_id),
                              sizeof(saved.student_id));
    if (status != FILE_STATUS_OK)
    {
        char message[160];
        snprintf(message, sizeof(message),
                 "Student account was not found; add the student before "
                 "cancelling: %s.",
                 file_status_message(status));
        refresh_dashboard(window, message);
        return 0;
    }

    if (memchr(saved.name, '\0', sizeof(saved.name)) == NULL ||
        memchr(saved.password, '\0', sizeof(saved.password)) == NULL ||
        strcmp(saved.name, student->name) != 0 ||
        strcmp(saved.password, student->password) != 0)
    {
        refresh_dashboard(window,
                          "Student ID, name, or password does not match the "
                          "saved student record.");
        return 0;
    }
    return 1;
}

static void refresh_dashboard(GtkWindow *window, const char *message)
{
    const char *schedule_id = get_entry_text(window, "schedule-entry");
    const char *student_id = get_entry_text(window, "student-entry");
    const char *student_name = get_entry_text(window, "student-name-entry");
    GString *report = g_string_new(NULL);
    SeatAvailability availability;
    void *booking_records = NULL;
    void *waiting_records = NULL;
    size_t booking_count = 0;
    size_t waiting_count = 0;
    Booking *bookings;
    SeatWaitingListEntry *waiting;
    FileStatus status;
    size_t index;
    size_t wait_position = 0;
    int user_seat = 0;
    int is_waiting = 0;

    if (message != NULL && message[0] != '\0')
        g_string_append_printf(report, "%s\n\n", message);

    if (schedule_id[0] == '\0' || strlen(schedule_id) >= 20)
    {
        g_string_append(report, "Enter a valid schedule ID to view its seats.");
        show_report(window, report->str);
        g_string_free(report, TRUE);
        return;
    }

    if (!get_seat_availability(schedule_id, &availability))
    {
        g_string_append(
            report,
            "Schedule or bus record not found, or seat data could not be read.\n"
            "The schedule and its bus must already be stored in the shared "
            "data files.");
        show_report(window, report->str);
        g_string_free(report, TRUE);
        return;
    }

    status = file_read_records(FILE_BOOKINGS_PATH, &booking_records,
                               sizeof(Booking), &booking_count);
    if (status == FILE_STATUS_NOT_FOUND)
    {
        booking_count = 0;
        status = FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
    {
        g_string_append_printf(report, "Could not read bookings: %s",
                               file_status_message(status));
        show_report(window, report->str);
        g_string_free(report, TRUE);
        return;
    }

    status = file_read_records(FILE_WAITLIST_PATH, &waiting_records,
                               sizeof(SeatWaitingListEntry), &waiting_count);
    if (status == FILE_STATUS_NOT_FOUND)
    {
        waiting_count = 0;
        status = FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
    {
        free(booking_records);
        g_string_append_printf(report, "Could not read waiting list: %s",
                               file_status_message(status));
        show_report(window, report->str);
        g_string_free(report, TRUE);
        return;
    }

    bookings = (Booking *)booking_records;
    waiting = (SeatWaitingListEntry *)waiting_records;
    for (index = 0; index < booking_count; ++index)
    {
        if (!valid_record_id(bookings[index].student_id,
                             sizeof(bookings[index].student_id)) ||
            !valid_record_id(bookings[index].schedule_id,
                             sizeof(bookings[index].schedule_id)))
        {
            free(booking_records);
            free(waiting_records);
            g_string_append(report, "Booking data contains an invalid record.");
            show_report(window, report->str);
            g_string_free(report, TRUE);
            return;
        }

        if (strcmp(bookings[index].schedule_id, schedule_id) == 0 &&
            strcmp(bookings[index].student_id, student_id) == 0)
            user_seat = bookings[index].seat_number;
    }

    for (index = 0; index < waiting_count; ++index)
    {
        if (!valid_record_id(waiting[index].booking.student_id,
                             sizeof(waiting[index].booking.student_id)) ||
            !valid_record_id(waiting[index].booking.schedule_id,
                             sizeof(waiting[index].booking.schedule_id)))
        {
            free(booking_records);
            free(waiting_records);
            g_string_append(report,
                            "Waiting-list data contains an invalid record.");
            show_report(window, report->str);
            g_string_free(report, TRUE);
            return;
        }

        if (strcmp(waiting[index].booking.schedule_id, schedule_id) == 0)
        {
            ++wait_position;
            if (student_id[0] != '\0' &&
                strcmp(waiting[index].booking.student_id, student_id) == 0)
                is_waiting = 1;
        }
    }

    g_string_append_printf(report, "Schedule: %s\n", schedule_id);
    g_string_append_printf(report,
                           "Occupancy: %d / %d | Available: %d\n",
                           availability.occupied_seats,
                           availability.total_seats,
                           availability.available_seats);
    if (student_id[0] != '\0')
    {
        if (student_name[0] != '\0')
            g_string_append_printf(report, "Student: %s (%s)\n",
                                   student_name, student_id);
        if (user_seat > 0)
            g_string_append_printf(report, "Allocated seat: %d\n", user_seat);
        else if (is_waiting)
        {
            size_t position = 0;
            for (index = 0; index < waiting_count; ++index)
            {
                if (strcmp(waiting[index].booking.schedule_id, schedule_id) != 0)
                    continue;
                ++position;
                if (strcmp(waiting[index].booking.student_id, student_id) == 0)
                {
                    g_string_append_printf(report,
                                           "Student %s is waiting at position "
                                           "%lu.\n",
                                           student_id,
                                           (unsigned long)position);
                    break;
                }
            }
        }
        else
            g_string_append_printf(report,
                                   "Student %s has no booking on this "
                                   "schedule.\n",
                                   student_id);
    }

    g_string_append(report, "\nSeat map\n");
    for (index = 1; index <= (size_t)availability.total_seats; ++index)
    {
        size_t booking_index;
        const char *occupant = NULL;

        for (booking_index = 0; booking_index < booking_count; ++booking_index)
        {
            if (strcmp(bookings[booking_index].schedule_id, schedule_id) == 0 &&
                bookings[booking_index].seat_number == (int)index)
            {
                occupant = bookings[booking_index].student_id;
                break;
            }
        }
        if (occupant == NULL)
            g_string_append_printf(report, "Seat %lu: Available\n",
                                   (unsigned long)index);
        else
            g_string_append_printf(report, "Seat %lu: Booked by %s%s\n",
                                   (unsigned long)index, occupant,
                                   student_id[0] != '\0' &&
                                           strcmp(occupant, student_id) == 0
                                       ? " (you)"
                                       : "");
    }

    g_string_append_printf(report, "\nWaiting list (%lu)\n",
                           (unsigned long)wait_position);
    wait_position = 0;
    for (index = 0; index < waiting_count; ++index)
    {
        const char *preference;

        if (strcmp(waiting[index].booking.schedule_id, schedule_id) != 0)
            continue;
        if (waiting[index].preference == SEAT_PREFERENCE_WINDOW)
            preference = "Window";
        else if (waiting[index].preference == SEAT_PREFERENCE_FRONT)
            preference = "Front";
        else
            preference = "Any";
        g_string_append_printf(report, "%lu. %s — %s\n",
                               (unsigned long)++wait_position,
                               waiting[index].booking.student_id, preference);
    }
    if (wait_position == 0)
        g_string_append(report, "No waiting students.\n");

    free(booking_records);
    free(waiting_records);
    show_report(window, report->str);
    g_string_free(report, TRUE);
}

static void refresh_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    refresh_dashboard(GTK_WINDOW(data), NULL);
}

static void add_student_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    const char *schedule_id = get_entry_text(window, "schedule-entry");
    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(
        get_window_widget(window, "preference-dropdown")));
    Student student;
    Booking booking = {0};
    SeatAllocationResult result;
    SeatAvailability availability;
    char message[160];

    (void)button;
    if (!read_student_details(window, &student) ||
        !valid_input_id(schedule_id) ||
        selected > SEAT_PREFERENCE_FRONT)
    {
        refresh_dashboard(window,
                          "Enter student ID, name, password, and schedule ID; "
                          "all text fields must fit their limits.");
        return;
    }
    if (!get_seat_availability(schedule_id, &availability))
    {
        refresh_dashboard(
            window,
            "Cannot add student: check that this schedule and its bus exist "
            "and that the bus has a valid seat capacity.");
        return;
    }

    if (!save_student(window, &student))
        return;

    snprintf(booking.student_id, sizeof(booking.student_id), "%s",
             student.student_id);
    snprintf(booking.schedule_id, sizeof(booking.schedule_id), "%s",
             schedule_id);
    result = allocate_seat_with_preference(&booking, (SeatPreference)selected);
    if (result == SEAT_ALLOCATION_BOOKED)
        snprintf(message, sizeof(message),
                 "%s added to %s and assigned seat %d.",
                 student.name, schedule_id, booking.seat_number);
    else if (result == SEAT_ALLOCATION_WAITLISTED)
        snprintf(message, sizeof(message),
                 "All %d seats on %s are occupied. %s was added to the "
                 "waiting list.",
                 availability.total_seats, schedule_id, student.name);
    else if (result == SEAT_ALLOCATION_DUPLICATE)
        snprintf(message, sizeof(message),
                 "%s already has a booking or waiting-list entry for %s.",
                 student.name, schedule_id);
    else
        snprintf(message, sizeof(message),
                 "%s was saved, but seat assignment failed. Check the "
                 "schedule and data files.",
                 student.name);

    refresh_dashboard(window, message);
}

static void cancel_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    const char *schedule_id = get_entry_text(window, "schedule-entry");
    Student student;

    (void)button;
    if (!read_student_details(window, &student) ||
        !valid_input_id(schedule_id))
    {
        refresh_dashboard(window,
                          "Enter valid student ID, name, password, and schedule "
                          "ID before cancelling.");
        return;
    }
    if (!verify_student_details(window, &student))
        return;

    if (cancel_seat(student.student_id, schedule_id))
        refresh_dashboard(window,
                          "Booking or waiting-list request cancelled. Any "
                          "waiting student was promoted automatically.");
    else
        refresh_dashboard(window,
                          "Cancellation failed: no matching request exists or "
                          "the records could not be updated.");
}

void show_seat_allocation_dashboard(GtkWindow *parent)
{
    GtkApplication *app = gtk_window_get_application(parent);
    static const char *preferences[] = {
        "Any available seat", "Window seat", "Front seat", NULL
    };
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *note;
    GtkWidget *schedule_label;
    GtkWidget *schedule_entry;
    GtkWidget *student_label;
    GtkWidget *student_entry;
    GtkWidget *name_label;
    GtkWidget *name_entry;
    GtkWidget *password_label;
    GtkWidget *password_entry;
    GtkWidget *preference_label;
    GtkWidget *preference_dropdown;
    GtkWidget *button_row;
    GtkWidget *add_student_button;
    GtkWidget *cancel_button;
    GtkWidget *refresh_button;
    GtkWidget *scroller;
    GtkWidget *report_view;
    GtkTextBuffer *report_buffer;

    window = gtk_application_window_new(app);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_title(GTK_WINDOW(window), "BusMate Seat Allocation");
    gtk_window_set_default_size(GTK_WINDOW(window), 760, 780);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 18);
    gtk_widget_set_margin_bottom(box, 18);
    gtk_widget_set_margin_start(box, 22);
    gtk_widget_set_margin_end(box, 22);

    title = gtk_label_new("BusMate — Seat Allocation");
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), title);

    note = gtk_label_new(
        "Seats use existing schedule and bus records. When full, booking joins "
        "the FIFO waiting list; cancellation removes your request and promotes "
        "the next student. Demo passwords are stored as entered in students.dat.");
    gtk_label_set_wrap(GTK_LABEL(note), TRUE);
    gtk_widget_set_halign(note, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), note);

    schedule_label = gtk_label_new("Schedule ID");
    gtk_widget_set_halign(schedule_label, GTK_ALIGN_START);
    schedule_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(schedule_entry),
                                   "Example: SCHED-1");
    gtk_box_append(GTK_BOX(box), schedule_label);
    gtk_box_append(GTK_BOX(box), schedule_entry);

    student_label = gtk_label_new("Student ID");
    gtk_widget_set_halign(student_label, GTK_ALIGN_START);
    student_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(student_entry), "Student ID");
    gtk_box_append(GTK_BOX(box), student_label);
    gtk_box_append(GTK_BOX(box), student_entry);

    name_label = gtk_label_new("Student name");
    gtk_widget_set_halign(name_label, GTK_ALIGN_START);
    name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Full name");
    gtk_box_append(GTK_BOX(box), name_label);
    gtk_box_append(GTK_BOX(box), name_entry);

    password_label = gtk_label_new("Password");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);
    password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_append(GTK_BOX(box), password_label);
    gtk_box_append(GTK_BOX(box), password_entry);

    preference_label = gtk_label_new("Seat preference");
    gtk_widget_set_halign(preference_label, GTK_ALIGN_START);
    preference_dropdown = gtk_drop_down_new_from_strings(preferences);
    gtk_box_append(GTK_BOX(box), preference_label);
    gtk_box_append(GTK_BOX(box), preference_dropdown);

    button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    add_student_button = gtk_button_new_with_label("Add student / assign seat");
    cancel_button = gtk_button_new_with_label("Cancel / leave waitlist");
    refresh_button = gtk_button_new_with_label("Refresh status");
    gtk_box_append(GTK_BOX(button_row), add_student_button);
    gtk_box_append(GTK_BOX(button_row), cancel_button);
    gtk_box_append(GTK_BOX(button_row), refresh_button);
    gtk_box_append(GTK_BOX(box), button_row);

    scroller = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroller, TRUE);
    report_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(report_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(report_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(report_view), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), report_view);
    gtk_box_append(GTK_BOX(box), scroller);
    gtk_window_set_child(GTK_WINDOW(window), box);

    report_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(report_view));
    g_object_set_data(G_OBJECT(window), "schedule-entry", schedule_entry);
    g_object_set_data(G_OBJECT(window), "student-entry", student_entry);
    g_object_set_data(G_OBJECT(window), "student-name-entry", name_entry);
    g_object_set_data(G_OBJECT(window), "student-password-entry",
                      password_entry);
    g_object_set_data(G_OBJECT(window), "preference-dropdown",
                      preference_dropdown);
    g_object_set_data(G_OBJECT(window), "report-buffer", report_buffer);

    g_signal_connect(add_student_button, "clicked",
                     G_CALLBACK(add_student_clicked), window);
    g_signal_connect(cancel_button, "clicked",
                     G_CALLBACK(cancel_clicked), window);
    g_signal_connect(refresh_button, "clicked",
                     G_CALLBACK(refresh_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
    refresh_dashboard(GTK_WINDOW(window), NULL);
}

static GtkWidget *add_bus_form_entry(GtkGrid *grid, const char *label,
                                     int row)
{
    GtkWidget *caption = gtk_label_new(label);
    GtkWidget *entry = gtk_entry_new();

    gtk_widget_set_halign(caption, GTK_ALIGN_START);
    gtk_grid_attach(grid, caption, 0, row, 1, 1);
    gtk_grid_attach(grid, entry, 1, row, 1, 1);
    return entry;
}

static void save_bus_dialog_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *dialog = GTK_WINDOW(data);
    GtkWindow *window = GTK_WINDOW(g_object_get_data(
        G_OBJECT(dialog), "allocation-window"));
    GtkWidget *id_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "bus-id-entry"));
    GtkWidget *number_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "bus-number-entry"));
    GtkWidget *capacity_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "bus-capacity-entry"));
    GtkWidget *status_dropdown =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "bus-status-dropdown"));
    const char *id = gtk_editable_get_text(GTK_EDITABLE(id_entry));
    const char *number = gtk_editable_get_text(GTK_EDITABLE(number_entry));
    const char *capacity_text =
        gtk_editable_get_text(GTK_EDITABLE(capacity_entry));
    char *end = NULL;
    long capacity;
    guint status_index = selected_index(GTK_DROP_DOWN(status_dropdown));
    const char *status = status_index == 0 ? "Active" : "Inactive";
    Bus bus = {0};
    FileStatus result;

    (void)button;
    capacity = strtol(capacity_text, &end, 10);
    if (id[0] == '\0' || strlen(id) >= sizeof(bus.bus_id) ||
        number[0] == '\0' || strlen(number) >= sizeof(bus.bus_number) ||
        capacity_text[0] == '\0' || end == capacity_text || *end != '\0' ||
        capacity <= 0 || capacity > INT_MAX || status_index > 1)
    {
        show_ui_message(dialog, "Invalid bus details",
                        "Enter a unique bus ID, a bus number, a positive "
                        "capacity, and a valid status.");
        return;
    }

    snprintf(bus.bus_id, sizeof(bus.bus_id), "%s", id);
    snprintf(bus.bus_number, sizeof(bus.bus_number), "%s", number);
    bus.capacity = (int)capacity;
    result = file_save_record(FILE_BUSES_PATH, &bus, sizeof(bus),
                              offsetof(Bus, bus_id), sizeof(bus.bus_id));
    if (result != FILE_STATUS_OK)
    {
        show_ui_message(dialog, "Could not save bus",
                        file_status_message(result));
        return;
    }
    result = save_bus_status(id, status);
    if (result != FILE_STATUS_OK)
    {
        FileStatus rollback = file_delete_record(
            FILE_BUSES_PATH, id, sizeof(Bus), offsetof(Bus, bus_id),
            sizeof(bus.bus_id));
        if (rollback != FILE_STATUS_OK)
            fprintf(stderr, "Could not roll back bus after status error.\n");
        show_ui_message(dialog, "Could not save bus status",
                        file_status_message(result));
        return;
    }

    gtk_window_destroy(dialog);
    populate_buses(window, id);
}

static void open_add_bus_dialog(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *status_label = gtk_label_new("Status");
    const char *statuses[] = {"Active", "Inactive", NULL};
    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *status_dropdown;
    GtkWidget *cancel = gtk_button_new_with_label("Cancel");
    GtkWidget *save = gtk_button_new_with_label("Save Bus");

    (void)button;
    gtk_window_set_title(GTK_WINDOW(dialog), "Add Bus");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 420, 320);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    id_entry = add_bus_form_entry(GTK_GRID(grid), "Bus ID", 0);
    number_entry = add_bus_form_entry(GTK_GRID(grid), "Bus Number", 1);
    capacity_entry = add_bus_form_entry(GTK_GRID(grid), "Seat Capacity", 2);
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    status_dropdown = gtk_drop_down_new_from_strings(statuses);
    gtk_grid_attach(GTK_GRID(grid), status_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), status_dropdown, 1, 3, 1, 1);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), save);
    gtk_box_append(GTK_BOX(box), grid);
    gtk_box_append(GTK_BOX(box), buttons);
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    g_object_set_data(G_OBJECT(dialog), "allocation-window", window);
    g_object_set_data(G_OBJECT(dialog), "bus-id-entry", id_entry);
    g_object_set_data(G_OBJECT(dialog), "bus-number-entry", number_entry);
    g_object_set_data(G_OBJECT(dialog), "bus-capacity-entry", capacity_entry);
    g_object_set_data(G_OBJECT(dialog), "bus-status-dropdown",
                      status_dropdown);
    g_signal_connect(save, "clicked", G_CALLBACK(save_bus_dialog_clicked),
                     dialog);
    g_signal_connect_swapped(cancel, "clicked",
                             G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

void show_seat_allocation_window(GtkWindow *parent,
                                 const char *logged_in_student_id)
{
    GtkWidget *window =
        gtk_application_window_new(gtk_window_get_application(parent));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new("Seat Allocation");
    GtkWidget *subtitle = gtk_label_new(
        "Choose a bus to view live seat availability, bookings, and the FIFO "
        "waiting list.");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *back = gtk_button_new_with_label("Back to Dashboard");
    GtkWidget *add_bus = gtk_button_new_with_label("+ Add Bus");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *bus_dropdown = gtk_drop_down_new_from_strings(NULL);
    GtkWidget *schedule_dropdown = gtk_drop_down_new_from_strings(NULL);
    GtkWidget *student_entry = gtk_entry_new();
    GtkWidget *preferences;
    GtkWidget *book = gtk_button_new_with_label("Book / Allocate Seat");
    GtkWidget *cancel = gtk_button_new_with_label("Cancel Seat");
    GtkWidget *occupancy = gtk_label_new("Occupancy unavailable");
    GtkWidget *headers = gtk_grid_new();
    GtkWidget *list = gtk_list_box_new();
    GtkWidget *scroller = gtk_scrolled_window_new();
    const char *preference_options[] = {"Any", "Window", "Front", NULL};
    const char *column_names[] = {"Bus", "Seat / Queue", "Student", "Status",
                                  "Preference"};
    BusMateSessionState state = {{0}, {0}, {0}};
    const char *student_id = logged_in_student_id;
    int column;

    gtk_window_set_title(GTK_WINDOW(window), "BusMate Seat Allocation");
    gtk_window_set_default_size(GTK_WINDOW(window), 1120, 720);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(toolbar), back);
    gtk_box_append(GTK_BOX(toolbar), add_bus);
    gtk_box_append(GTK_BOX(toolbar), refresh);

    gtk_box_append(GTK_BOX(controls), gtk_label_new("Bus"));
    gtk_widget_set_hexpand(bus_dropdown, TRUE);
    gtk_box_append(GTK_BOX(controls), bus_dropdown);
    gtk_box_append(GTK_BOX(controls), gtk_label_new("Schedule"));
    gtk_widget_set_hexpand(schedule_dropdown, TRUE);
    gtk_box_append(GTK_BOX(controls), schedule_dropdown);
    gtk_box_append(GTK_BOX(controls), gtk_label_new("Student ID"));
    gtk_entry_set_placeholder_text(GTK_ENTRY(student_entry), "Student ID");
    gtk_widget_set_size_request(student_entry, 130, -1);
    gtk_box_append(GTK_BOX(controls), student_entry);
    preferences = gtk_drop_down_new_from_strings(preference_options);
    gtk_box_append(GTK_BOX(controls), preferences);
    gtk_box_append(GTK_BOX(controls), book);
    gtk_box_append(GTK_BOX(controls), cancel);

    gtk_widget_add_css_class(occupancy, "heading");
    gtk_widget_set_halign(occupancy, GTK_ALIGN_START);
    gtk_grid_set_column_spacing(GTK_GRID(headers), 16);
    for (column = 0; column < 5; ++column)
    {
        GtkWidget *label = gtk_label_new(column_names[column]);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(headers), label, column, 0, 1, 1);
    }
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), list);
    gtk_widget_set_vexpand(scroller, TRUE);

    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), subtitle);
    gtk_box_append(GTK_BOX(box), toolbar);
    gtk_box_append(GTK_BOX(box), controls);
    gtk_box_append(GTK_BOX(box), occupancy);
    gtk_box_append(GTK_BOX(box), headers);
    gtk_box_append(GTK_BOX(box), scroller);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(G_OBJECT(window), "bus-dropdown", bus_dropdown);
    g_object_set_data(G_OBJECT(window), "schedule-dropdown",
                      schedule_dropdown);
    g_object_set_data(G_OBJECT(window), "student-id-entry", student_entry);
    g_object_set_data(G_OBJECT(window), "preference-dropdown", preferences);
    g_object_set_data(G_OBJECT(window), "occupancy-label", occupancy);
    g_object_set_data(G_OBJECT(window), "seat-list", list);
    g_signal_connect_swapped(back, "clicked",
                             G_CALLBACK(gtk_window_destroy), window);
    g_signal_connect(add_bus, "clicked", G_CALLBACK(open_add_bus_dialog),
                     window);
    g_signal_connect(refresh, "clicked", G_CALLBACK(refresh_seats_clicked),
                     window);
    g_signal_connect(bus_dropdown, "notify::selected",
                     G_CALLBACK(bus_selection_changed), window);
    g_signal_connect(schedule_dropdown, "notify::selected",
                     G_CALLBACK(schedule_selection_changed), window);
    g_signal_connect(book, "clicked", G_CALLBACK(book_seat_clicked), window);
    g_signal_connect(cancel, "clicked", G_CALLBACK(cancel_seat_clicked),
                     window);

    if (student_id == NULL &&
        file_read_session_state(&state) == FILE_STATUS_OK)
        student_id = state.student_id;
    if (student_id != NULL)
        gtk_editable_set_text(GTK_EDITABLE(student_entry), student_id);

    gtk_window_present(GTK_WINDOW(window));
    populate_buses(GTK_WINDOW(window), NULL);
}

static void open_student_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_student_login(GTK_WINDOW(data));
}

static void open_driver_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_driver_login(GTK_WINDOW(data));
}

static void open_student_management_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_student_management_window(GTK_WINDOW(data));
}

static void open_seat_allocation_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_seat_allocation_window(GTK_WINDOW(data), NULL);
}

static void exit_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    gtk_window_destroy(GTK_WINDOW(data));
}

static void apply_busmate_theme(GtkApplication *app)
{
    static const char css[] =
        "window { background-color: #141b2d; color: #f8fafc; }"
        "label { color: #f8fafc; }"
        "button { background: #2b3954; color: #f8fafc; "
        "border: 1px solid #40516f; border-radius: 8px; padding: 8px 14px; }"
        "button:hover { background: #374968; }"
        "button.primary { background: #3b82f6; border-color: #3b82f6; }"
        "entry, dropdown, list, scrolledwindow { background-color: #1f2940; "
        "color: #f8fafc; }"
        "list row { border-bottom: 1px solid #2b3954; }"
        ".stat-card { background-color: #1f2940; border: 1px solid #2b3954; "
        "border-radius: 12px; padding: 18px; }";
    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(), GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_set_data_full(G_OBJECT(app), "busmate-css-provider", provider,
                           g_object_unref);
}

static char *record_count_text(const char *path, size_t record_size,
                               const char *label)
{
    void *records = NULL;
    size_t count = 0;
    FileStatus status = file_read_records(path, &records, record_size, &count);
    char *text;

    free(records);
    if (status == FILE_STATUS_NOT_FOUND)
        count = 0;
    else if (status != FILE_STATUS_OK)
        return g_strdup_printf("%s: unavailable", label);
    text = g_strdup_printf("%s: %lu records", label, (unsigned long)count);
    return text;
}

static GtkWidget *create_summary_card(const char *text)
{
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_add_css_class(card, "stat-card");
    gtk_widget_set_hexpand(card, TRUE);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(card), label);
    g_object_set_data(G_OBJECT(card), "summary-label", label);
    return card;
}

static void update_dashboard_summary(GtkWindow *window)
{
    GtkWidget *students_label = GTK_WIDGET(g_object_get_data(
        G_OBJECT(window), "students-summary-label"));
    GtkWidget *seats_label = GTK_WIDGET(g_object_get_data(
        G_OBJECT(window), "seats-summary-label"));
    char *students;
    char *seats;

    if (students_label == NULL || seats_label == NULL)
        return;
    students = record_count_text(FILE_STUDENTS_PATH, sizeof(Student),
                                 "Students");
    seats = record_count_text(FILE_SEATS_PATH,
                              sizeof(SeatWaitingListEntry), "Seats");
    gtk_label_set_text(GTK_LABEL(students_label), students);
    gtk_label_set_text(GTK_LABEL(seats_label), seats);
    g_free(students);
    g_free(seats);
}

static gboolean refresh_dashboard_summary(gpointer data)
{
    DashboardTimer *timer = (DashboardTimer *)data;
    GObject *window = g_weak_ref_get(&timer->window);

    if (window == NULL)
        return G_SOURCE_REMOVE;
    update_dashboard_summary(GTK_WINDOW(window));
    g_object_unref(window);
    return G_SOURCE_CONTINUE;
}

static void free_dashboard_timer(gpointer data)
{
    DashboardTimer *timer = (DashboardTimer *)data;

    g_weak_ref_clear(&timer->window);
    g_free(timer);
}

void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *header;
    GtkWidget *title;
    GtkWidget *subtitle;
    GtkWidget *summary;
    GtkWidget *students_card;
    GtkWidget *seats_card;
    GtkWidget *workspace_title;
    GtkWidget *workspace;
    GtkWidget *students_button;
    GtkWidget *seats_button;
    GtkWidget *student_button;
    GtkWidget *driver_button;
    GtkWidget *exit_button;

    (void)user_data;
    apply_busmate_theme(app);
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window),
                         "BUSMATE - College Transport Operations Center");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 680);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    gtk_widget_set_margin_top(box, 28);
    gtk_widget_set_margin_bottom(box, 28);
    gtk_widget_set_margin_start(box, 36);
    gtk_widget_set_margin_end(box, 36);

    header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    title = gtk_label_new("BUSMATE - College Transport Operations Center");
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    subtitle = gtk_label_new("Live operations for students, buses, and seats");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(header), title);
    gtk_box_append(GTK_BOX(header), subtitle);

    summary = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    students_card = create_summary_card("Students: loading");
    seats_card = create_summary_card("Seats: loading");
    gtk_box_append(GTK_BOX(summary), students_card);
    gtk_box_append(GTK_BOX(summary), seats_card);

    workspace_title = gtk_label_new("Operations Workspace");
    gtk_widget_add_css_class(workspace_title, "title-2");
    gtk_widget_set_halign(workspace_title, GTK_ALIGN_START);
    workspace = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    students_button = gtk_button_new_with_label("Students");
    seats_button = gtk_button_new_with_label("Seat Allocation");
    gtk_widget_add_css_class(students_button, "primary");
    gtk_widget_add_css_class(seats_button, "primary");
    student_button = gtk_button_new_with_label("Student Login");
    driver_button = gtk_button_new_with_label("Driver Interface");
    exit_button = gtk_button_new_with_label("Exit BusMate");

    gtk_box_append(GTK_BOX(workspace), students_button);
    gtk_box_append(GTK_BOX(workspace), seats_button);
    gtk_box_append(GTK_BOX(workspace), student_button);
    gtk_box_append(GTK_BOX(workspace), driver_button);
    gtk_box_append(GTK_BOX(workspace), exit_button);
    gtk_box_append(GTK_BOX(box), header);
    gtk_box_append(GTK_BOX(box), summary);
    gtk_box_append(GTK_BOX(box), workspace_title);
    gtk_box_append(GTK_BOX(box), workspace);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(G_OBJECT(window), "students-summary-label",
                      g_object_get_data(G_OBJECT(students_card),
                                        "summary-label"));
    g_object_set_data(G_OBJECT(window), "seats-summary-label",
                      g_object_get_data(G_OBJECT(seats_card), "summary-label"));
    update_dashboard_summary(GTK_WINDOW(window));
    {
        DashboardTimer *timer = g_new0(DashboardTimer, 1);
        g_weak_ref_init(&timer->window, G_OBJECT(window));
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 2,
                                   refresh_dashboard_summary, timer,
                                   free_dashboard_timer);
    }

    g_signal_connect(students_button, "clicked",
                     G_CALLBACK(open_student_management_clicked), window);
    g_signal_connect(seats_button, "clicked",
                     G_CALLBACK(open_seat_allocation_clicked), window);
    g_signal_connect(student_button, "clicked",
                     G_CALLBACK(open_student_clicked), window);
    g_signal_connect(driver_button, "clicked",
                     G_CALLBACK(open_driver_clicked), window);
    g_signal_connect(exit_button, "clicked",
                     G_CALLBACK(exit_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}
