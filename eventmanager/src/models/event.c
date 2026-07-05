#include "event.h"

void get_query(const char* search, const char* sort, char** out) {
	if (search && search[0] != '\0') {
		*out = malloc(256);
		strcpy(*out,
			"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
			"FROM data.events e "
			"JOIN data.venues v ON e.venue_id = v.id "
			"WHERE e.title ILIKE '%' || $1 || '%' OR v.venue_name ILIKE '%' || $1 || '%';");
		return;
	}
	if (sort && sort[0] != '\0') {
		if (strcmp(sort, "price_asc") == 0) {
			*out = malloc(256);
			strcpy(*out,
				"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"WHERE e.begins_at > NOW() "
				"ORDER BY e.price ASC;");
			return;
		}
		else if (strcmp(sort, "price_desc") == 0) {
			*out = malloc(256);
			strcpy(*out,
				"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"WHERE e.begins_at > NOW() "
				"ORDER BY e.price DESC;");
			return;
		}
	}
	*out = malloc(256);
	strcpy(*out,
		"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"WHERE e.begins_at > NOW() "
		"ORDER BY e.begins_at ASC;");
	return;
}

void get_event_from_query(PGresult* res, Event* e, int i) {
	e->id = atoi(PQgetvalue(res, i, 0));
	strncpy(e->title, PQgetvalue(res, i, 1), 100);
	e->price = atof(PQgetvalue(res, i, 2));
	strncpy(e->begins_at, PQgetvalue(res, i, 3), 255);
	strncpy(e->img_path, PQgetvalue(res, i, 4), 255);
	strncpy(e->venue.venue_name, PQgetvalue(res, i, 5), 100);
	strncpy(e->venue.city, PQgetvalue(res, i, 6), 100);
	e->seats_left = atoi(PQgetvalue(res, i, 7));
}

int get_events(PGconn* db, const char* search, const char* sort, json_t* out) {
	if (!db) return NULL;

	char* sql = NULL;
	get_query(search, sort, &sql);

	PGresult* res = NULL;
	char param_str[256];
	if (search && search[0] != '\0') {
		snprintf(param_str, sizeof(param_str), "%s", search);
		const char* params[1] = { param_str };
		res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	}
	else if (sort && sort[0] != '\0') {
		snprintf(param_str, sizeof(param_str), "%s", sort);
		const char* params[1] = { param_str };
		res = PQexec(db, sql);
	}
	else
		res = PQexec(db, sql);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка при изпъляване на заявка: %s\n", PQerrorMessage(db));
		PQclear(res);
		return 0;
	}

	Event e = { 0 };
	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		get_event_from_query(res, &e, i);
		json_array_append_new(out, event_to_json(e));
	}

	PQclear(res);
	return count;
}

json_t* get_event(PGconn* db, int id) {
	if (!db) return NULL;

	char* sql =
		"SELECT e.id, e.title, e.price, e.begins_at, e.img_path, v.venue_name, v.city, e.seats_left "
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
	get_event_from_query(res, &e, 0);

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
