#ifndef SEAT_ALLOCATION_H
#define SEAT_ALLOCATION_H

int requestStudentSeat(int busId, int studentId, const char *preference, int *assignedSeat);
int checkSeatAvailability(int busId, int seatNumber);
int getBusSeatCapacity(int busId);
int getAllocatedSeatCount(int busId);
int cancelStudentSeat(int studentId, int busId);

#endif
