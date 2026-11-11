# BusMate File Handling

## Purpose

File handling is used to store BusMate data permanently so that information is available after the program is closed.

## Data Files

```text
data/
├── students.dat
├── drivers.dat
├── buses.dat
├── routes.dat
├── schedules.dat
└── seats.dat
```

## Basic Operations

```text
Program
   ↓
Shared file_handler API
   ↓
Read / Write / Update
   ↓
Close File
```

`file_handler.c` owns data-file creation and the shared save, lookup, dynamic
load, and seat-update operations declared in
`include/file_handler.h`. Student CRUD is exposed through `saveStudent`,
`loadStudentById`, `loadAllStudentsDynamic`, and `deleteStudentById`; the
existing admin menu functions `addStudent`, `viewStudents`, and `removeStudent`
use these operations. `seat_allocation.c` uses the shared file-handler API for
seat requests, cancellations, and waiting-list promotion. The C project uses
these module functions in place of a Python-style `StudentManager` class.

The student dashboard delegates seat requests to `requestStudentSeat`. It
validates the bus and student, records the requested preference, and either
assigns an available seat or appends a waiting record. `BUSMATE_SEATS_PER_ROW`
in `include/busmate_config.h` configures row layout (default: four seats);
bus capacity comes from the stored bus record. A preference is best-effort:
when no matching seat is available, another available seat may be assigned.
`Front Row`, `Middle Row`, `Corner Seat`, `Window`, and `Any` are supported.

The seat-allocation helpers are declared in `include/seat_allocation.h`.
`getBusSeatCapacity` and `getAllocatedSeatCount` report the configured
per-bus capacity and current occupancy; `checkSeatAvailability` returns `1`
for an available seat, `0` for an occupied seat, and `-1` for invalid input or
an unreadable record file. Seat and bus-occupancy changes are written through
temporary files and swapped into place so the bus availability count follows
the committed seat records.

The GTK seat-allocation screen selects a bus directly, matching the bus-based
reservation model. It displays each seat as Available or Booked, appends
waiting students in FIFO order, and refreshes the occupancy and availability
summary after allocation or cancellation. Student records are managed through
the Students screen; booking actions use the entered student ID.

This checkout's reservation model is bus-based and stores assignments and
waiting requests in `seats.dat`; schedules are informational and there is no
`Booking` structure or `bookings.dat`. The seat-allocation interface retains
that existing model rather than introducing an incompatible schedule-based
booking format.

### Waiting list

Waiting-list entries are stored in `seats.dat` as seat records with seat number
`0` and status `Waiting`. Record order defines the queue order for each bus.
When a booked seat is released through `updateSeatStatus`, the first active
waiting entry for that bus is booked into the released seat and marked
`Promoted`. The student cancellation flow uses this same operation, so
promotion happens automatically. Cancelled and promoted entries remain in the
file as history and are not treated as active requests. `loadAllSeatsDynamic`
is shared by the student and seat-allocation interfaces instead of opening the
seat data file directly.

## File Handling Flow

```text
GUI
 ↓
Module
 ↓
Data Structure
 ↓
File Handling
 ↓
Data File
```

## Responsibilities

- Open files
- Read records
- Write records
- Update records
- Maintain persistent data
- Handle file errors
- Close files safely
- Create empty data files without pre-populating hard-coded sample records