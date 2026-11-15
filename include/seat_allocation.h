#ifndef SEAT_ALLOCATION_H
#define SEAT_ALLOCATION_H

#include <stddef.h>
#include <stdio.h>

#include "structures.h"

typedef enum
{
    SEAT_ALLOCATION_ERROR = 0,
    SEAT_ALLOCATION_BOOKED,
    SEAT_ALLOCATION_WAITLISTED,
    SEAT_ALLOCATION_DUPLICATE
} SeatAllocationResult;

typedef enum
{
    SEAT_PREFERENCE_ANY = 0,
    SEAT_PREFERENCE_WINDOW,
    SEAT_PREFERENCE_FRONT
} SeatPreference;

typedef struct
{
    Booking booking;
    SeatPreference preference;
} SeatWaitingListEntry;

typedef struct
{
    int total_seats;
    int occupied_seats;
    int available_seats;
} SeatAvailability;

typedef enum
{
    SEAT_CANCELLATION_ERROR = 0,
    SEAT_CANCELLATION_NOT_FOUND,
    SEAT_CANCELLATION_PROMOTION_FAILED,
    SEAT_CANCELLATION_PROMOTED,
    SEAT_CANCELLATION_COMPLETED
} SeatCancellationResult;

int initializeBusSeats(int busId);

int validateBusSeatData(int busId);

int createBusWithSeats(const Bus *bus);

int removeBusWithSeats(int busId);

int getBusSeats(int busId,
                Seat **seats,
                size_t *count);

int getBusOccupancy(int busId,
                    int *bookedSeats,
                    int *capacity);

int setSeatPreference(int busId,
                      int seatNumber,
                      const char *preference);

int allocateSeat(int busId,
                 int studentId,
                 const char *preference,
                 Seat *allocatedSeat);

int allocateSpecificSeat(int busId,
                         int studentId,
                         int seatNumber,
                         const char *preference,
                         Seat *allocatedSeat);

SeatCancellationResult cancelSeat(int busId,
                                  int studentId,
                                  Seat *freedSeat,
                                  int *promotedStudentId);

int getWaitingList(int busId,
                   int **studentIds,
                   size_t *count);

int displayBusOccupancy(int busId,
                        FILE *output);

int displayWaitingList(int busId,
                       FILE *output);

int allocate_seat(Booking *booking);

int check_seat_availability(const char *schedule_id,
                            int seat_number);

int get_seat_availability(const char *schedule_id,
                          SeatAvailability *availability);

void display_seat_availability(const char *schedule_id);

void display_waiting_list(const char *schedule_id);

int cancel_seat(const char *student_id,
                const char *schedule_id);

#endif