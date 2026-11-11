#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "gui.h"
#include "file_handler.h"
#include "seat_allocation.h"

typedef enum
{
    STUDENTS,
    BUSES,
    ROUTES,
    DRIVERS,
    SCHEDULES,
    SEATS,
    REPORTS
} ModuleType;

typedef struct
{
    GtkWidget *window;
    GtkWidget *tree;
    GtkWidget *bus_combo;
    GtkWidget *student_entry;
    GtkWidget *preference_combo;
    GtkWidget *occupancy_label;
    ModuleType type;
} ModuleWindow;

static const char *module_name(ModuleType type)
{
    const char *names[] = {"Student Management", "Bus Management", "Route Management", "Driver Management", "Schedule Management", "Seat Allocation", "Reports & Analytics"};
    return names[type];
}

static void show_message(const char *title, const char *message, GtkMessageType type)
{
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, type, GTK_BUTTONS_CLOSE, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void show_module_message(ModuleWindow *module, const char *title, const char *message, GtkMessageType type)
{
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(module->window), GTK_DIALOG_MODAL, type, GTK_BUTTONS_CLOSE, "%s", message);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void add_column(GtkTreeView *tree, int index, const char *title)
{
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(title, renderer, "text", index, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(tree, column);
}

static int load_data_counts(int counts[6]);
static int parse_integer(const char *text, int *value);

static GtkListStore *load_store(ModuleType type)
{
    GtkListStore *store;

    if (type == STUDENTS)
    {
        Student *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        if (!loadAllStudentsDynamic(&items, &count))
        {
            g_warning("Unable to load student records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].studentId, 1, items[i].name, 2, items[i].collegeId, 3, "Active", -1);
        }
        free(items);
    }
    else if (type == BUSES)
    {
        Bus *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
        if (!loadAllBusesDynamic(&items, &count))
        {
            g_warning("Unable to load bus records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].busId, 1, items[i].busNumber, 2, items[i].capacity, 3, items[i].availableSeats, 4, items[i].routeId, 5, items[i].status, -1);
        }
        free(items);
    }
    else if (type == ROUTES)
    {
        Route *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        if (!loadAllRoutesDynamic(&items, &count))
        {
            g_warning("Unable to load route records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].routeId, 1, items[i].routeName, 2, items[i].stops, 3, items[i].timing, 4, items[i].type, -1);
        }
        free(items);
    }
    else if (type == DRIVERS)
    {
        Driver *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
        if (!loadAllDriversDynamic(&items, &count))
        {
            g_warning("Unable to load driver records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].driverId, 1, items[i].name, 2, items[i].phone, 3, items[i].assignedBusId, -1);
        }
        free(items);
    }
    else if (type == SCHEDULES)
    {
        Schedule *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        if (!loadAllSchedulesDynamic(&items, &count))
        {
            g_warning("Unable to load schedule records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].scheduleId, 1, items[i].busId, 2, items[i].routeId, 3, items[i].startTime, 4, items[i].endTime, 5, items[i].dayType, -1);
        }
        free(items);
    }
    else if (type == SEATS)
    {
        Seat *items = NULL;
        int count = 0;
        int i;
        store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);
        if (!loadAllSeatsDynamic(&items, &count))
        {
            g_warning("Unable to load seat records.");
            return store;
        }
        for (i = 0; i < count; i++)
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, items[i].busId, 1, items[i].seatNumber, 2, items[i].studentId, 3, items[i].status, 4, items[i].preference, -1);
        }
        free(items);
    }
    else
    {
        int counts[6] = {0};
        int countsLoaded = load_data_counts(counts);
        char summary[128];
        store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        if (countsLoaded)
            snprintf(summary, sizeof(summary), "%d students | %d buses | %d routes", counts[0], counts[1], counts[2]);
        else
            snprintf(summary, sizeof(summary), "One or more data files could not be read");
        gtk_list_store_set(store, &iter, 0, "Daily operations", 1, summary, 2, countsLoaded ? "Live" : "Error", -1);
        gtk_list_store_append(store, &iter);
        if (countsLoaded)
            snprintf(summary, sizeof(summary), "%d seat records across the fleet", counts[5]);
        else
            snprintf(summary, sizeof(summary), "One or more data files could not be read");
        gtk_list_store_set(store, &iter, 0, "Occupancy", 1, summary, 2, countsLoaded ? "Calculated" : "Error", -1);
        gtk_list_store_append(store, &iter);
        if (countsLoaded)
            snprintf(summary, sizeof(summary), "%d drivers | %d schedules", counts[3], counts[4]);
        else
            snprintf(summary, sizeof(summary), "One or more data files could not be read");
        gtk_list_store_set(store, &iter, 0, "Assignments", 1, summary, 2, countsLoaded ? "Tracked" : "Error", -1);
    }

    return store;
}

static int selected_seat_bus(ModuleWindow *module)
{
    const char *busIdText;
    int busId;
    Bus bus;

    if (module->bus_combo == NULL)
        return 0;
    busIdText = gtk_combo_box_get_active_id(GTK_COMBO_BOX(module->bus_combo));
    if (!parse_integer(busIdText, &busId) || !loadBusById(busId, &bus))
        return 0;
    return bus.busId;
}

static GtkListStore *load_seat_store_for_bus(int busId)
{
    Seat *items = NULL;
    int count = 0;
    int capacity;
    int waitingPosition = 0;
    int i;
    int seatNumber;
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

    if (busId <= 0)
        return store;
    capacity = getBusSeatCapacity(busId);
    if (capacity <= 0 || !loadAllSeatsDynamic(&items, &count))
    {
        g_warning("Unable to load bus capacity or seat records for bus %d.", busId);
        g_object_unref(store);
        return NULL;
    }
    for (seatNumber = 1; seatNumber <= capacity; seatNumber++)
    {
        const char *status = "Available";
        const char *preference = "Any";
        int studentId = 0;
        for (i = 0; i < count; i++)
        {
            if (items[i].busId == busId && items[i].seatNumber == seatNumber &&
                (strcmp(items[i].status, "Booked") == 0 || strcmp(items[i].status, "Available") == 0))
            {
                status = items[i].status;
                preference = items[i].preference;
                studentId = items[i].studentId;
                break;
            }
        }
        {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, busId, 1, seatNumber, 2, studentId,
                               3, status, 4, preference, -1);
        }
    }
    for (i = 0; i < count; i++)
    {
        if (items[i].busId == busId && items[i].seatNumber == 0 &&
            strcmp(items[i].status, "Waiting") == 0)
        {
            GtkTreeIter iter;
            char queueStatus[32];
            waitingPosition++;
            snprintf(queueStatus, sizeof(queueStatus), "Waiting #%d", waitingPosition);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, busId, 1, waitingPosition,
                               2, items[i].studentId, 3, queueStatus, 4, items[i].preference, -1);
        }
    }
    free(items);
    return store;
}

static void refresh_module(ModuleWindow *module)
{
    GtkListStore *store = module->type == SEATS
                              ? load_seat_store_for_bus(selected_seat_bus(module))
                              : load_store(module->type);
    if (store == NULL)
        return;
    gtk_tree_view_set_model(GTK_TREE_VIEW(module->tree), GTK_TREE_MODEL(store));
    g_object_unref(store);
    if (module->type == SEATS && module->occupancy_label != NULL)
    {
        int busId = selected_seat_bus(module);
        int capacity = getBusSeatCapacity(busId);
        int occupied = getAllocatedSeatCount(busId);
        char text[128];
        if (capacity < 0 || occupied < 0)
            snprintf(text, sizeof(text), "Occupancy: unavailable");
        else
            snprintf(text, sizeof(text), "Occupancy: %d / %d seats | %d available",
                     occupied, capacity, capacity - occupied);
        gtk_label_set_text(GTK_LABEL(module->occupancy_label), text);
    }
}

static void bus_selection_changed(GtkComboBox *combo, gpointer user_data)
{
    ModuleWindow *module = user_data;
    (void)combo;
    refresh_module(module);
}

static int read_seat_action_inputs(ModuleWindow *module, int *busId, int *studentId,
                                   gchar **preference)
{
    if (module->bus_combo == NULL ||
        gtk_combo_box_get_active(GTK_COMBO_BOX(module->bus_combo)) < 0 ||
        !parse_integer(gtk_entry_get_text(GTK_ENTRY(module->student_entry)), studentId) ||
        *studentId <= 0)
    {
        show_module_message(module, "Selection required",
                            "Choose a bus and enter a valid student ID.", GTK_MESSAGE_WARNING);
        return 0;
    }
    *busId = selected_seat_bus(module);
    if (*busId <= 0)
    {
        show_module_message(module, "Bus unavailable",
                            "The selected bus could not be loaded.", GTK_MESSAGE_ERROR);
        return 0;
    }
    *preference = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(module->preference_combo));
    return 1;
}

static void allocate_seat_clicked(ModuleWindow *module)
{
    int busId;
    int studentId;
    int assignedSeat = 0;
    gchar *preference = NULL;
    int result;
    char message[160];

    if (!read_seat_action_inputs(module, &busId, &studentId, &preference))
        return;
    if (preference == NULL)
    {
        show_module_message(module, "Preference required", "Choose Window, Front, or Any.", GTK_MESSAGE_WARNING);
        return;
    }
    result = requestStudentSeat(busId, studentId, preference, &assignedSeat);
    g_free(preference);
    if (result == 1)
        snprintf(message, sizeof(message), "Seat %d allocated successfully.", assignedSeat);
    else if (result == 0)
        snprintf(message, sizeof(message), "The bus is full. Student %d was added to the waiting list.", studentId);
    else
    {
        show_module_message(module, "Allocation failed",
                            "Check that the student exists and does not already have a booking or waiting-list entry.",
                            GTK_MESSAGE_WARNING);
        return;
    }
    refresh_module(module);
    show_module_message(module, result == 1 ? "Seat allocated" : "Added to waiting list",
                        message, GTK_MESSAGE_INFO);
}

static void cancel_seat_clicked(ModuleWindow *module)
{
    int busId;
    int studentId;
    gchar *preference = NULL;

    if (!read_seat_action_inputs(module, &busId, &studentId, &preference))
        return;
    g_free(preference);
    if (!cancelStudentSeat(studentId, busId))
    {
        show_module_message(module, "Cancellation failed",
                            "No active seat booking was found for this student on the selected bus.",
                            GTK_MESSAGE_WARNING);
        return;
    }
    refresh_module(module);
    show_module_message(module, "Seat cancelled",
                        "The seat was cancelled. The first waiting student, if any, has been promoted.",
                        GTK_MESSAGE_INFO);
}

static void populate_bus_combo(GtkComboBoxText *combo)
{
    Bus *buses = NULL;
    int count = 0;
    int i;

    gtk_combo_box_text_remove_all(combo);
    if (!loadAllBusesDynamic(&buses, &count))
    {
        g_warning("Unable to load buses for seat allocation.");
        return;
    }
    for (i = 0; i < count; i++)
    {
        char busId[32];
        char label[128];
        snprintf(busId, sizeof(busId), "%d", buses[i].busId);
        snprintf(label, sizeof(label), "Bus %d | %s | %d seats",
                 buses[i].busId, buses[i].busNumber, buses[i].capacity);
        gtk_combo_box_text_append(combo, busId, label);
    }
    free(buses);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), -1);
}

