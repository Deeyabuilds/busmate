#ifndef DRIVER_H
#define DRIVER_H

#include <gtk/gtk.h>
#include "structures.h"

void show_driver_login(GtkWindow *parent);
void show_driver_dashboard(GtkWindow *parent, const Driver *driver);

int login_driver(const char *driver_id, Driver *driver);

#endif