#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_handler.h"
#include "seat_allocation.h"
#include "busmate_config.h"
#include "admin.h"

static int preference_matches(int seatNumber, int capacity, const char *preference)
{
    int row;
    int column;
    int rowCount;

    if (BUSMATE_SEATS_PER_ROW <= 0 || seatNumber < 1 || seatNumber > capacity)
        return 0;
    row = (seatNumber - 1) / BUSMATE_SEATS_PER_ROW;
    column = (seatNumber - 1) % BUSMATE_SEATS_PER_ROW;
    rowCount = 1 + (capacity - 1) / BUSMATE_SEATS_PER_ROW;

    if (strcmp(preference, "Front") == 0 || strcmp(preference, "FRONT") == 0 ||
        strcmp(preference, "Front Row") == 0)
        return row == 0;
    if (strcmp(preference, "Middle Row") == 0)
        return row == (rowCount - 1) / 2 || row == rowCount / 2;
    if (strcmp(preference, "Window") == 0 || strcmp(preference, "WINDOW") == 0 ||
        strcmp(preference, "Corner Seat") == 0)
        return column == 0 || column == BUSMATE_SEATS_PER_ROW - 1;
    return strcmp(preference, "Any") == 0 || strcmp(preference, "ANY") == 0;
}

int getBusSeatCapacity(int busId)
{
    Bus bus;

    if (busId <= 0 || !loadBusById(busId, &bus) || bus.capacity <= 0)
        return -1;
    return bus.capacity;
}

int getAllocatedSeatCount(int busId)
{
    Seat *seats = NULL;
    int count = 0;
    int allocated = 0;
    int i;

    if (busId <= 0 || !loadAllSeatsDynamic(&seats, &count))
        return -1;
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber > 0 &&
            strcmp(seats[i].status, "Booked") == 0)
            allocated++;
    }
    free(seats);
    return allocated;
}

int checkSeatAvailability(int busId, int seatNumber)
{
    Seat *seats = NULL;
    int count = 0;
    int capacity = getBusSeatCapacity(busId);
    int found = 0;
    int available = 1;
    int i;

    if (capacity < 0 || seatNumber < 1 || seatNumber > capacity ||
        !loadAllSeatsDynamic(&seats, &count))
        return -1;
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].seatNumber == seatNumber)
        {
            if (found)
            {
                free(seats);
                return -1;
            }
            found = 1;
            available = strcmp(seats[i].status, "Available") == 0;
        }
    }
    free(seats);
    return available;
}

int requestStudentSeat(int busId, int studentId, const char *preference, int *assignedSeat)
{
    static const char *validPreferences[] = {
        "Front Row", "Middle Row", "Corner Seat", "Window", "WINDOW", "Front", "FRONT", "Any", "ANY"};
    Seat *seats = NULL;
    Seat waitingEntry = {0};
    Student student;
    int count = 0;
    int capacity = getBusSeatCapacity(busId);
    int preferenceValid = 0;
    int bookedCount = 0;
    int selectedSeat = 0;
    int fallbackSeat = 0;
    int i;
    int seatNumber;

    if (assignedSeat != NULL)
        *assignedSeat = 0;
    if (assignedSeat == NULL || capacity <= 0 || studentId <= 0 || preference == NULL ||
        !loadStudentById(studentId, &student) || !loadAllSeatsDynamic(&seats, &count))
        return -1;
    for (i = 0; i < (int)(sizeof(validPreferences) / sizeof(validPreferences[0])); i++)
    {
        if (strcmp(preference, validPreferences[i]) == 0)
        {
            preferenceValid = 1;
            break;
        }
    }
    if (!preferenceValid || BUSMATE_SEATS_PER_ROW <= 0)
    {
        free(seats);
        return -1;
    }
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].studentId == studentId &&
            (strcmp(seats[i].status, "Booked") == 0 || strcmp(seats[i].status, "Waiting") == 0))
        {
            free(seats);
            return -1;
        }
        if (seats[i].busId == busId && seats[i].seatNumber > 0 &&
            strcmp(seats[i].status, "Booked") == 0)
            bookedCount++;
    }
    if (bookedCount < capacity)
    {
        for (seatNumber = 1; seatNumber <= capacity; seatNumber++)
        {
            int isAvailable = 1;
            for (i = 0; i < count; i++)
            {
                if (seats[i].busId == busId && seats[i].seatNumber == seatNumber)
                {
                    isAvailable = strcmp(seats[i].status, "Available") == 0;
                    break;
                }
            }
            if (isAvailable)
            {
                if (fallbackSeat == 0)
                    fallbackSeat = seatNumber;
                if (selectedSeat == 0 && preference_matches(seatNumber, capacity, preference))
                    selectedSeat = seatNumber;
            }
        }
    }
    free(seats);

    if (selectedSeat == 0)
        selectedSeat = fallbackSeat;
    if (selectedSeat > 0)
    {
        if (!updateSeatStatus(busId, selectedSeat, studentId, "Booked", preference))
            return -1;
        *assignedSeat = selectedSeat;
        return 1;
    }

    waitingEntry.busId = busId;
    waitingEntry.seatNumber = 0;
    waitingEntry.studentId = studentId;
    snprintf(waitingEntry.status, sizeof(waitingEntry.status), "Waiting");
    snprintf(waitingEntry.preference, sizeof(waitingEntry.preference), "%s", preference);
    return saveSeat(waitingEntry) ? 0 : -1;
}

