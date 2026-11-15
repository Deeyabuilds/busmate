#include <stdio.h>

#include "../include/notification.h"

int send_notification(const char *message, const char *recipient_id)
{
    if (message == NULL || message[0] == '\0' ||
        recipient_id == NULL || recipient_id[0] == '\0')
        return 0;

    printf("Notification to %s: %s\n", recipient_id, message);
    return 1;
}