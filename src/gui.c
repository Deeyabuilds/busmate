#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/gui.h"
#include "../include/admin.h"
#include "../include/student.h"
#include "../include/driver.h"
#include "../include/bus.h"
#include "../include/route.h"
#include "../include/schedule.h"
#include "../include/booking.h"
#include "../include/seat_allocation.h"
#include "../include/notification.h"
#include "../include/report.h"
#include "../include/file_handler.h"
#include "../include/structures.h"


static int current_student_id = 0;
static int logged_driver_id = 0;


/* =========================================================
   COMMON GUI FUNCTIONS
   ========================================================= */

void show_message(GtkWindow *parent,
                  const char *title,
                  const char *message)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        message
    );

    gtk_window_set_title(
        GTK_WINDOW(dialog),
        title
    );

    gtk_window_present(
        GTK_WINDOW(dialog)
    );
}


GtkWidget *create_button(const char *text)
{
    GtkWidget *button;

    button = gtk_button_new_with_label(text);

    gtk_widget_set_hexpand(
        button,
        TRUE
    );

    return button;
}


GtkWidget *create_entry(const char *placeholder,
                        gboolean password)
{
    GtkWidget *entry;

    entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(entry),
        placeholder
    );

    if (password)
    {
        gtk_entry_set_visibility(
            GTK_ENTRY(entry),
            FALSE
        );
    }

    return entry;
}


GtkWidget *create_module_window(GtkWindow *parent,
                                const char *title)
{
    GtkWidget *window;
    GtkWidget *box;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        title
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        500,
        500
    );

    if (parent != NULL)
    {
        gtk_window_set_transient_for(
            GTK_WINDOW(window),
            parent
        );
    }

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(
        box,
        20
    );

    gtk_widget_set_margin_bottom(
        box,
        20
    );

    gtk_widget_set_margin_start(
        box,
        20
    );

    gtk_widget_set_margin_end(
        box,
        20
    );

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    return window;
}


/* =========================================================
   MAIN WINDOW
   ========================================================= */

static void main_student_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_login(
        GTK_WINDOW(user_data)
    );
}


static void main_driver_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_login(
        GTK_WINDOW(user_data)
    );
}


static void main_admin_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_admin_login(
        GTK_WINDOW(user_data)
    );
}


GtkWidget *create_main_window(GtkApplication *app)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *student_button;
    GtkWidget *driver_button;
    GtkWidget *admin_button;
    GtkWidget *exit_button;

    window = gtk_application_window_new(app);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "BusMate"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        500,
        500
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        15
    );

    gtk_widget_set_margin_top(
        box,
        40
    );

    gtk_widget_set_margin_bottom(
        box,
        40
    );

    gtk_widget_set_margin_start(
        box,
        40
    );

    gtk_widget_set_margin_end(
        box,
        40
    );

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "BusMate\nSmart College Bus Management System"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    student_button = create_button(
        "Student Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        student_button
    );

    driver_button = create_button(
        "Driver Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        driver_button
    );

    admin_button = create_button(
        "Admin Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        admin_button
    );

    exit_button = create_button(
        "Exit"
    );

    gtk_box_append(
        GTK_BOX(box),
        exit_button
    );

    g_signal_connect(
        student_button,
        "clicked",
        G_CALLBACK(main_student_login_clicked),
        window
    );

    g_signal_connect(
        driver_button,
        "clicked",
        G_CALLBACK(main_driver_login_clicked),
        window
    );

    g_signal_connect(
        admin_button,
        "clicked",
        G_CALLBACK(main_admin_login_clicked),
        window
    );

    g_signal_connect_swapped(
        exit_button,
        "clicked",
        G_CALLBACK(gtk_window_destroy),
        window
    );

    return window;
}


void start_gui(GtkApplication *app)
{
    GtkWidget *window;

    window = create_main_window(app);

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   ADMIN LOGIN
   ========================================================= */

static void admin_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWindow *window;

    const char *username;
    const char *password;

    username_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "username-entry"
        )
    );

    password_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "password-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    username = gtk_editable_get_text(
        GTK_EDITABLE(username_entry)
    );

    password = gtk_editable_get_text(
        GTK_EDITABLE(password_entry)
    );

    if (admin_login(username, password))
    {
        gtk_window_destroy(window);

        show_admin_dashboard(NULL);
    }
    else
    {
        show_message(
            window,
            "Login Failed",
            "Invalid admin username or password."
        );
    }
}


void show_admin_login(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;

    window = create_module_window(
        parent,
        "Admin Login"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Admin Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    username_entry = create_entry(
        "Username",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        username_entry
    );

    password_entry = create_entry(
        "Password",
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        password_entry
    );

    login_button = create_button(
        "Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        login_button
    );

    g_object_set_data(
        G_OBJECT(login_button),
        "username-entry",
        username_entry
    );

    g_object_set_data(
        G_OBJECT(login_button),
        "password-entry",
        password_entry
    );

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(admin_login_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   ADMIN DASHBOARD
   ========================================================= */

static void admin_student_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_driver_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_bus_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_route_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_route_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_schedule_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_schedule_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_booking_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_booking_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_seat_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_seat_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_notification_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_notification_module(
        GTK_WINDOW(user_data)
    );
}


static void admin_report_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_report_module(
        GTK_WINDOW(user_data)
    );
}


void show_admin_dashboard(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *student_button;
    GtkWidget *driver_button;
    GtkWidget *bus_button;
    GtkWidget *route_button;
    GtkWidget *schedule_button;
    GtkWidget *booking_button;
    GtkWidget *seat_button;
    GtkWidget *notification_button;
    GtkWidget *report_button;

    window = create_module_window(
        parent,
        "Admin Dashboard"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        600,
        700
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Admin Dashboard"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    student_button = create_button(
        "Student Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        student_button
    );

    driver_button = create_button(
        "Driver Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        driver_button
    );

    bus_button = create_button(
        "Bus Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        bus_button
    );

    route_button = create_button(
        "Route Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        route_button
    );

    schedule_button = create_button(
        "Schedule Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        schedule_button
    );

    booking_button = create_button(
        "Booking Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        booking_button
    );

    seat_button = create_button(
        "Seat Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        seat_button
    );

    notification_button = create_button(
        "Notification Management"
    );

    gtk_box_append(
        GTK_BOX(box),
        notification_button
    );

    report_button = create_button(
        "Reports"
    );

    gtk_box_append(
        GTK_BOX(box),
        report_button
    );

    g_signal_connect(
        student_button,
        "clicked",
        G_CALLBACK(admin_student_clicked),
        window
    );

    g_signal_connect(
        driver_button,
        "clicked",
        G_CALLBACK(admin_driver_clicked),
        window
    );

    g_signal_connect(
        bus_button,
        "clicked",
        G_CALLBACK(admin_bus_clicked),
        window
    );

    g_signal_connect(
        route_button,
        "clicked",
        G_CALLBACK(admin_route_clicked),
        window
    );

    g_signal_connect(
        schedule_button,
        "clicked",
        G_CALLBACK(admin_schedule_clicked),
        window
    );

    g_signal_connect(
        booking_button,
        "clicked",
        G_CALLBACK(admin_booking_clicked),
        window
    );

    g_signal_connect(
        seat_button,
        "clicked",
        G_CALLBACK(admin_seat_clicked),
        window
    );

    g_signal_connect(
        notification_button,
        "clicked",
        G_CALLBACK(admin_notification_clicked),
        window
    );

    g_signal_connect(
        report_button,
        "clicked",
        G_CALLBACK(admin_report_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   STUDENT MANAGEMENT - ADD
   ========================================================= */

static void save_student_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;

    GtkWindow *window;

    Student student;

    const char *id_text;
    const char *name;
    const char *password;

    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    password_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "password-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    name = gtk_editable_get_text(
        GTK_EDITABLE(name_entry)
    );

    password = gtk_editable_get_text(
        GTK_EDITABLE(password_entry)
    );

    student.studentId = atoi(id_text);

    strncpy(
        student.name,
        name,
        sizeof(student.name) - 1
    );

    student.name[
        sizeof(student.name) - 1
    ] = '\0';

    strncpy(
        student.password,
        password,
        sizeof(student.password) - 1
    );

    student.password[
        sizeof(student.password) - 1
    ] = '\0';

    result = add_student(student);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Student added successfully."
        );

        gtk_window_destroy(window);
    }
    else if (result == -2)
    {
        show_message(
            window,
            "Error",
            "Student ID already exists."
        );
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not add student."
        );
    }
}


static void show_add_student_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;
    GtkWidget *save_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Add Student"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        400
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Add New Student"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Student ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    name_entry = create_entry(
        "Student Name",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        name_entry
    );

    password_entry = create_entry(
        "Password",
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        password_entry
    );

    save_button = create_button(
        "Save Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        save_button
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "password-entry",
        password_entry
    );

    g_signal_connect(
        save_button,
        "clicked",
        G_CALLBACK(save_student_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   STUDENT MANAGEMENT - VIEW
   ========================================================= */

static void show_all_students(
    GtkWindow *parent)
{
    void *records;
    size_t count;
    size_t i;

    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *scroll;
    GtkWidget *list_box;

    Student *students;

    records = NULL;
    count = 0;

    if (readRecords(
            STUDENTS_FILE,
            sizeof(Student),
            &records,
            &count
        ) != 0)
    {
        show_message(
            parent,
            "Error",
            "Could not read student data."
        );

        return;
    }

    if (count == 0)
    {
        free(records);

        show_message(
            parent,
            "Students",
            "No students found."
        );

        return;
    }

    students = (Student *)records;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "View Students"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        500,
        500
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Registered Students"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    for (i = 0; i < count; i++)
    {
        GtkWidget *student_box;
        GtkWidget *student_label;

        char text[300];

        student_box = gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            4
        );

        student_label = gtk_label_new(NULL);

        snprintf(
            text,
            sizeof(text),
            "Student ID: %d\nName: %s",
            students[i].studentId,
            students[i].name
        );

        gtk_label_set_text(
            GTK_LABEL(student_label),
            text
        );

        gtk_label_set_xalign(
            GTK_LABEL(student_label),
            0.0
        );

        gtk_box_append(
            GTK_BOX(student_box),
            student_label
        );

        gtk_box_append(
            GTK_BOX(list_box),
            student_box
        );
    }

    free(records);

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void student_view_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_all_students(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   STUDENT MANAGEMENT - SEARCH
   ========================================================= */

static void search_student_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int studentId;

    Student student;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    studentId = atoi(id_text);

    if (search_student(
            studentId,
            &student
        ) == 1)
    {
        char message[300];

        snprintf(
            message,
            sizeof(message),
            "Student ID: %d\nName: %s",
            student.studentId,
            student.name
        );

        show_message(
            window,
            "Student Found",
            message
        );
    }
    else
    {
        show_message(
            window,
            "Student Not Found",
            "No student found with this ID."
        );
    }
}


static void show_search_student_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *search_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Search Student"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Search Student"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Student ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    search_button = create_button(
        "Search Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    g_object_set_data(
        G_OBJECT(search_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(search_student_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void student_search_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_search_student_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   STUDENT MANAGEMENT - UPDATE
   ========================================================= */

static void update_student_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;

    GtkWindow *window;

    Student student;

    const char *id_text;
    const char *name;
    const char *password;

    int studentId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    password_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "password-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    name = gtk_editable_get_text(
        GTK_EDITABLE(name_entry)
    );

    password = gtk_editable_get_text(
        GTK_EDITABLE(password_entry)
    );

    studentId = atoi(id_text);

    if (search_student(
            studentId,
            &student
        ) != 1)
    {
        show_message(
            window,
            "Error",
            "Student not found."
        );

        return;
    }

    strncpy(
        student.name,
        name,
        sizeof(student.name) - 1
    );

    student.name[
        sizeof(student.name) - 1
    ] = '\0';

    strncpy(
        student.password,
        password,
        sizeof(student.password) - 1
    );

    student.password[
        sizeof(student.password) - 1
    ] = '\0';

    result = update_student(student);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Student updated successfully."
        );

        gtk_window_destroy(window);
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not update student."
        );
    }
}


static void load_student_for_update(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;
    GtkWidget *update_button;

    GtkWindow *window;

    Student student;

    const char *id_text;

    int studentId;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    password_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "password-entry"
        )
    );

    update_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "update-button"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    studentId = atoi(id_text);

    if (search_student(
            studentId,
            &student
        ) == 1)
    {
        gtk_editable_set_text(
            GTK_EDITABLE(name_entry),
            student.name
        );

        gtk_editable_set_text(
            GTK_EDITABLE(password_entry),
            student.password
        );

        gtk_widget_set_sensitive(
            name_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            password_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            update_button,
            TRUE
        );

       gtk_widget_set_sensitive(
    GTK_WIDGET(button),
    FALSE
);
    }
    else
    {
        show_message(
            window,
            "Student Not Found",
            "No student found with this ID."
        );
    }
}


static void show_update_student_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *password_entry;

    GtkWidget *load_button;
    GtkWidget *update_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Update Student"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        450,
        450
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(
        box,
        25
    );

    gtk_widget_set_margin_bottom(
        box,
        25
    );

    gtk_widget_set_margin_start(
        box,
        25
    );

    gtk_widget_set_margin_end(
        box,
        25
    );

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Update Student"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Student ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    load_button = create_button(
        "Load Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        load_button
    );

    name_entry = create_entry(
        "Student Name",
        FALSE
    );

    gtk_widget_set_sensitive(
        name_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        name_entry
    );

    password_entry = create_entry(
        "Password",
        TRUE
    );

    gtk_widget_set_sensitive(
        password_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        password_entry
    );

    update_button = create_button(
        "Update Student"
    );

    gtk_widget_set_sensitive(
        update_button,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "password-entry",
        password_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "update-button",
        update_button
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "password-entry",
        password_entry
    );

    g_signal_connect(
        load_button,
        "clicked",
        G_CALLBACK(load_student_for_update),
        NULL
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(update_student_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void student_update_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_update_student_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   STUDENT MANAGEMENT - DELETE
   ========================================================= */

static void confirm_delete_student(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int studentId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    studentId = atoi(id_text);

    result = delete_student(studentId);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Student deleted successfully."
        );

        gtk_window_destroy(window);
    }
    else
    {
        show_message(
            window,
            "Error",
            "Student not found or could not be deleted."
        );
    }
}


static void delete_confirmation_response(
    GtkDialog *dialog,
    int response_id,
    gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *delete_button;

    window = GTK_WINDOW(
        g_object_get_data(
            G_OBJECT(dialog),
            "parent-window"
        )
    );

    delete_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(dialog),
            "delete-button"
        )
    );

    if (response_id == GTK_RESPONSE_YES)
    {
        confirm_delete_student(
            GTK_BUTTON(delete_button),
            NULL
        );
    }

    gtk_window_destroy(
        GTK_WINDOW(dialog)
    );

    (void)window;
}


