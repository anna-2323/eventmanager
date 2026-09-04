#include "venue.h"

static void venue_from_query(PGresult* res, Venue* v, int i) {
    v->id = atoi(PQgetvalue(res, i, 0));
    snprintf(v->city, sizeof(v->city), "%s", PQgetvalue(res, i, 1));
    snprintf(v->address, sizeof(v->address), "%s", PQgetvalue(res, i, 2));
    snprintf(v->venue_name, sizeof(v->venue_name), "%s", PQgetvalue(res, i, 3));
    if (!PQgetisnull(res, i, 4))
        v->has_sectors = (strcmp(PQgetvalue(res, i, 4), "t") == 0);
    if(!PQgetisnull(res, i, 5))
        v->active = (strcmp(PQgetvalue(res, i, 5), "t") == 0);
}

int get_venues(PGconn* db, Venue** out, int active) {
	CHECK_DB(db, 0);

    char active_str[16];
    snprintf(active_str, sizeof(active_str), "%d", active);
    const char* params[1] = { active_str };

    PGresult* res = PQexecPrepared(db, "get_venues", 1, params, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);

    *out = malloc(count * sizeof(Venue));

    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        venue_from_query(res, &(*out)[i], i);
    }

    PQclear(res);
    return count;
}

int get_venue(PGconn* db, int id, Venue* out) {
    CHECK_DB(db, 0);

    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecPrepared(db, "get_venue", 1, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Грешка в get_venue: %s\n", PQerrorMessage(db));
        PQclear(res);
        return 0;
    }

    venue_from_query(res, out, 0);

    PQclear(res);
    return 1;
}

int get_sectors(PGconn* db, int venue_id, Sector** out) {
    CHECK_DB(db, NULL);

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
    const char* params[1] = { venue_id_str };

    PGresult* res = PQexecPrepared(db, "get_sectors", 1, params, NULL, NULL, 0);
    CHECK_DB(db, NULL);

    int count = PQntuples(res);
    *out = malloc(count * sizeof(Sector));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        (*out)[i].id = atoi(PQgetvalue(res, i, 0));
        snprintf((*out)[i].name, sizeof((*out)[i].name), "%s", PQgetvalue(res, i, 1));
    }

    PQclear(res);
    return count;
}

int get_cities(PGconn* db, char*** out)
{
    CHECK_DB(db, 0);
    PGresult* res = PQexecPrepared(db, "get_cities", 0, NULL, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);
    int count = PQntuples(res);

    *out = malloc(count * sizeof(char*));
    if (*out == NULL && count > 0) {
        PQclear(res);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        const char* value = PQgetvalue(res, i, 0);
        (*out)[i] = malloc(strlen(value) + 1);
        // При неуспех се освобождава паметта
        if ((*out)[i] == NULL) {
            for (int j = 0; j < i; j++)
                free((*out)[j]);
            free(*out);
            *out = NULL;
            PQclear(res);
            return 0;
        }
        strcpy((*out)[i], value);
    }
    PQclear(res);
    return count;
}

int add_venue(PGconn* db, Venue* v) {
    CHECK_DB(db, 0);

    // 1. Добавяне на нова зала; приема се, че няма сектори
    const char* params1[3] = { v->city, v->address, v->venue_name };

    PGresult* res = PQexecPrepared(db, "add_venue", 3, params1, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int venue_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    // 2. Добавяне на един единствен сектор
    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
    const char* params2[1] = { venue_id_str };

    res = PQexecPrepared(db, "add_venue_sector", 1, params2, NULL, NULL, 0);
    CHECK_COMMAND_QUERY(res, db, 0);

    return venue_id;
}

int update_venue_name(PGconn* db, int id, const char* venue_name) {
    CHECK_DB(db, 0);

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);
    const char* params[2] = { venue_name, venue_id_str };

    PGresult* res = PQexecPrepared(db, "update_venue_name", 2, params, NULL, NULL, 0);
    CHECK_COMMAND_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int update_venue_address(PGconn* db, int id, const char* address) {
    CHECK_DB(db, 0);

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);
    const char* params[2] = { address, venue_id_str };

    PGresult* res = PQexecPrepared(db, "update_venue_address", 2, params, NULL, NULL, 0);
    CHECK_COMMAND_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int set_venue_active(PGconn* db, int id, int active) {
    CHECK_DB(db, 0);

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);

    const char* params[1] = { venue_id_str };

    const char* query_name = active
        ? "activate_venue"
        : "deactivate_venue";

    PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
    CHECK_COMMAND_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int get_total_venues(PGconn* db) {
    CHECK_DB(db, NULL);

    PGresult* res = PQexecPrepared(db, "get_total_venues", 0, NULL, NULL, NULL, 0);
    CHECK_QUERY(res, db, NULL);

    int total = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    return total;
}
