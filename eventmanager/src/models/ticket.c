#include "ticket.h"
#include <string.h>
#include <stdlib.h>
#include "../util.h"
#include "../controllers/stats_controller.h"

static void ticket_from_query(PGresult* res, TicketView* t, int i) {
    snprintf(t->event_name, sizeof(t->event_name), "%s", PQgetvalue(res, i, 0));
    snprintf(t->begins_at, sizeof(t->begins_at), "%s", PQgetvalue(res, i, 1));
    snprintf(t->venue_name, sizeof(t->venue_name), "%s", PQgetvalue(res, i, 2));
    snprintf(t->venue_city, sizeof(t->venue_city), "%s", PQgetvalue(res, i, 3));
    snprintf(t->venue_address, sizeof(t->venue_address), "%s", PQgetvalue(res, i, 4));
    snprintf(t->first_name, sizeof(t->first_name), "%s", PQgetvalue(res, i, 5));
    snprintf(t->last_name, sizeof(t->last_name), "%s", PQgetvalue(res, i, 6));
    snprintf(t->email, sizeof(t->email), "%s", PQgetvalue(res, i, 7));
    snprintf(t->phone, sizeof(t->phone), "%s", PQgetvalue(res, i, 8));
    snprintf(t->sector, sizeof(t->sector), "%s", PQgetvalue(res, i, 9));
    if(!PQgetisnull(res, i, 10)) snprintf(t->token, sizeof(t->token), "%s", PQgetvalue(res, i, 10));
    if (!PQgetisnull(res, i, 11)) t->price = atof(PQgetvalue(res, i, 11));
}

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

int get_ticket(PGconn* db, int ticket_id, TicketView* out) {
    CHECK_DB(db, 0);
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", ticket_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecPrepared(db, "get_ticket", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    ticket_from_query(res, out, 0);

    PQclear(res);
    return 1;
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

    TicketView t;
    ticket_from_query(res, &t, 0);

    char* html = read_file_to_string("html/ticket_template.html");
    if (!html) { PQclear(res); return 1; }

    char qr_relative_path[128];
    snprintf(qr_relative_path, sizeof(qr_relative_path),
        "qr/%s.svg", t.token);

    char qr_html[256];
    snprintf(qr_html, sizeof(qr_html),
        "<img src=\"%s\" alt=\"QR Code\">",
        qr_relative_path);

    const char* fields[11][2] = {
        { "{{EVENT_TITLE}}", t.event_name },
        { "{{BEGINS_AT}}",   t.begins_at },
        { "{{VENUE_NAME}}",  t.venue_name },
        { "{{CITY}}",        t.venue_city },
        { "{{ADDRESS}}",     t.venue_address },
        { "{{FIRST_NAME}}",  t.first_name },
        { "{{LAST_NAME}}",   t.last_name },
        { "{{EMAIL}}",       t.email },
        { "{{PHONE}}",       t.phone },
        { "{{SECTOR_NAME}}", t.sector ? "" : t.sector },
        { "{{QR_CODE}}",     qr_html }
    };

    for (int i = 0; i < 11; i++) {
        char* replaced = replace_placeholder(html, fields[i][0], fields[i][1]);
        free(html);
        html = replaced;
        if (!html) { PQclear(res); return 1; }
    }

    PQclear(res);

    snprintf(out_path, 128, "html/tickets/ticket_%s.html", t.token);

    int result = write_string_to_file(out_path, html);
    free(html);
    return result;
}

int get_user_tickets(PGconn* db, int user_id, TicketView** out) {
    CHECK_DB(db, 0);
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", user_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecPrepared(db, "get_user_tickets", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    int count = PQntuples(res);
    *out = malloc(count * sizeof(TicketView));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }
    for (int i = 0; i < count; i++) {
        ticket_from_query(res, &(*out)[i], i);
    }

    PQclear(res);
    return count;
}

int ticket_belongs_to_user(PGconn* db, int user_id, int ticket_id) {
    CHECK_DB(db, 0);
    char user_id_str[16];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    char ticket_id_str[16];
    snprintf(ticket_id_str, sizeof(ticket_id_str), "%d", ticket_id);
    const char* params[2] = { ticket_id_str, user_id_str };

    PGresult* res = PQexecPrepared(db, "get_user_tickets", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    return 1;
}

int get_total_tickets(PGconn* db, int organizer_id) {
    CHECK_DB(db, 0);

    char id_str[16];
    const char* params[1];
    if (organizer_id > 0) {
        snprintf(id_str, sizeof(id_str), "%d", organizer_id);
        params[0] = id_str;
    }
    else {
        params[0] = NULL;
    }

    PGresult* res = PQexecPrepared(db, "get_total_tickets", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int total = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    return total;
}

int get_tickets_growth(PGconn* db, StatType type, int organizer_id, StatGrowth** out) {
    CHECK_DB(db, 0);

    const char* query_name;
    switch (type) {
    case STAT_MONTHLY:
        query_name = "get_tickets_growth_monthly";
        break;

    case STAT_MONTHLY_ALL:
        query_name = "get_tickets_growth_monthly_all";
        break;

    case STAT_DAILY:
        query_name = "get_tickets_growth_daily";
        break;

    default:
        return 0;
    }

    char id_str[16];
    const char* params[1];

    if (organizer_id > 0) {
        snprintf(id_str, sizeof(id_str), "%d", organizer_id);
        params[0] = id_str;
    }
    else {
        params[0] = NULL;
    }

    PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);
    *out = malloc(count * sizeof(StatGrowth));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        snprintf((*out)[i].period, sizeof((*out)[i].period), "%s", PQgetvalue(res, i, 0));
        (*out)[i].count = atoi(PQgetvalue(res, i, 1));
    }

    PQclear(res);
    return count;
}

int get_revenue(PGconn* db, StatType type, int organizer_id, StatRevenue** out) {
    CHECK_DB(db, 0);

    const char* query_name;
    switch (type) {
    case STAT_MONTHLY:
        query_name = "get_revenue_monthly";
        break;

    case STAT_MONTHLY_ALL:
        query_name = "get_revenue_monthly_all";
        break;

    case STAT_DAILY:
        query_name = "get_revenue_daily";
        break;

    case STAT_BY_VENUE:
        query_name = "get_revenue_by_venue_monthly";
        break;

    case STAT_BY_VENUE_ALL:
        query_name = "get_revenue_by_venue_monthly_all";
        break;

    default:
        return 0;
    }

    char id_str[16];
    const char* params[1];

    if (organizer_id > 0) {
        snprintf(id_str, sizeof(id_str), "%d", organizer_id);
        params[0] = id_str;
    }
    else {
        params[0] = NULL;
    }

    PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);
    *out = malloc(count * sizeof(StatRevenue));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        snprintf((*out)[i].period, sizeof((*out)[i].period), "%s", PQgetvalue(res, i, 0));

        (*out)[i].revenue = atof(PQgetvalue(res, i, 1));
        (*out)[i].count = atoi(PQgetvalue(res, i, 2));

        // за STAT_BY_VENUE или STAT_BY_VENUE_ALL
        if (!PQgetisnull(res, i, 3))
            (*out)[i].venue_id = atoi(PQgetvalue(res, i, 3));
        else
            (*out)[i].venue_id = 0;
    }

    PQclear(res);
    return count;
}
