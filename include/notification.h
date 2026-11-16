#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "structures.h"

/* Notification operations */

int add_notification(Notification notification);

int search_notification(int notificationId,
                        Notification *notification);

int update_notification(Notification notification);

int delete_notification(int notificationId);

#endif