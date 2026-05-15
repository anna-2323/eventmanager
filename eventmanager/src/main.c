#include "civetweb.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include "controllers/api_controller.h"

static PGconn* db;

// handler
static int home_handler(struct mg_connection*, void*);
static int get_events_handler(struct mg_connection*, void*);
static int request_handler(struct mg_connection*, void*);

// помощни функции
static void init_db(void);

int main(void) {
    init_db();

    const char* options[] = {
        "listening_ports", "8080",
        "document_root", ".\\html",
        NULL
    };

    struct mg_callbacks callbacks = { 0 };
    struct mg_context* ctx = mg_start(&callbacks, NULL, options);

    mg_set_request_handler(ctx, "/home", home_handler, NULL);
    mg_set_request_handler(ctx, "/events", get_events_handler, NULL);
    mg_set_request_handler(ctx, "/api/events/**", api_events, db);
    mg_set_request_handler(ctx, "/api/events", api_events, db);
    mg_set_request_handler(ctx, "/api/users", api_users, db);

    printf("Server running on port 8080\n");
    getchar();

    mg_stop(ctx);
    return 0;
}

static void init_db(void) {
    db = PQconnectdb("host=localhost dbname=eventmanagement user=postgres password=secret");
    //...
}

// /home, /home?sort=recent
static int home_handler(struct mg_connection* conn, void* cbdata) {

    FILE* f = fopen("html/home.html", "rb");
    if (!f) {
        fprintf(stderr, "Empty file error\n");
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* home = malloc(fsize + 1);
    fread(home, 1, fsize, f);
    home[fsize] = '\0';
    fclose(f);

    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    mg_write(conn, home, strlen(home));

    free(home);
    return 1;
}

// /events
static int get_events_handler(struct mg_connection* conn, void* cbdata) {

    FILE* f = fopen("html/events.html", "rb");
    if (!f) {
        fprintf(stderr, "Empty file error\n");
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* events = malloc(fsize + 1);
    fread(events, 1, fsize, f);
    events[fsize] = '\0';
    fclose(f);

    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    mg_write(conn, events, strlen(events));

    free(events);
    return 1;
}

static int request_handler(struct mg_connection* conn, void* data) {

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_uri, "/api/events") == 0) {

        Event events[128];

        int count = get_events(conn, events);
        int out_size = count * 200 + 128;
        char* events_json = malloc(out_size);
        events_to_json(events, count, events_json, out_size);

        mg_send_http_ok(conn, "application/json", strlen(events_json));
        mg_write(conn, events_json, strlen(events_json));
        free(events_json);
        return 1;
    }
    // if ...

    return 0;
}
