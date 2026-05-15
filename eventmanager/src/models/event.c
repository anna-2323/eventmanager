#include "event.h"

int get_all_events(PGconn* db, json_t* out) {
	if (!db) return NULL;

	char* sql =
		"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id; ";
	PGresult* res = PQexec(db, sql);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		PQclear(res);
		return 0;
	}

	Event e = { 0 };
	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		e.id = atoi(PQgetvalue(res, i, 0));
		strncpy(e.title, PQgetvalue(res, i, 1), 100);
		e.price = atof(PQgetvalue(res, i, 2));
		strncpy(e.begins_at, PQgetvalue(res, i, 3), 255);
		strncpy(e.img_path, PQgetvalue(res, i, 4), 255);
		strncpy(e.venue.venue_name, PQgetvalue(res, i, 5), 100);
		strncpy(e.venue.city, PQgetvalue(res, i, 6), 100);
		e.seats_left = atoi(PQgetvalue(res, i, 7));

		json_array_append_new(out, event_to_json(e));
	}

	PQclear(res);
	return count;
}

json_t* get_event(PGconn* db, int id) {
	if (!db) return NULL;

	char* sql =
		"SELECT e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"WHERE e.id = $1;";
	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
		PQclear(res);
		return NULL;
	}

	Event e = { 0 };
	e.id = id;
	strncpy(e.title, PQgetvalue(res, 0, 0), 100);
	e.price = atof(PQgetvalue(res, 0, 1));
	strncpy(e.begins_at, PQgetvalue(res, 0, 2), 255);
	strncpy(e.img_path, PQgetvalue(res, 0, 3), 255);
	strncpy(e.venue.venue_name, PQgetvalue(res, 0, 4), 100);
	strncpy(e.venue.city, PQgetvalue(res, 0, 5), 100);
	e.seats_left = atoi(PQgetvalue(res, 0, 6));

	PQclear(res);
	return event_to_json(e);
}

json_t* event_to_json(Event e) {
	json_t* obj = json_object();
	json_object_set_new(obj, "id", json_integer(e.id));
	json_object_set_new(obj, "title", json_string(e.title));
	json_object_set_new(obj, "price", json_real(e.price));
	json_object_set_new(obj, "begins_at", json_string(e.begins_at));
	json_object_set_new(obj, "img_path", json_string(e.img_path));
	json_object_set_new(obj, "venue_name", json_string(e.venue.venue_name));
	json_object_set_new(obj, "city", json_string(e.venue.city));
	json_object_set_new(obj, "seats_left", json_integer(e.seats_left));
	return obj;
}
