#include "ticket.h"
#include <string.h>
#include <stdlib.h>
#include "../util.h"

// Открива първия {{шалбон}} и го заменя с дадената стойност
char* replace_placeholder(const char* src, const char* placeholder, const char* value) {
    const char* pos = strstr(src, placeholder);
    if (!pos) return _strdup(src);

    size_t prefix_len = pos - src;
    size_t suffix_len = strlen(pos + strlen(placeholder));
    size_t value_len = strlen(value);

    char* result = malloc(prefix_len + value_len + suffix_len + 1);
    if (!result) return NULL;

    memcpy(result, src, prefix_len);
    memcpy(result + prefix_len, value, value_len);
    memcpy(result + prefix_len + value_len, pos + strlen(placeholder), suffix_len + 1); // +1 copies the null terminator

    return result;
}

int purchase_ticket(PGconn* db, TicketData* data, int* ticket_id_out) {
    // Използва се транзакция за отмяна на действието при възникнали грешки
    PQexec(db, "BEGIN");

    char event_id_str[16];
    char sector_id_str[16];
    snprintf(event_id_str, sizeof(event_id_str), "%d", data->event_id);
    snprintf(sector_id_str, sizeof(sector_id_str), "%d", data->sector_id);
    const char* check_params[2] = { event_id_str, sector_id_str };

    // Проверка дали има места за това събитие и за тази категория места
    PGresult* res = PQexecPrepared(db, "check_seat", 2, check_params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        fprintf(stderr, "Грешка: %s\n", PQerrorMessage(db));
        PQexec(db, "ROLLBACK");
        return 0;
    }

    int seats_left = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    if (seats_left <= 0) {
        PQexec(db, "ROLLBACK");
        return -1;
    }
    
    // Ако има места, създава се нов билет:
    // Регистриран потребител:
    if (data->user_id > 0) {
        char user_id_str[16];
        snprintf(user_id_str, sizeof(user_id_str), "%d", data->user_id);
        const char* ins_params[7] = { event_id_str, user_id_str, sector_id_str, data->first_name, data->last_name, data->email, data->phone };

        PGresult* res = PQexecPrepared(db, "add_ticket_user", 7, ins_params, NULL, NULL, 0);
    }
    // Гост:
    else {
        const char* ins_params[6] = { event_id_str, sector_id_str, data->first_name, data->last_name, data->email, data->phone };

        PGresult* res = PQexecPrepared(db, "add_ticket_guest", 6, ins_params, NULL, NULL, 0);
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        fprintf(stderr, "Грешка: %s\n", PQerrorMessage(db));
        PQexec(db, "ROLLBACK");
        return 0;
    }
    
    *ticket_id_out = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    PQexec(db, "COMMIT");
    return 1;
}

json_t* get_ticket(PGconn* db, int ticket_id) {
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", ticket_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecPrepared(db, "get_ticket", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, NULL);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return NULL;
    }

    json_t* ticket = json_object();
    json_object_set_new(ticket, "id", json_integer(ticket_id));
    json_object_set_new(ticket, "event_name", json_string(PQgetvalue(res, 0, 0)));
    json_object_set_new(ticket, "begins_at", json_string(PQgetvalue(res, 0, 1)));
    json_object_set_new(ticket, "venue_name", json_string(PQgetvalue(res, 0, 2)));
    json_object_set_new(ticket, "venue_city", json_string(PQgetvalue(res, 0, 3)));
    json_object_set_new(ticket, "venue_address", json_string(PQgetvalue(res, 0, 4)));
    json_object_set_new(ticket, "first_name", json_string(PQgetvalue(res, 0, 5)));
    json_object_set_new(ticket, "last_name", json_string(PQgetvalue(res, 0, 6)));
    json_object_set_new(ticket, "email", json_string(PQgetvalue(res, 0, 7)));
    json_object_set_new(ticket, "phone", json_string(PQgetvalue(res, 0, 8)));
    json_object_set_new(ticket, "token", json_string(PQgetvalue(res, 0, 9)));
    json_object_set_new(ticket, "sector", json_string(PQgetvalue(res, 0, 10)));

    PQclear(res);
    return ticket;
}

int generate_ticket_html(PGconn* db, int ticket_id, const char* qr_path, 
        char* out_path, size_t out_size) {
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", ticket_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecPrepared(db, "get_ticket_for_html", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Ticket lookup failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return 1;
    }

    char* html = read_file_to_string("html/ticket_template.html");
    if (!html) { PQclear(res); return 1; }

    const char* token = PQgetvalue(res, 0, 9);

    char qr_relative_path[128];
    snprintf(qr_relative_path, sizeof(qr_relative_path),
        "qr/%s.svg", token);

    char qr_html[256];
    snprintf(qr_html, sizeof(qr_html),
        "<img src=\"%s\" alt=\"QR Code\">",
        qr_relative_path);

    const char* fields[11][2] = {
        { "{{EVENT_TITLE}}", PQgetvalue(res, 0, 0) },
        { "{{BEGINS_AT}}",   PQgetvalue(res, 0, 1) },
        { "{{VENUE_NAME}}",  PQgetvalue(res, 0, 2) },
        { "{{CITY}}",        PQgetvalue(res, 0, 3) },
        { "{{ADDRESS}}",     PQgetvalue(res, 0, 4) },
        { "{{FIRST_NAME}}",  PQgetvalue(res, 0, 5) },
        { "{{LAST_NAME}}",   PQgetvalue(res, 0, 6) },
        { "{{EMAIL}}",       PQgetvalue(res, 0, 7) },
        { "{{PHONE}}",       PQgetvalue(res, 0, 8) },
        { "{{SECTOR_NAME}}", PQgetisnull(res, 0, 10) ? "" : PQgetvalue(res, 0, 10) },
        { "{{QR_CODE}}",     qr_html }
    };

    for (int i = 0; i < 11; i++) {
        char* replaced = replace_placeholder(html, fields[i][0], fields[i][1]);
        free(html);
        html = replaced;
        if (!html) { PQclear(res); return 1; }
    }

    PQclear(res);

    snprintf(out_path, 128, "tickets/ticket_%s.html", token);

    int result = write_string_to_file(out_path, html);
    free(html);
    return result;
}

