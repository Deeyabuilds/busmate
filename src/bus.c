#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../include/bus.h"
#include "../include/file_handler.h"


/* =========================================================
   ADD BUS
   ========================================================= */

int add_bus(Bus bus)
{
    void *records;
    Bus *buses;
    size_t count;
    size_t i;

    records = NULL;
    count = 0;

    if (bus.busId <= 0)
    {
        return -1;
    }

    if (bus.busNumber[0] == '\0')
    {
        return -1;
    }

    if (bus.capacity <= 0)
    {
        return -1;
    }

    if (bus.availableSeats < 0 ||
        bus.availableSeats > bus.capacity)
    {
        return -1;
    }

    if (readRecords(
            BUSES_FILE,
            sizeof(Bus),
            &records,
            &count) == 0)
    {
        buses = (Bus *)records;

        for (i = 0; i < count; i++)
        {
            if (buses[i].busId == bus.busId)
            {
                free(records);
                return -2;
            }
        }

        free(records);
    }

    if (appendRecord(
            BUSES_FILE,
            sizeof(Bus),
            &bus) != 0)
    {
        return -1;
    }

    return 1;
}


/* =========================================================
   SEARCH BUS
   ========================================================= */

int search_bus(int busId,
               Bus *bus)
{
    void *records;
    Bus *buses;
    size_t count;
    size_t i;

    if (bus == NULL)
    {
        return -1;
    }

    records = NULL;
    count = 0;

    if (readRecords(
            BUSES_FILE,
            sizeof(Bus),
            &records,
            &count) != 0)
    {
        return -1;
    }

    buses = (Bus *)records;

    for (i = 0; i < count; i++)
    {
        if (buses[i].busId == busId)
        {
            *bus = buses[i];

            free(records);

            return 1;
        }
    }

    free(records);

    return 0;
}


/* =========================================================
   UPDATE BUS
   ========================================================= */

int update_bus(Bus bus)
{
    int updated;

    updated = 0;

    if (bus.busId <= 0 ||
        bus.busNumber[0] == '\0' ||
        bus.capacity <= 0 ||
        bus.availableSeats < 0 ||
        bus.availableSeats > bus.capacity)
    {
        return -1;
    }

    if (updateRecordById(
            BUSES_FILE,
            sizeof(Bus),
            offsetof(Bus, busId),
            bus.busId,
            &bus,
            &updated) != 0)
    {
        return -1;
    }

    if (updated)
    {
        return 1;
    }

    return 0;
}


/* =========================================================
   DELETE BUS
   ========================================================= */

int delete_bus(int busId)
{
    int deleted;

    deleted = 0;

    if (busId <= 0)
    {
        return -1;
    }

    if (deleteRecordById(
            BUSES_FILE,
            sizeof(Bus),
            offsetof(Bus, busId),
            busId,
            &deleted) != 0)
    {
        return -1;
    }

    if (deleted)
    {
        return 1;
    }

    return 0;
}