static int load_data_counts(int counts[6])
{
    Student *students = NULL;
    Bus *buses = NULL;
    Route *routes = NULL;
    Driver *drivers = NULL;
    Schedule *schedules = NULL;
    Seat *seats = NULL;
    int loaded = 1;

    loaded &= loadAllStudentsDynamic(&students, &counts[0]);
    loaded &= loadAllBusesDynamic(&buses, &counts[1]);
    loaded &= loadAllRoutesDynamic(&routes, &counts[2]);
    loaded &= loadAllDriversDynamic(&drivers, &counts[3]);
    loaded &= loadAllSchedulesDynamic(&schedules, &counts[4]);
    loaded &= loadAllSeatsDynamic(&seats, &counts[5]);
    free(students);
    free(buses);
    free(routes);
    free(drivers);
    free(schedules);
    free(seats);
    return loaded;
}

static int valid_text(const char *text)
{
    while (text != NULL && (*text == ' ' || *text == '\t'))
        text++;
    return text != NULL && text[0] != '\0' && text[0] != '\n';
}

static int parse_integer(const char *text, int *value)
{
    char *end;
    long parsed;

    if (!valid_text(text) || value == NULL)
        return 0;
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX || end == text || *end != '\0')
        return 0;
    *value = (int)parsed;
    return 1;
}

