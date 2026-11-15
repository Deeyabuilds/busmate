#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file_handler.h"
#include "../include/seat_allocation.h"
#include "../include/student.h"

static GtkWidget *add_labeled_entry(GtkWidget *box, const char *label_text,
                                    gboolean password)
{
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *entry = gtk_entry_new();

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    if (password)
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), entry);
    return entry;
}

static int has_valid_id(const char *id, size_t capacity)
{
    return id != NULL && id[0] != '\0' && memchr(id, '\0', capacity) != NULL;
}

static void set_status(GtkWindow *window, const char *message)
{
    GtkWidget *label = GTK_WIDGET(g_object_get_data(
        G_OBJECT(window), "status-label"));

    if (label != NULL)
        gtk_label_set_text(GTK_LABEL(label), message);
}

int register_student(Student student)
{
    if (student.student_id[0] == '\0' ||
        memchr(student.student_id, '\0', sizeof(student.student_id)) == NULL ||
        student.name[0] == '\0' ||
        memchr(student.name, '\0', sizeof(student.name)) == NULL ||
        student.password[0] == '\0' ||
        memchr(student.password, '\0', sizeof(student.password)) == NULL)
        return 0;

    return file_save_record(FILE_STUDENTS_PATH, &student, sizeof(student),
                            offsetof(Student, student_id),
                            sizeof(student.student_id)) == FILE_STATUS_OK;
}

int login_student(const char *student_id, const char *password)
{
    Student student;
    BusMateSessionState state = {{0}, {0}, {0}};
    FileStatus status;

    if (student_id == NULL || password == NULL ||
        student_id[0] == '\0' || password[0] == '\0' ||
        strlen(student_id) >= sizeof(student.student_id) ||
        strlen(password) >= sizeof(student.password))
        return 0;

    status = file_find_record(FILE_STUDENTS_PATH, student_id, &student,
                              sizeof(student),
                              offsetof(Student, student_id),
                              sizeof(student.student_id));
    if (status != FILE_STATUS_OK)
    {
        if (status != FILE_STATUS_NOT_FOUND)
            fprintf(stderr, "Could not read student account: %s\n",
                    file_status_message(status));
        return 0;
    }

    if (memchr(student.password, '\0', sizeof(student.password)) == NULL ||
        strcmp(student.password, password) != 0)
        return 0;

    status = file_read_session_state(&state);
    if (status != FILE_STATUS_OK && status != FILE_STATUS_NOT_FOUND)
    {
        fprintf(stderr, "Could not read active session: %s\n",
                file_status_message(status));
        return 0;
    }
    snprintf(state.student_id, sizeof(state.student_id), "%s", student_id);
    status = file_write_session_state(&state);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not save active student session: %s\n",
                file_status_message(status));
        return 0;
    }
    return 1;
}

