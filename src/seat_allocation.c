#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/seat_allocation.h"
#include "../include/file_handler.h"

static int find_seat_index(
    Seat *seats,
    size_t count,
    int busId,
    int seatNumber)
{
    size_t i;

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId &&
            seats[i].seatNumber == seatNumber)
        {
            return (int)i;
        }
    }

    return -1;
}

int add_seat(Seat seat)
{
    void *records = NULL;
    Seat *seats;
    size_t count = 0;

    if (seat.busId <= 0 || seat.seatNumber <= 0)
    {
        return -1;
    }

    if (seat.status[0] == '\0')
    {
        strcpy(seat.status, "Available");
    }

    if (strcmp(seat.status, "Available") != 0 &&
        strcmp(seat.status, "Occupied") != 0)
    {
        return -1;
    }

    if (strcmp(seat.status, "Available") == 0)
    {
        seat.studentId = 0;
        seat.preference[0] = '\0';
    }

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) == 0)
    {
        seats = (Seat *)records;

        if (find_seat_index(
                seats,
                count,
                seat.busId,
                seat.seatNumber) >= 0)
        {
            free(records);
            return -2;
        }

        free(records);
    }

    if (appendRecord(SEATS_FILE, sizeof(Seat), &seat) != 0)
    {
        return -1;
    }

    return 1;
}

int search_seat(
    int busId,
    int seatNumber,
    Seat *seat)
{
    void *records = NULL;
    Seat *seats;
    size_t count = 0;
    int index;

    if (seat == NULL || busId <= 0 || seatNumber <= 0)
    {
        return -1;
    }

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0)
    {
        return -1;
    }

    seats = (Seat *)records;

    index = find_seat_index(
        seats,
        count,
        busId,
        seatNumber
    );

    if (index >= 0)
    {
        *seat = seats[index];
        free(records);
        return 1;
    }

    free(records);
    return 0;
}

int update_seat(Seat seat)
{
    void *records = NULL;
    Seat *seats;
    size_t count = 0;
    int index;
    int result;

    if (seat.busId <= 0 || seat.seatNumber <= 0)
    {
        return -1;
    }

    if (strcmp(seat.status, "Available") != 0 &&
        strcmp(seat.status, "Occupied") != 0)
    {
        return -1;
    }

    if (strcmp(seat.status, "Available") == 0)
    {
        seat.studentId = 0;
        seat.preference[0] = '\0';
    }

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0)
    {
        return -1;
    }

    seats = (Seat *)records;

    index = find_seat_index(
        seats,
        count,
        seat.busId,
        seat.seatNumber
    );

    if (index < 0)
    {
        free(records);
        return 0;
    }

    seats[index] = seat;

    result = writeRecords(
        SEATS_FILE,
        sizeof(Seat),
        seats,
        count
    );

    free(records);

    return result == 0 ? 1 : -1;
}

int delete_seat(int busId, int seatNumber)
{
    void *records = NULL;
    Seat *seats;
    Seat *new_seats;
    size_t count = 0;
    size_t i;
    size_t new_count = 0;
    int found = 0;
    int index = -1;
    int result;

    if (busId <= 0 || seatNumber <= 0)
    {
        return -1;
    }

    if (readRecords(SEATS_FILE, sizeof(Seat), &records, &count) != 0)
    {
        return -1;
    }

    seats = (Seat *)records;

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId &&
            seats[i].seatNumber == seatNumber)
        {
            found = 1;
            index = (int)i;
            break;
        }
    }

    if (!found)
    {
        free(records);
        return 0;
    }

    if (count == 1)
    {
        result = writeRecords(
            SEATS_FILE,
            sizeof(Seat),
            NULL,
            0
        );

        free(records);
        return result == 0 ? 1 : -1;
    }

    new_seats = malloc(sizeof(Seat) * (count - 1));

    if (new_seats == NULL)
    {
        free(records);
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        if (i != (size_t)index)
        {
            new_seats[new_count] = seats[i];
            new_count++;
        }
    }

    result = writeRecords(
        SEATS_FILE,
        sizeof(Seat),
        new_seats,
        new_count
    );

    free(new_seats);
    free(records);

    return result == 0 ? 1 : -1;
}

int allocate_seat(
    int busId,
    int seatNumber,
    int studentId,
    const char *preference)
{
    Seat seat;

    if (studentId <= 0)
    {
        return -1;
    }

    if (search_seat(busId, seatNumber, &seat) != 1)
    {
        return 0;
    }

    if (strcmp(seat.status, "Available") != 0)
    {
        return -2;
    }

    seat.studentId = studentId;
    seat.status[0] = '\0';
    strcpy(seat.status, "Occupied");

    if (preference != NULL)
    {
        strncpy(
            seat.preference,
            preference,
            sizeof(seat.preference) - 1
        );
        seat.preference[sizeof(seat.preference) - 1] = '\0';
    }
    else
    {
        seat.preference[0] = '\0';
    }

    return update_seat(seat);
}

int release_seat(int busId, int seatNumber)
{
    Seat seat;

    if (search_seat(busId, seatNumber, &seat) != 1)
    {
        return 0;
    }

    strcpy(seat.status, "Available");
    seat.studentId = 0;
    seat.preference[0] = '\0';

    return update_seat(seat);
}
