#include "event.h"
#include "../util.h"

void get_query(const char* search, const char* sort, char** out) {
	if (search && search[0] != '\0') {
		const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
			"MIN(ls.price) AS price, "
			"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
			"FROM data.events e "
			"JOIN data.venues v ON e.venue_id = v.id "
			"JOIN data.layouts l ON e.layout_id = l.id "
			"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
			"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
			"WHERE e.title ILIKE '%' || $1 || '%' OR v.venue_name ILIKE '%' || $1 || '%' "
			"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city;";
		*out = malloc(strlen(query) + 1);
		strcpy(*out, query);
		return;
	}
	if (sort && sort[0] != '\0') {
		if (strcmp(sort, "price_asc") == 0) {
			const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
				"MIN(ls.price) AS price, "
				"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"JOIN data.layouts l ON e.layout_id = l.id "
				"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
				"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.price ASC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
		else if (strcmp(sort, "price_desc") == 0) {
			const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
				"MIN(ls.price) AS price, "
				"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"JOIN data.layouts l ON e.layout_id = l.id "
				"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
				"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.price DESC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
		else if (strcmp(sort, "recent") == 0) {
			const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
				"MIN(ls.price) AS price, "
				"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"JOIN data.layouts l ON e.layout_id = l.id "
				"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
				"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.id DESC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
	}
	const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
		"MIN(ls.price) AS price, "
		"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"JOIN data.layouts l ON e.layout_id = l.id "
		"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
		"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
		"WHERE e.begins_at > NOW() "
		"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
		"ORDER BY e.begins_at ASC;";
	*out = malloc(strlen(query) + 1);
	strcpy(*out, query);
	return;
}

void get_event_from_query(PGresult* res, Event* e, int i) {
	e->id = atoi(PQgetvalue(res, i, 0));
	strncpy(e->title, PQgetvalue(res, i, 1), 100);
	strncpy(e->begins_at, PQgetvalue(res, i, 2), 255);
	strncpy(e->img_path, PQgetvalue(res, i, 3), 255);
	strncpy(e->venue.venue_name, PQgetvalue(res, i, 4), 100);
	strncpy(e->venue.city, PQgetvalue(res, i, 5), 100);
	e->price = atof(PQgetvalue(res, i, 6));
	e->seats_left = atoi(PQgetvalue(res, i, 7));
}

int get_events(PGconn* db, const char* search, const char* sort, json_t* out) {
	CHECK_DB(db, 0);

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

	CHECK_QUERY(res, db, 0);

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
	CHECK_DB(db, NULL);

	char* sql =
		"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
		"MIN(ls.price) AS price, "
		"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"JOIN data.layouts l ON e.layout_id = l.id "
		"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
		"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
		"WHERE e.id = $1 "
		"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city; ";
	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);

	CHECK_QUERY(res, db, NULL);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return NULL;
	}

	Event e = { 0 };
	get_event_from_query(res, &e, 0);

	PQclear(res);
	return event_to_json(e);
}

json_t* get_event_layout(PGconn* db, int event_id) {
	CHECK_DB(db, NULL);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);

	const char* params1[1] = { id_str };
	// Проверка дали залата има данни за сектори и разположението им
	PGresult* layout_res = PQexecParams(db,
		"SELECT v.has_sectors, l.id, l.svg_viewbox, l.background_svg "
		"FROM data.events e "
		"JOIN data.venues v ON v.id = e.venue_id "
		"LEFT JOIN data.layouts l ON l.id = e.layout_id "
		"WHERE e.id = $1;",
		1, NULL, params1, NULL, NULL, 0);
	CHECK_QUERY(layout_res, db, NULL);

	if (PQntuples(layout_res) == 0) {
		PQclear(layout_res);
		return NULL;
	}

	int has_sectors = strcmp(PQgetvalue(layout_res, 0, 0), "t") == 0;

	json_t* root = json_object();
	json_object_set_new(root, "has_sectors", json_boolean(has_sectors));

	int layout_id = atoi(PQgetvalue(layout_res, 0, 1));

	// Ако няма сектори, функцията приключва тук
	if (!has_sectors) {
		char layout_id_str[16];
		snprintf(layout_id_str, sizeof(layout_id_str), "%d", layout_id);
		const char* params[1] = { layout_id_str };
		PGresult* ga_res = PQexecParams(db,
			"SELECT ls.id FROM data.layout_sectors ls "
			"WHERE ls.layout_id = $1;",
			1, NULL, params, NULL, NULL, 0);
		CHECK_QUERY(ga_res, db, NULL);

		if (PQntuples(ga_res) > 0) {
			json_object_set_new(root, "no_sector_id", json_integer(atoi(PQgetvalue(ga_res, 0, 0))));
		}
		json_object_set_new(root, "sectors", json_null());
		PQclear(ga_res);
		return root;
	}

	const char* viewbox = PQgetvalue(layout_res, 0, 2);
	const char* background_svg = PQgetvalue(layout_res, 0, 3);

	json_object_set_new(root, "viewbox", json_string(viewbox));
	json_object_set_new(root, "background_svg", background_svg ? json_string(background_svg) : json_null());

	PQclear(layout_res);

	// Получаване на сектори, разположението им и останали места в тях
	char layout_id_str[16];
	snprintf(layout_id_str, sizeof(layout_id_str), "%d", layout_id);

	const char* sectors_sql =
		"SELECT ls.id, s.name, ls.capacity, ls.price, s.color, s.svg_path, "
		"       ls.capacity - COALESCE(t.sold, 0) AS available "
		"FROM data.layout_sectors ls "
		"JOIN data.sectors s ON s.id = ls.sector_id "
		"LEFT JOIN ( "
		"    SELECT sector_id, COUNT(*) AS sold "
		"    FROM data.tickets "
		"    WHERE event_id = $1 "
		"    GROUP BY sector_id "
		") t ON t.sector_id = ls.id "
		"WHERE ls.layout_id = $2 "
		"ORDER BY s.display_order;";
	const char* params2[2] = { id_str, layout_id_str };
	PGresult* sec_res = PQexecParams(db, sectors_sql, 2, NULL, params2, NULL, NULL, 0);
	if (PQresultStatus(sec_res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Query failed: %s\n", PQerrorMessage(db));
		PQclear(sec_res);
		json_decref(root);
		return NULL;
	}

	json_t* sectors = json_array();
	int n = PQntuples(sec_res);
	for (int i = 0; i < n; i++) {
		json_t* sector = json_object();
		json_object_set_new(sector, "id", json_integer(atoi(PQgetvalue(sec_res, i, 0))));
		json_object_set_new(sector, "name", json_string(PQgetvalue(sec_res, i, 1)));
		json_object_set_new(sector, "capacity", json_integer(atoi(PQgetvalue(sec_res, i, 2))));
		json_object_set_new(sector, "price", json_real(atof(PQgetvalue(sec_res, i, 3))));
		json_object_set_new(sector, "color", json_string(PQgetvalue(sec_res, i, 4)));
		json_object_set_new(sector, "svg_path", json_string(PQgetvalue(sec_res, i, 5)));
		json_object_set_new(sector, "available", json_integer(atoi(PQgetvalue(sec_res, i, 6))));
		json_array_append_new(sectors, sector);
	}
	json_object_set_new(root, "sectors", sectors);

	PQclear(sec_res);
	return root;
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
