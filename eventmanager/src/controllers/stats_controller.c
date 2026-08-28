#include "stats_controller.h"
#include "controller.h"
#include "../models/user.h"
#include "../models/event.h"
#include "../models/ticket.h"
#include "../models/venue.h"

json_t* growth_array_to_json(StatGrowth* stats, int count);
json_t* revenue_array_to_json(StatRevenue* stats, int count);

// GET /api/admin/stats
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
        StatGrowth* users = NULL;
        StatGrowth* events = NULL;
        StatGrowth* tickets = NULL;

        int users_count = get_users_growth(db, STAT_MONTHLY, &users);
        int events_count = get_events_growth(db, STAT_MONTHLY, &events);
        int tickets_count = get_tickets_growth(db, STAT_MONTHLY, &tickets);

        if (users_count == 0 || events_count == 0 || tickets_count == 0) {
            free(users); free(events); free(tickets);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "users_growth",
            growth_array_to_json(users, users_count)
        );

        json_object_set_new(
            stats,
            "events_growth",
            growth_array_to_json(events, events_count)
        );

        json_object_set_new(
            stats,
            "tickets_growth",
            growth_array_to_json(tickets, tickets_count)
        );

        free(users);
        free(events);
        free(tickets);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/daily") == 0) {
        StatGrowth* users = NULL;
        StatGrowth* events = NULL;
        StatGrowth* tickets = NULL;

        int users_count = get_users_growth(db, STAT_DAILY, &users);
        int events_count = get_events_growth(db, STAT_DAILY, &events);
        int tickets_count = get_tickets_growth(db, STAT_DAILY, &tickets);

        if (users_count == 0 || events_count == 0 || tickets_count == 0) {
            free(users); free(events); free(tickets);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "users_growth",
            growth_array_to_json(users, users_count)
        );

        json_object_set_new(
            stats,
            "events_growth",
            growth_array_to_json(events, events_count)
        );

        json_object_set_new(
            stats,
            "tickets_growth",
            growth_array_to_json(tickets, tickets_count)
        );

        free(users);
        free(events);
        free(tickets);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/totals") == 0) {
        json_object_set_new(
            stats,
            "users",
            json_integer(get_total_users(db))
        );

        json_object_set_new(
            stats,
            "events",
            json_integer(get_total_events(db))
        );

        json_object_set_new(
            stats,
            "venues",
            json_integer(get_total_venues(db))
        );

        json_object_set_new(
            stats,
            "tickets",
            json_integer(get_total_tickets(db))
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/monthly") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_MONTHLY, &revenue);
        if (count == 0) {
            free(revenue);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "revenue",
            revenue_array_to_json(revenue, count)
        );

        free(revenue);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/daily") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_DAILY, &revenue);
        if (count == 0) {
            free(revenue);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "revenue",
            revenue_array_to_json(revenue, count)
        );

        free(revenue);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/revenue/venues") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_BY_VENUE, &revenue);
        if (count == 0) {
            free(revenue);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "revenue",
            revenue_array_to_json(revenue, count)
        );

        free(revenue);
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
        StatGrowth* users = NULL;
        int users_count = get_users_growth(db, STAT_MONTHLY_ALL, &users);

        if (users_count == 0) {
            free(users);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "users_growth",
            growth_array_to_json(users, users_count)
        );
        free(users);
    }
    // събития
    else if (strcmp(info->local_uri, "/api/admin/stats/export/growth/events") == 0) {
        StatGrowth* events = NULL;
        int events_count = get_events_growth(db, STAT_MONTHLY_ALL, &events);

        if (events_count == 0) {
            free(events);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "events_growth",
            growth_array_to_json(events, events_count)
        );
        free(events);
    }
    // билети
    else if (strcmp(info->local_uri, "/api/admin/stats/export/growth/tickets") == 0) {
        StatGrowth* tickets = NULL;
        int tickets_count = get_tickets_growth(db, STAT_MONTHLY_ALL, &tickets);

        if (tickets_count == 0) {
            free(tickets);
            json_decref(stats);
            return 500;
        }

        json_object_set_new(
            stats,
            "tickets_growth",
            growth_array_to_json(tickets, tickets_count)
        );
        free(tickets);
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/export/revenue") == 0) {
        StatRevenue* rev = NULL;
        int rev_count = get_revenue(db, STAT_MONTHLY_ALL, &rev);
        json_object_set_new(
            stats,
            "revenue",
            revenue_array_to_json(rev, rev_count)
        );
    }
    else if (strcmp(info->local_uri, "/api/admin/stats/export/revenue/venues") == 0) {
        StatRevenue* rev = NULL;
        int rev_count = get_revenue(db, STAT_BY_VENUE, &rev);
        json_object_set_new(
            stats,
            "revenue",
            revenue_array_to_json(rev, rev_count)
        );
    }

    return send_json(conn, stats);
}

json_t* growth_array_to_json(StatGrowth* stats, int count) {
    json_t* arr = json_array();
    for (int i = 0; i < count; i++) {
        json_array_append_new(arr, growth_to_json(&stats[i]));
    }
    return arr;
}

json_t* revenue_array_to_json(StatRevenue* stats, int count) {
    json_t* arr = json_array();
    for (int i = 0; i < count; i++) {
        json_array_append_new(arr, revenue_to_json(&stats[i]));
    }
    return arr;
}