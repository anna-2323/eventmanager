#include "api_controller.h"
#include "../session.h"
#include "../models/ticket.h"

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

//
// POST /api/purchase/{event_id}
//
int api_purchase_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    int event_id = atoi(info->local_uri + strlen("/api/purchase/"));
    if (event_id <= 0) { mg_send_http_error(conn, 400, "Invalid ID"); return 400; }

    char body[1024] = "";
    mg_read(conn, body, sizeof(body) - 1);

    json_error_t err;
    json_t* req = json_loads(body, 0, &err);
    if (!req) { mg_send_http_error(conn, 400, "Invalid JSON"); return 400; }

    const char* first_name = json_string_value(json_object_get(req, "first_name"));
    const char* last_name = json_string_value(json_object_get(req, "last_name"));
    const char* email = json_string_value(json_object_get(req, "email"));
    const char* phone = json_string_value(json_object_get(req, "phone"));

    int ticket_id = 0;
    int result = purchase_ticket(db, event_id, first_name, last_name, email, phone, &ticket_id);

    json_t* res = json_object();
    if (result == 1) {
        json_object_set_new(res, "success", json_true());
        json_object_set_new(res, "ticket_id", json_integer(ticket_id));
    }
    else if (result == -1) {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Няма свободни места."));
    }
    else {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Възникна грешка."));
    }

    char* json_str = json_dumps(res, JSON_COMPACT);
    mg_send_http_ok(conn, "application/json", strlen(json_str));
    mg_write(conn, json_str, strlen(json_str));

    free(json_str);
    json_decref(req);
    json_decref(res);
    return 1;
}

//
// GET /api/confirmation/{ticket_id}
//
int api_confirm_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    int ticket_id = atoi(info->local_uri + strlen("/api/confirmation/"));
    if (ticket_id <= 0) { mg_send_http_error(conn, 400, "Invalid ID"); return 400; }

    json_t* ticket = confirm_ticket(db, ticket_id);
    if (!ticket) { mg_send_http_error(conn, 404, "Not found"); return 404; }

    char* json_str = json_dumps(ticket, JSON_COMPACT);
    mg_send_http_ok(conn, "application/json", strlen(json_str));
    mg_write(conn, json_str, strlen(json_str));

    free(json_str);
    json_decref(ticket);
    return 1;
}

//
// GET /api/me
//
int api_me(struct mg_connection* conn, void* data) {
    Session* s = get_session(conn);
    json_t* res = json_object();

    // Ако потребителят е влязъл
    if (s) {
        json_object_set_new(res, "logged_in", json_true());
        json_object_set_new(res, "email", json_string(s->email));
        json_object_set_new(res, "role", json_string(s->role));
    }
    // Ако потребителят не е влязъл
    else {
        json_object_set_new(res, "logged_in", json_false());
    }

    char* json_str = json_dumps(res, JSON_COMPACT);
    mg_send_http_ok(conn, "application/json", strlen(json_str));
    mg_write(conn, json_str, strlen(json_str));

    free(json_str);
    json_decref(res);
    return 1;
}

//
// POST /api/login
//
int api_login(struct mg_connection* conn, void* data) {
    char body[512] = "";
    mg_read(conn, body, sizeof(body) - 1);

    json_error_t err;
    json_t* req = json_loads(body, 0, &err);
    const char* email = json_string_value(json_object_get(req, "email"));
    const char* password = json_string_value(json_object_get(req, "password"));

    User user = { 0 };
    int ok = verify_user((PGconn*)data, email, password, &user);

    json_t* res = json_object();
    if (ok > 0) {
        // Създава се бисквитка
        Session* s = session_create(user.id, user.email, user.role);
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n\r\n",
            s->token);

        json_object_set_new(res, "success", json_true());
        json_object_set_new(res, "email", json_string(user.email));
        json_object_set_new(res, "role", json_string(user.role));
    }
    else {
        mg_printf(conn, "HTTP/1.1 401 Unauthorized\r\nContent-Type: application/json\r\n\r\n");
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Невалиден имейл или парола."));
    }

    char* json_str = json_dumps(res, JSON_COMPACT);
    mg_write(conn, json_str, strlen(json_str));
    free(json_str);
    json_decref(req);
    json_decref(res);
    return 1;
}

//
// POST /api/signup
//
int api_signup(struct mg_connection* conn, void* data) {
    char body[1024] = "";
    mg_read(conn, body, sizeof(body) - 1);

    json_error_t err;
    json_t* req = json_loads(body, 0, &err);
    if (!req) {
        mg_send_http_error(conn, 400, "Invalid JSON");
        return 400;
    }

    const char* fname = json_string_value(json_object_get(req, "first_name"));
    const char* lname = json_string_value(json_object_get(req, "last_name"));
    const char* email = json_string_value(json_object_get(req, "email"));
    const char* phone = json_string_value(json_object_get(req, "phone"));
    if (!phone) phone = "";
    const char* password = json_string_value(json_object_get(req, "password"));
    json_t* role_json = json_object_get(req, "role");
    int role = role_json ? json_integer_value(role_json) : 0;

    if (!fname || !lname || !email || !phone || !password) {
        json_decref(req);
        mg_send_http_error(conn, 400, "Липсват данни");
        return 400;
    }

    json_t* user = add_user((PGconn*)data, fname, lname, email, phone, password, role);
    json_decref(req);

    json_t* res = json_object();
    if (!user) {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Имейлът вече е регистриран."));
        char* json_str = json_dumps(res, JSON_COMPACT);
        mg_send_http_error(conn, 409, "%s", json_str);
        free(json_str);
        json_decref(res);
        return 409;
    }

    // Създаване на бисквитка за новия потребител
    const char* u_fname = json_string_value(json_object_get(user, "fname"));
    const char* u_lname = json_string_value(json_object_get(user, "lname"));
    const char* u_email = json_string_value(json_object_get(user, "email"));
    int u_role = json_integer_value(json_object_get(user, "role"));
    int u_id = json_integer_value(json_object_get(user, "id"));
    char role_str[8];
    snprintf(role_str, sizeof(role_str), "%d", u_role);

    Session* s = session_create(u_id, u_email, role_str);
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n\r\n",
        s->token);

    json_object_set_new(res, "success", json_true());
    char* json_str = json_dumps(res, JSON_COMPACT);
    mg_write(conn, json_str, strlen(json_str));

    free(json_str);
    json_decref(res);
    json_decref(user);
    return 200;
}

//
// GET /api/logout
//
int api_logout(struct mg_connection* conn, void* data) {
    Session* s = get_session(conn);
    if (s) session_delete(s->token);

    // Max-Age = 0
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Set-Cookie: session=; HttpOnly; Path=/; Max-Age=0\r\n\r\n"
        "{\"success\":true}");

    return 1;
}