#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

void activate(GtkApplication *app, gpointer user_data);
void show_seat_allocation_dashboard(GtkWindow *parent);
void show_seat_allocation_window(GtkWindow *parent,
                                 const char *logged_in_student_id);

#endif