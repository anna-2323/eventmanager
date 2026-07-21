#include "user.h"

int get_all_users(PGconn* db, json_t* out) {
	if (!db) return NULL;

	char* sql =
		"SELECT id, email, first_name, last_name, phone, role, deleted_on "
		"FROM data.users; ";
	PGresult* res = PQexec(db, sql);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		PQclear(res);
		return 0;
	}

	User u = { 0 };
	int count = PQntuples(res);
	for (int i = 0; i < count; i++) {
		u.id = atoi(PQgetvalue(res, i, 0));
		strncpy(u.email, PQgetvalue(res, i, 1), 100);
		strncpy(u.first_name, PQgetvalue(res, i, 2), 255);
		strncpy(u.last_name, PQgetvalue(res, i, 3), 255);
		strncpy(u.phone, PQgetvalue(res, i, 4), 255);
		u.role = atoi(PQgetvalue(res, i, 5));
		strncpy(u.deleted_on, PQgetvalue(res, i, 6), 255);

		json_array_append_new(out, user_to_json(&u));
	}

	PQclear(res);
	return count;
}

json_t* get_user(PGconn* db, int id) {
	if (!db) return NULL;

	char* sql =
		"SELECT email, first_name, last_name, phone, role, deleted_on "
		"FROM data.users "
		"WHERE id = $1; ";
	char* id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	User u = { 0 };
	u.id = id;
	strncpy(u.email, PQgetvalue(res, 0, 0), 255);
	strncpy(u.first_name, PQgetvalue(res, 0, 1), 255);
	strncpy(u.last_name, PQgetvalue(res, 0, 2), 255);
	strncpy(u.phone, PQgetvalue(res, 0, 3), 255);
	u.role = atoi(PQgetvalue(res, 0, 4));
	strncpy(u.deleted_on, PQgetvalue(res, 0, 5), 255);

	PQclear(res);
	return user_to_json(&u);
}

// Използва се при вход на потребител
int verify_user(PGconn* db, const char* email, const char* password, User* out) {
	if (!db) return NULL;

	// TODO: хеширани пароли
	const char* check_params[2] = { email, password };
	PGresult* res = PQexecParams(db,
		"SELECT id, email, first_name, last_name, role, deleted_on "
		"FROM data.users "
		"WHERE email = $1 AND password = $2; ",
		2, NULL, check_params, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
		fprintf(stderr, "Грешка при изпъляване на заявка: %s\n", PQerrorMessage(db));
		PQclear(res);
		return 0;
	}

	out->id = atoi(PQgetvalue(res, 0, 0));
	strncpy(out->email, PQgetvalue(res, 0, 1), sizeof(out->email) - 1);
	strncpy(out->first_name, PQgetvalue(res, 0, 2), sizeof(out->first_name) - 1);
	strncpy(out->last_name, PQgetvalue(res, 0, 3), sizeof(out->last_name) - 1);
	out->role = atoi(PQgetvalue(res, 0, 4));
	strncpy(out->deleted_on, PQgetvalue(res, 0, 5), sizeof(out->deleted_on) - 1);
	
	PQclear(res);
	return 1;
}

// Използва се ако потребителят е влязъл за редактиране на данни
// 1 - успех, -1 - грешна парола, 0 - друга грешка
int verify_password(PGconn* db, int user_id, const char* password) {
	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { id_str, password };

	PGresult* res = PQexecParams(db,
		"SELECT id FROM data.users WHERE id = $1 AND password = $2",
		2, NULL, params, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка при SELECT: %s\n", PQerrorMessage(db));
		PQclear(res);
		return 0;
	}
	if (PQntuples(res) > 0) {
		PQclear(res);
		return 1;
	}
	else {
		PQclear(res);
		return -1;
	}
}

json_t* add_user(PGconn* db, const char* fname, const char* lname,
	const char* email, const char* phone,
	const char* password, int role) {
	if (!db) return NULL;

	// Съществува ли вече регистрация с този имейл
	const char* check_params[1] = { email };
	PGresult* res = PQexecParams(db,
		"SELECT id FROM data.users WHERE email = $1",
		1, NULL, check_params, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка при изпъляване на заявка: %s\n", PQerrorMessage(db));
		PQclear(res);
		return NULL;
	}
	if (PQntuples(res) > 0) {
		PQclear(res);
		return NULL;
	}
	PQclear(res);
	
	char role_str[8];
	snprintf(role_str, sizeof(role_str), "%d", role);

	const char* ins_params[6] = { fname, lname, email, phone, password, role_str };
	// TODO: хеширана парола
	res = PQexecParams(db,
		"INSERT INTO data.users (first_name, last_name, email, phone, password, role) "
		"VALUES ($1, $2, $3, NULLIF($4, ''), $5, $6) "
		"RETURNING id, first_name, last_name, email, phone, role",
		6, NULL, ins_params, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "Грешка при добавянето на нова регистрация: %s\n", PQerrorMessage(db));
		PQclear(res);
		return NULL;
	}

	User u;
	u.id = atoi(PQgetvalue(res, 0, 0));
	strncpy(u.first_name, PQgetvalue(res, 0, 1), sizeof(u.first_name) - 1);
	strncpy(u.last_name, PQgetvalue(res, 0, 2), sizeof(u.last_name) - 1);
	strncpy(u.email, PQgetvalue(res, 0, 3), sizeof(u.email) - 1);
	strncpy(u.phone, PQgetvalue(res, 0, 4), sizeof(u.phone) - 1);
	u.role = atoi(PQgetvalue(res, 0, 5));

	PQclear(res);
	return user_to_json(&u);
}

int update_user(PGconn* db, const char* sql, int user_id, const char* password, const char* param) {
	int verified = verify_password(db, user_id, password);
	if (verified <= 0) return verified;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { param, id_str };

	PGresult* res = PQexecParams(db, sql, 2, NULL, params, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "Грешка при UPDATE: %s\n", PQerrorMessage(db));
		PQclear(res);
		return 0;
	}
	PQclear(res);
	return 1;
}

int update_password(PGconn* db, int user_id, const char* current_password,
	const char* new_password) {
	char sql[255] = "UPDATE data.users SET password = $1 WHERE id = $2";
	return update_user(db, sql, user_id, current_password, new_password);;
}

int update_email(PGconn* db, int user_id, const char* password,
	const char* email) {
	char sql[255] = "UPDATE data.users SET email = $1 WHERE id = $2";
	return update_user(db, sql, user_id, password, email);
}

int update_phone(PGconn* db, int user_id, const char* password,
	const char* phone) {
	char sql[255] = "UPDATE data.users SET phone = $1 WHERE id = $2";
	return update_user(db, sql, user_id, password, phone);
}

json_t* user_to_json(User* u) {
	json_t* obj = json_object();
	json_object_set_new(obj, "id", json_integer(u->id));
	json_object_set_new(obj, "email", json_string(u->email));
	json_object_set_new(obj, "first_name", json_string(u->first_name));
	json_object_set_new(obj, "last_name", json_string(u->last_name));
	json_object_set_new(obj, "phone", json_string(u->phone));
	json_object_set_new(obj, "role", json_integer(u->role));
	return obj;
}