static int is_numeric_field(ModuleType type, int index)
{
    if (index == 0)
        return 1;
    if (type == BUSES)
        return index == 2 || index == 3 || index == 4;
    if (type == DRIVERS)
        return index == 4;
    if (type == SCHEDULES)
        return index == 1 || index == 2;
    return 0;
}

static int selected_id(ModuleWindow *module, int *secondary_id)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(module->tree));
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    int id = 0;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return 0;

    gtk_tree_model_get(model, &iter, 0, &id, -1);
    if (secondary_id != NULL)
    {
        *secondary_id = 0;
        if (module->type == SEATS)
            gtk_tree_model_get(model, &iter, 1, secondary_id, -1);
    }
    return id;
}

static int delete_record(ModuleType type, int id)
{
    switch (type)
    {
    case STUDENTS:
        return deleteStudentById(id);
    case BUSES:
        return deleteBusById(id);
    case ROUTES:
        return deleteRouteById(id);
    case DRIVERS:
        return deleteDriverById(id);
    case SCHEDULES:
        return deleteScheduleById(id);
    default:
        return 0;
    }
}

static void delete_selected(ModuleWindow *module)
{
    int secondary_id;
    int id = selected_id(module, &secondary_id);
    GtkWidget *confirm;
    (void)secondary_id;
    if (id == 0)
    {
        show_message("Selection required", "Select a record in the table first.", GTK_MESSAGE_WARNING);
        return;
    }
    confirm = gtk_message_dialog_new(GTK_WINDOW(module->window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "Delete record %d from %s?", id, module_name(module->type));
    if (gtk_dialog_run(GTK_DIALOG(confirm)) != GTK_RESPONSE_YES)
    {
        gtk_widget_destroy(confirm);
        return;
    }
    gtk_widget_destroy(confirm);
    if (delete_record(module->type, id))
    {
        refresh_module(module);
        show_message("Record Deleted", "The selected record was removed from the data store.", GTK_MESSAGE_INFO);
    }
    else
        show_message("Delete Failed", "The selected record could not be removed.", GTK_MESSAGE_ERROR);
}

static GtkWidget *make_entry(GtkWidget *grid, const char *label_text, int row);

static void manage_seat(ModuleWindow *module)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Manage Seat", GTK_WINDOW(module->window), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "Save Seat", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *bus_field;
    GtkWidget *seat_field;
    GtkWidget *student_field;
    GtkWidget *status_field;
    GtkWidget *preference_field;
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    gtk_container_set_border_width(GTK_CONTAINER(area), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    bus_field = make_entry(grid, "Bus ID", 0);
    seat_field = make_entry(grid, "Seat Number", 1);
    student_field = make_entry(grid, "Student ID (0 only for Available)", 2);
    status_field = make_entry(grid, "Status (Booked or Available)", 3);
    preference_field = make_entry(grid, "Preference (Front Row, Middle Row, Corner Seat, Any)", 4);
    gtk_box_pack_start(GTK_BOX(area), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        int bus_id;
        int seat_number;
        int student_id;
        const char *status = gtk_entry_get_text(GTK_ENTRY(status_field));
        const char *preference = gtk_entry_get_text(GTK_ENTRY(preference_field));
        Bus bus;
        Student student;

        if (!parse_integer(gtk_entry_get_text(GTK_ENTRY(bus_field)), &bus_id) ||
            !parse_integer(gtk_entry_get_text(GTK_ENTRY(seat_field)), &seat_number) ||
            !parse_integer(gtk_entry_get_text(GTK_ENTRY(student_field)), &student_id))
            show_module_message(module, "Validation Error", "Bus, seat, and student IDs must be whole numbers.", GTK_MESSAGE_WARNING);
        else if (bus_id <= 0 || !loadBusById(bus_id, &bus))
            show_module_message(module, "Validation Error", "Enter a bus ID that exists.", GTK_MESSAGE_WARNING);
        else if (seat_number <= 0 || seat_number > bus.capacity || student_id < 0 ||
                 !valid_text(status) || !valid_text(preference))
            show_module_message(module, "Validation Error", "Enter a seat within the bus capacity, a nonnegative student ID, and a valid status and preference.", GTK_MESSAGE_WARNING);
        else if ((strcmp(status, "Booked") != 0 && strcmp(status, "Available") != 0) ||
                 (strcmp(preference, "Front Row") != 0 && strcmp(preference, "Middle Row") != 0 &&
                  strcmp(preference, "Corner Seat") != 0 && strcmp(preference, "Any") != 0))
            show_module_message(module, "Validation Error", "Choose Booked or Available and a supported seat preference.", GTK_MESSAGE_WARNING);
        else if ((strcmp(status, "Booked") == 0 &&
                  (student_id <= 0 || !loadStudentById(student_id, &student))) ||
                 (strcmp(status, "Available") == 0 && student_id != 0))
            show_module_message(module, "Validation Error", "Booked seats require an existing student; Available seats require student ID 0.", GTK_MESSAGE_WARNING);
        else
        {
            if (updateSeatStatus(bus_id, seat_number, student_id, status, preference))
            {
                refresh_module(module);
                show_module_message(module, "Seat Updated", "Seat allocation was recorded successfully.", GTK_MESSAGE_INFO);
            }
            else
                show_module_message(module, "Seat Update Failed", "The seat is unavailable, the student ID is invalid, or the data could not be saved.", GTK_MESSAGE_WARNING);
        }
    }
    gtk_widget_destroy(dialog);
}

static GtkWidget *make_entry(GtkWidget *grid, const char *label_text, int row)
{
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *entry = gtk_entry_new();
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), label_text);
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, row, 1, 1);
    return entry;
}

