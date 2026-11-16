#include <gtk/gtk.h>

#include "../include/gui.h"
#include "../include/file_handler.h"


static void activate(GtkApplication *app,
                     gpointer user_data)
{
    initialize_default_data();

    start_gui(app);
}


int main(int argc, char *argv[])
{
    GtkApplication *app;
    int status;

    app = gtk_application_new(
        "com.busmate.app",
        G_APPLICATION_DEFAULT_FLAGS
    );

    g_signal_connect(
        app,
        "activate",
        G_CALLBACK(activate),
        NULL
    );

    status = g_application_run(
        G_APPLICATION(app),
        argc,
        argv
    );

    g_object_unref(app);

    return status;
}