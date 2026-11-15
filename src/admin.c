#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "admin.h"
#include "file_handler.h"
#include "seat_allocation.h"

// Input Integer
int getInteger(const char *message)
{
    char input[50];
    int value;

    while (1)
    {
        printf("%s", message);

        if (fgets(input, sizeof(input), stdin) == NULL)
            return -1;

        if (sscanf(input, "%d", &value) == 1)
            return value;

        printf("Invalid input! Please enter a number.\n");
    }
}

// Student Management

// Check Student ID
int studentIdExists(int id)
{
    FILE *file;
    Student student;

    file = fopen("data/students.dat", "rb");

    if (file == NULL)
        return 0;

    while (fread(&student, sizeof(Student), 1, file) == 1)
    {
        if (student.studentId == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Add Student
void addStudent(void)
{
    Student student;

    printf("\n====== ADD STUDENT ======\n");

    student.studentId = getInteger("Enter Student ID: ");

    if (student.studentId <= 0)
    {
        printf("Invalid Student ID!\n");
        return;
    }

    if (studentIdExists(student.studentId))
    {
        printf("Student ID already exists!\n");
        return;
    }

    printf("Enter Name: ");
    fgets(student.name, sizeof(student.name), stdin);
    student.name[strcspn(student.name, "\n")] = '\0';

    if (strlen(student.name) == 0)
    {
        printf("Invalid name!\n");
        return;
    }

    printf("Enter Password: ");
    fgets(student.password, sizeof(student.password), stdin);
    student.password[strcspn(student.password, "\n")] = '\0';

    if (strlen(student.password) == 0)
    {
        printf("Invalid password!\n");
        return;
    }

    saveStudent(student);

    printf("Student added successfully!\n");
}

// View Students
void viewStudents(void)
{
    FILE *file;
    Student student;

    file = fopen("data/students.dat", "rb");

    if (file == NULL)
    {
        printf("\nNo student records found.\n");
        return;
    }

    printf("\n====== STUDENT RECORDS ======\n");

    while (fread(&student, sizeof(Student), 1, file) == 1)
    {
        printf("\nStudent ID : %d", student.studentId);
        printf("\nName       : %s", student.name);
        printf("\nPassword   : %s\n", student.password);
    }

    fclose(file);
}

// Remove Student
void removeStudent(void)
{
    int studentId;
    int deleted = 0;

    printf("\n====== REMOVE STUDENT ======\n");

    studentId = getInteger("Enter Student ID: ");

    if (studentId <= 0)
    {
        printf("Invalid Student ID!\n");
        return;
    }

    if (!deleteRecordById(STUDENTS_FILE, sizeof(Student), offsetof(Student, studentId),
                          studentId, &deleted))
    {
        printf("Unable to update student records.\n");
        return;
    }

    if (deleted)
        printf("Student removed successfully!\n");
    else
        printf("Student not found.\n");
}

// Student Management
void studentManagement(void)
{
    int choice;

    do
    {
        printf("\n====== STUDENT MANAGEMENT ======\n");
        printf("1. Add Student\n");
        printf("2. View Students\n");
        printf("3. Remove Student\n");
        printf("0. Back\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            addStudent();
            break;

        case 2:
            viewStudents();
            break;

        case 3:
            removeStudent();
            break;

        case 0:
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}

// Bus Management

// Check Bus ID
int busIdExists(int id)
{
    FILE *file;
    Bus bus;

    file = fopen("data/buses.dat", "rb");

    if (file == NULL)
        return 0;

    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        if (bus.busId == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Add Bus
void addBus(void)
{
    Bus bus = {0};

    printf("\n====== ADD BUS ======\n");

    bus.busId = getInteger("Enter Bus ID: ");

    if (bus.busId <= 0)
    {
        printf("Invalid Bus ID!\n");
        return;
    }

    printf("Enter Bus Number: ");
    fgets(bus.busNumber, sizeof(bus.busNumber), stdin);
    bus.busNumber[strcspn(bus.busNumber, "\n")] = '\0';

    if (strlen(bus.busNumber) == 0)
    {
        printf("Invalid bus number!\n");
        return;
    }

    bus.capacity = getInteger("Enter Capacity: ");

    if (bus.capacity <= 0)
    {
        printf("Invalid capacity!\n");
        return;
    }

    bus.availableSeats = bus.capacity;
    if (!createBusWithSeats(&bus))
    {
        printf("Bus could not be added. Check for a duplicate ID or inconsistent seat data.\n");
        return;
    }
    printf("Bus added and seat records initialized successfully!\n");
}

// View Buses
void viewBuses(void)
{
    FILE *file;
    Bus bus;

    file = fopen("data/buses.dat", "rb");

    if (file == NULL)
    {
        printf("\nNo bus records found.\n");
        return;
    }

    printf("\n====== BUS RECORDS ======\n");

    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        printf("\nBus ID          : %d", bus.busId);
        printf("\nBus Number      : %s", bus.busNumber);
        printf("\nCapacity        : %d", bus.capacity);
        printf("\nAvailable Seats : %d\n", bus.availableSeats);
    }

    fclose(file);
}

// Remove Bus
void removeBus(void)
{
    int busId;

    printf("\n====== REMOVE BUS ======\n");

    busId = getInteger("Enter Bus ID: ");

    if (busId <= 0)
    {
        printf("Invalid Bus ID!\n");
        return;
    }

    if (!busIdExists(busId))
    {
        printf("Bus not found.\n");
        return;
    }

    if (!removeBusWithSeats(busId))
    {
        printf("Bus was not removed. Check seat consistency, bookings, and waiting-list entries.\n");
        return;
    }
    printf("Bus and its available seat records removed successfully.\n");
}

// Bus Management
void busManagement(void)
{
    int choice;

    do
    {
        printf("\n====== BUS MANAGEMENT ======\n");
        printf("1. Add Bus\n");
        printf("2. View Buses\n");
        printf("3. Remove Bus\n");
        printf("0. Back\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            addBus();
            break;

        case 2:
            viewBuses();
            break;

        case 3:
            removeBus();
            break;

        case 0:
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}

// Route Management

// Check Route ID
int routeIdExists(int id)
{
    FILE *file;
    Route route;

    file = fopen("data/routes.dat", "rb");

    if (file == NULL)
        return 0;

    while (fread(&route, sizeof(Route), 1, file) == 1)
    {
        if (route.routeId == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Add Route
void addRoute(void)
{
    Route route;

    printf("\n====== ADD ROUTE ======\n");

    route.routeId = getInteger("Enter Route ID: ");

    if (route.routeId <= 0)
    {
        printf("Invalid Route ID!\n");
        return;
    }

    if (routeIdExists(route.routeId))
    {
        printf("Route ID already exists!\n");
        return;
    }

    printf("Enter Route Name: ");
    fgets(route.routeName, sizeof(route.routeName), stdin);
    route.routeName[strcspn(route.routeName, "\n")] = '\0';

    if (strlen(route.routeName) == 0)
    {
        printf("Invalid route name!\n");
        return;
    }

    printf("Enter Stops: ");
    fgets(route.stops, sizeof(route.stops), stdin);
    route.stops[strcspn(route.stops, "\n")] = '\0';

    if (strlen(route.stops) == 0)
    {
        printf("Invalid stops!\n");
        return;
    }

    saveRoute(route);

    printf("Route added successfully!\n");
}

// View Routes
void viewRoutes(void)
{
    FILE *file;
    Route route;

    file = fopen("data/routes.dat", "rb");

    if (file == NULL)
    {
        printf("\nNo route records found.\n");
        return;
    }

    printf("\n====== ROUTE RECORDS ======\n");

    while (fread(&route, sizeof(Route), 1, file) == 1)
    {
        printf("\nRoute ID   : %d", route.routeId);
        printf("\nRoute Name : %s", route.routeName);
        printf("\nStops      : %s\n", route.stops);
    }

    fclose(file);
}

// Remove Route
void removeRoute(void)
{
    int routeId;
    int deleted = 0;

    printf("\n====== REMOVE ROUTE ======\n");

    routeId = getInteger("Enter Route ID: ");

    if (routeId <= 0)
    {
        printf("Invalid Route ID!\n");
        return;
    }

    if (!deleteRecordById(ROUTES_FILE, sizeof(Route), offsetof(Route, routeId), routeId,
                          &deleted))
    {
        printf("Unable to update route records.\n");
        return;
    }

    if (deleted)
        printf("Route removed successfully!\n");
    else
        printf("Route not found.\n");
}

// Route Management
void routeManagement(void)
{
    int choice;

    do
    {
        printf("\n====== ROUTE MANAGEMENT ======\n");
        printf("1. Add Route\n");
        printf("2. View Routes\n");
        printf("3. Remove Route\n");
        printf("0. Back\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            addRoute();
            break;

        case 2:
            viewRoutes();
            break;

        case 3:
            removeRoute();
            break;

        case 0:
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}

// Driver Management

// Check Driver ID
int driverIdExists(int id)
{
    FILE *file;
    Driver driver;

    file = fopen("data/drivers.dat", "rb");

    if (file == NULL)
        return 0;

    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        if (driver.driverId == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Add Driver
void addDriver(void)
{
    Driver driver;

    printf("\n====== ADD DRIVER ======\n");

    driver.driverId = getInteger("Enter Driver ID: ");

    if (driver.driverId <= 0)
    {
        printf("Invalid Driver ID!\n");
        return;
    }

    if (driverIdExists(driver.driverId))
    {
        printf("Driver ID already exists!\n");
        return;
    }

    printf("Enter Name: ");
    fgets(driver.name, sizeof(driver.name), stdin);
    driver.name[strcspn(driver.name, "\n")] = '\0';

    if (strlen(driver.name) == 0)
    {
        printf("Invalid name!\n");
        return;
    }

    printf("Enter Phone: ");
    fgets(driver.phone, sizeof(driver.phone), stdin);
    driver.phone[strcspn(driver.phone, "\n")] = '\0';

    if (strlen(driver.phone) == 0)
    {
        printf("Invalid phone number!\n");
        return;
    }

    saveDriver(driver);

    printf("Driver added successfully!\n");
}

// View Drivers
void viewDrivers(void)
{
    FILE *file;
    Driver driver;

    file = fopen("data/drivers.dat", "rb");

    if (file == NULL)
    {
        printf("\nNo driver records found.\n");
        return;
    }

    printf("\n====== DRIVER RECORDS ======\n");

    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        printf("\nDriver ID : %d", driver.driverId);
        printf("\nName      : %s", driver.name);
        printf("\nPhone     : %s\n", driver.phone);
    }

    fclose(file);
}

// Remove Driver
void removeDriver(void)
{
    int driverId;
    int deleted = 0;

    printf("\n====== REMOVE DRIVER ======\n");

    driverId = getInteger("Enter Driver ID: ");

    if (driverId <= 0)
    {
        printf("Invalid Driver ID!\n");
        return;
    }

    if (!deleteRecordById(DRIVERS_FILE, sizeof(Driver), offsetof(Driver, driverId), driverId,
                          &deleted))
    {
        printf("Unable to update driver records.\n");
        return;
    }

    if (deleted)
        printf("Driver removed successfully!\n");
    else
        printf("Driver not found.\n");
}

// Driver Management
void driverManagement(void)
{
    int choice;

    do
    {
        printf("\n====== DRIVER MANAGEMENT ======\n");
        printf("1. Add Driver\n");
        printf("2. View Drivers\n");
        printf("3. Remove Driver\n");
        printf("0. Back\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            addDriver();
            break;

        case 2:
            viewDrivers();
            break;

        case 3:
            removeDriver();
            break;

        case 0:
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}

// Schedule Management

// Check Schedule ID
int scheduleIdExists(int id)
{
    FILE *file;
    Schedule schedule;

    file = fopen("data/schedules.dat", "rb");

    if (file == NULL)
        return 0;

    while (fread(&schedule, sizeof(Schedule), 1, file) == 1)
    {
        if (schedule.scheduleId == id)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Add Schedule
void addSchedule(void)
{
    Schedule schedule;

    printf("\n====== ADD SCHEDULE ======\n");

    schedule.scheduleId = getInteger("Enter Schedule ID: ");

    if (schedule.scheduleId <= 0)
    {
        printf("Invalid Schedule ID!\n");
        return;
    }

    if (scheduleIdExists(schedule.scheduleId))
    {
        printf("Schedule ID already exists!\n");
        return;
    }

    schedule.busId = getInteger("Enter Bus ID: ");

    if (schedule.busId <= 0)
    {
        printf("Invalid Bus ID!\n");
        return;
    }

    schedule.routeId = getInteger("Enter Route ID: ");

    if (schedule.routeId <= 0)
    {
        printf("Invalid Route ID!\n");
        return;
    }

    printf("Enter Start Time: ");
    fgets(schedule.startTime, sizeof(schedule.startTime), stdin);
    schedule.startTime[strcspn(schedule.startTime, "\n")] = '\0';

    if (strlen(schedule.startTime) == 0)
    {
        printf("Invalid start time!\n");
        return;
    }

    printf("Enter End Time: ");
    fgets(schedule.endTime, sizeof(schedule.endTime), stdin);
    schedule.endTime[strcspn(schedule.endTime, "\n")] = '\0';

    if (strlen(schedule.endTime) == 0)
    {
        printf("Invalid end time!\n");
        return;
    }

    printf("Enter Day Type (Full Day / Half Day): ");
    fgets(schedule.dayType, sizeof(schedule.dayType), stdin);
    schedule.dayType[strcspn(schedule.dayType, "\n")] = '\0';

    if (strlen(schedule.dayType) == 0)
    {
        printf("Invalid day type!\n");
        return;
    }

    saveSchedule(schedule);

    printf("Schedule added successfully!\n");
}

// View Schedules
void viewSchedules(void)
{
    FILE *file;
    Schedule schedule;

    file = fopen("data/schedules.dat", "rb");

    if (file == NULL)
    {
        printf("\nNo schedule records found.\n");
        return;
    }

    printf("\n====== SCHEDULE RECORDS ======\n");

    while (fread(&schedule, sizeof(Schedule), 1, file) == 1)
    {
        printf("\nSchedule ID : %d", schedule.scheduleId);
        printf("\nBus ID      : %d", schedule.busId);
        printf("\nRoute ID    : %d", schedule.routeId);
        printf("\nStart Time  : %s", schedule.startTime);
        printf("\nEnd Time    : %s", schedule.endTime);
        printf("\nDay Type    : %s\n", schedule.dayType);
    }

    fclose(file);
}

// Remove Schedule
void removeSchedule(void)
{
    int id;
    int deleted = 0;

    printf("\n====== REMOVE SCHEDULE ======\n");

    id = getInteger("Enter Schedule ID to remove: ");

    if (id <= 0)
    {
        printf("Invalid Schedule ID!\n");
        return;
    }

    if (!deleteRecordById(SCHEDULES_FILE, sizeof(Schedule), offsetof(Schedule, scheduleId),
                          id, &deleted))
    {
        printf("Unable to update schedule records.\n");
        return;
    }

    if (deleted)
        printf("Schedule removed successfully!\n");
    else
        printf("Schedule ID not found!\n");
}

// Schedule Management
void scheduleManagement(void)
{
    int choice;

    do
    {
        printf("\n====== SCHEDULE MANAGEMENT ======\n");
        printf("1. Add Schedule\n");
        printf("2. View Schedules\n");
        printf("3. Remove Schedule\n");
        printf("0. Back\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            addSchedule();
            break;

        case 2:
            viewSchedules();
            break;

        case 3:
            removeSchedule();
            break;

        case 0:
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}

// Admin Menu
void adminMenu(void)
{
    int choice;

    do
    {
        printf("\n========== ADMIN MENU ==========\n");
        printf("1. Student Management\n");
        printf("2. Bus Management\n");
        printf("3. Route Management\n");
        printf("4. Driver Management\n");
        printf("5. Schedule Management\n");
        printf("0. Logout\n");

        choice = getInteger("Enter your choice: ");

        switch (choice)
        {
        case 1:
            studentManagement();
            break;

        case 2:
            busManagement();
            break;

        case 3:
            routeManagement();
            break;

        case 4:
            driverManagement();
            break;

        case 5:
            scheduleManagement();
            break;

        case 0:
            printf("Logging out...\n");
            break;

        default:
            printf("Invalid choice!\n");
        }

    } while (choice != 0);
}