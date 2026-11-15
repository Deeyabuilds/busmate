#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file_handler.h"

FILE *openDataFile(const char *path, const char *mode)
{
    if (path == NULL || mode == NULL)
    {
        return NULL;
    }

    return fopen(path, mode);
}

int closeDataFile(FILE *file)
{
    if (file == NULL)
    {
        return -1;
    }

    return fclose(file);
}

int readRecords(const char *path,
                size_t recordSize,
                void **records,
                size_t *count)
{
    FILE *file;
    long fileSize;
    size_t recordCount;
    void *data;

    if (path == NULL || records == NULL || count == NULL || recordSize == 0)
    {
        return -1;
    }

    *records = NULL;
    *count = 0;

    file = fopen(path, "rb");

    if (file == NULL)
    {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return -1;
    }

    fileSize = ftell(file);

    if (fileSize < 0)
    {
        fclose(file);
        return -1;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return -1;
    }

    if ((size_t)fileSize % recordSize != 0)
    {
        fclose(file);
        return -1;
    }

    recordCount = (size_t)fileSize / recordSize;

    if (recordCount == 0)
    {
        fclose(file);
        return 0;
    }

    data = malloc(recordCount * recordSize);

    if (data == NULL)
    {
        fclose(file);
        return -1;
    }

    if (fread(data, recordSize, recordCount, file) != recordCount)
    {
        free(data);
        fclose(file);
        return -1;
    }

    fclose(file);

    *records = data;
    *count = recordCount;

    return 0;
}

