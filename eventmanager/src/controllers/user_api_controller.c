#include "user_api_controller.h"

// Помощни функции
static int missing_fields(json_t* res);
static int handle_email(PGconn* db, Session* s, json_t* req, json_t* res);
static int handle_phone(PGconn* db, Session* s, json_t* req, json_t* res);
static int handle_password(PGconn* db, Session* s, json_t* req, json_t* res);
static int handle_delete(PGconn* db, Session* s, json_t* req, json_t* res);

// GET /api/admin/users
int api_users(struct mg_connection* conn, void* data) {
	if (check_role(conn, ROLE_ADMIN) < 1) {
		mg_send_http_error(conn, 403, "Forbidden");
		return 403;
	}

	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	// /api/admin/users
	if (strcmp(info->local_uri, "/api/admin/users") == 0) {
		if (strcmp(info->request_method, "GET") == 0) {
			User* users;
			int count = get_all_users(db, &users);
			json_t* res = json_array();
			for (int i = 0; i < count; i++) {
				json_array_append(res, user_to_json(&users[i]));
			}
			free(users);
			return send_json(conn, res);
		}
		else {
			mg_send_http_error(conn, 405, "Method Not Allowed");
			return 405;
		}
	}
	// /api/admin/users/{id}
	else {
		const char* id_str = info->local_uri + strlen("/api/admin/users/");
		int id = atoi(id_str);

		if (id <= 0) {
			mg_send_http_error(conn, 404, "Not found");
			return 404;
		}

		if (strcmp(info->request_method, "GET") == 0) {
			User user;
			if(get_user(db, id, &user))
				return send_json(conn, user_to_json(&user));
		}
		if (strcmp(info->request_method, "PATCH") == 0) {
			json_t* req = get_json(conn);
			if (!req) return 400;

			json_t* res = json_object();
			int result = 0;

			json_t* active_json = json_object_get(req, "active");
			const char* email = json_string_value(json_object_get(req, "email"));
			const char* phone = json_string_value(json_object_get(req, "phone"));
			const char* first_name = json_string_value(json_object_get(req, "first_name"));
			const char* last_name = json_string_value(json_object_get(req, "last_name"));
			json_t* role_json = json_object_get(req, "role");

			if (active_json) {
				if (!json_is_boolean(active_json)) {
					result = 0;
				}
				else {
					result = set_user_active(db, id, json_boolean_value(active_json));
				}
			}
			else if (email) {
				result = admin_update_email(db, id, email);
			}
			else if (phone) {
				result = admin_update_phone(db, id, phone);
			}
			else if (first_name || last_name) {
				if (!first_name || !last_name) {
					result = 0;
				}
				else {
					result = admin_update_name(db, id, first_name, last_name);
				}
			}
			else if (json_is_integer(role_json)) {
				int role = json_integer_value(role_json);

				char role_str[2];
				snprintf(role_str, sizeof(role_str), "%d", role);

				result = admin_update_role(db, id, role_str);
			}
			else
				result = 0;

			set_result(res, result);

			json_decref(req);
			return send_json(conn, res);
		}

		if (strcmp(info->request_method, "DELETE") == 0) {

			int result = delete_user(db, id);

			json_t* res = json_object();
			set_result(res, result);

			return send_json(conn, res);
		}

		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// GET /api/me
int api_me(struct mg_connection* conn, void* data) {
	Session* s = get_session(conn);
	json_t* res = json_object();

	// Ако потребителят е влязъл
	if (s) {
		json_object_set_new(res, "logged_in", json_true());
		json_object_set_new(res, "id", json_integer(s->user_id));
		json_object_set_new(res, "email", json_string(s->email));
		json_object_set_new(res, "first_name", json_string(s->first_name));
		json_object_set_new(res, "last_name", json_string(s->last_name));
		json_object_set_new(res, "phone", json_string(s->phone));
		json_object_set_new(res, "role", json_integer(s->role));
	}
	// Ако потребителят не е влязъл
	else {
		json_object_set_new(res, "logged_in", json_false());
	}

	return send_json(conn, res);
}

// POST /api/login
int api_login(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;

		const char* email = json_string_value(json_object_get(req, "email"));
		const char* password = json_string_value(json_object_get(req, "password"));

		User user = { 0 };
		int ok = verify_user((PGconn*)data, email, password, &user);

		json_t* res = json_object();
		if (ok > 0) {
			// Създава се бисквитка
			Session* s = session_create(&user);
			mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"
				"Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n\r\n",
				s->token);

			json_object_set_new(res, "success", json_true());
			json_object_set_new(res, "email", json_string(user.email));
			json_object_set_new(res, "phone", json_string(user.phone));
			json_object_set_new(res, "first_name", json_string(user.first_name));
			json_object_set_new(res, "last_name", json_string(user.last_name));
			json_object_set_new(res, "role", json_integer(user.role));
		}
		else {
			json_object_set_new(res, "success", json_false());
			if(ok == 0)
				json_object_set_new(res, "error", json_string("Невалиден имейл или парола."));
			if(ok == -1)
				json_object_set_new(res, "error", json_string("Възникна грешка."));
		}
		return send_json(conn, res);
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// POST /api/signup
int api_signup(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;

		User u;
		snprintf(u.first_name, sizeof(u.first_name), "%s", json_string_value(json_object_get(req, "first_name")));
		snprintf(u.last_name, sizeof(u.last_name), "%s", json_string_value(json_object_get(req, "last_name")));
		snprintf(u.email, sizeof(u.email), "%s", json_string_value(json_object_get(req, "email")));
		snprintf(u.phone, sizeof(u.phone), "%s", json_string_value(json_object_get(req, "phone")));
		if (!u.phone) snprintf(u.phone, sizeof(u.phone), "%s", "");
		const char* password = json_string_value(json_object_get(req, "password"));
		json_t* role_json = json_object_get(req, "role");
		u.role = role_json ? json_integer_value(role_json) : 0;
		json_decref(req);

		if (!u.first_name || !u.last_name || !u.email || !u.phone || !password) {
			json_decref(req);
			return 400;
		}

		json_t* res = json_object();
		u.id = add_user((PGconn*)data, &u, password);
		if (!u.id) {
			json_object_set_new(res, "success", json_false());
			json_object_set_new(res, "error", json_string("Имейлът вече е регистриран."));
			return send_json(conn, res);
		}

		// Създаване на бисквитка за новия потребител
		json_t* user = user_to_json(&u);
		Session* s = session_create(&u);
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n\r\n",
			s->token);

		json_object_set_new(res, "success", json_true());
		json_decref(user);
		return send_json(conn, res);
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// POST /api/logout
int api_logout(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		Session* s = get_session(conn);
		if (s) session_delete(s->token);

		// Max-Age = 0
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Set-Cookie: session=; HttpOnly; Path=/; Max-Age=0\r\n\r\n"
			"{\"success\":true}");

		return 1;
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// PATCH/DELETE /api/profile
int api_profile(struct mg_connection* conn, void* data) {
	Session* s = get_session(conn);
	if (!s) { 
		mg_send_http_error(conn, 401, "Unauthorized");
		return 401;
	}

	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	json_t* req = get_json(conn);
	if (!req) return 400;

	json_t* res = json_object();
	int result = 0;

	if (strcmp(info->request_method, "PATCH") == 0) {
		if (json_object_get(req, "email"))
			result = handle_email(db, s, req, res);
		else if (json_object_get(req, "phone"))
			result = handle_phone(db, s, req, res);
		else if (json_object_get(req, "new_password"))
			result = handle_password(db, s, req, res);
	}
	else if (strcmp(info->request_method, "DELETE") == 0)
		result = handle_delete(db, s, req, res);
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}

	json_decref(req);

	if (result)
		return send_json(conn, res);
	mg_send_http_error(conn, 404, "Not found");
	return 404;
}

// POST /api/forgot
int api_forgot(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "PATCH") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;
		const char* email = json_string_value(json_object_get(req, "email"));

		json_t* res = json_object();
		json_object_set_new(res, "success", json_true());
		json_object_set_new(res, "message", json_string("Ако имейлът е регистриран, ще получите линк за смяна на паролата."));

		int user_id = verify_email((PGconn*)data, email);
		if (user_id > 0) {
			char* token = create_reset_token((PGconn*)data, user_id);
			if (token) {
				char link[512];
				snprintf(link, sizeof(link),
					"<p>Може да смените паролата си като <a href='http://localhost:8080/reset?token=%s'>натиснете тук.</а></p>", token);
				send_forgot_email(email, "Забравена парола", link);
				free(token);
			}
		}
		json_decref(req);
		return send_json(conn, res);
		return 1;
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// POST /api/reset
int api_reset_password(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;
		const char* token = json_string_value(json_object_get(req, "token"));
		const char* new_password = json_string_value(json_object_get(req, "new_password"));

		PGconn* db = (PGconn*)data;
		json_t* res = json_object();
		int result = reset_password(db, token, new_password);

		if (result == 1) {
			json_object_set_new(res, "success", json_true());
		}
		else if (result == -1) {
			json_object_set_new(res, "success", json_false());
			json_object_set_new(res, "error",
				json_string("Линкът е невалиден или изтекъл."));
		}
		else {
			json_object_set_new(res, "success", json_false());
			json_object_set_new(res, "error", json_string("Възникна грешка."));
		}

		json_decref(req);
		return send_json(conn, res);
		return 1;
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

static int missing_fields(json_t* res)
{
	json_object_set_new(res, "success", json_false());
	json_object_set_new(res, "error", json_string("Липсват полета."));
	return 0;
}

static int handle_email(PGconn* db, Session* s, json_t* req, json_t* res) {
	const char* email = json_string_value(json_object_get(req, "email"));
	const char* password = json_string_value(json_object_get(req, "password"));
	if (!email || !password) {
		missing_fields(res);
	}
	int result = update_email(db, s->user_id, password, email);
	snprintf(s->email, sizeof(s->email), "%s", email);
	set_result(res, result);
	return result;
}

static int handle_phone(PGconn* db, Session* s, json_t* req, json_t* res) {
	const char* phone = json_string_value(json_object_get(req, "phone"));
	const char* password = json_string_value(json_object_get(req, "password"));
	if (!phone || !password) {
		missing_fields(res);
	}
	int result = update_phone(db, s->user_id, password, phone);
	snprintf(s->phone, sizeof(s->phone), "%s", phone);
	set_result(res, result);
	return result;
}

static int handle_password(PGconn* db, Session* s, json_t* req, json_t* res) {
	const char* current = json_string_value(json_object_get(req, "current_password"));
	const char* next = json_string_value(json_object_get(req, "new_password"));
	if (!current || !next) {
		missing_fields(res);
	}
	int result = update_password(db, s->user_id, current, next);
	set_result(res, result);
	return result;
}

static int handle_delete(PGconn* db, Session* s, json_t* req, json_t* res) {
	const char* password = json_string_value(json_object_get(req, "password"));
	if (!password) {
		missing_fields(res);
	}
	int result = soft_delete_user(db, s->user_id, password);
	if (result == 1) {
		session_delete(s->token);
		json_object_set_new(res, "success", json_true());
	}
	set_result(res, result);
	return result;
}