#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "student.h"
#include "structures.h"
#include "file_handler.h"

/* Member 1: Student-side module only. */
static int current_student_id = 0;

static void show_message(GtkWindow *parent, const char *message)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    g_signal_connect_swapped(dialog, "response",
        G_CALLBACK(gtk_window_destroy), dialog);
    gtk_window_present(GTK_WINDOW(dialog));
}

static int parse_positive_id(const char *text, int *value)
{
    char *end;
    long n;
    if (!text || !*text || !value) return 0;
    n = strtol(text, &end, 10);
    if (*end || n <= 0 || n > 2147483647L) return 0;
    *value = (int)n;
    return 1;
}

static int has_text(const char *s) { return s && s[0] != '\0'; }

int register_student(Student student)
{
    Student *items = NULL;
    size_t count = 0, i;
    if (student.studentId <= 0 || !has_text(student.name) ||
        !has_text(student.password)) return 0;
    if (readRecords(STUDENTS_FILE, sizeof(Student),
                    (void **)&items, &count) != 0) return 0;
    for (i = 0; i < count; ++i) {
        if (items[i].studentId == student.studentId) {
            free(items);
            return 0;
        }
    }
    free(items);
    return appendRecord(STUDENTS_FILE, sizeof(Student), &student) == 0;
}

int login_student(const char *student_id, const char *password)
{
    Student *items = NULL;
    size_t count = 0, i;
    int id, ok = 0;
    if (!parse_positive_id(student_id, &id) || !has_text(password)) return 0;
    if (readRecords(STUDENTS_FILE, sizeof(Student),
                    (void **)&items, &count) != 0) return 0;
    for (i = 0; i < count; ++i) {
        if (items[i].studentId == id && strcmp(items[i].password, password) == 0) {
            ok = 1; break;
        }
    }
    free(items);
    if (ok) current_student_id = id;
    return ok;
}

