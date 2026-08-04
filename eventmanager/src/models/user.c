#include "user.h"
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#include "../util.h"
#include <openssl/rand.h>
#include <openssl/evp.h>

int hash_password(const char* pass, const unsigned char* salt, const unsigned char* hash) {
	return PKCS5_PBKDF2_HMAC(
		pass,
		strlen(pass),
		salt,
		sizeof(salt),
		100000, // итерации
		EVP_sha256(),
		sizeof(hash),
		hash
	);
}

int check_password(const char* stored_hash, const char* salt, int hash_len, int salt_len, const char* password) {
	unsigned char computed_hash[32];
	hash_password(password, salt, computed_hash);
	// CRYPTO_memcp вместо memcp за защита на хешираната парола 
	int result = (hash_len == sizeof(computed_hash) &&
					CRYPTO_memcmp(stored_hash, computed_hash, hash_len) == 0);
	return result;
}

int get_all_users(PGconn* db, json_t* out) {
	CHECK_DB(db, 0);

	char* sql =
		"SELECT id, email, first_name, last_name, phone, role, deleted_on "
		"FROM data.users; ";
	PGresult* res = PQexec(db, sql);
	CHECK_QUERY(res, db, 0);

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
	CHECK_DB(db, NULL);

	char* sql =
		"SELECT email, first_name, last_name, phone, role, deleted_on "
		"FROM data.users "
		"WHERE id = $1; ";
	char* id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };
	PGresult* res = PQexecParams(db, sql, 1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);
	if (PQntuples(res) == 0) {
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
	CHECK_DB(db, 0);

	// Първо се прави проверка по имейл
	const char* check_params[1] = { email };
	PGresult* res = PQexecParams(db,
		"SELECT id, email, first_name, last_name, phone, role, deleted_on "
		"FROM data.users "
		"WHERE email = $1; ",
		1, NULL, check_params, NULL, NULL, 0);

	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	// След това - проверка по парола
	unsigned const char* stored_hash = (const char*)PQgetvalue(res, 0, 0);
	unsigned const char* salt = (const char*)PQgetvalue(res, 0, 1);
	int hash_len = PQgetlength(res, 0, 0);
	int salt_len = PQgetlength(res, 0, 1);
	int result = check_password(stored_hash, salt, hash_len, salt_len, password);

	if (result == 0) {
		out->id = atoi(PQgetvalue(res, 0, 0));
		strncpy(out->email, PQgetvalue(res, 0, 1), sizeof(out->email) - 1);
		strncpy(out->first_name, PQgetvalue(res, 0, 2), sizeof(out->first_name) - 1);
		strncpy(out->last_name, PQgetvalue(res, 0, 3), sizeof(out->last_name) - 1);
		strncpy(out->phone, PQgetvalue(res, 0, 4), sizeof(out->phone) - 1);
		out->role = atoi(PQgetvalue(res, 0, 5));
		strncpy(out->deleted_on, PQgetvalue(res, 0, 6), sizeof(out->deleted_on) - 1);
	}
	
	PQclear(res);
	return 1;
}

// Използва се ако потребителят е влязъл за редактиране на данни
// 1 - успех, -1 - грешна парола, 0 - друга грешка
int verify_password(PGconn* db, int user_id, const char* password) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecParams(db,
		"SELECT password_hash, salt FROM data.users WHERE id = $1",
		1, NULL, params, NULL, NULL, 1);
	CHECK_QUERY(res, db, 0);

	if (PQntuples(res) > 0) {
		size_t hash_len = PQgetlength(res, 0, 0);
		size_t salt_len = PQgetlength(res, 0, 1);
		unsigned const char* stored_hash = (const unsigned char*)PQgetvalue(res, 0, 0);
		unsigned const char* salt = (const unsigned char*)PQgetvalue(res, 0, 1);
		int result = check_password(stored_hash, salt, hash_len, salt_len, password);

		PQclear(res);
		return result ? 1 : -1;
	}
	else {
		PQclear(res);
		return 0;
	}
}

