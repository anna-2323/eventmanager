#include "stats_controller.h"
#include "controller.h"
#include "../models/user.h"
#include "../models/event.h"
#include "../models/ticket.h"
#include "../models/venue.h"

int api_admin_stats(struct mg_connection* conn, void* data)
{
    if (!check_role(conn, ROLE_ADMIN)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_method, "GET") != 0) {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }

    PGconn* db = data;
    json_t* stats = json_object();

    if (strcmp(info->local_uri, "/api/admin/stats/monthly") == 0) {
        // потребители
        json_object_set_new(
            stats,
            "users_growth",
            get_users_growth(db, 0)
        );
        // събития
        json_object_set_new(
            stats,
            "events_growth",
            get_events_growth(db, 0)
        );
        // билети
        json_object_set_new(
            stats,
            "tickets_growth",
            get_tickets_growth(db, 0)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/daily") == 0) {
        // потребители
        json_object_set_new(
            stats,
            "users_growth",
            get_users_growth(db, 2)
        );
        // събития
        json_object_set_new(
            stats,
            "events_growth",
            get_events_growth(db, 2)
        );
        // билети
        json_object_set_new(
            stats,
            "tickets_growth",
            get_tickets_growth(db, 2)
        );
        char* json = json_dumps(stats, JSON_COMPACT);
        printf("%s\n", json);
        free(json);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/totals") == 0) {
        // потребители
        json_object_set_new(
            stats,
            "users",
            get_total_users(db)
        );
        // събития
        json_object_set_new(
            stats,
            "events",
            get_total_events(db)
        );
        // зали
        json_object_set_new(
            stats,
            "venues",
            get_total_venues(db)
        );
        // билети
        json_object_set_new(
            stats,
            "tickets",
            get_total_tickets(db)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/monthly") == 0) {
        json_object_set_new(
            stats,
            "revenue",
            get_revenue(db, 0)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/daily") == 0) {
        json_object_set_new(
            stats,
            "revenue",
            get_revenue(db, 2)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/venues") == 0) {
        json_object_set_new(
            stats,
            "revenue",
            get_revenue(db, 3)
        );
    }
    return send_json(conn, stats);
}

// GET /api/admin/stats/export
int api_admin_stats_export(struct mg_connection* conn, void* data)
{
    if (!check_role(conn, ROLE_ADMIN)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_method, "GET") != 0) {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }

    PGconn* db = (PGconn*)data;
    json_t* stats = json_object();

    // Content-Type: text/csv
    // потребители
    if (strcmp(info->local_uri, "/api/admin/stats/export/growth/users") == 0) {
        json_object_set_new(
            stats,
            "users_growth",
            get_users_growth(db, 1)
        );
    }
    // събития
    else if (strcmp(info->local_uri, "/api/admin/stats/export/growth/events") == 0) {
        json_object_set_new(
            stats,
            "events_growth",
            get_events_growth(db, 1)
        );
    }
    // билети
    else if (strcmp(info->local_uri, "/api/admin/stats/export/growth/tickets") == 0) {
        json_object_set_new(
            stats,
            "tickets_growth",
            get_tickets_growth(db, 1)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/export/revenue") == 0) {
        json_object_set_new(
            stats,
            "revenue",
            get_revenue(db, 1)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/export/revenue/venues") == 0) {
        json_object_set_new(
            stats,
            "revenue",
            get_revenue(db, 4)
        );
    }

    return send_json(conn, stats);
}