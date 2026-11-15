#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file_handler.h"
#include "../include/seat_allocation.h"

static void make_booking(Booking *booking, const char *student_id)
{
    memset(booking, 0, sizeof(*booking));
    snprintf(booking->student_id, sizeof(booking->student_id), "%s",
             student_id);
    snprintf(booking->schedule_id, sizeof(booking->schedule_id), "%s",
             "SCHED-1");
}

static size_t waiting_count(void)
{
    void *records = NULL;
    size_t count = 0;
    FileStatus status = file_read_records(FILE_WAITLIST_PATH, &records,
                                          sizeof(SeatWaitingListEntry),
                                          &count);
    assert(status == FILE_STATUS_OK || status == FILE_STATUS_NOT_FOUND);
    free(records);
    return count;
}

static void test_common_file_operations(void)
{
    Student student;
    Student found;
    FileStatus status;

    memset(&student, 0, sizeof(student));
    strcpy(student.student_id, "STUDENT-1");
    strcpy(student.name, "Test Student");

    status = file_save_record(FILE_STUDENTS_PATH, &student, sizeof(student),
                              offsetof(Student, student_id),
                              sizeof(student.student_id));
    assert(status == FILE_STATUS_OK);
    assert(file_save_record(FILE_STUDENTS_PATH, &student, sizeof(student),
                            offsetof(Student, student_id),
                            sizeof(student.student_id)) ==
           FILE_STATUS_ALREADY_EXISTS);
    assert(file_find_record(FILE_STUDENTS_PATH, "STUDENT-1", &found,
                            sizeof(found), offsetof(Student, student_id),
                            sizeof(found.student_id)) == FILE_STATUS_OK);
    assert(strcmp(found.name, "Test Student") == 0);

    strcpy(student.name, "Updated Student");
    assert(file_update_record(FILE_STUDENTS_PATH, &student, sizeof(student),
                              offsetof(Student, student_id),
                              sizeof(student.student_id)) == FILE_STATUS_OK);
    assert(file_delete_record(FILE_STUDENTS_PATH, "STUDENT-1",
                              sizeof(Student), offsetof(Student, student_id),
                              sizeof(student.student_id)) == FILE_STATUS_OK);
    assert(file_find_record(FILE_STUDENTS_PATH, "STUDENT-1", &found,
                            sizeof(found), offsetof(Student, student_id),
                            sizeof(found.student_id)) == FILE_STATUS_NOT_FOUND);
}