static void update_dashboard(GtkWindow *window, const char *action_message)
{
    const char *student_id =
        (const char *)g_object_get_data(G_OBJECT(window), "student-id");
    GtkWidget *schedule_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "schedule-entry"));
    GtkTextBuffer *buffer =
        GTK_TEXT_BUFFER(g_object_get_data(G_OBJECT(window), "report-buffer"));
    const char *schedule_id =
        gtk_editable_get_text(GTK_EDITABLE(schedule_entry));
    GString *report = g_string_new(NULL);
    SeatAvailability availability;
    void *booking_data = NULL;
    void *waitlist_data = NULL;
    size_t booking_count = 0;
    size_t waitlist_count = 0;
    Booking *bookings;
    SeatWaitingListEntry *waitlist;
    size_t index;
    size_t waiting_position = 0;
    int user_seat = 0;
    int user_waiting = 0;
    FileStatus status;

    if (action_message != NULL && action_message[0] != '\0')
        g_string_append_printf(report, "%s\n\n", action_message);
    if (schedule_id[0] == '\0')
    {
        g_string_append(report, "Enter a schedule ID to view seats or book.");
        gtk_text_buffer_set_text(buffer, report->str, -1);
        g_string_free(report, TRUE);
        return;
    }

    if (!get_seat_availability(schedule_id, &availability))
    {
        g_string_append_printf(
            report,
            "Could not load this schedule. Check that the schedule and its bus "
            "are set up and that the schedule ID is correct.\n");
        gtk_text_buffer_set_text(buffer, report->str, -1);
        g_string_free(report, TRUE);
        return;
    }

    status = file_read_records(FILE_BOOKINGS_PATH, &booking_data,
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
        gtk_text_buffer_set_text(buffer, report->str, -1);
        g_string_free(report, TRUE);
        return;
    }

    status = file_read_records(FILE_WAITLIST_PATH, &waitlist_data,
                               sizeof(SeatWaitingListEntry), &waitlist_count);
    if (status == FILE_STATUS_NOT_FOUND)
    {
        waitlist_count = 0;
        status = FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
    {
        free(booking_data);
        g_string_append_printf(report, "Could not read waiting list: %s",
                               file_status_message(status));
        gtk_text_buffer_set_text(buffer, report->str, -1);
        g_string_free(report, TRUE);
        return;
    }

    bookings = (Booking *)booking_data;
    waitlist = (SeatWaitingListEntry *)waitlist_data;
    for (index = 0; index < booking_count; ++index)
    {
        if (!has_valid_id(bookings[index].schedule_id,
                          sizeof(bookings[index].schedule_id)) ||
            !has_valid_id(bookings[index].student_id,
                          sizeof(bookings[index].student_id)))
        {
            free(booking_data);
            free(waitlist_data);
            g_string_free(report, TRUE);
            gtk_text_buffer_set_text(buffer,
                                     "Booking data contains an invalid record.",
                                     -1);
            return;
        }
        if (strcmp(bookings[index].schedule_id, schedule_id) == 0 &&
            strcmp(bookings[index].student_id, student_id) == 0)
            user_seat = bookings[index].seat_number;
    }
    for (index = 0; index < waitlist_count; ++index)
    {
        if (!has_valid_id(waitlist[index].booking.schedule_id,
                          sizeof(waitlist[index].booking.schedule_id)) ||
            !has_valid_id(waitlist[index].booking.student_id,
                          sizeof(waitlist[index].booking.student_id)))
        {
            free(booking_data);
            free(waitlist_data);
            g_string_free(report, TRUE);
            gtk_text_buffer_set_text(
                buffer, "Waiting-list data contains an invalid record.", -1);
            return;
        }
        if (strcmp(waitlist[index].booking.schedule_id, schedule_id) != 0)
            continue;
        if (strcmp(waitlist[index].booking.student_id, student_id) == 0)
            user_waiting = 1;
    }

    g_string_append_printf(report, "Schedule: %s\n", schedule_id);
    g_string_append_printf(report,
                           "Seats: %d total | %d occupied | %d available\n",
                           availability.total_seats,
                           availability.occupied_seats,
                           availability.available_seats);
    if (user_seat > 0)
        g_string_append_printf(report, "Your seat: %d\n", user_seat);
    else if (user_waiting)
    {
        size_t position = 0;
        for (index = 0; index < waitlist_count; ++index)
        {
            if (strcmp(waitlist[index].booking.schedule_id, schedule_id) != 0)
                continue;
            ++position;
            if (strcmp(waitlist[index].booking.student_id, student_id) == 0)
            {
                waiting_position = position;
                break;
            }
        }
        g_string_append_printf(report, "Waiting list position: %lu\n",
                               (unsigned long)waiting_position);
    }
    else
        g_string_append(report, "You have no booking for this schedule.\n");

    g_string_append(report, "\nSeat map\n");
    for (index = 1; index <= (size_t)availability.total_seats; ++index)
    {
        size_t booking_index;
        const char *seat_status = "Available";
        const char *occupant = NULL;

        for (booking_index = 0; booking_index < booking_count; ++booking_index)
        {
            if (strcmp(bookings[booking_index].schedule_id, schedule_id) == 0 &&
                bookings[booking_index].seat_number == (int)index)
            {
                seat_status = "Booked";
                occupant = bookings[booking_index].student_id;
                break;
            }
        }
        if (occupant != NULL)
            g_string_append_printf(report, "Seat %lu: %s (%s)\n",
                                   (unsigned long)index, seat_status, occupant);
        else
            g_string_append_printf(report, "Seat %lu: %s\n",
                                   (unsigned long)index, seat_status);
    }

    free(booking_data);
    free(waitlist_data);
    gtk_text_buffer_set_text(buffer, report->str, -1);
    g_string_free(report, TRUE);
}