static void delete_student_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;

    GtkWindow *window;

    const char *id_text;

    int studentId;

    Student student;

    GtkWidget *dialog;
    char message[300];

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    studentId = atoi(id_text);

    if (search_student(
            studentId,
            &student
        ) != 1)
    {
        show_message(
            window,
            "Student Not Found",
            "No student found with this ID."
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Are you sure you want to delete student %d?\n\nName: %s",
        student.studentId,
        student.name
    );

    dialog = gtk_message_dialog_new(
        window,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "%s",
        message
    );

    gtk_window_set_title(
        GTK_WINDOW(dialog),
        "Confirm Delete"
    );

    g_object_set_data(
        G_OBJECT(dialog),
        "parent-window",
        window
    );

    g_object_set_data(
        G_OBJECT(dialog),
        "delete-button",
        button
    );

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(delete_confirmation_response),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(dialog)
    );
}


static void show_delete_student_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *delete_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Delete Student"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(
        box,
        25
    );

    gtk_widget_set_margin_bottom(
        box,
        25
    );

    gtk_widget_set_margin_start(
        box,
        25
    );

    gtk_widget_set_margin_end(
        box,
        25
    );

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Delete Student"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Student ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    delete_button = create_button(
        "Delete Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_object_set_data(
        G_OBJECT(delete_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(delete_student_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void student_delete_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_delete_student_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   STUDENT MANAGEMENT
   ========================================================= */

static void student_add_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_add_student_window(
        GTK_WINDOW(user_data)
    );
}


void show_student_module(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;

    GtkWidget *add_button;
    GtkWidget *view_button;
    GtkWidget *search_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;

    window = create_module_window(
        parent,
        "Student Management"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    add_button = create_button(
        "Add Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        add_button
    );

    view_button = create_button(
        "View Students"
    );

    gtk_box_append(
        GTK_BOX(box),
        view_button
    );

    search_button = create_button(
        "Search Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    update_button = create_button(
        "Update Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    delete_button = create_button(
        "Delete Student"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(student_add_button_clicked),
        window
    );

    g_signal_connect(
        view_button,
        "clicked",
        G_CALLBACK(student_view_button_clicked),
        window
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(student_search_button_clicked),
        window
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(student_update_button_clicked),
        window
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(student_delete_button_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   STUDENT LOGIN
   ========================================================= */

static void student_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *password_entry;
    GtkWindow *window;

    const char *id_text;
    const char *password;

    int studentId;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    password_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "password-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    password = gtk_editable_get_text(
        GTK_EDITABLE(password_entry)
    );

    studentId = atoi(id_text);

    if (login_student(
            studentId,
            password
        ))
    {
        current_student_id = studentId;

        gtk_window_destroy(window);

        show_student_dashboard(NULL);
    }
    else
    {
        show_message(
            window,
            "Login Failed",
            "Invalid student ID or password."
        );
    }
}


void show_student_login(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;

    window = create_module_window(
        parent,
        "Student Login"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Student Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Student ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    password_entry = create_entry(
        "Password",
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        password_entry
    );

    login_button = create_button(
        "Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        login_button
    );

    g_object_set_data(
        G_OBJECT(login_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(login_button),
        "password-entry",
        password_entry
    );

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(student_login_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   STUDENT DASHBOARD
   ========================================================= */

static void student_profile_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_profile(
        GTK_WINDOW(user_data)
    );
}


static void student_buses_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_buses(
        GTK_WINDOW(user_data)
    );
}


static void student_routes_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_routes(
        GTK_WINDOW(user_data)
    );
}


static void student_schedules_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_schedules(
        GTK_WINDOW(user_data)
    );
}


static void student_booking_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_booking(
        GTK_WINDOW(user_data)
    );
}


static void student_notifications_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_student_notifications(
        GTK_WINDOW(user_data)
    );
}


void show_student_dashboard(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *profile_button;
    GtkWidget *buses_button;
    GtkWidget *routes_button;
    GtkWidget *schedules_button;
    GtkWidget *booking_button;
    GtkWidget *notification_button;

    window = create_module_window(
        parent,
        "Student Dashboard"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Student Dashboard"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    profile_button = create_button(
        "My Profile"
    );

    gtk_box_append(
        GTK_BOX(box),
        profile_button
    );

    buses_button = create_button(
        "View Buses"
    );

    gtk_box_append(
        GTK_BOX(box),
        buses_button
    );

    routes_button = create_button(
        "View Routes"
    );

    gtk_box_append(
        GTK_BOX(box),
        routes_button
    );

    schedules_button = create_button(
        "View Schedules"
    );

    gtk_box_append(
        GTK_BOX(box),
        schedules_button
    );

    booking_button = create_button(
        "Book Seat"
    );

    gtk_box_append(
        GTK_BOX(box),
        booking_button
    );

    notification_button = create_button(
        "Notifications"
    );

    gtk_box_append(
        GTK_BOX(box),
        notification_button
    );

    g_signal_connect(
        profile_button,
        "clicked",
        G_CALLBACK(student_profile_clicked),
        window
    );

    g_signal_connect(
        buses_button,
        "clicked",
        G_CALLBACK(student_buses_clicked),
        window
    );

    g_signal_connect(
        routes_button,
        "clicked",
        G_CALLBACK(student_routes_clicked),
        window
    );

    g_signal_connect(
        schedules_button,
        "clicked",
        G_CALLBACK(student_schedules_clicked),
        window
    );

    g_signal_connect(
        booking_button,
        "clicked",
        G_CALLBACK(student_booking_clicked),
        window
    );

    g_signal_connect(
        notification_button,
        "clicked",
        G_CALLBACK(student_notifications_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   DRIVER LOGIN
   ========================================================= */

static void driver_login_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int driverId;
    Driver driver;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    driverId = atoi(id_text);

    if (search_driver(
            driverId,
            &driver
        ) == 1)
    {
        logged_driver_id = driverId;

        gtk_window_destroy(window);

        show_driver_dashboard(NULL);
    }
    else
    {
        show_message(
            window,
            "Login Failed",
            "Invalid driver ID."
        );
    }
}


void show_driver_login(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *login_button;

    window = create_module_window(
        parent,
        "Driver Login"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Driver Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Driver ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    login_button = create_button(
        "Login"
    );

    gtk_box_append(
        GTK_BOX(box),
        login_button
    );

    g_object_set_data(
        G_OBJECT(login_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(driver_login_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   DRIVER DASHBOARD
   ========================================================= */

static void driver_profile_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_profile(
        GTK_WINDOW(user_data)
    );
}


static void driver_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_bus(
        GTK_WINDOW(user_data)
    );
}


static void driver_route_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_route(
        GTK_WINDOW(user_data)
    );
}


static void driver_schedule_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_schedule(
        GTK_WINDOW(user_data)
    );
}


static void driver_passengers_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_driver_passengers(
        GTK_WINDOW(user_data)
    );
}


void show_driver_dashboard(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *profile_button;
    GtkWidget *bus_button;
    GtkWidget *route_button;
    GtkWidget *schedule_button;
    GtkWidget *passengers_button;

    window = create_module_window(
        parent,
        "Driver Dashboard"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Driver Dashboard"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    profile_button = create_button(
        "My Profile"
    );

    gtk_box_append(
        GTK_BOX(box),
        profile_button
    );

    bus_button = create_button(
        "Assigned Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        bus_button
    );

    route_button = create_button(
        "Assigned Route"
    );

    gtk_box_append(
        GTK_BOX(box),
        route_button
    );

    schedule_button = create_button(
        "My Schedule"
    );

    gtk_box_append(
        GTK_BOX(box),
        schedule_button
    );

    passengers_button = create_button(
        "Passenger Information"
    );

    gtk_box_append(
        GTK_BOX(box),
        passengers_button
    );

    g_signal_connect(
        profile_button,
        "clicked",
        G_CALLBACK(driver_profile_clicked),
        window
    );

    g_signal_connect(
        bus_button,
        "clicked",
        G_CALLBACK(driver_bus_clicked),
        window
    );

    g_signal_connect(
        route_button,
        "clicked",
        G_CALLBACK(driver_route_clicked),
        window
    );

    g_signal_connect(
        schedule_button,
        "clicked",
        G_CALLBACK(driver_schedule_clicked),
        window
    );

    g_signal_connect(
        passengers_button,
        "clicked",
        G_CALLBACK(driver_passengers_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   OTHER ADMIN MODULES
   ========================================================= */

/* =========================================================
   DRIVER MANAGEMENT - ADD
   ========================================================= */

static void save_driver_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *phone_entry;
    GtkWidget *bus_entry;

    GtkWindow *window;

    Driver driver;

    const char *id_text;
    const char *name;
    const char *phone;
    const char *bus_text;

    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    phone_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "phone-entry"
        )
    );

    bus_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "bus-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    name = gtk_editable_get_text(
        GTK_EDITABLE(name_entry)
    );

    phone = gtk_editable_get_text(
        GTK_EDITABLE(phone_entry)
    );

    bus_text = gtk_editable_get_text(
        GTK_EDITABLE(bus_entry)
    );

    driver.driverId = atoi(id_text);
    driver.assignedBusId = atoi(bus_text);

    strncpy(
        driver.name,
        name,
        sizeof(driver.name) - 1
    );

    driver.name[
        sizeof(driver.name) - 1
    ] = '\0';

    strncpy(
        driver.phone,
        phone,
        sizeof(driver.phone) - 1
    );

    driver.phone[
        sizeof(driver.phone) - 1
    ] = '\0';

    result = add_driver(driver);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Driver added successfully."
        );

        gtk_window_destroy(window);
    }
    else if (result == -2)
    {
        show_message(
            window,
            "Error",
            "Driver ID already exists."
        );
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not add driver."
        );
    }
}


static void show_add_driver_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *phone_entry;
    GtkWidget *bus_entry;
    GtkWidget *save_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Add Driver"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        450,
        450
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Add New Driver"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Driver ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    name_entry = create_entry(
        "Driver Name",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        name_entry
    );

    phone_entry = create_entry(
        "Phone Number",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        phone_entry
    );

    bus_entry = create_entry(
        "Assigned Bus ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        bus_entry
    );

    save_button = create_button(
        "Save Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        save_button
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "phone-entry",
        phone_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "bus-entry",
        bus_entry
    );

    g_signal_connect(
        save_button,
        "clicked",
        G_CALLBACK(save_driver_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   DRIVER MANAGEMENT - VIEW
   ========================================================= */

static void show_all_drivers(
    GtkWindow *parent)
{
    void *records;
    size_t count;
    size_t i;

    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *scroll;
    GtkWidget *list_box;

    Driver *drivers;

    records = NULL;
    count = 0;

    if (readRecords(
            DRIVERS_FILE,
            sizeof(Driver),
            &records,
            &count
        ) != 0)
    {
        show_message(
            parent,
            "Error",
            "Could not read driver data."
        );

        return;
    }

    if (count == 0)
    {
        free(records);

        show_message(
            parent,
            "Drivers",
            "No drivers found."
        );

        return;
    }

    drivers = (Driver *)records;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "View Drivers"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        550,
        500
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Registered Drivers"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    for (i = 0; i < count; i++)
    {
        GtkWidget *driver_box;
        GtkWidget *driver_label;

        char text[400];

        driver_box = gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            4
        );

        driver_label = gtk_label_new(NULL);

        snprintf(
            text,
            sizeof(text),
            "Driver ID: %d\nName: %s\nPhone: %s\nAssigned Bus ID: %d",
            drivers[i].driverId,
            drivers[i].name,
            drivers[i].phone,
            drivers[i].assignedBusId
        );

        gtk_label_set_text(
            GTK_LABEL(driver_label),
            text
        );

        gtk_label_set_xalign(
            GTK_LABEL(driver_label),
            0.0
        );

        gtk_box_append(
            GTK_BOX(driver_box),
            driver_label
        );

        gtk_box_append(
            GTK_BOX(list_box),
            driver_box
        );
    }

    free(records);

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void driver_view_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_all_drivers(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   DRIVER MANAGEMENT - SEARCH
   ========================================================= */

static void search_driver_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int driverId;
    Driver driver;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    driverId = atoi(id_text);

    if (search_driver(
            driverId,
            &driver
        ) == 1)
    {
        char message[400];

        snprintf(
            message,
            sizeof(message),
            "Driver ID: %d\nName: %s\nPhone: %s\nAssigned Bus ID: %d",
            driver.driverId,
            driver.name,
            driver.phone,
            driver.assignedBusId
        );

        show_message(
            window,
            "Driver Found",
            message
        );
    }
    else
    {
        show_message(
            window,
            "Driver Not Found",
            "No driver found with this ID."
        );
    }
}


static void show_search_driver_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *search_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Search Driver"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Search Driver"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Driver ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    search_button = create_button(
        "Search Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    g_object_set_data(
        G_OBJECT(search_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(search_driver_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void driver_search_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_search_driver_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   DRIVER MANAGEMENT - UPDATE
   ========================================================= */

static void update_driver_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *phone_entry;
    GtkWidget *bus_entry;

    GtkWindow *window;

    Driver driver;

    const char *id_text;
    const char *name;
    const char *phone;
    const char *bus_text;

    int driverId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    phone_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "phone-entry"
        )
    );

    bus_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "bus-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    name = gtk_editable_get_text(
        GTK_EDITABLE(name_entry)
    );

    phone = gtk_editable_get_text(
        GTK_EDITABLE(phone_entry)
    );

    bus_text = gtk_editable_get_text(
        GTK_EDITABLE(bus_entry)
    );

    driverId = atoi(id_text);

    if (search_driver(
            driverId,
            &driver
        ) != 1)
    {
        show_message(
            window,
            "Error",
            "Driver not found."
        );

        return;
    }

    strncpy(
        driver.name,
        name,
        sizeof(driver.name) - 1
    );

    driver.name[
        sizeof(driver.name) - 1
    ] = '\0';

    strncpy(
        driver.phone,
        phone,
        sizeof(driver.phone) - 1
    );

    driver.phone[
        sizeof(driver.phone) - 1
    ] = '\0';

    driver.assignedBusId = atoi(bus_text);

    result = update_driver(driver);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Driver updated successfully."
        );

        gtk_window_destroy(window);
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not update driver."
        );
    }
}


static void load_driver_for_update(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *phone_entry;
    GtkWidget *bus_entry;
    GtkWidget *update_button;

    GtkWindow *window;

    Driver driver;

    const char *id_text;

    int driverId;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    name_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "name-entry"
        )
    );

    phone_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "phone-entry"
        )
    );

    bus_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "bus-entry"
        )
    );

    update_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "update-button"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    driverId = atoi(id_text);

    if (search_driver(
            driverId,
            &driver
        ) == 1)
    {
        char bus_text[30];

        gtk_editable_set_text(
            GTK_EDITABLE(name_entry),
            driver.name
        );

        gtk_editable_set_text(
            GTK_EDITABLE(phone_entry),
            driver.phone
        );

        snprintf(
            bus_text,
            sizeof(bus_text),
            "%d",
            driver.assignedBusId
        );

        gtk_editable_set_text(
            GTK_EDITABLE(bus_entry),
            bus_text
        );

        gtk_widget_set_sensitive(
            name_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            phone_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            bus_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            update_button,
            TRUE
        );

        gtk_widget_set_sensitive(
            GTK_WIDGET(button),
            FALSE
        );
    }
    else
    {
        show_message(
            window,
            "Driver Not Found",
            "No driver found with this ID."
        );
    }
}