static void on_register_response(GtkDialog *dialog, int response, gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *idw = g_object_get_data(G_OBJECT(dialog), "id");
        GtkWidget *namew = g_object_get_data(G_OBJECT(dialog), "name");
        GtkWidget *passw = g_object_get_data(G_OBJECT(dialog), "password");
        Student s = {0};
        int id;
        if (!parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(idw)), &id)) {
            show_message(GTK_WINDOW(data), "Enter a valid numeric College ID.");
        } else {
            s.studentId = id;
            g_strlcpy(s.name, gtk_editable_get_text(GTK_EDITABLE(namew)), sizeof(s.name));
            g_strlcpy(s.password, gtk_editable_get_text(GTK_EDITABLE(passw)), sizeof(s.password));
            if (!has_text(s.name) || !has_text(s.password))
                show_message(GTK_WINDOW(data), "Name and password cannot be empty.");
            else if (register_student(s))
                show_message(GTK_WINDOW(data), "Registration successful. You can now log in.");
            else
                show_message(GTK_WINDOW(data), "Registration failed. ID may already exist or data file could not be saved.");
        }
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void show_registration(GtkWindow *parent)
{
    GtkWidget *d = gtk_dialog_new_with_buttons("Student Registration", parent,
        GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Register", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *id = gtk_entry_new(), *name = gtk_entry_new(), *pass = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(id), "Numeric College ID");
    gtk_entry_set_placeholder_text(GTK_ENTRY(name), "Student Name");
    gtk_entry_set_placeholder_text(GTK_ENTRY(pass), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(pass), FALSE);
    gtk_box_append(GTK_BOX(content), id);
    gtk_box_append(GTK_BOX(content), name);
    gtk_box_append(GTK_BOX(content), pass);
    g_object_set_data(G_OBJECT(d), "id", id);
    g_object_set_data(G_OBJECT(d), "name", name);
    g_object_set_data(G_OBJECT(d), "password", pass);
    g_signal_connect(d, "response", G_CALLBACK(on_register_response), parent);
    gtk_window_present(GTK_WINDOW(d));
}

static void show_records(GtkWindow *parent, const char *heading,
                         const char *path, size_t size, int kind)
{
    void *records = NULL;
    size_t count = 0, i;
    GString *out = g_string_new(heading);
    if (readRecords(path, size, &records, &count) != 0) {
        show_message(parent, "Could not read the requested records.");
        g_string_free(out, TRUE); return;
    }
    for (i = 0; i < count; ++i) {
        if (kind == 1) {
            Bus *b = (Bus *)records + i;
            g_string_append_printf(out, "\nBus ID: %d | Number: %s | Capacity: %d | Available: %d",
                b->busId, b->busNumber, b->capacity, b->availableSeats);
        } else if (kind == 2) {
            Route *r = (Route *)records + i;
            g_string_append_printf(out, "\nRoute ID: %d | %s | Stops: %s",
                r->routeId, r->routeName, r->stops);
        } else if (kind == 3) {
            Schedule *s = (Schedule *)records + i;
            g_string_append_printf(out, "\nSchedule: %d | Bus: %d | Route: %d | %s - %s | %s",
                s->scheduleId, s->busId, s->routeId, s->startTime, s->endTime, s->dayType);
        }
    }
    if (count == 0) g_string_append(out, "\nNo records found.");
    show_message(parent, out->str);
    free(records);
    g_string_free(out, TRUE);
}

static void on_view_buses(GtkButton *b, gpointer data)
{ (void)b; show_records(GTK_WINDOW(data), "AVAILABLE BUSES", BUSES_FILE, sizeof(Bus), 1); }
static void on_view_routes(GtkButton *b, gpointer data)
{ (void)b; show_records(GTK_WINDOW(data), "ROUTES AND STOPS", ROUTES_FILE, sizeof(Route), 2); }
static void on_view_schedules(GtkButton *b, gpointer data)
{ (void)b; show_records(GTK_WINDOW(data), "BUS TIMINGS / SCHEDULES", SCHEDULES_FILE, sizeof(Schedule), 3); }

static void on_search_bus_response(GtkDialog *dialog, int response, gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "bus");
        const char *query = gtk_editable_get_text(GTK_EDITABLE(entry));
        Bus *items = NULL; size_t count = 0, i; int id, found = 0;
        GString *out = g_string_new("BUS SEARCH RESULTS\n");
        int numeric = parse_positive_id(query, &id);
        if (readRecords(BUSES_FILE, sizeof(Bus), (void **)&items, &count) != 0) {
            show_message(GTK_WINDOW(data), "Could not read bus records.");
        } else {
            for (i = 0; i < count; ++i) {
                if ((numeric && items[i].busId == id) ||
                    (!numeric && g_ascii_strcasecmp(items[i].busNumber, query) == 0)) {
                    g_string_append_printf(out, "\nBus ID: %d | Number: %s | Capacity: %d | Available seats: %d",
                        items[i].busId, items[i].busNumber, items[i].capacity, items[i].availableSeats);
                    found = 1;
                }
            }
            if (!found) g_string_append(out, "\nNo matching bus found. Search by Bus ID or Bus Number.");
            show_message(GTK_WINDOW(data), out->str);
        }
        free(items); g_string_free(out, TRUE);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_search_bus(GtkButton *button, gpointer data)
{
    GtkWidget *d = gtk_dialog_new_with_buttons("Search Bus", GTK_WINDOW(data), GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CANCEL, "_Search", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *entry = gtk_entry_new();
    (void)button;
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter Bus ID or Bus Number");
    gtk_box_append(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), entry);
    g_object_set_data(G_OBJECT(d), "bus", entry);
    g_signal_connect(d, "response", G_CALLBACK(on_search_bus_response), data);
    gtk_window_present(GTK_WINDOW(d));
}

static void on_seat_availability_response(GtkDialog *dialog, int response, gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *entry = g_object_get_data(G_OBJECT(dialog), "bus");
        int bus_id; Seat *items = NULL; size_t count = 0, i; int booked = 0, total = 0;
        GString *out = g_string_new("SEAT AVAILABILITY\n");
        if (!parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(entry)), &bus_id)) {
            show_message(GTK_WINDOW(data), "Enter a valid Bus ID.");
        } else if (readRecords(SEATS_FILE, sizeof(Seat), (void **)&items, &count) != 0) {
            show_message(GTK_WINDOW(data), "Could not read seat records.");
        } else {
            for (i = 0; i < count; ++i) if (items[i].busId == bus_id) {
                ++total;
                if (strcmp(items[i].status, "Booked") == 0) ++booked;
            }
            g_string_append_printf(out, "\nBus ID: %d\nSeat records: %d\nBooked: %d\nAvailable in recorded seats: %d",
                bus_id, total, booked, total - booked);
            if (total == 0) g_string_append(out, "\nNo individual seat records exist for this bus yet.");
            show_message(GTK_WINDOW(data), out->str);
        }
        free(items); g_string_free(out, TRUE);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_check_seats(GtkButton *button, gpointer data)
{
    GtkWidget *d = gtk_dialog_new_with_buttons("Check Seat Availability", GTK_WINDOW(data), GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CANCEL, "_Check", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *entry = gtk_entry_new(); (void)button;
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Bus ID");
    gtk_box_append(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), entry);
    g_object_set_data(G_OBJECT(d), "bus", entry);
    g_signal_connect(d, "response", G_CALLBACK(on_seat_availability_response), data);
    gtk_window_present(GTK_WINDOW(d));
}

