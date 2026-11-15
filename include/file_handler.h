#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stddef.h>
#include "structures.h"

typedef enum {
    FILE_STATUS_OK = 0,
    FILE_STATUS_NOT_FOUND,
    FILE_STATUS_ALREADY_EXISTS,
    FILE_STATUS_INVALID_ARGUMENT,
    FILE_STATUS_IO_ERROR,
    FILE_STATUS_CORRUPT
} FileStatus;

#define FILE_STUDENTS_PATH "data/students.dat"
#define FILE_STUDENT_PROFILES_PATH "data/student_profiles.dat"
#define FILE_BUSES_PATH "data/buses.dat"
#define FILE_ROUTES_PATH "data/routes.dat"
#define FILE_DRIVERS_PATH "data/drivers.dat"
#define FILE_SCHEDULES_PATH "data/schedules.dat"
#define FILE_BOOKINGS_PATH "data/bookings.dat"
#define FILE_SEATS_PATH "data/seats.dat"
#define FILE_WAITLIST_PATH "data/waitlist.dat"
#define FILE_SESSION_PATH "data/temp.data"

typedef struct {
    char student_id[20];
    char bus_id[20];
    char schedule_id[20];
} BusMateSessionState;

/* Reusable binary-record operations. The caller owns memory returned by read. */
FileStatus file_append_record(const char *path, const void *record,
                              size_t record_size);
FileStatus file_read_records(const char *path, void **records,
                             size_t record_size, size_t *record_count);
FileStatus file_replace_records(const char *path, const void *records,
                                size_t record_size, size_t record_count);
FileStatus file_find_record(const char *path, const char *id,
                            void *record, size_t record_size,
                            size_t id_offset, size_t id_size);
FileStatus file_save_record(const char *path, const void *record,
                            size_t record_size, size_t id_offset,
                            size_t id_size);
FileStatus file_update_record(const char *path, const void *record,
                              size_t record_size, size_t id_offset,
                              size_t id_size);
FileStatus file_delete_record(const char *path, const char *id,
                              size_t record_size, size_t id_offset,
                              size_t id_size);
FileStatus file_write_session_state(const BusMateSessionState *state);
FileStatus file_read_session_state(BusMateSessionState *state);
const char *file_status_message(FileStatus status);

/* Student */
void saveStudent(Student student);
void viewStudents(void);
void updateStudent(Student student);
void deleteStudent(char student_id[]);

/* Bus */
void saveBus(Bus bus);
void viewBuses(void);
void updateBus(Bus bus);
void deleteBus(char bus_id[]);

/* Route */
void saveRoute(Route route);
void viewRoutes(void);
void updateRoute(Route route);
void deleteRoute(char route_id[]);

/* Driver */
void saveDriver(Driver driver);
void viewDrivers(void);
void updateDriver(Driver driver);
void deleteDriver(char driver_id[]);

/* Schedule */
void saveSchedule(Schedule schedule);
void viewSchedules(void);
void updateSchedule(Schedule schedule);
void deleteSchedule(char schedule_id[]);

/* Booking */
void saveBooking(Booking booking);
void viewBookings(void);
void deleteBooking(char student_id[], char schedule_id[]);

#endif