static void show_update_driver_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *phone_entry;
    GtkWidget *bus_entry;

    GtkWidget *load_button;
    GtkWidget *update_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Update Driver"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        450,
        500
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Update Driver"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Driver ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    load_button = create_button(
        "Load Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        load_button
    );

    name_entry = create_entry(
        "Driver Name",
        FALSE
    );

    gtk_widget_set_sensitive(
        name_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        name_entry
    );

    phone_entry = create_entry(
        "Phone Number",
        FALSE
    );

    gtk_widget_set_sensitive(
        phone_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        phone_entry
    );

    bus_entry = create_entry(
        "Assigned Bus ID",
        FALSE
    );

    gtk_widget_set_sensitive(
        bus_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        bus_entry
    );

    update_button = create_button(
        "Update Driver"
    );

    gtk_widget_set_sensitive(
        update_button,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "phone-entry",
        phone_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "bus-entry",
        bus_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "update-button",
        update_button
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "name-entry",
        name_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "phone-entry",
        phone_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "bus-entry",
        bus_entry
    );

    g_signal_connect(
        load_button,
        "clicked",
        G_CALLBACK(load_driver_for_update),
        NULL
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(update_driver_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void driver_update_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_update_driver_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   DRIVER MANAGEMENT - DELETE
   ========================================================= */

static void confirm_delete_driver(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int driverId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    driverId = atoi(id_text);

    result = delete_driver(driverId);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Driver deleted successfully."
        );

        gtk_window_destroy(window);
    }
    else if (result == 0)
    {
        show_message(
            window,
            "Driver Not Found",
            "No driver found with this ID."
        );
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not delete driver."
        );
    }
}


static void driver_delete_confirmation_response(
    GtkDialog *dialog,
    int response_id,
    gpointer user_data)
{
    GtkWidget *delete_button;

    delete_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(dialog),
            "delete-button"
        )
    );

    if (response_id == GTK_RESPONSE_YES)
    {
        confirm_delete_driver(
            GTK_BUTTON(delete_button),
            NULL
        );
    }

    gtk_window_destroy(
        GTK_WINDOW(dialog)
    );

    (void)user_data;
}


static void delete_driver_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int driverId;
    Driver driver;

    GtkWidget *dialog;
    char message[400];

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    driverId = atoi(id_text);

    if (search_driver(
            driverId,
            &driver
        ) != 1)
    {
        show_message(
            window,
            "Driver Not Found",
            "No driver found with this ID."
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Are you sure you want to delete driver %d?\n\nName: %s",
        driver.driverId,
        driver.name
    );

    dialog = gtk_message_dialog_new(
        window,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "%s",
        message
    );

    gtk_window_set_title(
        GTK_WINDOW(dialog),
        "Confirm Delete"
    );

    g_object_set_data(
        G_OBJECT(dialog),
        "delete-button",
        button
    );

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(driver_delete_confirmation_response),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(dialog)
    );
}


static void show_delete_driver_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *delete_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Delete Driver"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Delete Driver"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Driver ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    delete_button = create_button(
        "Delete Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_object_set_data(
        G_OBJECT(delete_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(delete_driver_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void driver_delete_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_delete_driver_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   DRIVER MANAGEMENT
   ========================================================= */

static void driver_add_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_add_driver_window(
        GTK_WINDOW(user_data)
    );
}


void show_driver_module(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;

    GtkWidget *add_button;
    GtkWidget *view_button;
    GtkWidget *search_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;

    window = create_module_window(
        parent,
        "Driver Management"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    add_button = create_button(
        "Add Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        add_button
    );

    view_button = create_button(
        "View Drivers"
    );

    gtk_box_append(
        GTK_BOX(box),
        view_button
    );

    search_button = create_button(
        "Search Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    update_button = create_button(
        "Update Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    delete_button = create_button(
        "Delete Driver"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(driver_add_button_clicked),
        window
    );

    g_signal_connect(
        view_button,
        "clicked",
        G_CALLBACK(driver_view_button_clicked),
        window
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(driver_search_button_clicked),
        window
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(driver_update_button_clicked),
        window
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(driver_delete_button_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   BUS MANAGEMENT - ADD
   ========================================================= */

static void save_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *available_entry;

    GtkWindow *window;

    Bus bus;

    const char *id_text;
    const char *number;
    const char *capacity_text;
    const char *available_text;

    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    number_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "number-entry"
        )
    );

    capacity_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "capacity-entry"
        )
    );

    available_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "available-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    number = gtk_editable_get_text(
        GTK_EDITABLE(number_entry)
    );

    capacity_text = gtk_editable_get_text(
        GTK_EDITABLE(capacity_entry)
    );

    available_text = gtk_editable_get_text(
        GTK_EDITABLE(available_entry)
    );

    bus.busId = atoi(id_text);

    strncpy(
        bus.busNumber,
        number,
        sizeof(bus.busNumber) - 1
    );

    bus.busNumber[
        sizeof(bus.busNumber) - 1
    ] = '\0';

    bus.capacity = atoi(capacity_text);
    bus.availableSeats = atoi(available_text);

    result = add_bus(bus);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Bus added successfully."
        );

        gtk_window_destroy(window);
    }
    else if (result == -2)
    {
        show_message(
            window,
            "Error",
            "Bus ID already exists."
        );
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not add bus. Check the entered values."
        );
    }
}


static void show_add_bus_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *available_entry;
    GtkWidget *save_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Add Bus"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        450,
        450
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Add New Bus"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Bus ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    number_entry = create_entry(
        "Bus Number",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        number_entry
    );

    capacity_entry = create_entry(
        "Capacity",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        capacity_entry
    );

    available_entry = create_entry(
        "Available Seats",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        available_entry
    );

    save_button = create_button(
        "Save Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        save_button
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "number-entry",
        number_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "capacity-entry",
        capacity_entry
    );

    g_object_set_data(
        G_OBJECT(save_button),
        "available-entry",
        available_entry
    );

    g_signal_connect(
        save_button,
        "clicked",
        G_CALLBACK(save_bus_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   BUS MANAGEMENT - VIEW
   ========================================================= */

static void show_all_buses(
    GtkWindow *parent)
{
    void *records;
    size_t count;
    size_t i;

    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *scroll;
    GtkWidget *list_box;

    Bus *buses;

    records = NULL;
    count = 0;

    if (readRecords(
            BUSES_FILE,
            sizeof(Bus),
            &records,
            &count
        ) != 0)
    {
        show_message(
            parent,
            "Error",
            "Could not read bus data."
        );

        return;
    }

    if (count == 0)
    {
        free(records);

        show_message(
            parent,
            "Buses",
            "No buses found."
        );

        return;
    }

    buses = (Bus *)records;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "View Buses"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        550,
        500
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Registered Buses"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    for (i = 0; i < count; i++)
    {
        GtkWidget *bus_box;
        GtkWidget *bus_label;

        char text[300];

        bus_box = gtk_box_new(
            GTK_ORIENTATION_VERTICAL,
            4
        );

        bus_label = gtk_label_new(NULL);

        snprintf(
            text,
            sizeof(text),
            "Bus ID: %d\nBus Number: %s\nCapacity: %d\nAvailable Seats: %d",
            buses[i].busId,
            buses[i].busNumber,
            buses[i].capacity,
            buses[i].availableSeats
        );

        gtk_label_set_text(
            GTK_LABEL(bus_label),
            text
        );

        gtk_label_set_xalign(
            GTK_LABEL(bus_label),
            0.0
        );

        gtk_box_append(
            GTK_BOX(bus_box),
            bus_label
        );

        gtk_box_append(
            GTK_BOX(list_box),
            bus_box
        );
    }

    free(records);

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void bus_view_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_all_buses(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   BUS MANAGEMENT - SEARCH
   ========================================================= */

static void search_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int busId;

    Bus bus;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    busId = atoi(id_text);

    if (search_bus(
            busId,
            &bus
        ) == 1)
    {
        char message[350];

        snprintf(
            message,
            sizeof(message),
            "Bus ID: %d\nBus Number: %s\nCapacity: %d\nAvailable Seats: %d",
            bus.busId,
            bus.busNumber,
            bus.capacity,
            bus.availableSeats
        );

        show_message(
            window,
            "Bus Found",
            message
        );
    }
    else
    {
        show_message(
            window,
            "Bus Not Found",
            "No bus found with this ID."
        );
    }
}


static void show_search_bus_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *search_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Search Bus"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Search Bus"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Bus ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    search_button = create_button(
        "Search Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    g_object_set_data(
        G_OBJECT(search_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(search_bus_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void bus_search_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_search_bus_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   BUS MANAGEMENT - UPDATE
   ========================================================= */

static void update_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *available_entry;

    GtkWindow *window;

    Bus bus;

    const char *id_text;
    const char *number;
    const char *capacity_text;
    const char *available_text;

    int busId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    number_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "number-entry"
        )
    );

    capacity_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "capacity-entry"
        )
    );

    available_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "available-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    number = gtk_editable_get_text(
        GTK_EDITABLE(number_entry)
    );

    capacity_text = gtk_editable_get_text(
        GTK_EDITABLE(capacity_entry)
    );

    available_text = gtk_editable_get_text(
        GTK_EDITABLE(available_entry)
    );

    busId = atoi(id_text);

    if (search_bus(
            busId,
            &bus
        ) != 1)
    {
        show_message(
            window,
            "Error",
            "Bus not found."
        );

        return;
    }

    strncpy(
        bus.busNumber,
        number,
        sizeof(bus.busNumber) - 1
    );

    bus.busNumber[
        sizeof(bus.busNumber) - 1
    ] = '\0';

    bus.capacity = atoi(capacity_text);
    bus.availableSeats = atoi(available_text);

    result = update_bus(bus);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Bus updated successfully."
        );

        gtk_window_destroy(window);
    }
    else
    {
        show_message(
            window,
            "Error",
            "Could not update bus. Check the entered values."
        );
    }
}


static void load_bus_for_update(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *available_entry;
    GtkWidget *update_button;

    GtkWindow *window;

    Bus bus;

    const char *id_text;

    int busId;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    number_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "number-entry"
        )
    );

    capacity_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "capacity-entry"
        )
    );

    available_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "available-entry"
        )
    );

    update_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "update-button"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    busId = atoi(id_text);

    if (search_bus(
            busId,
            &bus
        ) == 1)
    {
        gtk_editable_set_text(
            GTK_EDITABLE(number_entry),
            bus.busNumber
        );

        {
            char text[30];

            snprintf(
                text,
                sizeof(text),
                "%d",
                bus.capacity
            );

            gtk_editable_set_text(
                GTK_EDITABLE(capacity_entry),
                text
            );
        }

        {
            char text[30];

            snprintf(
                text,
                sizeof(text),
                "%d",
                bus.availableSeats
            );

            gtk_editable_set_text(
                GTK_EDITABLE(available_entry),
                text
            );
        }

        gtk_widget_set_sensitive(
            number_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            capacity_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            available_entry,
            TRUE
        );

        gtk_widget_set_sensitive(
            update_button,
            TRUE
        );

        gtk_widget_set_sensitive(
            GTK_WIDGET(button),
            FALSE
        );
    }
    else
    {
        show_message(
            window,
            "Bus Not Found",
            "No bus found with this ID."
        );
    }
}


static void show_update_bus_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *number_entry;
    GtkWidget *capacity_entry;
    GtkWidget *available_entry;

    GtkWidget *load_button;
    GtkWidget *update_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Update Bus"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        450,
        500
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Update Bus"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Bus ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    load_button = create_button(
        "Load Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        load_button
    );

    number_entry = create_entry(
        "Bus Number",
        FALSE
    );

    gtk_widget_set_sensitive(
        number_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        number_entry
    );

    capacity_entry = create_entry(
        "Capacity",
        FALSE
    );

    gtk_widget_set_sensitive(
        capacity_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        capacity_entry
    );

    available_entry = create_entry(
        "Available Seats",
        FALSE
    );

    gtk_widget_set_sensitive(
        available_entry,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        available_entry
    );

    update_button = create_button(
        "Update Bus"
    );

    gtk_widget_set_sensitive(
        update_button,
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "number-entry",
        number_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "capacity-entry",
        capacity_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "available-entry",
        available_entry
    );

    g_object_set_data(
        G_OBJECT(load_button),
        "update-button",
        update_button
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "id-entry",
        id_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "number-entry",
        number_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "capacity-entry",
        capacity_entry
    );

    g_object_set_data(
        G_OBJECT(update_button),
        "available-entry",
        available_entry
    );

    g_signal_connect(
        load_button,
        "clicked",
        G_CALLBACK(load_bus_for_update),
        NULL
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(update_bus_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void bus_update_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_update_bus_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   BUS MANAGEMENT - DELETE
   ========================================================= */

static void confirm_delete_bus(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;
    GtkWindow *window;

    const char *id_text;

    int busId;
    int result;

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    busId = atoi(id_text);

    result = delete_bus(busId);

    if (result == 1)
    {
        show_message(
            window,
            "Success",
            "Bus deleted successfully."
        );

        gtk_window_destroy(window);
    }
    else
    {
        show_message(
            window,
            "Error",
            "Bus not found or could not be deleted."
        );
    }
}


static void bus_delete_confirmation_response(
    GtkDialog *dialog,
    int response_id,
    gpointer user_data)
{
    GtkWidget *delete_button;

    delete_button = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(dialog),
            "delete-button"
        )
    );

    if (response_id == GTK_RESPONSE_YES)
    {
        confirm_delete_bus(
            GTK_BUTTON(delete_button),
            NULL
        );
    }

    gtk_window_destroy(
        GTK_WINDOW(dialog)
    );

    (void)user_data;
}


static void delete_bus_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWidget *id_entry;

    GtkWindow *window;

    const char *id_text;

    int busId;

    Bus bus;

    GtkWidget *dialog;
    char message[350];

    id_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(button),
            "id-entry"
        )
    );

    window = GTK_WINDOW(
        gtk_widget_get_root(
            GTK_WIDGET(button)
        )
    );

    id_text = gtk_editable_get_text(
        GTK_EDITABLE(id_entry)
    );

    busId = atoi(id_text);

    if (search_bus(
            busId,
            &bus
        ) != 1)
    {
        show_message(
            window,
            "Bus Not Found",
            "No bus found with this ID."
        );

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Are you sure you want to delete bus %d?\n\nBus Number: %s",
        bus.busId,
        bus.busNumber
    );

    dialog = gtk_message_dialog_new(
        window,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "%s",
        message
    );

    gtk_window_set_title(
        GTK_WINDOW(dialog),
        "Confirm Delete"
    );

    g_object_set_data(
        G_OBJECT(dialog),
        "delete-button",
        button
    );

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(bus_delete_confirmation_response),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(dialog)
    );
}


