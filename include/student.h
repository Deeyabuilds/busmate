#ifndef STUDENT_H
#define STUDENT_H

#include "structures.h"

/* Student operations */

int add_student(Student student);

int search_student(int studentId,
                   Student *student);

int update_student(Student student);

int delete_student(int studentId);

int login_student(int studentId,
                  const char *password);

#endif