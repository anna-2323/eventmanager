#include "controller.h"

// Помощна функция, обобщаваща действията при получаване на JSON
json_t* get_json(struct mg_connection* conn) {
    char body[1024] = "";
    mg_read(conn, body, sizeof(body) - 1);

    json_error_t err;
    json_t* req = json_loads(body, 0, &err);

    if (!req) {
        mg_send_http_error(conn, 400, "Invalid JSON");
        return NULL;
    }
    return req;
}

// Помощна функция, обобщаваща действията при изпращане на JSON
int send_json(struct mg_connection* conn, json_t* json) {
    if (!json) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    char* json_str = json_dumps(json, JSON_COMPACT);
    mg_send_http_ok(conn, "application/json", strlen(json_str));
    mg_write(conn, json_str, strlen(json_str));
    free(json_str);
    json_decref(json);
    return 1;
}

int check_role(struct mg_connection* conn, int role) {
    Session* s = get_session(conn);
    if (s->role == role)
        return 1;
    return 0;
}