static void show_delete_bus_window(
    GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;

    GtkWidget *id_entry;
    GtkWidget *delete_button;

    window = gtk_window_new();

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Delete Bus"
    );

    gtk_window_set_default_size(
        GTK_WINDOW(window),
        400,
        300
    );

    gtk_window_set_transient_for(
        GTK_WINDOW(window),
        parent
    );

    gtk_window_set_modal(
        GTK_WINDOW(window),
        TRUE
    );

    box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        12
    );

    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);

    gtk_window_set_child(
        GTK_WINDOW(window),
        box
    );

    title = gtk_label_new(
        "Delete Bus"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    id_entry = create_entry(
        "Enter Bus ID",
        FALSE
    );

    gtk_box_append(
        GTK_BOX(box),
        id_entry
    );

    delete_button = create_button(
        "Delete Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_object_set_data(
        G_OBJECT(delete_button),
        "id-entry",
        id_entry
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(delete_bus_clicked),
        NULL
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


static void bus_delete_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_delete_bus_window(
        GTK_WINDOW(user_data)
    );
}


/* =========================================================
   BUS MANAGEMENT
   ========================================================= */

static void bus_add_button_clicked(
    GtkButton *button,
    gpointer user_data)
{
    show_add_bus_window(
        GTK_WINDOW(user_data)
    );
}


void show_bus_module(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;

    GtkWidget *add_button;
    GtkWidget *view_button;
    GtkWidget *search_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;

    window = create_module_window(
        parent,
        "Bus Management"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    add_button = create_button(
        "Add Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        add_button
    );

    view_button = create_button(
        "View Buses"
    );

    gtk_box_append(
        GTK_BOX(box),
        view_button
    );

    search_button = create_button(
        "Search Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        search_button
    );

    update_button = create_button(
        "Update Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        update_button
    );

    delete_button = create_button(
        "Delete Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        delete_button
    );

    g_signal_connect(
        add_button,
        "clicked",
        G_CALLBACK(bus_add_button_clicked),
        window
    );

    g_signal_connect(
        view_button,
        "clicked",
        G_CALLBACK(bus_view_button_clicked),
        window
    );

    g_signal_connect(
        search_button,
        "clicked",
        G_CALLBACK(bus_search_button_clicked),
        window
    );

    g_signal_connect(
        update_button,
        "clicked",
        G_CALLBACK(bus_update_button_clicked),
        window
    );

    g_signal_connect(
        delete_button,
        "clicked",
        G_CALLBACK(bus_delete_button_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   ROUTE MANAGEMENT
   ========================================================= */

static void route_save_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "id-entry"));
    GtkWidget *name_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "name-entry"));
    GtkWidget *stops_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "stops-entry"));
    GtkWidget *start_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "start-entry"));
    GtkWidget *destination_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "destination-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Route route;
    int result;

    memset(&route, 0, sizeof(Route));
    route.routeId = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));
    strncpy(route.routeName, gtk_editable_get_text(GTK_EDITABLE(name_entry)), sizeof(route.routeName) - 1);
    strncpy(route.stops, gtk_editable_get_text(GTK_EDITABLE(stops_entry)), sizeof(route.stops) - 1);
    strncpy(route.starting_point, gtk_editable_get_text(GTK_EDITABLE(start_entry)), sizeof(route.starting_point) - 1);
    strncpy(route.destination, gtk_editable_get_text(GTK_EDITABLE(destination_entry)), sizeof(route.destination) - 1);

    result = add_route(route);

    if (result == 1) {
        show_message(window, "Success", "Route added successfully.");
        gtk_window_destroy(window);
    } else if (result == -2) {
        show_message(window, "Error", "Route ID already exists.");
    } else {
        show_message(window, "Error", "Could not add route. Check the entered values.");
    }
}

static void route_add_button_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *id_entry;
    GtkWidget *name_entry;
    GtkWidget *stops_entry;
    GtkWidget *start_entry;
    GtkWidget *destination_entry;
    GtkWidget *save_button;

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Add Route");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 550);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(user_data));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    title = gtk_label_new("Add Route");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), title);

    id_entry = create_entry("Route ID", FALSE);
    name_entry = create_entry("Route Name", FALSE);
    stops_entry = create_entry("Stops (comma separated)", FALSE);
    start_entry = create_entry("Starting Point", FALSE);
    destination_entry = create_entry("Destination", FALSE);

    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), name_entry);
    gtk_box_append(GTK_BOX(box), stops_entry);
    gtk_box_append(GTK_BOX(box), start_entry);
    gtk_box_append(GTK_BOX(box), destination_entry);

    save_button = create_button("Add Route");
    gtk_box_append(GTK_BOX(box), save_button);

    g_object_set_data(G_OBJECT(save_button), "id-entry", id_entry);
    g_object_set_data(G_OBJECT(save_button), "name-entry", name_entry);
    g_object_set_data(G_OBJECT(save_button), "stops-entry", stops_entry);
    g_object_set_data(G_OBJECT(save_button), "start-entry", start_entry);
    g_object_set_data(G_OBJECT(save_button), "destination-entry", destination_entry);

    g_signal_connect(save_button, "clicked", G_CALLBACK(route_save_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}

static void show_all_routes(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *scroll;
    GtkWidget *box;
    void *records = NULL;
    Route *routes;
    size_t count = 0;
    size_t i;

    if (readRecords(ROUTES_FILE, sizeof(Route), &records, &count) != 0) {
        show_message(parent, "Error", "Could not read route data.");
        return;
    }

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "All Routes");
    gtk_window_set_default_size(GTK_WINDOW(window), 650, 500);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);

    scroll = gtk_scrolled_window_new();
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);
    gtk_window_set_child(GTK_WINDOW(window), scroll);

    routes = (Route *)records;

    if (count == 0) {
        gtk_box_append(GTK_BOX(box), gtk_label_new("No routes found."));
    } else {
        for (i = 0; i < count; i++) {
            char text[400];
            snprintf(text, sizeof(text),
                "Route ID: %d\nName: %s\nStops: %s\nFrom: %s\nTo: %s",
                routes[i].routeId,
                routes[i].routeName,
                routes[i].stops,
                routes[i].starting_point,
                routes[i].destination);
            gtk_box_append(GTK_BOX(box), gtk_label_new(text));
            gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
        }
    }

    free(records);
    gtk_window_present(GTK_WINDOW(window));
}

static void route_view_button_clicked(GtkButton *button, gpointer user_data)
{
    show_all_routes(GTK_WINDOW(user_data));
}

static void route_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "id-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Route route;
    int id = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));
    char message[500];

    if (search_route(id, &route) == 1) {
        snprintf(message, sizeof(message),
            "Route ID: %d\nName: %s\nStops: %s\nFrom: %s\nTo: %s",
            route.routeId, route.routeName, route.stops,
            route.starting_point, route.destination);
        show_message(window, "Route Found", message);
    } else {
        show_message(window, "Not Found", "No route found with this ID.");
    }
}

static void show_search_route_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *id_entry = create_entry("Enter Route ID", FALSE);
    GtkWidget *search_button = create_button("Search Route");

    gtk_window_set_title(GTK_WINDOW(window), "Search Route");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Search Route"));
    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), search_button);
    g_object_set_data(G_OBJECT(search_button), "id-entry", id_entry);
    g_signal_connect(search_button, "clicked", G_CALLBACK(route_search_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void route_search_button_clicked(GtkButton *button, gpointer user_data)
{
    show_search_route_window(GTK_WINDOW(user_data));
}

static void route_load_for_update(GtkButton *button, gpointer user_data)
{
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "id-entry"));
    GtkWidget *name_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "name-entry"));
    GtkWidget *stops_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "stops-entry"));
    GtkWidget *start_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "start-entry"));
    GtkWidget *destination_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "destination-entry"));
    GtkWidget *update_button = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "update-button"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Route route;
    int id = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));

    if (search_route(id, &route) == 1) {
        gtk_editable_set_text(GTK_EDITABLE(name_entry), route.routeName);
        gtk_editable_set_text(GTK_EDITABLE(stops_entry), route.stops);
        gtk_editable_set_text(GTK_EDITABLE(start_entry), route.starting_point);
        gtk_editable_set_text(GTK_EDITABLE(destination_entry), route.destination);
        gtk_widget_set_sensitive(name_entry, TRUE);
        gtk_widget_set_sensitive(stops_entry, TRUE);
        gtk_widget_set_sensitive(start_entry, TRUE);
        gtk_widget_set_sensitive(destination_entry, TRUE);
        gtk_widget_set_sensitive(update_button, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    } else {
        show_message(window, "Not Found", "No route found with this ID.");
    }
}

static void route_update_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "id-entry"));
    GtkWidget *name_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "name-entry"));
    GtkWidget *stops_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "stops-entry"));
    GtkWidget *start_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "start-entry"));
    GtkWidget *destination_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "destination-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Route route;
    int id = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));

    if (search_route(id, &route) != 1) {
        show_message(window, "Error", "Route not found.");
        return;
    }

    strncpy(route.routeName, gtk_editable_get_text(GTK_EDITABLE(name_entry)), sizeof(route.routeName) - 1);
    route.routeName[sizeof(route.routeName) - 1] = '\0';
    strncpy(route.stops, gtk_editable_get_text(GTK_EDITABLE(stops_entry)), sizeof(route.stops) - 1);
    route.stops[sizeof(route.stops) - 1] = '\0';
    strncpy(route.starting_point, gtk_editable_get_text(GTK_EDITABLE(start_entry)), sizeof(route.starting_point) - 1);
    route.starting_point[sizeof(route.starting_point) - 1] = '\0';
    strncpy(route.destination, gtk_editable_get_text(GTK_EDITABLE(destination_entry)), sizeof(route.destination) - 1);
    route.destination[sizeof(route.destination) - 1] = '\0';

    if (update_route(route) == 1) {
        show_message(window, "Success", "Route updated successfully.");
        gtk_window_destroy(window);
    } else {
        show_message(window, "Error", "Could not update route. Check the entered values.");
    }
}

static void show_update_route_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *id_entry = create_entry("Route ID", FALSE);
    GtkWidget *load_button = create_button("Load Route");
    GtkWidget *name_entry = create_entry("Route Name", FALSE);
    GtkWidget *stops_entry = create_entry("Stops (comma separated)", FALSE);
    GtkWidget *start_entry = create_entry("Starting Point", FALSE);
    GtkWidget *destination_entry = create_entry("Destination", FALSE);
    GtkWidget *update_button = create_button("Update Route");

    gtk_window_set_title(GTK_WINDOW(window), "Update Route");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 500);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Update Route"));
    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), load_button);
    gtk_box_append(GTK_BOX(box), name_entry);
    gtk_box_append(GTK_BOX(box), stops_entry);
    gtk_box_append(GTK_BOX(box), start_entry);
    gtk_box_append(GTK_BOX(box), destination_entry);
    gtk_box_append(GTK_BOX(box), update_button);

    gtk_widget_set_sensitive(name_entry, FALSE);
    gtk_widget_set_sensitive(stops_entry, FALSE);
    gtk_widget_set_sensitive(start_entry, FALSE);
    gtk_widget_set_sensitive(destination_entry, FALSE);
    gtk_widget_set_sensitive(update_button, FALSE);

    g_object_set_data(G_OBJECT(load_button), "id-entry", id_entry);
    g_object_set_data(G_OBJECT(load_button), "name-entry", name_entry);
    g_object_set_data(G_OBJECT(load_button), "stops-entry", stops_entry);
    g_object_set_data(G_OBJECT(load_button), "start-entry", start_entry);
    g_object_set_data(G_OBJECT(load_button), "destination-entry", destination_entry);
    g_object_set_data(G_OBJECT(load_button), "update-button", update_button);

    g_object_set_data(G_OBJECT(update_button), "id-entry", id_entry);
    g_object_set_data(G_OBJECT(update_button), "name-entry", name_entry);
    g_object_set_data(G_OBJECT(update_button), "stops-entry", stops_entry);
    g_object_set_data(G_OBJECT(update_button), "start-entry", start_entry);
    g_object_set_data(G_OBJECT(update_button), "destination-entry", destination_entry);

    g_signal_connect(load_button, "clicked", G_CALLBACK(route_load_for_update), window);
    g_signal_connect(update_button, "clicked", G_CALLBACK(route_update_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void route_delete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *id_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "id-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    int id = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));
    int result = delete_route(id);

    if (result == 1) {
        show_message(window, "Success", "Route deleted successfully.");
        gtk_window_destroy(window);
    } else if (result == 0) {
        show_message(window, "Not Found", "No route found with this ID.");
    } else {
        show_message(window, "Error", "Could not delete route.");
    }
}

static void show_delete_route_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *id_entry = create_entry("Enter Route ID", FALSE);
    GtkWidget *delete_button = create_button("Delete Route");

    gtk_window_set_title(GTK_WINDOW(window), "Delete Route");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Delete Route"));
    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), delete_button);
    g_object_set_data(G_OBJECT(delete_button), "id-entry", id_entry);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(route_delete_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void route_delete_button_clicked(GtkButton *button, gpointer user_data)
{
    show_delete_route_window(GTK_WINDOW(user_data));
}

static void route_add_button_menu_clicked(GtkButton *button, gpointer user_data)
{
    route_add_button_clicked(button, user_data);
}

static void route_update_button_clicked(GtkButton *button, gpointer user_data)
{
    show_update_route_window(GTK_WINDOW(user_data));
}

void show_route_module(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *add_button;
    GtkWidget *view_button;
    GtkWidget *search_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;

    window = create_module_window(parent, "Route Management");
    box = gtk_window_get_child(GTK_WINDOW(window));

    add_button = create_button("Add Route");
    view_button = create_button("View Routes");
    search_button = create_button("Search Route");
    update_button = create_button("Update Route");
    delete_button = create_button("Delete Route");

    gtk_box_append(GTK_BOX(box), add_button);
    gtk_box_append(GTK_BOX(box), view_button);
    gtk_box_append(GTK_BOX(box), search_button);
    gtk_box_append(GTK_BOX(box), update_button);
    gtk_box_append(GTK_BOX(box), delete_button);

    g_signal_connect(add_button, "clicked", G_CALLBACK(route_add_button_menu_clicked), window);
    g_signal_connect(view_button, "clicked", G_CALLBACK(route_view_button_clicked), window);
    g_signal_connect(search_button, "clicked", G_CALLBACK(route_search_button_clicked), window);
    g_signal_connect(update_button, "clicked", G_CALLBACK(route_update_button_clicked), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(route_delete_button_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   SEAT MANAGEMENT
   ========================================================= */

static void seat_add_save_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWidget *status_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "status-entry"));
    GtkWidget *student_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "student-entry"));
    GtkWidget *preference_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "preference-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Seat seat;
    int result;

    memset(&seat, 0, sizeof(Seat));
    seat.busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    seat.seatNumber = atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)));
    strncpy(seat.status, gtk_editable_get_text(GTK_EDITABLE(status_entry)), sizeof(seat.status) - 1);
    seat.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student_entry)));
    strncpy(seat.preference, gtk_editable_get_text(GTK_EDITABLE(preference_entry)), sizeof(seat.preference) - 1);

    if (seat.status[0] == '\0') {
        strcpy(seat.status, "Available");
    }

    result = add_seat(seat);

    if (result == 1) {
        show_message(window, "Success", "Seat added successfully.");
        gtk_window_destroy(window);
    } else if (result == -2) {
        show_message(window, "Error", "This seat already exists for the bus.");
    } else {
        show_message(window, "Error", "Could not add seat. Status must be Available or Occupied.");
    }
}

