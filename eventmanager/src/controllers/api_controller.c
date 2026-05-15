#include "api_controller.h"

//
//  GET /api/events
//
int api_events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/api/events") == 0) {
        json_t* json = json_array();
        get_all_events((PGconn*)data, json);
        char* json_str = json_dumps(json, JSON_COMPACT);

        mg_send_http_ok(conn, "application/json", strlen(json_str));
        mg_write(conn, json_str, strlen(json_str));
        free(json_str);
        return 1;
    }
    else {
        const char* id_str = info->local_uri + strlen("/api/events/");
        int id = atoi(id_str);
        if (id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        json_t* json = get_event((PGconn*)data, id);
        if (!json) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }
        char* json_str = json_dumps(json, JSON_COMPACT);

        mg_send_http_ok(conn, "application/json", strlen(json_str));
        mg_write(conn, json_str, strlen(json_str));
        free(json_str);
        return 1;

    }

    return 0;
}

//
//  GET /api/users
//
int api_users(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/api/users") == 0) {
        json_t* json = json_array();
        get_all_users((PGconn*)data, json);
        char* json_str = json_dumps(json, JSON_COMPACT);

        mg_send_http_ok(conn, "application/json", strlen(json_str));
        mg_write(conn, json_str, strlen(json_str));
        free(json_str);
        return 1;
    }
    else {
        const char* id_str = info->local_uri + strlen("/api/users/");
        int id = atoi(id_str);
        if (id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        json_t* json = get_user((PGconn*)data, id);
        if (!json) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }
        char* json_str = json_dumps(json, JSON_COMPACT);

        mg_send_http_ok(conn, "application/json", strlen(json_str));
        mg_write(conn, json_str, strlen(json_str));
        free(json_str);
        return 1;

    }

    return 0;
}