static void add_bus_for_seat_module(ModuleWindow *module)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Add Bus", GTK_WINDOW(module->window), GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL, "Save Bus", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *bus_id_field;
    GtkWidget *bus_number_field;
    GtkWidget *capacity_field;
    GtkWidget *status_field;
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    gtk_container_set_border_width(GTK_CONTAINER(area), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    bus_id_field = make_entry(grid, "Bus ID", 0);
    bus_number_field = make_entry(grid, "Bus Number", 1);
    capacity_field = make_entry(grid, "Seat Capacity", 2);
    status_field = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_field), "Active");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_field), "Inactive");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_field), "Maintenance");
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Status:"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), status_field, 1, 3, 1, 1);
    gtk_entry_set_max_length(GTK_ENTRY(bus_id_field), 10);
    gtk_entry_set_max_length(GTK_ENTRY(bus_number_field), sizeof(((Bus *)0)->busNumber) - 1);
    gtk_entry_set_max_length(GTK_ENTRY(capacity_field), 10);
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_field), -1);
    gtk_box_pack_start(GTK_BOX(area), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        Bus bus = {0};
        const char *busNumber = gtk_entry_get_text(GTK_ENTRY(bus_number_field));
        gchar *status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_field));
        int saved = 0;

        if (!parse_integer(gtk_entry_get_text(GTK_ENTRY(bus_id_field)), &bus.busId) ||
            !parse_integer(gtk_entry_get_text(GTK_ENTRY(capacity_field)), &bus.capacity) ||
            bus.busId <= 0 || bus.capacity <= 0 || !valid_text(busNumber) ||
            strlen(busNumber) >= sizeof(bus.busNumber) || status == NULL)
        {
            show_module_message(module, "Validation Error",
                                "Enter a unique positive bus ID, a bus number, a positive seat capacity, and a status.",
                                GTK_MESSAGE_WARNING);
        }
        else
        {
            bus.availableSeats = bus.capacity;
            snprintf(bus.busNumber, sizeof(bus.busNumber), "%s", busNumber);
            snprintf(bus.status, sizeof(bus.status), "%s", status);
            saved = saveBus(bus);
            if (!saved)
                show_module_message(module, "Bus Save Failed",
                                    "The bus could not be saved. Check that its ID is unique and the data files are writable.",
                                    GTK_MESSAGE_ERROR);
            else
            {
                char busId[32];
                populate_bus_combo(GTK_COMBO_BOX_TEXT(module->bus_combo));
                snprintf(busId, sizeof(busId), "%d", bus.busId);
                gtk_combo_box_set_active_id(GTK_COMBO_BOX(module->bus_combo), busId);
                refresh_module(module);
                show_module_message(module, "Bus Added",
                                    "The bus and its seat capacity were saved successfully.",
                                    GTK_MESSAGE_INFO);
            }
        }
        g_free(status);
    }
    gtk_widget_destroy(dialog);
}

