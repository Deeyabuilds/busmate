
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../include/admin.h"
#include "../include/file_handler.h"
#include "../include/gui.h"

int admin_login(const char *username, const char *password)
{
    return g_strcmp0(username, "admin") == 0 &&
           g_strcmp0(password, "admin123") == 0;
}

static void show_admin_report(GtkWindow *parent, const char *title,
                              const char *text)
{
    GtkWidget *window =
        gtk_application_window_new(gtk_window_get_application(parent));
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *view = gtk_text_view_new();

    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)), text, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), view);
    gtk_window_set_child(GTK_WINDOW(window), scroller);
    gtk_window_present(GTK_WINDOW(window));
}

static void view_students_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    Student *students;
    void *records = NULL;
    size_t count = 0;
    size_t index;
    GString *report = g_string_new("Student records\n\n");
    FileStatus status;

    (void)button;
    status = file_read_records(FILE_STUDENTS_PATH, &records, sizeof(Student),
                               &count);
    if (status == FILE_STATUS_NOT_FOUND)
        g_string_append(report, "No students have been added.");
    else if (status != FILE_STATUS_OK)
        g_string_append_printf(report, "Could not read students: %s.",
                               file_status_message(status));
    else
    {
        students = (Student *)records;
        for (index = 0; index < count; ++index)
        {
            if (memchr(students[index].student_id, '\0',
                       sizeof(students[index].student_id)) == NULL ||
                memchr(students[index].name, '\0',
                       sizeof(students[index].name)) == NULL)
            {
                g_string_append_printf(report, "Record %lu is corrupt.\n",
                                       (unsigned long)(index + 1));
                continue;
            }
            g_string_append_printf(report, "%s  |  %s\n",
                                   students[index].student_id,
                                   students[index].name);
        }
    }

    show_admin_report(window, "Student Records", report->str);
    g_string_free(report, TRUE);
    free(records);
}

static void view_buses_and_schedules_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    Bus *buses;
    Schedule *schedules;
    void *bus_records = NULL;
    void *schedule_records = NULL;
    size_t bus_count = 0;
    size_t schedule_count = 0;
    size_t index;
    GString *report = g_string_new("Buses and schedules\n\nBuses\n");
    FileStatus bus_status;
    FileStatus schedule_status;

    (void)button;
    bus_status = file_read_records(FILE_BUSES_PATH, &bus_records, sizeof(Bus),
                                   &bus_count);
    if (bus_status == FILE_STATUS_NOT_FOUND)
        g_string_append(report, "No buses have been added.\n");
    else if (bus_status != FILE_STATUS_OK)
        g_string_append_printf(report, "Could not read buses: %s.\n",
                               file_status_message(bus_status));
    else
    {
        buses = (Bus *)bus_records;
        for (index = 0; index < bus_count; ++index)
        {
            if (memchr(buses[index].bus_id, '\0',
                       sizeof(buses[index].bus_id)) == NULL ||
                memchr(buses[index].bus_number, '\0',
                       sizeof(buses[index].bus_number)) == NULL)
            {
                g_string_append_printf(report, "Bus record %lu is corrupt.\n",
                                       (unsigned long)(index + 1));
                continue;
            }
            g_string_append_printf(report, "%s  |  %s  |  %d seats\n",
                                   buses[index].bus_id,
                                   buses[index].bus_number,
                                   buses[index].capacity);
        }
    }

    g_string_append(report, "\nSchedules\n");
    schedule_status = file_read_records(FILE_SCHEDULES_PATH, &schedule_records,
                                        sizeof(Schedule), &schedule_count);
    if (schedule_status == FILE_STATUS_NOT_FOUND)
        g_string_append(report, "No schedules have been added.");
    else if (schedule_status != FILE_STATUS_OK)
        g_string_append_printf(report, "Could not read schedules: %s.",
                               file_status_message(schedule_status));
    else
    {
        schedules = (Schedule *)schedule_records;
        for (index = 0; index < schedule_count; ++index)
        {
            if (memchr(schedules[index].schedule_id, '\0',
                       sizeof(schedules[index].schedule_id)) == NULL ||
                memchr(schedules[index].bus_id, '\0',
                       sizeof(schedules[index].bus_id)) == NULL ||
                memchr(schedules[index].route_id, '\0',
                       sizeof(schedules[index].route_id)) == NULL ||
                memchr(schedules[index].departure_time, '\0',
                       sizeof(schedules[index].departure_time)) == NULL)
            {
                g_string_append_printf(
                    report, "Schedule record %lu is corrupt.\n",
                    (unsigned long)(index + 1));
                continue;
            }
            g_string_append_printf(report, "%s  |  bus %s  |  route %s  |  %s\n",
                                   schedules[index].schedule_id,
                                   schedules[index].bus_id,
                                   schedules[index].route_id,
                                   schedules[index].departure_time);
        }
    }

    show_admin_report(window, "Buses and Schedules", report->str);
    g_string_free(report, TRUE);
    free(bus_records);
    free(schedule_records);
}

