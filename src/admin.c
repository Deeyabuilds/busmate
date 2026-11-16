#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "../include/admin.h"
#include "../include/file_handler.h"


/* =========================================================
   ADMIN LOGIN
   ========================================================= */

int admin_login(const char *username,
                const char *password)
{
    if (username == NULL ||
        password == NULL)
    {
        return 0;
    }

    /*
       Default administrator account.
       This can later be changed to file-based
       authentication if required.
    */

    if (strcmp(username, "admin") == 0 &&
        strcmp(password, "admin123") == 0)
    {
        return 1;
    }

    return 0;
}


/* =========================================================
   ADD ADMIN
   ========================================================= */

int add_admin(Admin admin)
{
    void *records;
    Admin *admins;

    size_t count;
    size_t i;

    records = NULL;
    admins = NULL;
    count = 0;

    if (admin.adminId <= 0)
    {
        return -1;
    }

    if (admin.username[0] == '\0')
    {
        return -1;
    }

    if (admin.password[0] == '\0')
    {
        return -1;
    }

    /*
       Admin records use the students file handler
       functions, but require their own file.
    */

    if (readRecords(
            "data/admins.dat",
            sizeof(Admin),
            &records,
            &count) == 0)
    {
        admins = (Admin *)records;

        for (i = 0; i < count; i++)
        {
            if (admins[i].adminId ==
                admin.adminId)
            {
                free(records);
                return -2;
            }
        }

        free(records);
    }

    if (appendRecord(
            "data/admins.dat",
            sizeof(Admin),
            &admin) != 0)
    {
        return -1;
    }

    return 1;
}


/* =========================================================
   SEARCH ADMIN
   ========================================================= */

int search_admin(int adminId,
                 Admin *admin)
{
    void *records;
    Admin *admins;

    size_t count;
    size_t i;

    if (admin == NULL ||
        adminId <= 0)
    {
        return -1;
    }

    records = NULL;
    admins = NULL;
    count = 0;

    if (readRecords(
            "data/admins.dat",
            sizeof(Admin),
            &records,
            &count) != 0)
    {
        return -1;
    }

    admins = (Admin *)records;

    for (i = 0; i < count; i++)
    {
        if (admins[i].adminId == adminId)
        {
            *admin = admins[i];

            free(records);

            return 1;
        }
    }

    free(records);

    return 0;
}


/* =========================================================
   UPDATE ADMIN
   ========================================================= */

int update_admin(Admin admin)
{
    int updated;

    updated = 0;

    if (admin.adminId <= 0)
    {
        return -1;
    }

    if (updateRecordById(
            "data/admins.dat",
            sizeof(Admin),
            offsetof(Admin, adminId),
            admin.adminId,
            &admin,
            &updated) != 0)
    {
        return -1;
    }

    if (updated)
    {
        return 1;
    }

    return 0;
}


/* =========================================================
   DELETE ADMIN
   ========================================================= */

int delete_admin(int adminId)
{
    int deleted;

    deleted = 0;

    if (adminId <= 0)
    {
        return -1;
    }

    if (deleteRecordById(
            "data/admins.dat",
            sizeof(Admin),
            offsetof(Admin, adminId),
            adminId,
            &deleted) != 0)
    {
        return -1;
    }

    if (deleted)
    {
        return 1;
    }

    return 0;
}