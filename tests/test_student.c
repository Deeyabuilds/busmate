#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/student.h"

int main(void)
{
    Student student = {0};

    strcpy(student.student_id, "STUDENT-TEST");
    strcpy(student.name, "Test Student");
    strcpy(student.password, "test-password");

    assert(register_student(student));
    assert(!register_student(student));
    assert(login_student("STUDENT-TEST", "test-password"));
    assert(!login_student("STUDENT-TEST", "wrong-password"));
    assert(!login_student("UNKNOWN", "test-password"));

    puts("Student registration and login tests passed.");
    return 0;
}