static void seat_allocation_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    show_seat_allocation_window(GTK_WINDOW(data), NULL);
}

void show_admin_dashboard(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *note;
    GtkWidget *seat_button;
    GtkWidget *students_button;
    GtkWidget *schedules_button;

    window = gtk_application_window_new(gtk_window_get_application(parent));
    gtk_window_set_title(GTK_WINDOW(window), "BusMate Admin");
    gtk_window_set_default_size(GTK_WINDOW(window), 620, 480);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_top(box, 28);
    gtk_widget_set_margin_bottom(box, 28);
    gtk_widget_set_margin_start(box, 32);
    gtk_widget_set_margin_end(box, 32);

    title = gtk_label_new("Admin dashboard");
    gtk_widget_add_css_class(title, "title-1");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    note = gtk_label_new("Choose an administration task.");
    gtk_widget_set_halign(note, GTK_ALIGN_START);
    seat_button = gtk_button_new_with_label("Seat allocation and waitlist");
    students_button = gtk_button_new_with_label("View student records");
    schedules_button = gtk_button_new_with_label("View buses and schedules");
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), note);
    gtk_box_append(GTK_BOX(box), seat_button);
    gtk_box_append(GTK_BOX(box), students_button);
    gtk_box_append(GTK_BOX(box), schedules_button);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_signal_connect(seat_button, "clicked",
                     G_CALLBACK(seat_allocation_clicked), window);
    g_signal_connect(students_button, "clicked",
                     G_CALLBACK(view_students_clicked), window);
    g_signal_connect(schedules_button, "clicked",
                     G_CALLBACK(view_buses_and_schedules_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

static void admin_login_clicked(GtkWidget *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *status_label;

    (void)button;
    username_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "username_entry"));
    password_entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "password_entry"));
    status_label =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "status-label"));

    {
        const char *username =
            gtk_editable_get_text(GTK_EDITABLE(username_entry));
        const char *password =
            gtk_editable_get_text(GTK_EDITABLE(password_entry));

        if (admin_login(username, password))
        {
            show_admin_dashboard(window);
            gtk_window_destroy(window);
        }
        else
            gtk_label_set_text(GTK_LABEL(status_label),
                               "Invalid username or password.");
    }
}

void show_admin_login(GtkWindow *parent)
{
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *title;
    GtkWidget *username_label;
    GtkWidget *password_label;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;
    GtkWidget *back_button;
    GtkWidget *status_label;

    window = gtk_application_window_new(gtk_window_get_application(parent));

    gtk_window_set_title(GTK_WINDOW(window), "Admin Login");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 350);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 30);
    gtk_widget_set_margin_start(box, 35);
    gtk_widget_set_margin_end(box, 35);

    title = gtk_label_new("Admin Login");
    gtk_widget_add_css_class(title, "title-2");

    username_label = gtk_label_new("Username");
    gtk_widget_set_halign(username_label, GTK_ALIGN_START);

    username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(username_entry), "Enter username"
    );

    password_label = gtk_label_new("Password");
    gtk_widget_set_halign(password_label, GTK_ALIGN_START);

    password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(password_entry), "Enter password"
    );
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);

    login_button = gtk_button_new_with_label("Login");
    back_button = gtk_button_new_with_label("Back");
    status_label = gtk_label_new("");
    gtk_label_set_wrap(GTK_LABEL(status_label), TRUE);

    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), username_label);
    gtk_box_append(GTK_BOX(box), username_entry);
    gtk_box_append(GTK_BOX(box), password_label);
    gtk_box_append(GTK_BOX(box), password_entry);
    gtk_box_append(GTK_BOX(box), login_button);
    gtk_box_append(GTK_BOX(box), status_label);
    gtk_box_append(GTK_BOX(box), back_button);

    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(
        G_OBJECT(window), "username_entry", username_entry
    );

    g_object_set_data(
        G_OBJECT(window), "password_entry", password_entry
    );
    g_object_set_data(G_OBJECT(window), "status-label", status_label);

    g_signal_connect(
        login_button,
        "clicked",
        G_CALLBACK(admin_login_clicked),
        window
    );

    g_signal_connect_swapped(
        back_button,
        "clicked",
        G_CALLBACK(gtk_window_destroy),
        window
    );

    gtk_window_present(GTK_WINDOW(window));
}
