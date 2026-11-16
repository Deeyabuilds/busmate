#ifndef DRIVER_H
#define DRIVER_H

#include "structures.h"

/* Driver operations */

int add_driver(Driver driver);

int search_driver(int driverId,
                  Driver *driver);

int update_driver(Driver driver);

int delete_driver(int driverId);

#endif