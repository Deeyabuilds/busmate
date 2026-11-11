#include <gtk/gtk.h>
#include <stdlib.h>
#include "../include/seat_manager.h"

static Seat global_seats[TOTAL_SEATS];
static GtkWidget *seat_grid_widget = NULL;

// Seat button click callback
static void on_seat_button_clicked(GtkWidget *widget, gpointer user_data)
{
    (void)widget;
    int seat_num = GPOINTER_TO_INT(user_data);
    int idx = seat_num - 1;

    char msg[128];
    if (global_seats[idx].status == SEAT_BOOKED)
    {
        snprintf(msg, sizeof(msg), "Seat %d: Booked by %s (Age: %d)",
                 seat_num, global_seats[idx].passenger, global_seats[idx].age);
    }
    else
    {
        snprintf(msg, sizeof(msg), "Seat %d (%s) is Available!",
                 seat_num, global_seats[idx].type);
    }

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Seat GUI Window Creation Function
GtkWidget *create_seat_gui_window(GtkApplication *app)
{
    load_seats_from_file("seats_data.dat", global_seats, TOTAL_SEATS);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "BusMate - Seat Allocation GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<big><b>BusMate Seat Allocation Grid</b></big>");
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    seat_grid_widget = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(seat_grid_widget), 8);
    gtk_grid_set_column_spacing(GTK_GRID(seat_grid_widget), 8);

    gtk_box_pack_start(GTK_BOX(vbox), seat_grid_widget, TRUE, TRUE, 5);

    update_seat_ui_grid(seat_grid_widget, global_seats, TOTAL_SEATS, G_CALLBACK(on_seat_button_clicked));

    return window;
}