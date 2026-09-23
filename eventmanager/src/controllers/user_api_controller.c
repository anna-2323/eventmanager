#include "user_api_controller.h"

// Помощни функции
static int handle_email(struct mg_connection* conn, PGconn* db, Session* s, const char* email, const char* password);
static int handle_phone(struct mg_connection* conn, PGconn* db, Session* s, const char* phone, const char* password);
static int handle_password(struct mg_connection* conn, PGconn* db, Session* s, const char* current, const char* next);
static int handle_delete(struct mg_connection* conn, PGconn* db, Session* s, const char* password);

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
			return send_result(conn, 1, 200, "", res);
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

		if (id <= 0)
			return send_result(conn, 0, 400, "Невалидно ID на потребител", NULL);

		if (strcmp(info->request_method, "GET") == 0) {
			User user;
			if(get_user(db, id, &user))
				return send_result(conn, 1, 200, "", user_to_json(&user));
		}
		if (strcmp(info->request_method, "PATCH") == 0) {
			json_t* req = get_json(conn);
			if (!req)
				return 400;

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
					if (result) {
						if (json_boolean_value(active_json)) {
							return send_result(conn, 1, 200, "Акаунтът е успешно активиран.", NULL);
						}
						else {
							return send_result(conn, 1, 200, "Акаунтът е успешно деактивиран.", NULL);
						}
					}
					else
						return 500;
				}
			}
			else if (email && is_valid_email(email)) {
				result = admin_update_email(db, id, email);
				json_decref(req);
				if (result)
					return send_result(conn, 1, 200, "Успешно сменен имейл адрес.", NULL);
				else
					return 500;
			}
			else if (phone && is_valid_phone(phone)) {
				result = admin_update_phone(db, id, phone);
				json_decref(req);
				if (result)
					return send_result(conn, 1, 200, "Успешно сменен телефонен номер.", NULL);
				else
					return 500;
			}
			else if (first_name && last_name) {
				result = admin_update_name(db, id, first_name, last_name);
				json_decref(req);
				if (result)
					return send_result(conn, 1, 200, "Успешно сменени имена.", NULL);
				else
					return 500;
			}
			else if (json_is_integer(role_json)) {
				int role = json_integer_value(role_json);

				char role_str[2];
				snprintf(role_str, sizeof(role_str), "%d", role);

				result = admin_update_role(db, id, role_str);
				if (result)
					return send_result(conn, 1, 200, "Успешно сменена роля.", NULL);
				else
					return 500;
			}
			else {
				return send_result(conn, 0, 400, "Невалидни данни за редактиране на потребител.", NULL);
			}
		}
		if (strcmp(info->request_method, "DELETE") == 0) {
			int result = delete_user(db, id);
			if (result) {
				return send_result(conn, 1, 200, "Успешно изтрит акаунт.", NULL);
			}
			else {
				return 500;
			}
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

	return send_result(conn, 1, 200, "", res);
}