static void show_add_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *status_entry = create_entry("Status: Available / Occupied", FALSE);
    GtkWidget *student_entry = create_entry("Student ID (0 if available)", FALSE);
    GtkWidget *preference_entry = create_entry("Preference", FALSE);
    GtkWidget *save_button = create_button("Add Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Add Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 450);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Add Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), status_entry);
    gtk_box_append(GTK_BOX(box), student_entry);
    gtk_box_append(GTK_BOX(box), preference_entry);
    gtk_box_append(GTK_BOX(box), save_button);

    g_object_set_data(G_OBJECT(save_button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(save_button), "seat-entry", seat_entry);
    g_object_set_data(G_OBJECT(save_button), "status-entry", status_entry);
    g_object_set_data(G_OBJECT(save_button), "student-entry", student_entry);
    g_object_set_data(G_OBJECT(save_button), "preference-entry", preference_entry);
    g_signal_connect(save_button, "clicked", G_CALLBACK(seat_add_save_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}

static void show_all_seats(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *scroll;
    GtkWidget *box;
    void *records = NULL;
    Seat *seats;
    size_t count = 0;
    size_t i;

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0) {
        show_message(parent, "Error", "Could not read seat data.");
        return;
    }

    window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "All Seats");
    gtk_window_set_default_size(GTK_WINDOW(window), 650, 500);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);

    scroll = gtk_scrolled_window_new();
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);
    gtk_window_set_child(GTK_WINDOW(window), scroll);

    seats = (Seat *)records;

    if (count == 0) {
        gtk_box_append(GTK_BOX(box), gtk_label_new("No seats found."));
    } else {
        for (i = 0; i < count; i++) {
            char text[300];
            snprintf(text, sizeof(text),
                "Bus ID: %d\nSeat: %d\nStatus: %s\nStudent ID: %d\nPreference: %s",
                seats[i].busId,
                seats[i].seatNumber,
                seats[i].status,
                seats[i].studentId,
                seats[i].preference);
            gtk_box_append(GTK_BOX(box), gtk_label_new(text));
            gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
        }
    }

    free(records);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_view_button_clicked(GtkButton *button, gpointer user_data)
{
    show_all_seats(GTK_WINDOW(user_data));
}

static void seat_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Seat seat;
    int busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    int seatNumber = atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)));
    char message[300];

    if (search_seat(busId, seatNumber, &seat) == 1) {
        snprintf(message, sizeof(message),
            "Bus ID: %d\nSeat: %d\nStatus: %s\nStudent ID: %d\nPreference: %s",
            seat.busId, seat.seatNumber, seat.status,
            seat.studentId, seat.preference);
        show_message(window, "Seat Found", message);
    } else {
        show_message(window, "Not Found", "No seat found for this bus and seat number.");
    }
}

static void show_search_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *search_button = create_button("Search Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Search Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Search Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), search_button);
    g_object_set_data(G_OBJECT(search_button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(search_button), "seat-entry", seat_entry);
    g_signal_connect(search_button, "clicked", G_CALLBACK(seat_search_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_search_button_clicked(GtkButton *button, gpointer user_data)
{
    show_search_seat_window(GTK_WINDOW(user_data));
}

static void seat_load_for_update(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWidget *status_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "status-entry"));
    GtkWidget *student_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "student-entry"));
    GtkWidget *preference_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "preference-entry"));
    GtkWidget *update_button = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "update-button"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Seat seat;
    int busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    int seatNumber = atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)));
    char text[30];

    if (search_seat(busId, seatNumber, &seat) == 1) {
        gtk_editable_set_text(GTK_EDITABLE(status_entry), seat.status);
        snprintf(text, sizeof(text), "%d", seat.studentId);
        gtk_editable_set_text(GTK_EDITABLE(student_entry), text);
        gtk_editable_set_text(GTK_EDITABLE(preference_entry), seat.preference);
        gtk_widget_set_sensitive(status_entry, TRUE);
        gtk_widget_set_sensitive(student_entry, TRUE);
        gtk_widget_set_sensitive(preference_entry, TRUE);
        gtk_widget_set_sensitive(update_button, TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    } else {
        show_message(window, "Not Found", "No seat found for this bus and seat number.");
    }
}

static void seat_update_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWidget *status_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "status-entry"));
    GtkWidget *student_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "student-entry"));
    GtkWidget *preference_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "preference-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    Seat seat;
    int busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    int seatNumber = atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)));

    if (search_seat(busId, seatNumber, &seat) != 1) {
        show_message(window, "Error", "Seat not found.");
        return;
    }

    strncpy(seat.status, gtk_editable_get_text(GTK_EDITABLE(status_entry)), sizeof(seat.status) - 1);
    seat.status[sizeof(seat.status) - 1] = '\0';
    seat.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student_entry)));
    strncpy(seat.preference, gtk_editable_get_text(GTK_EDITABLE(preference_entry)), sizeof(seat.preference) - 1);
    seat.preference[sizeof(seat.preference) - 1] = '\0';

    if (update_seat(seat) == 1) {
        show_message(window, "Success", "Seat updated successfully.");
        gtk_window_destroy(window);
    } else {
        show_message(window, "Error", "Could not update seat. Status must be Available or Occupied.");
    }
}

static void show_update_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *load_button = create_button("Load Seat");
    GtkWidget *status_entry = create_entry("Status: Available / Occupied", FALSE);
    GtkWidget *student_entry = create_entry("Student ID", FALSE);
    GtkWidget *preference_entry = create_entry("Preference", FALSE);
    GtkWidget *update_button = create_button("Update Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Update Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 450);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Update Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), load_button);
    gtk_box_append(GTK_BOX(box), status_entry);
    gtk_box_append(GTK_BOX(box), student_entry);
    gtk_box_append(GTK_BOX(box), preference_entry);
    gtk_box_append(GTK_BOX(box), update_button);

    gtk_widget_set_sensitive(status_entry, FALSE);
    gtk_widget_set_sensitive(student_entry, FALSE);
    gtk_widget_set_sensitive(preference_entry, FALSE);
    gtk_widget_set_sensitive(update_button, FALSE);

    g_object_set_data(G_OBJECT(load_button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(load_button), "seat-entry", seat_entry);
    g_object_set_data(G_OBJECT(load_button), "status-entry", status_entry);
    g_object_set_data(G_OBJECT(load_button), "student-entry", student_entry);
    g_object_set_data(G_OBJECT(load_button), "preference-entry", preference_entry);
    g_object_set_data(G_OBJECT(load_button), "update-button", update_button);

    g_object_set_data(G_OBJECT(update_button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(update_button), "seat-entry", seat_entry);
    g_object_set_data(G_OBJECT(update_button), "status-entry", status_entry);
    g_object_set_data(G_OBJECT(update_button), "student-entry", student_entry);
    g_object_set_data(G_OBJECT(update_button), "preference-entry", preference_entry);

    g_signal_connect(load_button, "clicked", G_CALLBACK(seat_load_for_update), window);
    g_signal_connect(update_button, "clicked", G_CALLBACK(seat_update_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_delete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    int busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    int seatNumber = atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)));
    int result = delete_seat(busId, seatNumber);

    if (result == 1) {
        show_message(window, "Success", "Seat deleted successfully.");
        gtk_window_destroy(window);
    } else if (result == 0) {
        show_message(window, "Not Found", "Seat not found.");
    } else {
        show_message(window, "Error", "Could not delete seat.");
    }
}

static void show_delete_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *delete_button = create_button("Delete Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Delete Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Delete Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), delete_button);
    g_object_set_data(G_OBJECT(delete_button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(delete_button), "seat-entry", seat_entry);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(seat_delete_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_delete_button_clicked(GtkButton *button, gpointer user_data)
{
    show_delete_seat_window(GTK_WINDOW(user_data));
}

static void seat_allocate_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWidget *student_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "student-entry"));
    GtkWidget *preference_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "preference-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    int result = allocate_seat(
        atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry))),
        atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry))),
        atoi(gtk_editable_get_text(GTK_EDITABLE(student_entry))),
        gtk_editable_get_text(GTK_EDITABLE(preference_entry))
    );

    if (result == 1) {
        show_message(window, "Success", "Seat allocated successfully.");
        gtk_window_destroy(window);
    } else if (result == -2) {
        show_message(window, "Error", "Seat is already occupied.");
    } else if (result == 0) {
        show_message(window, "Not Found", "Seat not found.");
    } else {
        show_message(window, "Error", "Could not allocate seat.");
    }
}

static void show_allocate_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *student_entry = create_entry("Student ID", FALSE);
    GtkWidget *preference_entry = create_entry("Preference", FALSE);
    GtkWidget *button = create_button("Allocate Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Allocate Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 350);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Allocate Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), student_entry);
    gtk_box_append(GTK_BOX(box), preference_entry);
    gtk_box_append(GTK_BOX(box), button);

    g_object_set_data(G_OBJECT(button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(button), "seat-entry", seat_entry);
    g_object_set_data(G_OBJECT(button), "student-entry", student_entry);
    g_object_set_data(G_OBJECT(button), "preference-entry", preference_entry);
    g_signal_connect(button, "clicked", G_CALLBACK(seat_allocate_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_release_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *bus_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "bus-entry"));
    GtkWidget *seat_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "seat-entry"));
    GtkWindow *window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));
    int result = release_seat(
        atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry))),
        atoi(gtk_editable_get_text(GTK_EDITABLE(seat_entry)))
    );

    if (result == 1) {
        show_message(window, "Success", "Seat released successfully.");
        gtk_window_destroy(window);
    } else if (result == 0) {
        show_message(window, "Not Found", "Seat not found.");
    } else {
        show_message(window, "Error", "Could not release seat.");
    }
}

