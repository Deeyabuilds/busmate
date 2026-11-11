#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/file_handler.h"
#include "../include/admin.h"

int adminLogin(const char *username, const char *password)
{
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0)
    {
        return 1;
    }
    return 0;
}

int studentLogin(int id, const char *password)
{
    Student student;
    FILE *file = fopen("data/students.dat", "rb");

    if (file == NULL)
    {
        return 0;
    }

    while (fread(&student, sizeof(Student), 1, file) == 1)
    {
        if (student.studentId == id && strcmp(student.password, password) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static int studentLoginByCollegeId(const char *collegeId, const char *password)
{
    Student student;
    FILE *file = fopen("data/students.dat", "rb");

    if (file == NULL)
    {
        return 0;
    }

    while (fread(&student, sizeof(Student), 1, file) == 1)
    {
        if (strcmp(student.collegeId, collegeId) == 0 && strcmp(student.password, password) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int driverLogin(int id, const char *password)
{
    Driver driver;
    FILE *file = fopen("data/drivers.dat", "rb");

    if (file == NULL)
    {
        return 0;
    }

    while (fread(&driver, sizeof(Driver), 1, file) == 1)
    {
        if (driver.driverId == id && strcmp(driver.password, password) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

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

int run_busmate_system(void)
{
    int choice = 0;
    int id;
    char username[30];
    char password[30];

    do
    {
        printf("\n========== BUSMATE MAIN MENU ==========");
        printf("\n1. Admin Login\n");
        printf("2. Student Login\n");
        printf("3. Driver Login\n");
        printf("4. View Bus Routes\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        choice = read_menu_choice();

        switch (choice)
        {
        case 1:
            printf("Admin username: ");
            fgets(username, sizeof(username), stdin);
            username[strcspn(username, "\n")] = '\0';
            printf("Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = '\0';
            if (adminLogin(username, password))
            {
                adminMenu();
            }
            else
            {
                printf("Invalid admin credentials.\n");
            }
            break;
        case 2:
            char collegeId[20];
            printf("College ID: ");
            fgets(collegeId, sizeof(collegeId), stdin);
            collegeId[strcspn(collegeId, "\n")] = '\0';
            printf("Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = '\0';
            if (studentLoginByCollegeId(collegeId, password))
            {
                Student student;
                FILE *file = fopen("data/students.dat", "rb");
                while (file != NULL && fread(&student, sizeof(Student), 1, file) == 1)
                {
                    if (strcmp(student.collegeId, collegeId) == 0)
                    {
                        fclose(file);
                        studentDashboard(student.studentId);
                        goto student_login_done;
                    }
                }
                if (file != NULL)
                    fclose(file);
                printf("Student record not found.\n");
            }
            else
            {
                printf("Invalid student credentials.\n");
            }
        student_login_done:
            break;
        case 3:
            printf("Driver ID: ");
            scanf("%d", &id);
            getchar();
            printf("Password: ");
            fgets(password, sizeof(password), stdin);
            password[strcspn(password, "\n")] = '\0';
            if (driverLogin(id, password))
            {
                driverDashboard(id);
            }
            else
            {
                printf("Invalid driver credentials.\n");
            }
            break;
        case 4:
        {
            FILE *file = fopen("data/routes.dat", "rb");
            Route route;
            printf("\n====== Available Routes ======\n");
            if (file == NULL)
            {
                printf("No routes available.\n");
                break;
            }
            while (fread(&route, sizeof(Route), 1, file) == 1)
            {
                printf("Route %d: %s | %s | %s\n", route.routeId, route.routeName, route.timing, route.stops);
            }
            fclose(file);
        }
        break;
        case 0:
            printf("Exiting BusMate...\n");
            break;
        default:
            printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