static void on_my_bookings(GtkButton *button, gpointer data)
{
    Seat *items = NULL; size_t count = 0, i; int found = 0;
    GString *out = g_string_new("MY ACTIVE BOOKINGS\n"); (void)button;
    if (readRecords(SEATS_FILE, sizeof(Seat), (void **)&items, &count) != 0) {
        show_message(GTK_WINDOW(data), "Could not read seat records.");
    } else {
        for (i = 0; i < count; ++i) if (items[i].studentId == current_student_id &&
            strcmp(items[i].status, "Booked") == 0) {
            g_string_append_printf(out, "\nBus ID: %d | Seat: %d | Preference: %s",
                items[i].busId, items[i].seatNumber, items[i].preference); found = 1;
        }
        if (!found) g_string_append(out, "\nNo active bookings found.");
        show_message(GTK_WINDOW(data), out->str);
    }
    free(items); g_string_free(out, TRUE);
}

/* Student-side booking request. Seat availability/allocation rules are coordinated
   with Member 3; this function records the student's selected seat request. */
static void on_booking_response(GtkDialog *dialog, int response, gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *bw = g_object_get_data(G_OBJECT(dialog), "bus");
        GtkWidget *sw = g_object_get_data(G_OBJECT(dialog), "seat");
        GtkWidget *dw = g_object_get_data(G_OBJECT(dialog), "day");
        GtkWidget *pw = g_object_get_data(G_OBJECT(dialog), "preference");
        int bus_id, seat_no; const char *day = gtk_editable_get_text(GTK_EDITABLE(dw));
        const char *pref = gtk_editable_get_text(GTK_EDITABLE(pw));
        Bus *buses = NULL; Schedule *schedules = NULL; Seat *seats = NULL;
        size_t bc = 0, sc = 0, nc = 0, i; int bus_found = 0, schedule_found = 0, seat_found = 0;
        if (!parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(bw)), &bus_id) ||
            !parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(sw)), &seat_no) ||
            (strcmp(day,"Full Day") && strcmp(day,"Half Day")) ||
            (strcmp(pref,"Window") && strcmp(pref,"Front") && strcmp(pref,"Any"))) {
            show_message(GTK_WINDOW(data), "Enter valid Bus ID, seat number, Full Day/Half Day and Window/Front/Any.");
            goto done;
        }
        if (readRecords(BUSES_FILE,sizeof(Bus),(void **)&buses,&bc) ||
            readRecords(SCHEDULES_FILE,sizeof(Schedule),(void **)&schedules,&sc) ||
            readRecords(SEATS_FILE,sizeof(Seat),(void **)&seats,&nc)) {
            show_message(GTK_WINDOW(data), "Could not read booking information."); goto done;
        }
        for (i=0;i<bc;i++) if (buses[i].busId==bus_id && buses[i].availableSeats>0) bus_found=1;
        for (i=0;i<sc;i++) if (schedules[i].busId==bus_id && strcmp(schedules[i].dayType,day)==0) schedule_found=1;
        for (i=0;i<nc;i++) if (seats[i].busId==bus_id && seats[i].seatNumber==seat_no) {
            seat_found=1; if (!strcmp(seats[i].status,"Booked")) {
                show_message(GTK_WINDOW(data), "That seat is already booked."); goto done;
            }
        }
        if (!bus_found) { show_message(GTK_WINDOW(data), "Bus not found or no seats available."); goto done; }
        if (!schedule_found) { show_message(GTK_WINDOW(data), "No matching schedule for this day type."); goto done; }
        {
            Seat seat = {0}; seat.busId=bus_id; seat.seatNumber=seat_no; seat.studentId=current_student_id;
            g_strlcpy(seat.status,"Booked",sizeof(seat.status));
            g_strlcpy(seat.preference,pref,sizeof(seat.preference));
            if (seat_found) {
                for (i=0;i<nc;i++) if (seats[i].busId==bus_id && seats[i].seatNumber==seat_no) { seats[i]=seat; break; }
                if (writeRecords(SEATS_FILE,sizeof(Seat),seats,nc)!=0) {
                    show_message(GTK_WINDOW(data), "Could not save seat request."); goto done;
                }
            } else if (saveSeat(seat)!=0) {
                show_message(GTK_WINDOW(data), "Could not save seat request."); goto done;
            }
            show_message(GTK_WINDOW(data), "Booking request recorded. Seat allocation and final confirmation are handled with Member 3's module.");
        }
    }
