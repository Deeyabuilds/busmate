#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct {
    int studentId;
    char name[50];
    char password[50];
} Student;

typedef struct {
    int adminId;
    char username[50];
    char password[50];
} Admin;

typedef struct {
    int driverId;
    char name[50];
    char phone[20];
    int assignedBusId;
} Driver;

typedef struct {
    int busId;
    char busNumber[20];
    int capacity;
    int availableSeats;
} Bus;

typedef struct {
    int routeId;
    char routeName[50];
    char stops[100];
    char starting_point[50];
    char destination[50];
} Route;

typedef struct {
    int scheduleId;
    int busId;
    int routeId;
    char startTime[20];
    char endTime[20];
    char dayType[20];
    char departure_time[20];
} Schedule;

typedef struct {
    int studentId;
    int scheduleId;
    int seat_number;
} Booking;

typedef struct {
    int busId;
    int seatNumber;
    char status[20];
    int studentId;
    char preference[20];
} Seat;

#endif