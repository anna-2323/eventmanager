#include "civetweb.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>

static PGconn* db;

typedef struct {
    int id;
    char name[256];
    double price;
    char begins_at[256];
    char venue[256];
    char img_path[256];
} Event;

// handler
static int home_handler(struct mg_connection*, void*);
static int get_events_handler(struct mg_connection*, void*);
static int request_handler(struct mg_connection*, void*);

// помощни функции
static void init_db(void);
static int get_events(struct mg_connection*, Event* events);
static void events_to_json(Event*, int, char*, size_t);
static int get_insert_pos(char*);

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
    mg_set_request_handler(ctx, "/api/", request_handler, NULL);

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

// форматиране на данни в json
static void events_to_json(Event* events, int count, char* out, size_t out_size) {

    // {
    //   [
    //      "id": 0,
    //      "name": sample,
    //      "preview_img": "event_image.png"
    //   ]
    // }, ...

    int pos = 0;
    pos += snprintf(out + pos, out_size - pos,
        "[");

    for (int i = 0; i < count; i++) {
        pos += snprintf(out + pos, out_size - pos,
            "{ \"id\": %d,"
            "\"name\": \"%s\","
            "\"preview_img\": \"%s\" }",
            events[i].id, events[i].name, events[i].img_path);
        if (i < count - 1) {
            pos += snprintf(out + pos, out_size - pos, ",");
        }
    }

    snprintf(out + pos, out_size - pos, "]");
}

// sql заявка за извличане на всички събития
static int get_events(struct mg_connection* conn, Event* events) {

    const struct mg_request_info* info = mg_get_request_info(conn);

    char sort[32] = { 0 };

    mg_get_var(info->query_string, strlen(info->query_string ? info->query_string : ""), "sort", sort, sizeof(sort));

    const char* query;
    if (strcmp(sort, "recent") == 0)
        query = "SELECT * FROM data.events ORDER BY begins_at ASC;";
    else
        query = "SELECT * FROM data.events;";

    PGresult* res = PQexec(db, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        return 0;
    }

    int count = PQntuples(res);
    for (int i = 0; i < count; i++) {
        events[i].id = atoi(PQgetvalue(res, i, 0));
        strncpy(events[i].name, PQgetvalue(res, i, 1), 100);
        strncpy(events[i].begins_at, PQgetvalue(res, i, 2), 255);
        strncpy(events[i].venue, PQgetvalue(res, i, 3), 100);
        strncpy(events[i].img_path, PQgetvalue(res, i, 4), 255);
        events[i].price = atof(PQgetvalue(res, i, 5));
    }

    PQclear(res);
    return count;
}
