#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "file_handler.h"
#include "busmate_config.h"

static int ensure_data_directory(void)
{
#ifdef _WIN32
    if (_mkdir("data") != 0 && errno != EEXIST)
#else
    if (mkdir("data", 0775) != 0 && errno != EEXIST)
#endif
    {
        perror("data");
        return 0;
    }
    return 1;
}

static int write_records_atomically(const char *path, const void *records, size_t count, size_t recordSize)
{
    char temporaryPath[260];
    char backupPath[260];
    FILE *temporary;

    if (path == NULL || (count > 0 && records == NULL) || recordSize == 0 ||
        count > (size_t)-1 / recordSize ||
        snprintf(temporaryPath, sizeof(temporaryPath), "%s.tmp", path) >= (int)sizeof(temporaryPath) ||
        snprintf(backupPath, sizeof(backupPath), "%s.bak", path) >= (int)sizeof(backupPath) ||
        !ensure_data_directory())
        return 0;

    temporary = fopen(temporaryPath, "wb");
    if (temporary == NULL)
    {
        perror(temporaryPath);
        return 0;
    }
    if ((count > 0 && fwrite(records, recordSize, count, temporary) != count) ||
        fflush(temporary) != 0)
    {
        perror(temporaryPath);
        fclose(temporary);
        remove(temporaryPath);
        return 0;
    }
    if (fclose(temporary) != 0)
    {
        perror(temporaryPath);
        remove(temporaryPath);
        return 0;
    }

#ifdef _WIN32
    {
        FILE *current = fopen(path, "rb");
        int targetExists = current != NULL;
        if (current != NULL)
            fclose(current);
        else if (errno != ENOENT)
        {
            perror(path);
            remove(temporaryPath);
            return 0;
        }

        if (targetExists)
        {
            FILE *backup = fopen(backupPath, "rb");
            if (backup != NULL)
            {
                fclose(backup);
                if (remove(backupPath) != 0)
                {
                    perror(backupPath);
                    remove(temporaryPath);
                    return 0;
                }
            }
            if (rename(path, backupPath) != 0)
            {
                perror(path);
                remove(temporaryPath);
                return 0;
            }
            if (rename(temporaryPath, path) != 0)
            {
                perror(path);
                if (rename(backupPath, path) != 0)
                    perror(backupPath);
                return 0;
            }
            if (remove(backupPath) != 0)
                perror(backupPath);
        }
        else if (rename(temporaryPath, path) != 0)
        {
            perror(path);
            remove(temporaryPath);
            return 0;
        }
    }
#else
    if (rename(temporaryPath, path) != 0)
    {
        perror(path);
        remove(temporaryPath);
        return 0;
    }
#endif
    return 1;
}

