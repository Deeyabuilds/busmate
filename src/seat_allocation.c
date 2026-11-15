#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/seat_allocation.h"
#include "../include/file_handler.h"

/* ---------------------------------------------------------
   Helper functions
   --------------------------------------------------------- */

static int get_bus(int busId, Bus *bus)
{
    Bus *buses = NULL;
    size_t count = 0;
    size_t i;

    if (readRecords(BUSES_FILE, sizeof(Bus),
                    (void **)&buses, &count) != 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (buses[i].busId == busId)
        {
            if (bus != NULL)
            {
                *bus = buses[i];
            }

            free(buses);
            return 1;
        }
    }

    free(buses);
    return 0;
}

static int save_all_seats(Seat *seats, size_t count)
{
    if (count == 0)
    {
        return 0;
    }

    return writeRecords(SEATS_FILE,
                        sizeof(Seat),
                        seats,
                        count) == 0;
}

static int get_seat_index(Seat *seats,
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

/* ---------------------------------------------------------
   Initialize seats for a bus
   --------------------------------------------------------- */

int initializeBusSeats(int busId)
{
    Bus bus;
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int existingSeats = 0;

    if (!get_bus(busId, &bus))
    {
        return 0;
    }

    /*
       Missing seats.dat is allowed.
       This is important when the first bus is being created.
    */
    if (readRecords(SEATS_FILE,
                    sizeof(Seat),
                    (void **)&seats,
                    &count) != 0)
    {
        seats = NULL;
        count = 0;
    }

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId)
        {
            existingSeats++;
        }
    }

    /*
       If all seats already exist, nothing to do.
    */
    if (existingSeats == bus.capacity)
    {
        free(seats);
        return 1;
    }

    /*
       Remove incomplete seat data for this bus.
       We rebuild this bus's seat records.
    */
    if (existingSeats > 0)
    {
        size_t newCount = count - (size_t)existingSeats;
        Seat *newSeats = NULL;
        size_t newIndex = 0;

        if (newCount > 0)
        {
            newSeats = malloc(newCount * sizeof(Seat));

            if (newSeats == NULL)
            {
                free(seats);
                return 0;
            }
        }

        for (i = 0; i < count; i++)
        {
            if (seats[i].busId != busId)
            {
                newSeats[newIndex] = seats[i];
                newIndex++;
            }
        }

        free(seats);
        seats = newSeats;
        count = newCount;
    }

    /*
       Create seats 1..capacity.
    */
    {
        size_t newCount = count + (size_t)bus.capacity;
        Seat *newSeats = realloc(
            seats,
            newCount * sizeof(Seat)
        );

        if (newSeats == NULL)
        {
            free(seats);
            return 0;
        }

        seats = newSeats;

        for (i = 0; i < (size_t)bus.capacity; i++)
        {
            size_t index = count + i;

            seats[index].busId = bus.busId;
            seats[index].seatNumber = (int)i + 1;
            strcpy(seats[index].status, "Available");
            seats[index].studentId = 0;
            strcpy(seats[index].preference, "Any");
        }

        count = newCount;
    }

    if (!save_all_seats(seats, count))
    {
        free(seats);
        return 0;
    }

    free(seats);
    return 1;
}

/* ---------------------------------------------------------
   Validate bus seat data
   --------------------------------------------------------- */

int validateBusSeatData(int busId)
{
    Bus bus;
    Seat *seats = NULL;
    size_t count = 0;
    int found = 0;
    int i;

    if (!get_bus(busId, &bus))
    {
        return 0;
    }

    if (readRecords(SEATS_FILE,
                    sizeof(Seat),
                    (void **)&seats,
                    &count) != 0)
    {
        return 0;
    }

    for (i = 1; i <= bus.capacity; i++)
    {
        size_t j;
        int seatFound = 0;

        for (j = 0; j < count; j++)
        {
            if (seats[j].busId == busId &&
                seats[j].seatNumber == i)
            {
                seatFound++;
                found++;
                break;
            }
        }

        if (!seatFound)
        {
            free(seats);
            return 0;
        }
    }

    free(seats);

    return found == bus.capacity;
}

/* ---------------------------------------------------------
   Create bus + seats
   --------------------------------------------------------- */