int cancelStudentSeat(int studentId, int busId)
{
    Seat *seats = NULL;
    int count = 0;
    int i;

    if (studentId <= 0 || busId <= 0 || !loadAllSeatsDynamic(&seats, &count))
        return 0;
    for (i = 0; i < count; i++)
    {
        if (seats[i].busId == busId && seats[i].studentId == studentId &&
            seats[i].seatNumber > 0 && strcmp(seats[i].status, "Booked") == 0)
        {
            int seatNumber = seats[i].seatNumber;
            free(seats);
            return updateSeatStatus(busId, seatNumber, 0, "Available", "Any");
        }
    }
    free(seats);
    return 0;
}

void seatAllocationManagement(void)
{
    int busId, seatNumber, studentId;
    int choice;

    printf("\n====== SEAT ALLOCATION MANAGEMENT ======\n");
    printf("1. Assign seat\n");
    printf("2. Cancel seat\n");
    printf("3. Show available seats\n");
    printf("0. Back\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    getchar();

    switch (choice)
    {
    case 1:
        printf("Bus ID: ");
        scanf("%d", &busId);
        getchar();
        printf("Seat Number: ");
        scanf("%d", &seatNumber);
        getchar();
        printf("Student ID: ");
        scanf("%d", &studentId);
        getchar();
        if (updateSeatStatus(busId, seatNumber, studentId, "Booked", "Any"))
            printf("Seat allocated successfully.\n");
        else
            printf("Seat allocation failed. Check that the bus and seat exist and the seat is available.\n");
        break;
    case 2:
        printf("Bus ID: ");
        scanf("%d", &busId);
        getchar();
        printf("Seat Number: ");
        scanf("%d", &seatNumber);
        getchar();
        if (updateSeatStatus(busId, seatNumber, 0, "Available", "Any"))
            printf("Seat cancelled successfully. The next waiting student, if any, has been promoted.\n");
        else
            printf("Seat cancellation failed. Check that the seat exists and is booked.\n");
        break;
    case 3:
    {
        Seat *seats = NULL;
        int count = 0;
        int i;
        int found = 0;

        if (!loadAllSeatsDynamic(&seats, &count))
        {
            printf("Unable to load seat data.\n");
            break;
        }
        for (i = 0; i < count; i++)
        {
            if (strcmp(seats[i].status, "Available") == 0)
            {
                printf("Bus %d | Seat %d | Status %s\n", seats[i].busId, seats[i].seatNumber, seats[i].status);
                found = 1;
            }
        }
        if (!found)
            printf("No available seats found.\n");
        free(seats);
        break;
    }
    case 0:
        break;
    default:
        printf("Invalid choice.\n");
    }
}
