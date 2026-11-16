#ifndef SEAT_ALLOCATION_H
#define SEAT_ALLOCATION_H

#include "structures.h"

/* Seat allocation operations */

int add_seat(Seat seat);

int search_seat(int busId,
                int seatNumber,
                Seat *seat);

int update_seat(Seat seat);

int delete_seat(int busId,
                int seatNumber);

int allocate_seat(int busId,
                   int seatNumber,
                   int studentId,
                   const char *preference);

int release_seat(int busId,
                 int seatNumber);

#endif