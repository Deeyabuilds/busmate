#ifndef SEAT_MANAGER_H
#define SEAT_MANAGER_H

#include <gtk/gtk.h>

#define TOTAL_SEATS 40
#define WINDOW_SEATS 10
#define AISLE_SEATS 10
#define REGULAR_SEATS 20

#define SEAT_FREE 0
#define SEAT_BOOKED 1
#define SEAT_RESERVED 2

typedef struct
{
    int seat_number;
    int status;    // 0: FREE, 1: BOOKED, 2: RESERVED
    char type[15]; // "Window", "Aisle", "Regular"
    char passenger[50];
    int age;
} Seat;

// Function Prototypes
void init_default_seats(Seat seats[], int size);
int load_seats_from_file(const char *filename, Seat seats[], int size);
int save_seats_to_file(const char *filename, const Seat seats[], int size);
int allocate_seat(Seat seats[], int size, const char *preferred_type, const char *passenger_name, int age);
int cancel_seat(Seat seats[], int size, int seat_number);
void update_seat_ui_grid(GtkWidget *grid, Seat seats[], int size, GCallback on_seat_click);

#endif // SEAT_MANAGER_H