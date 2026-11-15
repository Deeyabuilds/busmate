
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"
#include "structures.h"
#include "file_handler.h"

/* ---------- MESSAGE ---------- */

static void show_message(GtkWindow *parent, const char *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(
        parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s",
        message
    );

    g_signal_connect_swapped(
        dialog, "response",
        G_CALLBACK(gtk_window_destroy),
        dialog
    );

    gtk_window_present(GTK_WINDOW(dialog));
}

/* ---------- REGISTRATION ---------- */

int register_student(Student student)
{
    if (strlen(student.student_id) == 0 ||
        strlen(student.name) == 0 ||
        strlen(student.password) == 0)
    {
        return 0;
    }

    saveStudent(student);
    return 1;
}

/* ---------- LOGIN ---------- */

int login_student(const char *student_id, const char *password)
{
    /*
       The supplied file_handler.h does not contain
       a student authentication function.

       Login verification cannot be performed using
       the currently supplied backend functions.
    */

    (void)student_id;
    (void)password;

    return 0;
}

/* ---------- REGISTRATION CALLBACK ---------- */

static void on_register_response(GtkDialog *dialog,
                                 int response,
                                 gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT)
    {
        GtkWidget *id_entry =
            g_object_get_data(G_OBJECT(dialog), "id");

        GtkWidget *name_entry =
            g_object_get_data(G_OBJECT(dialog), "name");

        GtkWidget *password_entry =
            g_object_get_data(G_OBJECT(dialog), "password");

        Student student = {0};

        snprintf(student.student_id,
                 sizeof(student.student_id), "%s",
                 gtk_editable_get_text(GTK_EDITABLE(id_entry)));

        snprintf(student.name,
                 sizeof(student.name), "%s",
                 gtk_editable_get_text(GTK_EDITABLE(name_entry)));

        snprintf(student.password,
                 sizeof(student.password), "%s",
                 gtk_editable_get_text(GTK_EDITABLE(password_entry)));

        if (register_student(student))
        {
            show_message(GTK_WINDOW(data),
                         "Registration request submitted.");
        }
        else
        {
            show_message(GTK_WINDOW(data),
                         "Please fill all the fields.");
        }
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}

/* ---------- REGISTRATION WINDOW ---------- */

static void show_registration(GtkWindow *parent)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Student Registration",
        parent,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Register", GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkWidget *content =
        gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(id_entry), "College ID");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(name_entry), "Student Name");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(password_entry), "Password");

    gtk_entry_set_visibility(
        GTK_ENTRY(password_entry), FALSE);

    gtk_box_append(GTK_BOX(content), id_entry);
    gtk_box_append(GTK_BOX(content), name_entry);
    gtk_box_append(GTK_BOX(content), password_entry);

    g_object_set_data(G_OBJECT(dialog), "id", id_entry);
    g_object_set_data(G_OBJECT(dialog), "name", name_entry);
    g_object_set_data(G_OBJECT(dialog), "password", password_entry);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_register_response), parent);

    gtk_window_present(GTK_WINDOW(dialog));
}

/* ---------- VIEW BUSES ---------- */

static void on_view_buses(GtkButton *button, gpointer data)
{
    viewBuses();

    show_message(GTK_WINDOW(data),
                 "Bus information requested. Check backend output.");
}

/* ---------- VIEW ROUTES ---------- */

static void on_view_routes(GtkButton *button, gpointer data)
{
    viewRoutes();

    show_message(GTK_WINDOW(data),
                 "Route information requested. Check backend output.");
}

/* ---------- VIEW SCHEDULES ---------- */

static void on_view_schedules(GtkButton *button, gpointer data)
{
    viewSchedules();

    show_message(GTK_WINDOW(data),
                 "Schedule information requested. Check backend output.");
}

/* ---------- VIEW BOOKINGS ---------- */

