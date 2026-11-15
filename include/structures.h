#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct {
    char studentId[20];
    char name[50];
    char password[50];
} Student;

typedef struct {
    char adminId[20];
    char username[50];
    char password[50];
} Admin;

typedef struct {
    char driverId[20];
    char name[50];
    char assignedBusId[20];
} Driver;

typedef struct {
    char busId[20];
    char busNumber[20];
    int capacity;
} Bus;

typedef struct {
    char routeId[20];
    char starting_point[50];
    char destination[50];
} Route;

typedef struct {
    char scheduleId[20];
    char busId[20];
    char routeId[20];
    char departure_time[20];
} Schedule;

typedef struct {
    char studentId[20];
    char scheduleId[20];
    int seat_number;
} Booking;

typedef struct {
    int busId;
    int seatNumber;
    char status[20];
    int studentId;
} Seat;

#endif