#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file_handler.h"
#include "../include/seat_allocation.h"

static int valid_text(const char *text, size_t capacity)
{
    return text != NULL && text[0] != '\0' &&
           memchr(text, '\0', capacity) != NULL;
}

static FileStatus read_bookings(Booking **bookings, size_t *count)
{
    void *records = NULL;
    size_t index;
    FileStatus status = file_read_records(FILE_BOOKINGS_PATH, &records,
                                          sizeof(Booking), count);

    *bookings = (Booking *)records;
    if (status == FILE_STATUS_NOT_FOUND)
    {
        *bookings = NULL;
        *count = 0;
        return FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
        return status;

    for (index = 0; index < *count; ++index)
    {
        if (!valid_text((*bookings)[index].student_id,
                        sizeof((*bookings)[index].student_id)) ||
            !valid_text((*bookings)[index].schedule_id,
                        sizeof((*bookings)[index].schedule_id)) ||
            (*bookings)[index].seat_number <= 0)
        {
            free(*bookings);
            *bookings = NULL;
            *count = 0;
            return FILE_STATUS_CORRUPT;
        }
    }
    return status;
}

static FileStatus read_waitlist(SeatWaitingListEntry **entries, size_t *count)
{
    void *records = NULL;
    size_t index;
    FileStatus status = file_read_records(FILE_WAITLIST_PATH, &records,
                                          sizeof(SeatWaitingListEntry), count);

    *entries = (SeatWaitingListEntry *)records;
    if (status == FILE_STATUS_NOT_FOUND)
    {
        *entries = NULL;
        *count = 0;
        return FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
        return status;

    for (index = 0; index < *count; ++index)
    {
        Booking *booking = &(*entries)[index].booking;
        if (!valid_text(booking->student_id, sizeof(booking->student_id)) ||
            !valid_text(booking->schedule_id, sizeof(booking->schedule_id)) ||
            booking->seat_number != 0 ||
            (*entries)[index].preference < SEAT_PREFERENCE_ANY ||
            (*entries)[index].preference > SEAT_PREFERENCE_FRONT)
        {
            free(*entries);
            *entries = NULL;
            *count = 0;
            return FILE_STATUS_CORRUPT;
        }
    }
    return status;
}

static int get_schedule_capacity(const char *schedule_id, int *capacity)
{
    Schedule schedule;
    Bus bus;
    FileStatus status;

    status = file_find_record(FILE_SCHEDULES_PATH, schedule_id, &schedule,
                              sizeof(schedule),
                              offsetof(Schedule, schedule_id),
                              sizeof(schedule.schedule_id));
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Cannot find schedule '%s': %s\n", schedule_id,
                file_status_message(status));
        return 0;
    }

    if (!valid_text(schedule.bus_id, sizeof(schedule.bus_id)))
    {
        fprintf(stderr, "Schedule '%s' has an invalid bus ID.\n", schedule_id);
        return 0;
    }

    status = file_find_record(FILE_BUSES_PATH, schedule.bus_id, &bus,
                              sizeof(bus), offsetof(Bus, bus_id),
                              sizeof(bus.bus_id));
    if (status != FILE_STATUS_OK || bus.capacity <= 0)
    {
        fprintf(stderr, "Cannot determine bus capacity for schedule '%s'.\n",
                schedule_id);
        return 0;
    }

    *capacity = bus.capacity;
    return 1;
}

