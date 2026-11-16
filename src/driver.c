#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/driver.h"
#include "../include/file_handler.h"

int add_driver(Driver driver)
{
    void *records;
    size_t count;
    size_t i;
    Driver *drivers;

    records = NULL;
    count = 0;

    if (driver.driverId <= 0 || driver.name[0] == '\0' ||
        driver.phone[0] == '\0' || driver.assignedBusId <= 0)
    {
        return -1;
    }

    if (readRecords(DRIVERS_FILE, sizeof(Driver), &records, &count) != 0)
    {
        free(records);
        return -1;
    }

    drivers = (Driver *)records;

    for (i = 0; i < count; i++)
    {
        if (drivers[i].driverId == driver.driverId)
        {
            free(records);
            return -2;
        }
    }

    free(records);

    if (appendRecord(DRIVERS_FILE, sizeof(Driver), &driver) != 0)
    {
        return -1;
    }

    return 1;
}

int search_driver(int driverId, Driver *driver)
{
    void *records;
    size_t count;
    size_t i;
    Driver *drivers;

    records = NULL;
    count = 0;

    if (driver == NULL || driverId <= 0)
    {
        return 0;
    }

    if (readRecords(DRIVERS_FILE, sizeof(Driver), &records, &count) != 0)
    {
        return 0;
    }

    drivers = (Driver *)records;

    for (i = 0; i < count; i++)
    {
        if (drivers[i].driverId == driverId)
        {
            *driver = drivers[i];
            free(records);
            return 1;
        }
    }

    free(records);
    return 0;
}

int update_driver(Driver driver)
{
    int updated;

    updated = 0;

    if (driver.driverId <= 0 || driver.name[0] == '\0' ||
        driver.phone[0] == '\0' || driver.assignedBusId <= 0)
    {
        return -1;
    }

    if (updateRecordById(
            DRIVERS_FILE,
            sizeof(Driver),
            offsetof(Driver, driverId),
            driver.driverId,
            &driver,
            &updated) != 0)
    {
        return -1;
    }

    if (updated == 1)
    {
        return 1;
    }

    return 0;
}

int delete_driver(int driverId)
{
    int deleted;

    deleted = 0;

    if (driverId <= 0)
    {
        return -1;
    }

    if (deleteRecordById(
            DRIVERS_FILE,
            sizeof(Driver),
            offsetof(Driver, driverId),
            driverId,
            &deleted) != 0)
    {
        return -1;
    }

    if (deleted == 1)
    {
        return 1;
    }

    return 0;
}