int createBusWithSeats(const Bus *bus)
{
    Bus *buses = NULL;
    size_t count = 0;
    size_t i;

    if (bus == NULL)
    {
        return 0;
    }

    if (bus->busId <= 0 || bus->capacity <= 0)
    {
        return 0;
    }

    /*
       Missing buses.dat means there are currently no buses.
    */
    if (readRecords(BUSES_FILE,
                    sizeof(Bus),
                    (void **)&buses,
                    &count) != 0)
    {
        buses = NULL;
        count = 0;
    }

    /*
       Check duplicate ID.
    */
    for (i = 0; i < count; i++)
    {
        if (buses[i].busId == bus->busId)
        {
            free(buses);
            return 0;
        }
    }

    /*
       Append the bus.
    */
    if (appendRecord(BUSES_FILE,
                     sizeof(Bus),
                     bus) != 0)
    {
        free(buses);
        return 0;
    }

    free(buses);

    /*
       Create all seats.
    */
    if (!initializeBusSeats(bus->busId))
    {
        /*
           Seat creation failed.
           Try to remove the bus so data does not become inconsistent.
        */
        int deleted = 0;

        deleteRecordById(
            BUSES_FILE,
            sizeof(Bus),
            offsetof(Bus, busId),
            bus->busId,
            &deleted
        );

        return 0;
    }

    /*
       Final validation.
    */
    if (!validateBusSeatData(bus->busId))
    {
        int deleted = 0;

        deleteRecordById(
            BUSES_FILE,
            sizeof(Bus),
            offsetof(Bus, busId),
            bus->busId,
            &deleted
        );

        return 0;
    }

    return 1;
}

/* ---------------------------------------------------------
   Remove bus + seats
   --------------------------------------------------------- */

int removeBusWithSeats(int busId)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    size_t newCount = 0;
    Seat *newSeats = NULL;
    int deleted = 0;

    if (readRecords(SEATS_FILE,
                    sizeof(Seat),
                    (void **)&seats,
                    &count) == 0)
    {
        for (i = 0; i < count; i++)
        {
            if (seats[i].busId != busId)
            {
                newCount++;
            }
        }

        if (newCount > 0)
        {
            newSeats = malloc(newCount * sizeof(Seat));

            if (newSeats == NULL)
            {
                free(seats);
                return 0;
            }

            {
                size_t index = 0;

                for (i = 0; i < count; i++)
                {
                    if (seats[i].busId != busId)
                    {
                        newSeats[index] = seats[i];
                        index++;
                    }
                }
            }

            if (!save_all_seats(newSeats, newCount))
            {
                free(newSeats);
                free(seats);
                return 0;
            }

            free(newSeats);
        }
        else
        {
            /*
               No seat records remain.
               An empty seats.dat is acceptable.
            */
            remove(SEATS_FILE);
        }
    }

    free(seats);

    if (deleteRecordById(
            BUSES_FILE,
            sizeof(Bus),
            offsetof(Bus, busId),
            busId,
            &deleted) != 0)
    {
        return 0;
    }

    return deleted;
}

/* ---------------------------------------------------------
   Get seats of a bus
   --------------------------------------------------------- */

int getBusSeats(int busId,
                Seat **seats,
                size_t *count)
{
    Seat *allSeats = NULL;
    size_t allCount = 0;
    Seat *result = NULL;
    size_t resultCount = 0;
    size_t i;

    if (seats == NULL || count == NULL)
    {
        return 0;
    }

    *seats = NULL;
    *count = 0;

    /*
       Make sure seats exist.
    */
    if (!validateBusSeatData(busId))
    {
        if (!initializeBusSeats(busId))
        {
            return 0;
        }
    }

    if (readRecords(SEATS_FILE,
                    sizeof(Seat),
                    (void **)&allSeats,
                    &allCount) != 0)
    {
        return 0;
    }

    for (i = 0; i < allCount; i++)
    {
        if (allSeats[i].busId == busId)
        {
            resultCount++;
        }
    }

    if (resultCount == 0)
    {
        free(allSeats);
        return 0;
    }

    result = malloc(resultCount * sizeof(Seat));

    if (result == NULL)
    {
        free(allSeats);
        return 0;
    }

    {
        size_t index = 0;

        for (i = 0; i < allCount; i++)
        {
            if (allSeats[i].busId == busId)
            {
                result[index] = allSeats[i];
                index++;
            }
        }
    }

    free(allSeats);

    *seats = result;
    *count = resultCount;

    return 1;
}

