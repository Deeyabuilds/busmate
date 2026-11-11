#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_handler.h"
#include "../include/seat_allocation.h"
#include "../include/admin.h"

static int read_menu_choice(void)
{
    char input[32];
    int value;

    while (1)
    {
        if (fgets(input, sizeof(input), stdin) == NULL)
            return 0;

        if (sscanf(input, "%d", &value) == 1)
            return value;

        printf("Invalid choice. Please enter a number.\n");
        printf("Enter your choice: ");
    }
}

static void print_bus_list(void)
{
    Bus *buses = NULL;
    Route route;
    int found = 0;
    int count = 0;
    int i;

    if (!loadAllBusesDynamic(&buses, &count))
    {
        printf("\nUnable to load bus records.\n");
        return;
    }

    printf("\n====== AVAILABLE BUSES ======\n");
    for (i = 0; i < count; i++)
    {
        if (loadRouteById(buses[i].routeId, &route) == 1)
        {
            printf("Bus ID: %d | Number: %s | Route: %s | Status: %s | Seats Left: %d\n",
                   buses[i].busId, buses[i].busNumber, route.routeName, buses[i].status, buses[i].availableSeats);
            found = 1;
        }
    }

    free(buses);
    if (!found)
        printf("No buses found.\n");
}

static void print_student_routes(void)
{
    Route *routes = NULL;
    int found = 0;
    int count = 0;
    int i;

    if (!loadAllRoutesDynamic(&routes, &count))
    {
        printf("\nUnable to load route records.\n");
        return;
    }

    printf("\n====== ROUTES & TIMINGS ======\n");
    for (i = 0; i < count; i++)
    {
        printf("Route ID: %d | Name: %s | Type: %s | Timing: %s | Stops: %s\n",
               routes[i].routeId, routes[i].routeName, routes[i].type, routes[i].timing, routes[i].stops);
        found = 1;
    }
    free(routes);

    if (!found)
        printf("No routes found.\n");
}

static void print_student_seats(int studentId)
{
    Seat *seats;
    int count = 0;
    int i;
    int found = 0;

    if (!loadAllSeatsDynamic(&seats, &count))
    {
        printf("\nUnable to load seat records.\n");
        return;
    }
    if (count == 0)
    {
        printf("\nNo seat records found.\n");
        free(seats);
        return;
    }

    printf("\n====== MY SEAT DETAILS ======\n");
    for (i = 0; i < count; i++)
    {
        if (seats[i].studentId == studentId && strcmp(seats[i].status, "Booked") == 0)
        {
            printf("Bus ID: %d | Seat: %d | Preference: %s | Status: %s\n",
                   seats[i].busId, seats[i].seatNumber, seats[i].preference, seats[i].status);
            found = 1;
        }
    }

    if (!found)
        printf("No seat assigned yet.\n");
    free(seats);
}

static void cancel_student_seat(int studentId)
{
    Seat *seats;
    int count = 0;
    int i;
    int found = 0;

    if (!loadAllSeatsDynamic(&seats, &count))
    {
        printf("\nUnable to load seat records.\n");
        return;
    }
    if (count == 0)
    {
        printf("\nNo seat records found.\n");
        free(seats);
        return;
    }

    for (i = 0; i < count; i++)
    {
        if (seats[i].studentId == studentId && strcmp(seats[i].status, "Booked") == 0)
        {
            int busId = seats[i].busId;
            found = updateSeatStatus(busId, seats[i].seatNumber, 0, "Available", "Any");
            break;
        }
    }
    free(seats);

    if (found)
        printf("Seat cancelled successfully. The next waiting student, if any, has been promoted.\n");
    else
        printf("No booked seat was cancelled. The seat may have changed; please try again.\n");
}