// POST /api/login
int api_login(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		json_t* req = get_json(conn);
		if (!req)
			return 400;

		const char* email = json_string_value(json_object_get(req, "email"));
		const char* password = json_string_value(json_object_get(req, "password"));

		User user = { 0 };
		int ok = verify_user((PGconn*)data, email, password, &user);

		json_t* res = json_object();
		if (ok > 0) {
			// Създава се бисквитка
			Session* s = session_create(&user);

			json_object_set_new(res, "success", json_true());
			json_object_set_new(res, "email", json_string(user.email));
			json_object_set_new(res, "phone", json_string(user.phone));
			json_object_set_new(res, "first_name", json_string(user.first_name));
			json_object_set_new(res, "last_name", json_string(user.last_name));
			json_object_set_new(res, "role", json_integer(user.role));

			char* json = json_dumps(res, JSON_COMPACT);

			mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"
				"Content-Length: %zu\r\n"
				"Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n"
				"\r\n"
				"%s",
				strlen(json),
				s->token,
				json
			);

			free(json);
			json_decref(res);
			json_decref(req);

			return 1;
		}
		else {
			json_decref(req);
			if (ok == 0) {
				return send_result(conn, 0, 400, "Невалидна парола.", res);
			}
			if (ok == -1)
				return 500;
		}
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
		if (!req)
			return 400;

		User u;
		snprintf(u.first_name, sizeof(u.first_name), "%s", json_string_value(json_object_get(req, "first_name")));
		snprintf(u.last_name, sizeof(u.last_name), "%s", json_string_value(json_object_get(req, "last_name")));
		snprintf(u.email, sizeof(u.email), "%s", json_string_value(json_object_get(req, "email")));
		snprintf(u.phone, sizeof(u.phone), "%s", json_string_value(json_object_get(req, "phone")));
		const char* password = json_string_value(json_object_get(req, "password"));
		json_t* role_json = json_object_get(req, "role");
		u.role = role_json ? json_integer_value(role_json) : 2;

		if (!is_valid_password(password)) {
			json_decref(req);
			return send_result(conn, 0, 400, "Паролата трябва да бъде поне 8 символа.", NULL);
		}
		if (!is_valid_email(u.email)) {
			json_decref(req);
			return send_result(conn, 0, 400, "Невалиден имейл адрес.", NULL);
		}
		if (!is_valid_phone(u.phone)) {
			json_decref(req);
			return send_result(conn, 0, 400, "Невалиден телефонен номер.", NULL);
		}
		if (!u.first_name || !u.last_name) {
			json_decref(req);
			return send_result(conn, 0, 400, "Невалидни имена.", NULL);
		}

		u.id = add_user((PGconn*)data, &u, password);
		if (!u.id) {
			json_decref(req);
			return send_result(conn, 0, 400, "Имейлът вече е регистриран.", NULL);
		}

		json_t* json = json_object();
		json_object_set_new(json, "success", json_true());
		const char* res = json_dumps(json, JSON_COMPACT);

		// Създаване на бисквитка за новия потребител
		json_t* user = user_to_json(&u);
		Session* s = session_create(&u);
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: %zu\r\n"
			"Set-Cookie: session=%s; HttpOnly; Path=/; Max-Age=86400\r\n\r\n"
			"%s",
			strlen(res),
			s->token,
			res);

		json_decref(req);
		json_decref(user);
		json_decref(json);
		free(res);
		return 1;
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

		return send_result(conn, 1, 200, "", NULL);
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
		return send_result(conn, 0, 401, "Нямате права за това действие.", NULL);
	}

	PGconn* db = (PGconn*)data;
	const struct mg_request_info* info = mg_get_request_info(conn);

	json_t* req = get_json(conn);
	if (!req)
		return 400;
	int result = 0;

	if (strcmp(info->request_method, "PATCH") == 0) {
		if (json_object_get(req, "email")) {
			const char* email = json_string_value(json_object_get(req, "email"));
			const char* password = json_string_value(json_object_get(req, "password"));
			int result = handle_email(conn, db, s, email, password);
		}
		else if (json_object_get(req, "phone")) {
			const char* phone = json_string_value(json_object_get(req, "phone"));
			const char* password = json_string_value(json_object_get(req, "password"));
			int result = handle_phone(conn, db, s, phone, password);
		}
		else if (json_object_get(req, "new_password")) {
			const char* current = json_string_value(json_object_get(req, "current_password"));
			const char* next = json_string_value(json_object_get(req, "new_password"));
			int result = handle_password(conn, db, s, current, next);
		}
		json_decref(req);
		return result;
	}
	else if (strcmp(info->request_method, "DELETE") == 0) {
		const char* password = json_string_value(json_object_get(req, "password"));
		int result = handle_delete(conn, db, s, password);
		json_decref(req);
		return result;
	}
	else {
		json_decref(req);
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

// POST /api/forgot
int api_forgot(struct mg_connection* conn, void* data) {
	const struct mg_request_info* info = mg_get_request_info(conn);

	if (strcmp(info->request_method, "POST") == 0) {
		json_t* req = get_json(conn);
		if (!req) return 400;
		const char* email = json_string_value(json_object_get(req, "email"));
		if (!email || email[0] == '\0' || !is_valid_email(email)) {
			json_decref(req);
			return send_result(conn, 0, 400, "Моля, въведете имейл адрес", NULL);
		}

		json_t* res = json_object();

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
		return send_result(conn, 1, 200, "Ако имейлът е регистриран, ще получите линк за смяна на паролата.", res);
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
		if (!req)
			return 400;
		const char* token = json_string_value(json_object_get(req, "token"));
		const char* new_password = json_string_value(json_object_get(req, "new_password"));
		if (!is_valid_password(new_password)) {
			json_decref(req);
			return send_result(conn, 0, 400, "Невалидна парола.", NULL);
		}

		PGconn* db = (PGconn*)data;
		json_t* res = json_object();
		int result = reset_password(db, token, new_password);
		json_decref(req);

		if (result == 1)
			return send_result(conn, 1, 200, "", NULL);
		else if (result == -1)
			return send_result(conn, 0, 400, "Линкът е невалиден или изтекъл.", NULL);
		else
			return send_result(conn, 0, 500, "Възникна грешка.", NULL);
	}
	else {
		mg_send_http_error(conn, 405, "Method Not Allowed");
		return 405;
	}
}

static int handle_email(struct mg_connection* conn, PGconn* db, Session* s, 
		const char* email, const char* password) {
	if (is_valid_email(email)) {
		int result = update_email(db, s->user_id, password, email);
		snprintf(s->email, sizeof(s->email), "%s", email);
		if (result) {
			return send_result(conn, 1, 200, "Успешно сменен имейл адрес.", NULL);
		}
		else
			return 500;
	}
	else
		return send_result(conn, 0, 400, "Невалиден имейл адрес.", NULL);
}

static int handle_phone(struct mg_connection* conn, PGconn* db, Session* s, 
		const char* phone, const char* password) {
	if (is_valid_phone(phone)) {
		int result = update_phone(db, s->user_id, password, phone);
		snprintf(s->phone, sizeof(s->phone), "%s", phone);
		if (result) {
			return send_result(conn, 1, 200, "Успешно сменен телефонен номер.", NULL);
		}
		else
			return 500;
	}
	else
		return send_result(conn, 0, 400, "Невалиден телефонен номер.", NULL);
}

static int handle_password(struct mg_connection* conn, PGconn* db, Session* s,
		const char* current, const char* next) {
	if (is_valid_password(next)) {
		int result = update_password(db, s->user_id, current, next);
		if (result) {
			return send_result(conn, 1, 200, "Успешно сменена парола.", NULL);
		}
		else
			return 500;
	}
	else
		return send_result(conn, 0, 400, "Невалидна нова парола.", NULL);
}

static int handle_delete(struct mg_connection* conn, PGconn* db, Session* s,
		const char* password) {
	if (is_valid_password(password)) {
		int result = soft_delete_user(db, s->user_id, password);
		if (result) {
			session_delete(s->token);
			return send_result(conn, 1, 200, "Успешно изтрит акаунт.", NULL);
		}
		else
			return 500;
	}
	else
		return send_result(conn, 0, 400, "Невалидна парола.", NULL);
}