/* ---------------------------------------------------------
   Get occupancy
   --------------------------------------------------------- */

int getBusOccupancy(int busId,
                    int *bookedSeats,
                    int *capacity)
{
    Bus bus;
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int booked = 0;

    if (bookedSeats == NULL || capacity == NULL)
    {
        return 0;
    }

    if (!get_bus(busId, &bus))
    {
        return 0;
    }

    if (!getBusSeats(busId, &seats, &count))
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(seats[i].status, "Booked") == 0)
        {
            booked++;
        }
    }

    free(seats);

    *bookedSeats = booked;
    *capacity = bus.capacity;

    return 1;
}

/* ---------------------------------------------------------
   Set seat preference
   --------------------------------------------------------- */

int setSeatPreference(int busId,
                      int seatNumber,
                      const char *preference)
{
    Seat *seats = NULL;
    size_t count = 0;
    int index;

    if (preference == NULL)
    {
        return 0;
    }

    if (!getBusSeats(busId, &seats, &count))
    {
        return 0;
    }

    index = get_seat_index(
        seats,
        count,
        busId,
        seatNumber
    );

    if (index < 0)
    {
        free(seats);
        return 0;
    }

    strncpy(
        seats[index].preference,
        preference,
        sizeof(seats[index].preference) - 1
    );

    seats[index].preference[
        sizeof(seats[index].preference) - 1
    ] = '\0';

    if (!save_all_seats(seats, count))
    {
        free(seats);
        return 0;
    }

    free(seats);
    return 1;
}

/* ---------------------------------------------------------
   Allocate any available seat
   --------------------------------------------------------- */

int allocateSeat(int busId,
                 int studentId,
                 const char *preference,
                 Seat *allocatedSeat)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int selected = -1;

    if (!getBusSeats(busId, &seats, &count))
    {
        return SEAT_ALLOCATION_ERROR;
    }

    /*
       First try to satisfy preference.
    */
    if (preference != NULL &&
        strcmp(preference, "Any") != 0)
    {
        for (i = 0; i < count; i++)
        {
            if (strcmp(seats[i].status, "Available") == 0 &&
                strcmp(seats[i].preference, preference) == 0)
            {
                selected = (int)i;
                break;
            }
        }
    }

    /*
       If preferred seat is not available,
       take the first available seat.
    */
    if (selected == -1)
    {
        for (i = 0; i < count; i++)
        {
            if (strcmp(seats[i].status, "Available") == 0)
            {
                selected = (int)i;
                break;
            }
        }
    }

    if (selected == -1)
    {
        free(seats);
        return SEAT_ALLOCATION_WAITLISTED;
    }

    seats[selected].studentId = studentId;
    strcpy(seats[selected].status, "Booked");

    if (preference != NULL)
    {
        strncpy(
            seats[selected].preference,
            preference,
            sizeof(seats[selected].preference) - 1
        );

        seats[selected].preference[
            sizeof(seats[selected].preference) - 1
        ] = '\0';
    }

    if (!save_all_seats(seats, count))
    {
        free(seats);
        return SEAT_ALLOCATION_ERROR;
    }

    if (allocatedSeat != NULL)
    {
        *allocatedSeat = seats[selected];
    }

    free(seats);

    return SEAT_ALLOCATION_BOOKED;
}

/* ---------------------------------------------------------
   Allocate a specific seat
   --------------------------------------------------------- */