int writeRecords(const char *path,
                 size_t recordSize,
                 const void *records,
                 size_t count)
{
    FILE *file;

    if (path == NULL || recordSize == 0)
    {
        return -1;
    }

    if (count > 0 && records == NULL)
    {
        return -1;
    }

    file = fopen(path, "wb");

    if (file == NULL)
    {
        return -1;
    }

    if (count > 0)
    {
        if (fwrite(records, recordSize, count, file) != count)
        {
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    return 0;
}

int appendRecord(const char *path,
                 size_t recordSize,
                 const void *record)
{
    FILE *file;

    if (path == NULL || record == NULL || recordSize == 0)
    {
        return -1;
    }

    file = fopen(path, "ab");

    if (file == NULL)
    {
        return -1;
    }

    if (fwrite(record, recordSize, 1, file) != 1)
    {
        fclose(file);
        return -1;
    }

    fclose(file);

    return 0;
}

int updateRecordById(const char *path,
                     size_t recordSize,
                     size_t idOffset,
                     int id,
                     const void *replacement,
                     int *updated)
{
    void *records = NULL;
    size_t count = 0;
    size_t i;
    int found = 0;

    if (updated != NULL)
    {
        *updated = 0;
    }

    if (path == NULL ||
        recordSize == 0 ||
        replacement == NULL ||
        idOffset + sizeof(int) > recordSize)
    {
        return -1;
    }

    if (readRecords(path, recordSize, &records, &count) != 0)
    {
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        char *record;
        int recordId;

        record = (char *)records + (i * recordSize);

        memcpy(&recordId,
               record + idOffset,
               sizeof(int));

        if (recordId == id)
        {
            memcpy(record, replacement, recordSize);
            found = 1;
            break;
        }
    }

    if (found)
    {
        if (writeRecords(path, recordSize, records, count) != 0)
        {
            free(records);
            return -1;
        }

        if (updated != NULL)
        {
            *updated = 1;
        }
    }

    free(records);

    return 0;
}

int deleteRecordById(const char *path,
                     size_t recordSize,
                     size_t idOffset,
                     int id,
                     int *deleted)
{
    void *records = NULL;
    void *newRecords = NULL;
    size_t count = 0;
    size_t i;
    size_t newCount = 0;
    int found = 0;

    if (deleted != NULL)
    {
        *deleted = 0;
    }

    if (path == NULL ||
        recordSize == 0 ||
        idOffset + sizeof(int) > recordSize)
    {
        return -1;
    }

    if (readRecords(path, recordSize, &records, &count) != 0)
    {
        return -1;
    }

    if (count == 0)
    {
        free(records);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        char *record;
        int recordId;

        record = (char *)records + (i * recordSize);

        memcpy(&recordId,
               record + idOffset,
               sizeof(int));

        if (!found && recordId == id)
        {
            found = 1;
            continue;
        }

        newCount++;
    }

    if (!found)
    {
        free(records);
        return 0;
    }

    if (newCount > 0)
    {
        newRecords = malloc(newCount * recordSize);

        if (newRecords == NULL)
        {
            free(records);
            return -1;
        }
    }

    newCount = 0;

    for (i = 0; i < count; i++)
    {
        char *record;
        int recordId;

        record = (char *)records + (i * recordSize);

        memcpy(&recordId,
               record + idOffset,
               sizeof(int));

        if (recordId == id)
        {
            continue;
        }

        memcpy((char *)newRecords + (newCount * recordSize),
               record,
               recordSize);

        newCount++;
    }

    if (writeRecords(path,
                     recordSize,
                     newRecords,
                     newCount) != 0)
    {
        free(records);
        free(newRecords);
        return -1;
    }

    free(records);
    free(newRecords);

    if (deleted != NULL)
    {
        *deleted = 1;
    }

    return 0;
}

void saveStudent(Student student)
{
    appendRecord(STUDENTS_FILE,
                 sizeof(Student),
                 &student);
}

void saveBus(Bus bus)
{
    appendRecord(BUSES_FILE,
                 sizeof(Bus),
                 &bus);
}

void saveRoute(Route route)
{
    appendRecord(ROUTES_FILE,
                 sizeof(Route),
                 &route);
}

void saveDriver(Driver driver)
{
    appendRecord(DRIVERS_FILE,
                 sizeof(Driver),
                 &driver);
}

void saveSchedule(Schedule schedule)
{
    appendRecord(SCHEDULES_FILE,
                 sizeof(Schedule),
                 &schedule);
}

int saveSeat(Seat seat)
{
    return appendRecord(SEATS_FILE,
                        sizeof(Seat),
                        &seat);
}

int updateStudent(Student student)
{
    int updated = 0;

    if (updateRecordById(STUDENTS_FILE,
                         sizeof(Student),
                         offsetof(Student, studentId),
                         student.studentId,
                         &student,
                         &updated) != 0)
    {
        return -1;
    }

    return updated;
}

int updateBus(Bus bus)
{
    int updated = 0;

    if (updateRecordById(BUSES_FILE,
                         sizeof(Bus),
                         offsetof(Bus, busId),
                         bus.busId,
                         &bus,
                         &updated) != 0)
    {
        return -1;
    }

    return updated;
}

int updateRoute(Route route)
{
    int updated = 0;

    if (updateRecordById(ROUTES_FILE,
                         sizeof(Route),
                         offsetof(Route, routeId),
                         route.routeId,
                         &route,
                         &updated) != 0)
    {
        return -1;
    }

    return updated;
}

int updateDriver(Driver driver)
{
    int updated = 0;

    if (updateRecordById(DRIVERS_FILE,
                         sizeof(Driver),
                         offsetof(Driver, driverId),
                         driver.driverId,
                         &driver,
                         &updated) != 0)
    {
        return -1;
    }

    return updated;
}

int updateSchedule(Schedule schedule)
{
    int updated = 0;

    if (updateRecordById(SCHEDULES_FILE,
                         sizeof(Schedule),
                         offsetof(Schedule, scheduleId),
                         schedule.scheduleId,
                         &schedule,
                         &updated) != 0)
    {
        return -1;
    }

    return updated;
}

int updateSeatStatus(int busId,
                     int seatNumber,
                     const char *status,
                     int studentId)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int found = 0;

    if (status == NULL)
    {
        return -1;
    }

    if (readRecords(SEATS_FILE,
                    sizeof(Seat),
                    (void **)&seats,
                    &count) != 0)
    {
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId &&
            seats[i].seatNumber == seatNumber)
        {
            strncpy(seats[i].status,
                    status,
                    sizeof(seats[i].status) - 1);

            seats[i].status[sizeof(seats[i].status) - 1] = '\0';

            seats[i].studentId = studentId;

            found = 1;
            break;
        }
    }

    if (found)
    {
        if (writeRecords(SEATS_FILE,
                         sizeof(Seat),
                         seats,
                         count) != 0)
        {
            free(seats);
            return -1;
        }
    }

    free(seats);

    return found;
}