#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "admin.h"
#include "file_handler.h"

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
    Student student;

    return loadStudentById(id, &student);
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

    printf("Enter College ID: ");
    fgets(student.collegeId, sizeof(student.collegeId), stdin);
    student.collegeId[strcspn(student.collegeId, "\n")] = '\0';

    if (strlen(student.collegeId) == 0)
    {
        printf("Invalid college ID!\n");
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

    if (saveStudent(student))
        printf("Student added successfully!\n");
    else
        printf("Unable to save student record.\n");
}

// View Students
void viewStudents(void)
{
    Student *students = NULL;
    int count = 0;
    int i;

    if (!loadAllStudentsDynamic(&students, &count))
    {
        printf("\nUnable to load student records.\n");
        return;
    }

    printf("\n====== STUDENT RECORDS ======\n");
    for (i = 0; i < count; i++)
    {
        printf("\nStudent ID : %d", students[i].studentId);
        printf("\nName       : %s", students[i].name);
        printf("\nPassword   : %s\n", students[i].password);
    }
    if (count == 0)
        printf("No student records found.\n");
    free(students);
}

// Remove Student
void removeStudent(void)
{
    int studentId;

    printf("\n====== REMOVE STUDENT ======\n");

    studentId = getInteger("Enter Student ID: ");

    if (studentId <= 0)
    {
        printf("Invalid Student ID!\n");
        return;
    }

    if (deleteStudentById(studentId))
        printf("Student removed successfully!\n");
    else
        printf("Unable to remove student. Confirm the ID exists and check for file errors.\n");
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

    if (busIdExists(bus.busId))
    {
        printf("Bus ID already exists!\n");
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
    snprintf(bus.status, sizeof(bus.status), "Active");

    if (saveBus(bus))
        printf("Bus added successfully!\n");
    else
        printf("Unable to save bus. Check that the ID is unique and the data file is writable.\n");
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
    FILE *file;
    FILE *temp;
    Bus bus;
    int busId;
    int found = 0;

    printf("\n====== REMOVE BUS ======\n");

    busId = getInteger("Enter Bus ID: ");

    if (busId <= 0)
    {
        printf("Invalid Bus ID!\n");
        return;
    }

    file = fopen("data/buses.dat", "rb");
    temp = fopen("data/temp.dat", "wb");

    if (file == NULL || temp == NULL)
    {
        printf("Unable to open bus records.\n");

        if (file != NULL)
            fclose(file);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fread(&bus, sizeof(Bus), 1, file) == 1)
    {
        if (bus.busId == busId)
        {
            found = 1;
            continue;
        }

        fwrite(&bus, sizeof(Bus), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove("data/buses.dat");
    rename("data/temp.dat", "data/buses.dat");

    if (found)
        printf("Bus removed successfully!\n");
    else
        printf("Bus not found.\n");
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
    Route route = {0};

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

    if (saveRoute(route))
        printf("Route added successfully!\n");
    else
        printf("Unable to save route. Check that the ID is unique and the data file is writable.\n");
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
    FILE *file;
    FILE *temp;
    Route route;
    int routeId;
    int found = 0;

    printf("\n====== REMOVE ROUTE ======\n");

    routeId = getInteger("Enter Route ID: ");

    if (routeId <= 0)
    {
        printf("Invalid Route ID!\n");
        return;
    }

    file = fopen("data/routes.dat", "rb");
    temp = fopen("data/temp.dat", "wb");

    if (file == NULL || temp == NULL)
    {
        printf("Unable to open route records.\n");

        if (file != NULL)
            fclose(file);

        if (temp != NULL)
            fclose(temp);

        return;
    }

    while (fread(&route, sizeof(Route), 1, file) == 1)
    {
        if (route.routeId == routeId)
        {
            found = 1;
            continue;
        }

        fwrite(&route, sizeof(Route), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove("data/routes.dat");
    rename("data/temp.dat", "data/routes.dat");

    if (found)
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
    Driver driver = {0};

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

    printf("Enter Password: ");
    fgets(driver.password, sizeof(driver.password), stdin);
    driver.password[strcspn(driver.password, "\n")] = '\0';

    if (strlen(driver.password) == 0)
    {
        printf("Invalid password!\n");
        return;
    }

    if (saveDriver(driver))
        printf("Driver added successfully!\n");
    else
        printf("Unable to save driver. Check that the ID is unique and the data file is writable.\n");
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
    FILE *file;
    FILE *temp;
    Driver driver;
    int driverId;
    int found = 0;

    printf("\n====== REMOVE DRIVER ======\n");

    driverId = getInteger("Enter Driver ID: ");

    if (driverId <= 0)
    {
        printf("Invalid Driver ID!\n");
        return;
    }

    file = fopen("data/drivers.dat", "rb");
    temp = fopen("data/temp.dat", "wb");

    if (file == NULL || temp == NULL)
    {
        printf("Unable to open driver records.\n");

        if (file != NULL)
            fclose(file);

        if (temp != NULL)
            fclose(temp);

        return;
    }

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
    rename("data/temp.dat", "data/drivers.dat");

    if (found)
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
    Schedule schedule = {0};

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

    if (saveSchedule(schedule))
        printf("Schedule added successfully!\n");
    else
        printf("Unable to save schedule. Check that the ID is unique and the data file is writable.\n");
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
    FILE *file;
    FILE *temp;
    Schedule schedule;
    int id;
    int found = 0;

    printf("\n====== REMOVE SCHEDULE ======\n");

    id = getInteger("Enter Schedule ID to remove: ");

    if (id <= 0)
    {
        printf("Invalid Schedule ID!\n");
        return;
    }

    file = fopen("data/schedules.dat", "rb");

    if (file == NULL)
    {
        printf("No schedule records found.\n");
        return;
    }

    temp = fopen("data/temp.dat", "wb");

    if (temp == NULL)
    {
        fclose(file);
        printf("Unable to remove schedule.\n");
        return;
    }

    while (fread(&schedule, sizeof(Schedule), 1, file) == 1)
    {
        if (schedule.scheduleId == id)
        {
            found = 1;
            continue;
        }

        fwrite(&schedule, sizeof(Schedule), 1, temp);
    }

    fclose(file);
    fclose(temp);

    remove("data/schedules.dat");
    rename("data/temp.dat", "data/schedules.dat");

    if (found)
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