int allocateSpecificSeat(int busId,
                         int studentId,
                         int seatNumber,
                         const char *preference,
                         Seat *allocatedSeat)
{
    Seat *seats = NULL;
    size_t count = 0;
    int index;

    if (!getBusSeats(busId, &seats, &count))
    {
        return SEAT_ALLOCATION_ERROR;
    }

    index = get_seat_index(
        seats,
        count,
        busId,
        seatNumber
    );

    if (index < 0)
    {
        free(seats);
        return SEAT_ALLOCATION_ERROR;
    }

    if (strcmp(seats[index].status, "Booked") == 0)
    {
        free(seats);
        return SEAT_ALLOCATION_DUPLICATE;
    }

    seats[index].studentId = studentId;
    strcpy(seats[index].status, "Booked");

    if (preference != NULL)
    {
        strncpy(
            seats[index].preference,
            preference,
            sizeof(seats[index].preference) - 1
        );

        seats[index].preference[
            sizeof(seats[index].preference) - 1
        ] = '\0';
    }

    if (!save_all_seats(seats, count))
    {
        free(seats);
        return SEAT_ALLOCATION_ERROR;
    }

    if (allocatedSeat != NULL)
    {
        *allocatedSeat = seats[index];
    }

    free(seats);

    return SEAT_ALLOCATION_BOOKED;
}

/* ---------------------------------------------------------
   Cancel seat
   --------------------------------------------------------- */

SeatCancellationResult cancelSeat(int busId,
                                  int studentId,
                                  Seat *freedSeat,
                                  int *promotedStudentId)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;

    if (promotedStudentId != NULL)
    {
        *promotedStudentId = 0;
    }

    if (!getBusSeats(busId, &seats, &count))
    {
        return SEAT_CANCELLATION_ERROR;
    }

    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId &&
            seats[i].studentId == studentId &&
            strcmp(seats[i].status, "Booked") == 0)
        {
            if (freedSeat != NULL)
            {
                *freedSeat = seats[i];
            }

            seats[i].studentId = 0;
            strcpy(seats[i].status, "Available");
            strcpy(seats[i].preference, "Any");

            if (!save_all_seats(seats, count))
            {
                free(seats);
                return SEAT_CANCELLATION_ERROR;
            }

            free(seats);
            return SEAT_CANCELLATION_COMPLETED;
        }
    }

    free(seats);

    return SEAT_CANCELLATION_NOT_FOUND;
}

/* ---------------------------------------------------------
   Waiting list
   --------------------------------------------------------- */

int getWaitingList(int busId,
                   int **studentIds,
                   size_t *count)
{
    /*
       Current BusMate data model does not store
       waiting-list records in a separate file.

       Return an empty waiting list safely.
    */
    (void)busId;

    if (studentIds == NULL || count == NULL)
    {
        return 0;
    }

    *studentIds = NULL;
    *count = 0;

    return 1;
}

/* ---------------------------------------------------------
   Display occupancy
   --------------------------------------------------------- */

int displayBusOccupancy(int busId,
                        FILE *output)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int booked = 0;

    if (output == NULL)
    {
        output = stdout;
    }

    if (!getBusSeats(busId, &seats, &count))
    {
        fprintf(output, "Unable to load bus seats.\n");
        return -1;
    }

    fprintf(output, "\nBus ID: %d\n", busId);
    fprintf(output, "-------------------------\n");

    for (i = 0; i < count; i++)
    {
        fprintf(
            output,
            "Seat %d : %s",
            seats[i].seatNumber,
            seats[i].status
        );

        if (seats[i].studentId != 0)
        {
            fprintf(
                output,
                " (Student %d)",
                seats[i].studentId
            );

            booked++;
        }

        fprintf(output, "\n");
    }

    fprintf(
        output,
        "\nOccupied: %d / %zu\n",
        booked,
        count
    );

    fprintf(
        output,
        "Available: %zu\n",
        count - (size_t)booked
    );

    free(seats);

    return 0;
}

/* ---------------------------------------------------------
   Display waiting list
   --------------------------------------------------------- */

int displayWaitingList(int busId,
                       FILE *output)
{
    int *studentIds = NULL;
    size_t count = 0;
    size_t i;

    if (output == NULL)
    {
        output = stdout;
    }

    if (!getWaitingList(busId, &studentIds, &count))
    {
        fprintf(output, "Unable to load waiting list.\n");
        return -1;
    }

    fprintf(output, "\nWaiting List - Bus %d\n", busId);
    fprintf(output, "-------------------------\n");

    if (count == 0)
    {
        fprintf(output, "Waiting list is empty.\n");
        free(studentIds);
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        fprintf(
            output,
            "%zu. Student ID: %d\n",
            i + 1,
            studentIds[i]
        );
    }

    free(studentIds);

    return 0;
}

