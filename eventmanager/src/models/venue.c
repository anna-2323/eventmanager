#include "venue.h"

static json_t* venue_from_query(PGresult* res, int i) {
    json_t* v = json_object();
    json_object_set_new(v, "id",
        json_integer(atoi(PQgetvalue(res, i, 0))));
    json_object_set_new(v, "city",
        json_string(PQgetvalue(res, i, 1)));
    json_object_set_new(v, "address",
        json_string(PQgetvalue(res, i, 2)));
    json_object_set_new(v, "venue_name",
        json_string(PQgetvalue(res, i, 3)));
    if (!PQgetisnull(res, i, 4))
        json_object_set_new(v, "active",
            json_integer(atoi(PQgetvalue(res, i, 4))));
    if(!PQgetisnull(res, i, 5))
        json_object_set_new(v, "has_sectors",
            json_integer(atoi(PQgetvalue(res, i, 5))));
    return v;
}

int get_venues(PGconn* db, json_t* out) {
	CHECK_DB(db, 0);

    const char* sql =
        "SELECT v.id, v.city, v.address, v.venue_name "
        "FROM data.venues v;";
    PGresult* res = PQexec(db, sql);
    CHECK_QUERY(res, db, 0);

    int count = PQntuples(res);
    for (int i = 0; i < count; i++) {
        json_array_append_new(out, venue_from_query(res, i));
    }

    PQclear(res);
    return count;
}

json_t* get_venue(PGconn* db, int id) {
    CHECK_DB(db, NULL);

    const char* sql =
        "SELECT v.id, v.city, v.address, v.venue_name, v.active, v.has_sectors::int "
        "FROM data.venues v "
        "WHERE v.id = $1;";

    char id_str[16];
    snprintf(id_str, sizeof(id_str), "%d", id);
    const char* params[1] = { id_str };

    PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Грешка в get_venue: %s\n", PQerrorMessage(db));
        PQclear(res);
        return 1;
    }

    json_t* v = venue_from_query(res, 0);

    PQclear(res);
    return v;
}

json_t* get_sectors(PGconn* db, int venue_id) {
    CHECK_DB(db, NULL);

    const char* sql =
        "SELECT s.name "
        "FROM data.sectors s "
        "WHERE s.venue_id = $1;";

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
    const char* params[1] = { venue_id_str };

    PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
    CHECK_DB(db, NULL);

    json_t* sectors = json_array();

    int count = PQntuples(res);
    for (int i = 0; i < count; i++) {
        json_array_append_new(
            sectors,
            json_string(PQgetvalue(res, i, 0))
        );
    }

    PQclear(res);
    return sectors;
}

json_t* get_cities(PGconn* db)
{
    CHECK_DB(db, NULL);

    const char* sql =
        "SELECT DISTINCT city "
        "FROM data.venues "
        "ORDER BY city;";

    PGresult* res = PQexec(db, sql);
    CHECK_QUERY(res, db, NULL);

    json_t* cities = json_array();

    int count = PQntuples(res);
    for (int i = 0; i < count; i++) {
        json_array_append_new(
            cities,
            json_string(PQgetvalue(res, i, 0))
        );
    }

    PQclear(res);
    return cities;
}

int add_venue(PGconn* db, const char* city, const char* address, const char* venue_name) {
    CHECK_DB(db, 0);

    // 1. Добавяне на нова зала; приема се, че няма сектори
    const char* sql1 =
        "INSERT INTO data.venues (city, address, venue_name, has_sectors) "
        "VALUES ($1, $2, $3, false) "
        "RETURNING id";

    const char* params1[3] = { city, address, venue_name };

    PGresult* res = PQexecParams(db, sql1, 3, NULL, params1, NULL, NULL, 0);
    CHECK_QUERY(res, db, 0);

    int venue_id = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
    const char* params2[1] = { venue_id_str };

    // 2. Добавяне на един единствен сектор
    const char* sql2 =
        "INSERT INTO data.sectors "
        "(venue_id) VALUES ($1)";
    res = PQexecParams(db, sql2, 1, NULL, params2, NULL, NULL, 0);
    CHECK_UPDATE_QUERY(res, db, 0);

    return venue_id;
}

int update_venue_name(PGconn* db, int id, const char* venue_name) {
    CHECK_DB(db, 0);

    const char* sql = "UPDATE data.venues SET venue_name = $1 WHERE id = $2;";

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);
    const char* params[2] = { venue_name, venue_id_str };

    PGresult* res = PQexecParams(db, sql, 2, NULL, params, NULL, NULL, 0);
    CHECK_UPDATE_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int soft_delete_venue(PGconn* db, int id) {
    CHECK_DB(db, 0);

    // Залата не се изтрива напълно за да се предотвратят конфликти в минали записи, свързани с тази зала
    const char* sql = "UPDATE data.venues SET active = FALSE WHERE id = $1;";

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);
    const char* params[1] = { venue_id_str };

    PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
    CHECK_UPDATE_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}

int restore_venue(PGconn* db, int id) {
    CHECK_DB(db, 0);

    // Залата не се изтрива напълно за да се предотвратят конфликти в минали записи, свързани с тази зала
    const char* sql = "UPDATE data.venues SET active = TRUE WHERE id = $1;";

    char venue_id_str[16];
    snprintf(venue_id_str, sizeof(venue_id_str), "%d", id);
    const char* params[1] = { venue_id_str };

    PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
    CHECK_UPDATE_QUERY(res, db, 0);

    PQclear(res);
    return 1;
}
