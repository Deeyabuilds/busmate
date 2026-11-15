#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include "../include/file_handler.h"
#include "../include/seat_allocation.h"

static int ensure_data_directory(const char *path)
{
    if (strncmp(path, "data/", 5) != 0 &&
        strncmp(path, "data\\", 5) != 0)
        return 1;

#ifdef _WIN32
    if (_mkdir("data") == 0 || errno == EEXIST)
#else
    if (mkdir("data", 0777) == 0 || errno == EEXIST)
#endif
        return 1;

    return 0;
}

static int replace_file(const char *temporary_path, const char *path)
{
#ifdef _WIN32
    return MoveFileExA(temporary_path, path,
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)
               != 0;
#else
    return rename(temporary_path, path) == 0;
#endif
}

static int valid_record_key(const void *record, size_t record_size,
                            size_t id_offset, size_t id_size)
{
    const char *key;

    if (record == NULL || id_size == 0 || id_offset > record_size ||
        id_size > record_size - id_offset)
        return 0;

    key = (const char *)record + id_offset;
    return key[0] != '\0' && memchr(key, '\0', id_size) != NULL;
}

static int key_matches(const void *record, const char *id,
                       size_t record_size, size_t id_offset, size_t id_size)
{
    const char *key;

    if (id == NULL || !valid_record_key(record, record_size,
                                        id_offset, id_size))
        return 0;

    key = (const char *)record + id_offset;
    return strncmp(key, id, id_size) == 0 &&
           strlen(id) < id_size;
}

const char *file_status_message(FileStatus status)
{
    switch (status)
    {
    case FILE_STATUS_OK:
        return "success";
    case FILE_STATUS_NOT_FOUND:
        return "record or file not found";
    case FILE_STATUS_ALREADY_EXISTS:
        return "record already exists";
    case FILE_STATUS_INVALID_ARGUMENT:
        return "invalid argument";
    case FILE_STATUS_IO_ERROR:
        return "file access failed";
    case FILE_STATUS_CORRUPT:
        return "file contains an incomplete record";
    default:
        return "unknown file error";
    }
}

FileStatus file_append_record(const char *path, const void *record,
                              size_t record_size)
{
    FILE *file;
    int close_result;

    if (path == NULL || record == NULL || record_size == 0)
        return FILE_STATUS_INVALID_ARGUMENT;
    if (!ensure_data_directory(path))
        return FILE_STATUS_IO_ERROR;

    file = fopen(path, "ab");
    if (file == NULL)
        return FILE_STATUS_IO_ERROR;

    if (fwrite(record, record_size, 1, file) != 1)
    {
        fclose(file);
        return FILE_STATUS_IO_ERROR;
    }

    close_result = fclose(file);
    return close_result == 0 ? FILE_STATUS_OK : FILE_STATUS_IO_ERROR;
}

FileStatus file_read_records(const char *path, void **records,
                             size_t record_size, size_t *record_count)
{
    FILE *file;
    long file_size;
    size_t count;
    void *buffer = NULL;

    if (path == NULL || records == NULL || record_count == NULL ||
        record_size == 0)
        return FILE_STATUS_INVALID_ARGUMENT;

    *records = NULL;
    *record_count = 0;
    file = fopen(path, "rb");
    if (file == NULL)
        return errno == ENOENT ? FILE_STATUS_NOT_FOUND : FILE_STATUS_IO_ERROR;

    if (fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return FILE_STATUS_IO_ERROR;
    }

    if ((unsigned long)file_size % record_size != 0)
    {
        fclose(file);
        return FILE_STATUS_CORRUPT;
    }

    count = (size_t)((unsigned long)file_size / record_size);
    if (count > 0)
    {
        if (count > (size_t)-1 / record_size)
        {
            fclose(file);
            return FILE_STATUS_IO_ERROR;
        }

        buffer = malloc(count * record_size);
        if (buffer == NULL)
        {
            fclose(file);
            return FILE_STATUS_IO_ERROR;
        }

        if (fread(buffer, record_size, count, file) != count)
        {
            free(buffer);
            fclose(file);
            return FILE_STATUS_IO_ERROR;
        }
    }

    if (fclose(file) != 0)
    {
        free(buffer);
        return FILE_STATUS_IO_ERROR;
    }

    *records = buffer;
    *record_count = count;
    return FILE_STATUS_OK;
}

