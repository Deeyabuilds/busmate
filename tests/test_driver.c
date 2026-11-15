#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "../include/driver.h"
#include "../include/file_handler.h"

int main(void)
{
    Driver driver = {0};
    Driver found = {0};

    strcpy(driver.driver_id, "DRIVER-TEST");
    strcpy(driver.name, "Test Driver");
    strcpy(driver.assigned_bus_id, "BUS-TEST");

    assert(file_save_record(FILE_DRIVERS_PATH, &driver, sizeof(driver),
                            offsetof(Driver, driver_id),
                            sizeof(driver.driver_id)) == FILE_STATUS_OK);
    assert(login_driver("DRIVER-TEST", &found));
    assert(strcmp(found.name, "Test Driver") == 0);
    assert(strcmp(found.assigned_bus_id, "BUS-TEST") == 0);
    assert(!login_driver("UNKNOWN", &found));
    assert(!login_driver("", &found));

    puts("Driver lookup tests passed.");
    return 0;
}
