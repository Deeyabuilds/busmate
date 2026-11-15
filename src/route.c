#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/file_handler.h"
#include "../include/route.h"

int add_route(Route route)
{
    if (route.route_id[0] == '\0')
        return 0;
    return file_save_record(FILE_ROUTES_PATH, &route, sizeof(route),
                            offsetof(Route, route_id),
                            sizeof(route.route_id)) == FILE_STATUS_OK;
}

int view_routes(void)
{
    void *records = NULL;
    size_t count = 0;
    size_t index;
    FileStatus status = file_read_records(FILE_ROUTES_PATH, &records,
                                          sizeof(Route), &count);
    if (status == FILE_STATUS_NOT_FOUND)
        return 1;
    if (status != FILE_STATUS_OK)
        return 0;

    for (index = 0; index < count; ++index)
    {
        Route *routes = (Route *)records;
        printf("Route %s | %s to %s\n", routes[index].route_id,
               routes[index].starting_point, routes[index].destination);
    }
    free(records);
    return 1;
}

int update_route(Route route)
{
    return file_update_record(FILE_ROUTES_PATH, &route, sizeof(route),
                              offsetof(Route, route_id),
                              sizeof(route.route_id)) == FILE_STATUS_OK;
}

int delete_route(const char *route_id)
{
    return file_delete_record(FILE_ROUTES_PATH, route_id, sizeof(Route),
                              offsetof(Route, route_id),
                              sizeof(((Route *)0)->route_id)) == FILE_STATUS_OK;
}