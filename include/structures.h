#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct
{
    int studentId;
    char name[50];
    char password[30];
    char collegeId[20];
} Student;

typedef struct
{
    int busId;
    char busNumber[20];
    int capacity;
    int availableSeats;
    int routeId;
    int driverId;
    char status[20];
} Bus;

typedef struct
{
    int routeId;
    char routeName[50];
    char stops[200];
    char timing[30];
    char type[15];
} Route;

typedef struct
{
    int driverId;
    char name[50];
    char phone[15];
    char password[30];
    int assignedBusId;
} Driver;

typedef struct
{
    int seatNumber;
    int busId;
    int studentId;
    char status[15];     // Available / Booked
    char preference[15]; // Window / Front / Any
} Seat;

typedef struct
{
    int scheduleId;
    int busId;
    int routeId;
    char startTime[10];
    char endTime[10];
    char dayType[15]; // Full Day / Half Day
} Schedule;

#endif