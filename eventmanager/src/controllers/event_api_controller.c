#include "event_api_controller.h"

// GET /api/events
int api_events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/api/events") == 0) {
        EventFilters filters = { 0 };

        filters.upcoming = 1;

        char search[256] = "";
        char city[128] = "";
        char category[32] = "";

        if (info->query_string) {
            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "search",
                search,
                sizeof(search)
            );

            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "city",
                city,
                sizeof(city)
            );

            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "category",
                category,
                sizeof(category)
            );
        }

        filters.search = search;
        filters.city = city;
        filters.category_id = atoi(category);

        json_t* json = json_array();

        get_events((PGconn*)data, &filters, json);

        return send_json(conn, json);
    }
    else {
        const char* id_str = info->local_uri + strlen("/api/events/");
        int id = atoi(id_str);
        if (id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        json_t* json = get_event((PGconn*)data, id);
        return send_json(conn, json);
    }

    return 0;
}

// GET /api/events/seatmap/{id}
int api_event_seatmap(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    const char* id_str = info->local_uri + strlen("/api/events/seatmap/");
    int id = atoi(id_str);
    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    json_t* json = get_event_seatmap((PGconn*)data, id);
    return send_json(conn, json);
}

// GET/PATCH/DELETE /api/admin/events
int api_admin_events(struct mg_connection* conn, void* data) {
    if (check_role(conn, ROLE_USER)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    // /api/admin/events
    if (strcmp(info->local_uri, "/api/admin/events") == 0) {
        if (strcmp(info->request_method, "GET") == 0) {
            EventFilters filters = { 0 };

            filters.upcoming = 0;

            char search[256] = "";
            char city[128] = "";
            char category[32] = "";

            if (info->query_string) {
                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "search",
                    search,
                    sizeof(search)
                );

                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "city",
                    city,
                    sizeof(city)
                );

                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "category",
                    category,
                    sizeof(category)
                );
            }

            filters.search = search;
            filters.city = city;
            filters.category_id = atoi(category);

            json_t* json = json_array();

            int result = get_events(db, &filters, json);

            return send_json(conn, json);
        }
        if (strcmp(info->request_method, "POST") == 0) {
            if (check_role(conn, ROLE_USER)) {
                mg_send_http_error(conn, 403, "Forbidden");
                return 403;
            }

            json_t* req = get_json(conn);
            if (!req) return 400;

            json_t* res = json_object();

            Session* s = get_session(conn);

            int venue_id = json_integer_value(json_object_get(req, "venue_id"));
            const char* title = json_string_value(json_object_get(req, "title"));
            const char* begins_at = json_string_value(json_object_get(req, "begins_at"));
            double price = json_number_value(json_object_get(req, "price"));
            int capacity = json_integer_value(json_object_get(req, "capacity"));
            int result = add_event(db, s->user_id, venue_id, title, begins_at, price, capacity);

            set_result(res, result);
            json_decref(req);
            return send_json(conn, res);
        }

    }

    // /api/admin/events/{id}
    const char* id_str = info->local_uri + strlen("/api/admin/events/");
    int id = atoi(id_str);

    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    if (strcmp(info->request_method, "GET") == 0) {
        return send_json(conn, get_event(db, id));
    }

    if (strcmp(info->request_method, "PATCH") == 0) {
        json_t* req = get_json(conn);
        if (!req)
            return 400;

        json_t* res = json_object();
        int result = 0;

        const char* title = json_string_value(json_object_get(req, "title"));
        if (title)
            result = admin_update_title(db, id, title);

        const char* begins_at = json_string_value(json_object_get(req, "begins_at"));
        if (begins_at)
            result = admin_update_begins_at(db, id, begins_at);

        json_t* verified_json = json_object_get(req, "verified");
        if (!json_is_boolean(verified_json))
            result = 0;
        else if (json_boolean_value(verified_json))
            result = verify_event(db, id);
        else
            result = unverify_event(db, id);

        set_result(res, result);

        json_decref(req);
        return send_json(conn, res);
    }

    if (strcmp(info->request_method, "DELETE") == 0) {
        json_t* res = json_object();
        set_result(res, delete_event(db, id));
        return send_json(conn, res);
    }

    mg_send_http_error(conn, 405, "Method Not Allowed");
    return 405;
}