static void show_release_seat_window(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *bus_entry = create_entry("Bus ID", FALSE);
    GtkWidget *seat_entry = create_entry("Seat Number", FALSE);
    GtkWidget *button = create_button("Release Seat");

    gtk_window_set_title(GTK_WINDOW(window), "Release Seat");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_widget_set_margin_top(box, 25);
    gtk_widget_set_margin_bottom(box, 25);
    gtk_widget_set_margin_start(box, 25);
    gtk_widget_set_margin_end(box, 25);
    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box), gtk_label_new("Release Seat"));
    gtk_box_append(GTK_BOX(box), bus_entry);
    gtk_box_append(GTK_BOX(box), seat_entry);
    gtk_box_append(GTK_BOX(box), button);
    g_object_set_data(G_OBJECT(button), "bus-entry", bus_entry);
    g_object_set_data(G_OBJECT(button), "seat-entry", seat_entry);
    g_signal_connect(button, "clicked", G_CALLBACK(seat_release_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void seat_add_button_clicked(GtkButton *button, gpointer user_data)
{
    show_add_seat_window(GTK_WINDOW(user_data));
}

static void seat_update_button_clicked(GtkButton *button, gpointer user_data)
{
    show_update_seat_window(GTK_WINDOW(user_data));
}

static void seat_allocate_button_clicked(GtkButton *button, gpointer user_data)
{
    show_allocate_seat_window(GTK_WINDOW(user_data));
}

static void seat_release_button_clicked(GtkButton *button, gpointer user_data)
{
    show_release_seat_window(GTK_WINDOW(user_data));
}

void show_seat_module(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *add_button;
    GtkWidget *view_button;
    GtkWidget *search_button;
    GtkWidget *update_button;
    GtkWidget *delete_button;
    GtkWidget *allocate_button;
    GtkWidget *release_button;

    window = create_module_window(parent, "Seat Management");
    box = gtk_window_get_child(GTK_WINDOW(window));

    add_button = create_button("Add Seat");
    view_button = create_button("View Seats");
    search_button = create_button("Search Seat");
    update_button = create_button("Update Seat");
    delete_button = create_button("Delete Seat");
    allocate_button = create_button("Allocate Seat");
    release_button = create_button("Release Seat");

    gtk_box_append(GTK_BOX(box), add_button);
    gtk_box_append(GTK_BOX(box), view_button);
    gtk_box_append(GTK_BOX(box), search_button);
    gtk_box_append(GTK_BOX(box), update_button);
    gtk_box_append(GTK_BOX(box), delete_button);
    gtk_box_append(GTK_BOX(box), allocate_button);
    gtk_box_append(GTK_BOX(box), release_button);

    g_signal_connect(add_button, "clicked", G_CALLBACK(seat_add_button_clicked), window);
    g_signal_connect(view_button, "clicked", G_CALLBACK(seat_view_button_clicked), window);
    g_signal_connect(search_button, "clicked", G_CALLBACK(seat_search_button_clicked), window);
    g_signal_connect(update_button, "clicked", G_CALLBACK(seat_update_button_clicked), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(seat_delete_button_clicked), window);
    g_signal_connect(allocate_button, "clicked", G_CALLBACK(seat_allocate_button_clicked), window);
    g_signal_connect(release_button, "clicked", G_CALLBACK(seat_release_button_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   SCHEDULE MANAGEMENT
   ========================================================= */

static void schedule_add_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *id_entry = g_object_get_data(G_OBJECT(box), "schedule-id");
    GtkWidget *bus_entry = g_object_get_data(G_OBJECT(box), "schedule-bus-id");
    GtkWidget *route_entry = g_object_get_data(G_OBJECT(box), "schedule-route-id");
    GtkWidget *start_entry = g_object_get_data(G_OBJECT(box), "schedule-start");
    GtkWidget *end_entry = g_object_get_data(G_OBJECT(box), "schedule-end");
    GtkWidget *day_entry = g_object_get_data(G_OBJECT(box), "schedule-day");
    GtkWidget *departure_entry = g_object_get_data(G_OBJECT(box), "schedule-departure");
    Schedule schedule;
    int result;

    schedule.scheduleId = atoi(gtk_editable_get_text(GTK_EDITABLE(id_entry)));
    schedule.busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus_entry)));
    schedule.routeId = atoi(gtk_editable_get_text(GTK_EDITABLE(route_entry)));
    snprintf(schedule.startTime, sizeof(schedule.startTime), "%s", gtk_editable_get_text(GTK_EDITABLE(start_entry)));
    snprintf(schedule.endTime, sizeof(schedule.endTime), "%s", gtk_editable_get_text(GTK_EDITABLE(end_entry)));
    snprintf(schedule.dayType, sizeof(schedule.dayType), "%s", gtk_editable_get_text(GTK_EDITABLE(day_entry)));
    snprintf(schedule.departure_time, sizeof(schedule.departure_time), "%s", gtk_editable_get_text(GTK_EDITABLE(departure_entry)));

    result = add_schedule(schedule);

    if (result == 1)
    {
        show_message(window, "Success", "Schedule added successfully.");
        gtk_window_destroy(window);
    }
    else if (result == -2)
        show_message(window, "Error", "Schedule ID already exists.");
    else
        show_message(window, "Error", "Invalid schedule data or file error.");
}

static void show_add_schedule_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Add Schedule");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *id = create_entry("Schedule ID", FALSE);
    GtkWidget *bus = create_entry("Bus ID", FALSE);
    GtkWidget *route = create_entry("Route ID", FALSE);
    GtkWidget *start = create_entry("Start Time", FALSE);
    GtkWidget *end = create_entry("End Time", FALSE);
    GtkWidget *day = create_entry("Day Type (Full Day/Half Day)", FALSE);
    GtkWidget *departure = create_entry("Departure Time", FALSE);
    GtkWidget *button = create_button("Add Schedule");

    g_object_set_data(G_OBJECT(box), "schedule-id", id);
    g_object_set_data(G_OBJECT(box), "schedule-bus-id", bus);
    g_object_set_data(G_OBJECT(box), "schedule-route-id", route);
    g_object_set_data(G_OBJECT(box), "schedule-start", start);
    g_object_set_data(G_OBJECT(box), "schedule-end", end);
    g_object_set_data(G_OBJECT(box), "schedule-day", day);
    g_object_set_data(G_OBJECT(box), "schedule-departure", departure);

    gtk_box_append(GTK_BOX(box), id);
    gtk_box_append(GTK_BOX(box), bus);
    gtk_box_append(GTK_BOX(box), route);
    gtk_box_append(GTK_BOX(box), start);
    gtk_box_append(GTK_BOX(box), end);
    gtk_box_append(GTK_BOX(box), day);
    gtk_box_append(GTK_BOX(box), departure);
    gtk_box_append(GTK_BOX(box), button);

    g_signal_connect(button, "clicked", G_CALLBACK(schedule_add_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void show_all_schedules(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "All Schedules");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *scrolled = gtk_scrolled_window_new();
    GtkWidget *view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    void *records = NULL;
    Schedule *schedules;
    size_t count = 0;
    size_t i;
    char text[512];

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 450);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_box_append(GTK_BOX(box), scrolled);

    if (readRecords(SCHEDULES_FILE, sizeof(Schedule), &records, &count) != 0 || count == 0)
    {
        gtk_text_buffer_set_text(buffer, "No schedules found.", -1);
    }
    else
    {
        schedules = (Schedule *)records;
        for (i = 0; i < count; i++)
        {
            snprintf(text, sizeof(text),
                     "Schedule ID: %d\nBus ID: %d\nRoute ID: %d\nStart Time: %s\nEnd Time: %s\nDay Type: %s\nDeparture Time: %s\n\n",
                     schedules[i].scheduleId, schedules[i].busId, schedules[i].routeId,
                     schedules[i].startTime, schedules[i].endTime,
                     schedules[i].dayType, schedules[i].departure_time);
            gtk_text_buffer_insert_at_cursor(buffer, text, -1);
        }
        free(records);
    }

    gtk_window_present(GTK_WINDOW(window));
}

static void schedule_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *entry = g_object_get_data(G_OBJECT(box), "search-id");
    Schedule schedule;
    char text[512];
    int id = atoi(gtk_editable_get_text(GTK_EDITABLE(entry)));
    int result = search_schedule(id, &schedule);

    if (result == 1)
    {
        snprintf(text, sizeof(text),
                 "Schedule ID: %d\nBus ID: %d\nRoute ID: %d\nStart Time: %s\nEnd Time: %s\nDay Type: %s\nDeparture Time: %s",
                 schedule.scheduleId, schedule.busId, schedule.routeId,
                 schedule.startTime, schedule.endTime,
                 schedule.dayType, schedule.departure_time);
        show_message(window, "Schedule Found", text);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Schedule not found.");
    else
        show_message(window, "Error", "Could not read schedule data.");
}

static void show_search_schedule_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Search Schedule");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *entry = create_entry("Schedule ID", FALSE);
    GtkWidget *button = create_button("Search Schedule");
    g_object_set_data(G_OBJECT(box), "search-id", entry);
    gtk_box_append(GTK_BOX(box), entry);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(schedule_search_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void schedule_update_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *id = g_object_get_data(G_OBJECT(box), "schedule-id");
    GtkWidget *bus = g_object_get_data(G_OBJECT(box), "schedule-bus-id");
    GtkWidget *route = g_object_get_data(G_OBJECT(box), "schedule-route-id");
    GtkWidget *start = g_object_get_data(G_OBJECT(box), "schedule-start");
    GtkWidget *end = g_object_get_data(G_OBJECT(box), "schedule-end");
    GtkWidget *day = g_object_get_data(G_OBJECT(box), "schedule-day");
    GtkWidget *departure = g_object_get_data(G_OBJECT(box), "schedule-departure");
    Schedule schedule;
    int result;

    schedule.scheduleId = atoi(gtk_editable_get_text(GTK_EDITABLE(id)));
    schedule.busId = atoi(gtk_editable_get_text(GTK_EDITABLE(bus)));
    schedule.routeId = atoi(gtk_editable_get_text(GTK_EDITABLE(route)));
    snprintf(schedule.startTime, sizeof(schedule.startTime), "%s", gtk_editable_get_text(GTK_EDITABLE(start)));
    snprintf(schedule.endTime, sizeof(schedule.endTime), "%s", gtk_editable_get_text(GTK_EDITABLE(end)));
    snprintf(schedule.dayType, sizeof(schedule.dayType), "%s", gtk_editable_get_text(GTK_EDITABLE(day)));
    snprintf(schedule.departure_time, sizeof(schedule.departure_time), "%s", gtk_editable_get_text(GTK_EDITABLE(departure)));

    result = update_schedule(schedule);
    if (result == 1)
    {
        show_message(window, "Success", "Schedule updated successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Schedule not found.");
    else
        show_message(window, "Error", "Invalid schedule data or file error.");
}

static void show_update_schedule_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Update Schedule");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *id = create_entry("Schedule ID", FALSE);
    GtkWidget *bus = create_entry("Bus ID", FALSE);
    GtkWidget *route = create_entry("Route ID", FALSE);
    GtkWidget *start = create_entry("Start Time", FALSE);
    GtkWidget *end = create_entry("End Time", FALSE);
    GtkWidget *day = create_entry("Day Type", FALSE);
    GtkWidget *departure = create_entry("Departure Time", FALSE);
    GtkWidget *button = create_button("Update Schedule");

    g_object_set_data(G_OBJECT(box), "schedule-id", id);
    g_object_set_data(G_OBJECT(box), "schedule-bus-id", bus);
    g_object_set_data(G_OBJECT(box), "schedule-route-id", route);
    g_object_set_data(G_OBJECT(box), "schedule-start", start);
    g_object_set_data(G_OBJECT(box), "schedule-end", end);
    g_object_set_data(G_OBJECT(box), "schedule-day", day);
    g_object_set_data(G_OBJECT(box), "schedule-departure", departure);

    gtk_box_append(GTK_BOX(box), id);
    gtk_box_append(GTK_BOX(box), bus);
    gtk_box_append(GTK_BOX(box), route);
    gtk_box_append(GTK_BOX(box), start);
    gtk_box_append(GTK_BOX(box), end);
    gtk_box_append(GTK_BOX(box), day);
    gtk_box_append(GTK_BOX(box), departure);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(schedule_update_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void schedule_delete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *entry = g_object_get_data(G_OBJECT(box), "schedule-id");
    int result = delete_schedule(atoi(gtk_editable_get_text(GTK_EDITABLE(entry))));

    if (result == 1)
    {
        show_message(window, "Success", "Schedule deleted successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Schedule not found.");
    else
        show_message(window, "Error", "Invalid Schedule ID or file error.");
}

static void show_delete_schedule_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Delete Schedule");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *entry = create_entry("Schedule ID", FALSE);
    GtkWidget *button = create_button("Delete Schedule");
    g_object_set_data(G_OBJECT(box), "schedule-id", entry);
    gtk_box_append(GTK_BOX(box), entry);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(schedule_delete_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void schedule_add_window_clicked(GtkButton *button, gpointer user_data) { show_add_schedule_window(GTK_WINDOW(user_data)); }
static void schedule_view_window_clicked(GtkButton *button, gpointer user_data) { show_all_schedules(GTK_WINDOW(user_data)); }
static void schedule_search_window_clicked(GtkButton *button, gpointer user_data) { show_search_schedule_window(GTK_WINDOW(user_data)); }
static void schedule_update_window_clicked(GtkButton *button, gpointer user_data) { show_update_schedule_window(GTK_WINDOW(user_data)); }
static void schedule_delete_window_clicked(GtkButton *button, gpointer user_data) { show_delete_schedule_window(GTK_WINDOW(user_data)); }

void show_schedule_module(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Schedule Management");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *add = create_button("Add Schedule");
    GtkWidget *view = create_button("View Schedules");
    GtkWidget *search = create_button("Search Schedule");
    GtkWidget *update = create_button("Update Schedule");
    GtkWidget *delete_button = create_button("Delete Schedule");

    gtk_box_append(GTK_BOX(box), add);
    gtk_box_append(GTK_BOX(box), view);
    gtk_box_append(GTK_BOX(box), search);
    gtk_box_append(GTK_BOX(box), update);
    gtk_box_append(GTK_BOX(box), delete_button);

    g_signal_connect(add, "clicked", G_CALLBACK(schedule_add_window_clicked), window);
    g_signal_connect(view, "clicked", G_CALLBACK(schedule_view_window_clicked), window);
    g_signal_connect(search, "clicked", G_CALLBACK(schedule_search_window_clicked), window);
    g_signal_connect(update, "clicked", G_CALLBACK(schedule_update_window_clicked), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(schedule_delete_window_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   BOOKING MANAGEMENT
   ========================================================= */

static void booking_add_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "booking-student");
    GtkWidget *schedule = g_object_get_data(G_OBJECT(box), "booking-schedule");
    GtkWidget *seat = g_object_get_data(G_OBJECT(box), "booking-seat");
    Booking booking;
    int result;

    booking.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student)));
    booking.scheduleId = atoi(gtk_editable_get_text(GTK_EDITABLE(schedule)));
    booking.seat_number = atoi(gtk_editable_get_text(GTK_EDITABLE(seat)));
    result = add_booking(booking);

    if (result == 1)
    {
        show_message(window, "Success", "Booking added successfully.");
        gtk_window_destroy(window);
    }
    else if (result == -2)
        show_message(window, "Error", "This student already has a booking for this schedule.");
    else if (result == -3)
        show_message(window, "Error", "This seat is already booked for this schedule.");
    else
        show_message(window, "Error", "Invalid booking data or file error.");
}

static void show_add_booking_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Add Booking");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *schedule = create_entry("Schedule ID", FALSE);
    GtkWidget *seat = create_entry("Seat Number", FALSE);
    GtkWidget *button = create_button("Add Booking");

    g_object_set_data(G_OBJECT(box), "booking-student", student);
    g_object_set_data(G_OBJECT(box), "booking-schedule", schedule);
    g_object_set_data(G_OBJECT(box), "booking-seat", seat);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), schedule);
    gtk_box_append(GTK_BOX(box), seat);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(booking_add_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void show_all_bookings(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "All Bookings");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *scrolled = gtk_scrolled_window_new();
    GtkWidget *view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    void *records = NULL;
    Booking *bookings;
    size_t count = 0;
    size_t i;
    char text[256];

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 450);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_box_append(GTK_BOX(box), scrolled);

    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) != 0 || count == 0)
    {
        gtk_text_buffer_set_text(buffer, "No bookings found.", -1);
    }
    else
    {
        bookings = (Booking *)records;
        for (i = 0; i < count; i++)
        {
            snprintf(text, sizeof(text), "Student ID: %d\nSchedule ID: %d\nSeat Number: %d\n\n",
                     bookings[i].studentId, bookings[i].scheduleId, bookings[i].seat_number);
            gtk_text_buffer_insert_at_cursor(buffer, text, -1);
        }
        free(records);
    }
    gtk_window_present(GTK_WINDOW(window));
}

static void booking_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "booking-student");
    GtkWidget *schedule = g_object_get_data(G_OBJECT(box), "booking-schedule");
    Booking booking;
    char text[256];
    int result = search_booking(atoi(gtk_editable_get_text(GTK_EDITABLE(student))),
                                atoi(gtk_editable_get_text(GTK_EDITABLE(schedule))), &booking);

    if (result == 1)
    {
        snprintf(text, sizeof(text), "Student ID: %d\nSchedule ID: %d\nSeat Number: %d",
                 booking.studentId, booking.scheduleId, booking.seat_number);
        show_message(window, "Booking Found", text);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Booking not found.");
    else
        show_message(window, "Error", "Could not read booking data.");
}

static void show_search_booking_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Search Booking");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *schedule = create_entry("Schedule ID", FALSE);
    GtkWidget *button = create_button("Search Booking");
    g_object_set_data(G_OBJECT(box), "booking-student", student);
    g_object_set_data(G_OBJECT(box), "booking-schedule", schedule);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), schedule);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(booking_search_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void booking_update_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "booking-student");
    GtkWidget *schedule = g_object_get_data(G_OBJECT(box), "booking-schedule");
    GtkWidget *seat = g_object_get_data(G_OBJECT(box), "booking-seat");
    Booking booking;
    int result;

    booking.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student)));
    booking.scheduleId = atoi(gtk_editable_get_text(GTK_EDITABLE(schedule)));
    booking.seat_number = atoi(gtk_editable_get_text(GTK_EDITABLE(seat)));
    result = update_booking(booking);

    if (result == 1)
    {
        show_message(window, "Success", "Booking updated successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Booking not found.");
    else if (result == -3)
        show_message(window, "Error", "That seat is already booked for this schedule.");
    else
        show_message(window, "Error", "Invalid booking data or file error.");
}