static void refresh_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    update_dashboard(GTK_WINDOW(data), NULL);
}

static void book_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *schedule_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "schedule-entry"));
    GtkWidget *preference_dropdown =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "preference-dropdown"));
    const char *student_id =
        (const char *)g_object_get_data(G_OBJECT(window), "student-id");
    const char *schedule_id =
        gtk_editable_get_text(GTK_EDITABLE(schedule_entry));
    guint selected = gtk_drop_down_get_selected(
        GTK_DROP_DOWN(preference_dropdown));
    Booking booking = {0};
    SeatAllocationResult result;
    char message[160];

    (void)button;
    if (schedule_id[0] == '\0' || selected > SEAT_PREFERENCE_FRONT)
    {
        update_dashboard(window, "Enter a schedule ID and select a preference.");
        return;
    }

    snprintf(booking.student_id, sizeof(booking.student_id), "%s", student_id);
    snprintf(booking.schedule_id, sizeof(booking.schedule_id), "%s",
             schedule_id);
    result = allocate_seat_with_preference(&booking, (SeatPreference)selected);

    if (result == SEAT_ALLOCATION_BOOKED)
        snprintf(message, sizeof(message), "Booking confirmed: seat %d.",
                 booking.seat_number);
    else if (result == SEAT_ALLOCATION_WAITLISTED)
        snprintf(message, sizeof(message),
                 "All seats are booked. You have been added to the waiting list.");
    else if (result == SEAT_ALLOCATION_DUPLICATE)
        snprintf(message, sizeof(message),
                 "You already have a booking or waiting-list request.");
    else
        snprintf(message, sizeof(message),
                 "Booking failed. Check the schedule and stored records.");

    update_dashboard(window, message);
}

static void cancel_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *schedule_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "schedule-entry"));
    const char *student_id =
        (const char *)g_object_get_data(G_OBJECT(window), "student-id");
    const char *schedule_id =
        gtk_editable_get_text(GTK_EDITABLE(schedule_entry));

    (void)button;
    if (schedule_id[0] == '\0')
    {
        update_dashboard(window, "Enter a schedule ID before cancelling.");
        return;
    }

    if (cancel_seat(student_id, schedule_id))
        update_dashboard(window,
                         "Your booking or waiting-list request was cancelled.");
    else
        update_dashboard(window,
                         "Cancellation failed: no matching request was found "
                         "or the data could not be updated.");
}

