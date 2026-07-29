#include "civetweb.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "controllers/api_controller.h"
#include "controllers/html_controller.h"
#include "session.h"

static PGconn* db;

// помощни функции
static void init_db(void);
void set_handlers(struct mg_context*);

int main(void) {
    init_db();

    session_init();

    const char* options[] = {
        "listening_ports", "8080",
        "document_root", ".\\html",
        "num_threads", "1",
        NULL
    };

    struct mg_callbacks callbacks = { 0 };
    struct mg_context* ctx = mg_start(&callbacks, NULL, options);
    mg_set_request_handler(ctx, "/res/**", NULL, NULL);  // статични ресурси
    set_handlers(ctx);

    permanent_delete_users(db);
    delete_tokens(db);

    printf("Server running on port 8080\n");
    getchar();


    mg_stop(ctx);
    return 0;
}

static void init_db(void) {
    db = PQconnectdb("host=localhost dbname=eventmanagement user=postgres password=secret");
}

void set_handlers(struct mg_context* ctx) {
    mg_set_request_handler(ctx, "/home", html_controller, NULL);
    mg_set_request_handler(ctx, "/events/**", html_controller, NULL);
    mg_set_request_handler(ctx, "/events", html_controller, NULL);
    mg_set_request_handler(ctx, "/purchase/**", html_controller, NULL);
    mg_set_request_handler(ctx, "/confirmation/**", html_controller, NULL);
    mg_set_request_handler(ctx, "/login", html_controller, NULL);
    mg_set_request_handler(ctx, "/signup", html_controller, NULL);
    mg_set_request_handler(ctx, "/profile", html_controller, NULL);
    mg_set_request_handler(ctx, "/forgot", html_controller, NULL);
    mg_set_request_handler(ctx, "/reset", html_controller, NULL);
    mg_set_request_handler(ctx, "/admin", html_controller, NULL);
    mg_set_request_handler(ctx, "/admin/**", html_controller, NULL);

    mg_set_request_handler(ctx, "/api/events/layout", api_event_layout, db);
    mg_set_request_handler(ctx, "/api/events/**", api_events, db);
    mg_set_request_handler(ctx, "/api/events", api_events, db);
    mg_set_request_handler(ctx, "/api/users", api_users, db);
    mg_set_request_handler(ctx, "/api/purchase/**", api_purchase_ticket, db);
    mg_set_request_handler(ctx, "/api/confirmation/**", api_confirm_ticket, db);
    mg_set_request_handler(ctx, "/api/me", api_me, db);
    mg_set_request_handler(ctx, "/api/signup", api_signup, db);
    mg_set_request_handler(ctx, "/api/login", api_login, db);
    mg_set_request_handler(ctx, "/api/logout", api_logout, db);
    mg_set_request_handler(ctx, "/api/forgot", api_forgot, db);
    mg_set_request_handler(ctx, "/api/reset", api_reset_password, db);
    mg_set_request_handler(ctx, "/api/profile/**", api_profile, db);
}