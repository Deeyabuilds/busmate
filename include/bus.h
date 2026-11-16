#ifndef BUS_H
#define BUS_H

#include "structures.h"

/* Bus operations */

int add_bus(Bus bus);

int search_bus(int busId,
               Bus *bus);

int update_bus(Bus bus);

int delete_bus(int busId);

#endif