static int sync_shared_state(const char *student_id, const char *schedule_id,
                             SeatPreference preference)
{
    BusMateSessionState state = {{0}, {0}, {0}};
    Schedule schedule;
    void *records = NULL;
    void *old_seats = NULL;
    SeatWaitingListEntry *seat_records = NULL;
    size_t count = 0;
    size_t old_seat_count = 0;
    size_t index;
    FileStatus status;

    status = file_find_record(FILE_SCHEDULES_PATH, schedule_id, &schedule,
                              sizeof(schedule),
                              offsetof(Schedule, schedule_id),
                              sizeof(schedule.schedule_id));
    if (status != FILE_STATUS_OK)
        goto failed;
    if (!valid_text(schedule.bus_id, sizeof(schedule.bus_id)))
    {
        status = FILE_STATUS_CORRUPT;
        goto failed;
    }

    status = file_read_records(FILE_BOOKINGS_PATH, &records, sizeof(Booking),
                               &count);
    if (status == FILE_STATUS_NOT_FOUND)
    {
        records = NULL;
        count = 0;
        status = FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
        goto failed;

    status = file_read_records(FILE_SEATS_PATH, &old_seats,
                               sizeof(SeatWaitingListEntry), &old_seat_count);
    if (status == FILE_STATUS_NOT_FOUND)
    {
        old_seats = NULL;
        old_seat_count = 0;
        status = FILE_STATUS_OK;
    }
    else if (status == FILE_STATUS_CORRUPT)
    {
        fprintf(stderr, "Existing seat snapshot has an older or invalid "
                        "record format; preferences default to Any.\n");
        free(old_seats);
        old_seats = NULL;
        old_seat_count = 0;
        status = FILE_STATUS_OK;
    }
    if (status != FILE_STATUS_OK)
        goto failed;

    if (count > 0)
    {
        if (count > (size_t)-1 / sizeof(*seat_records))
        {
            status = FILE_STATUS_IO_ERROR;
            goto failed;
        }
        seat_records = (SeatWaitingListEntry *)calloc(
            count, sizeof(*seat_records));
        if (seat_records == NULL)
        {
            status = FILE_STATUS_IO_ERROR;
            goto failed;
        }
    }
    for (index = 0; index < count; ++index)
    {
        Booking *bookings = (Booking *)records;
        SeatPreference saved_preference = SEAT_PREFERENCE_ANY;
        size_t old_index;

        seat_records[index].booking = bookings[index];
        if (strcmp(bookings[index].student_id, student_id) == 0 &&
            strcmp(bookings[index].schedule_id, schedule_id) == 0)
            saved_preference = preference;
        else
        {
            for (old_index = 0; old_index < old_seat_count; ++old_index)
            {
                SeatWaitingListEntry *old =
                    (SeatWaitingListEntry *)old_seats;
                if (strcmp(old[old_index].booking.student_id,
                           bookings[index].student_id) == 0 &&
                    strcmp(old[old_index].booking.schedule_id,
                           bookings[index].schedule_id) == 0 &&
                    old[old_index].booking.seat_number ==
                        bookings[index].seat_number)
                {
                    if (old[old_index].preference >= SEAT_PREFERENCE_ANY &&
                        old[old_index].preference <= SEAT_PREFERENCE_FRONT)
                        saved_preference = old[old_index].preference;
                    break;
                }
            }
        }
        seat_records[index].preference = saved_preference;
    }

    snprintf(state.student_id, sizeof(state.student_id), "%s", student_id);
    snprintf(state.bus_id, sizeof(state.bus_id), "%s", schedule.bus_id);
    snprintf(state.schedule_id, sizeof(state.schedule_id), "%s", schedule_id);

    status = file_replace_records(FILE_SEATS_PATH, seat_records,
                                  sizeof(*seat_records), count);
    if (status == FILE_STATUS_OK)
        status = file_write_session_state(&state);
    free(records);
    free(old_seats);
    free(seat_records);
    records = NULL;
    old_seats = NULL;
    seat_records = NULL;
    if (status == FILE_STATUS_OK)
        return 1;

failed:
    free(records);
    free(old_seats);
    free(seat_records);
    fprintf(stderr, "Could not synchronize shared seat state: %s\n",
            file_status_message(status));
    return 0;
}

static int booking_matches(const Booking *booking,
                           const char *student_id,
                           const char *schedule_id)
{
    return strcmp(booking->student_id, student_id) == 0 &&
           strcmp(booking->schedule_id, schedule_id) == 0;
}

static int seat_is_occupied(const Booking *bookings, size_t count,
                            const char *schedule_id, int seat_number)
{
    size_t index;

    for (index = 0; index < count; ++index)
    {
        if (strcmp(bookings[index].schedule_id, schedule_id) == 0 &&
            bookings[index].seat_number == seat_number)
            return 1;
    }
    return 0;
}

static int choose_seat(const Booking *bookings, size_t count,
                       const char *schedule_id, int capacity,
                       SeatPreference preference)
{
    int seat;

    if (preference == SEAT_PREFERENCE_FRONT)
    {
        int front_limit =
            (capacity / 10) * 3 + ((capacity % 10) * 3 + 9) / 10;
        for (seat = 1; seat <= front_limit; ++seat)
        {
            if (!seat_is_occupied(bookings, count, schedule_id, seat))
                return seat;
        }
    }
    else if (preference == SEAT_PREFERENCE_WINDOW)
    {
        for (seat = 1; seat <= capacity; ++seat)
        {
            if ((seat % 2) != 0 &&
                !seat_is_occupied(bookings, count, schedule_id, seat))
                return seat;
        }
    }

    for (seat = 1; seat <= capacity; ++seat)
    {
        if (!seat_is_occupied(bookings, count, schedule_id, seat))
            return seat;
    }
    return 0;
}

static int has_duplicate_request(const Booking *bookings, size_t booking_count,
                                 const SeatWaitingListEntry *entries,
                                 size_t entry_count, const Booking *request)
{
    size_t index;

    for (index = 0; index < booking_count; ++index)
    {
        if (booking_matches(&bookings[index], request->student_id,
                            request->schedule_id))
            return 1;
    }
    for (index = 0; index < entry_count; ++index)
    {
        if (booking_matches(&entries[index].booking, request->student_id,
                            request->schedule_id))
            return 1;
    }
    return 0;
}

static int remove_waitlist_entry(size_t remove_index)
{
    SeatWaitingListEntry *entries = NULL;
    SeatWaitingListEntry *remaining = NULL;
    size_t count = 0;
    size_t index;
    size_t kept = 0;
    FileStatus status = read_waitlist(&entries, &count);

    if (status != FILE_STATUS_OK)
        return 0;
    if (remove_index >= count)
    {
        free(entries);
        return 0;
    }

    if (count > 1)
    {
        remaining = (SeatWaitingListEntry *)malloc(
            (count - 1) * sizeof(SeatWaitingListEntry));
        if (remaining == NULL)
        {
            free(entries);
            return 0;
        }
    }

    for (index = 0; index < count; ++index)
    {
        if (index != remove_index)
            remaining[kept++] = entries[index];
    }

    status = file_replace_records(FILE_WAITLIST_PATH, remaining,
                                  sizeof(SeatWaitingListEntry), kept);
    free(remaining);
    free(entries);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not update waiting list: %s\n",
                file_status_message(status));
        return 0;
    }
    return 1;
}

