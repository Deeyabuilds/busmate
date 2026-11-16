#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/report.h"
#include "../include/file_handler.h"

static int count_records(const char *path, size_t recordSize)
{
    void *records = NULL;
    size_t count = 0;

    if (readRecords(path, recordSize, &records, &count) != 0)
        return 0;

    free(records);
    return (int)count;
}

int get_student_count(void) { return count_records(STUDENTS_FILE, sizeof(Student)); }
int get_driver_count(void) { return count_records(DRIVERS_FILE, sizeof(Driver)); }
int get_bus_count(void) { return count_records(BUSES_FILE, sizeof(Bus)); }
int get_route_count(void) { return count_records(ROUTES_FILE, sizeof(Route)); }
int get_schedule_count(void) { return count_records(SCHEDULES_FILE, sizeof(Schedule)); }
int get_booking_count(void) { return count_records(BOOKINGS_FILE, sizeof(Booking)); }
int get_notification_count(void) { return count_records(NOTIFICATIONS_FILE, sizeof(Notification)); }
int get_seat_count(void) { return count_records(SEATS_FILE, sizeof(Seat)); }

int get_available_seat_count(void)
{
    void *records = NULL;
    Seat *seats;
    size_t count = 0;
    size_t i;
    int total = 0;

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0)
        return 0;

    seats = (Seat *)records;
    for (i = 0; i < count; i++)
    {
        if (strcmp(seats[i].status, "Available") == 0)
            total++;
    }

    free(records);
    return total;
}

int get_occupied_seat_count(void)
{
    void *records = NULL;
    Seat *seats;
    size_t count = 0;
    size_t i;
    int total = 0;

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0)
        return 0;

    seats = (Seat *)records;
    for (i = 0; i < count; i++)
    {
        if (strcmp(seats[i].status, "Occupied") == 0)
            total++;
    }

    free(records);
    return total;
}
