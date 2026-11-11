#ifndef ADMIN_H
#define ADMIN_H

// System entry
void ensure_data_files(void);
int run_busmate_system(void);

// Login helpers
int adminLogin(const char *username, const char *password);
int studentLogin(int id, const char *password);
int driverLogin(int id, const char *password);

// Admin Menu
void adminMenu(void);

// Student Management
void studentManagement(void);
void addStudent(void);
void viewStudents(void);
void removeStudent(void);

// Bus Management
void busManagement(void);
void addBus(void);
void viewBuses(void);
void removeBus(void);

// Route Management
void routeManagement(void);
void addRoute(void);
void viewRoutes(void);
void removeRoute(void);

// Driver Management
void driverManagement(void);
void addDriver(void);
void viewDrivers(void);
void removeDriver(void);

// Schedule Management
void scheduleManagement(void);
void addSchedule(void);
void viewSchedules(void);
void removeSchedule(void);

// Student, driver and report features
void studentDashboard(int studentId);
void driverDashboard(int driverId);
void seatAllocationManagement(void);
void reportsMenu(void);

#endif