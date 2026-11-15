#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct {
    char student_id[20];
    char name[50];
    char password[50];
} Student;

typedef struct {
    char admin_id[20];
    char username[50];
    char password[50];
} Admin;

typedef struct {
    char driver_id[20];
    char name[50];
    char assigned_bus_id[20];
} Driver;

typedef struct {
    char bus_id[20];
    char bus_number[20];
    int capacity;
} Bus;

typedef struct {
    char route_id[20];
    char starting_point[50];
    char destination[50];
} Route;

typedef struct {
    char schedule_id[20];
    char bus_id[20];
    char route_id[20];
    char departure_time[20];
} Schedule;

typedef struct {
    char student_id[20];
    char schedule_id[20];
    int seat_number;
} Booking;

typedef enum {
    SEAT_PREFERENCE_ANY = 0,
    SEAT_PREFERENCE_WINDOW,
    SEAT_PREFERENCE_FRONT
} SeatPreference;

typedef struct {
    Booking booking;
    SeatPreference preference;
} SeatWaitingListEntry;

#endif