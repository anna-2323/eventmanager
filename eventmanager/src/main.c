#include "civetweb.h"
#include <libpq-fe.h>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "controllers/event_api_controller.h"
#include "controllers/user_api_controller.h"
#include "controllers/ticket_api_controller.h"
#include "controllers/venue_api_controller.h"
#include "controllers/html_controller.h"
#include "controllers/stats_controller.h"
#include "session.h"
#include "db/queries.h"

static PGconn* db;
static Config config;

// помощни функции
static void init_db(void);
void set_handlers(struct mg_context*, TicketContext* ticket_context);

int main(void) {
    if (!load_config(&config)) {
        fprintf(stderr, "Грешка при зареждане на конфигурацията.\n");
        return 1;
    }

    init_db();

    session_init();

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", config.server_port);
    const char* options[] = {
        "listening_ports", port_str,
        "document_root", ".\\html",
        "num_threads", "1",
        NULL
    };

    struct mg_callbacks callbacks = { 0 };
    struct mg_context* ctx = mg_start(&callbacks, NULL, options);

    TicketContext ticket_context = {
        .db = db,
        .config = &config
    };

    mg_set_request_handler(ctx, "/res/**", NULL, NULL);  // статични ресурси
    set_handlers(ctx, &ticket_context);

    permanent_delete_users(db);
    delete_tokens(db);

    printf("Сървърът работи на порт %s\n", port_str);
    getchar();


    mg_stop(ctx);
    return 0;
}

static void init_db(void) {
    char conn_str[256];
    snprintf(conn_str, sizeof(conn_str), "host=%s port=%s dbname=%s user=%s password=%s",
        config.db_host, config.db_port, 
        config.db_name, config.db_user, 
        config.db_password);
    db = PQconnectdb(conn_str);

    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Няма връзка с БД: %s\n",
            PQerrorMessage(db));
        return 0;
    }

    if (!prepare_queries(db)) {
        return 0;
    }
}

void set_handlers(struct mg_context* ctx, TicketContext* ticket_context) {
    mg_set_request_handler(ctx, "/home", html_controller, &config);
    mg_set_request_handler(ctx, "/events/**", html_controller, &config);
    mg_set_request_handler(ctx, "/events", html_controller, &config);
    mg_set_request_handler(ctx, "/purchase/**", html_controller, &config);
    mg_set_request_handler(ctx, "/confirmation/**", html_controller, &config);
    mg_set_request_handler(ctx, "/login", html_controller, &config);
    mg_set_request_handler(ctx, "/signup", html_controller, &config);
    mg_set_request_handler(ctx, "/profile", html_controller, &config);
    mg_set_request_handler(ctx, "/forgot", html_controller, &config);
    mg_set_request_handler(ctx, "/reset", html_controller, &config);
    mg_set_request_handler(ctx, "/admin", html_controller, &config);
    mg_set_request_handler(ctx, "/admin/**", html_controller, &config);
    mg_set_request_handler(ctx, "/organizer", html_controller, &config);

    mg_set_request_handler(ctx, "/api/events/seatmap", api_event_seatmap, db);
    mg_set_request_handler(ctx, "/api/events/**", api_events, db);
    mg_set_request_handler(ctx, "/api/users/*/events", api_user_events, db);
    mg_set_request_handler(ctx, "/api/events", api_events, db);

    mg_set_request_handler(ctx, "/api/venues/seatmap", api_venue_seatmap, db);
    mg_set_request_handler(ctx, "/api/categories", api_categories, db);
    mg_set_request_handler(ctx, "/api/cities", api_cities, db);
    mg_set_request_handler(ctx, "/api/venues", api_venues, db);

    mg_set_request_handler(ctx, "/api/admin/users", api_users, db);
    mg_set_request_handler(ctx, "/api/admin/events", api_admin_events, db);
    mg_set_request_handler(ctx, "/api/admin/venues", api_admin_venues, db);
    mg_set_request_handler(ctx, "/api/admin/tickets", api_admin_tickets, db);

    // mg_set_request_handler(ctx, "/api/stats/export", api_admin_stats_export, db);
    // mg_set_request_handler(ctx, "/api/stats/export/**", api_admin_stats_export, db);
    mg_set_request_handler(ctx, "/api/stats", api_stats, db);;
    mg_set_request_handler(ctx, "/api/stats/**", api_stats, db);

    mg_set_request_handler(ctx, "/api/purchase/**", api_purchase_ticket, db);
    mg_set_request_handler(ctx, "/api/confirmation/**", api_confirm_ticket, ticket_context);
    mg_set_request_handler(ctx, "/api/users/*/tickets", api_user_tickets, db);
    mg_set_request_handler(ctx, "/api/tickets", api_tickets, db);
    mg_set_request_handler(ctx, "/tickets/**", api_ticket_file, db);

    mg_set_request_handler(ctx, "/api/me", api_me, db);
    mg_set_request_handler(ctx, "/api/signup", api_signup, db);
    mg_set_request_handler(ctx, "/api/login", api_login, db);
    mg_set_request_handler(ctx, "/api/logout", api_logout, db);
    mg_set_request_handler(ctx, "/api/forgot", api_forgot, db);
    mg_set_request_handler(ctx, "/api/reset", api_reset_password, db);
    mg_set_request_handler(ctx, "/api/profile", api_profile, db);

}