void show_student_dashboard(GtkWindow *parent, const char *student_id)
{
    static const char *preferences[] = {
        "Any available seat", "Window seat", "Front seat", NULL
    };
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *schedule_label;
    GtkWidget *schedule_entry;
    GtkWidget *preference_label;
    GtkWidget *preference_dropdown;
    GtkWidget *button_row;
    GtkWidget *book_button;
    GtkWidget *cancel_button;
    GtkWidget *refresh_button;
    GtkWidget *scroller;
    GtkWidget *report_view;
    GtkTextBuffer *report_buffer;
    window = GTK_WIDGET(parent);
    gtk_window_set_title(GTK_WINDOW(window), "Student Seat Allocation");
    gtk_window_set_default_size(GTK_WINDOW(window), 650, 620);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);

    title = gtk_label_new("Book your bus seat");
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), title);

    schedule_label = gtk_label_new("Schedule ID");
    gtk_widget_set_halign(schedule_label, GTK_ALIGN_START);
    schedule_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(schedule_entry),
                                   "Enter the schedule ID");
    gtk_box_append(GTK_BOX(box), schedule_label);
    gtk_box_append(GTK_BOX(box), schedule_entry);

    preference_label = gtk_label_new("Seat preference");
    gtk_widget_set_halign(preference_label, GTK_ALIGN_START);
    preference_dropdown = gtk_drop_down_new_from_strings(preferences);
    gtk_box_append(GTK_BOX(box), preference_label);
    gtk_box_append(GTK_BOX(box), preference_dropdown);

    button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    book_button = gtk_button_new_with_label("Book seat");
    cancel_button = gtk_button_new_with_label("Cancel booking");
    refresh_button = gtk_button_new_with_label("Refresh seats");
    gtk_box_append(GTK_BOX(button_row), book_button);
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
    g_object_set_data_full(G_OBJECT(window), "student-id",
                           g_strdup(student_id), g_free);
    g_object_set_data(G_OBJECT(window), "schedule-entry", schedule_entry);
    g_object_set_data(G_OBJECT(window), "preference-dropdown",
                      preference_dropdown);
    g_object_set_data(G_OBJECT(window), "report-buffer", report_buffer);

    g_signal_connect(book_button, "clicked", G_CALLBACK(book_clicked), window);
    g_signal_connect(cancel_button, "clicked",
                     G_CALLBACK(cancel_clicked), window);
    g_signal_connect(refresh_button, "clicked",
                     G_CALLBACK(refresh_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
    update_dashboard(GTK_WINDOW(window), NULL);
}

static void registration_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *id_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "id-entry"));
    GtkWidget *name_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "name-entry"));
    GtkWidget *password_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "password-entry"));
    Student student = {0};
    const char *id = gtk_editable_get_text(GTK_EDITABLE(id_entry));
    const char *name = gtk_editable_get_text(GTK_EDITABLE(name_entry));
    const char *password =
        gtk_editable_get_text(GTK_EDITABLE(password_entry));

    (void)button;
    if (strlen(id) >= sizeof(student.student_id) ||
        strlen(name) >= sizeof(student.name) ||
        strlen(password) >= sizeof(student.password))
    {
        set_status(window, "One or more fields exceed the allowed length.");
        return;
    }

    snprintf(student.student_id, sizeof(student.student_id), "%s", id);
    snprintf(student.name, sizeof(student.name), "%s", name);
    snprintf(student.password, sizeof(student.password), "%s", password);

    if (register_student(student))
    {
        BusMateSessionState state = {{0}, {0}, {0}};
        FileStatus state_status = file_read_session_state(&state);
        GtkWindow *login_window = GTK_WINDOW(
            g_object_get_data(G_OBJECT(window), "login-window"));
        if (state_status != FILE_STATUS_OK &&
            state_status != FILE_STATUS_NOT_FOUND)
        {
            set_status(window, "Could not read shared session state.");
            return;
        }
        snprintf(state.student_id, sizeof(state.student_id), "%s",
                 student.student_id);
        state_status = file_write_session_state(&state);
        if (state_status != FILE_STATUS_OK)
        {
            set_status(window, "Could not save shared session state.");
            return;
        }
        gtk_window_destroy(window);
        show_student_dashboard(login_window, student.student_id);
    }
    else
    {
        set_status(window,
                   "Registration failed. Fill every field and use a unique ID.");
    }
}

static void show_student_registration(GtkWindow *login_window)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;
    GtkWidget *register_button;
    GtkWidget *status_label;

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Create Student Account");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 460);
    gtk_window_set_transient_for(GTK_WINDOW(window), login_window);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);
    gtk_widget_set_margin_start(box, 28);
    gtk_widget_set_margin_end(box, 28);

    title = gtk_label_new("Student registration");
    gtk_widget_add_css_class(title, "title-2");
    gtk_box_append(GTK_BOX(box), title);
    id_entry = add_labeled_entry(box, "Student ID", FALSE);
    name_entry = add_labeled_entry(box, "Full name", FALSE);
    password_entry = add_labeled_entry(box, "Password", TRUE);
    register_button = gtk_button_new_with_label("Create account");
    status_label = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(status_label), TRUE);
    gtk_box_append(GTK_BOX(box), register_button);
    gtk_box_append(GTK_BOX(box), status_label);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(G_OBJECT(window), "id-entry", id_entry);
    g_object_set_data(G_OBJECT(window), "name-entry", name_entry);
    g_object_set_data(G_OBJECT(window), "password-entry", password_entry);
    g_object_set_data(G_OBJECT(window), "status-label", status_label);
    g_object_set_data(G_OBJECT(window), "login-window", login_window);
    g_signal_connect(register_button, "clicked",
                     G_CALLBACK(registration_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}

static void login_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *id_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "id-entry"));
    GtkWidget *password_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "password-entry"));
    const char *student_id = gtk_editable_get_text(GTK_EDITABLE(id_entry));
    const char *password =
        gtk_editable_get_text(GTK_EDITABLE(password_entry));

    (void)button;
    if (!login_student(student_id, password))
    {
        set_status(window, "Student ID or password is incorrect.");
        return;
    }

    {
        char *student_id_copy = g_strdup(student_id);
        show_student_dashboard(window, student_id_copy);
        g_free(student_id_copy);
    }
}

