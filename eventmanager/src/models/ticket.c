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
    PGresult* res = PQexecParams(db,
        "SELECT ls.capacity - COUNT(t.id) "
        "FROM data.layout_sectors ls "
        "LEFT JOIN data.tickets t ON t.event_id = $1 AND t.sector_id = ls.id "
        "WHERE ls.id = $2 "
        "GROUP BY ls.capacity; ",
        2, NULL, check_params, NULL, NULL, 0);
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
        res = PQexecParams(db,
            "INSERT INTO data.tickets (event_id, user_id, sector_id, first_name, last_name, email, phone) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id",
            7, NULL, ins_params, NULL, NULL, 0);
    }
    // Гост:
    else {
        const char* ins_params[6] = { event_id_str, sector_id_str, data->first_name, data->last_name, data->email, data->phone };
        res = PQexecParams(db,
            "INSERT INTO data.tickets (event_id, sector_id, first_name, last_name, email, phone) "
            "VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
            6, NULL, ins_params, NULL, NULL, 0);
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

    PGresult* res = PQexecParams(db,
        "SELECT e.title, e.begins_at, v.venue_name, v.city, v.address, "
        "t.first_name, t.last_name, t.email, t.phone, t.access_token, "
        "CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
        "FROM data.tickets t "
        "JOIN data.events e ON t.event_id = e.id "
        "JOIN data.venues v ON e.venue_id = v.id "
        "JOIN data.layouts l ON e.layout_id = l.id "
        "JOIN data.layout_sectors ls ON t.sector_id = ls.id "
        "JOIN data.sectors s ON ls.sector_id = s.id "
        "WHERE t.id = $1; ",
        1, NULL, params, NULL, NULL, 0);
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

int generate_ticket_html(PGconn* db, int ticket_id, const char* out_path) {
    const char* sql =
        "SELECT e.title, e.begins_at, v.venue_name, v.city, v.address, "
        "t.first_name, t.last_name, t.email, t.phone, t.access_token, "
        "CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
        "FROM data.tickets t "
        "JOIN data.events e ON t.event_id = e.id "
        "JOIN data.venues v ON e.venue_id = v.id "
        "JOIN data.layouts l ON e.layout_id = l.id "
        "JOIN data.layout_sectors ls ON t.sector_id = ls.id "
        "JOIN data.sectors s ON ls.sector_id = s.id "
        "WHERE t.id = $1;";

    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", ticket_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Ticket lookup failed: %s\n", PQerrorMessage(db));
        PQclear(res);
        return 1;
    }

    char* html = read_file_to_string("html/ticket_template.html");
    if (!html) { PQclear(res); return 1; }

    const char* fields[10][2] = {
        { "{{EVENT_TITLE}}", PQgetvalue(res, 0, 0) },
        { "{{BEGINS_AT}}",   PQgetvalue(res, 0, 1) },
        { "{{VENUE_NAME}}",  PQgetvalue(res, 0, 2) },
        { "{{CITY}}",        PQgetvalue(res, 0, 3) },
        { "{{ADDRESS}}",     PQgetvalue(res, 0, 4) },
        { "{{FIRST_NAME}}",  PQgetvalue(res, 0, 5) },
        { "{{LAST_NAME}}",   PQgetvalue(res, 0, 6) },
        { "{{EMAIL}}",       PQgetvalue(res, 0, 7) },
        { "{{PHONE}}",       PQgetvalue(res, 0, 8) },
        { "{{SECTOR_NAME}}", PQgetisnull(res, 0, 10) ? "" : PQgetvalue(res, 0, 10) }
    };

    char token[37];
    snprintf(token, sizeof(token), PQgetvalue(res, 0, 9));

    for (int i = 0; i < 10; i++) {
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