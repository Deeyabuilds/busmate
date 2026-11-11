#include <stdio.h>
#include <string.h>
#include "../include/file_handler.h"

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

static void print_daily_bus_report(void)
{
    FILE *file = fopen("data/buses.dat", "rb");
    Bus bus;
    int count = 0;

    if (file == NULL)
    {
        printf("\nNo daily bus report available.\n");
        return;
    }

    printf("\n====== DAILY BUS REPORT ======\n");
    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        printf("Bus %s | Route ID: %d | Status: %s | Available Seats: %d\n",
               bus.busNumber, bus.routeId, bus.status, bus.availableSeats);
        count++;
    }
    fclose(file);

    if (count == 0)
    {
        printf("No buses available.\n");
    }
}

static void print_occupancy_report(void)
{
    FILE *file = fopen("data/buses.dat", "rb");
    Bus bus;

    if (file == NULL)
    {
        printf("\nNo occupancy report available.\n");
        return;
    }

    printf("\n====== OCCUPANCY REPORT ======\n");
    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        printf("Bus %s | Capacity: %d | Available: %d\n",
               bus.busNumber, bus.capacity, bus.availableSeats);
    }
    fclose(file);
}

static void print_route_report(void)
{
    FILE *file = fopen("data/routes.dat", "rb");
    Route route;

    if (file == NULL)
    {
        printf("\nNo route-wise inventory.\n");
        return;
    }

    printf("\n====== ROUTE-WISE REPORT ======\n");
    while (fread(&route, sizeof(Route), 1, file) == 1)
    {
        printf("Route %d | %s | Type: %s | Timing: %s\n",
               route.routeId, route.routeName, route.type, route.timing);
    }
    fclose(file);
}

static void print_driver_report(void)
{
    FILE *file = fopen("data/drivers.dat", "rb");
    Driver driver;

    if (file == NULL)
    {
        printf("\nNo driver report available.\n");
        return;
    }

    printf("\n====== DRIVER REPORT ======\n");
    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        printf("Driver ID: %d | Name: %s | Phone: %s | Bus: %d\n",
               driver.driverId, driver.name, driver.phone, driver.assignedBusId);
    }
    fclose(file);
}

static void print_student_seat_report(void)
{
    FILE *file = fopen("data/seats.dat", "rb");
    Seat seat;

    if (file == NULL)
    {
        printf("\nNo student seat report available.\n");
        return;
    }

    printf("\n====== STUDENT SEAT REPORT ======\n");
    while (fread(&seat, sizeof(Seat), 1, file) == 1)
    {
        printf("Student ID: %d | Bus: %d | Seat: %d | Preference: %s | Status: %s\n",
               seat.studentId, seat.busId, seat.seatNumber, seat.preference, seat.status);
    }
    fclose(file);
}

void reportsMenu(void)
{
    int choice = 0;

    do
    {
        printf("\n========== REPORTS MENU ==========");
        printf("\n1. Daily bus report\n");
        printf("2. Occupancy report\n");
        printf("3. Route-wise report\n");
        printf("4. Driver report\n");
        printf("5. Student seat report\n");
        printf("0. Back\n");
        printf("Enter your choice: ");
        choice = read_menu_choice();

        switch (choice)
        {
        case 1:
            print_daily_bus_report();
            break;
        case 2:
            print_occupancy_report();
            break;
        case 3:
            print_route_report();
            break;
        case 4:
            print_driver_report();
            break;
        case 5:
            print_student_seat_report();
            break;
        case 0:
            printf("Returning to main menu...\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 0);
}