static void add_record(ModuleWindow *module)
{
    GtkWidget *dialog;
    GtkWidget *grid;
    GtkWidget *fields[6] = {0};
    const char *labels[6] = {0};
    int count = 0;
    int i;

    if (module->type == SEATS || module->type == REPORTS)
    {
        show_message("Read-only view", "Seat allocation and reports are displayed here. Use the console seat workflow for reservations.", GTK_MESSAGE_INFO);
        return;
    }
    if (module->type == STUDENTS)
    {
        labels[0] = "Student ID";
        labels[1] = "Name";
        labels[2] = "College ID";
        labels[3] = "Password";
        count = 4;
    }
    else if (module->type == BUSES)
    {
        labels[0] = "Bus ID";
        labels[1] = "Bus Number";
        labels[2] = "Seat Capacity";
        labels[3] = "Route ID";
        labels[4] = "Driver ID";
        labels[5] = "Status";
        count = 6;
    }
    else if (module->type == ROUTES)
    {
        labels[0] = "Route ID";
        labels[1] = "Route Name";
        labels[2] = "Stops";
        labels[3] = "Timing";
        labels[4] = "Type";
        count = 5;
    }
    else if (module->type == DRIVERS)
    {
        labels[0] = "Driver ID";
        labels[1] = "Name";
        labels[2] = "Phone";
        labels[3] = "Password";
        labels[4] = "Assigned Bus ID";
        count = 5;
    }
    else
    {
        labels[0] = "Schedule ID";
        labels[1] = "Bus ID";
        labels[2] = "Route ID";
        labels[3] = "Start Time";
        labels[4] = "End Time";
        labels[5] = "Day Type";
        count = 6;
    }

    dialog = gtk_dialog_new_with_buttons("Add Record", GTK_WINDOW(module->window), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "Save Record", GTK_RESPONSE_ACCEPT, NULL);
    grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    for (i = 0; i < count; i++)
        fields[i] = make_entry(grid, labels[i], i);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        int values[6] = {0};
        for (i = 0; i < count; i++)
        {
            const char *text = gtk_entry_get_text(GTK_ENTRY(fields[i]));
            if (!valid_text(text))
            {
                show_message("Validation Error", "Complete every field before saving the record.", GTK_MESSAGE_WARNING);
                gtk_widget_destroy(dialog);
                return;
            }
            if (is_numeric_field(module->type, i) && !parse_integer(text, &values[i]))
            {
                show_message("Validation Error", "ID, capacity, and assignment fields must be whole numbers.", GTK_MESSAGE_WARNING);
                gtk_widget_destroy(dialog);
                return;
            }
        }
        if (values[0] <= 0 ||
            (module->type == BUSES && (values[2] <= 0 || values[3] < 0 || values[4] < 0)) ||
            (module->type == DRIVERS && values[4] < 0) ||
            (module->type == SCHEDULES && (values[1] <= 0 || values[2] <= 0)))
        {
            show_message("Validation Error", "IDs and capacity must be positive; optional assignment IDs must be zero or greater.", GTK_MESSAGE_WARNING);
            gtk_widget_destroy(dialog);
            return;
        }
        if (module->type == BUSES)
        {
            Route route;
            Driver driver;
            if (!loadRouteById(values[3], &route) || (values[4] > 0 && !loadDriverById(values[4], &driver)))
            {
                show_message("Validation Error", "Choose an existing route and, if provided, an existing driver.", GTK_MESSAGE_WARNING);
                gtk_widget_destroy(dialog);
                return;
            }
        }
        if (module->type == DRIVERS && values[4] > 0)
        {
            Bus bus;
            if (!loadBusById(values[4], &bus))
            {
                show_message("Validation Error", "Choose an existing bus or enter 0 for no assignment.", GTK_MESSAGE_WARNING);
                gtk_widget_destroy(dialog);
                return;
            }
        }
        if (module->type == SCHEDULES)
        {
            Bus bus;
            Route route;
            if (!loadBusById(values[1], &bus) || !loadRouteById(values[2], &route))
            {
                show_message("Validation Error", "Schedules must reference an existing bus and route.", GTK_MESSAGE_WARNING);
                gtk_widget_destroy(dialog);
                return;
            }
        }

        int saved = 0;
        if (module->type == STUDENTS)
        {
            Student item = {0};
            item.studentId = values[0];
            snprintf(item.name, sizeof(item.name), "%s", gtk_entry_get_text(GTK_ENTRY(fields[1])));
            snprintf(item.collegeId, sizeof(item.collegeId), "%s", gtk_entry_get_text(GTK_ENTRY(fields[2])));
            snprintf(item.password, sizeof(item.password), "%s", gtk_entry_get_text(GTK_ENTRY(fields[3])));
            saved = saveStudent(item);
        }
        else if (module->type == BUSES)
        {
            Bus item = {0};
            item.busId = values[0];
            item.capacity = values[2];
            item.availableSeats = values[2];
            item.routeId = values[3];
            item.driverId = values[4];
            snprintf(item.busNumber, sizeof(item.busNumber), "%s", gtk_entry_get_text(GTK_ENTRY(fields[1])));
            snprintf(item.status, sizeof(item.status), "%s", gtk_entry_get_text(GTK_ENTRY(fields[5])));
            saved = saveBus(item);
        }
        else if (module->type == ROUTES)
        {
            Route item = {0};
            item.routeId = values[0];
            snprintf(item.routeName, sizeof(item.routeName), "%s", gtk_entry_get_text(GTK_ENTRY(fields[1])));
            snprintf(item.stops, sizeof(item.stops), "%s", gtk_entry_get_text(GTK_ENTRY(fields[2])));
            snprintf(item.timing, sizeof(item.timing), "%s", gtk_entry_get_text(GTK_ENTRY(fields[3])));
            snprintf(item.type, sizeof(item.type), "%s", gtk_entry_get_text(GTK_ENTRY(fields[4])));
            saved = saveRoute(item);
        }
        else if (module->type == DRIVERS)
        {
            Driver item = {0};
            item.driverId = values[0];
            item.assignedBusId = values[4];
            snprintf(item.name, sizeof(item.name), "%s", gtk_entry_get_text(GTK_ENTRY(fields[1])));
            snprintf(item.phone, sizeof(item.phone), "%s", gtk_entry_get_text(GTK_ENTRY(fields[2])));
            snprintf(item.password, sizeof(item.password), "%s", gtk_entry_get_text(GTK_ENTRY(fields[3])));
            saved = saveDriver(item);
        }
        else
        {
            Schedule item = {0};
            item.scheduleId = values[0];
            item.busId = values[1];
            item.routeId = values[2];
            snprintf(item.startTime, sizeof(item.startTime), "%s", gtk_entry_get_text(GTK_ENTRY(fields[3])));
            snprintf(item.endTime, sizeof(item.endTime), "%s", gtk_entry_get_text(GTK_ENTRY(fields[4])));
            snprintf(item.dayType, sizeof(item.dayType), "%s", gtk_entry_get_text(GTK_ENTRY(fields[5])));
            saved = saveSchedule(item);
        }
        if (saved)
        {
            refresh_module(module);
            show_message("Record Saved", "The record was added successfully.", GTK_MESSAGE_INFO);
        }
        else
            show_message("Save Failed", "The record could not be saved. Check that the ID is unique and the data files are writable.", GTK_MESSAGE_ERROR);
    }
    gtk_widget_destroy(dialog);
}

