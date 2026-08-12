#include "venue_api_controller.h"
#include "controller.h"

// GET /api/cities
int api_cities(struct mg_connection* conn, void* data) {
	json_t* res = get_cities((PGconn*)data);
	if (!res)
		return 500;
	return send_json(conn, res);
}

// GET /api/venues
int api_venues(struct mg_connection* conn, void* data) {
	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->local_uri, "/api/venues") == 0) {
		// GET /api/venues
		if (strcmp(info->request_method, "GET") == 0) {
			json_t* res = json_array();
			int count = get_venues(db, res);
			return send_json(conn, res);
		}
	}
	
	const char* id_str = info->local_uri + strlen("/api/venues/");
	char* end;
	long id = strtol(id_str, &end, 10);

	json_t* res = json_object();

	// GET /api/venues/{id}/events
	if (strcmp(end, "/events") == 0) {

		if (strcmp(info->request_method, "GET") != 0) {
			mg_send_http_error(conn, 405, "Method Not Allowed");
			return 405;
		}

		json_t* res = get_events_in_venue(db, id);
		return send_json(conn, res);
	}

	if (*end != '\0') {
		mg_send_http_error(conn, 404, "Not found");
		return 404;
	}

	// GET /api/venues/{id}
	if (strcmp(info->request_method, "GET") == 0) {
		res = get_venue(db, id);
		return send_json(conn, res);
	}
	

	mg_send_http_error(conn, 405, "Method Not Allowed");
	return 405;
}

// POST, PATCH /api/admin/venues
int api_admin_venues(struct mg_connection* conn, void* data) {
	if (check_role(conn, ROLE_USER)) {
		mg_send_http_error(conn, 403, "Forbidden");
		return 403;
	}

	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	// POST /api/venues
	if (strcmp(info->local_uri, "/api/admin/venues") == 0) {
		if (strcmp(info->request_method, "POST") == 0) {
			if (check_role(conn, ROLE_USER)) {
				mg_send_http_error(conn, 403, "Forbidden");
				return 403;
			}
			json_t* req = get_json(conn);
			if (!req) return 400;

			json_t* res = json_object();

			const char* city = json_string_value(json_object_get(req, "city"));
			const char* address = json_string_value(json_object_get(req, "address"));
			const char* venue_name = json_string_value(json_object_get(req, "venue_name"));
			int result = add_venue(db, city, address, venue_name);

			set_result(res, result);
			json_decref(req);
			return send_json(conn, res);
		}
	}
	const char* id_str = info->local_uri + strlen("/api/venues/");
	char* end;
	long id = strtol(id_str, &end, 10);

	json_t* res = json_object();

	// PATCH /api/venues/{id}
	if (strcmp(info->request_method, "PATCH") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;

		int result = 0;

		json_t* active_json = json_object_get(req, "active");
		if (!json_is_boolean(active_json))
			result = 0;
		else {
			if (!check_role(conn, ROLE_ADMIN)) {
				mg_send_http_error(conn, 403, "Forbidden");
				return 403;
			}
			if (json_boolean_value(active_json))
				result = soft_delete_venue(db, id);
			else
				result = restore_venue(db, id);
		}

		const char* venue_name = json_string_value(json_object_get(req, "venue_name"));
		if (venue_name)
			result = update_venue_name(db, id, venue_name);

		set_result(res, result);
		json_decref(req);
		return send_json(conn, res);
	}

}