void studentDashboard(int studentId)
{
    int choice = 0;

    do
    {
        printf("\n========== STUDENT DASHBOARD ==========\n");
        printf("1. View available buses\n");
        printf("2. View routes and timings\n");
        printf("3. View my seat\n");
        printf("4. Cancel seat\n");
        printf("5. Request a seat / join waiting list\n");
        printf("6. View my waiting list requests\n");
        printf("7. Cancel a waiting list request\n");
        printf("0. Logout\n");
        printf("Enter your choice: ");
        choice = read_menu_choice();

        switch (choice)
        {
        case 1:
            print_bus_list();
            break;
        case 2:
            print_student_routes();
            break;
        case 3:
            print_student_seats(studentId);
            break;
        case 4:
            cancel_student_seat(studentId);
            break;
        case 5:
        {
            int assignedSeat = 0;
            int busId;
            int preferenceChoice;
            const char *preference;
            int count = 0;
            int i;
            int waitingCount = 0;
            Seat *seats;
            int requestResult;

            printf("Enter Bus ID: ");
            busId = read_menu_choice();
            if (busId <= 0)
            {
                printf("Bus not found.\n");
                break;
            }
            printf("Seat preference:\n");
            printf("1. Front Row\n2. Middle Row\n3. Corner Seat\n4. No preference\n");
            preferenceChoice = read_menu_choice();
            if (preferenceChoice == 1)
                preference = "Front Row";
            else if (preferenceChoice == 2)
                preference = "Middle Row";
            else if (preferenceChoice == 3)
                preference = "Corner Seat";
            else if (preferenceChoice == 4)
                preference = "Any";
            else
            {
                printf("Invalid preference.\n");
                break;
            }

            requestResult = requestStudentSeat(busId, studentId, preference, &assignedSeat);
            if (requestResult < 0)
            {
                printf("Unable to request a seat. Check the bus and student records, or your existing request.\n");
                break;
            }
            if (requestResult > 0)
            {
                printf("Seat %d on bus %d was assigned (%s preference).\n", assignedSeat, busId, preference);
                break;
            }

            if (!loadAllSeatsDynamic(&seats, &count))
            {
                printf("Your waiting request was saved, but its queue position could not be loaded.\n");
                break;
            }
            for (i = 0; i < count; i++)
            {
                if (seats[i].busId == busId && seats[i].seatNumber == 0 &&
                    strcmp(seats[i].status, "Waiting") == 0)
                    waitingCount++;
            }
            free(seats);
            printf("Bus is full. You joined the waiting list at position %d.\n", waitingCount);
            break;
        }
        case 6:
        {
            Seat *seats;
            int count = 0;
            int i;
            int found = 0;

            if (!loadAllSeatsDynamic(&seats, &count))
            {
                printf("Unable to load seat records.\n");
                break;
            }
            if (count > 0)
            {
                printf("\n====== MY WAITING LIST REQUESTS ======\n");
                for (i = 0; i < count; i++)
                {
                    if (seats[i].studentId == studentId && seats[i].seatNumber == 0 &&
                        strcmp(seats[i].status, "Waiting") == 0)
                    {
                        int j;
                        int position = 1;
                        for (j = 0; j < i; j++)
                        {
                            if (seats[j].busId == seats[i].busId && seats[j].seatNumber == 0 &&
                                strcmp(seats[j].status, "Waiting") == 0)
                                position++;
                        }
                        printf("Bus ID: %d | Queue position: %d\n", seats[i].busId, position);
                        found = 1;
                    }
                }
            }
            free(seats);
            if (!found)
                printf("You have no active waiting list requests.\n");
            break;
        }
        case 7:
        {
            Seat *seats;
            int count = 0;
            int busId;
            int i;
            int found = 0;

            if (!loadAllSeatsDynamic(&seats, &count))
            {
                printf("Unable to load seat records.\n");
                break;
            }
            if (count == 0)
            {
                free(seats);
                printf("You have no active waiting list requests.\n");
                break;
            }
            for (i = 0; i < count; i++)
            {
                if (seats[i].studentId == studentId && seats[i].seatNumber == 0 &&
                    strcmp(seats[i].status, "Waiting") == 0)
                {
                    printf("Waiting request for Bus ID: %d\n", seats[i].busId);
                    found = 1;
                }
            }
            if (!found)
            {
                free(seats);
                printf("You have no active waiting list requests.\n");
                break;
            }

            printf("Enter the Bus ID of the request to cancel: ");
            busId = read_menu_choice();
            found = 0;
            for (i = 0; i < count; i++)
            {
                if (seats[i].studentId == studentId && seats[i].busId == busId &&
                    seats[i].seatNumber == 0 && strcmp(seats[i].status, "Waiting") == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                free(seats);
                printf("No active waiting list request was found for that bus.\n");
                break;
            }
            if (!updateSeatStatus(busId, 0, studentId, "Cancelled", "Any"))
            {
                free(seats);
                printf("Unable to cancel the waiting list request.\n");
                break;
            }
            free(seats);
            printf("Waiting list request cancelled.\n");
            break;
        }
        case 0:
            printf("Logging out student dashboard...\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 0);
}
