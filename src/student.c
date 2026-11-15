#include <gtk/gtk.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/student.h"
#include "../include/file_handler.h"
#include "../include/seat_allocation.h"

/* Member 1: student registration, login and student-facing dashboard.
   Seat allocation/cancellation is delegated to Member 3's API. */

static int current_student_id = 0;

typedef struct {
    GtkWindow *window;
    GtkWidget *student_id;
    GtkWidget *password;
    GtkWidget *status;
} LoginContext;

typedef struct {
    GtkWindow *window;
    GtkWidget *bus_id;
    GtkWidget *seat_no;
    GtkWidget *preference;
    GtkWidget *output;
    GtkWidget *status;
} DashboardContext;

static void set_label(GtkWidget *label, const char *message)
{
    if (label != NULL)
        gtk_label_set_text(GTK_LABEL(label), message != NULL ? message : "");
}

static void set_output(DashboardContext *ctx, const char *message)
{
    GtkTextBuffer *buffer;
    if (ctx == NULL || ctx->output == NULL) return;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ctx->output));
    gtk_text_buffer_set_text(buffer, message != NULL ? message : "", -1);
}

static int parse_positive_int(const char *text, int *value)
{
    char *end = NULL;
    long parsed;
    if (text == NULL || *text == '\0' || value == NULL) return 0;
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || parsed <= 0 || parsed > INT_MAX)
        return 0;
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return 0;
    *value = (int)parsed;
    return 1;
}

static int read_student_records(Student **students, size_t *count)
{
    return readRecords(STUDENTS_FILE, sizeof(Student), (void **)students, count) == 0;
}

int register_student(Student student)
{
    Student *students = NULL;
    size_t count = 0, i;
    int result = 0;

    if (student.studentId <= 0 || student.name[0] == '\0' ||
        student.password[0] == '\0' ||
        memchr(student.name, '\0', sizeof(student.name)) == NULL ||
        memchr(student.password, '\0', sizeof(student.password)) == NULL)
        return 0;

    if (!read_student_records(&students, &count)) return 0;
    for (i = 0; i < count; ++i) {
        if (students[i].studentId == student.studentId) {
            free(students);
            return 0;
        }
    }
    free(students);
    result = appendRecord(STUDENTS_FILE, sizeof(Student), &student);
    return result == 0;
}

int login_student(const char *student_id, const char *password)
{
    Student *students = NULL;
    size_t count = 0, i;
    int id;
    if (!parse_positive_int(student_id, &id) || password == NULL || *password == '\0')
        return 0;
    if (!read_student_records(&students, &count)) return 0;
    for (i = 0; i < count; ++i) {
        if (students[i].studentId == id &&
            memchr(students[i].password, '\0', sizeof(students[i].password)) != NULL &&
            strcmp(students[i].password, password) == 0) {
            current_student_id = id;
            free(students);
            return 1;
        }
    }
    free(students);
    return 0;
}

static void on_registration_save(GtkButton *button, gpointer user_data)
{
    GtkWindow *dialog = GTK_WINDOW(user_data);
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "reg-id"));
    GtkWidget *name_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "reg-name"));
    GtkWidget *pass_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(dialog), "reg-pass"));
    LoginContext *ctx = g_object_get_data(G_OBJECT(dialog), "login-context");
    const char *id_text = gtk_editable_get_text(GTK_EDITABLE(id_entry));
    const char *name = gtk_editable_get_text(GTK_EDITABLE(name_entry));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(pass_entry));
    Student student = {0};
    int id;
    (void)button;

    if (!parse_positive_int(id_text, &id) || name[0] == '\0' || password[0] == '\0' ||
        strlen(name) >= sizeof(student.name) || strlen(password) >= sizeof(student.password)) {
        set_label(ctx->status, "Enter a valid ID, name and password within the allowed lengths.");
        return;
    }
    student.studentId = id;
    g_strlcpy(student.name, name, sizeof(student.name));
    g_strlcpy(student.password, password, sizeof(student.password));
    if (!register_student(student)) {
        set_label(ctx->status, "Registration failed. The ID may already exist or the data file is unavailable.");
        return;
    }
    gtk_editable_set_text(GTK_EDITABLE(ctx->student_id), id_text);
    gtk_editable_set_text(GTK_EDITABLE(ctx->password), password);
    set_label(ctx->status, "Account created. Sign in now.");
    gtk_window_destroy(dialog);
}

