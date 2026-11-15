#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/driver.h"
#include "../include/file_handler.h"

static int valid_text(const char *text, size_t capacity)
{
    return text != NULL && text[0] != '\0' &&
           memchr(text, '\0', capacity) != NULL;
}

int login_driver(const char *driver_id, Driver *driver)
{
    FileStatus status;

    if (driver_id == NULL || driver == NULL || driver_id[0] == '\0' ||
        strlen(driver_id) >= sizeof(driver->driver_id))
        return 0;

    status = file_find_record(FILE_DRIVERS_PATH, driver_id, driver,
                              sizeof(*driver), offsetof(Driver, driver_id),
                              sizeof(driver->driver_id));
    if (status != FILE_STATUS_OK)
    {
        if (status != FILE_STATUS_NOT_FOUND)
            fprintf(stderr, "Could not read driver record: %s\n",
                    file_status_message(status));
        return 0;
    }

    return valid_text(driver->driver_id, sizeof(driver->driver_id)) &&
           valid_text(driver->name, sizeof(driver->name)) &&
           memchr(driver->assigned_bus_id, '\0',
                  sizeof(driver->assigned_bus_id)) != NULL;
}

static void set_driver_status(GtkWindow *window, const char *message)
{
    GtkWidget *label =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "status-label"));

    gtk_label_set_text(GTK_LABEL(label), message);
}

static void lookup_driver_clicked(GtkButton *button, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(data);
    GtkWidget *entry =
        GTK_WIDGET(g_object_get_data(G_OBJECT(window), "driver-id-entry"));
    const char *driver_id = gtk_editable_get_text(GTK_EDITABLE(entry));
    Driver driver;

    (void)button;
    if (!login_driver(driver_id, &driver))
    {
        set_driver_status(window,
                          "Driver ID was not found. Check the ID and try again.");
        return;
    }

    show_driver_dashboard(window, &driver);
}

void show_driver_login(GtkWindow *parent)
{
    GtkWidget *window =
        gtk_application_window_new(gtk_window_get_application(parent));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new("Driver interface");
    GtkWidget *note = gtk_label_new(
        "Enter your driver ID to view your assigned bus and schedules.");
    GtkWidget *id_label = gtk_label_new("Driver ID");
    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *open_button = gtk_button_new_with_label("View my schedule");
    GtkWidget *status_label = gtk_label_new("");

    gtk_window_set_title(GTK_WINDOW(window), "BusMate Driver");
    gtk_window_set_default_size(GTK_WINDOW(window), 440, 340);
    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_widget_set_margin_top(box, 28);
    gtk_widget_set_margin_bottom(box, 28);
    gtk_widget_set_margin_start(box, 32);
    gtk_widget_set_margin_end(box, 32);
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_set_halign(note, GTK_ALIGN_START);
    gtk_widget_set_halign(id_label, GTK_ALIGN_START);
    gtk_entry_set_placeholder_text(GTK_ENTRY(id_entry), "Enter driver ID");
    gtk_label_set_wrap(GTK_LABEL(note), TRUE);
    gtk_label_set_wrap(GTK_LABEL(status_label), TRUE);
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), note);
    gtk_box_append(GTK_BOX(box), id_label);
    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), open_button);
    gtk_box_append(GTK_BOX(box), status_label);
    gtk_window_set_child(GTK_WINDOW(window), box);

    g_object_set_data(G_OBJECT(window), "driver-id-entry", id_entry);
    g_object_set_data(G_OBJECT(window), "status-label", status_label);
    g_signal_connect(open_button, "clicked",
                     G_CALLBACK(lookup_driver_clicked), window);
    gtk_window_present(GTK_WINDOW(window));
}

void show_driver_dashboard(GtkWindow *window, const Driver *driver)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new("My assigned service");
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *view = gtk_text_view_new();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    GString *report = g_string_new(NULL);
    Bus bus;
    void *schedule_records = NULL;
    Schedule *schedules;
    size_t schedule_count = 0;
    size_t index;
    size_t matching_schedules = 0;
    FileStatus status;

    gtk_window_set_title(window, "BusMate Driver Schedule");
    gtk_window_set_default_size(window, 620, 480);
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);

    g_string_append_printf(report, "Driver: %s (%s)\n",
                           driver->name, driver->driver_id);
    if (driver->assigned_bus_id[0] == '\0')
        g_string_append(report, "No bus is assigned to this driver.\n");
    else
    {
        status = file_find_record(FILE_BUSES_PATH, driver->assigned_bus_id,
                                  &bus, sizeof(bus), offsetof(Bus, bus_id),
                                  sizeof(bus.bus_id));
        if (status == FILE_STATUS_OK &&
            memchr(bus.bus_number, '\0', sizeof(bus.bus_number)) != NULL)
            g_string_append_printf(report, "Bus: %s (%s), %d seats\n",
                                   bus.bus_number, bus.bus_id, bus.capacity);
        else
            g_string_append_printf(report, "Assigned bus: %s\n",
                                   driver->assigned_bus_id);
    }

    g_string_append(report, "\nSchedules\n");
    status = file_read_records(FILE_SCHEDULES_PATH, &schedule_records,
                               sizeof(Schedule), &schedule_count);
    if (status == FILE_STATUS_NOT_FOUND)
        g_string_append(report, "No schedules have been added.");
    else if (status != FILE_STATUS_OK)
        g_string_append_printf(report, "Could not read schedules: %s.",
                               file_status_message(status));
    else if (driver->assigned_bus_id[0] == '\0')
        g_string_append(report, "A bus assignment is required to list schedules.");
    else
    {
        schedules = (Schedule *)schedule_records;
        for (index = 0; index < schedule_count; ++index)
        {
            if (memchr(schedules[index].bus_id, '\0',
                       sizeof(schedules[index].bus_id)) == NULL)
            {
                g_string_append_printf(report,
                                       "Schedule record %lu is corrupt.\n",
                                       (unsigned long)(index + 1));
                continue;
            }
            if (strcmp(schedules[index].bus_id, driver->assigned_bus_id) != 0)
                continue;
            if (memchr(schedules[index].schedule_id, '\0',
                       sizeof(schedules[index].schedule_id)) == NULL ||
                memchr(schedules[index].route_id, '\0',
                       sizeof(schedules[index].route_id)) == NULL ||
                memchr(schedules[index].departure_time, '\0',
                       sizeof(schedules[index].departure_time)) == NULL)
            {
                g_string_append_printf(report,
                                       "Schedule record %lu is corrupt.\n",
                                       (unsigned long)(index + 1));
                continue;
            }
            g_string_append_printf(report, "%s  |  route %s  |  %s\n",
                                   schedules[index].schedule_id,
                                   schedules[index].route_id,
                                   schedules[index].departure_time);
            ++matching_schedules;
        }
        if (matching_schedules == 0)
            g_string_append(report,
                            "No schedules are assigned to this bus.");
    }

    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_buffer_set_text(buffer, report->str, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), view);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_append(GTK_BOX(box), title);
    gtk_box_append(GTK_BOX(box), scroller);
    gtk_window_set_child(window, box);
    gtk_window_present(window);

    g_string_free(report, TRUE);
    free(schedule_records);
}