done:
    free(buses); free(schedules); free(seats);
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_book_seat(GtkButton *button, gpointer data)
{
    GtkWidget *d = gtk_dialog_new_with_buttons("Book Seat", GTK_WINDOW(data), GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CANCEL, "_Submit Request", GTK_RESPONSE_ACCEPT, NULL);
    GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *bus=gtk_entry_new(), *seat=gtk_entry_new(), *day=gtk_entry_new(), *pref=gtk_entry_new();
    (void)button;
    gtk_entry_set_placeholder_text(GTK_ENTRY(bus),"Bus ID");
    gtk_entry_set_placeholder_text(GTK_ENTRY(seat),"Preferred Seat Number");
    gtk_entry_set_placeholder_text(GTK_ENTRY(day),"Full Day or Half Day");
    gtk_entry_set_placeholder_text(GTK_ENTRY(pref),"Window, Front or Any");
    gtk_box_append(GTK_BOX(content),bus); gtk_box_append(GTK_BOX(content),seat);
    gtk_box_append(GTK_BOX(content),day); gtk_box_append(GTK_BOX(content),pref);
    g_object_set_data(G_OBJECT(d),"bus",bus); g_object_set_data(G_OBJECT(d),"seat",seat);
    g_object_set_data(G_OBJECT(d),"day",day); g_object_set_data(G_OBJECT(d),"preference",pref);
    g_signal_connect(d,"response",G_CALLBACK(on_booking_response),data);
    gtk_window_present(GTK_WINDOW(d));
}

static void on_cancel_response(GtkDialog *dialog, int response, gpointer data)
{
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkWidget *bw=g_object_get_data(G_OBJECT(dialog),"bus");
        GtkWidget *sw=g_object_get_data(G_OBJECT(dialog),"seat");
        int bus_id, seat_no, found=0; Seat *items=NULL; size_t count=0,i;
        if (!parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(bw)),&bus_id) ||
            !parse_positive_id(gtk_editable_get_text(GTK_EDITABLE(sw)),&seat_no)) {
            show_message(GTK_WINDOW(data),"Enter valid Bus ID and Seat Number.");
        } else if (readRecords(SEATS_FILE,sizeof(Seat),(void **)&items,&count)!=0) {
            show_message(GTK_WINDOW(data),"Could not read seat records.");
        } else {
            for (i=0;i<count;i++) if (items[i].busId==bus_id && items[i].seatNumber==seat_no &&
                items[i].studentId==current_student_id && !strcmp(items[i].status,"Booked")) {
                g_strlcpy(items[i].status,"Available",sizeof(items[i].status));
                items[i].studentId=0; found=1; break;
            }
            if (!found) show_message(GTK_WINDOW(data),"No active booking found for your account with those details.");
            else if (writeRecords(SEATS_FILE,sizeof(Seat),items,count)!=0)
                show_message(GTK_WINDOW(data),"Could not save cancellation. Contact the project team.");
            else show_message(GTK_WINDOW(data),"Cancellation request recorded. Member 3's module must synchronize seat availability.");
        }
        free(items);
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_cancel_booking(GtkButton *button, gpointer data)
{
    GtkWidget *d=gtk_dialog_new_with_buttons("Cancel Booking",GTK_WINDOW(data),GTK_DIALOG_MODAL,
        "_Close",GTK_RESPONSE_CANCEL,"_Cancel Booking",GTK_RESPONSE_ACCEPT,NULL);
    GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *bus=gtk_entry_new(), *seat=gtk_entry_new(); (void)button;
    gtk_entry_set_placeholder_text(GTK_ENTRY(bus),"Bus ID");
    gtk_entry_set_placeholder_text(GTK_ENTRY(seat),"Seat Number");
    gtk_box_append(GTK_BOX(content),bus); gtk_box_append(GTK_BOX(content),seat);
    g_object_set_data(G_OBJECT(d),"bus",bus); g_object_set_data(G_OBJECT(d),"seat",seat);
    g_signal_connect(d,"response",G_CALLBACK(on_cancel_response),data);
    gtk_window_present(GTK_WINDOW(d));
}

