#ifndef BOOKING_H
#define BOOKING_H

#include "structures.h"

/* Booking operations */

int add_booking(Booking booking);

int search_booking(int studentId,
                   int scheduleId,
                   Booking *booking);

int update_booking(Booking booking);

int delete_booking(int studentId,
                   int scheduleId);

#endif