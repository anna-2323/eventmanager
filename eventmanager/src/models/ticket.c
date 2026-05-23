#include "ticket.h"
#include <string.h>
#include <stdlib.h>

int purchase_ticket(PGconn* db, int event_id, const char* first_name,
    const char* last_name, const char* email, const char* phone, int* ticket_id_out) {
    // Използва се транзакция за отмяна на действието при възникнали грешки
    PQexec(db, "BEGIN");

    char event_id_str[16];
    snprintf(event_id_str, sizeof(event_id_str), "%d", event_id);
    const char* check_params[1] = { event_id_str };

    PGresult* res = PQexecParams(db,
        "SELECT seats_left "
        "FROM data.events "
        "WHERE id = $1 FOR UPDATE;",
        1, NULL, check_params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        PQexec(db, "ROLLBACK");
        return 0;
    }

    int seats_left = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    if (seats_left <= 0) {
        PQexec(db, "ROLLBACK");
        return -1;
    }

    const char* ins_params[5] = { event_id_str, first_name, last_name, email, phone };
    res = PQexecParams(db,
        "INSERT INTO data.tickets (event_id, first_name, last_name, email, phone) "
        "VALUES ($1, $2, $3, $4, $5) RETURNING id",
        5, NULL, ins_params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        PQexec(db, "ROLLBACK");
        return 0;
    }

    *ticket_id_out = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    // Намаляне на seats_left с 1
    res = PQexecParams(db,
        "UPDATE data.events SET seats_left = seats_left - 1 WHERE id = $1",
        1, NULL, check_params, NULL, NULL, 0);
    PQclear(res);

    PQexec(db, "COMMIT");
    return 1;
}

json_t* confirm_ticket(PGconn* db, int ticket_id) {
    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", ticket_id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecParams(db,
        "SELECT t.id, t.first_name, t.last_name, t.email, t.phone, t.purchased_at, e.title "
        "FROM data.tickets t "
        "JOIN data.events e ON t.event_id = e.id "
        "WHERE t.id = $1",
        1, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res); return NULL;
    }

    json_t* obj = json_object();
    json_object_set_new(obj, "id", json_integer(atoi(PQgetvalue(res, 0, 0))));
    json_object_set_new(obj, "first_name", json_string(PQgetvalue(res, 0, 1)));
    json_object_set_new(obj, "last_name", json_string(PQgetvalue(res, 0, 2)));
    json_object_set_new(obj, "email", json_string(PQgetvalue(res, 0, 3)));
    json_object_set_new(obj, "phone", json_string(PQgetvalue(res, 0, 4)));
    json_object_set_new(obj, "purchased_at", json_string(PQgetvalue(res, 0, 5)));
    json_object_set_new(obj, "event_title", json_string(PQgetvalue(res, 0, 6)));

    PQclear(res);
    return obj;
}