// Използва се ако потребителят иска да възстанови парола
// id - успех, -1 - грешна парола, 0 - друга грешка
int verify_email(PGconn* db, const char* email) {
	CHECK_DB(db, 0);

	const char* params[1] = { email };
	PGresult* res = PQexecParams(db,
		"SELECT id FROM data.users WHERE email = $1",
		1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	if (PQntuples(res) > 0) {
		int id = atoi(PQgetvalue(res, 0, 0));
		PQclear(res);
		return id;
	}
	else {
		PQclear(res);
		return -1;
	}
}

json_t* add_user(PGconn* db, const char* fname, const char* lname,
	const char* email, const char* phone,
	const char* password, int role) {
	CHECK_DB(db, NULL);

	// Съществува ли вече регистрация с този имейл
	const char* check_params[1] = { email };
	PGresult* res = PQexecParams(db,
		"SELECT id FROM data.users WHERE email = $1",
		1, NULL, check_params, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);

	if (PQntuples(res) > 0) {
		PQclear(res);
		return NULL;
	}
	PQclear(res);
	
	char role_str[8];
	snprintf(role_str, sizeof(role_str), "%d", role);

	unsigned char salt[16];
	unsigned char hash[32];
	RAND_bytes(salt, sizeof(salt));
	hash_password(password, salt, hash);

	const char* ins_params[7] = { fname, lname, email, phone, hash, salt, role_str };
	int lengths[7] = { 0, 0, 0, 0, 32, 16, 0 };
	int formats[7] = { 0, 0, 0, 0, 1, 1, 0 };
	
	res = PQexecParams(db,
		"INSERT INTO data.users (first_name, last_name, email, phone, password_hash, salt, role) "
		"VALUES ($1, $2, $3, NULLIF($4, ''), $5, $6, $7) "
		"RETURNING id, first_name, last_name, email, phone, role",
		7, NULL, ins_params, lengths, formats, 0);

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
	CHECK_DB(db, 0);

	int verified = verify_password(db, user_id, password);
	if (verified <= 0) return verified;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { param, id_str };

	PGresult* res = PQexecParams(db, sql, 2, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

// Не може да се използва общия update_user в случай на паролата
int update_password(PGconn* db, int user_id, const char* current_password,
	const char* new_password) {
	int verified = verify_password(db, user_id, current_password);
	if (!verified)
		return verified;
	
	unsigned char salt[16];
	unsigned char hash[32];
	RAND_bytes(salt, sizeof(salt));
	hash_password(new_password, salt, hash);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[3] = { hash, salt, id_str };
	int lenghts[3] = { 32, 16, 0 };
	int formats[3] = { 1, 1, 0 };

	char sql[255] = "UPDATE data.users SET password_hash = $1, salt = $2 WHERE id = $3";
	// 1 = двоична стойност
	PGresult* res = PQexecParams(db, sql, 3, NULL, params, lenghts, formats, 0);
	CHECK_QUERY(res, db, 0);

	PQclear(res);
	return 1;
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

int soft_delete_user(PGconn* db, int user_id, const char* password) {
	CHECK_DB(db, 0);

	int verified = verify_password(db, user_id, password);
	if (verified != 1)
		return 0;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecParams(db,
		"UPDATE data.users SET deleted_on = NOW() WHERE id = $1",
		1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

void permanent_delete_users(PGconn* db) {
	CHECK_DB(db, NULL);

	PGresult* res = PQexec(db,
		"DELETE FROM data.users "
		"WHERE deleted_on IS NOT NULL "
		"AND deleted_on < NOW() - INTERVAL '30 days'");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
		fprintf(stderr, "Грешка при окончателно изтриване: %s\n", PQerrorMessage(db));
	else
		fprintf(stderr, "Окончателно изтрити %s акаунти\n", PQcmdTuples(res));
	PQclear(res);
}

char* create_reset_token(PGconn* db, int user_id) {
	CHECK_DB(db, NULL);

	char* token = malloc(65);
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	// Осигурява случайно генериране на токени
	unsigned char randomBytes[64];
	if (BCryptGenRandom(NULL, randomBytes, sizeof(randomBytes),
		BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
		return 1;
	}

	for (int i = 0; i < 64; i++) {
		token[i] = charset[randomBytes[i] % (sizeof(charset) - 1)];
	}

	token[64] = '\0';

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { token, id_str };
	PGresult* res = PQexecParams(db,
		"INSERT INTO data.password_resets (token, user_id) VALUES ($1, $2)",
		2, NULL, params, NULL, NULL, 0);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		PQclear(res);
		free(token);
		return NULL;
	}
	PQclear(res);
	return token;
}

int validate_reset_token(PGconn* db, const char* token) {
	CHECK_DB(db, 0);

	const char* params[1] = { token };
	PGresult* res = PQexecParams(db,
		"SELECT user_id FROM data.password_resets "
		"WHERE token = $1 AND expires_at > NOW()",
		1, NULL, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	int user_id = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);
	return user_id;
}

int reset_password(PGconn* db, const char* token, const char* new_password) {
	CHECK_DB(db, 0);

	int user_id = validate_reset_token(db, token);
	if (user_id < 0) return -1;

	PQexec(db, "BEGIN");

	// Смяна на парола
	unsigned char salt[16];
	unsigned char hash[32];
	RAND_bytes(salt, sizeof(salt));
	hash_password(new_password, salt, hash);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[3] = { hash, salt, id_str };
	int lengths[3] = { 32, 16, 0 };
	int formats[3] = { 1, 1, 0 };

	PGresult* res = PQexecParams(db,
		"UPDATE data.users SET password_hash = $1, salt = $2 WHERE id = $3",
		3, NULL, params, lengths, formats, 0);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		PQclear(res); PQexec(db, "ROLLBACK"); return 0;
	}
	PQclear(res);

	// Изтриване на токен
	const char* del_params[1] = { token };
	res = PQexecParams(db,
		"DELETE FROM data.password_resets WHERE token = $1",
		1, NULL, del_params, NULL, NULL, 0);
	PQclear(res);

	PQexec(db, "COMMIT");
	return 1;
}

int delete_tokens(PGconn* db) {
	CHECK_DB(db, 0);
	PQexec(db, "DELETE FROM data.password_resets WHERE expires_at < NOW()");
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
