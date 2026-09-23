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
static int init_db(void);
void set_handlers(struct mg_context*, TicketContext* ticket_context);
int init_db_structure(const char* target_db);
int apply_schema(const char* target_db);

int main(void) {
    if (!load_config(&config)) {
        fprintf(stderr, "Грешка при зареждане на конфигурацията.\n");
        return 1;
    }

    if (!init_db()) {
        return 1;
    }

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

static int init_db(void) {
    if (!init_db_structure(config.db_name)) {
        return 0;
    }
    char conn_str[256];
    snprintf(conn_str, sizeof(conn_str), "host=%s port=%s dbname=%s user=%s password=%s client_encoding=UTF8",
        config.db_host, config.db_port, 
        config.db_name, config.db_user, 
        config.db_password);
    db = PQconnectdb(conn_str);

    if (PQstatus(db) != CONNECTION_OK) {
        fprintf(stderr, "Няма връзка с БД: %s\n",
            PQerrorMessage(db));
        return 0;
    }

    if (!apply_schema(db)) {
        PQfinish(db);
        db = NULL;
        return 0;
    }

    if (!prepare_queries(db)) {
        PQfinish(db);
        db = NULL;
        return 0;
    }

    return 1;
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

int init_db_structure(const char* target_db) {
    // Връзка с БД по подразбиране
    PGconn* conn = PQconnectdb("host=localhost port=5432 dbname=postgres user=postgres password=secret");

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Грешка при свързване с Postgres: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 0;
    }

    // Съществува ли необходимата БД
    const char* check_sql = "SELECT 1 FROM pg_database WHERE datname = $1;";
    const char* params[1] = { target_db };
    PGresult* res = PQexecParams(conn, check_sql, 1, NULL, params, NULL, NULL, 0);

    int db_exists = (PQntuples(res) > 0);
    PQclear(res);

    // Създава се ако я няма
    if (!db_exists) {
        char create_sql[256];
        // CREATE DATABASE не приема параметри
        snprintf(create_sql, sizeof(create_sql), "CREATE DATABASE \"%s\";", target_db);

        res = PQexec(conn, create_sql);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Грешка при CREATE DATABASE: %s", PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            return 0;
        }
        PQclear(res);
        printf("База данни '%s' създадена успешно.\n", target_db);
    }


    PQfinish(conn);
    return 1;
}

int apply_schema(PGconn* conn) {
    const char* init_script = "SET client_min_messages = warning; " 
        "CREATE SCHEMA IF NOT EXISTS data; "
        "CREATE TABLE IF NOT EXISTS data.categories ( "
        "    id serial primary key, "
        "    title varchar(255) NOT NULL "
        "); "
        "CREATE TABLE IF NOT EXISTS data.users ( "
        "    id serial primary key, "
        "    role integer DEFAULT 2, "
        "    email varchar(255) NOT NULL UNIQUE, "
        "    password_hash bytea NOT NULL, "
        "    first_name varchar(255), "
        "    last_name varchar(255), "
        "    phone varchar(255), "
        "    deleted_on timestamp with time zone, "
        "    salt bytea NOT NULL, "
        "    active boolean DEFAULT true, "
        "    created_at timestamp with time zone DEFAULT now() "
        "); "
        "CREATE TABLE IF NOT EXISTS data.venues ( "
        "    id serial primary key, "
        "    city varchar(255) NOT NULL, "
        "    address varchar(255) NOT NULL, "
        "    venue_name varchar(255) NOT NULL, "
        "    active boolean DEFAULT true, "
        "    background_svg text, "
        "    has_sectors boolean DEFAULT false, "
        "    viewbox varchar(50) "
        "); "
        "CREATE TABLE IF NOT EXISTS data.events ( "
        "    id serial primary key, "
        "    title varchar(255) NOT NULL, "
        "    begins_at timestamp with time zone NOT NULL, "
        "    venue_id integer NOT NULL REFERENCES data.venues(id), "
        "    img_path varchar(225) DEFAULT '/res/default1.png'::character varying, "
        "    organizer_id integer NOT NULL REFERENCES data.users(id), "
        "    uploaded_at timestamp with time zone DEFAULT now(), "
        "    active boolean DEFAULT false, "
        "    category_id integer REFERENCES data.categories(id), "
        "    description varchar(512) "
        "); "
        "CREATE TABLE IF NOT EXISTS data.sectors ( "
        "    id serial primary key, "
        "    venue_id integer NOT NULL REFERENCES data.venues(id), "
        "    name varchar(100), "
        "    display_order integer DEFAULT 1, "
        "    svg_path text, "
        "    color varchar(7), "
        "    capacity integer "
        "); "
        "CREATE TABLE IF NOT EXISTS data.event_sectors ( "
        "    event_id integer NOT NULL REFERENCES data.events(id), "
        "    sector_id integer NOT NULL REFERENCES data.sectors(id), "
        "    price numeric(10,2) NOT NULL, "
        "    CONSTRAINT event_sectors_price_check CHECK ((price >= (0)::numeric)), "
        "    PRIMARY KEY (event_id, sector_id) "
        "); "
        "CREATE TABLE  IF NOT EXISTS data.password_resets ( "
        "    token varchar(128) primary key, "
        "    user_id integer REFERENCES data.users(id), "
        "    expires_at timestamp with time zone DEFAULT (now() + '01:00:00'::interval) "
        "); "
        "CREATE TABLE IF NOT EXISTS data.tickets ( "
        "    id serial primary key, "
        "    event_id integer REFERENCES data.events(id), "
        "    user_id integer REFERENCES data.users(id), "
        "    sector_id integer REFERENCES data.sectors(id), "
        "    first_name varchar(255) NOT NULL, "
        "    last_name varchar(255) NOT NULL, "
        "    email varchar(255) NOT NULL, "
        "    phone varchar(50) NOT NULL, "
        "    price numeric(10,2), "
        "    purchased_at timestamp with time zone DEFAULT now(), "
        "    access_token uuid DEFAULT gen_random_uuid() NOT NULL, "
        "    active boolean DEFAULT true "
        "); "
        "INSERT INTO data.categories (id, title) VALUES "
        "   (1, 'Концерти'), (2, 'Семинари'), (3, 'Спорт'), (4, 'Театър')"
        "   ON CONFLICT(id) DO NOTHING; "
        "INSERT INTO data.users (role, email, password_hash, first_name, last_name, phone, salt, active) VALUES (0, 'admin', "
        "   '\\x930365301949658c92db875392d0a9b1b30f1889c6a4fbab1b5e3516e81ecceb', NULL, NULL, NULL, '\\x16ea4ae8c868cab33ddcadc7233a7e69', true) "
        "   ON CONFLICT(email) DO NOTHING; ";

    // Инициализиране
    PGresult* res = PQexec(conn, init_script);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Грешка при инициализация: %s", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return 0;
    }

    PQclear(res);

    return 1;
}