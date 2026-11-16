#ifndef ADMIN_H
#define ADMIN_H

#include "structures.h"

/* =========================================================
   ADMIN LOGIN
   ========================================================= */

int admin_login(const char *username,
                const char *password);


/* =========================================================
   ADMIN DATA OPERATIONS
   ========================================================= */

int add_admin(Admin admin);

int search_admin(int adminId,
                 Admin *admin);

int update_admin(Admin admin);

int delete_admin(int adminId);

#endif