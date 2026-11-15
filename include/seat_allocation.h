#ifndef SEAT_ALLOCATION_H
#define SEAT_ALLOCATION_H

#include "structures.h"

typedef enum {
    SEAT_ALLOCATION_ERROR = -1,
    SEAT_ALLOCATION_WAITLISTED = 0,
    SEAT_ALLOCATION_BOOKED = 1,
    SEAT_ALLOCATION_DUPLICATE = -2
} SeatAllocationResult;

typedef struct {
    int total_seats;
    int occupied_seats;
    int available_seats;
} SeatAvailability;

int allocate_seat(Booking *booking);
SeatAllocationResult allocate_seat_with_preference(
    Booking *booking, SeatPreference preference);
int check_seat_availability(const char *schedule_id,
                            int seat_number);
int cancel_seat(const char *student_id, const char *schedule_id);
int get_seat_availability(const char *schedule_id,
                          SeatAvailability *availability);
void display_seat_availability(const char *schedule_id);
void display_waiting_list(const char *schedule_id);

#endif