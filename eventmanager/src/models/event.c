#include "event.h"
#include "../util.h"

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

int get_events(PGconn* db, const EventFilters* filters, json_t* out) {
	CHECK_DB(db, 0);

	const char* params[4];

	char upcoming_str[2];
	snprintf(upcoming_str, sizeof(upcoming_str), "%d", filters->upcoming);
	params[0] = upcoming_str;
	params[1] = filters->search && filters->search[0] != '\0'
		? filters->search
		: NULL;
	params[2] = filters->city && filters->city[0] != '\0'
		? filters->city
		: NULL;

	char category_str[32];

	if (filters->category_id > 0) {
		snprintf(
			category_str,
			sizeof(category_str),
			"%d",
			filters->category_id
		);

		params[3] = category_str;
	}
	else {
		params[3] = NULL;
	}

	PGresult* res = PQexecPrepared(
		db,
		"get_events",
		4,
		params,
		NULL,
		NULL,
		0
	);

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

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecPrepared(db, "get_event", 1, params, NULL, NULL, 0);

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
	PGresult* layout_res = PQexecPrepared(db, "has_seatmap", 1, params, NULL, NULL, 0);
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
		PGresult* ga_res = PQexecPrepared(db, "no_seatmap", 1, params, NULL, NULL, 0);
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
	PGresult* sec_res = PQexecPrepared(db, "get_seatmap", 1, params, NULL, NULL, 0);
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

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	
	// Проверка за роля на потребителя
	PGresult* res = PQexecPrepared(db, "check_role", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return NULL;
	}
	int role = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	// Ако е организатор, показва качени събития
	if (role == 1) {
		res = PQexecPrepared(db, "get_uploaded_events", 1, params, NULL, NULL, 0);
	}
	// В противен случай, показва резервирани събития
	else {
		res = PQexecPrepared(db, "get_booked_events", 1, params, NULL, NULL, 0);
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

json_t* get_events_in_venue(PGconn* db, int venue_id) {
	CHECK_DB(db, NULL);

	char venue_id_str[16];
	snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
	const char* params[1] = { venue_id_str };

	PGresult* res = PQexecPrepared(db, "get_venue_events", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);

	json_t* events = json_array();
	Event e = { 0 };
	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		json_t* event = json_object();
		event_from_query(res, &e, i);
		json_array_append_new(events, event_to_json(e));
	}

	PQclear(res);
	return events;
}

int add_event(
	PGconn* db, int organizer_id, int venue_id, const char* title, 
	const char* begins_at, double price, int capacity) {
	CHECK_DB(db, 0);

	PQexec(db, "BEGIN");

	// 1. Добавяне на ново събитие
	char venue_id_str[16];
	snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
	char organizer_id_str[16];
	snprintf(organizer_id_str, sizeof(organizer_id_str), "%d", organizer_id);
	const char* params1[4] = { title, begins_at, venue_id_str, organizer_id_str };

	PGresult* res = PQexecPrepared(db, "add_event", 4, params1, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		PQclear(res);
		fprintf(stderr, "Грешка във функцията add_event: %s\n", PQerrorMessage(db));
		PQexec(db, "ROLLBACK");
		return 0;
	}

	int event_id = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	// 2. Намиране на сектор на зала
	json_t* sectors = get_sectors(db, venue_id);
	int sector_id =
		json_integer_value(json_object_get(json_array_get(sectors, 0), "id"));

	char event_id_str[16];
	snprintf(event_id_str, sizeof(event_id_str), "%d", event_id);
	char sector_id_str[16];
	snprintf(sector_id_str, sizeof(sector_id_str), "%d", sector_id);
	char price_str[16];
	snprintf(price_str, sizeof(price_str), "%.2f", price);
	char capacity_str[16];
	snprintf(capacity_str, sizeof(capacity_str), "%d", capacity);
	const char* params2[4] = { event_id_str, sector_id_str, price_str, capacity_str };

	// 3. Добавяне на запис в event_sectors
	res = PQexecPrepared(db, "add_event_sectors", 4, params2, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		PQclear(res);
		fprintf(stderr, "Грешка във функцията add_event: %s\n", PQerrorMessage(db));
		PQexec(db, "ROLLBACK");
		return 0;
	}

	json_decref(sectors);
	PQclear(res);
	PQexec(db, "COMMIT");
	return event_id;
}

int admin_update_title(PGconn* db, int id, const char* title) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[2] = { title, id_str };

	PGresult* res = PQexecPrepared(db, "update_event_title", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_begins_at(PGconn* db, int id, const char* begins_at) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[2] = { begins_at, id_str };

	PGresult* res = PQexecPrepared(db, "update_event_begins_at", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int verify_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "verify_event", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int unverify_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "unverify_event", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int delete_event(PGconn* db, int id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "delete_event", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

json_t* get_categories(PGconn* db) {
	CHECK_DB(db, NULL);

	PGresult* res = PQexecPrepared(db, "get_categories", 0, NULL, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);

	json_t* categories = json_array();

	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		json_t* category = json_object();
		json_object_set_new(category, "id", json_string(PQgetvalue(res, i, 0)));
		json_object_set_new(category, "title", json_string(PQgetvalue(res, i, 1)));
		json_array_append_new(
			categories,
			category
		);
	}

	PQclear(res);
	return categories;
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

json_t* get_total_events(PGconn* db) {
	CHECK_DB(db, NULL);

	PGresult* res = PQexecPrepared(db, "get_total_events", 0, NULL, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);

	int total = atoi(PQgetvalue(res, 0, 0));

	return json_integer(total);
}

static json_t* get_events_growth_json(PGresult* res, int type) {
	json_t* growth = json_array();
	int count = PQntuples(res);

	for (int i = 0; i < count; i++) {
		const char* period = PQgetvalue(res, i, 0);
		int event_count = atoi(PQgetvalue(res, i, 1));

		json_t* entry = json_object();

		if (type == 0 || type == 1)
			json_object_set_new(
				entry,
				"month",
				json_string(period)
			);
		else if (type == 2)
			json_object_set_new(
				entry,
				"day",
				json_string(period)
			);

		json_object_set_new(
			entry,
			"event_count",
			json_integer(event_count)
		);

		json_array_append_new(growth, entry);
	}

	PQclear(res);
	return growth;
}

json_t* get_events_growth(PGconn* db, int type) {
	CHECK_DB(db, NULL);

	PGresult* res;
	if (type == 0) {
		res = PQexecPrepared(db, "get_events_growth_monthly", 0, NULL, NULL, NULL, 0);
		CHECK_QUERY(res, db, NULL);
		return get_events_growth_json(res, type);
	}
	else if (type == 1) {
		res = PQexecPrepared(db, "get_events_growth_monthly_all", 0, NULL, NULL, NULL, 0);
		CHECK_QUERY(res, db, NULL);
		return get_events_growth_json(res, type);
	}
	else if (type == 2) {
		res = PQexecPrepared(db, "get_events_growth_daily", 0, NULL, NULL, NULL, 0);
		CHECK_QUERY(res, db, NULL);
		return get_events_growth_json(res, type);
	}
}
