#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../include/booking.h"
#include "../include/file_handler.h"

int add_booking(Booking booking)
{
    void *records = NULL;
    Booking *bookings;
    size_t count = 0;
    size_t i;

    if (booking.studentId <= 0 || booking.scheduleId <= 0 || booking.seat_number <= 0)
        return -1;

    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) == 0)
    {
        bookings = (Booking *)records;
        for (i = 0; i < count; i++)
        {
            if (bookings[i].studentId == booking.studentId &&
                bookings[i].scheduleId == booking.scheduleId)
            {
                free(records);
                return -2;
            }

            if (bookings[i].scheduleId == booking.scheduleId &&
                bookings[i].seat_number == booking.seat_number)
            {
                free(records);
                return -3;
            }
        }
        free(records);
    }

    if (appendRecord(BOOKINGS_FILE, sizeof(Booking), &booking) != 0)
        return -1;

    return 1;
}

int search_booking(int studentId, int scheduleId, Booking *booking)
{
    void *records = NULL;
    Booking *bookings;
    size_t count = 0;
    size_t i;

    if (booking == NULL)
        return -1;

    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) != 0)
        return -1;

    bookings = (Booking *)records;
    for (i = 0; i < count; i++)
    {
        if (bookings[i].studentId == studentId && bookings[i].scheduleId == scheduleId)
        {
            *booking = bookings[i];
            free(records);
            return 1;
        }
    }

    free(records);
    return 0;
}

int update_booking(Booking booking)
{
    void *records = NULL;
    Booking *bookings;
    size_t count = 0;
    size_t i;
    int found = 0;

    if (booking.studentId <= 0 || booking.scheduleId <= 0 || booking.seat_number <= 0)
        return -1;

    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) != 0)
        return -1;

    bookings = (Booking *)records;
    for (i = 0; i < count; i++)
    {
        if (bookings[i].scheduleId == booking.scheduleId &&
            bookings[i].seat_number == booking.seat_number &&
            (bookings[i].studentId != booking.studentId ||
             bookings[i].scheduleId != booking.scheduleId))
        {
            free(records);
            return -3;
        }

        if (bookings[i].studentId == booking.studentId &&
            bookings[i].scheduleId == booking.scheduleId)
        {
            found = 1;
        }
    }
    free(records);

    if (!found)
        return 0;

    records = NULL;
    count = 0;
    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) != 0)
        return -1;

    bookings = (Booking *)records;
    for (i = 0; i < count; i++)
    {
        if (bookings[i].studentId == booking.studentId &&
            bookings[i].scheduleId == booking.scheduleId)
        {
            bookings[i] = booking;
            break;
        }
    }

    if (writeRecords(BOOKINGS_FILE, sizeof(Booking), bookings, count) != 0)
    {
        free(records);
        return -1;
    }

    free(records);
    return 1;
}

int delete_booking(int studentId, int scheduleId)
{
    void *records = NULL;
    Booking *bookings;
    Booking *filtered;
    size_t count = 0;
    size_t newCount = 0;
    size_t i;
    int found = 0;

    if (studentId <= 0 || scheduleId <= 0)
        return -1;

    if (readRecords(BOOKINGS_FILE, sizeof(Booking), &records, &count) != 0)
        return -1;

    bookings = (Booking *)records;
    filtered = (Booking *)malloc(sizeof(Booking) * count);
    if (filtered == NULL && count > 0)
    {
        free(records);
        return -1;
    }

    for (i = 0; i < count; i++)
    {
        if (bookings[i].studentId == studentId && bookings[i].scheduleId == scheduleId)
        {
            found = 1;
        }
        else
        {
            filtered[newCount++] = bookings[i];
        }
    }

    if (!found)
    {
        free(filtered);
        free(records);
        return 0;
    }

    if (writeRecords(BOOKINGS_FILE, sizeof(Booking), filtered, newCount) != 0)
    {
        free(filtered);
        free(records);
        return -1;
    }

    free(filtered);
    free(records);
    return 1;
}
