#include <stdio.h>
#include <stdlib.h>

#include "../include/file_handler.h"
#include "../include/report.h"

static int print_record_count(const char *path, size_t record_size,
                              const char *label)
{
    void *records = NULL;
    size_t count = 0;
    FileStatus status = file_read_records(path, &records, record_size, &count);

    free(records);
    if (status == FILE_STATUS_NOT_FOUND)
        count = 0;
    else if (status != FILE_STATUS_OK)
    {
        fprintf(stderr, "Could not read %s report data: %s\n", label,
                file_status_message(status));
        return 0;
    }
    printf("%s records: %lu\n", label, (unsigned long)count);
    return 1;
}

int generate_booking_report(void)
{
    return print_record_count(FILE_BOOKINGS_PATH, sizeof(Booking), "Booking");
}

int generate_bus_report(void)
{
    return print_record_count(FILE_BUSES_PATH, sizeof(Bus), "Bus");
}

int generate_student_report(void)
{
    return print_record_count(FILE_STUDENTS_PATH, sizeof(Student), "Student");
}