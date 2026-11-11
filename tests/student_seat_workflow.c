#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "file_handler.h"
#include "seat_allocation.h"
#include "busmate_config.h"

static void add_student(int studentId)
{
    Student student = {0};
    student.studentId = studentId;
    snprintf(student.name, sizeof(student.name), "Student %d", studentId);
    snprintf(student.collegeId, sizeof(student.collegeId), "C%d", studentId);
    snprintf(student.password, sizeof(student.password), "test");
    assert(saveStudent(student));
}

static Seat find_seat(const Seat seats[], int count, int busId, int seatNumber, int studentId, const char *status)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber == seatNumber &&
            seats[i].studentId == studentId && strcmp(seats[i].status, status) == 0)
            return seats[i];
    }
    assert(0 && "Expected seat record was not found");
    return (Seat){0};
}

int main(void)
{
    Bus bus = {0};
    Student student = {0};
    Student loadedStudent;
    int seat = 0;
    int studentId;
    int count = 0;
    int studentIdBase = (int)time(NULL);
    Bus loadedBus;
    Bus secondBus = {0};
    Seat *seats = NULL;
    int secondCapacity = BUSMATE_SEATS_PER_ROW + 1;
    int i;

    ensure_data_files();
    bus.busId = studentIdBase;
    bus.capacity = 12;
    bus.availableSeats = bus.capacity;
    snprintf(bus.busNumber, sizeof(bus.busNumber), "TEST-%d", bus.busId);
    snprintf(bus.status, sizeof(bus.status), "Active");
    assert(saveBus(bus));
    assert(getBusSeatCapacity(bus.busId) == bus.capacity);
    assert(getAllocatedSeatCount(bus.busId) == 0);
    assert(checkSeatAvailability(bus.busId, 1) == 1);
    assert(checkSeatAvailability(bus.busId, bus.capacity + 1) == -1);

    for (studentId = studentIdBase + 1; studentId <= studentIdBase + 14; studentId++)
        add_student(studentId);

    assert(requestStudentSeat(bus.busId, studentIdBase + 1, "Middle Row", &seat) == 1 && seat == 5);
    assert(requestStudentSeat(bus.busId, studentIdBase + 2, "Front Row", &seat) == 1 && seat == 1);
    assert(requestStudentSeat(bus.busId, studentIdBase + 3, "Corner Seat", &seat) == 1 && seat == 4);
    assert(requestStudentSeat(bus.busId, studentIdBase + 1, "Any", &seat) == -1);
    assert(checkSeatAvailability(bus.busId, 1) == 0);
    assert(!updateSeatStatus(bus.busId, 1, studentIdBase + 3, "Booked", "Any"));
    for (studentId = studentIdBase + 4; studentId <= studentIdBase + 12; studentId++)
        assert(requestStudentSeat(bus.busId, studentId, "Any", &seat) == 1);
    assert(getAllocatedSeatCount(bus.busId) == bus.capacity);

    assert(requestStudentSeat(bus.busId, studentIdBase + 13, "Front Row", &seat) == 0);
    assert(requestStudentSeat(bus.busId, studentIdBase + 14, "Corner Seat", &seat) == 0);

    assert(updateSeatStatus(bus.busId, 5, 0, "Available", "Any"));
    assert(loadAllSeatsDynamic(&seats, &count));
    find_seat(seats, count, bus.busId, 5, studentIdBase + 13, "Booked");
    find_seat(seats, count, bus.busId, 0, studentIdBase + 13, "Promoted");
    find_seat(seats, count, bus.busId, 0, studentIdBase + 14, "Waiting");
    assert(strcmp(find_seat(seats, count, bus.busId, 5, studentIdBase + 13, "Booked").preference, "Front Row") == 0);
    free(seats);
    seats = NULL;
    assert(loadBusById(bus.busId, &loadedBus) && loadedBus.availableSeats == 0);

    assert(updateSeatStatus(bus.busId, 1, 0, "Available", "Any"));
    assert(loadAllSeatsDynamic(&seats, &count));
    find_seat(seats, count, bus.busId, 1, studentIdBase + 14, "Booked");
    find_seat(seats, count, bus.busId, 0, studentIdBase + 14, "Promoted");
    free(seats);
    assert(loadBusById(bus.busId, &loadedBus) && loadedBus.availableSeats == 0);

    add_student(studentIdBase + 15);
    assert(requestStudentSeat(bus.busId, studentIdBase + 15, "Any", &seat) == 0);
    assert(cancelStudentSeat(studentIdBase + 3, bus.busId));
    assert(loadAllSeatsDynamic(&seats, &count));
    find_seat(seats, count, bus.busId, 4, studentIdBase + 15, "Booked");
    find_seat(seats, count, bus.busId, 0, studentIdBase + 15, "Promoted");
    free(seats);
    assert(loadBusById(bus.busId, &loadedBus) && loadedBus.availableSeats == 0);

    secondBus.busId = studentIdBase + 1000;
    secondBus.capacity = secondCapacity;
    secondBus.availableSeats = secondCapacity;
    snprintf(secondBus.busNumber, sizeof(secondBus.busNumber), "TEST-%d", secondBus.busId);
    snprintf(secondBus.status, sizeof(secondBus.status), "Active");
    assert(saveBus(secondBus));
    for (i = 0; i <= secondCapacity; i++)
        add_student(studentIdBase + 100 + i);
    assert(getBusSeatCapacity(secondBus.busId) == secondCapacity);
    for (i = 0; i < secondCapacity; i++)
        assert(requestStudentSeat(secondBus.busId, studentIdBase + 100 + i, "Any", &seat) == 1);
    assert(requestStudentSeat(secondBus.busId, studentIdBase + 100 + secondCapacity, "Any", &seat) == 0);
    assert(getAllocatedSeatCount(secondBus.busId) == secondCapacity);
    assert(loadBusById(secondBus.busId, &loadedBus) && loadedBus.availableSeats == 0);

    student.studentId = studentIdBase + 16;
    snprintf(student.name, sizeof(student.name), "Student %d", student.studentId);
    snprintf(student.collegeId, sizeof(student.collegeId), "C%d", student.studentId);
    snprintf(student.password, sizeof(student.password), "test");
    assert(saveStudent(student));
    assert(!saveStudent(student));
    assert(loadStudentById(student.studentId, &loadedStudent));
    assert(deleteStudentById(student.studentId));
    assert(!loadStudentById(student.studentId, &loadedStudent));
    puts("Student seat workflow passed.");
    return 0;
}
