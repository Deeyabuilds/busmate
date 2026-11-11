#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_handler.h"
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

static void print_driver_bus_status(int driverId)
{
    FILE *file = fopen("data/drivers.dat", "rb");
    FILE *busFile = fopen("data/buses.dat", "rb");
    Driver driver;
    Bus bus;
    int found = 0;

    if (file == NULL)
    {
        printf("\nDriver record not found.\n");
        return;
    }

    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        if (driver.driverId == driverId)
        {
            if (busFile != NULL)
            {
                rewind(busFile);
                while (fread(&bus, sizeof(Bus), 1, busFile) == 1)
                {
                    if (bus.driverId == driverId || bus.busId == driver.assignedBusId)
                    {
                        printf("\nDriver ID: %d | Assigned Bus: %s | Status: %s | Available Seats: %d\n",
                               driver.driverId, bus.busNumber, bus.status, bus.availableSeats);
                        found = 1;
                    }
                }
            }
            break;
        }
    }

    fclose(file);
    if (busFile != NULL)
        fclose(busFile);

    if (!found)
    {
        printf("\nNo assigned bus found for this driver.\n");
    }
}

static void update_driver_bus_status(int driverId)
{
    FILE *file = fopen("data/buses.dat", "rb+");
    Bus bus;
    int choice;
    int found = 0;

    if (file == NULL)
    {
        printf("\nNo buses found.\n");
        return;
    }

    printf("\nUpdate bus status for driver %d\n", driverId);
    printf("1. Not Started\n2. Running\n3. Completed\nEnter choice: ");
    scanf("%d", &choice);
    getchar();

    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        if (bus.driverId == driverId || bus.busId == driverId)
        {
            found = 1;
            if (choice == 1)
                strcpy(bus.status, "Not Started");
            else if (choice == 2)
                strcpy(bus.status, "Running");
            else if (choice == 3)
                strcpy(bus.status, "Completed");
            else
            {
                printf("Invalid choice.\n");
                fclose(file);
                return;
            }

            fseek(file, -((long)sizeof(Bus)), SEEK_CUR);
            fwrite(&bus, sizeof(Bus), 1, file);
            break;
        }
    }

    fclose(file);

    if (found)
    {
        printf("Bus status updated successfully.\n");
    }
    else
    {
        printf("No matching bus for this driver.\n");
    }
}

void driverDashboard(int driverId)
{
    int choice = 0;

    do
    {
        printf("\n========== DRIVER DASHBOARD ==========");
        printf("\n1. View assigned bus status\n");
        printf("2. Update bus status\n");
        printf("0. Logout\n");
        printf("Enter your choice: ");
        choice = read_menu_choice();

        switch (choice)
        {
        case 1:
            print_driver_bus_status(driverId);
            break;
        case 2:
            update_driver_bus_status(driverId);
            break;
        case 0:
            printf("Logging out driver dashboard...\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 0);
}
