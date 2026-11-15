#include <stdio.h>
#include <stdlib.h>

#include "../include/booking.h"
#include "../include/file_handler.h"
#include "../include/seat_allocation.h"

int create_booking(Booking booking)
{
    SeatAllocationResult result =
        allocate_seat_with_preference(&booking, SEAT_PREFERENCE_ANY);
    return result == SEAT_ALLOCATION_BOOKED ||
           result == SEAT_ALLOCATION_WAITLISTED;
}

int view_bookings(void)
{
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(FILE_BOOKINGS_PATH, &records,
                                          sizeof(Booking), &count);
    if (status == FILE_STATUS_NOT_FOUND)
        return 1;
    if (status != FILE_STATUS_OK)
        return 0;

    for (index = 0; index < count; ++index)
    {
        Booking *bookings = (Booking *)records;
        printf("Student %s | schedule %s | seat %d\n",
               bookings[index].student_id, bookings[index].schedule_id,
               bookings[index].seat_number);
    }
    free(records);
    return 1;
}

int cancel_booking(const char *student_id, const char *schedule_id)
{
    return cancel_seat(student_id, schedule_id);
}