json_t* get_total_tickets(PGconn* db) {
    CHECK_DB(db, NULL);

    PGresult* res = PQexecPrepared(db, "get_total_tickets", 0, NULL, NULL, NULL, 0);
    CHECK_QUERY(res, db, NULL);

    int total = atoi(PQgetvalue(res, 0, 0));

    return json_integer(total);
}

static json_t* get_tickets_growth_json(PGresult* res, int type) {
    json_t* growth = json_array();
    int count = PQntuples(res);

    for (int i = 0; i < count; i++) {
        const char* period = PQgetvalue(res, i, 0);
        int ticket_count = atoi(PQgetvalue(res, i, 1));

        json_t* entry = json_object();

        if (type == 0 || type == 1)
            json_object_set_new(
                entry,
                "month",
                json_string(period)
            );
        else if (type == 2)
            json_object_set_new(
                entry,
                "day",
                json_string(period)
            );

        json_object_set_new(
            entry,
            "ticket_count",
            json_integer(ticket_count)
        );

        json_array_append_new(growth, entry);
    }

    PQclear(res);
    return growth;
}

json_t* get_tickets_growth(PGconn* db, int type) {
    CHECK_DB(db, NULL);

    PGresult* res;
    if (type == 0) {
        res = PQexecPrepared(db, "get_tickets_growth_monthly", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_tickets_growth_json(res, type);
    }
    else if (type == 1) {
        res = PQexecPrepared(db, "get_tickets_growth_monthly_all", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_tickets_growth_json(res, type);
    }
    else if (type == 2) {
        res = PQexecPrepared(db, "get_tickets_growth_daily", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_tickets_growth_json(res, type);
    }
}

static json_t* get_revenue_json(PGresult* res, int type) {
    json_t* json = json_array();
    int count = PQntuples(res);

    for (int i = 0; i < count; i++) {
        const char* period = PQgetvalue(res, i, 0);
        float revenue = atof(PQgetvalue(res, i, 1));
        int tickets_sold = atoi(PQgetvalue(res, i, 2));

        json_t* entry = json_object();

        if (type != 2)
            json_object_set_new(
                entry,
                "month",
                json_string(period)
            );
        else if (type == 2)
            json_object_set_new(
                entry,
                "day",
                json_string(period)
            );

        json_object_set_new(
            entry,
            "revenue",
            json_real(revenue)
        );
        json_object_set_new(
            entry,
            "tickets_sold",
            json_integer(tickets_sold)
        );

        json_array_append_new(json, entry);
    }

    PQclear(res);
    return json;
}

static json_t* get_revenue_by_venue_json(PGresult* res) {
    json_t* json = json_array();
    int count = PQntuples(res);

    for (int i = 0; i < count; i++) {
        const char* period = PQgetvalue(res, i, 0);
        float revenue = atof(PQgetvalue(res, i, 1));
        int tickets_sold = atoi(PQgetvalue(res, i, 2));
        int venue_id = atoi(PQgetvalue(res, i, 3));

        json_t* entry = json_object();

        json_object_set_new(
          entry,
          "day",
          json_string(period)
        );

        json_object_set_new(
            entry,
            "venue",
            json_integer(venue_id)
        );
        json_object_set_new(
            entry,
            "revenue",
            json_real(revenue)
        );
        json_object_set_new(
            entry,
            "tickets_sold",
            json_integer(tickets_sold)
        );

        json_array_append_new(json, entry);
    }

    PQclear(res);
    return json;
}

json_t* get_revenue(PGconn* db, int type) {
    CHECK_DB(db, NULL);

    PGresult* res;
    if (type == 0) {
        res = PQexecPrepared(db, "get_revenue_monthly", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_revenue_json(res, type);
    }
    else if (type == 1) {
        res = PQexecPrepared(db, "get_revenue_monthly_all", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_revenue_json(res, type);
    }
    else if (type == 2) {
        res = PQexecPrepared(db, "get_revenue_daily", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_revenue_json(res, type);
    }
    else if (type == 3) {
        res = PQexecPrepared(db, "get_revenue_by_venue_monthly", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_revenue_by_venue_json(res);
    }
    else if (type == 4) {
        res = PQexecPrepared(db, "get_revenue_by_venue_monthly_all", 0, NULL, NULL, NULL, 0);
        CHECK_QUERY(res, db, NULL);
        return get_revenue_by_venue_json(res);
    }
}