static void show_update_booking_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Update Booking");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *schedule = create_entry("Schedule ID", FALSE);
    GtkWidget *seat = create_entry("New Seat Number", FALSE);
    GtkWidget *button = create_button("Update Booking");
    g_object_set_data(G_OBJECT(box), "booking-student", student);
    g_object_set_data(G_OBJECT(box), "booking-schedule", schedule);
    g_object_set_data(G_OBJECT(box), "booking-seat", seat);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), schedule);
    gtk_box_append(GTK_BOX(box), seat);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(booking_update_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void booking_delete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "booking-student");
    GtkWidget *schedule = g_object_get_data(G_OBJECT(box), "booking-schedule");
    int result = delete_booking(atoi(gtk_editable_get_text(GTK_EDITABLE(student))),
                                atoi(gtk_editable_get_text(GTK_EDITABLE(schedule))));

    if (result == 1)
    {
        show_message(window, "Success", "Booking deleted successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Booking not found.");
    else
        show_message(window, "Error", "Invalid booking data or file error.");
}

static void show_delete_booking_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Delete Booking");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *schedule = create_entry("Schedule ID", FALSE);
    GtkWidget *button = create_button("Delete Booking");
    g_object_set_data(G_OBJECT(box), "booking-student", student);
    g_object_set_data(G_OBJECT(box), "booking-schedule", schedule);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), schedule);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(booking_delete_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void booking_add_window_clicked(GtkButton *button, gpointer user_data) { show_add_booking_window(GTK_WINDOW(user_data)); }
static void booking_view_window_clicked(GtkButton *button, gpointer user_data) { show_all_bookings(GTK_WINDOW(user_data)); }
static void booking_search_window_clicked(GtkButton *button, gpointer user_data) { show_search_booking_window(GTK_WINDOW(user_data)); }
static void booking_update_window_clicked(GtkButton *button, gpointer user_data) { show_update_booking_window(GTK_WINDOW(user_data)); }
static void booking_delete_window_clicked(GtkButton *button, gpointer user_data) { show_delete_booking_window(GTK_WINDOW(user_data)); }

void show_booking_module(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Booking Management");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *add = create_button("Add Booking");
    GtkWidget *view = create_button("View Bookings");
    GtkWidget *search = create_button("Search Booking");
    GtkWidget *update = create_button("Update Booking");
    GtkWidget *delete_button = create_button("Delete Booking");

    gtk_box_append(GTK_BOX(box), add);
    gtk_box_append(GTK_BOX(box), view);
    gtk_box_append(GTK_BOX(box), search);
    gtk_box_append(GTK_BOX(box), update);
    gtk_box_append(GTK_BOX(box), delete_button);
    g_signal_connect(add, "clicked", G_CALLBACK(booking_add_window_clicked), window);
    g_signal_connect(view, "clicked", G_CALLBACK(booking_view_window_clicked), window);
    g_signal_connect(search, "clicked", G_CALLBACK(booking_search_window_clicked), window);
    g_signal_connect(update, "clicked", G_CALLBACK(booking_update_window_clicked), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(booking_delete_window_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   NOTIFICATION MANAGEMENT
   ========================================================= */

static void notification_add_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    Notification notification;
    GtkWidget *id = g_object_get_data(G_OBJECT(box), "notification-id");
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "notification-student");
    GtkWidget *message = g_object_get_data(G_OBJECT(box), "notification-message");
    GtkWidget *date = g_object_get_data(G_OBJECT(box), "notification-date");
    GtkWidget *status = g_object_get_data(G_OBJECT(box), "notification-status");
    int result;

    notification.notificationId = atoi(gtk_editable_get_text(GTK_EDITABLE(id)));
    notification.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student)));
    snprintf(notification.message, sizeof(notification.message), "%s", gtk_editable_get_text(GTK_EDITABLE(message)));
    snprintf(notification.date, sizeof(notification.date), "%s", gtk_editable_get_text(GTK_EDITABLE(date)));
    snprintf(notification.status, sizeof(notification.status), "%s", gtk_editable_get_text(GTK_EDITABLE(status)));
    result = add_notification(notification);

    if (result == 1)
    {
        show_message(window, "Success", "Notification added successfully.");
        gtk_window_destroy(window);
    }
    else if (result == -2)
        show_message(window, "Error", "Notification ID already exists.");
    else
        show_message(window, "Error", "Invalid notification data or file error.");
}

static void show_add_notification_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Add Notification");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *id = create_entry("Notification ID", FALSE);
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *message = create_entry("Message", FALSE);
    GtkWidget *date = create_entry("Date", FALSE);
    GtkWidget *status = create_entry("Status (Unread/Read)", FALSE);
    GtkWidget *button = create_button("Add Notification");

    g_object_set_data(G_OBJECT(box), "notification-id", id);
    g_object_set_data(G_OBJECT(box), "notification-student", student);
    g_object_set_data(G_OBJECT(box), "notification-message", message);
    g_object_set_data(G_OBJECT(box), "notification-date", date);
    g_object_set_data(G_OBJECT(box), "notification-status", status);
    gtk_box_append(GTK_BOX(box), id);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), message);
    gtk_box_append(GTK_BOX(box), date);
    gtk_box_append(GTK_BOX(box), status);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(notification_add_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void show_all_notifications(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "All Notifications");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *scrolled = gtk_scrolled_window_new();
    GtkWidget *view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    void *records = NULL;
    Notification *notifications;
    size_t count = 0;
    size_t i;
    char text[512];

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 450);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_box_append(GTK_BOX(box), scrolled);

    if (readRecords(NOTIFICATIONS_FILE, sizeof(Notification), &records, &count) != 0 || count == 0)
    {
        gtk_text_buffer_set_text(buffer, "No notifications found.", -1);
    }
    else
    {
        notifications = (Notification *)records;
        for (i = 0; i < count; i++)
        {
            snprintf(text, sizeof(text),
                     "Notification ID: %d\nStudent ID: %d\nMessage: %s\nDate: %s\nStatus: %s\n\n",
                     notifications[i].notificationId, notifications[i].studentId,
                     notifications[i].message, notifications[i].date, notifications[i].status);
            gtk_text_buffer_insert_at_cursor(buffer, text, -1);
        }
        free(records);
    }
    gtk_window_present(GTK_WINDOW(window));
}

static void notification_search_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *entry = g_object_get_data(G_OBJECT(box), "notification-id");
    Notification notification;
    char text[512];
    int result = search_notification(atoi(gtk_editable_get_text(GTK_EDITABLE(entry))), &notification);

    if (result == 1)
    {
        snprintf(text, sizeof(text), "Notification ID: %d\nStudent ID: %d\nMessage: %s\nDate: %s\nStatus: %s",
                 notification.notificationId, notification.studentId,
                 notification.message, notification.date, notification.status);
        show_message(window, "Notification Found", text);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Notification not found.");
    else
        show_message(window, "Error", "Could not read notification data.");
}