static void open_registration_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_student_registration(GTK_WINDOW(data));
}

void show_student_login(GtkWindow *parent)
{
    GtkApplication *app = gtk_window_get_application(parent);
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *id_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;
    GtkWidget *register_button;
    GtkWidget *status_label;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Student Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 420);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);
    gtk_widget_set_margin_start(box, 28);
    gtk_widget_set_margin_end(box, 28);

    title = gtk_label_new("Student login");
    gtk_widget_add_css_class(title, "title-2");
    gtk_box_append(GTK_BOX(box), title);
    id_entry = add_labeled_entry(box, "Student ID", FALSE);
    password_entry = add_labeled_entry(box, "Password", TRUE);
    login_button = gtk_button_new_with_label("Log in");
    register_button = gtk_button_new_with_label("Create an account");
    status_label = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(status_label), TRUE);
    gtk_box_append(GTK_BOX(box), login_button);
    gtk_box_append(GTK_BOX(box), register_button);
    gtk_box_append(GTK_BOX(box), status_label);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(G_OBJECT(window), "id-entry", id_entry);
    g_object_set_data(G_OBJECT(window), "password-entry", password_entry);
    g_object_set_data(G_OBJECT(window), "status-label", status_label);
    g_signal_connect(login_button, "clicked",
                     G_CALLBACK(login_clicked), window);
    g_signal_connect(register_button, "clicked",
                     G_CALLBACK(open_registration_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}

typedef struct {
    char student_id[20];
    char college_id[30];
    char status[20];
} StudentProfile;

static void show_management_message(GtkWindow *parent, const char *message)
{
    GtkWidget *dialog = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *label = gtk_label_new(message);
    GtkWidget *button = gtk_button_new_with_label("OK");

    gtk_window_set_title(GTK_WINDOW(dialog), "BusMate");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 360, 140);
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

static void append_student_row(GtkListBox *list, const Student *student)
{
    StudentProfile profile = {{0}, "--", "Active"};
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *grid = gtk_grid_new();
    FileStatus status;

    status = file_find_record(FILE_STUDENT_PROFILES_PATH, student->student_id,
                              &profile, sizeof(profile),
                              offsetof(StudentProfile, student_id),
                              sizeof(profile.student_id));
    if (status != FILE_STATUS_OK && status != FILE_STATUS_NOT_FOUND)
    {
        show_management_message(GTK_WINDOW(gtk_widget_get_root(
                                    GTK_WIDGET(list))),
                                "Could not read student profile metadata.");
        return;
    }
    if (status == FILE_STATUS_NOT_FOUND)
    {
        snprintf(profile.college_id, sizeof(profile.college_id), "--");
        snprintf(profile.status, sizeof(profile.status), "Active");
    }
    if (memchr(profile.college_id, '\0', sizeof(profile.college_id)) == NULL ||
        memchr(profile.status, '\0', sizeof(profile.status)) == NULL)
    {
        show_management_message(GTK_WINDOW(gtk_widget_get_root(
                                    GTK_WIDGET(list))),
                                "A student profile record is corrupt.");
        return;
    }

    gtk_grid_set_column_spacing(GTK_GRID(grid), 18);
    gtk_widget_set_margin_top(grid, 8);
    gtk_widget_set_margin_bottom(grid, 8);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(student->student_id), 0, 0,
                    1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(student->name), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(profile.college_id), 2, 0,
                    1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(profile.status), 3, 0, 1, 1);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), grid);
    g_object_set_data_full(G_OBJECT(row), "student-id",
                           g_strdup(student->student_id), g_free);
    gtk_list_box_append(list, row);
}

static void refresh_student_management(GtkWindow *window)
{
    GtkListBox *list = GTK_LIST_BOX(g_object_get_data(
        G_OBJECT(window), "student-list"));
    GtkWidget *child;
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status;

    child = gtk_widget_get_first_child(GTK_WIDGET(list));
    while (child != NULL)
    {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(list, child);
        child = next;
    }

    status = file_read_records(FILE_STUDENTS_PATH, &records, sizeof(Student),
                               &count);
    if (status == FILE_STATUS_NOT_FOUND)
        return;
    if (status != FILE_STATUS_OK)
    {
        show_management_message(window, file_status_message(status));
        return;
    }

    for (index = 0; index < count; ++index)
    {
        Student *students = (Student *)records;
        if (memchr(students[index].student_id, '\0',
                   sizeof(students[index].student_id)) == NULL ||
            memchr(students[index].name, '\0',
                   sizeof(students[index].name)) == NULL)
        {
            free(records);
            show_management_message(window,
                                    "Student data contains a corrupt record.");
            return;
        }
        append_student_row(list, &students[index]);
    }
    free(records);
}