static void open_module(ModuleType type)
{
    ModuleWindow *module = g_new0(ModuleWindow, 1);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *seat_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *subtitle = gtk_label_new(type == SEATS
                                            ? "Choose a bus to view live seat availability, bookings, and the FIFO waiting list."
                                            : "Live records from the BusMate data store");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *back_button = gtk_button_new_with_label("Back to Dashboard");
    GtkWidget *add_button = gtk_button_new_with_label("+ Add Record");
    GtkWidget *add_bus_button = gtk_button_new_with_label("+ Add Bus");
    GtkWidget *seat_button = gtk_button_new_with_label("Manage Seat");
    GtkWidget *delete_button = gtk_button_new_with_label("Delete Selected");
    GtkWidget *refresh_button = gtk_button_new_with_label("Refresh");
    GtkWidget *bus_label;
    GtkWidget *student_label;
    GtkWidget *preference_label;
    GtkWidget *allocate_button;
    GtkWidget *cancel_button;
    GtkListStore *store;

    module->type = type;
    module->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (type == SEATS)
    {
        module->bus_combo = gtk_combo_box_text_new();
        module->student_entry = gtk_entry_new();
        module->preference_combo = gtk_combo_box_text_new();
        module->occupancy_label = gtk_label_new("Occupancy: unavailable");
        populate_bus_combo(GTK_COMBO_BOX_TEXT(module->bus_combo));
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(module->preference_combo), "Window");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(module->preference_combo), "Front");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(module->preference_combo), "Any");
        gtk_combo_box_set_active(GTK_COMBO_BOX(module->preference_combo), -1);

        bus_label = gtk_label_new("Bus:");
        student_label = gtk_label_new("Student ID:");
        preference_label = gtk_label_new("Preference:");
        allocate_button = gtk_button_new_with_label("Book / Allocate Seat");
        cancel_button = gtk_button_new_with_label("Cancel Seat");
        gtk_entry_set_placeholder_text(GTK_ENTRY(module->student_entry), "Student ID");
        gtk_entry_set_max_length(GTK_ENTRY(module->student_entry), 10);
        gtk_widget_set_size_request(module->bus_combo, 250, -1);
        gtk_widget_set_size_request(module->student_entry, 100, -1);
        gtk_widget_set_size_request(module->preference_combo, 110, -1);
        gtk_box_pack_start(GTK_BOX(seat_controls), bus_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), module->bus_combo, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), student_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), module->student_entry, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), preference_label, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), module->preference_combo, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), allocate_button, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(seat_controls), cancel_button, FALSE, FALSE, 0);
        g_signal_connect_swapped(allocate_button, "clicked", G_CALLBACK(allocate_seat_clicked), module);
        g_signal_connect_swapped(cancel_button, "clicked", G_CALLBACK(cancel_seat_clicked), module);
        g_signal_connect(module->bus_combo, "changed", G_CALLBACK(bus_selection_changed), module);
    }
    store = type == SEATS
                ? load_seat_store_for_bus(selected_seat_bus(module))
                : load_store(type);
    gtk_window_set_title(GTK_WINDOW(module->window), module_name(type));
    gtk_window_set_default_size(GTK_WINDOW(module->window), 1000, 620);
    gtk_container_set_border_width(GTK_CONTAINER(module->window), 18);
    gtk_label_set_markup(GTK_LABEL(title), g_strdup_printf("<span size='x-large' weight='bold'>%s</span>", module_name(type)));
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_box_pack_start(GTK_BOX(toolbar), back_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), add_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), add_bus_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), seat_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), delete_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), refresh_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), seat_controls, FALSE, FALSE, 0);
    if (type == SEATS)
    {
        gtk_label_set_xalign(GTK_LABEL(module->occupancy_label), 0.0);
        gtk_box_pack_start(GTK_BOX(box), module->occupancy_label, FALSE, FALSE, 0);
    }
    module->tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(module->tree), GTK_TREE_VIEW_GRID_LINES_BOTH);
    if (type == STUDENTS)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "ID");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Name");
        add_column(GTK_TREE_VIEW(module->tree), 2, "College ID");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Status");
    }
    else if (type == BUSES)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "ID");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Bus Number");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Capacity");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Available");
        add_column(GTK_TREE_VIEW(module->tree), 4, "Route");
        add_column(GTK_TREE_VIEW(module->tree), 5, "Status");
    }
    else if (type == ROUTES)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "ID");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Name");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Stops");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Timing");
        add_column(GTK_TREE_VIEW(module->tree), 4, "Type");
    }
    else if (type == DRIVERS)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "ID");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Name");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Phone");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Assigned Bus");
    }
    else if (type == SCHEDULES)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "ID");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Bus");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Route");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Start");
        add_column(GTK_TREE_VIEW(module->tree), 4, "End");
        add_column(GTK_TREE_VIEW(module->tree), 5, "Day Type");
    }
    else if (type == SEATS)
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "Bus");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Seat / Queue");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Student");
        add_column(GTK_TREE_VIEW(module->tree), 3, "Status");
        add_column(GTK_TREE_VIEW(module->tree), 4, "Preference");
    }
    else
    {
        add_column(GTK_TREE_VIEW(module->tree), 0, "Report");
        add_column(GTK_TREE_VIEW(module->tree), 1, "Coverage");
        add_column(GTK_TREE_VIEW(module->tree), 2, "Status");
    }
    g_object_unref(store);
    gtk_container_add(GTK_CONTAINER(scroll), module->tree);
    gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(module->window), box);
    gtk_widget_set_visible(add_button, type != SEATS && type != REPORTS);
    gtk_widget_set_visible(add_bus_button, type == SEATS);
    gtk_widget_set_visible(seat_button, FALSE);
    gtk_widget_set_visible(delete_button, type != SEATS && type != REPORTS);
    gtk_widget_set_visible(seat_controls, type == SEATS);
    g_signal_connect_swapped(back_button, "clicked", G_CALLBACK(gtk_widget_destroy), module->window);
    g_signal_connect_swapped(add_button, "clicked", G_CALLBACK(add_record), module);
    g_signal_connect_swapped(add_bus_button, "clicked", G_CALLBACK(add_bus_for_seat_module), module);
    g_signal_connect_swapped(seat_button, "clicked", G_CALLBACK(manage_seat), module);
    g_signal_connect_swapped(delete_button, "clicked", G_CALLBACK(delete_selected), module);
    g_signal_connect_swapped(refresh_button, "clicked", G_CALLBACK(refresh_module), module);
    g_signal_connect_swapped(module->window, "destroy", G_CALLBACK(g_free), module);
    gtk_widget_show_all(module->window);
    gtk_widget_set_visible(add_button, type != SEATS && type != REPORTS);
    gtk_widget_set_visible(add_bus_button, type == SEATS);
    gtk_widget_set_visible(seat_button, FALSE);
    gtk_widget_set_visible(delete_button, type != SEATS && type != REPORTS);
    if (type == SEATS)
        refresh_module(module);
}

