#include "event_api_controller.h"

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

// GET /api/events/layout/{id}
int api_event_layout(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    const char* id_str = info->local_uri + strlen("/api/events/layout/");
    int id = atoi(id_str);
    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    json_t* json = get_event_layout((PGconn*)data, id);
    return send_json(conn, json);
}