static int promote_waiting_student(const char *schedule_id, int capacity)
{
    SeatWaitingListEntry *entries = NULL;
    Booking *bookings = NULL;
    FileStatus status;

    for (;;)
    {
        size_t entry_count = 0;
        size_t booking_count = 0;
        size_t index;
        int repaired_entry = 0;

        entries = NULL;
        bookings = NULL;
        status = read_waitlist(&entries, &entry_count);
        if (status != FILE_STATUS_OK)
            goto failed;
        status = read_bookings(&bookings, &booking_count);
        if (status != FILE_STATUS_OK)
            goto failed;

        for (index = 0; index < entry_count; ++index)
        {
            Booking promoted;
            SeatPreference promoted_preference;
            int seat;
            size_t existing_index;

            if (strcmp(entries[index].booking.schedule_id, schedule_id) != 0)
                continue;

            for (existing_index = 0; existing_index < booking_count;
                 ++existing_index)
            {
                if (booking_matches(&bookings[existing_index],
                                    entries[index].booking.student_id,
                                    schedule_id))
                {
                    if (!remove_waitlist_entry(index))
                    {
                        status = FILE_STATUS_IO_ERROR;
                        goto failed;
                    }
                    repaired_entry = 1;
                    break;
                }
            }

            if (repaired_entry)
                break;

            seat = choose_seat(bookings, booking_count, schedule_id, capacity,
                               entries[index].preference);
            if (seat == 0)
            {
                free(entries);
                free(bookings);
                return 1;
            }

            promoted = entries[index].booking;
            promoted_preference = entries[index].preference;
            promoted.seat_number = seat;
            status = file_append_record(FILE_BOOKINGS_PATH, &promoted,
                                        sizeof(promoted));
            if (status != FILE_STATUS_OK)
                goto failed;

            if (!remove_waitlist_entry(index))
            {
                status = FILE_STATUS_IO_ERROR;
                goto failed;
            }

            free(entries);
            free(bookings);
            return sync_shared_state(promoted.student_id, schedule_id,
                                     promoted_preference);
        }

        free(entries);
        free(bookings);
        if (!repaired_entry)
            return 1;
    }

failed:
    fprintf(stderr, "Waiting-list promotion failed: %s\n",
            file_status_message(status));
    free(entries);
    free(bookings);
    return 0;
}

