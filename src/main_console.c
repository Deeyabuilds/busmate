#include <stdio.h>
#include "../include/admin.h"

int main(void)
{
    ensure_data_files();
    return run_busmate_system();
}
