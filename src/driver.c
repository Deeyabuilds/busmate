#include <stdio.h>
#include <string.h>
#include "driver.h"
#include "file_handler.h"

Driver drivers[MAX_DRIVERS];
int driverCount = 0;

void loadDrivers(void)
{
    driverCount = 0;
    FILE *file = fopen("data/drivers.dat", "rb");
    if (file == NULL)
        return;

    while (driverCount < MAX_DRIVERS && fread(&drivers[driverCount], sizeof(Driver), 1, file) == 1)
    {
        driverCount++;
    }

    fclose(file);
}

int driverExists(int driverId)
{
    loadDrivers();
    for (int i = 0; i < driverCount; i++)
    {
        if (drivers[i].driverId == driverId)
            return 1;
    }
    return 0;
}

int addDriver(int driverId, const char *name, const char *phone)
{
    loadDrivers();

    if (driverCount >= MAX_DRIVERS)
    {
        printf("Driver limit reached!\n");
        return -1;
    }

    if (driverExists(driverId))
    {
        printf("Driver ID already exists!\n");
        return -2;
    }

    Driver newDriver;
    newDriver.driverId = driverId;
    snprintf(newDriver.name, sizeof(newDriver.name), "%s", name ? name : "");
    snprintf(newDriver.phone, sizeof(newDriver.phone), "%s", phone ? phone : "");

    saveDriver(newDriver);
    loadDrivers();

    printf("Driver added successfully!\n");
    return 0;
}

void displayDrivers(void)
{
    loadDrivers();

    if (driverCount == 0)
    {
        printf("\nNo drivers available.\n");
        return;
    }

    printf("\n------ DRIVER LIST ------\n");

    for (int i = 0; i < driverCount; i++)
    {
        printf("\nDriver ID: %d", drivers[i].driverId);
        printf("\nName     : %s", drivers[i].name);
        printf("\nPhone    : %s\n", drivers[i].phone);
    }
}

