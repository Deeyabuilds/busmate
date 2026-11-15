#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/bus.h"
#include "../include/file_handler.h"

int add_bus(Bus bus)
{
    if (bus.bus_id[0] == '\0' || bus.bus_number[0] == '\0' ||
        bus.capacity <= 0)
        return 0;
    return file_save_record(FILE_BUSES_PATH, &bus, sizeof(bus),
                            offsetof(Bus, bus_id), sizeof(bus.bus_id)) ==
           FILE_STATUS_OK;
}

int view_buses(void)
{
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(FILE_BUSES_PATH, &records,
                                          sizeof(Bus), &count);
    if (status == FILE_STATUS_NOT_FOUND)
        return 1;
    if (status != FILE_STATUS_OK)
        return 0;

    for (index = 0; index < count; ++index)
    {
        Bus *buses = (Bus *)records;
        if (buses[index].capacity <= 0)
        {
            free(records);
            return 0;
        }
        printf("Bus %s | %s | %d seats\n", buses[index].bus_id,
               buses[index].bus_number, buses[index].capacity);
    }
    free(records);
    return 1;
}

int update_bus(Bus bus)
{
    if (bus.bus_id[0] == '\0' || bus.bus_number[0] == '\0' ||
        bus.capacity <= 0)
        return 0;
    return file_update_record(FILE_BUSES_PATH, &bus, sizeof(bus),
                              offsetof(Bus, bus_id), sizeof(bus.bus_id)) ==
           FILE_STATUS_OK;
}

int delete_bus(const char *bus_id)
{
    return file_delete_record(FILE_BUSES_PATH, bus_id, sizeof(Bus),
                              offsetof(Bus, bus_id),
                              sizeof(((Bus *)0)->bus_id)) == FILE_STATUS_OK;
}