FileStatus file_replace_records(const char *path, const void *records,
                                size_t record_size, size_t record_count)
{
    FILE *file;
    char *temporary_path;
    size_t path_length;
    size_t written = 0;
    int close_result;
    FileStatus result = FILE_STATUS_IO_ERROR;

    if (path == NULL || record_size == 0 ||
        (record_count > 0 && records == NULL))
        return FILE_STATUS_INVALID_ARGUMENT;
    if (record_count > (size_t)-1 / record_size ||
        !ensure_data_directory(path))
        return FILE_STATUS_IO_ERROR;

    path_length = strlen(path);
    if (path_length > (size_t)-1 - 5)
        return FILE_STATUS_INVALID_ARGUMENT;
    temporary_path = (char *)malloc(path_length + 5);
    if (temporary_path == NULL)
        return FILE_STATUS_IO_ERROR;

    memcpy(temporary_path, path, path_length);
    memcpy(temporary_path + path_length, ".tmp", 5);
    file = fopen(temporary_path, "wb");
    if (file == NULL)
    {
        free(temporary_path);
        return FILE_STATUS_IO_ERROR;
    }

    if (record_count == 0 ||
        (written = fwrite(records, record_size, record_count, file)) ==
            record_count)
    {
        close_result = fclose(file);
        if (close_result == 0 && replace_file(temporary_path, path))
            result = FILE_STATUS_OK;
    }
    else
    {
        fclose(file);
    }

    if (result != FILE_STATUS_OK)
        remove(temporary_path);
    free(temporary_path);
    return result;
}

FileStatus file_find_record(const char *path, const char *id,
                            void *record, size_t record_size,
                            size_t id_offset, size_t id_size)
{
    void *records;
    size_t count;
    size_t index;
    FileStatus status;

    if (id == NULL || record == NULL)
        return FILE_STATUS_INVALID_ARGUMENT;

    status = file_read_records(path, &records, record_size, &count);
    if (status != FILE_STATUS_OK)
        return status;

    for (index = 0; index < count; ++index)
    {
        const void *candidate =
            (const char *)records + index * record_size;
        if (key_matches(candidate, id, record_size, id_offset, id_size))
        {
            memcpy(record, candidate, record_size);
            free(records);
            return FILE_STATUS_OK;
        }
    }

    free(records);
    return FILE_STATUS_NOT_FOUND;
}

FileStatus file_save_record(const char *path, const void *record,
                            size_t record_size, size_t id_offset,
                            size_t id_size)
{
    void *existing;
    size_t count;
    FileStatus status;

    if (!valid_record_key(record, record_size, id_offset, id_size))
        return FILE_STATUS_INVALID_ARGUMENT;

    status = file_read_records(path, &existing, record_size, &count);
    if (status == FILE_STATUS_OK)
    {
        size_t index;
        for (index = 0; index < count; ++index)
        {
            const void *candidate =
                (const char *)existing + index * record_size;
            if (key_matches(candidate,
                            (const char *)record + id_offset,
                            record_size, id_offset, id_size))
            {
                free(existing);
                return FILE_STATUS_ALREADY_EXISTS;
            }
        }
        free(existing);
    }
    else if (status != FILE_STATUS_NOT_FOUND)
    {
        return status;
    }

    return file_append_record(path, record, record_size);
}

FileStatus file_update_record(const char *path, const void *record,
                              size_t record_size, size_t id_offset,
                              size_t id_size)
{
    void *records;
    size_t count;
    size_t index;
    size_t match_count = 0;
    FileStatus status;

    if (!valid_record_key(record, record_size, id_offset, id_size))
        return FILE_STATUS_INVALID_ARGUMENT;

    status = file_read_records(path, &records, record_size, &count);
    if (status != FILE_STATUS_OK)
        return status;

    for (index = 0; index < count; ++index)
    {
        void *candidate = (char *)records + index * record_size;
        if (key_matches(candidate, (const char *)record + id_offset,
                        record_size, id_offset, id_size))
        {
            memcpy(candidate, record, record_size);
            ++match_count;
        }
    }

    if (match_count == 0)
        status = FILE_STATUS_NOT_FOUND;
    else if (match_count > 1)
        status = FILE_STATUS_CORRUPT;
    else
        status = file_replace_records(path, records, record_size, count);

    free(records);
    return status;
}