static void open_module_clicked(GtkWidget *button, gpointer data)
{
    (void)button;
    open_module(GPOINTER_TO_INT(data));
}

static void apply_theme(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css = "window, dialog { background: #101827; color: #f8fafc; } box, grid { background: transparent; } label { color: #f8fafc; } button { background: #263449; color: #ffffff; border: 1px solid #64748b; border-radius: 7px; padding: 10px 14px; font-weight: 700; } button:hover { background: #365277; } button:disabled { color: #94a3b8; } entry { background: #f8fafc; color: #111827; caret-color: #111827; padding: 8px; } frame { background: #182235; border: 1px solid #475569; border-radius: 10px; padding: 10px; } treeview { background: #172033; color: #f8fafc; } treeview.view { color: #f8fafc; } treeview:selected { background: #2563eb; color: #ffffff; } treeview header button { background: #263449; color: #ffffff; font-weight: 700; } scrolledwindow { border: 1px solid #475569; }";
    GError *error = NULL;
    if (!gtk_css_provider_load_from_data(provider, css, -1, &error))
    {
        g_warning("Unable to load BusMate theme: %s", error->message);
        g_error_free(error);
        g_object_unref(provider);
        return;
    }
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static GtkWidget *create_card(const char *title, const char *value)
{
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *name = gtk_label_new(title);
    GtkWidget *amount = gtk_label_new(value);
    gtk_label_set_xalign(GTK_LABEL(name), 0.0);
    gtk_label_set_xalign(GTK_LABEL(amount), 0.0);
    gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), amount, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_widget_set_size_request(frame, 175, 82);
    return frame;
}

