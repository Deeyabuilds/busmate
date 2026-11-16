#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../include/notification.h"
#include "../include/file_handler.h"

int add_notification(Notification notification)
{
    void *records = NULL;
    Notification *notifications;
    size_t count = 0;
    size_t i;

    if (notification.notificationId <= 0 || notification.studentId <= 0)
        return -1;

    if (notification.message[0] == '\0')
        return -1;

    if (readRecords(NOTIFICATIONS_FILE, sizeof(Notification), &records, &count) == 0)
    {
        notifications = (Notification *)records;
        for (i = 0; i < count; i++)
        {
            if (notifications[i].notificationId == notification.notificationId)
            {
                free(records);
                return -2;
            }
        }
        free(records);
    }

    if (appendRecord(NOTIFICATIONS_FILE, sizeof(Notification), &notification) != 0)
        return -1;

    return 1;
}

int search_notification(int notificationId, Notification *notification)
{
    void *records = NULL;
    Notification *notifications;
    size_t count = 0;
    size_t i;

    if (notification == NULL)
        return -1;

    if (readRecords(NOTIFICATIONS_FILE, sizeof(Notification), &records, &count) != 0)
        return -1;

    notifications = (Notification *)records;
    for (i = 0; i < count; i++)
    {
        if (notifications[i].notificationId == notificationId)
        {
            *notification = notifications[i];
            free(records);
            return 1;
        }
    }

    free(records);
    return 0;
}

int update_notification(Notification notification)
{
    int updated = 0;

    if (notification.notificationId <= 0 || notification.studentId <= 0 ||
        notification.message[0] == '\0')
        return -1;

    if (updateRecordById(NOTIFICATIONS_FILE, sizeof(Notification),
                         offsetof(Notification, notificationId),
                         notification.notificationId, &notification, &updated) != 0)
        return -1;

    return updated ? 1 : 0;
}

int delete_notification(int notificationId)
{
    int deleted = 0;

    if (notificationId <= 0)
        return -1;

    if (deleteRecordById(NOTIFICATIONS_FILE, sizeof(Notification),
                         offsetof(Notification, notificationId), notificationId,
                         &deleted) != 0)
        return -1;

    return deleted ? 1 : 0;
}
