#include "event.h"
#include "../util.h"
#include "../controllers/stats_controller.h"

static void event_from_query(PGresult* res, Event* e, int i) {
	e->id = atoi(PQgetvalue(res, i, 0));
	snprintf(e->title, sizeof(e->title), "%s", PQgetvalue(res, i, 1));
	snprintf(e->begins_at, sizeof(e->begins_at), "%s", PQgetvalue(res, i, 2));
	snprintf(e->img_path, sizeof(e->img_path), "%s", PQgetvalue(res, i, 3));
	snprintf(e->venue.venue_name, sizeof(e->venue.venue_name), "%s", PQgetvalue(res, i, 4));
	snprintf(e->venue.city, sizeof(e->venue.city), "%s", PQgetvalue(res, i, 5));
	e->price = atof(PQgetvalue(res, i, 6));
	e->seats_left = atoi(PQgetvalue(res, i, 7));
	if (!PQgetisnull(res, i, 8))
		snprintf(e->description, sizeof(e->description), "%s", PQgetvalue(res, i, 8));
	if (!PQgetisnull(res, i, 9))
		e->active = (strcmp(PQgetvalue(res, i, 9), "t") == 0);
}

int get_events(PGconn* db, const EventFilters* filters, Event** out) {
	CHECK_DB(db, 0);

	const char* params[7];

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
		snprintf(category_str, sizeof(category_str), "%d", filters->category_id);
		params[3] = category_str;
	}
	else {
		params[3] = NULL;
	}

	char active_str[2];
	snprintf(active_str, sizeof(active_str), "%d", filters->active);
	params[4] = active_str;

	params[5] = filters->from && filters->from[0] != '\0'
		? filters->from
		: NULL;

	params[6] = filters->to && filters->to[0] != '\0'
		? filters->to
		: NULL;

	PGresult* res = PQexecPrepared(db, "get_events", 7, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	int count = PQntuples(res);
	*out = malloc(count * sizeof(Event));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}

	for (int i = 0; i < count; i++) {
		event_from_query(res, &(*out)[i], i);
	}

	PQclear(res);
	return count;
}

int get_event(PGconn* db, int id, Event* out) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecPrepared(db, "get_event", 1, params, NULL, NULL, 0);

	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	event_from_query(res, out, 0);

	PQclear(res);
	return 1;
}

int get_event_seatmap(PGconn* db, int event_id, SeatMap** out) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);
	const char* params[1] = { id_str };

	// Проверка дали залата има данни за сектори и разположението им
	PGresult* layout_res = PQexecPrepared(db, "has_seatmap", 1, params, NULL, NULL, 0);
	CHECK_QUERY(layout_res, db, 0);

	if (PQntuples(layout_res) == 0) {
		PQclear(layout_res);
		return 0;
	}

	*out = malloc(sizeof(SeatMap));
	(*out)->has_sectors = strcmp(PQgetvalue(layout_res, 0, 0), "t") == 0;

	// Ако няма сектори, функцията приключва тук
	if (!(*out)->has_sectors) {
		PGresult* ga_res = PQexecPrepared(db, "no_seatmap", 1, params, NULL, NULL, 0);
		CHECK_QUERY(ga_res, db, 0);

		(*out)->sectors = malloc(sizeof(Sector));
		(*out)->sectors[0].id = atoi(PQgetvalue(ga_res, 0, 0));
		(*out)->sector_count = 1;

		PQclear(ga_res);
		return 1;
	}

	snprintf((*out)->background_svg, sizeof((*out)->background_svg), "%s", PQgetvalue(layout_res, 0, 1));
	snprintf((*out)->viewbox, sizeof((*out)->viewbox), "%s", PQgetvalue(layout_res, 0, 2));

	PQclear(layout_res);

	// Получаване на сектори, разположението им и останали места в тях
	PGresult* sec_res = PQexecPrepared(db, "get_seatmap", 1, params, NULL, NULL, 0);
	if (PQresultStatus(sec_res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка във функцията get_event_layout: %s\n", PQerrorMessage(db));
		PQclear(sec_res);
		return 0;
	}

	int count = PQntuples(sec_res);
	(*out)->sectors = malloc(count * sizeof(Sector));
	if ((*out)->sectors == NULL && count > 0) {
		PQclear(sec_res);
		return 0;
	}

	for (int i = 0; i < count; i++) {
		(*out)->sectors[i].id = atoi(PQgetvalue(sec_res, i, 0));
		snprintf((*out)->sectors[i].name, sizeof((*out)->sectors[i].name), "%s", PQgetvalue(sec_res, i, 1));
		(*out)->sectors[i].capacity = atoi(PQgetvalue(sec_res, i, 2));
		(*out)->sectors[i].price = atof(PQgetvalue(sec_res, i, 3));
		snprintf((*out)->sectors[i].color, sizeof((*out)->sectors[i].color), "%s", PQgetvalue(sec_res, i, 4));
		snprintf((*out)->sectors[i].svg_path, sizeof((*out)->sectors[i].svg_path), "%s", PQgetvalue(sec_res, i, 5));
		(*out)->sectors[i].available = atoi(PQgetvalue(sec_res, i, 6));
	}
	(*out)->sector_count = count;

	PQclear(sec_res);
	return 1;
}