int removeDriver(int driverId)
{
    loadDrivers();

    FILE *file = fopen("data/drivers.dat", "rb");
    if (file == NULL)
    {
        printf("Unable to open driver records.\n");
        return -1;
    }

    FILE *temp = fopen("data/temp_driver.dat", "wb");
    if (temp == NULL)
    {
        fclose(file);
        printf("Unable to write temporary file.\n");
        return -1;
    }

    Driver driver;
    int found = 0;

    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        if (driver.driverId == driverId)
        {
            found = 1;
            continue;
        }

        fwrite(&driver, sizeof(Driver), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove("data/drivers.dat");
    rename("data/temp_driver.dat", "data/drivers.dat");

    loadDrivers();

    if (found)
    {
        printf("Driver removed successfully!\n");
        return 0;
    }
    else
    {
        printf("Driver ID not found!\n");
        return -1;
    }
}
/* ================= DRIVER PORTAL ================= */
/* Note: Driver aur Bus ke beech link ka field structures.h mein nahi hai,
   isliye assumption: Driver ID N ko Bus ID N assigned hai.
   Agar leader koi aur rule de, to sirf portalAssignedBusId() badalna hai. */

#define BUS_STATUS_FILE "data/bus_status.dat"

typedef struct
{
    int busId;
    char status[20];
} PortalBusStatus;

static int portalReadInt(const char *prompt)
{
    char buf[64];
    int value;

    while (1)
    {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            return -1;
        if (sscanf(buf, "%d", &value) == 1)
            return value;
        printf("Invalid input! Please enter a number.\n");
    }
}

static int portalAssignedBusId(int driverId)
{
    return driverId;
}

/* Driver ID puchta hai aur driver ka record deta hai */
static int portalLoginDriver(Driver *out)
{
    int id = portalReadInt("Enter your Driver ID: ");

    loadDrivers();
    for (int i = 0; i < driverCount; i++)
    {
        if (drivers[i].driverId == id)
        {
            *out = drivers[i];
            return 1;
        }
    }

    printf("Driver ID not found!\n");
    return 0;
}

static int portalFindBus(int busId, Bus *out)
{
    FILE *file = fopen(BUSES_FILE, "rb");
    if (file == NULL)
        return 0;

    Bus bus;
    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        if (bus.busId == busId)
        {
            *out = bus;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static int portalFindRoute(int routeId, Route *out)
{
    FILE *file = fopen(ROUTES_FILE, "rb");
    if (file == NULL)
        return 0;

    Route route;
    while (fread(&route, sizeof(Route), 1, file) == 1)
    {
        if (route.routeId == routeId)
        {
            *out = route;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static void portalGetStatus(int busId, char *out, size_t size)
{
    snprintf(out, size, "%s", "Not Updated");

    FILE *file = fopen(BUS_STATUS_FILE, "rb");
    if (file == NULL)
        return;

    PortalBusStatus rec;
    while (fread(&rec, sizeof(rec), 1, file) == 1)
    {
        if (rec.busId == busId)
        {
            snprintf(out, size, "%s", rec.status);
            break;
        }
    }

    fclose(file);
}

static void portalSetStatus(int busId, const char *status)
{
    FILE *file = fopen(BUS_STATUS_FILE, "rb+");
    if (file == NULL)
        file = fopen(BUS_STATUS_FILE, "wb+");
    if (file == NULL)
    {
        printf("Unable to save bus status.\n");
        return;
    }

    PortalBusStatus rec;
    while (fread(&rec, sizeof(rec), 1, file) == 1)
    {
        if (rec.busId == busId)
        {
            snprintf(rec.status, sizeof(rec.status), "%s", status);
            fseek(file, -(long)sizeof(rec), SEEK_CUR);
            fwrite(&rec, sizeof(rec), 1, file);
            fclose(file);
            return;
        }
    }

    memset(&rec, 0, sizeof(rec));
    rec.busId = busId;
    snprintf(rec.status, sizeof(rec.status), "%s", status);
    fseek(file, 0, SEEK_END);
    fwrite(&rec, sizeof(rec), 1, file);
    fclose(file);
}

void viewDriverProfile(void)
{
    Driver driver;

    if (!portalLoginDriver(&driver))
        return;

    printf("\n------ DRIVER PROFILE ------\n");
    printf("Driver ID : %d\n", driver.driverId);
    printf("Name      : %s\n", driver.name);
    printf("Phone     : %s\n", driver.phone);
}

void viewAssignedBus(void)
{
    Driver driver;
    Bus bus;
    char status[20];

    if (!portalLoginDriver(&driver))
        return;

    int busId = portalAssignedBusId(driver.driverId);

    if (!portalFindBus(busId, &bus))
    {
        printf("No bus assigned to this driver.\n");
        return;
    }

    portalGetStatus(bus.busId, status, sizeof(status));

    printf("\n------ ASSIGNED BUS ------\n");
    printf("Bus ID          : %d\n", bus.busId);
    printf("Bus Number      : %s\n", bus.busNumber);
    printf("Capacity        : %d\n", bus.capacity);
    printf("Available Seats : %d\n", bus.availableSeats);
    printf("Status          : %s\n", status);
}

void viewAssignedRoute(void)
{
    Driver driver;
    Schedule sched;
    Route route;
    int found = 0;

    if (!portalLoginDriver(&driver))
        return;

    int busId = portalAssignedBusId(driver.driverId);

    FILE *file = fopen(SCHEDULES_FILE, "rb");
    if (file == NULL)
    {
        printf("No route assigned to this driver.\n");
        return;
    }

    printf("\n------ ASSIGNED ROUTE ------\n");

    while (fread(&sched, sizeof(Schedule), 1, file) == 1)
    {
        if (sched.busId != busId)
            continue;

        if (portalFindRoute(sched.routeId, &route))
        {
            printf("\nRoute ID   : %d", route.routeId);
            printf("\nRoute Name : %s", route.routeName);
            printf("\nStops      : %s\n", route.stops);
            found = 1;
        }
    }

    fclose(file);

    if (!found)
        printf("No route assigned to this driver.\n");
}

void viewDriverSchedule(void)
{
    Driver driver;
    Schedule sched;
    Route route;
    int found = 0;

    if (!portalLoginDriver(&driver))
        return;

    int busId = portalAssignedBusId(driver.driverId);

    FILE *file = fopen(SCHEDULES_FILE, "rb");
    if (file == NULL)
    {
        printf("No schedule found for this driver.\n");
        return;
    }

    printf("\n------ DRIVER SCHEDULE ------\n");

    while (fread(&sched, sizeof(Schedule), 1, file) == 1)
    {
        if (sched.busId != busId)
            continue;

        printf("\nSchedule ID : %d", sched.scheduleId);
        if (portalFindRoute(sched.routeId, &route))
            printf("\nRoute       : %s", route.routeName);
        printf("\nStart Time  : %s", sched.startTime);
        printf("\nEnd Time    : %s", sched.endTime);
        printf("\nDay Type    : %s\n", sched.dayType);
        found = 1;
    }

    fclose(file);

    if (!found)
        printf("No schedule found for this driver.\n");
}

void updateBusStatus(void)
{
    Driver driver;
    Bus bus;
    char current[20];

    if (!portalLoginDriver(&driver))
        return;

    int busId = portalAssignedBusId(driver.driverId);

    if (!portalFindBus(busId, &bus))
    {
        printf("No bus assigned to this driver.\n");
        return;
    }

    portalGetStatus(bus.busId, current, sizeof(current));
    printf("\nBus %s current status: %s\n", bus.busNumber, current);
    printf("1. Running\n");
    printf("2. Delayed\n");
    printf("3. Not Running\n");

    int choice = portalReadInt("Enter new status: ");

    switch (choice)
    {
    case 1:
        portalSetStatus(bus.busId, "Running");
        break;
    case 2:
        portalSetStatus(bus.busId, "Delayed");
        break;
    case 3:
        portalSetStatus(bus.busId, "Not Running");
        break;
    default:
        printf("Invalid choice!\n");
        return;
    }

    printf("Bus status updated successfully!\n");
}

void driverMenu(void)
{
    int choice;

    do
    {
        printf("\n====== DRIVER PORTAL ======\n");
        printf("1. View Driver Profile\n");
        printf("2. View Assigned Bus\n");
        printf("3. View Assigned Route\n");
        printf("4. View Driver Schedule\n");
        printf("5. Update Bus Status\n");
        printf("0. Back\n");

        choice = portalReadInt("Enter choice: ");

        switch (choice)
        {
        case 1:
            viewDriverProfile();
            break;
        case 2:
            viewAssignedBus();
            break;
        case 3:
            viewAssignedRoute();
            break;
        case 4:
            viewDriverSchedule();
            break;
        case 5:
            updateBusStatus();
            break;
        case 0:
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 0);
}