void show_student_dashboard(GtkWindow *parent)
{
    GtkWidget *window=gtk_window_new(), *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    const char *labels[]={"View Available Buses","Search Bus","View Routes and Stops",
        "View Bus Timings","Check Seat Availability","Book Seat","My Bookings","Cancel Booking"};
    GCallback callbacks[]={G_CALLBACK(on_view_buses),G_CALLBACK(on_search_bus),
        G_CALLBACK(on_view_routes),G_CALLBACK(on_view_schedules),G_CALLBACK(on_check_seats),
        G_CALLBACK(on_book_seat),G_CALLBACK(on_my_bookings),G_CALLBACK(on_cancel_booking)};
    int i;
    gtk_window_set_title(GTK_WINDOW(window),"Student Dashboard");
    gtk_window_set_default_size(GTK_WINDOW(window),440,560);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(window),parent);
    gtk_widget_set_margin_top(box,18); gtk_widget_set_margin_bottom(box,18);
    gtk_widget_set_margin_start(box,18); gtk_widget_set_margin_end(box,18);
    gtk_box_append(GTK_BOX(box),gtk_label_new("BUSMATE STUDENT DASHBOARD"));
    for (i=0;i<8;i++) {
        GtkWidget *button=gtk_button_new_with_label(labels[i]);
        gtk_box_append(GTK_BOX(box),button);
        g_signal_connect(button,"clicked",callbacks[i],window);
    }
    gtk_window_set_child(GTK_WINDOW(window),box);
    gtk_window_present(GTK_WINDOW(window));
}

static void on_login_clicked(GtkButton *button, gpointer data)
{
    GtkWidget *window=GTK_WIDGET(data);
    GtkWidget *id=g_object_get_data(G_OBJECT(window),"login_id");
    GtkWidget *pass=g_object_get_data(G_OBJECT(window),"login_password");
    (void)button;
    if (login_student(gtk_editable_get_text(GTK_EDITABLE(id)),
                      gtk_editable_get_text(GTK_EDITABLE(pass))))
        show_student_dashboard(GTK_WINDOW(window));
    else show_message(GTK_WINDOW(window),"Invalid College ID or password.");
}

static void on_register_clicked(GtkButton *button, gpointer data)
{ (void)button; show_registration(GTK_WINDOW(data)); }

void show_student_login(GtkWindow *parent)
{
    GtkWidget *window=gtk_window_new(), *box=gtk_box_new(GTK_ORIENTATION_VERTICAL,12);
    GtkWidget *id=gtk_entry_new(), *pass=gtk_entry_new();
    GtkWidget *login=gtk_button_new_with_label("Login");
    GtkWidget *reg=gtk_button_new_with_label("Register");
    gtk_window_set_title(GTK_WINDOW(window),"Student Login");
    gtk_window_set_default_size(GTK_WINDOW(window),350,260);
    if (parent) gtk_window_set_transient_for(GTK_WINDOW(window),parent);
    gtk_widget_set_margin_top(box,20); gtk_widget_set_margin_bottom(box,20);
    gtk_widget_set_margin_start(box,20); gtk_widget_set_margin_end(box,20);
    gtk_entry_set_placeholder_text(GTK_ENTRY(id),"Numeric College ID");
    gtk_entry_set_placeholder_text(GTK_ENTRY(pass),"Password");
    gtk_entry_set_visibility(GTK_ENTRY(pass),FALSE);
    gtk_box_append(GTK_BOX(box),gtk_label_new("STUDENT LOGIN"));
    gtk_box_append(GTK_BOX(box),id); gtk_box_append(GTK_BOX(box),pass);
    gtk_box_append(GTK_BOX(box),login); gtk_box_append(GTK_BOX(box),reg);
    gtk_window_set_child(GTK_WINDOW(window),box);
    g_object_set_data(G_OBJECT(window),"login_id",id);
    g_object_set_data(G_OBJECT(window),"login_password",pass);
    g_signal_connect(login,"clicked",G_CALLBACK(on_login_clicked),window);
    g_signal_connect(reg,"clicked",G_CALLBACK(on_register_clicked),window);
    gtk_window_present(GTK_WINDOW(window));
}