static void on_view_bookings(GtkButton *button, gpointer data)
{
    viewBookings();

    show_message(GTK_WINDOW(data),
                 "Booking information requested. Check backend output.");
}

/* ---------- BOOKING CALLBACK ---------- */

static void on_booking_response(GtkDialog *dialog,
                                int response,
                                gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT)
    {
        GtkWidget *id_entry =
            g_object_get_data(G_OBJECT(dialog), "student_id");

        GtkWidget *schedule_entry =
            g_object_get_data(G_OBJECT(dialog), "schedule_id");

        GtkWidget *seat_entry =
            g_object_get_data(G_OBJECT(dialog), "seat_number");

        const char *student_id =
            gtk_editable_get_text(GTK_EDITABLE(id_entry));

        const char *schedule_id =
            gtk_editable_get_text(GTK_EDITABLE(schedule_entry));

        const char *seat_text =
            gtk_editable_get_text(GTK_EDITABLE(seat_entry));

        char *end;
        long seat_number = strtol(seat_text, &end, 10);

        if (strlen(student_id) == 0 ||
            strlen(schedule_id) == 0 ||
            strlen(seat_text) == 0 ||
            *end != '\0' ||
            seat_number <= 0 ||
            seat_number > 2147483647)
        {
            show_message(GTK_WINDOW(data),
                         "Enter valid booking details.");
        }
        else
        {
            Booking booking = {0};

            snprintf(booking.student_id,
                     sizeof(booking.student_id),
                     "%s", student_id);

            snprintf(booking.schedule_id,
                     sizeof(booking.schedule_id),
                     "%s", schedule_id);

            booking.seat_number = (int)seat_number;

            saveBooking(booking);

            show_message(GTK_WINDOW(data),
                         "Booking request submitted.");
        }
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}

/* ---------- BOOK SEAT WINDOW ---------- */

static void on_book_seat(GtkButton *button, gpointer data)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Book Seat",
        GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Book", GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkWidget *content =
        gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *schedule_entry = gtk_entry_new();
    GtkWidget *seat_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(id_entry), "Student ID");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(schedule_entry), "Schedule ID");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(seat_entry), "Seat Number");

    gtk_box_append(GTK_BOX(content), id_entry);
    gtk_box_append(GTK_BOX(content), schedule_entry);
    gtk_box_append(GTK_BOX(content), seat_entry);

    g_object_set_data(G_OBJECT(dialog), "student_id", id_entry);
    g_object_set_data(G_OBJECT(dialog), "schedule_id", schedule_entry);
    g_object_set_data(G_OBJECT(dialog), "seat_number", seat_entry);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_booking_response), data);

    gtk_window_present(GTK_WINDOW(dialog));
}

/* ---------- CANCEL BOOKING CALLBACK ---------- */

static void on_cancel_response(GtkDialog *dialog,
                               int response,
                               gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT)
    {
        GtkWidget *id_entry =
            g_object_get_data(G_OBJECT(dialog), "student_id");

        GtkWidget *schedule_entry =
            g_object_get_data(G_OBJECT(dialog), "schedule_id");

        const char *student_id =
            gtk_editable_get_text(GTK_EDITABLE(id_entry));

        const char *schedule_id =
            gtk_editable_get_text(GTK_EDITABLE(schedule_entry));

        if (strlen(student_id) == 0 ||
            strlen(schedule_id) == 0)
        {
            show_message(GTK_WINDOW(data),
                         "Please enter all details.");
        }
        else
        {
            deleteBooking((char *)student_id,
                          (char *)schedule_id);

            show_message(GTK_WINDOW(data),
                         "Cancellation request submitted.");
        }
    }

    gtk_window_destroy(GTK_WINDOW(dialog));
}

/* ---------- CANCEL BOOKING WINDOW ---------- */