static void save_management_student(GtkButton *button, gpointer data)
{
    GtkWindow *dialog = GTK_WINDOW(data);
    GtkWindow *window = GTK_WINDOW(g_object_get_data(
        G_OBJECT(dialog), "management-window"));
    GtkWidget *id_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "student-id"));
    GtkWidget *name_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "student-name"));
    GtkWidget *college_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "college-id"));
    GtkWidget *password_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "student-password"));
    const char *id = gtk_editable_get_text(GTK_EDITABLE(id_entry));
    const char *name = gtk_editable_get_text(GTK_EDITABLE(name_entry));
    const char *college = gtk_editable_get_text(GTK_EDITABLE(college_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(password_entry));
    Student student = {0};
    StudentProfile profile = {{0}, {0}, "Active"};
    FileStatus status;

    (void)button;
    if (id[0] == '\0' || strlen(id) >= sizeof(student.student_id) ||
        name[0] == '\0' || strlen(name) >= sizeof(student.name) ||
        college[0] == '\0' || strlen(college) >= sizeof(profile.college_id) ||
        password[0] == '\0' || strlen(password) >= sizeof(student.password))
    {
        show_management_message(dialog,
                                "Complete each field using the allowed length.");
        return;
    }

    snprintf(student.student_id, sizeof(student.student_id), "%s", id);
    snprintf(student.name, sizeof(student.name), "%s", name);
    snprintf(student.password, sizeof(student.password), "%s", password);
    if (!register_student(student))
    {
        show_management_message(dialog,
                                "Could not save the student; the ID may "
                                "already exist or the data store is unavailable.");
        return;
    }

    snprintf(profile.student_id, sizeof(profile.student_id), "%s", id);
    snprintf(profile.college_id, sizeof(profile.college_id), "%s", college);
    status = file_save_record(FILE_STUDENT_PROFILES_PATH, &profile,
                              sizeof(profile),
                              offsetof(StudentProfile, student_id),
                              sizeof(profile.student_id));
    if (status != FILE_STATUS_OK)
    {
        FileStatus rollback = file_delete_record(
            FILE_STUDENTS_PATH, id, sizeof(Student),
            offsetof(Student, student_id), sizeof(student.student_id));
        if (rollback != FILE_STATUS_OK)
            fprintf(stderr, "Could not roll back student after profile error.\n");
        show_management_message(dialog, file_status_message(status));
        return;
    }

    gtk_window_destroy(dialog);
    refresh_student_management(window);
}

static GtkWidget *add_profile_entry(GtkGrid *grid, const char *label,
                                    int row, gboolean password)
{
    GtkWidget *caption = gtk_label_new(label);
    GtkWidget *entry = gtk_entry_new();

    gtk_widget_set_halign(caption, GTK_ALIGN_START);
    if (password)
        gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_grid_attach(grid, caption, 0, row, 1, 1);
    gtk_grid_attach(grid, entry, 1, row, 1, 1);
    return entry;
}