int get_user_events(PGconn* db, int id, Event** out) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	
	// Проверка за роля на потребителя
	PGresult* res = PQexecPrepared(db, "check_role", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}
	int role = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	if (role != 2) {
		res = PQexecPrepared(db, "get_uploaded_events", 1, params, NULL, NULL, 0);
	}

	CHECK_QUERY(res, db, 0);

	int count = PQntuples(res);
	*out = malloc(count * sizeof(Event));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}
 	for (int i = 0; i < count; i++) {
		event_from_query(res, &(*out)[i], i);
	}
	return count;
}

int get_events_in_venue(PGconn* db, int venue_id, Event** out) {
	CHECK_DB(db, 0);

	char venue_id_str[16];
	snprintf(venue_id_str, sizeof(venue_id_str), "%d", venue_id);
	const char* params[1] = { venue_id_str };

	PGresult* res = PQexecPrepared(db, "get_venue_events", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	int count = PQntuples(res);
	*out = malloc(count * sizeof(Event));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}
	for (int i = 0; i < count; i++) {
		event_from_query(res, &(*out)[i], i);
	}

	PQclear(res);
	return count;
}

int add_event(PGconn* db, EventData* data) {
	CHECK_DB(db, 0);

	PQexec(db, "BEGIN");

	// 1. Добавяне на ново събитие
	char venue_id_str[16];
	char organizer_id_str[16];

	snprintf(venue_id_str, sizeof(venue_id_str),
		"%d", data->venue_id);

	snprintf(organizer_id_str, sizeof(organizer_id_str),
		"%d", data->organizer_id);

	const char* params1[6] = {
		data->title,
		data->description,
		data->begins_at,
		venue_id_str,
		organizer_id_str,
		data->img_path
	};

	PGresult* res =
		PQexecPrepared(db, "add_event", 6, params1, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		PQclear(res);
		fprintf(stderr, "Грешка във функцията add_event: %s\n", PQerrorMessage(db));
		PQexec(db, "ROLLBACK");
		return 0;
	}

	int event_id = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	// 2. Намиране на сектор на зала
	Sector* sectors;
	int count = get_sectors(db, data->venue_id, &sectors);
	if(count == 0)
		return 0;

	char event_id_str[16];
	snprintf(event_id_str, sizeof(event_id_str), "%d", event_id);
	char sector_id_str[16];
	snprintf(sector_id_str, sizeof(sector_id_str), "%d", sectors[0].id);
	char price_str[16];
	snprintf(price_str, sizeof(price_str), "%.2f", data->price);
	char capacity_str[16];
	snprintf(capacity_str, sizeof(capacity_str), "%d", data->capacity);
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

int admin_update_description(PGconn* db, int id, const char* description) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[2] = { description, id_str };

	PGresult* res = PQexecPrepared(db, "update_event_description", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_image(PGconn* db, int event_id, const char* img_path) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);
	const char* params[2] = { img_path, id_str };

	PGresult* res = PQexecPrepared(db, "admin_update_event_image", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int get_event_image_path(PGconn* db, int event_id, char* path, size_t pathlen) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", event_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "get_event_image_path", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	snprintf(path, pathlen, "%s", PQgetvalue(res, 0, 0));

	PQclear(res);
	return 1;
}

int set_event_active(PGconn* db, int id, int active) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	const char* query_name = active
		? "activate_event"
		: "deactivate_event";

	PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
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

int get_categories(PGconn* db, Category** out) {
	CHECK_DB(db, 0);

	PGresult* res = PQexecPrepared(db, "get_categories", 0, NULL, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	int count = PQntuples(res);
	*out = malloc(count * sizeof(Category));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}

	for (int i = 0; i < count; i++) {
		(*out)[i].id = atoi(PQgetvalue(res, i, 0));
		snprintf((*out)[i].title, sizeof((*out)[i].title), "%s", PQgetvalue(res, i, 1));
	}

	PQclear(res);
	return count;
}

int get_total_events(PGconn* db, int organizer_id) {
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

	PGresult* res = PQexecPrepared(db, "get_total_events", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, -1);

	int total = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	return total;
}
int get_events_growth(PGconn* db, int type, int organizer_id, StatGrowth** out) {
	CHECK_DB(db, -1);

	const char* query_name;

	if (type == STAT_MONTHLY) {
		query_name = "get_events_growth_monthly";
	}
	else if (type == STAT_MONTHLY_ALL) {
		query_name = "get_events_growth_monthly_all";
	}
	else if (type == STAT_DAILY) {
		query_name = "get_events_growth_daily";
	}
	else {
		return -1;
	}

	char id_str[16];
	const char* params[1];

	if (organizer_id > 0) {
		snprintf(id_str, sizeof(id_str), "%d", organizer_id);
		params[0] = id_str;
	}
	else {
		params[0] = NULL;
	}

	PGresult* res = PQexecPrepared(db, query_name, 1, params, NULL, NULL, 0);
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