FileStatus file_delete_record(const char *path, const char *id,
                              size_t record_size, size_t id_offset,
                              size_t id_size)
{
    void *records;
    void *remaining = NULL;
    size_t count;
    size_t kept = 0;
    size_t index;
    FileStatus status;

    if (id == NULL || strlen(id) >= id_size || record_size == 0)
        return FILE_STATUS_INVALID_ARGUMENT;

    status = file_read_records(path, &records, record_size, &count);
    if (status != FILE_STATUS_OK)
        return status;

    if (count > 0)
    {
        remaining = malloc(count * record_size);
        if (remaining == NULL)
        {
            free(records);
            return FILE_STATUS_IO_ERROR;
        }
    }

    for (index = 0; index < count; ++index)
    {
        const void *candidate = (const char *)records + index * record_size;
        if (!key_matches(candidate, id, record_size, id_offset, id_size))
        {
            memcpy((char *)remaining + kept * record_size,
                   candidate, record_size);
            ++kept;
        }
    }

    if (kept == count)
        status = FILE_STATUS_NOT_FOUND;
    else
        status = file_replace_records(path, remaining, record_size, kept);

    free(remaining);
    free(records);
    return status;
}

FileStatus file_write_session_state(const BusMateSessionState *state)
{
    if (state == NULL ||
        memchr(state->student_id, '\0', sizeof(state->student_id)) == NULL ||
        memchr(state->bus_id, '\0', sizeof(state->bus_id)) == NULL ||
        memchr(state->schedule_id, '\0', sizeof(state->schedule_id)) == NULL)
        return FILE_STATUS_INVALID_ARGUMENT;

    return file_replace_records(FILE_SESSION_PATH, state, sizeof(*state), 1);
}

FileStatus file_read_session_state(BusMateSessionState *state)
{
    void *records = NULL;
    size_t count = 0;
    FileStatus status;

    if (state == NULL)
        return FILE_STATUS_INVALID_ARGUMENT;

    status = file_read_records(FILE_SESSION_PATH, &records, sizeof(*state),
                               &count);
    if (status != FILE_STATUS_OK)
        return status;
    if (count != 1)
    {
        free(records);
        return FILE_STATUS_CORRUPT;
    }

    memcpy(state, records, sizeof(*state));
    free(records);
    if (memchr(state->student_id, '\0', sizeof(state->student_id)) == NULL ||
        memchr(state->bus_id, '\0', sizeof(state->bus_id)) == NULL ||
        memchr(state->schedule_id, '\0', sizeof(state->schedule_id)) == NULL)
        return FILE_STATUS_CORRUPT;

    return FILE_STATUS_OK;
}

static void report_status(const char *operation, const char *entity,
                          FileStatus status)
{
    if (status != FILE_STATUS_OK)
        fprintf(stderr, "%s %s failed: %s\n", operation, entity,
                file_status_message(status));
}

#define DEFINE_ENTITY_HANDLERS(Type, name, path_macro, id_field, label)         \
    void save##name(Type record)                                                \
    {                                                                           \
        report_status("Saving", label,                                          \
                      file_save_record(path_macro, &record, sizeof(record),     \
                          offsetof(Type, id_field), sizeof(record.id_field)));  \
    }                                                                           \
    void update##name(Type record)                                              \
    {                                                                           \
        report_status("Updating", label,                                        \
                      file_update_record(path_macro, &record, sizeof(record),  \
                          offsetof(Type, id_field), sizeof(record.id_field)));  \
    }                                                                           \
    void delete##name(char id[])                                                \
    {                                                                           \
        report_status("Deleting", label,                                        \
                      file_delete_record(path_macro, id, sizeof(Type),          \
                          offsetof(Type, id_field), sizeof(((Type *)0)->id_field))); \
    }                                                                           \

DEFINE_ENTITY_HANDLERS(Student, Student, FILE_STUDENTS_PATH, student_id,
                       "student")
DEFINE_ENTITY_HANDLERS(Bus, Bus, FILE_BUSES_PATH, bus_id, "bus")
DEFINE_ENTITY_HANDLERS(Route, Route, FILE_ROUTES_PATH, route_id, "route")
DEFINE_ENTITY_HANDLERS(Driver, Driver, FILE_DRIVERS_PATH, driver_id, "driver")
DEFINE_ENTITY_HANDLERS(Schedule, Schedule, FILE_SCHEDULES_PATH, schedule_id,
                       "schedule")