static int record_id_exists(const char *path, size_t recordSize, int id)
{
    FILE *file = fopen(path, "rb");
    int recordId;
    long length;

    if (file == NULL)
    {
        if (errno == ENOENT)
            return 0;
        perror(path);
        return -1;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        (size_t)length % recordSize != 0 || fseek(file, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Invalid or unreadable record file: %s\n", path);
        fclose(file);
        return -1;
    }
    while (fread(&recordId, sizeof(recordId), 1, file) == 1)
    {
        if (recordId == id)
        {
            fclose(file);
            return 1;
        }
        if (fseek(file, (long)(recordSize - sizeof(recordId)), SEEK_CUR) != 0)
        {
            perror(path);
            fclose(file);
            return -1;
        }
    }
    if (ferror(file))
    {
        perror(path);
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}

static int append_unique_record(const char *path, const void *record, size_t recordSize, int id)
{
    FILE *file;
    int exists = record_id_exists(path, recordSize, id);

    if (exists != 0 || !ensure_data_directory())
        return 0;
    file = fopen(path, "ab");
    if (file == NULL)
    {
        perror(path);
        return 0;
    }
    if (fwrite(record, recordSize, 1, file) != 1)
    {
        perror(path);
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0)
    {
        perror(path);
        return 0;
    }
    return 1;
}

static int delete_record(const char *path, size_t recordSize, int id)
{
    FILE *source;
    unsigned char *record;
    unsigned char *records = NULL;
    long length;
    size_t count = 0;
    size_t capacity = 0;
    int found = 0;

    if (recordSize < sizeof(id))
        return 0;
    source = fopen(path, "rb");
    if (source == NULL)
    {
        perror(path);
        return 0;
    }
    if (fseek(source, 0, SEEK_END) != 0 || (length = ftell(source)) < 0 ||
        (size_t)length % recordSize != 0 || fseek(source, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Invalid or unreadable record file: %s\n", path);
        fclose(source);
        return 0;
    }
    record = (unsigned char *)malloc(recordSize);
    if (record == NULL)
    {
        fclose(source);
        return 0;
    }

    while (fread(record, recordSize, 1, source) == 1)
    {
        int recordId;
        memcpy(&recordId, record, sizeof(recordId));
        if (recordId == id)
            found = 1;
        else
        {
            if (count == capacity)
            {
                size_t nextCapacity = capacity == 0 ? 16 : capacity * 2;
                unsigned char *expanded;
                if (nextCapacity < capacity || nextCapacity > (size_t)-1 / recordSize)
                {
                    free(record);
                    free(records);
                    fclose(source);
                    return 0;
                }
                expanded = (unsigned char *)realloc(records, nextCapacity * recordSize);
                if (expanded == NULL)
                {
                    free(record);
                    free(records);
                    fclose(source);
                    return 0;
                }
                records = expanded;
                capacity = nextCapacity;
            }
            memcpy(records + count * recordSize, record, recordSize);
            count++;
        }
    }
    free(record);
    if (ferror(source))
    {
        perror(path);
        fclose(source);
        free(records);
        return 0;
    }
    if (fclose(source) != 0)
    {
        perror(path);
        free(records);
        return 0;
    }
    if (!found)
    {
        free(records);
        return 0;
    }

    {
        int result = write_records_atomically(path, records, count, recordSize);
        free(records);
        return result;
    }
}

void ensure_data_files(void)
{
    const char *files[] = {
        "data/students.dat",
        "data/buses.dat",
        "data/routes.dat",
        "data/drivers.dat",
        "data/schedules.dat",
        "data/seats.dat"};
    size_t i;

    if (!ensure_data_directory())
        return;
    for (i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        FILE *file = fopen(files[i], "ab");
        if (file == NULL)
        {
            perror(files[i]);
            continue;
        }
        fclose(file);
    }
}

int saveStudent(Student student)
{
    if (student.studentId <= 0 || student.name[0] == '\0' ||
        student.password[0] == '\0' || student.collegeId[0] == '\0')
        return 0;
    return append_unique_record("data/students.dat", &student, sizeof(student), student.studentId);
}

int saveBus(Bus bus)
{
    Route route;
    Driver driver;

    if (bus.busId <= 0 || bus.capacity <= 0 || bus.busNumber[0] == '\0' ||
        bus.routeId < 0 || bus.driverId < 0 ||
        (bus.routeId > 0 && !loadRouteById(bus.routeId, &route)) ||
        (bus.driverId > 0 && !loadDriverById(bus.driverId, &driver)))
        return 0;
    return append_unique_record("data/buses.dat", &bus, sizeof(bus), bus.busId);
}

int saveRoute(Route route)
{
    if (route.routeId <= 0 || route.routeName[0] == '\0' || route.stops[0] == '\0')
        return 0;
    return append_unique_record("data/routes.dat", &route, sizeof(route), route.routeId);
}

int saveDriver(Driver driver)
{
    if (driver.driverId <= 0 || driver.name[0] == '\0' || driver.phone[0] == '\0' ||
        driver.password[0] == '\0' || driver.assignedBusId < 0)
        return 0;
    return append_unique_record("data/drivers.dat", &driver, sizeof(driver), driver.driverId);
}

int saveSchedule(Schedule schedule)
{
    Bus bus;
    Route route;

    if (schedule.scheduleId <= 0 || schedule.busId <= 0 || schedule.routeId <= 0 ||
        schedule.startTime[0] == '\0' || schedule.endTime[0] == '\0' || schedule.dayType[0] == '\0' ||
        !loadBusById(schedule.busId, &bus) || !loadRouteById(schedule.routeId, &route))
        return 0;
    return append_unique_record("data/schedules.dat", &schedule, sizeof(schedule), schedule.scheduleId);
}

int saveSeat(Seat seat)
{
    Student student;
    Bus bus;

    if (seat.seatNumber != 0 || strcmp(seat.status, "Waiting") != 0 ||
        seat.busId <= 0 || seat.studentId <= 0 ||
        !loadBusById(seat.busId, &bus) || !loadStudentById(seat.studentId, &student))
        return 0;
    if (!ensure_data_directory())
        return 0;
    {
        Seat *existingSeats = NULL;
        int count = 0;
        int i;

        if (!loadAllSeatsDynamic(&existingSeats, &count))
            return 0;
        for (i = 0; i < count; i++)
        {
            if (existingSeats[i].busId == seat.busId &&
                existingSeats[i].studentId == seat.studentId &&
                (strcmp(existingSeats[i].status, "Waiting") == 0 ||
                 strcmp(existingSeats[i].status, "Booked") == 0))
            {
                free(existingSeats);
                return 0;
            }
        }
        free(existingSeats);
    }
    FILE *file = fopen("data/seats.dat", "ab");
    if (file == NULL)
    {
        perror("data/seats.dat");
        return 0;
    }
    if (fwrite(&seat, sizeof(Seat), 1, file) != 1)
    {
        perror("data/seats.dat");
        fclose(file);
        return 0;
    }
    if (fclose(file) != 0)
    {
        perror("data/seats.dat");
        return 0;
    }
    return 1;
}

int loadAllSeats(Seat seats[], int maxCount, int *count)
{
    Seat *allSeats = NULL;
    int total = 0;
    int loaded;

    if (count == NULL || maxCount < 0 || (maxCount > 0 && seats == NULL))
        return 0;
    if (!loadAllSeatsDynamic(&allSeats, &total))
    {
        *count = 0;
        return 0;
    }

    loaded = total < maxCount ? total : maxCount;
    if (loaded > 0)
        memcpy(seats, allSeats, (size_t)loaded * sizeof(Seat));
    free(allSeats);
    *count = loaded;
    return loaded > 0;
}

static int load_records_dynamic(const char *path, void **records, size_t recordSize, int *count)
{
    FILE *file;
    long length;
    size_t recordCount;

    if (path == NULL || records == NULL || recordSize == 0 || count == NULL)
        return 0;
    *records = NULL;
    *count = 0;
    file = fopen(path, "rb");
    if (file == NULL)
    {
        if (errno == ENOENT)
            return 1;
        perror(path);
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        (size_t)length % recordSize != 0 || fseek(file, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Invalid or unreadable record file: %s\n", path);
        fclose(file);
        return 0;
    }
    recordCount = (size_t)length / recordSize;
    if (recordCount > INT_MAX || recordCount > (size_t)-1 / recordSize)
    {
        fprintf(stderr, "Record file is too large: %s\n", path);
        fclose(file);
        return 0;
    }
    if (recordCount > 0)
    {
        *records = malloc(recordCount * recordSize);
        if (*records == NULL)
        {
            fclose(file);
            return 0;
        }
        if (fread(*records, recordSize, recordCount, file) != recordCount)
        {
            perror(path);
            fclose(file);
            free(*records);
            *records = NULL;
            return 0;
        }
    }
    if (fclose(file) != 0)
    {
        perror(path);
        free(*records);
        *records = NULL;
        return 0;
    }
    *count = (int)recordCount;
    return 1;
}

int loadAllBusesDynamic(Bus **buses, int *count)
{
    void *records = NULL;
    int result;

    if (buses == NULL)
        return 0;
    *buses = NULL;
    result = load_records_dynamic("data/buses.dat", &records, sizeof(Bus), count);
    *buses = (Bus *)records;
    return result;
}

int loadAllStudentsDynamic(Student **students, int *count)
{
    void *records = NULL;
    int result;

    if (students == NULL)
        return 0;
    *students = NULL;
    result = load_records_dynamic("data/students.dat", &records, sizeof(Student), count);
    *students = (Student *)records;
    return result;
}

int loadAllRoutesDynamic(Route **routes, int *count)
{
    void *records = NULL;
    int result;

    if (routes == NULL)
        return 0;
    *routes = NULL;
    result = load_records_dynamic("data/routes.dat", &records, sizeof(Route), count);
    *routes = (Route *)records;
    return result;
}

int loadAllDriversDynamic(Driver **drivers, int *count)
{
    void *records = NULL;
    int result;

    if (drivers == NULL)
        return 0;
    *drivers = NULL;
    result = load_records_dynamic("data/drivers.dat", &records, sizeof(Driver), count);
    *drivers = (Driver *)records;
    return result;
}

int loadAllSchedulesDynamic(Schedule **schedules, int *count)
{
    void *records = NULL;
    int result;

    if (schedules == NULL)
        return 0;
    *schedules = NULL;
    result = load_records_dynamic("data/schedules.dat", &records, sizeof(Schedule), count);
    *schedules = (Schedule *)records;
    return result;
}

int loadAllSeatsDynamic(Seat **seats, int *count)
{
    void *records = NULL;
    int result;

    if (seats == NULL)
        return 0;
    *seats = NULL;
    result = load_records_dynamic("data/seats.dat", &records, sizeof(Seat), count);
    *seats = (Seat *)records;
    return result;
}

int loadSeatsForBus(int busId, Seat seats[], int maxCount, int *count)
{
    Seat *allSeats = NULL;
    int total = 0;
    int idx = 0;
    int i;

    if (count == NULL || maxCount < 0 || busId <= 0 ||
        (maxCount > 0 && seats == NULL))
        return 0;
    if (!loadAllSeatsDynamic(&allSeats, &total))
    {
        *count = 0;
        return 0;
    }

    for (i = 0; i < total && idx < maxCount; i++)
    {
        if (allSeats[i].busId == busId)
            seats[idx++] = allSeats[i];
    }
    free(allSeats);
    *count = idx;
    return idx > 0;
}

static int save_seats_and_bus_availability(int busId, const Seat seats[], int seatCount)
{
    Bus *buses = NULL;
    int busCount = 0;
    int i;
    int busIndex = -1;
    int bookedSeats = 0;
    int previousAvailability;

    if (!loadAllBusesDynamic(&buses, &busCount))
        return 0;
    for (i = 0; i < busCount; i++)
    {
        if (buses[i].busId == busId)
        {
            busIndex = i;
            break;
        }
    }
    if (busIndex < 0)
    {
        free(buses);
        return 0;
    }
    for (i = 0; i < seatCount; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber > 0 &&
            strcmp(seats[i].status, "Booked") == 0)
            bookedSeats++;
    }
    previousAvailability = buses[busIndex].availableSeats;
    buses[busIndex].availableSeats = buses[busIndex].capacity - bookedSeats;
    if (buses[busIndex].availableSeats < 0)
        buses[busIndex].availableSeats = 0;

    if (!write_records_atomically("data/buses.dat", buses, (size_t)busCount, sizeof(Bus)))
    {
        free(buses);
        return 0;
    }
    if (!write_records_atomically("data/seats.dat", seats, (size_t)seatCount, sizeof(Seat)))
    {
        buses[busIndex].availableSeats = previousAvailability;
        if (!write_records_atomically("data/buses.dat", buses, (size_t)busCount, sizeof(Bus)))
            fprintf(stderr, "Unable to roll back bus availability after a seat update failure.\n");
        free(buses);
        return 0;
    }
    free(buses);
    return 1;
}

int updateSeatStatus(int busId, int seatNumber, int studentId, const char *status, const char *preference)
{
    Seat *seats = NULL;
    int count = 0;
    int seatIndex = -1;
    int waitingIndex = -1;
    int i;
    Bus bus;
    Student student;

    if (busId <= 0 || seatNumber < 0 || studentId < 0 || status == NULL || preference == NULL ||
        !loadBusById(busId, &bus) || !loadAllSeatsDynamic(&seats, &count))
        return 0;

    if (seatNumber == 0)
    {
        if (studentId <= 0 || strcmp(status, "Cancelled") != 0)
        {
            free(seats);
            return 0;
        }
        for (i = 0; i < count; i++)
        {
            if (seats[i].busId == busId && seats[i].seatNumber == 0 &&
                seats[i].studentId == studentId && strcmp(seats[i].status, "Waiting") == 0)
            {
                snprintf(seats[i].status, sizeof(seats[i].status), "Cancelled");
                if (!write_records_atomically("data/seats.dat", seats, (size_t)count, sizeof(Seat)))
                {
                    free(seats);
                    return 0;
                }
                free(seats);
                return 1;
            }
        }
        free(seats);
        return 0;
    }

    if (seatNumber > bus.capacity)
    {
        free(seats);
        return 0;
    }
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber == seatNumber)
        {
            if (seatIndex >= 0)
            {
                free(seats);
                return 0;
            }
            seatIndex = i;
        }
    }

    if (strcmp(status, "Booked") == 0)
    {
        if (studentId <= 0 || !loadStudentById(studentId, &student) ||
            (seatIndex >= 0 && strcmp(seats[seatIndex].status, "Available") != 0))
        {
            free(seats);
            return 0;
        }
        for (i = 0; i < count; i++)
        {
            if (seats[i].busId == busId && seats[i].studentId == studentId &&
                (strcmp(seats[i].status, "Booked") == 0 || strcmp(seats[i].status, "Waiting") == 0))
            {
                free(seats);
                return 0;
            }
        }
        if (seatIndex < 0)
        {
            Seat *expanded = (Seat *)realloc(seats, (size_t)(count + 1) * sizeof(Seat));
            if (expanded == NULL)
            {
                free(seats);
                return 0;
            }
            seats = expanded;
            seatIndex = count++;
            memset(&seats[seatIndex], 0, sizeof(Seat));
            seats[seatIndex].busId = busId;
            seats[seatIndex].seatNumber = seatNumber;
        }
        seats[seatIndex].studentId = studentId;
        snprintf(seats[seatIndex].status, sizeof(seats[seatIndex].status), "Booked");
        snprintf(seats[seatIndex].preference, sizeof(seats[seatIndex].preference), "%s", preference);
    }
    else if (strcmp(status, "Available") == 0 && studentId == 0 && seatIndex >= 0 &&
             strcmp(seats[seatIndex].status, "Booked") == 0)
    {
        for (i = 0; i < count; i++)
        {
            if (seats[i].busId == busId && seats[i].seatNumber == 0 &&
                strcmp(seats[i].status, "Waiting") == 0)
            {
                waitingIndex = i;
                break;
            }
        }
        if (waitingIndex >= 0)
        {
            char waitingPreference[sizeof(seats[waitingIndex].preference)];
            snprintf(waitingPreference, sizeof(waitingPreference), "%s", seats[waitingIndex].preference);
            seats[seatIndex].studentId = seats[waitingIndex].studentId;
            snprintf(seats[seatIndex].status, sizeof(seats[seatIndex].status), "Booked");
            snprintf(seats[seatIndex].preference, sizeof(seats[seatIndex].preference), "%s",
                     waitingPreference);
            snprintf(seats[waitingIndex].status, sizeof(seats[waitingIndex].status), "Promoted");
        }
        else
        {
            seats[seatIndex].studentId = 0;
            snprintf(seats[seatIndex].status, sizeof(seats[seatIndex].status), "Available");
            snprintf(seats[seatIndex].preference, sizeof(seats[seatIndex].preference), "%s", preference);
        }
    }
    else
    {
        free(seats);
        return 0;
    }

    if (!save_seats_and_bus_availability(busId, seats, count))
    {
        free(seats);
        return 0;
    }
    free(seats);
    return 1;
}

void updateBusAvailability(int busId)
{
    Bus *buses = NULL;
    Seat *seats = NULL;
    int busCount = 0;
    int seatCount = 0;
    int i;
    int bookedSeats = 0;
    int foundBus = 0;

    if (busId <= 0 || !loadAllBusesDynamic(&buses, &busCount) ||
        !loadAllSeatsDynamic(&seats, &seatCount))
    {
        free(buses);
        free(seats);
        return;
    }

    for (i = 0; i < seatCount; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber > 0 &&
            strcmp(seats[i].status, "Booked") == 0)
            bookedSeats++;
    }

    for (i = 0; i < busCount; i++)
    {
        if (buses[i].busId == busId)
        {
            buses[i].availableSeats = buses[i].capacity - bookedSeats;
            if (buses[i].availableSeats < 0)
                buses[i].availableSeats = 0;
            foundBus = 1;
            break;
        }
    }

    if (foundBus && !write_records_atomically("data/buses.dat", buses, (size_t)busCount, sizeof(Bus)))
        fprintf(stderr, "Unable to update availability for bus %d.\n", busId);
    free(buses);
    free(seats);
}

