#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <stddef.h>
#include <stdio.h>
#include "structures.h"

#define STUDENTS_FILE "data/students.dat"
#define BUSES_FILE "data/buses.dat"
#define ROUTES_FILE "data/routes.dat"
#define DRIVERS_FILE "data/drivers.dat"
#define SCHEDULES_FILE "data/schedules.dat"
#define SEATS_FILE "data/seats.dat"

FILE *openDataFile(const char *path, const char *mode);
int closeDataFile(FILE *file);
int readRecords(const char *path, size_t recordSize, void **records, size_t *count);
int writeRecords(const char *path, size_t recordSize, const void *records, size_t count);
int appendRecord(const char *path, size_t recordSize, const void *record);
int updateRecordById(const char *path, size_t recordSize, size_t idOffset,
                     int id, const void *replacement, int *updated);
int deleteRecordById(const char *path, size_t recordSize, size_t idOffset,
                     int id, int *deleted);

void saveStudent(Student student);
void saveBus(Bus bus);
void saveRoute(Route route);
void saveDriver(Driver driver);
void saveSchedule(Schedule schedule);
int saveSeat(Seat seat);

int updateStudent(Student student);
int updateBus(Bus bus);
int updateRoute(Route route);
int updateDriver(Driver driver);
int updateSchedule(Schedule schedule);
int updateSeatStatus(int busId, int seatNumber, const char *status, int studentId);
#endif