static void on_cancel_booking(GtkButton *button, gpointer data)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Cancel Booking",
        GTK_WINDOW(data),
        GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CANCEL,
        "_Cancel Booking", GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkWidget *content =
        gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *schedule_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(id_entry), "Student ID");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(schedule_entry), "Schedule ID");

    gtk_box_append(GTK_BOX(content), id_entry);
    gtk_box_append(GTK_BOX(content), schedule_entry);

    g_object_set_data(G_OBJECT(dialog), "student_id", id_entry);
    g_object_set_data(G_OBJECT(dialog), "schedule_id", schedule_entry);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_cancel_response), data);

    gtk_window_present(GTK_WINDOW(dialog));
}

/* ---------- DASHBOARD ---------- */

void show_student_dashboard(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();

    gtk_window_set_title(GTK_WINDOW(window),
                         "Student Dashboard");

    gtk_window_set_default_size(GTK_WINDOW(window),
                                450, 500);

    if (parent != NULL)
    {
        gtk_window_set_transient_for(
            GTK_WINDOW(window), parent);
    }

    GtkWidget *box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 12);

    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);

    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box),
        gtk_label_new("BUSMATE STUDENT DASHBOARD"));

    const char *labels[] = {
        "View Available Buses",
        "View Routes",
        "View Schedules",
        "Book Seat",
        "My Bookings",
        "Cancel Booking"
    };

    GCallback callbacks[] = {
        G_CALLBACK(on_view_buses),
        G_CALLBACK(on_view_routes),
        G_CALLBACK(on_view_schedules),
        G_CALLBACK(on_book_seat),
        G_CALLBACK(on_view_bookings),
        G_CALLBACK(on_cancel_booking)
    };

    for (int i = 0; i < 6; i++)
    {
        GtkWidget *button =
            gtk_button_new_with_label(labels[i]);

        gtk_box_append(GTK_BOX(box), button);

        g_signal_connect(button, "clicked",
                         callbacks[i], window);
    }

    gtk_window_present(GTK_WINDOW(window));
}

/* ---------- LOGIN CALLBACK ---------- */

static void on_login_clicked(GtkButton *button, gpointer data)
{
    show_message(GTK_WINDOW(data),
                 "Login verification needs backend support.");
}

/* ---------- REGISTER BUTTON CALLBACK ---------- */

static void on_register_clicked(GtkButton *button, gpointer data)
{
    show_registration(GTK_WINDOW(data));
}

/* ---------- LOGIN WINDOW ---------- */

void show_student_login(GtkWindow *parent)
{
    GtkWidget *window = gtk_window_new();

    gtk_window_set_title(GTK_WINDOW(window),
                         "Student Login");

    gtk_window_set_default_size(GTK_WINDOW(window),
                                350, 250);

    if (parent != NULL)
    {
        gtk_window_set_transient_for(
            GTK_WINDOW(window), parent);
    }

    GtkWidget *box = gtk_box_new(
        GTK_ORIENTATION_VERTICAL, 12);

    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);

    gtk_window_set_child(GTK_WINDOW(window), box);

    gtk_box_append(GTK_BOX(box),
                   gtk_label_new("STUDENT LOGIN"));

    GtkWidget *id_entry = gtk_entry_new();
    GtkWidget *password_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(id_entry), "College ID");

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(password_entry), "Password");

    gtk_entry_set_visibility(
        GTK_ENTRY(password_entry), FALSE);

    gtk_box_append(GTK_BOX(box), id_entry);
    gtk_box_append(GTK_BOX(box), password_entry);

    GtkWidget *login_button =
        gtk_button_new_with_label("Login");

    GtkWidget *register_button =
        gtk_button_new_with_label("Register");

    gtk_box_append(GTK_BOX(box), login_button);
    gtk_box_append(GTK_BOX(box), register_button);

    g_signal_connect(login_button, "clicked",
                     G_CALLBACK(on_login_clicked), window);

    g_signal_connect(register_button, "clicked",
                     G_CALLBACK(on_register_clicked), window);

    gtk_window_present(GTK_WINDOW(window));
}
