#include "civetweb.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include "controllers/api_controller.h"
#include "controllers/html_controller.h"

static PGconn* db;

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

    mg_set_request_handler(ctx, "/home", home, NULL);
    mg_set_request_handler(ctx, "/events/**", events, NULL);
    mg_set_request_handler(ctx, "/events", events, NULL);
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
}
