# BusMate

**Smart College Bus Management System**

BusMate is a C-based GTK4 application with separate Admin, Student, and Driver
interfaces. The home screen opens a task-specific portal for each role.

## Building

From the project root, build and run BusMate:

```bat
build.bat
```

The build command compiles all modules with GTK4 flags from `pkg-config` and
starts `BusMate.exe` after a successful build. To build manually from an MSYS2
UCRT64 terminal:

```sh
gcc -std=c99 -Wall -Wextra -Werror src/main.c src/gui.c src/admin.c src/student.c src/driver.c src/bus.c src/route.c src/schedule.c src/booking.c src/seat_allocation.c src/file_handler.c src/notification.c src/report.c -o BusMate.exe -Iinclude $(pkg-config --cflags --libs gtk4) -mwindows
./BusMate.exe
```

## How the code is organised

The application is divided into modules, with each module handling a specific part of the system.

```text
                    main.c
                      |
                      v
                    gui.c
                      |
          +-----------+-----------+
          |           |           |
          v           v           v
        Admin       Student      Driver
          |           |           |
          +-----------+-----------+
                      |
       +--------------+--------------+
       |       |       |       |     |
       v       v       v       v     v
      Bus    Route  Schedule Booking Seat Allocation
       |       |       |       |     |
       +-------+-------+-------+-----+
                      |
          +-----------+-----------+
          |           |           |
          v           v           v
     Notification   Report   File Handler
                                  |
                                  v
                               Data Files
```

## Data Storage

The application uses files to store records.

| File                | Purpose                             |
| ------------------- | ----------------------------------- |
| `students.dat`      | Student records                     |
| `admins.dat`        | Admin records                       |
| `drivers.dat`       | Driver records                      |
| `buses.dat`         | Bus details                         |
| `routes.dat`        | Route information                   |
| `schedules.dat`     | Schedule details                    |
| `bookings.dat`      | Booking and seat allocation records |
| `seats.dat`         | Synchronized booked-seat records      |
| `notifications.dat` | Notification records                |
| `reports.dat`       | Report records                      |
| `waitlist.dat`      | Ordered seat requests and preferences |
| `temp.data`         | Active student, bus, and schedule IDs |
| `student_profiles.dat` | College ID and management status    |
| `bus_status.dat`    | Bus activation status                 |

The shared file operations are declared in `include/file_handler.h` and
implemented in `src/file_handler.c`. They provide status-returning append,
read, replace, find, save, update, and delete operations for fixed-size binary
records. The existing entity-specific functions remain available for other
modules. The `data` directory is created when a standard data file is first
written. Data files contain native C structures and should be used with the
same build/schema that created them.

Seat allocation is declared in `include/seat_allocation.h` and implemented in
`src/seat_allocation.c`. A schedule's bus determines its capacity. Window
preference selects an available odd-numbered seat; Front preference first
tries the lowest seats in the first 30% of capacity. Both fall back to the
lowest available seat. If the bus is full, requests are appended to the FIFO
waiting list. Cancelling a booking promotes the first waiting student for that
schedule. Bookings, `seats.dat`, and the active session file are synchronized
when a request is placed, a seat is allocated, or a request is cancelled.

The seat module exposes `allocate_seat_with_preference`, `cancel_seat`,
`check_seat_availability`, `get_seat_availability`,
`display_seat_availability`, and `display_waiting_list`. The original
`allocate_seat` interface remains available and requests any seat.
`allocate_seat_with_preference` returns `SEAT_ALLOCATION_BOOKED`,
`SEAT_ALLOCATION_WAITLISTED`, `SEAT_ALLOCATION_DUPLICATE`, or
`SEAT_ALLOCATION_ERROR`, so callers can distinguish an assigned seat from a
queued request or a failure.

The dark operations dashboard displays live student and seat counts and opens
Student Management or Seat Allocation. Student Management supports adding and
deleting records, with College ID and status stored separately so the shared
`Student` structure remains unchanged. Seat Allocation lists active buses and
their schedules from the data files, reports live occupancy, shows seat and
queue rows, and supports booking, cancellation, and bus creation. A valid
student and a schedule linked to the selected bus must already exist before
booking. The current `Student` structure stores passwords as entered, so this
is suitable only for a local classroom demo, not real credentials.

The home screen opens separate role flows: students can register or sign in and
manage their own seat booking; admins can review students and bus/schedule
records or open seat allocation; drivers can look up their assigned bus and
schedules by driver ID. Driver records currently have no password field, so the
driver screen is a record lookup rather than secure authentication.

## Project Structure

```text
BusMate/
├── include/
├── src/
├── data/
├── documentation/
├── tests/
├── build.bat
└── README.md
```

For details about the system design and implementation, see the `documentation/` directory.
