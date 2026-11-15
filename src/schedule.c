
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/file_handler.h"
#include "../include/schedule.h"

int add_schedule(Schedule schedule)
{
    if (schedule.scheduleId <= 0 ||
        schedule.busId <= 0 ||
        schedule.routeId <= 0)
        return 0;

    return appendRecord(SCHEDULES_FILE,
                        sizeof(Schedule), &schedule) == 0;
}

int view_schedules(void)
{
    Schedule *schedules = NULL;
    size_t count = 0;
    size_t i;

    if (readRecords(SCHEDULES_FILE, sizeof(Schedule),
                    (void **)&schedules, &count) != 0)
        return 0;

    for (i = 0; i < count; i++)
    {
        printf("Schedule ID: %d | Bus ID: %d | Route ID: %d | "
               "Departure: %s | Start: %s | End: %s | Day: %s\n",
               schedules[i].scheduleId,
               schedules[i].busId,
               schedules[i].routeId,
               schedules[i].departure_time,
               schedules[i].startTime,
               schedules[i].endTime,
               schedules[i].dayType);
    }

    free(schedules);
    return 1;
}

int update_schedule(Schedule schedule)
{
    int updated = 0;

    if (schedule.scheduleId <= 0)
        return 0;

    if (updateRecordById(SCHEDULES_FILE, sizeof(Schedule),
                         offsetof(Schedule, scheduleId),
                         schedule.scheduleId,
                         &schedule, &updated) != 0)
        return 0;

    return updated;
}

int delete_schedule(int schedule_id)
{
    int deleted = 0;

    if (schedule_id <= 0)
        return 0;

    if (deleteRecordById(SCHEDULES_FILE, sizeof(Schedule),
                         offsetof(Schedule, scheduleId),
                         schedule_id, &deleted) != 0)
        return 0;

    return deleted;
}