void start_gui(int argc, char *argv[])
{
    GtkWidget *window, *main_box, *summary_grid, *controls, *header, *section, *button;
    const char *card_names[] = {"Students", "Seats"};
    char card_values[2][48];
    const char *actions[] = {"Students", "Seat Allocation"};
    ModuleType types[] = {STUDENTS, SEATS};
    int counts[6] = {0};
    int i;

    gtk_init(&argc, &argv);
    if (!load_data_counts(counts))
        g_warning("One or more BusMate data files could not be read; dashboard counts may be incomplete.");
    apply_theme();
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    summary_grid = gtk_grid_new();
    controls = gtk_grid_new();
    gtk_window_set_title(GTK_WINDOW(window), "BusMate Professional Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 820, 560);
    gtk_container_set_border_width(GTK_CONTAINER(window), 22);
    snprintf(card_values[0], sizeof(card_values[0]), "%d records", counts[0]);
    snprintf(card_values[1], sizeof(card_values[1]), "%d seat records", counts[5]);
    header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold' foreground='#93c5fd'>BUSMATE</span>\n<span size='medium'>College Transport Operations Center</span>");
    gtk_label_set_xalign(GTK_LABEL(header), 0.0);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);
    gtk_grid_set_column_spacing(GTK_GRID(summary_grid), 12);
    gtk_grid_set_row_spacing(GTK_GRID(summary_grid), 12);
    for (i = 0; i < 2; i++)
        gtk_grid_attach(GTK_GRID(summary_grid), create_card(card_names[i], card_values[i]), i, 0, 1, 1);
    gtk_box_pack_start(GTK_BOX(main_box), summary_grid, FALSE, FALSE, 0);
    section = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(section), "<span size='large' weight='bold'>Operations Workspace</span>");
    gtk_label_set_xalign(GTK_LABEL(section), 0.0);
    gtk_box_pack_start(GTK_BOX(main_box), section, FALSE, FALSE, 0);
    gtk_grid_set_column_spacing(GTK_GRID(controls), 12);
    gtk_grid_set_row_spacing(GTK_GRID(controls), 12);
    for (i = 0; i < 2; i++)
    {
        button = gtk_button_new_with_label(actions[i]);
        gtk_widget_set_size_request(button, 260, 52);
        g_signal_connect(button, "clicked", G_CALLBACK(open_module_clicked), GINT_TO_POINTER(types[i]));
        gtk_grid_attach(GTK_GRID(controls), button, i, 0, 1, 1);
    }
    button = gtk_button_new_with_label("Exit BusMate");
    gtk_widget_set_size_request(button, 540, 50);
    g_signal_connect(button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_grid_attach(GTK_GRID(controls), button, 0, 1, 2, 1);
    gtk_box_pack_start(GTK_BOX(main_box), controls, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}