SeatAllocationResult allocate_seat_with_preference(
    Booking *booking, SeatPreference preference)
{
    Booking *bookings = NULL;
    SeatWaitingListEntry *entries = NULL;
    size_t booking_count = 0;
    size_t entry_count = 0;
    size_t booked_count = 0;
    size_t index;
    int capacity;
    int seat;
    FileStatus status;

    if (booking == NULL ||
        !valid_text(booking->student_id, sizeof(booking->student_id)) ||
        !valid_text(booking->schedule_id, sizeof(booking->schedule_id)) ||
        preference < SEAT_PREFERENCE_ANY ||
        preference > SEAT_PREFERENCE_FRONT)
        return SEAT_ALLOCATION_ERROR;

    if (!get_schedule_capacity(booking->schedule_id, &capacity))
        return SEAT_ALLOCATION_ERROR;

    status = read_bookings(&bookings, &booking_count);
    if (status != FILE_STATUS_OK)
        goto failed;
    status = read_waitlist(&entries, &entry_count);
    if (status != FILE_STATUS_OK)
        goto failed;

    if (has_duplicate_request(bookings, booking_count, entries, entry_count,
                              booking))
    {
        free(bookings);
        free(entries);
        return SEAT_ALLOCATION_DUPLICATE;
    }

    for (index = 0; index < booking_count; ++index)
    {
        size_t previous;
        if (strcmp(bookings[index].schedule_id, booking->schedule_id) != 0)
            continue;
        if (bookings[index].seat_number > capacity)
        {
            fprintf(stderr, "Booking exceeds the bus capacity.\n");
            free(bookings);
            free(entries);
            return SEAT_ALLOCATION_ERROR;
        }
        for (previous = 0; previous < index; ++previous)
        {
            if (strcmp(bookings[previous].schedule_id,
                       booking->schedule_id) == 0 &&
                bookings[previous].seat_number == bookings[index].seat_number)
            {
                fprintf(stderr, "Duplicate seat found in booking data.\n");
                free(bookings);
                free(entries);
                return SEAT_ALLOCATION_ERROR;
            }
        }
        ++booked_count;
    }

    seat = booked_count >= (size_t)capacity
               ? 0
               : choose_seat(bookings, booking_count, booking->schedule_id,
                             capacity, preference);
    if (seat > 0)
    {
        booking->seat_number = seat;
        status = file_append_record(FILE_BOOKINGS_PATH, booking,
                                    sizeof(*booking));
        free(bookings);
        free(entries);
        if (status != FILE_STATUS_OK)
        {
            fprintf(stderr, "Could not save seat booking: %s\n",
                    file_status_message(status));
            return SEAT_ALLOCATION_ERROR;
        }
        if (!sync_shared_state(booking->student_id, booking->schedule_id,
                               preference))
            return SEAT_ALLOCATION_ERROR;
        return SEAT_ALLOCATION_BOOKED;
    }

    booking->seat_number = 0;
    {
        SeatWaitingListEntry entry;
        entry.booking = *booking;
        entry.preference = preference;
        status = file_append_record(FILE_WAITLIST_PATH, &entry,
                                    sizeof(entry));
    }
    free(bookings);
    free(entries);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not save waiting-list request: %s\n",
                file_status_message(status));
        return SEAT_ALLOCATION_ERROR;
    }
    if (!sync_shared_state(booking->student_id, booking->schedule_id,
                           preference))
        return SEAT_ALLOCATION_ERROR;
    return SEAT_ALLOCATION_WAITLISTED;

