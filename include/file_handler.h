#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "structures.h"

void ensure_data_files(void);
int saveStudent(Student student);
int saveBus(Bus bus);
int saveRoute(Route route);
int saveDriver(Driver driver);
int saveSchedule(Schedule schedule);
int saveSeat(Seat seat);

int studentLogin(int id, const char *password);
int driverLogin(int id, const char *password);
int adminLogin(const char *username, const char *password);

int loadStudentById(int id, Student *student);
int loadAllStudentsDynamic(Student **students, int *count);
int deleteStudentById(int studentId);
int deleteBusById(int busId);
int deleteRouteById(int routeId);
int deleteDriverById(int driverId);
int deleteScheduleById(int scheduleId);
int loadDriverById(int id, Driver *driver);
int loadBusById(int busId, Bus *bus);
int loadRouteById(int routeId, Route *route);
int loadScheduleByBus(int busId, Schedule *schedule);
int loadAllBusesDynamic(Bus **buses, int *count);
int loadAllRoutesDynamic(Route **routes, int *count);
int loadAllDriversDynamic(Driver **drivers, int *count);
int loadAllSchedulesDynamic(Schedule **schedules, int *count);
int loadAllSeats(Seat seats[], int maxCount, int *count);
int loadAllSeatsDynamic(Seat **seats, int *count);
int loadSeatsForBus(int busId, Seat seats[], int maxCount, int *count);

void updateBusAvailability(int busId);
int updateSeatStatus(int busId, int seatNumber, int studentId, const char *status, const char *preference);
#endif