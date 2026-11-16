#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>


/* =========================================================
   APPLICATION
   ========================================================= */

void start_gui(GtkApplication *app);


/* =========================================================
   MAIN WINDOW
   ========================================================= */

GtkWidget *create_main_window(GtkApplication *app);


/* =========================================================
   COMMON GUI HELPERS
   ========================================================= */

void show_message(GtkWindow *parent,
                  const char *title,
                  const char *message);

GtkWidget *create_button(const char *text);

GtkWidget *create_entry(const char *placeholder,
                        gboolean password);

GtkWidget *create_module_window(GtkWindow *parent,
                                const char *title);


/* =========================================================
   ADMIN
   ========================================================= */

void show_admin_login(GtkWindow *parent);

void show_admin_dashboard(GtkWindow *parent);


/* =========================================================
   STUDENT
   ========================================================= */

void show_student_login(GtkWindow *parent);

void show_student_dashboard(GtkWindow *parent);


/* =========================================================
   DRIVER
   ========================================================= */

void show_driver_login(GtkWindow *parent);

void show_driver_dashboard(GtkWindow *parent);


/* =========================================================
   ADMIN MODULE WINDOWS
   ========================================================= */

void show_student_module(GtkWindow *parent);

void show_driver_module(GtkWindow *parent);

void show_bus_module(GtkWindow *parent);

void show_route_module(GtkWindow *parent);

void show_schedule_module(GtkWindow *parent);

void show_booking_module(GtkWindow *parent);

void show_seat_module(GtkWindow *parent);

void show_notification_module(GtkWindow *parent);

void show_report_module(GtkWindow *parent);


/* =========================================================
   STUDENT MODULE WINDOWS
   ========================================================= */

void show_student_profile(GtkWindow *parent);

void show_student_buses(GtkWindow *parent);

void show_student_routes(GtkWindow *parent);

void show_student_schedules(GtkWindow *parent);

void show_student_booking(GtkWindow *parent);

void show_student_notifications(GtkWindow *parent);


/* =========================================================
   DRIVER MODULE WINDOWS
   ========================================================= */

void show_driver_profile(GtkWindow *parent);

void show_driver_bus(GtkWindow *parent);

void show_driver_route(GtkWindow *parent);

void show_driver_schedule(GtkWindow *parent);

void show_driver_passengers(GtkWindow *parent);


/* =========================================================
   LOGOUT
   ========================================================= */

void logout_to_main_window(GtkWindow *current_window);

#endif