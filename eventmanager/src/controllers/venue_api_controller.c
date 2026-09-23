#include "venue_api_controller.h"
#include "controller.h"

// GET /api/cities
int api_cities(struct mg_connection* conn, void* data) {
	char** cities;
	int count = get_cities((PGconn*)data, &cities);

	json_t* res = json_array();
	for (int i = 0; i < count; i++) {
		json_array_append(res, json_string(cities[i]));
	}
	
	free(cities);
	return send_result(conn, 1, 200, "", res);
}

// GET /api/venues
int api_venues(struct mg_connection* conn, void* data) {
	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->local_uri, "/api/venues") == 0) {
		// GET /api/venues
		if (strcmp(info->request_method, "GET") == 0) {
			json_t* res = json_array();
			Venue* venues;
			int count = get_venues(db, &venues, 1);
			for (int i = 0; i < count; i++) {
				json_array_append(res, venue_to_json(&venues[i]));
			}
			free(venues);
			return send_result(conn, 1, 200, "", res);
		}
		else {
			mg_send_http_error(conn, 405, "Method Not Allowed");
			return 405;
		}
	}

	json_t* res = json_object();

	const char* id_str = info->local_uri + strlen("/api/venues/");
	int venue_id = atoi(id_str);

	char uri[64];
	snprintf(uri, sizeof(uri), "/api/venues/%d/events", venue_id);
	// GET /api/venues/{id}/events
	if (strcmp(info->local_uri, uri) == 0) {

		if (strcmp(info->request_method, "GET") != 0) {
			mg_send_http_error(conn, 405, "Method Not Allowed");
			return 405;
		}

		Event* events = NULL;
		int count = get_events_in_venue(db, venue_id, &events);
		json_t* res = json_array();
		for (int i = 0; i < count; i++) {
			json_array_append(res, event_to_json(&events[i]));
		}
		free(events);
		return send_result(conn, 1, 200, "", res);
	}

	if (venue_id <= 0)
		return send_result(conn, 0, 404, "Невалидно ID на зала.", NULL);

	// GET /api/venues/{id}
	if (strcmp(info->request_method, "GET") == 0) {
		Venue venue;
		if(get_venue(db, venue_id, &venue))
			return send_result(conn, 1, 200, "", venue_to_json(&venue));
		else
			return send_result(conn, 0, 404, "Залата не съществува.", NULL);
	}
	
	mg_send_http_error(conn, 405, "Method Not Allowed");
	return 405;
}