static void on_register_clicked(GtkButton *button, gpointer user_data)
{
    LoginContext *ctx = user_data;
    GtkWidget *dialog = gtk_window_new();
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *id = gtk_entry_new(), *name = gtk_entry_new(), *pass = gtk_entry_new();
    GtkWidget *save = gtk_button_new_with_label("Create Account");
    GtkWidget *cancel = gtk_button_new_with_label("Cancel");
    (void)button;

    gtk_window_set_title(GTK_WINDOW(dialog), "Student Registration");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 380, 260);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), ctx->window);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_entry_set_input_purpose(GTK_ENTRY(id), GTK_INPUT_PURPOSE_DIGITS);
    gtk_entry_set_max_length(GTK_ENTRY(name), sizeof(((Student *)0)->name) - 1);
    gtk_entry_set_max_length(GTK_ENTRY(pass), sizeof(((Student *)0)->password) - 1);
    gtk_entry_set_visibility(GTK_ENTRY(pass), FALSE);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Student ID"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Name"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Password"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pass, 1, 2, 1, 1);
    gtk_box_append(GTK_BOX(box), grid);
    gtk_box_append(GTK_BOX(box), save);
    gtk_box_append(GTK_BOX(box), cancel);
    gtk_widget_set_margin_top(box, 18); gtk_widget_set_margin_bottom(box, 18);
    gtk_widget_set_margin_start(box, 18); gtk_widget_set_margin_end(box, 18);
    gtk_window_set_child(GTK_WINDOW(dialog), box);

    g_object_set_data(G_OBJECT(dialog), "reg-id", id);
    g_object_set_data(G_OBJECT(dialog), "reg-name", name);
    g_object_set_data(G_OBJECT(dialog), "reg-pass", pass);
    g_object_set_data(G_OBJECT(dialog), "login-context", ctx);
    g_signal_connect(save, "clicked", G_CALLBACK(on_registration_save), dialog);
    g_signal_connect_swapped(cancel, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

static int get_bus_id(DashboardContext *ctx, int *bus_id)
{
    return parse_positive_int(gtk_editable_get_text(GTK_EDITABLE(ctx->bus_id)), bus_id);
}

static const char *get_preference(DashboardContext *ctx)
{
    GtkStringObject *selected = GTK_STRING_OBJECT(gtk_drop_down_get_selected_item(GTK_DROP_DOWN(ctx->preference)));
    return selected ? gtk_string_object_get_string(selected) : "Any";
}

static void show_buses(DashboardContext *ctx)
{
    Bus *buses = NULL; size_t count = 0, i; GString *s = g_string_new("Available buses\n\n");
    if (readRecords(BUSES_FILE, sizeof(Bus), (void **)&buses, &count) != 0) {
        g_string_free(s, TRUE); set_output(ctx, "Could not read bus records."); return;
    }
    if (!count) g_string_append(s, "No bus records found.\n");
    for (i = 0; i < count; ++i)
        g_string_append_printf(s, "Bus ID: %d | Number: %s | Capacity: %d | Available: %d\n",
            buses[i].busId, buses[i].busNumber, buses[i].capacity, buses[i].availableSeats);
    free(buses); set_output(ctx, s->str); g_string_free(s, TRUE);
}

static void show_routes(DashboardContext *ctx)
{
    Route *routes = NULL; size_t count = 0, i; GString *s = g_string_new("Routes and stops\n\n");
    if (readRecords(ROUTES_FILE, sizeof(Route), (void **)&routes, &count) != 0) {
        g_string_free(s, TRUE); set_output(ctx, "Could not read route records."); return;
    }
    if (!count) g_string_append(s, "No route records found.\n");
    for (i = 0; i < count; ++i)
        g_string_append_printf(s, "Route ID: %d | %s\nStops: %s\nFrom: %s | To: %s\n\n",
            routes[i].routeId, routes[i].routeName, routes[i].stops,
            routes[i].starting_point, routes[i].destination);
    free(routes); set_output(ctx, s->str); g_string_free(s, TRUE);
}

static void show_schedules(DashboardContext *ctx)
{
    Schedule *items = NULL; size_t count = 0, i; GString *s = g_string_new("Bus timings\n\n");
    if (readRecords(SCHEDULES_FILE, sizeof(Schedule), (void **)&items, &count) != 0) {
        g_string_free(s, TRUE); set_output(ctx, "Could not read schedule records."); return;
    }
    if (!count) g_string_append(s, "No schedule records found.\n");
    for (i = 0; i < count; ++i)
        g_string_append_printf(s, "Schedule ID: %d | Bus ID: %d | Route ID: %d\n%s - %s | %s\n\n",
            items[i].scheduleId, items[i].busId, items[i].routeId,
            items[i].startTime, items[i].endTime, items[i].dayType);
    free(items); set_output(ctx, s->str); g_string_free(s, TRUE);
}

static void on_view_buses(GtkButton *b, gpointer data) { (void)b; show_buses(data); }
static void on_view_routes(GtkButton *b, gpointer data) { (void)b; show_routes(data); }
static void on_view_schedules(GtkButton *b, gpointer data) { (void)b; show_schedules(data); }

static void on_search_bus(GtkButton *button, gpointer data)
{
    DashboardContext *ctx = data; int id; Bus *buses = NULL; size_t count = 0, i;
    (void)button;
    if (!get_bus_id(ctx, &id)) { set_output(ctx, "Enter a valid numeric Bus ID first."); return; }
    if (readRecords(BUSES_FILE, sizeof(Bus), (void **)&buses, &count) != 0) { set_output(ctx, "Could not read buses."); return; }
    for (i = 0; i < count; ++i) if (buses[i].busId == id) {
        char text[256];
        g_snprintf(text, sizeof(text), "Bus found\nBus ID: %d\nBus Number: %s\nCapacity: %d\nAvailable seats (stored count): %d",
            buses[i].busId, buses[i].busNumber, buses[i].capacity, buses[i].availableSeats);
        free(buses); set_output(ctx, text); return;
    }
    free(buses); set_output(ctx, "No bus found with that ID.");
}

static void on_check_seats(GtkButton *button, gpointer data)
{
    DashboardContext *ctx = data; int bus_id, booked = 0, capacity = 0; Seat *seats = NULL; size_t count = 0, i; GString *s;
    (void)button;
    if (!get_bus_id(ctx, &bus_id)) { set_output(ctx, "Enter a valid numeric Bus ID first."); return; }
    if (!getBusSeats(bus_id, &seats, &count) || !getBusOccupancy(bus_id, &booked, &capacity)) {
        free(seats); set_output(ctx, "Could not load seat information. Check that this bus has been initialized."); return;
    }
    s = g_string_new(NULL);
    g_string_append_printf(s, "Bus %d seat availability\nTotal: %d | Booked: %d | Available: %d\n\n",
        bus_id, capacity, booked, capacity - booked);
    for (i = 0; i < count; ++i)
        g_string_append_printf(s, "Seat %d: %s%s\n", seats[i].seatNumber, seats[i].status,
            seats[i].studentId == current_student_id ? " (Your seat)" : "");
    free(seats); set_output(ctx, s->str); g_string_free(s, TRUE);
}

static void on_book_seat(GtkButton *button, gpointer data)
{
    DashboardContext *ctx = data; int bus_id, seat_no = 0; const char *pref; Seat allocated = {0}; int result;
    (void)button;
    if (current_student_id <= 0 || !get_bus_id(ctx, &bus_id)) { set_output(ctx, "Please sign in and enter a valid Bus ID."); return; }
    pref = get_preference(ctx);
    {
        const char *seat_text = gtk_editable_get_text(GTK_EDITABLE(ctx->seat_no));
        if (seat_text[0] != '\0' && !parse_positive_int(seat_text, &seat_no)) { set_output(ctx, "Seat number must be a positive number or left blank for automatic allocation."); return; }
    }
    result = seat_no > 0 ? allocateSpecificSeat(bus_id, current_student_id, seat_no, pref, &allocated)
                         : allocateSeat(bus_id, current_student_id, pref, &allocated);
    if (result == SEAT_ALLOCATION_BOOKED) {
        char message[160];
        g_snprintf(message, sizeof(message), "Booking confirmed! Bus %d, Seat %d, Preference: %s.", bus_id, allocated.seatNumber, pref);
        set_output(ctx, message);
    } else if (result == SEAT_ALLOCATION_WAITLISTED) {
        set_output(ctx, "The bus is full. You have been added to the waiting list.");
    } else if (result == SEAT_ALLOCATION_DUPLICATE) {
        set_output(ctx, "You already have a seat or waiting-list entry for this bus.");
    } else set_output(ctx, "Booking could not be completed. Check the Bus ID and seat availability.");
}

static void on_cancel_booking(GtkButton *button, gpointer data)
{
    DashboardContext *ctx = data; int bus_id, promoted = 0; Seat freed = {0}; SeatCancellationResult result;
    (void)button;
    if (current_student_id <= 0 || !get_bus_id(ctx, &bus_id)) { set_output(ctx, "Please sign in and enter a valid Bus ID."); return; }
    result = cancelSeat(bus_id, current_student_id, &freed, &promoted);
    if (result == SEAT_CANCELLATION_COMPLETED)
        set_output(ctx, "Your booking was cancelled successfully.");
    else if (result == SEAT_CANCELLATION_PROMOTED)
        set_output(ctx, "Your booking was cancelled and a waiting-list student was promoted.");
    else if (result == SEAT_CANCELLATION_NOT_FOUND)
        set_output(ctx, "No booking was found for you on this bus.");
    else set_output(ctx, "Cancellation failed. Please try again.");
}

static void on_dashboard_logout(GtkButton *button, gpointer data)
{
    DashboardContext *ctx = data; (void)button; current_student_id = 0; gtk_window_destroy(ctx->window);
}

void show_student_dashboard(GtkWindow *parent, const char *student_id)
{
    GtkWidget *window, *outer, *title, *form, *buttons, *scroller, *preference_label;
    DashboardContext *ctx;
    int parsed_id = 0;
    if (student_id == NULL || !parse_positive_int(student_id, &parsed_id)) return;
    current_student_id = parsed_id;
    window = gtk_application_window_new(parent ? gtk_window_get_application(parent) : NULL);
    gtk_window_set_title(GTK_WINDOW(window), "BusMate - Student Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 650);
    ctx = g_new0(DashboardContext, 1); ctx->window = GTK_WINDOW(window);
    outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(outer, 18); gtk_widget_set_margin_bottom(outer, 18);
    gtk_widget_set_margin_start(outer, 20); gtk_widget_set_margin_end(outer, 20);
    title = gtk_label_new("STUDENT DASHBOARD"); gtk_widget_add_css_class(title, "title-1");
    gtk_box_append(GTK_BOX(outer), title);
    form = gtk_grid_new(); gtk_grid_set_row_spacing(GTK_GRID(form), 8); gtk_grid_set_column_spacing(GTK_GRID(form), 10);
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Bus ID"), 0, 0, 1, 1);
    ctx->bus_id = gtk_entry_new(); gtk_entry_set_input_purpose(GTK_ENTRY(ctx->bus_id), GTK_INPUT_PURPOSE_DIGITS);
    gtk_grid_attach(GTK_GRID(form), ctx->bus_id, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(form), gtk_label_new("Specific seat number (optional)"), 0, 1, 1, 1);
    ctx->seat_no = gtk_entry_new(); gtk_entry_set_input_purpose(GTK_ENTRY(ctx->seat_no), GTK_INPUT_PURPOSE_DIGITS);
    gtk_grid_attach(GTK_GRID(form), ctx->seat_no, 1, 1, 1, 1);
    preference_label = gtk_label_new("Preference"); gtk_grid_attach(GTK_GRID(form), preference_label, 0, 2, 1, 1);
    { const char *opts[] = {"Any", "Window", "Front", NULL}; GtkStringList *list = gtk_string_list_new(opts); ctx->preference = gtk_drop_down_new(G_LIST_MODEL(list), NULL); }
    gtk_grid_attach(GTK_GRID(form), ctx->preference, 1, 2, 1, 1);
    gtk_box_append(GTK_BOX(outer), form);
    buttons = gtk_grid_new(); gtk_grid_set_row_spacing(GTK_GRID(buttons), 8); gtk_grid_set_column_spacing(GTK_GRID(buttons), 8);
#define ADD_BUTTON(text, col, row, callback) do { GtkWidget *btn = gtk_button_new_with_label(text); gtk_grid_attach(GTK_GRID(buttons), btn, col, row, 1, 1); g_signal_connect(btn, "clicked", G_CALLBACK(callback), ctx); } while (0)
    ADD_BUTTON("View Available Buses", 0, 0, on_view_buses);
    ADD_BUTTON("Search Bus", 1, 0, on_search_bus);
    ADD_BUTTON("View Routes and Stops", 2, 0, on_view_routes);
    ADD_BUTTON("View Bus Timings", 0, 1, on_view_schedules);
    ADD_BUTTON("Check Seat Availability", 1, 1, on_check_seats);
    ADD_BUTTON("Book Seat", 2, 1, on_book_seat);
    ADD_BUTTON("Cancel Booking", 0, 2, on_cancel_booking);
    ADD_BUTTON("Logout", 1, 2, on_dashboard_logout);
#undef ADD_BUTTON
    gtk_box_append(GTK_BOX(outer), buttons);
    scroller = gtk_scrolled_window_new(); gtk_widget_set_vexpand(scroller, TRUE);
    ctx->output = gtk_text_view_new(); gtk_text_view_set_editable(GTK_TEXT_VIEW(ctx->output), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(ctx->output), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), ctx->output);
    gtk_box_append(GTK_BOX(outer), scroller);
    ctx->status = gtk_label_new("Choose an action. Leave seat number blank for automatic allocation.");
    gtk_widget_set_halign(ctx->status, GTK_ALIGN_START); gtk_box_append(GTK_BOX(outer), ctx->status);
    gtk_window_set_child(GTK_WINDOW(window), outer);
    g_object_set_data_full(G_OBJECT(window), "dashboard-context", ctx, g_free);
    gtk_window_present(GTK_WINDOW(window));
}

static void on_login_clicked(GtkButton *button, gpointer data)
{
    LoginContext *ctx = data;
    const char *id = gtk_editable_get_text(GTK_EDITABLE(ctx->student_id));
    const char *password = gtk_editable_get_text(GTK_EDITABLE(ctx->password));
    char id_copy[32];
    (void)button;
    if (!login_student(id, password)) { set_label(ctx->status, "Invalid Student ID or password."); return; }
    g_snprintf(id_copy, sizeof(id_copy), "%d", current_student_id);
    show_student_dashboard(ctx->window, id_copy);
    gtk_window_destroy(ctx->window);
}

void show_student_login(GtkWindow *parent)
{
    GtkWidget *window, *box, *grid, *login, *reg;
    LoginContext *ctx;
    window = gtk_application_window_new(parent ? gtk_window_get_application(parent) : NULL);
    gtk_window_set_title(GTK_WINDOW(window), "BusMate - Student Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 420, 300);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    ctx = g_new0(LoginContext, 1); ctx->window = GTK_WINDOW(window);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(box, 22); gtk_widget_set_margin_bottom(box, 22);
    gtk_widget_set_margin_start(box, 22); gtk_widget_set_margin_end(box, 22);
    gtk_box_append(GTK_BOX(box), gtk_label_new("STUDENT LOGIN"));
    grid = gtk_grid_new(); gtk_grid_set_row_spacing(GTK_GRID(grid), 10); gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Student ID"), 0, 0, 1, 1);
    ctx->student_id = gtk_entry_new(); gtk_entry_set_input_purpose(GTK_ENTRY(ctx->student_id), GTK_INPUT_PURPOSE_DIGITS);
    gtk_grid_attach(GTK_GRID(grid), ctx->student_id, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Password"), 0, 1, 1, 1);
    ctx->password = gtk_entry_new(); gtk_entry_set_visibility(GTK_ENTRY(ctx->password), FALSE);
    gtk_grid_attach(GTK_GRID(grid), ctx->password, 1, 1, 1, 1);
    gtk_box_append(GTK_BOX(box), grid);
    login = gtk_button_new_with_label("Sign In"); reg = gtk_button_new_with_label("Create Account");
    gtk_box_append(GTK_BOX(box), login); gtk_box_append(GTK_BOX(box), reg);
    ctx->status = gtk_label_new("Sign in with your Student ID and password."); gtk_box_append(GTK_BOX(box), ctx->status);
    gtk_window_set_child(GTK_WINDOW(window), box);
    g_object_set_data_full(G_OBJECT(window), "login-context", ctx, g_free);
    g_signal_connect(login, "clicked", G_CALLBACK(on_login_clicked), ctx);
    g_signal_connect(reg, "clicked", G_CALLBACK(on_register_clicked), ctx);
    gtk_window_present(GTK_WINDOW(window));
}

void show_student_management_window(GtkWindow *parent)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
        "Student records are managed by the Admin module in this project.");
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}