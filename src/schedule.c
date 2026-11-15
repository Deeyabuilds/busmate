#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/file_handler.h"
#include "../include/schedule.h"

int add_schedule(Schedule schedule)
{
    if (schedule.schedule_id[0] == '\0' || schedule.bus_id[0] == '\0')
        return 0;
    return file_save_record(FILE_SCHEDULES_PATH, &schedule, sizeof(schedule),
                            offsetof(Schedule, schedule_id),
                            sizeof(schedule.schedule_id)) == FILE_STATUS_OK;
}

int view_schedules(void)
{
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(FILE_SCHEDULES_PATH, &records,
                                          sizeof(Schedule), &count);
    if (status == FILE_STATUS_NOT_FOUND)
        return 1;
    if (status != FILE_STATUS_OK)
        return 0;

    for (index = 0; index < count; ++index)
    {
        Schedule *schedules = (Schedule *)records;
        printf("Schedule %s | bus %s | route %s | %s\n",
               schedules[index].schedule_id, schedules[index].bus_id,
               schedules[index].route_id, schedules[index].departure_time);
    }
    free(records);
    return 1;
}

int update_schedule(Schedule schedule)
{
    return file_update_record(FILE_SCHEDULES_PATH, &schedule, sizeof(schedule),
                              offsetof(Schedule, schedule_id),
                              sizeof(schedule.schedule_id)) == FILE_STATUS_OK;
}

int delete_schedule(const char *schedule_id)
{
    return file_delete_record(FILE_SCHEDULES_PATH, schedule_id,
                              sizeof(Schedule),
                              offsetof(Schedule, schedule_id),
                              sizeof(((Schedule *)0)->schedule_id)) ==
           FILE_STATUS_OK;
}