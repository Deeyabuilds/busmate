#ifndef REPORT_H
#define REPORT_H

#include "structures.h"

/* Report operations */

int get_student_count(void);
int get_driver_count(void);
int get_bus_count(void);
int get_route_count(void);
int get_schedule_count(void);
int get_booking_count(void);
int get_notification_count(void);
int get_seat_count(void);

int get_available_seat_count(void);
int get_occupied_seat_count(void);

#endif