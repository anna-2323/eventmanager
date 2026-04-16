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

// помощни функции
static void init_db(void);
static int get_events(struct mg_connection*, Event* events);
static void events_to_html(Event*, int, char*, size_t);
static int get_insert_pos(char*);

int main(void) {
    init_db();

    FILE* f = fopen("html/template.html", "rb");
    if (!f) {
        fprintf(stderr, "Empty file error\n");
        return;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* template = malloc(fsize + 1);
    fread(template, 1, fsize, f);
    template[fsize] = '\0';
    fclose(f);

    const char* options[] = {
        "listening_ports", "8080",
        "document_root", ".\\html",
        NULL
    };

    struct mg_callbacks callbacks = { 0 };
    struct mg_context* ctx = mg_start(&callbacks, NULL, options);

    mg_set_request_handler(ctx, "/home", home_handler, template);
    mg_set_request_handler(ctx, "/events", get_events_handler, template);

    printf("Server running on port 8080\n");
    getchar();

    mg_stop(ctx);
    free(template);
    return 0;
}

static void init_db(void) {
    db = PQconnectdb("host=localhost dbname=eventmanagement user=postgres password=secret");
    //...
}

// /home, /home?sort=recent
static int home_handler(struct mg_connection* conn, void* cbdata) {

    char* tmpl = (char*)cbdata;

    size_t tmpl_len = strlen(tmpl);
    size_t insert_pos = get_insert_pos(tmpl);

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
    mg_write(conn, tmpl, insert_pos);
    mg_write(conn, home, strlen(home));
    mg_write(conn, tmpl + insert_pos, tmpl_len - insert_pos);

    free(home);
    return 1;
}

// /events
static int get_events_handler(struct mg_connection* conn, void* cbdata) {

    char* tmpl = (char*)cbdata;

    Event events[128];

    int count = get_events(conn, events);

    int out_size = count * 200 + 128;
    char* events_html = malloc(out_size);
    events_to_html(events, count, events_html, out_size);

    int tmpl_len = strlen(tmpl);
    int insert_pos = get_insert_pos(tmpl);

    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    mg_write(conn, tmpl, insert_pos);
    mg_write(conn, events_html, strlen(events_html));
    mg_write(conn, tmpl + insert_pos, tmpl_len - insert_pos);

    free(events_html);
    return 1;
}

// получаване позиция за вмъкване на съдържание
static int get_insert_pos(char* tmpl) {
    size_t tmpl_len = strlen(tmpl);
    size_t insert_pos = 0;
    for (size_t i = 0; i < tmpl_len; i++) {
        if (tmpl[i] == '<' && strncmp(&tmpl[i], "<main class=\"container\">", 24) == 0) {
            insert_pos = i + 24;
            break;
        }
    }
    return insert_pos;
}

// форматиране на данни в html
static void events_to_html(Event* events, int count, char* out, size_t out_size) {
    int pos = 0;
    pos += snprintf(out + pos, out_size - pos,
        "<div class=\"grid is-col-min-5 is-column-gap-3 is-row-gap-3\">");

    for (int i = 0; i < count; i++) {
        pos += snprintf(out + pos, out_size - pos,
            "<div class=\"cell\">"
            "<img src=\"%s\"/>"
            "<p>%s</p>"
            "</div>",
            events[i].img_path, events[i].name);
    }

    snprintf(out + pos, out_size - pos, "</div>");
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
