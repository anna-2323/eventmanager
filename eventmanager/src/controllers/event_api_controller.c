#include "event_api_controller.h"

static int handle_edit_event();

// GET /api/events
int api_events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/api/events") == 0) {
        char search[256] = "";
        char sort[256] = "";

        if (info->query_string) {
            mg_get_var(info->query_string, strlen(info->query_string), "search", search, sizeof(search));
            mg_get_var(info->query_string, strlen(info->query_string), "sort", sort, sizeof(sort));
        }

        json_t* json = json_array();
        get_events((PGconn*)data, search, sort, json);
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
    if (check_role(conn, ROLE_ADMIN) < 1) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    // /api/admin/events
    if (strcmp(info->local_uri, "/api/admin/events") == 0) {
        if (strcmp(info->request_method, "GET") != 0) {
            mg_send_http_error(conn, 405, "Method Not Allowed");
            return 405;
        }

        char search[256] = "";
        char sort[256] = "";

        if (info->query_string) {
            mg_get_var(info->query_string, strlen(info->query_string),
                "search", search, sizeof(search));
            mg_get_var(info->query_string, strlen(info->query_string),
                "sort", sort, sizeof(sort));
        }

        json_t* json = json_array();
        int result = get_events(db, search, sort, json);
        return send_json(conn, json);
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