// POST, PATCH /api/admin/venues
int api_admin_venues(struct mg_connection* conn, void* data) {
	if (!check_role(conn, ROLE_ADMIN) && !check_role(conn, ROLE_ORGANIZATOR)) {
		return send_result(conn, 0, 403, "Нямате права за това действие", NULL);
	}

	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	// GET, POST /api/admin/venues
	if (strcmp(info->local_uri, "/api/admin/venues") == 0) {
		if (strcmp(info->request_method, "GET") == 0) {
			json_t* res = json_array();
			Venue* venues;
			int count = get_venues(db, &venues, 0);
			for (int i = 0; i < count; i++) {
				json_array_append(res, venue_to_json(&venues[i]));
			}
			free(venues);
			return send_result(conn, 1, 200, "", res);
		}
		if (strcmp(info->request_method, "POST") == 0) {
			json_t* req = get_json(conn);
			if (!req) return 400;

			Venue v;
			snprintf(v.city, sizeof(v.city), "%s", json_string_value(json_object_get(req, "city")));
			snprintf(v.address, sizeof(v.address), "%s", json_string_value(json_object_get(req, "address")));
			snprintf(v.venue_name, sizeof(v.venue_name), "%s", json_string_value(json_object_get(req, "venue_name")));
			if (!v.city || !v.address || !v.venue_name)
				return send_result(conn, 0, 400, "Моля, попълнете всички полета", NULL);

			int result = add_venue(db, &v);

			json_decref(req);
			return send_result(conn, 1, 201, "Залата е добавена успешно.", NULL);
		}
		else {
			mg_send_http_error(conn, 405, "Method Not Allowed");
			return 405;
		}
	}
	
	const char* id_str = info->local_uri + strlen("/api/admin/venues/");
	char* end;
	long id = strtol(id_str, &end, 10);

	json_t* res = json_object();

	// PATCH /api/venues/{id}
	if (strcmp(info->request_method, "PATCH") == 0) {
		json_t* req = get_json(conn);
		if (!req)
			return 400;

		int result = 0;

		json_t* active_json = json_object_get(req, "active");
		const char* venue_name = json_string_value(json_object_get(req, "venue_name"));
		const char* address = json_string_value(json_object_get(req, "address"));

		if (active_json) {
			if (!json_is_boolean(active_json)) {
				result = 0;
			}
			else if (!check_role(conn, ROLE_ADMIN)) {
				send_result(conn, 0, 403, "Нямате права за това действие", NULL);
				json_decref(req);
				return 1;
			}
			else {
				result = set_venue_active(db, id, json_boolean_value(active_json));
				json_decref(req);
				if (result) {
					if (json_boolean_value(active_json)) {
						return send_result(conn, 1, 200, "Залата е успешно активирана.", NULL);
					}
					else {
						return send_result(conn, 1, 200, "Залата е успешно деактивирана.", NULL);
					}
				}
				else return 500;
			}
		}
		else if (venue_name && venue_name[0] != '\0') {
			result = update_venue_name(db, id, venue_name);
			json_decref(req);
			if (result)
				return send_result(conn, 1, 200, "Успешно променено име на залата.", NULL);
			else return 500;
		}
		else if (address && address[0] != '\0') {
			result = update_venue_address(db, id, address);
			json_decref(req);
			if (result)
				return send_result(conn, 1, 200, "Успешно променен адрес на залата.", NULL);
			else return 500;
		}
		else {
			return send_result(conn, 0, 400, "Невалидни данни за редактиране на зала.", NULL);
		}
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

int api_venue_seatmap(struct mg_connection* conn, void* data) {
	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "GET") == 0) {
		const char* id_str = info->local_uri + strlen("/api/venues/seatmap/");
		int venue_id = atoi(id_str);

		if (venue_id <= 0) {
			return send_result(conn, 0, 401, "Залата не съществува.", NULL);
		}

		SeatMap* seatMap = NULL;
		if (get_seatmap((PGconn*)data, venue_id, &seatMap, 1)) {
			json_t* res = json_object();
			json_object_set_new(res, "has_sectors", json_boolean(seatMap->has_sectors));
			if (seatMap->has_sectors) {
				json_object_set_new(res, "background_svg",
					seatMap->background_svg ? json_string(seatMap->background_svg) : json_null());
				json_object_set_new(res, "viewbox",
					seatMap->viewbox ? json_string(seatMap->viewbox) : json_null());
				json_t* sectors = json_array();
				for (int i = 0; i < seatMap->sector_count; i++) {
					json_t* sector = json_object();
					json_object_set_new(sector, "id", json_integer(seatMap->sectors[i].id));
					json_object_set_new(sector, "name", json_string(seatMap->sectors[i].name));
					json_object_set_new(sector, "capacity", json_integer(seatMap->sectors[i].capacity));
					json_object_set_new(sector, "color", json_string(seatMap->sectors[i].color));
					json_object_set_new(sector, "svg_path", json_string(seatMap->sectors[i].svg_path));
					json_array_append(sectors, sector);
				}
				json_object_set_new(res, "sectors", sectors);
			}
			else {
				json_object_set_new(res, "no_sector_id", json_integer(seatMap->sectors[0].id));
				json_object_set_new(res, "sectors", json_null());
			}
			return send_result(conn, 1, 200, "", res);
		}
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}