static void view_records(const char *path, size_t record_size,
                         const char *label,
                         void (*print_record)(const void *record))
{
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(path, &records, record_size, &count);

    if (status == FILE_STATUS_NOT_FOUND ||
        (status == FILE_STATUS_OK && count == 0))
    {
        printf("No %s records found.\n", label);
        free(records);
        return;
    }
    if (status != FILE_STATUS_OK)
    {
        report_status("Reading", label, status);
        return;
    }

    printf("\n--- %s Records ---\n", label);
    for (index = 0; index < count; ++index)
        print_record((const char *)records + index * record_size);
    free(records);
}

static void print_student(const void *record)
{
    const Student *student = (const Student *)record;
    printf("Student ID: %s\nName: %s\n", student->student_id, student->name);
}

static void print_bus(const void *record)
{
    const Bus *bus = (const Bus *)record;
    printf("Bus ID: %s\nNumber: %s\nCapacity: %d\n",
           bus->bus_id, bus->bus_number, bus->capacity);
}

static void print_route(const void *record)
{
    const Route *route = (const Route *)record;
    printf("Route ID: %s\nFrom: %s\nTo: %s\n",
           route->route_id, route->starting_point, route->destination);
}

static void print_driver(const void *record)
{
    const Driver *driver = (const Driver *)record;
    printf("Driver ID: %s\nName: %s\nBus ID: %s\n",
           driver->driver_id, driver->name, driver->assigned_bus_id);
}

static void print_schedule(const void *record)
{
    const Schedule *schedule = (const Schedule *)record;
    printf("Schedule ID: %s\nBus ID: %s\nRoute ID: %s\nDeparture: %s\n",
           schedule->schedule_id, schedule->bus_id, schedule->route_id,
           schedule->departure_time);
}

void viewStudents(void)
{
    view_records(FILE_STUDENTS_PATH, sizeof(Student), "student", print_student);
}

void viewBuses(void)
{
    view_records(FILE_BUSES_PATH, sizeof(Bus), "bus", print_bus);
}

void viewRoutes(void)
{
    view_records(FILE_ROUTES_PATH, sizeof(Route), "route", print_route);
}

void viewDrivers(void)
{
    view_records(FILE_DRIVERS_PATH, sizeof(Driver), "driver", print_driver);
}

void viewSchedules(void)
{
    view_records(FILE_SCHEDULES_PATH, sizeof(Schedule), "schedule",
                 print_schedule);
}

void saveBooking(Booking booking)
{
    SeatAllocationResult result =
        allocate_seat_with_preference(&booking, SEAT_PREFERENCE_ANY);

    if (result != SEAT_ALLOCATION_BOOKED &&
        result != SEAT_ALLOCATION_WAITLISTED)
        fprintf(stderr, "Saving booking failed: seat allocation result %d\n",
                (int)result);
}

void viewBookings(void)
{
    Booking *records = NULL;
    void *buffer = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(FILE_BOOKINGS_PATH, &buffer,
                                          sizeof(Booking), &count);

    if (status == FILE_STATUS_NOT_FOUND ||
        (status == FILE_STATUS_OK && count == 0))
    {
        printf("No booking records found.\n");
        free(buffer);
        return;
    }
    if (status != FILE_STATUS_OK)
    {
        report_status("Reading", "booking", status);
        return;
    }

    records = (Booking *)buffer;
    printf("\n--- Booking Records ---\n");
    for (index = 0; index < count; ++index)
        printf("Student: %s, Schedule: %s, Seat: %d\n",
               records[index].student_id, records[index].schedule_id,
               records[index].seat_number);
    free(records);
}

void deleteBooking(char student_id[], char schedule_id[])
{
    if (student_id == NULL || schedule_id == NULL)
    {
        report_status("Deleting", "booking", FILE_STATUS_INVALID_ARGUMENT);
        return;
    }
    if (!cancel_seat(student_id, schedule_id))
        fprintf(stderr, "Deleting booking failed: no matching request or "
                        "shared data update failed.\n");
}

#undef DEFINE_ENTITY_HANDLERS