int main(void)
{
    Bus bus;
    Schedule schedule;
    Booking booking;
    Booking found;
    SeatAvailability availability;
    SeatAllocationResult result;
    int index;

    test_common_file_operations();

    memset(&bus, 0, sizeof(bus));
    strcpy(bus.bus_id, "BUS-1");
    bus.capacity = 8;
    assert(file_save_record(FILE_BUSES_PATH, &bus, sizeof(bus),
                            offsetof(Bus, bus_id),
                            sizeof(bus.bus_id)) == FILE_STATUS_OK);

    memset(&schedule, 0, sizeof(schedule));
    strcpy(schedule.schedule_id, "SCHED-1");
    strcpy(schedule.bus_id, "BUS-1");
    assert(file_save_record(FILE_SCHEDULES_PATH, &schedule, sizeof(schedule),
                            offsetof(Schedule, schedule_id),
                            sizeof(schedule.schedule_id)) == FILE_STATUS_OK);

    make_booking(&booking, "STUDENT-1");
    result = allocate_seat_with_preference(&booking, SEAT_PREFERENCE_WINDOW);
    assert(result == SEAT_ALLOCATION_BOOKED && booking.seat_number == 1);
    {
        void *seats = NULL;
        size_t seat_count = 0;
        BusMateSessionState state;
        assert(file_read_records(FILE_SEATS_PATH, &seats,
                                 sizeof(SeatWaitingListEntry),
                                 &seat_count) == FILE_STATUS_OK);
        assert(seat_count == 1);
        assert(((SeatWaitingListEntry *)seats)[0].booking.seat_number == 1);
        assert(((SeatWaitingListEntry *)seats)[0].preference ==
               SEAT_PREFERENCE_WINDOW);
        free(seats);
        assert(file_read_session_state(&state) == FILE_STATUS_OK);
        assert(strcmp(state.student_id, "STUDENT-1") == 0);
        assert(strcmp(state.bus_id, "BUS-1") == 0);
        assert(strcmp(state.schedule_id, "SCHED-1") == 0);
    }
    make_booking(&booking, "STUDENT-2");
    result = allocate_seat_with_preference(&booking, SEAT_PREFERENCE_FRONT);
    assert(result == SEAT_ALLOCATION_BOOKED && booking.seat_number == 2);
    make_booking(&booking, "STUDENT-3");
    assert(allocate_seat(&booking) == SEAT_ALLOCATION_BOOKED);
    assert(booking.seat_number == 3);

    assert(get_seat_availability("SCHED-1", &availability));
    assert(availability.total_seats == 8);
    assert(availability.occupied_seats == 3);
    assert(availability.available_seats == 5);
    assert(!check_seat_availability("SCHED-1", 2));
    assert(check_seat_availability("SCHED-1", 8));

    for (index = 4; index <= 8; ++index)
    {
        char student_id[20];
        snprintf(student_id, sizeof(student_id), "STUDENT-%d", index);
        make_booking(&booking, student_id);
        assert(allocate_seat(&booking) == SEAT_ALLOCATION_BOOKED);
        assert(booking.seat_number == index);
    }

    make_booking(&booking, "WAIT-1");
    assert(allocate_seat_with_preference(&booking, SEAT_PREFERENCE_WINDOW) ==
           SEAT_ALLOCATION_WAITLISTED);
    make_booking(&booking, "WAIT-2");
    assert(allocate_seat_with_preference(&booking, SEAT_PREFERENCE_FRONT) ==
           SEAT_ALLOCATION_WAITLISTED);
    make_booking(&booking, "WAIT-1");
    assert(allocate_seat(&booking) == SEAT_ALLOCATION_DUPLICATE);
    assert(waiting_count() == 2);

    assert(cancel_seat("STUDENT-2", "SCHED-1"));
    assert(file_find_record(FILE_BOOKINGS_PATH, "WAIT-1", &found,
                            sizeof(found), offsetof(Booking, student_id),
                            sizeof(found.student_id)) == FILE_STATUS_OK);
    assert(found.seat_number == 2);
    assert(waiting_count() == 1);

    assert(cancel_seat("STUDENT-3", "SCHED-1"));
    assert(file_find_record(FILE_BOOKINGS_PATH, "WAIT-2", &found,
                            sizeof(found), offsetof(Booking, student_id),
                            sizeof(found.student_id)) == FILE_STATUS_OK);
    assert(found.seat_number == 3);
    assert(waiting_count() == 0);

    assert(get_seat_availability("SCHED-1", &availability));
    assert(availability.occupied_seats == 8);
    assert(availability.available_seats == 0);

    make_booking(&booking, "WAIT-3");
    assert(allocate_seat(&booking) == SEAT_ALLOCATION_WAITLISTED);
    make_booking(&booking, "WAIT-4");
    assert(allocate_seat(&booking) == SEAT_ALLOCATION_WAITLISTED);
    assert(cancel_seat("WAIT-3", "SCHED-1"));
    assert(waiting_count() == 1);
    assert(cancel_seat("STUDENT-1", "SCHED-1"));
    assert(file_find_record(FILE_BOOKINGS_PATH, "WAIT-4", &found,
                            sizeof(found), offsetof(Booking, student_id),
                            sizeof(found.student_id)) == FILE_STATUS_OK);
    assert(waiting_count() == 0);

    puts("Seat allocation and common file-handler tests passed.");
    return 0;
}