static void add_management_student(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *dialog = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *college_entry;
    GtkWidget *password_entry;
    GtkWidget *save = gtk_button_new_with_label("Save Record");
    GtkWidget *cancel = gtk_button_new_with_label("Cancel");

    (void)button;
    gtk_window_set_title(GTK_WINDOW(dialog), "Add Student Record");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 420, 360);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    id_entry = add_profile_entry(GTK_GRID(grid), "Student ID", 0, FALSE);
    name_entry = add_profile_entry(GTK_GRID(grid), "Name", 1, FALSE);
    college_entry = add_profile_entry(GTK_GRID(grid), "College ID", 2, FALSE);
    password_entry = add_profile_entry(GTK_GRID(grid), "Password", 3, TRUE);
    gtk_box_append(GTK_BOX(buttons), cancel);
    gtk_box_append(GTK_BOX(buttons), save);
    gtk_box_append(GTK_BOX(box), grid);
    gtk_box_append(GTK_BOX(box), buttons);
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    g_object_set_data(G_OBJECT(dialog), "management-window", window);
    g_object_set_data(G_OBJECT(dialog), "student-id", id_entry);
    g_object_set_data(G_OBJECT(dialog), "student-name", name_entry);
    g_object_set_data(G_OBJECT(dialog), "college-id", college_entry);
    g_object_set_data(G_OBJECT(dialog), "student-password", password_entry);
    g_signal_connect(save, "clicked",
                     G_CALLBACK(save_management_student), dialog);
    g_signal_connect_swapped(cancel, "clicked",
                             G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

static void delete_management_student(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkListBox *list = GTK_LIST_BOX(g_object_get_data(
        G_OBJECT(window), "student-list"));
    GtkListBoxRow *row = gtk_list_box_get_selected_row(list);
    const char *id;
    FileStatus status;

    (void)button;
    if (row == NULL)
    {
        show_management_message(window, "Select a student record to delete.");
        return;
    }
    id = (const char *)g_object_get_data(G_OBJECT(row), "student-id");
    status = file_delete_record(FILE_STUDENTS_PATH, id, sizeof(Student),
                                offsetof(Student, student_id),
                                sizeof(((Student *)0)->student_id));
    if (status != FILE_STATUS_OK)
    {
        show_management_message(window, file_status_message(status));
        return;
    }
    status = file_delete_record(
        FILE_STUDENT_PROFILES_PATH, id, sizeof(StudentProfile),
        offsetof(StudentProfile, student_id),
        sizeof(((StudentProfile *)0)->student_id));
    if (status != FILE_STATUS_OK && status != FILE_STATUS_NOT_FOUND)
    {
        show_management_message(window,
                                "Student was deleted, but profile metadata "
                                "could not be removed.");
        return;
    }
    refresh_student_management(window);
}

void show_student_management_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_application_window_new(
        gtk_window_get_application(parent));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new("Student Management");
    GtkWidget *subtitle =
        gtk_label_new("Live records from the BusMate data store");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *back = gtk_button_new_with_label("Back to Dashboard");
    GtkWidget *add = gtk_button_new_with_label("+ Add Record");
    GtkWidget *delete = gtk_button_new_with_label("Delete Selected");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *headers = gtk_grid_new();
    GtkWidget *list = gtk_list_box_new();
    GtkWidget *scroller = gtk_scrolled_window_new();

    gtk_window_set_title(GTK_WINDOW(window), "BusMate Student Management");
    gtk_window_set_default_size(GTK_WINDOW(window), 860, 600);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_widget_set_margin_top(box, 22);
    gtk_widget_set_margin_bottom(box, 22);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_box_append(GTK_BOX(toolbar), back);
    gtk_box_append(GTK_BOX(toolbar), add);
    gtk_box_append(GTK_BOX(toolbar), delete);
    gtk_box_append(GTK_BOX(toolbar), refresh);
    gtk_grid_set_column_spacing(GTK_GRID(headers), 18);
    gtk_widget_set_margin_start(headers, 10);
    gtk_grid_attach(GTK_GRID(headers), gtk_label_new("ID"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(headers), gtk_label_new("Name"), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(headers), gtk_label_new("College ID"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(headers), gtk_label_new("Status"), 3, 0, 1, 1);
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list), GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), list);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), subtitle);
    gtk_box_append(GTK_BOX(box), toolbar);
    gtk_box_append(GTK_BOX(box), headers);
    gtk_box_append(GTK_BOX(box), scroller);
    gtk_window_set_child(GTK_WINDOW(window), box);
    g_object_set_data(G_OBJECT(window), "student-list", list);
    g_signal_connect_swapped(back, "clicked",
                             G_CALLBACK(gtk_window_destroy), window);
    g_signal_connect(add, "clicked", G_CALLBACK(add_management_student), window);
    g_signal_connect(delete, "clicked",
                     G_CALLBACK(delete_management_student), window);
    g_signal_connect_swapped(refresh, "clicked",
                             G_CALLBACK(refresh_student_management), window);
    gtk_window_present(GTK_WINDOW(window));
    refresh_student_management(GTK_WINDOW(window));
}