static void show_search_notification_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Search Notification");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *entry = create_entry("Notification ID", FALSE);
    GtkWidget *button = create_button("Search Notification");
    g_object_set_data(G_OBJECT(box), "notification-id", entry);
    gtk_box_append(GTK_BOX(box), entry);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(notification_search_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void notification_update_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    Notification notification;
    GtkWidget *id = g_object_get_data(G_OBJECT(box), "notification-id");
    GtkWidget *student = g_object_get_data(G_OBJECT(box), "notification-student");
    GtkWidget *message = g_object_get_data(G_OBJECT(box), "notification-message");
    GtkWidget *date = g_object_get_data(G_OBJECT(box), "notification-date");
    GtkWidget *status = g_object_get_data(G_OBJECT(box), "notification-status");
    int result;

    notification.notificationId = atoi(gtk_editable_get_text(GTK_EDITABLE(id)));
    notification.studentId = atoi(gtk_editable_get_text(GTK_EDITABLE(student)));
    snprintf(notification.message, sizeof(notification.message), "%s", gtk_editable_get_text(GTK_EDITABLE(message)));
    snprintf(notification.date, sizeof(notification.date), "%s", gtk_editable_get_text(GTK_EDITABLE(date)));
    snprintf(notification.status, sizeof(notification.status), "%s", gtk_editable_get_text(GTK_EDITABLE(status)));
    result = update_notification(notification);

    if (result == 1)
    {
        show_message(window, "Success", "Notification updated successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Notification not found.");
    else
        show_message(window, "Error", "Invalid notification data or file error.");
}

static void show_update_notification_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Update Notification");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *id = create_entry("Notification ID", FALSE);
    GtkWidget *student = create_entry("Student ID", FALSE);
    GtkWidget *message = create_entry("Message", FALSE);
    GtkWidget *date = create_entry("Date", FALSE);
    GtkWidget *status = create_entry("Status", FALSE);
    GtkWidget *button = create_button("Update Notification");

    g_object_set_data(G_OBJECT(box), "notification-id", id);
    g_object_set_data(G_OBJECT(box), "notification-student", student);
    g_object_set_data(G_OBJECT(box), "notification-message", message);
    g_object_set_data(G_OBJECT(box), "notification-date", date);
    g_object_set_data(G_OBJECT(box), "notification-status", status);
    gtk_box_append(GTK_BOX(box), id);
    gtk_box_append(GTK_BOX(box), student);
    gtk_box_append(GTK_BOX(box), message);
    gtk_box_append(GTK_BOX(box), date);
    gtk_box_append(GTK_BOX(box), status);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(notification_update_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void notification_delete_clicked(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *entry = g_object_get_data(G_OBJECT(box), "notification-id");
    int result = delete_notification(atoi(gtk_editable_get_text(GTK_EDITABLE(entry))));

    if (result == 1)
    {
        show_message(window, "Success", "Notification deleted successfully.");
        gtk_window_destroy(window);
    }
    else if (result == 0)
        show_message(window, "Not Found", "Notification not found.");
    else
        show_message(window, "Error", "Invalid Notification ID or file error.");
}

static void show_delete_notification_window(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Delete Notification");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *entry = create_entry("Notification ID", FALSE);
    GtkWidget *button = create_button("Delete Notification");
    g_object_set_data(G_OBJECT(box), "notification-id", entry);
    gtk_box_append(GTK_BOX(box), entry);
    gtk_box_append(GTK_BOX(box), button);
    g_signal_connect(button, "clicked", G_CALLBACK(notification_delete_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void notification_add_window_clicked(GtkButton *button, gpointer user_data) { show_add_notification_window(GTK_WINDOW(user_data)); }
static void notification_view_window_clicked(GtkButton *button, gpointer user_data) { show_all_notifications(GTK_WINDOW(user_data)); }
static void notification_search_window_clicked(GtkButton *button, gpointer user_data) { show_search_notification_window(GTK_WINDOW(user_data)); }
static void notification_update_window_clicked(GtkButton *button, gpointer user_data) { show_update_notification_window(GTK_WINDOW(user_data)); }
static void notification_delete_window_clicked(GtkButton *button, gpointer user_data) { show_delete_notification_window(GTK_WINDOW(user_data)); }

void show_notification_module(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Notification Management");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *add = create_button("Add Notification");
    GtkWidget *view = create_button("View Notifications");
    GtkWidget *search = create_button("Search Notification");
    GtkWidget *update = create_button("Update Notification");
    GtkWidget *delete_button = create_button("Delete Notification");

    gtk_box_append(GTK_BOX(box), add);
    gtk_box_append(GTK_BOX(box), view);
    gtk_box_append(GTK_BOX(box), search);
    gtk_box_append(GTK_BOX(box), update);
    gtk_box_append(GTK_BOX(box), delete_button);
    g_signal_connect(add, "clicked", G_CALLBACK(notification_add_window_clicked), window);
    g_signal_connect(view, "clicked", G_CALLBACK(notification_view_window_clicked), window);
    g_signal_connect(search, "clicked", G_CALLBACK(notification_search_window_clicked), window);
    g_signal_connect(update, "clicked", G_CALLBACK(notification_update_window_clicked), window);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(notification_delete_window_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   REPORTS
   ========================================================= */

static void show_report_data(GtkWindow *window)
{
    GtkWidget *box = gtk_window_get_child(window);
    GtkWidget *label = g_object_get_data(G_OBJECT(box), "report-label");
    char text[1000];

    snprintf(text, sizeof(text),
             "BUSMATE SYSTEM REPORT\n\n"
             "Students: %d\n"
             "Drivers: %d\n"
             "Buses: %d\n"
             "Routes: %d\n"
             "Schedules: %d\n"
             "Bookings: %d\n"
             "Seats: %d\n"
             "Available Seats: %d\n"
             "Occupied Seats: %d\n"
             "Notifications: %d",
             get_student_count(), get_driver_count(), get_bus_count(),
             get_route_count(), get_schedule_count(), get_booking_count(),
             get_seat_count(), get_available_seat_count(),
             get_occupied_seat_count(), get_notification_count());

    gtk_label_set_text(GTK_LABEL(label), text);
}

static void report_refresh_clicked(GtkButton *button, gpointer user_data)
{
    show_report_data(GTK_WINDOW(user_data));
}

void show_report_module(GtkWindow *parent)
{
    GtkWidget *window = create_module_window(parent, "Reports");
    GtkWidget *box = gtk_window_get_child(GTK_WINDOW(window));
    GtkWidget *label = gtk_label_new("");
    GtkWidget *refresh = create_button("Refresh Report");

    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    g_object_set_data(G_OBJECT(box), "report-label", label);
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), refresh);
    g_signal_connect(refresh, "clicked", G_CALLBACK(report_refresh_clicked), window);
    show_report_data(GTK_WINDOW(window));
    gtk_window_present(GTK_WINDOW(window));
}


/* =========================================================
   STUDENT MODULES
   ========================================================= */

void show_student_profile(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    Student student;
    GtkWidget *title;
    GtkWidget *info;
    char text[300];

    window = create_module_window(
        parent,
        "My Profile"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "My Profile"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    if (current_student_id <= 0 ||
        search_student(current_student_id, &student) != 1)
    {
        info = gtk_label_new(
            "Student profile could not be loaded."
        );

        gtk_box_append(
            GTK_BOX(box),
            info
        );
    }
    else
    {
        snprintf(
            text,
            sizeof(text),
            "Student ID: %d\nName: %s",
            student.studentId,
            student.name
        );

        info = gtk_label_new(text);
        gtk_label_set_selectable(GTK_LABEL(info), TRUE);
        gtk_label_set_xalign(GTK_LABEL(info), 0.0f);

        gtk_box_append(
            GTK_BOX(box),
            info
        );
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_student_buses(GtkWindow *parent)
{
    show_all_buses(parent);
}


void show_student_routes(GtkWindow *parent)
{
    show_all_routes(parent);
}


void show_student_schedules(GtkWindow *parent)
{
    show_all_schedules(parent);
}


static void student_booking_save_clicked(
    GtkButton *button,
    gpointer user_data)
{
    GtkWindow *window;
    GtkWidget *box;
    GtkWidget *schedule_entry;
    GtkWidget *seat_entry;
    Schedule schedule;
    Seat seat;
    Bus bus;
    Booking booking;
    int schedule_id;
    int seat_number;
    int result;

    window = GTK_WINDOW(user_data);
    box = gtk_window_get_child(window);

    schedule_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(box),
            "student-booking-schedule"
        )
    );

    seat_entry = GTK_WIDGET(
        g_object_get_data(
            G_OBJECT(box),
            "student-booking-seat"
        )
    );

    if (current_student_id <= 0)
    {
        show_message(
            window,
            "Error",
            "Student session is not active."
        );
        return;
    }

    schedule_id = atoi(
        gtk_editable_get_text(
            GTK_EDITABLE(schedule_entry)
        )
    );

    seat_number = atoi(
        gtk_editable_get_text(
            GTK_EDITABLE(seat_entry)
        )
    );

    if (schedule_id <= 0 || seat_number <= 0)
    {
        show_message(
            window,
            "Error",
            "Enter valid Schedule ID and Seat Number."
        );
        return;
    }

    result = search_schedule(
        schedule_id,
        &schedule
    );

    if (result != 1)
    {
        show_message(
            window,
            "Error",
            "Schedule not found."
        );
        return;
    }

    result = search_seat(
        schedule.busId,
        seat_number,
        &seat
    );

    if (result != 1)
    {
        show_message(
            window,
            "Error",
            "Seat not found for this bus."
        );
        return;
    }

    if (strcmp(seat.status, "Available") != 0)
    {
        show_message(
            window,
            "Error",
            "This seat is already occupied."
        );
        return;
    }

    booking.studentId = current_student_id;
    booking.scheduleId = schedule_id;
    booking.seat_number = seat_number;

    result = add_booking(booking);

    if (result == -2)
    {
        show_message(
            window,
            "Error",
            "You already have a booking for this schedule."
        );
        return;
    }

    if (result == -3)
    {
        show_message(
            window,
            "Error",
            "This seat is already booked for this schedule."
        );
        return;
    }

    if (result != 1)
    {
        show_message(
            window,
            "Error",
            "Could not create booking."
        );
        return;
    }

    result = allocate_seat(
        schedule.busId,
        seat_number,
        current_student_id,
        "Student Selected"
    );

    if (result != 1)
    {
        delete_booking(
            current_student_id,
            schedule_id
        );

        show_message(
            window,
            "Error",
            "Seat allocation failed. Booking was cancelled."
        );
        return;
    }

    if (search_bus(schedule.busId, &bus) == 1)
    {
        if (bus.availableSeats > 0)
        {
            bus.availableSeats--;
            update_bus(bus);
        }
    }

    show_message(
        window,
        "Success",
        "Seat booked successfully."
    );

    gtk_window_destroy(window);
}


void show_student_booking(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *schedule_entry;
    GtkWidget *seat_entry;
    GtkWidget *button;

    window = create_module_window(
        parent,
        "Book Seat"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Book Seat"
    );

    gtk_widget_set_halign(
        title,
        GTK_ALIGN_CENTER
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    schedule_entry = create_entry(
        "Schedule ID",
        FALSE
    );

    seat_entry = create_entry(
        "Seat Number",
        FALSE
    );

    button = create_button(
        "Book Seat"
    );

    gtk_box_append(
        GTK_BOX(box),
        schedule_entry
    );

    gtk_box_append(
        GTK_BOX(box),
        seat_entry
    );

    gtk_box_append(
        GTK_BOX(box),
        button
    );

    g_object_set_data(
        G_OBJECT(box),
        "student-booking-schedule",
        schedule_entry
    );

    g_object_set_data(
        G_OBJECT(box),
        "student-booking-seat",
        seat_entry
    );

    g_signal_connect(
        button,
        "clicked",
        G_CALLBACK(student_booking_save_clicked),
        window
    );

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_student_notifications(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *scroll;
    GtkWidget *view;
    GtkTextBuffer *buffer;
    void *records;
    Notification *notifications;
    size_t count;
    size_t i;
    int found;

    window = create_module_window(
        parent,
        "Notifications"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    scroll = gtk_scrolled_window_new();
    view = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(
        GTK_TEXT_VIEW(view)
    );

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(view),
        FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(view),
        FALSE
    );

    gtk_scrolled_window_set_min_content_height(
        GTK_SCROLLED_WINDOW(scroll),
        450
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        view
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    records = NULL;
    count = 0;
    found = 0;

    if (current_student_id <= 0 ||
        readRecords(
            NOTIFICATIONS_FILE,
            sizeof(Notification),
            &records,
            &count
        ) != 0)
    {
        gtk_text_buffer_set_text(
            buffer,
            "Could not load notifications.",
            -1
        );

        gtk_window_present(
            GTK_WINDOW(window)
        );
        return;
    }

    notifications = (Notification *)records;

    for (i = 0; i < count; i++)
    {
        if (notifications[i].studentId == current_student_id)
        {
            char text[500];

            snprintf(
                text,
                sizeof(text),
                "Notification ID: %d\nMessage: %s\nDate: %s\nStatus: %s\n\n",
                notifications[i].notificationId,
                notifications[i].message,
                notifications[i].date,
                notifications[i].status
            );

            gtk_text_buffer_insert_at_cursor(
                buffer,
                text,
                -1
            );

            found = 1;
        }
    }

    if (!found)
    {
        gtk_text_buffer_set_text(
            buffer,
            "No notifications found.",
            -1
        );
    }

    free(records);

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   DRIVER MODULES
   ========================================================= */

void show_driver_profile(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *label;
    Driver driver;
    char text[400];

    window = create_module_window(
        parent,
        "My Profile"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    if (search_driver(logged_driver_id, &driver) != 1)
    {
        label = gtk_label_new(
            "Driver information could not be found."
        );

        gtk_box_append(
            GTK_BOX(box),
            label
        );
    }
    else
    {
        label = gtk_label_new(NULL);

        snprintf(
            text,
            sizeof(text),
            "Driver ID: %d\nName: %s\nPhone: %s\nAssigned Bus ID: %d",
            driver.driverId,
            driver.name,
            driver.phone,
            driver.assignedBusId
        );

        gtk_label_set_text(
            GTK_LABEL(label),
            text
        );

        gtk_label_set_xalign(
            GTK_LABEL(label),
            0.0
        );

        gtk_box_append(
            GTK_BOX(box),
            label
        );
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_driver_bus(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *label;
    Driver driver;
    Bus bus;
    char text[400];

    window = create_module_window(
        parent,
        "Assigned Bus"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    if (search_driver(logged_driver_id, &driver) != 1)
    {
        label = gtk_label_new(
            "Driver information could not be found."
        );

        gtk_box_append(
            GTK_BOX(box),
            label
        );
    }
    else if (search_bus(driver.assignedBusId, &bus) != 1)
    {
        label = gtk_label_new(
            "Assigned bus information could not be found."
        );

        gtk_box_append(
            GTK_BOX(box),
            label
        );
    }
    else
    {
        label = gtk_label_new(NULL);

        snprintf(
            text,
            sizeof(text),
            "Bus ID: %d\nBus Number: %s\nCapacity: %d\nAvailable Seats: %d",
            bus.busId,
            bus.busNumber,
            bus.capacity,
            bus.availableSeats
        );

        gtk_label_set_text(
            GTK_LABEL(label),
            text
        );

        gtk_label_set_xalign(
            GTK_LABEL(label),
            0.0
        );

        gtk_box_append(
            GTK_BOX(box),
            label
        );
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_driver_route(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *scroll;
    GtkWidget *list_box;
    GtkWidget *title;

    Driver driver;
    void *schedule_records = NULL;
    void *route_records = NULL;
    size_t schedule_count = 0;
    size_t route_count = 0;
    Schedule *schedules;
    Route *routes;
    size_t i;
    size_t j;
    int found = 0;

    window = create_module_window(
        parent,
        "Assigned Route"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Routes for Assigned Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    if (search_driver(logged_driver_id, &driver) != 1)
    {
        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Driver information could not be found.")
        );
    }
    else if (readRecords(
                 SCHEDULES_FILE,
                 sizeof(Schedule),
                 &schedule_records,
                 &schedule_count) != 0 ||
             readRecords(
                 ROUTES_FILE,
                 sizeof(Route),
                 &route_records,
                 &route_count) != 0)
    {
        free(schedule_records);
        free(route_records);

        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Route information could not be loaded.")
        );
    }
    else
    {
        schedules = (Schedule *)schedule_records;
        routes = (Route *)route_records;

        for (i = 0; i < schedule_count; i++)
        {
            if (schedules[i].busId != driver.assignedBusId)
            {
                continue;
            }

            for (j = 0; j < route_count; j++)
            {
                if (routes[j].routeId == schedules[i].routeId)
                {
                    GtkWidget *label;
                    char text[500];

                    label = gtk_label_new(NULL);

                    snprintf(
                        text,
                        sizeof(text),
                        "Route ID: %d\nRoute Name: %s\nStops: %s\nStarting Point: %s\nDestination: %s",
                        routes[j].routeId,
                        routes[j].routeName,
                        routes[j].stops,
                        routes[j].starting_point,
                        routes[j].destination
                    );

                    gtk_label_set_text(
                        GTK_LABEL(label),
                        text
                    );

                    gtk_label_set_xalign(
                        GTK_LABEL(label),
                        0.0
                    );

                    gtk_box_append(
                        GTK_BOX(list_box),
                        label
                    );

                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
            gtk_box_append(
                GTK_BOX(list_box),
                gtk_label_new("No route is assigned to this bus.")
            );
        }

        free(schedule_records);
        free(route_records);
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_driver_schedule(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *scroll;
    GtkWidget *list_box;
    GtkWidget *title;

    Driver driver;
    void *records = NULL;
    size_t count = 0;
    Schedule *schedules;
    size_t i;
    int found = 0;

    window = create_module_window(
        parent,
        "My Schedule"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Schedules for Assigned Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    if (search_driver(logged_driver_id, &driver) != 1)
    {
        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Driver information could not be found.")
        );
    }
    else if (readRecords(
                 SCHEDULES_FILE,
                 sizeof(Schedule),
                 &records,
                 &count) != 0)
    {
        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Schedule information could not be loaded.")
        );
    }
    else
    {
        schedules = (Schedule *)records;

        for (i = 0; i < count; i++)
        {
            if (schedules[i].busId == driver.assignedBusId)
            {
                GtkWidget *label;
                char text[500];

                label = gtk_label_new(NULL);

                snprintf(
                    text,
                    sizeof(text),
                    "Schedule ID: %d\nBus ID: %d\nRoute ID: %d\nStart Time: %s\nEnd Time: %s\nDay Type: %s\nDeparture Time: %s",
                    schedules[i].scheduleId,
                    schedules[i].busId,
                    schedules[i].routeId,
                    schedules[i].startTime,
                    schedules[i].endTime,
                    schedules[i].dayType,
                    schedules[i].departure_time
                );

                gtk_label_set_text(
                    GTK_LABEL(label),
                    text
                );

                gtk_label_set_xalign(
                    GTK_LABEL(label),
                    0.0
                );

                gtk_box_append(
                    GTK_BOX(list_box),
                    label
                );

                found = 1;
            }
        }

        if (!found)
        {
            gtk_box_append(
                GTK_BOX(list_box),
                gtk_label_new("No schedule is available for this bus.")
            );
        }

        free(records);
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


void show_driver_passengers(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *scroll;
    GtkWidget *list_box;
    GtkWidget *title;

    Driver driver;
    void *schedule_records = NULL;
    void *booking_records = NULL;
    void *student_records = NULL;
    size_t schedule_count = 0;
    size_t booking_count = 0;
    size_t student_count = 0;
    Schedule *schedules;
    Booking *bookings;
    Student *students;
    size_t i;
    size_t j;
    size_t k;
    int found = 0;

    window = create_module_window(
        parent,
        "Passenger Information"
    );

    box = gtk_window_get_child(
        GTK_WINDOW(window)
    );

    title = gtk_label_new(
        "Passengers on Assigned Bus"
    );

    gtk_box_append(
        GTK_BOX(box),
        title
    );

    scroll = gtk_scrolled_window_new();

    gtk_widget_set_vexpand(
        scroll,
        TRUE
    );

    gtk_box_append(
        GTK_BOX(box),
        scroll
    );

    list_box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL,
        10
    );

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(scroll),
        list_box
    );

    if (search_driver(logged_driver_id, &driver) != 1)
    {
        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Driver information could not be found.")
        );
    }
    else if (readRecords(
                 SCHEDULES_FILE,
                 sizeof(Schedule),
                 &schedule_records,
                 &schedule_count) != 0 ||
             readRecords(
                 BOOKINGS_FILE,
                 sizeof(Booking),
                 &booking_records,
                 &booking_count) != 0 ||
             readRecords(
                 STUDENTS_FILE,
                 sizeof(Student),
                 &student_records,
                 &student_count) != 0)
    {
        free(schedule_records);
        free(booking_records);
        free(student_records);

        gtk_box_append(
            GTK_BOX(list_box),
            gtk_label_new("Passenger information could not be loaded.")
        );
    }
    else
    {
        schedules = (Schedule *)schedule_records;
        bookings = (Booking *)booking_records;
        students = (Student *)student_records;

        for (i = 0; i < booking_count; i++)
        {
            int matching_schedule = 0;

            for (j = 0; j < schedule_count; j++)
            {
                if (schedules[j].scheduleId == bookings[i].scheduleId &&
                    schedules[j].busId == driver.assignedBusId)
                {
                    matching_schedule = 1;
                    break;
                }
            }

            if (!matching_schedule)
            {
                continue;
            }

            for (k = 0; k < student_count; k++)
            {
                if (students[k].studentId == bookings[i].studentId)
                {
                    GtkWidget *label;
                    char text[400];

                    label = gtk_label_new(NULL);

                    snprintf(
                        text,
                        sizeof(text),
                        "Student ID: %d\nName: %s\nSchedule ID: %d\nSeat Number: %d",
                        students[k].studentId,
                        students[k].name,
                        bookings[i].scheduleId,
                        bookings[i].seat_number
                    );

                    gtk_label_set_text(
                        GTK_LABEL(label),
                        text
                    );

                    gtk_label_set_xalign(
                        GTK_LABEL(label),
                        0.0
                    );

                    gtk_box_append(
                        GTK_BOX(list_box),
                        label
                    );

                    found = 1;
                    break;
                }
            }
        }

        if (!found)
        {
            gtk_box_append(
                GTK_BOX(list_box),
                gtk_label_new("No passengers have booked this bus yet.")
            );
        }

        free(schedule_records);
        free(booking_records);
        free(student_records);
    }

    gtk_window_present(
        GTK_WINDOW(window)
    );
}


/* =========================================================
   LOGOUT
   ========================================================= */

void logout_to_main_window(
    GtkWindow *current_window)
{
    gtk_window_destroy(
        current_window
    );
}