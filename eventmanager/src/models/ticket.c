#include "ticket.h"
#include <string.h>
#include <stdlib.h>
#include "../util.h"
#include "../controllers/stats_controller.h"

static void ticket_from_query(PGresult* res, TicketView* t, int i) {
    t->id = atoi(PQgetvalue(res, i, 0));
    snprintf(t->event_name, sizeof(t->event_name), "%s", PQgetvalue(res, i, 1));
    snprintf(t->begins_at, sizeof(t->begins_at), "%s", PQgetvalue(res, i, 2));
    snprintf(t->venue_name, sizeof(t->venue_name), "%s", PQgetvalue(res, i, 3));
    snprintf(t->venue_city, sizeof(t->venue_city), "%s", PQgetvalue(res, i, 4));
    snprintf(t->venue_address, sizeof(t->venue_address), "%s", PQgetvalue(res, i, 5));
    snprintf(t->first_name, sizeof(t->first_name), "%s", PQgetvalue(res, i, 6));
    snprintf(t->last_name, sizeof(t->last_name), "%s", PQgetvalue(res, i, 7));
    snprintf(t->email, sizeof(t->email), "%s", PQgetvalue(res, i, 8));
    snprintf(t->phone, sizeof(t->phone), "%s", PQgetvalue(res, i, 9));
    t->price = atof(PQgetvalue(res, i, 10));
    if(!PQgetisnull(res, i, 11)) 
        snprintf(t->token, sizeof(t->token), "%s", PQgetvalue(res, i, 11));
    if (!PQgetisnull(res, i, 12)) 
        t->event_id = atoi(PQgetvalue(res, i, 12));
    if (!PQgetisnull(res, i, 13))
        t->user_id = atoi(PQgetvalue(res, i, 13));
    if (!PQgetisnull(res, i, 14))
        t->active = (strcmp(PQgetvalue(res, i, 14), "t") == 0);
    if (!PQgetisnull(res, i, 15))
        snprintf(t->sector, sizeof(t->sector), "%s", PQgetvalue(res, i, 15));
}

static double get_sector_price(PGconn* db, const char* sector_id_str) {
    const char* params[1] = { sector_id_str };

    PGresult* res = PQexecPrepared(db, "get_sector_price", 1, params, NULL, NULL, 0);

    double price = 0.0;
    if (PQntuples(res) > 0) {
        price = strtod(PQgetvalue(res, 0, 0), NULL);
    }

    PQclear(res);
    return price;
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
    
    data->price = get_sector_price(db, sector_id_str);
    if (!data->price) {
        PQexec(db, "ROLLBACK");
        return -1;
    }
    char price_str[16];
    snprintf(price_str, sizeof(price_str), "%f", data->price);

    // Ако има места, създава се нов билет:
    // Регистриран потребител:
    if (data->user_id > 0) {
        char user_id_str[16];
        snprintf(user_id_str, sizeof(user_id_str), "%d", data->user_id);
        const char* ins_params[8] = { event_id_str, user_id_str, sector_id_str, data->first_name, data->last_name, data->email, data->phone, price_str };

        res = PQexecPrepared(db, "add_ticket_user", 8, ins_params, NULL, NULL, 0);
    }
    // Гост:
    else {
        const char* ins_params[7] = { event_id_str, sector_id_str, data->first_name, data->last_name, data->email, data->phone, price_str };

        res = PQexecPrepared(db, "add_ticket_guest", 7, ins_params, NULL, NULL, 0);
    }

    *ticket_id_out = atoi(PQgetvalue(res, 0, 0));

    PQclear(res);

    PQexec(db, "COMMIT");
    return 1;
}

int get_tickets(PGconn* db, int organizer_id, TicketView** out) {
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

    PGresult* res = PQexecPrepared(db, "get_tickets", 1, params, NULL, NULL, 0);
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

int ticket_id_belongs_to_user(PGconn* db, int user_id, int ticket_id) {
    CHECK_DB(db, 0);
    char user_id_str[16];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    char ticket_id_str[16];
    snprintf(ticket_id_str, sizeof(ticket_id_str), "%d", ticket_id);
    const char* params[2] = { ticket_id_str, user_id_str };

    PGresult* res = PQexecPrepared(db, "ticket_belongs_to_user_id", 2, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    return 1;
}

int ticket_uuid_belongs_to_user(PGconn* db, int user_id, const char* ticket_uuid) {
    CHECK_DB(db, 0);
    char user_id_str[16];
    snprintf(user_id_str, sizeof(user_id_str), "%d", user_id);
    const char* params[2] = { ticket_uuid, user_id_str };

    PGresult* res = PQexecPrepared(db, "ticket_belongs_to_user_uuid", 2, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    if (PQntuples(res) == 0) {
        PQclear(res);
        return 0;
    }

    return 1;
}

int set_ticket_active(PGconn* db, int id, int active) {
    CHECK_DB(db, 0);

    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", id);
    const char* params[1] = { id_str };

    const char* query_name = active
        ? "activate_ticket"
        : "deactivate_ticket";

    PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
    CHECK_COMMAND_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int get_total_tickets(PGconn* db, int organizer_id) {
    CHECK_DB(db, -1);

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
    CHECK_QUERY(res, db, -1);

    int total = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    return total;
}

int get_tickets_growth(PGconn* db, StatType type, int organizer_id, int months, StatGrowth** out) {
    CHECK_DB(db, -1);

    const char* query_name;
    switch (type) {
    case STAT_MONTHLY:
        query_name = "get_tickets_growth_monthly";
        break;

    case STAT_DAILY:
        query_name = "get_tickets_growth_daily";
        break;

    default:
        return -1;
    }

    char id_str[16];
    char months_str[16];
    const char* params[2];

    if (organizer_id > 0) {
        snprintf(id_str, sizeof(id_str), "%d", organizer_id);
        params[0] = id_str;
    }
    else
        params[0] = NULL;

    if (months > 0) {
        snprintf(months_str, sizeof(months_str), "%d", months);
        params[1] = months_str;
    }
    else
        params[1] = NULL;

    PGresult* res = PQexecPrepared(db, query_name, 2, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);
    *out = malloc(count * sizeof(StatGrowth));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        snprintf((*out)[i].period, sizeof((*out)[i].period), "%s", PQgetvalue(res, i, 0));
        (*out)[i].count = atoi(PQgetvalue(res, i, 1));
    }

    PQclear(res);
    return count;
}

int get_revenue(PGconn* db, StatType type, int organizer_id, int months, StatRevenue** out) {
    CHECK_DB(db, -1);

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
        return -1;
    }

    char id_str[16];
    char months_str[16];
    const char* params[2];

    if (organizer_id > 0) {
        snprintf(id_str, sizeof(id_str), "%d", organizer_id);
        params[0] = id_str;
    }
    else
        params[0] = NULL;

    if (months > 0) {
        snprintf(months_str, sizeof(months_str), "%d", months);
        params[1] = months_str;
    }
    else
        params[1] = NULL;

    PGresult* res = PQexecPrepared(db, query_name, 2, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);
    *out = malloc(count * sizeof(StatRevenue));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        snprintf((*out)[i].period, sizeof((*out)[i].period), "%s", PQgetvalue(res, i, 0));

        (*out)[i].revenue = atof(PQgetvalue(res, i, 1));
        (*out)[i].count = atoi(PQgetvalue(res, i, 2));

        if (!PQgetisnull(res, i, 3))
            (*out)[i].venue_id = atoi(PQgetvalue(res, i, 3));
        else
            (*out)[i].venue_id = 0;
    }

    PQclear(res);
    return count;
}
