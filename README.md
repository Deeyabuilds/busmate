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