/* ---------------------------------------------------------
   Legacy / academic functions
   --------------------------------------------------------- */

int allocate_seat(Booking *booking)
{
    Seat allocatedSeat;
    int result;
    int busId = 0;

    if (booking == NULL)
    {
        return 0;
    }

    /*
       Booking currently contains studentId,
       scheduleId and seat_number.
       The schedule -> bus relationship can be handled
       by the higher-level booking module.
       This legacy function therefore uses the scheduleId
       as the bus identifier when called independently.
    */
    busId = booking->scheduleId;

    if (booking->seat_number > 0)
    {
        result = allocateSpecificSeat(
            busId,
            booking->studentId,
            booking->seat_number,
            "Any",
            &allocatedSeat
        );
    }
    else
    {
        result = allocateSeat(
            busId,
            booking->studentId,
            "Any",
            &allocatedSeat
        );

        if (result == SEAT_ALLOCATION_BOOKED)
        {
            booking->seat_number = allocatedSeat.seatNumber;
        }
    }

    return result == SEAT_ALLOCATION_BOOKED;
}

/* ---------------------------------------------------------
   Check seat availability - legacy interface
   --------------------------------------------------------- */

int check_seat_availability(const char *schedule_id,
                            int seat_number)
{
    Seat *seats = NULL;
    size_t count = 0;
    int busId;

    if (schedule_id == NULL)
    {
        return 0;
    }

    busId = atoi(schedule_id);

    if (!getBusSeats(busId, &seats, &count))
    {
        return 0;
    }

    {
        int index = get_seat_index(
            seats,
            count,
            busId,
            seat_number
        );

        if (index >= 0)
        {
            int available =
                strcmp(
                    seats[index].status,
                    "Available"
                ) == 0;

            free(seats);
            return available;
        }
    }

    free(seats);
    return 0;
}

/* ---------------------------------------------------------
   Get seat availability
   --------------------------------------------------------- */

int get_seat_availability(const char *schedule_id,
                          SeatAvailability *availability)
{
    Seat *seats = NULL;
    size_t count = 0;
    size_t i;
    int busId;
    int occupied = 0;

    if (schedule_id == NULL ||
        availability == NULL)
    {
        return 0;
    }

    busId = atoi(schedule_id);

    if (!getBusSeats(busId, &seats, &count))
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        if (strcmp(seats[i].status, "Booked") == 0)
        {
            occupied++;
        }
    }

    availability->total_seats = (int)count;
    availability->occupied_seats = occupied;
    availability->available_seats =
        (int)count - occupied;

    free(seats);

    return 1;
}

/* ---------------------------------------------------------
   Display seat availability
   --------------------------------------------------------- */

void display_seat_availability(const char *schedule_id)
{
    SeatAvailability availability;

    if (!get_seat_availability(
            schedule_id,
            &availability))
    {
        printf("Unable to get seat availability.\n");
        return;
    }

    printf("\nSeat Availability\n");
    printf("-------------------------\n");
    printf(
        "Total Seats     : %d\n",
        availability.total_seats
    );
    printf(
        "Occupied Seats  : %d\n",
        availability.occupied_seats
    );
    printf(
        "Available Seats : %d\n",
        availability.available_seats
    );
}

/* ---------------------------------------------------------
   Display waiting list - legacy interface
   --------------------------------------------------------- */

void display_waiting_list(const char *schedule_id)
{
    int busId;

    if (schedule_id == NULL)
    {
        return;
    }

    busId = atoi(schedule_id);

    displayWaitingList(
        busId,
        stdout
    );
}

/* ---------------------------------------------------------
   Cancel seat - legacy interface
   --------------------------------------------------------- */

int cancel_seat(const char *student_id,
                const char *schedule_id)
{
    int studentId;
    int busId;
    Seat freedSeat;
    int promotedStudentId = 0;

    if (student_id == NULL ||
        schedule_id == NULL)
    {
        return 0;
    }

    studentId = atoi(student_id);
    busId = atoi(schedule_id);

    return cancelSeat(
        busId,
        studentId,
        &freedSeat,
        &promotedStudentId
    ) == SEAT_CANCELLATION_COMPLETED;
}