failed:
    fprintf(stderr, "Could not load seat records: %s\n",
            file_status_message(status));
    free(bookings);
    free(entries);
    return SEAT_ALLOCATION_ERROR;
}

int allocate_seat(Booking *booking)
{
    return (int)allocate_seat_with_preference(booking, SEAT_PREFERENCE_ANY);
}

int check_seat_availability(const char *schedule_id, int seat_number)
{
    Booking *bookings = NULL;
    size_t count = 0;
    int capacity;
    FileStatus status;
    int available;

    if (!valid_text(schedule_id, 20) || seat_number <= 0 ||
        !get_schedule_capacity(schedule_id, &capacity) ||
        seat_number > capacity)
        return 0;

    status = read_bookings(&bookings, &count);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not check seat availability: %s\n",
                file_status_message(status));
        return 0;
    }

    available = !seat_is_occupied(bookings, count, schedule_id, seat_number);
    free(bookings);
    return available;
}

int get_seat_availability(const char *schedule_id,
                          SeatAvailability *availability)
{
    Booking *bookings = NULL;
    size_t count = 0;
    size_t index;
    int capacity;
    int occupied = 0;
    FileStatus status;

    if (availability == NULL || !valid_text(schedule_id, 20) ||
        !get_schedule_capacity(schedule_id, &capacity))
        return 0;

    status = read_bookings(&bookings, &count);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not read seat bookings: %s\n",
                file_status_message(status));
        return 0;
    }

    for (index = 0; index < count; ++index)
    {
        size_t previous;
        if (strcmp(bookings[index].schedule_id, schedule_id) != 0)
            continue;
        if (bookings[index].seat_number < 1 ||
            bookings[index].seat_number > capacity)
        {
            free(bookings);
            fprintf(stderr, "Invalid seat number found in booking data.\n");
            return 0;
        }
        for (previous = 0; previous < index; ++previous)
        {
            if (strcmp(bookings[previous].schedule_id, schedule_id) == 0 &&
                bookings[previous].seat_number == bookings[index].seat_number)
            {
                free(bookings);
                fprintf(stderr, "Duplicate seat found in booking data.\n");
                return 0;
            }
        }
        ++occupied;
    }

    free(bookings);
    availability->total_seats = capacity;
    availability->occupied_seats = occupied;
    availability->available_seats = capacity - occupied;
    return 1;
}

void display_seat_availability(const char *schedule_id)
{
    SeatAvailability availability;
    int seat;

    if (!get_seat_availability(schedule_id, &availability))
        return;

    printf("\nSchedule %s: %d total, %d occupied, %d available\n",
           schedule_id, availability.total_seats,
           availability.occupied_seats, availability.available_seats);
    for (seat = 1; seat <= availability.total_seats; ++seat)
        printf("Seat %d: %s\n", seat,
               check_seat_availability(schedule_id, seat)
                   ? "Available"
                   : "Booked");
}

