#include "gui.h"
#include "admin.h"

int main(int argc, char *argv[])
{
    ensure_data_files();
    start_gui(argc, argv);

    return 0;
}