#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/seat_manager.h"

// Default seats initialize karne ke liye
void init_default_seats(Seat seats[], int size)
{
    for (int i = 0; i < size; i++)
    {
        seats[i].seat_number = i + 1;
        seats[i].status = SEAT_FREE;
        seats[i].passenger[0] = '\0';
        seats[i].age = 0;

        if (i < WINDOW_SEATS)
        {
            strcpy(seats[i].type, "Window");
        }
        else if (i < WINDOW_SEATS + AISLE_SEATS)
        {
            strcpy(seats[i].type, "Aisle");
        }
        else
        {
            strcpy(seats[i].type, "Regular");
        }
    }
}

// Binary file se seat status load karne ka function
int load_seats_from_file(const char *filename, Seat seats[], int size)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        init_default_seats(seats, size);
        save_seats_to_file(filename, seats, size);
        return 0;
    }

    size_t read_count = fread(seats, sizeof(Seat), size, fp);
    fclose(fp);

    if (read_count != (size_t)size)
    {
        init_default_seats(seats, size);
        return -1;
    }
    return 1;
}

// Binary file mein seat data save karne ke liye
int save_seats_to_file(const char *filename, const Seat seats[], int size)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        return 0;
    }

    fwrite(seats, sizeof(Seat), size, fp);
    fclose(fp);
    return 1;
}

// Preferred seat allocate karne ka logic
int allocate_seat(Seat seats[], int size, const char *preferred_type, const char *passenger_name, int age)
{
    int allocated_index = -1;

    for (int i = 0; i < size; i++)
    {
        if (seats[i].status == SEAT_FREE && strcmp(seats[i].type, preferred_type) == 0)
        {
            allocated_index = i;
            break;
        }
    }

    if (allocated_index == -1)
    {
        for (int i = 0; i < size; i++)
        {
            if (seats[i].status == SEAT_FREE)
            {
                allocated_index = i;
                break;
            }
        }
    }

    if (allocated_index != -1)
    {
        seats[allocated_index].status = SEAT_BOOKED;
        strncpy(seats[allocated_index].passenger, passenger_name, 49);
        seats[allocated_index].passenger[49] = '\0';
        seats[allocated_index].age = age;
        return seats[allocated_index].seat_number;
    }

    return -1;
}

// Seat cancel karne ka logic
int cancel_seat(Seat seats[], int size, int seat_number)
{
    if (seat_number < 1 || seat_number > size)
    {
        return 0;
    }

    int idx = seat_number - 1;
    if (seats[idx].status == SEAT_FREE)
    {
        return 0;
    }

    seats[idx].status = SEAT_FREE;
    seats[idx].passenger[0] = '\0';
    seats[idx].age = 0;

    return 1;
}

// GTK Grid UI render karne ke liye helper
void update_seat_ui_grid(GtkWidget *grid, Seat seats[], int size, GCallback on_seat_click)
{
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    int cols = 4;
    for (int i = 0; i < size; i++)
    {
        char label[32];
        snprintf(label, sizeof(label), "S%d\n(%s)", seats[i].seat_number, seats[i].type);

        GtkWidget *btn = gtk_button_new_with_label(label);

        if (seats[i].status == SEAT_BOOKED)
        {
            gtk_widget_set_sensitive(btn, FALSE);
        }

        g_signal_connect(btn, "clicked", on_seat_click, GINT_TO_POINTER(seats[i].seat_number));

        int row = i / cols;
        int col = i % cols;
        gtk_grid_attach(GTK_GRID(grid), btn, col, row, 1, 1);
    }
    gtk_widget_show_all(grid);
}