
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/file_handler.h"
#include "../include/route.h"

int add_route(Route route)
{
    if (route.routeId <= 0)
        return 0;

    return appendRecord(ROUTES_FILE, sizeof(Route), &route) == 0;
}

int view_routes(void)
{
    Route *routes = NULL;
    size_t count = 0;
    size_t i;

    if (readRecords(ROUTES_FILE, sizeof(Route),
                    (void **)&routes, &count) != 0)
        return 0;

    for (i = 0; i < count; i++)
    {
        printf("Route ID: %d | Name: %s | Stops: %s | %s to %s\n",
               routes[i].routeId,
               routes[i].routeName,
               routes[i].stops,
               routes[i].starting_point,
               routes[i].destination);
    }

    free(routes);
    return 1;
}

int update_route(Route route)
{
    int updated = 0;

    if (route.routeId <= 0)
        return 0;

    if (updateRecordById(ROUTES_FILE, sizeof(Route),
                         offsetof(Route, routeId),
                         route.routeId, &route, &updated) != 0)
        return 0;

    return updated;
}

int delete_route(int route_id)
{
    int deleted = 0;

    if (route_id <= 0)
        return 0;

    if (deleteRecordById(ROUTES_FILE, sizeof(Route),
                         offsetof(Route, routeId),
                         route_id, &deleted) != 0)
        return 0;

    return deleted;
}
