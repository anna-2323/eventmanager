#include "event.h"
#include "../util.h"

static void get_query(const char* search, const char* sort, char** out);
static void event_from_query(PGresult* res, Event* e, int i);
static void get_query(const char* search, const char* sort, char** out) {
	if (search && search[0] != '\0') {
		const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, "
			"       v.venue_name, v.city, "
			"       MIN(es.price) AS price, "
			"       SUM(es.capacity) - COUNT(t.id) AS seats_left "
			"FROM data.events e "
			"JOIN data.venues v ON e.venue_id = v.id "
			"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
			"LEFT JOIN data.tickets t "
			"    ON t.event_id = e.id "
			"   AND t.sector_id = es.sector_id "
			"WHERE e.title ILIKE '%' || $1 || '%' OR v.venue_name ILIKE '%' || $1 || '%' "
			"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city;";
		*out = malloc(strlen(query) + 1);
		strcpy(*out, query);
		return;
	}
	if (sort && sort[0] != '\0') {
		if (strcmp(sort, "price_asc") == 0) {
					const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, "
				"       v.venue_name, v.city, "
				"       MIN(es.price) AS price, "
				"       SUM(es.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
				"LEFT JOIN data.tickets t "
				"    ON t.event_id = e.id "
				"   AND t.sector_id = es.sector_id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.price ASC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
		else if (strcmp(sort, "price_desc") == 0) {
			const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, "
				"       v.venue_name, v.city, "
				"       MIN(es.price) AS price, "
				"       SUM(es.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
				"LEFT JOIN data.tickets t "
				"    ON t.event_id = e.id "
				"   AND t.sector_id = es.sector_id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.price DESC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
		else if (strcmp(sort, "recent") == 0) {
					const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, "
				"       v.venue_name, v.city, "
				"       MIN(es.price) AS price, "
				"       SUM(es.capacity) - COUNT(t.id) AS seats_left "
				"FROM data.events e "
				"JOIN data.venues v ON e.venue_id = v.id "
				"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
				"LEFT JOIN data.tickets t "
				"    ON t.event_id = e.id "
				"   AND t.sector_id = es.sector_id "
				"WHERE e.begins_at > NOW() "
				"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city "
				"ORDER BY e.id DESC;";
			*out = malloc(strlen(query) + 1);
			strcpy(*out, query);
			return;
		}
	}
	const char* query = "SELECT e.id, e.title, e.begins_at, e.img_path, "
		"       v.venue_name, v.city, "
		"       MIN(es.price) AS price, "
		"       SUM(es.capacity) - COUNT(t.id) AS seats_left "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
		"LEFT JOIN data.tickets t "
		"    ON t.event_id = e.id "
		"   AND t.sector_id = es.sector_id "
		"WHERE e.begins_at > NOW() "
		"GROUP BY e.id, e.title, e.begins_at, e.img_path, "
		"         v.venue_name, v.city "
		"ORDER BY e.begins_at ASC;";
	*out = malloc(strlen(query) + 1);
	strcpy(*out, query);
	return;
}

static void event_from_query(PGresult* res, Event* e, int i) {
	e->id = atoi(PQgetvalue(res, i, 0));
	strncpy(e->title, PQgetvalue(res, i, 1), 100);
	strncpy(e->begins_at, PQgetvalue(res, i, 2), 255);
	strncpy(e->img_path, PQgetvalue(res, i, 3), 255);
	strncpy(e->venue.venue_name, PQgetvalue(res, i, 4), 100);
	strncpy(e->venue.city, PQgetvalue(res, i, 5), 100);
	e->price = atof(PQgetvalue(res, i, 6));
	e->seats_left = atoi(PQgetvalue(res, i, 7));
	if (!PQgetisnull(res, i, 8)) e->verified = atoi(PQgetvalue(res, i, 8));
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
		event_from_query(res, &e, i);
		json_array_append_new(out, event_to_json(e));
	}

	PQclear(res);
	return count;
}

json_t* get_event(PGconn* db, int id) {
	CHECK_DB(db, NULL);

	char* sql =
		"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
		"MIN(es.price) AS price, "
		"SUM(es.capacity) - COUNT(t.id) AS seats_left, "
		"e.verified::int "
		"FROM data.events e "
		"JOIN data.venues v ON e.venue_id = v.id "
		"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
		"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = es.sector_id "
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
	event_from_query(res, &e, 0);

	PQclear(res);
	return event_to_json(e);
}

json_t* get_event_seatmap(PGconn* db, int event_id) {
	CHECK_DB(db, NULL);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);

	const char* params[1] = { id_str };
	// Проверка дали залата има данни за сектори и разположението им
	PGresult* layout_res = PQexecParams(db,
		"SELECT v.has_sectors, v.background_svg, v.viewBox "
		"FROM data.events e "
		"JOIN data.venues v ON v.id = e.venue_id "
		"WHERE e.id = $1;",
		1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(layout_res, db, NULL);

	if (PQntuples(layout_res) == 0) {
		PQclear(layout_res);
		return NULL;
	}

	int has_sectors = strcmp(PQgetvalue(layout_res, 0, 0), "t") == 0;

	json_t* root = json_object();
	json_object_set_new(root, "has_sectors", json_boolean(has_sectors));

	// Ако няма сектори, функцията приключва тук
	if (!has_sectors) {
		PGresult* ga_res = PQexecParams(db,
			"SELECT es.sector_id "
			"FROM data.event_sectors es "
			"WHERE es.event_id = $1;",
			1, NULL, params, NULL, NULL, 0);
		CHECK_QUERY(ga_res, db, NULL);

		if (PQntuples(ga_res) > 0) {
			json_object_set_new(root, "no_sector_id", json_integer(atoi(PQgetvalue(ga_res, 0, 0))));
		}
		json_object_set_new(root, "sectors", json_null());
		PQclear(ga_res);
		return root;
	}

	const char* background_svg = PQgetvalue(layout_res, 0, 1);
	const char* viewbox = PQgetvalue(layout_res, 0, 2);
	json_object_set_new(root, "background_svg", background_svg ? json_string(background_svg) : json_null());
	json_object_set_new(root, "viewbox", viewbox ? json_string(viewbox) : json_null());

	PQclear(layout_res);

	// Получаване на сектори, разположението им и останали места в тях
	const char* sectors_sql =
		"SELECT s.id, s.name, es.capacity, es.price, s.color, s.svg_path, "
		"       es.capacity - COALESCE(t.sold, 0) AS available "
		"FROM data.event_sectors es "
		"JOIN data.sectors s ON s.id = es.sector_id "
		"LEFT JOIN ( "
		"    SELECT sector_id, COUNT(*) AS sold "
		"    FROM data.tickets "
		"    WHERE event_id = $1 "
		"    GROUP BY sector_id "
		") t ON t.sector_id = es.sector_id "
		"WHERE es.event_id = $1 "
		"ORDER BY s.display_order;";
	PGresult* sec_res = PQexecParams(db, sectors_sql, 1, NULL, params, NULL, NULL, 0);
	if (PQresultStatus(sec_res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка във функцията get_event_layout: %s\n", PQerrorMessage(db));
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

json_t* get_user_events(PGconn* db, int id) {
	CHECK_DB(db, NULL);

	// Проверка за роля на потребителя
	char* check_role_sql =
		"SELECT role "
		"FROM data.users "
		"WHERE id = $1; ";

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	
	PGresult* res = PQexecParams(db, check_role_sql, 1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return NULL;
	}
	int role = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	// Ако е организатор, показва качени събития
	if (role == 1) {
		char* sql =
			"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
			"MIN(ls.price) AS price, "
			"SUM(ls.capacity) - COUNT(t.id) AS seats_left "
			"FROM data.events e "
			"JOIN data.venues v ON e.venue_id = v.id "
			"JOIN data.layouts l ON e.layout_id = l.id "
			"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
			"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = ls.id "
			"WHERE e.organizer_id = $1 "
			"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city; ";
		res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	}
	// В противен случай, показва резервирани събития
	else {
		char* sql =
			"SELECT t.event_id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, ls.price, s.name "
			"FROM data.tickets t "
			"JOIN data.events e ON t.event_id = e.id "
			"JOIN data.venues v ON e.venue_id = v.id "
			"JOIN data.layouts l ON e.layout_id = l.id "
			"JOIN data.layout_sectors ls ON ls.layout_id = l.id "
			"JOIN data.sectors s ON ls.sector_id = s.id "
			"WHERE t.user_id = $1 ";
		res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	}

	CHECK_QUERY(res, db, NULL);

	json_t* events = json_array();
	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		json_t* event = json_object();
		json_object_set_new(event, "id",
			json_integer(atoi(PQgetvalue(res, i, 0))));
		json_object_set_new(event, "title",
			json_string(PQgetvalue(res, i, 1)));
		json_object_set_new(event, "begins_at",
			json_string(PQgetvalue(res, i, 2)));
		json_object_set_new(event, "img_path",
			json_string(PQgetvalue(res, i, 3)));
		json_object_set_new(event, "venue_name",
			json_string(PQgetvalue(res, i, 4)));
		json_object_set_new(event, "city",
			json_string(PQgetvalue(res, i, 5)));
		json_object_set_new(event, "price",
			json_real(atof(PQgetvalue(res, i, 6))));
		if (!PQgetisnull(res, i, 7))
			json_object_set_new(event, "seats_left",
				json_integer(atoi(PQgetvalue(res, i, 7))));
		json_array_append_new(events, event);
	}
	return events;
}

int update_event(PGconn* db, const char* sql, int event_id, const char* param) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);
	const char* params[2] = { param, id_str };

	PGresult* res = PQexecParams(db, sql, 2, NULL, params, NULL, NULL, 0);
	CHECK_UPDATE_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_title(PGconn* db, int id, const char* title) {
	char sql[255] = "UPDATE data.events SET title = $1 WHERE id = $2";
	return update_event(db, sql, id, title);
}

int admin_update_begins_at(PGconn* db, int id, const char* begins_at) {
	char sql[255] = "UPDATE data.events SET begins_at = $1 WHERE id = $2";
	return update_event(db, sql, id, begins_at);
}

int verify_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecParams(db,
		"UPDATE data.events SET verified = TRUE WHERE id = $1",
		1, NULL, params, NULL, NULL, 0);
	CHECK_UPDATE_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int unverify_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecParams(db,
		"UPDATE data.events SET verified = FALSE WHERE id = $1",
		1, NULL, params, NULL, NULL, 0);
	CHECK_UPDATE_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int delete_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	char sql[255] = "DELETE FROM data.events "
		"WHERE id = $1 ";
	PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	CHECK_UPDATE_QUERY(res, db, 0);
	PQclear(res);
	return 1;
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
	json_object_set_new(obj, "verified", json_integer(e.verified));
	return obj;
}