void display_waiting_list(const char *schedule_id)
{
    SeatWaitingListEntry *entries = NULL;
    size_t count = 0;
    size_t index;
    size_t position = 0;
    FileStatus status;

    if (!valid_text(schedule_id, 20))
    {
        fprintf(stderr, "Invalid schedule ID.\n");
        return;
    }

    status = read_waitlist(&entries, &count);
    if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not read waiting list: %s\n",
                file_status_message(status));
        return;
    }

    printf("\nWaiting list for schedule %s:\n", schedule_id);
    for (index = 0; index < count; ++index)
    {
        const char *preference;
        if (strcmp(entries[index].booking.schedule_id, schedule_id) != 0)
            continue;
        if (entries[index].preference == SEAT_PREFERENCE_WINDOW)
            preference = "window";
        else if (entries[index].preference == SEAT_PREFERENCE_FRONT)
            preference = "front";
        else
            preference = "any";
        printf("%lu. Student %s (preference: %s)\n",
               (unsigned long)++position,
               entries[index].booking.student_id, preference);
    }
    if (position == 0)
        printf("No students are waiting.\n");
    free(entries);
}

int cancel_seat(const char *student_id, const char *schedule_id)
{
    Booking *bookings = NULL;
    Booking *remaining = NULL;
    SeatWaitingListEntry *entries = NULL;
    SeatWaitingListEntry *remaining_entries = NULL;
    size_t booking_count = 0;
    size_t entry_count = 0;
    size_t kept_bookings = 0;
    size_t kept_entries = 0;
    size_t index;
    int found = 0;
    int capacity;
    FileStatus status;

    if (!valid_text(student_id, 20) ||
        !valid_text(schedule_id, 20) ||
        !get_schedule_capacity(schedule_id, &capacity))
        return 0;

    status = read_bookings(&bookings, &booking_count);
    if (status != FILE_STATUS_OK)
        goto failed;
    status = read_waitlist(&entries, &entry_count);
    if (status != FILE_STATUS_OK)
        goto failed;

    if (booking_count > 0)
    {
        remaining = (Booking *)malloc(booking_count * sizeof(Booking));
        if (remaining == NULL)
            goto failed_memory;
    }
    if (entry_count > 0)
    {
        remaining_entries = (SeatWaitingListEntry *)malloc(
            entry_count * sizeof(SeatWaitingListEntry));
        if (remaining_entries == NULL)
            goto failed_memory;
    }

    for (index = 0; index < booking_count; ++index)
    {
        if (booking_matches(&bookings[index], student_id, schedule_id))
            found = 1;
        else
            remaining[kept_bookings++] = bookings[index];
    }
    for (index = 0; index < entry_count; ++index)
    {
        if (booking_matches(&entries[index].booking, student_id, schedule_id))
            found = 1;
        else
            remaining_entries[kept_entries++] = entries[index];
    }

    if (!found)
    {
        status = FILE_STATUS_NOT_FOUND;
        goto failed;
    }

    if (kept_bookings != booking_count)
    {
        status = file_replace_records(FILE_BOOKINGS_PATH, remaining,
                                      sizeof(Booking), kept_bookings);
        if (status != FILE_STATUS_OK)
            goto failed;
    }
    if (kept_entries != entry_count)
    {
        status = file_replace_records(FILE_WAITLIST_PATH, remaining_entries,
                                      sizeof(SeatWaitingListEntry),
                                      kept_entries);
        if (status != FILE_STATUS_OK)
            goto failed;
    }

    free(bookings);
    free(remaining);
    free(entries);
    free(remaining_entries);

    if (kept_bookings != booking_count &&
        !sync_shared_state(student_id, schedule_id, SEAT_PREFERENCE_ANY))
        return 0;
    if (kept_bookings != booking_count &&
        !promote_waiting_student(schedule_id, capacity))
        return 0;
    return 1;

failed_memory:
    status = FILE_STATUS_IO_ERROR;
failed:
    fprintf(stderr, "Could not cancel seat: %s\n",
            file_status_message(status));
    free(bookings);
    free(remaining);
    free(entries);
    free(remaining_entries);
    return 0;
}