int loadStudentById(int id, Student *student)
{
    FILE *file = fopen("data/students.dat", "rb");
    if (file == NULL || student == NULL)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fread(student, sizeof(Student), 1, file) == 1)
    {
        if (student->studentId == id)
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int deleteStudentById(int studentId)
{
    if (studentId <= 0)
        return 0;
    return delete_record("data/students.dat", sizeof(Student), studentId);
}

int deleteBusById(int busId)
{
    if (busId <= 0)
        return 0;
    return delete_record("data/buses.dat", sizeof(Bus), busId);
}

int deleteRouteById(int routeId)
{
    if (routeId <= 0)
        return 0;
    return delete_record("data/routes.dat", sizeof(Route), routeId);
}

int deleteDriverById(int driverId)
{
    if (driverId <= 0)
        return 0;
    return delete_record("data/drivers.dat", sizeof(Driver), driverId);
}

int deleteScheduleById(int scheduleId)
{
    if (scheduleId <= 0)
        return 0;
    return delete_record("data/schedules.dat", sizeof(Schedule), scheduleId);
}

int loadDriverById(int id, Driver *driver)
{
    FILE *file = fopen("data/drivers.dat", "rb");
    if (file == NULL || driver == NULL)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fread(driver, sizeof(Driver), 1, file) == 1)
    {
        if (driver->driverId == id)
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int loadBusById(int busId, Bus *bus)
{
    FILE *file = fopen("data/buses.dat", "rb");
    if (file == NULL || bus == NULL)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fread(bus, sizeof(Bus), 1, file) == 1)
    {
        if (bus->busId == busId)
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int loadRouteById(int routeId, Route *route)
{
    FILE *file = fopen("data/routes.dat", "rb");
    if (file == NULL || route == NULL)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fread(route, sizeof(Route), 1, file) == 1)
    {
        if (route->routeId == routeId)
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

int loadScheduleByBus(int busId, Schedule *schedule)
{
    FILE *file = fopen("data/schedules.dat", "rb");
    if (file == NULL || schedule == NULL)
    {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    while (fread(schedule, sizeof(Schedule), 1, file) == 1)
    {
        if (schedule->busId == busId)
        {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}
