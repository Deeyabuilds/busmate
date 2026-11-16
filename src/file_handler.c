#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/file_handler.h"


/* =========================================================
   BASIC FILE FUNCTIONS
   ========================================================= */

FILE *openDataFile(const char *path, const char *mode)
{
    FILE *file;

    if (path == NULL || mode == NULL)
    {
        return NULL;
    }

    file = fopen(path, mode);

    return file;
}


int closeDataFile(FILE *file)
{
    if (file == NULL)
    {
        return -1;
    }

    return fclose(file);
}


/* =========================================================
   READ RECORDS
   ========================================================= */

int readRecords(const char *path,
                size_t recordSize,
                void **records,
                size_t *count)
{
    FILE *file;
    long fileSize;
    size_t recordCount;
    void *data;

    if (path == NULL ||
        records == NULL ||
        count == NULL ||
        recordSize == 0)
    {
        return -1;
    }

    *records = NULL;
    *count = 0;

    file = fopen(path, "rb");

    if (file == NULL)
    {
        return -1;
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

    rewind(file);

    if (fileSize == 0)
    {
        fclose(file);
        return 0;
    }

    if ((size_t)fileSize % recordSize != 0)
    {
        fclose(file);
        return -1;
    }

    recordCount = (size_t)fileSize / recordSize;

    data = malloc(recordCount * recordSize);

    if (data == NULL)
    {
        fclose(file);
        return -1;
    }

    if (fread(data,
              recordSize,
              recordCount,
              file) != recordCount)
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


/* =========================================================
   WRITE RECORDS
   ========================================================= */

int writeRecords(const char *path,
                 size_t recordSize,
                 const void *records,
                 size_t count)
{
    FILE *file;

    if (path == NULL ||
        recordSize == 0)
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
        if (records == NULL)
        {
            fclose(file);
            return -1;
        }

        if (fwrite(records,
                   recordSize,
                   count,
                   file) != count)
        {
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    return 0;
}


/* =========================================================
   APPEND RECORD
   ========================================================= */

int appendRecord(const char *path,
                 size_t recordSize,
                 const void *record)
{
    FILE *file;

    if (path == NULL ||
        record == NULL ||
        recordSize == 0)
    {
        return -1;
    }

    file = fopen(path, "ab");

    if (file == NULL)
    {
        return -1;
    }

    if (fwrite(record,
               recordSize,
               1,
               file) != 1)
    {
        fclose(file);
        return -1;
    }

    fclose(file);

    return 0;
}


/* =========================================================
   UPDATE RECORD BY ID
   ========================================================= */

int updateRecordById(const char *path,
                     size_t recordSize,
                     size_t idOffset,
                     int id,
                     const void *replacement,
                     int *updated)
{
    void *records;
    size_t count;
    size_t i;

    if (path == NULL ||
        replacement == NULL ||
        updated == NULL ||
        recordSize == 0)
    {
        return -1;
    }

    *updated = 0;
    records = NULL;
    count = 0;

    if (readRecords(path,
                    recordSize,
                    &records,
                    &count) != 0)
    {
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        char *record;
        int *recordId;

        record = (char *)records +
                 (i * recordSize);

        recordId = (int *)(record + idOffset);

        if (*recordId == id)
        {
            memcpy(record,
                   replacement,
                   recordSize);

            *updated = 1;
            break;
        }
    }

    if (*updated)
    {
        if (writeRecords(path,
                         recordSize,
                         records,
                         count) != 0)
        {
            free(records);
            return -1;
        }
    }

    free(records);

    return 0;
}


/* =========================================================
   DELETE RECORD BY ID
   ========================================================= */

int deleteRecordById(const char *path,
                     size_t recordSize,
                     size_t idOffset,
                     int id,
                     int *deleted)
{
    void *records;
    void *newRecords;

    size_t count;
    size_t newCount;
    size_t i;

    if (path == NULL ||
        deleted == NULL ||
        recordSize == 0)
    {
        return -1;
    }

    *deleted = 0;

    records = NULL;
    newRecords = NULL;
    count = 0;
    newCount = 0;

    if (readRecords(path,
                    recordSize,
                    &records,
                    &count) != 0)
    {
        return -1;
    }

    if (count == 0)
    {
        free(records);
        return 0;
    }

    newRecords = malloc(count * recordSize);

    if (newRecords == NULL)
    {
        free(records);
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        char *record;
        int *recordId;

        record = (char *)records +
                 (i * recordSize);

        recordId = (int *)(record + idOffset);

        if (*recordId == id &&
            !(*deleted))
        {
            *deleted = 1;
            continue;
        }

        memcpy((char *)newRecords +
                   (newCount * recordSize),
               record,
               recordSize);

        newCount++;
    }

    if (*deleted)
    {
        if (writeRecords(path,
                         recordSize,
                         newRecords,
                         newCount) != 0)
        {
            free(records);
            free(newRecords);
            return -1;
        }
    }

    free(records);
    free(newRecords);

    return 0;
}


/* =========================================================
   SAVE FUNCTIONS
   ========================================================= */

void saveStudent(Student student)
{
    appendRecord(
        STUDENTS_FILE,
        sizeof(Student),
        &student
    );
}


void saveBus(Bus bus)
{
    appendRecord(
        BUSES_FILE,
        sizeof(Bus),
        &bus
    );
}


void saveRoute(Route route)
{
    appendRecord(
        ROUTES_FILE,
        sizeof(Route),
        &route
    );
}


void saveDriver(Driver driver)
{
    appendRecord(
        DRIVERS_FILE,
        sizeof(Driver),
        &driver
    );
}


void saveSchedule(Schedule schedule)
{
    appendRecord(
        SCHEDULES_FILE,
        sizeof(Schedule),
        &schedule
    );
}


int saveSeat(Seat seat)
{
    return appendRecord(
        SEATS_FILE,
        sizeof(Seat),
        &seat
    );
}


/* =========================================================
   UPDATE FUNCTIONS
   ========================================================= */

int updateStudent(Student student)
{
    int updated;

    updated = 0;

    if (updateRecordById(
            STUDENTS_FILE,
            sizeof(Student),
            offsetof(Student, studentId),
            student.studentId,
            &student,
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


int updateBusRecord(Bus bus)
{
    int updated;

    updated = 0;

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


int updateRoute(Route route)
{
    int updated;

    updated = 0;

    if (updateRecordById(
            ROUTES_FILE,
            sizeof(Route),
            offsetof(Route, routeId),
            route.routeId,
            &route,
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


int updateDriver(Driver driver)
{
    int updated;

    updated = 0;

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

    if (updated)
    {
        return 1;
    }

    return 0;
}


int updateSchedule(Schedule schedule)
{
    int updated;

    updated = 0;

    if (updateRecordById(
            SCHEDULES_FILE,
            sizeof(Schedule),
            offsetof(Schedule, scheduleId),
            schedule.scheduleId,
            &schedule,
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
   UPDATE SEAT STATUS
   ========================================================= */

int updateSeatStatus(int busId,
                     int seatNumber,
                     const char *status,
                     int studentId)
{
    void *records;
    Seat *seats;

    size_t count;
    size_t i;

    int found;

    if (status == NULL)
    {
        return -1;
    }

    records = NULL;
    seats = NULL;
    count = 0;
    found = 0;

    if (readRecords(
            SEATS_FILE,
            sizeof(Seat),
            &records,
            &count) != 0)
    {
        return -1;
    }

    seats = (Seat *)records;

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId &&
            seats[i].seatNumber == seatNumber)
        {
            strncpy(
                seats[i].status,
                status,
                sizeof(seats[i].status) - 1
            );

            seats[i].status[
                sizeof(seats[i].status) - 1
            ] = '\0';

            seats[i].studentId = studentId;

            found = 1;
            break;
        }
    }

    if (found)
    {
        if (writeRecords(
                SEATS_FILE,
                sizeof(Seat),
                seats,
                count) != 0)
        {
            free(records);
            return -1;
        }
    }

    free(records);

    if (found)
    {
        return 0;
    }

    return -1;
}

void initialize_default_data(void)
{
    void *records;
    size_t count;
    size_t i;

    Student student;
    Driver driver;

    int student_found;
    int driver_found;


    /* =====================================================
       DEFAULT STUDENT
       ===================================================== */

    records = NULL;
    count = 0;
    student_found = 0;

    if (readRecords(
            STUDENTS_FILE,
            sizeof(Student),
            &records,
            &count) == 0)
    {
        Student *students;

        students = (Student *)records;

        for (i = 0; i < count; i++)
        {
            if (students[i].studentId == 1001)
            {
                student_found = 1;
                break;
            }
        }
    }

    free(records);


    if (!student_found)
    {
        student.studentId = 1001;

        strcpy(
            student.name,
            "Deeya"
        );

        strcpy(
            student.password,
            "student123"
        );

        appendRecord(
            STUDENTS_FILE,
            sizeof(Student),
            &student
        );
    }


    /* =====================================================
       DEFAULT DRIVER
       ===================================================== */

    records = NULL;
    count = 0;
    driver_found = 0;

    if (readRecords(
            DRIVERS_FILE,
            sizeof(Driver),
            &records,
            &count) == 0)
    {
        Driver *drivers;

        drivers = (Driver *)records;

        for (i = 0; i < count; i++)
        {
            if (drivers[i].driverId == 2001)
            {
                driver_found = 1;
                break;
            }
        }
    }

    free(records);


    if (!driver_found)
    {
        driver.driverId = 2001;

        strcpy(
            driver.name,
            "Raj"
        );

        strcpy(
            driver.phone,
            "9876543210"
        );

        driver.assignedBusId = 101;

        appendRecord(
            DRIVERS_FILE,
            sizeof(Driver),
            &driver
        );
    }
}