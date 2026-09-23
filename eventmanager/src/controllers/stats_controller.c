#include "stats_controller.h"
#include "controller.h"
#include "../models/user.h"
#include "../models/event.h"
#include "../models/ticket.h"
#include "../models/venue.h"

json_t* growth_array_to_json(StatGrowth* stats, int count);
json_t* revenue_array_to_json(StatRevenue* stats, int count);

// GET /api/stats
int api_stats(struct mg_connection* conn, void* data)
{
    Session* s = get_session(conn);

    if (!check_role(conn, ROLE_ADMIN) && !check_role(conn, ROLE_ORGANIZATOR)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    int organizer_id = 0;
    if (s->role == ROLE_ORGANIZATOR)
        organizer_id = s->user_id;

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_method, "GET") != 0) {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }

    // Извличане на параметъра months (по подразбиране 0 = всички)
    int months = 0;
    if (info->query_string) {
        char months_str[16] = "";
        mg_get_var(info->query_string, strlen(info->query_string), "months", months_str, sizeof(months_str));
        if (months_str[0] != '\0')
            months = atoi(months_str);
    }

    PGconn* db = data;
    json_t* stats = json_object();

    if (strcmp(info->local_uri, "/api/stats/monthly") == 0) {
        StatGrowth* users = NULL;
        StatGrowth* events = NULL;
        StatGrowth* tickets = NULL;

        int users_count = -1;
        if (organizer_id == 0)
            users_count = get_users_growth(db, STAT_MONTHLY, months, &users);
        int events_count = get_events_growth(db, STAT_MONTHLY, organizer_id, months, &events);
        int tickets_count = get_tickets_growth(db, STAT_MONTHLY, organizer_id, months, &tickets);

        if (users_count < 0 || events_count < 0 || tickets_count < 0) {
            free(users); free(events); free(tickets);
            json_decref(stats);
            return 500;
        }

        if (organizer_id == 0)
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
    else if (strcmp(info->local_uri, "/api/stats/daily") == 0) {
        StatGrowth* users = NULL;
        StatGrowth* events = NULL;
        StatGrowth* tickets = NULL;

        int users_count = -1;
        if (organizer_id == 0)
            users_count = get_users_growth(db, STAT_DAILY, months, &users);
        int events_count = get_events_growth(db, STAT_DAILY, organizer_id, months, &events);
        int tickets_count = get_tickets_growth(db, STAT_DAILY, organizer_id, months, &tickets);

        if (users_count < 0 || events_count < 0 || tickets_count < 0) {
            free(users); free(events); free(tickets);
            json_decref(stats);
            return 500;
        }

        if (organizer_id == 0)
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
    else if (strcmp(info->local_uri, "/api/stats/totals") == 0) {
        if (organizer_id == 0) {
            json_object_set_new(
                stats,
                "users",
                json_integer(get_total_users(db))
            );
            json_object_set_new(
                stats,
                "venues",
                json_integer(get_total_venues(db))
            );
        }

        json_object_set_new(
            stats,
            "events",
            json_integer(get_total_events(db, organizer_id))
        );

        json_object_set_new(
            stats,
            "tickets",
            json_integer(get_total_tickets(db, organizer_id))
        );
    }
    else if (strcmp(info->local_uri, "/api/stats/revenue/monthly") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_MONTHLY, organizer_id, months, &revenue);
        if (count < 0) {
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
    else if (strcmp(info->local_uri, "/api/stats/revenue/daily") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_DAILY, organizer_id, months, &revenue);
        if (count < 0) {
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
    else if (strcmp(info->local_uri, "/api/stats/revenue/venues") == 0) {
        StatRevenue* revenue = NULL;

        int count = get_revenue(db, STAT_BY_VENUE, organizer_id, months, &revenue);
        if (count < 0) {
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
    return send_result(conn, 1, 200, "", stats);
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