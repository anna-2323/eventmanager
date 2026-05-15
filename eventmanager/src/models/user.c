#include "user.h"

int get_all_users(PGconn* db, json_t* out) {
	if (!db) return NULL;

	char* sql =
		"SELECT id, email, first_name, last_name, role "
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
		u.role = atoi(PQgetvalue(res, i, 4));

		json_array_append_new(out, user_to_json(u));
	}

	PQclear(res);
	return count;
}

json_t* get_user(PGconn* db, int id) {
	if (!db) return NULL;

	char* sql =
		"SELECT email, first_name, last_name, role "
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
	u.role = atoi(PQgetvalue(res, 0, 3));

	PQclear(res);
	return user_to_json(u);
}

json_t* user_to_json(User u) {
	json_t* obj = json_object();
	json_object_set_new(obj, "id", json_integer(u.id));
	json_object_set_new(obj, "email", json_string(u.email));
	json_object_set_new(obj, "first_name", json_string(u.first_name));
	json_object_set_new(obj, "last_name", json_string(u.last_name));
	json_object_set_new(obj, "role", json_integer(u.role));
	return obj;
}