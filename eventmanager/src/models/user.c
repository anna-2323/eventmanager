#include "user.h"
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#include "../util.h"
#include <openssl/rand.h>
#include <openssl/evp.h>

static int hash_password(const char* pass, const unsigned char* salt, size_t salt_len, const unsigned char* hash, size_t hash_len) {
	return PKCS5_PBKDF2_HMAC(
		pass,
		strlen(pass),
		salt,
		salt_len,
		100000, // итерации
		EVP_sha256(),
		hash_len,
		hash
	);
}

static int check_password(
	const unsigned char* stored_hash,
	const unsigned char* salt,
	size_t hash_len,
	size_t salt_len,
	const char* password) {
	unsigned char computed_hash[32];
	hash_password(password, salt, salt_len, computed_hash, hash_len);
	// CRYPTO_memcp вместо memcp за защита на хешираната парола 
	int result = (hash_len == sizeof(computed_hash) &&
		CRYPTO_memcmp(stored_hash, computed_hash, hash_len) == 0);
	return result;
}

static void user_from_query(PGresult * res, User * u, int i) {
	u->id = atoi(PQgetvalue(res, i, 0));
	snprintf(u->email, sizeof(u->email), "%s", PQgetvalue(res, i, 1));
	snprintf(u->first_name, sizeof(u->first_name), "%s", PQgetvalue(res, i, 2));
	snprintf(u->last_name, sizeof(u->last_name), "%s", PQgetvalue(res, i, 3));
	snprintf(u->phone, sizeof(u->phone), "%s", PQgetvalue(res, i, 4));
	u->role = atoi(PQgetvalue(res, i, 5));
	snprintf(u->deleted_on, sizeof(u->deleted_on), "%s", PQgetvalue(res, i, 6));
}

int get_all_users(PGconn* db, User** out) {
	CHECK_DB(db, 0);

	PGresult* res = PQexecPrepared(db, "get_users", 0, NULL, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);

	int count = PQntuples(res);
	*out = malloc(count * sizeof(User));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}

	for (int i = 0; i < count; i++) {
		user_from_query(res, &(*out)[i], i);
	}

	PQclear(res);
	return count;
}

int get_user(PGconn* db, int id, User* out) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "get_user_by_id", 1, params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}

	user_from_query(res, out, 0);

	PQclear(res);
	return 1;
}

// Използва се при вход на потребител
int verify_user(PGconn* db, const char* email, const char* password, User* out) {
	CHECK_DB(db, 0);

	// Първо се прави проверка по имейл
	const char* check_params[1] = { email };

	PGresult* res = PQexecPrepared(db, "verify_email", 1, check_params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) == 0) {
		PQclear(res);
		return 0;
	}
	int id = atoi(PQgetvalue(res, 0, 0));
	PQclear(res);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", id);
	const char* params[1] = { id_str };

	// След това - проверка по парола
	res = PQexecPrepared(db, "get_user_password", 1, params, NULL, NULL, 1);
	unsigned const char* stored_hash = (const char*)PQgetvalue(res, 0, 0);
	unsigned const char* salt = (const char*)PQgetvalue(res, 0, 1);
	int hash_len = PQgetlength(res, 0, 0);
	int salt_len = PQgetlength(res, 0, 1);
	int result = check_password(stored_hash, salt, hash_len, salt_len, password);
	PQclear(res);

	if (result == 1) {
		res = PQexecPrepared(db, "get_user_by_id", 1, params, NULL, NULL, 0);
		out->id = id;
		strncpy(out->email, PQgetvalue(res, 0, 1), sizeof(out->email) - 1);
		strncpy(out->first_name, PQgetvalue(res, 0, 2), sizeof(out->first_name) - 1);
		strncpy(out->last_name, PQgetvalue(res, 0, 3), sizeof(out->last_name) - 1);
		strncpy(out->phone, PQgetvalue(res, 0, 4), sizeof(out->phone) - 1);
		out->role = atoi(PQgetvalue(res, 0, 5));
		strncpy(out->deleted_on, PQgetvalue(res, 0, 6), sizeof(out->deleted_on) - 1);
		PQclear(res);
	}
	
	return result;
}

// Използва се ако потребителят е влязъл за редактиране на данни
// 1 - успех, -1 - грешна парола, 0 - друга грешка
int verify_password(PGconn* db, int user_id, const char* password) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "verify_password", 1, params, NULL, NULL, 1);
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
	PGresult* res = PQexecPrepared(db, "verify_email", 1, params, NULL, NULL, 0);
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

int add_user(PGconn* db, User* u, const char* password) {
	CHECK_DB(db, 0);

	// Съществува ли вече регистрация с този имейл
	const char* check_params[1] = { u->email };
	PGresult* res = PQexecPrepared(db, "verify_email", 1, check_params, NULL, NULL, 0);
	CHECK_QUERY(res, db, 0);
	if (PQntuples(res) > 0) {
		PQclear(res);
		return 0;
	}
	PQclear(res);
	
	char role_str[8];
	snprintf(role_str, sizeof(role_str), "%d", u->role);

	unsigned char salt[16];
	unsigned char hash[32];
	RAND_bytes(salt, sizeof(salt));
	hash_password(password, salt, 16, hash, 32);

	const char* ins_params[7] = { u->first_name, u->last_name, u->email, u->phone, hash, salt, role_str };
	int lengths[7] = { 0, 0, 0, 0, 32, 16, 0 };
	int formats[7] = { 0, 0, 0, 0, 1, 1, 0 };

	res = PQexecPrepared(db, "add_user", 7, ins_params, lengths, formats, 0);
	CHECK_DB(db, 0);

	int id = atoi(PQgetvalue(res, 0, 0));
	
	PQclear(res);
	return id;
}

int admin_update_email(PGconn* db, int user_id, const char* email) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { email, id_str };

	PGresult* res = PQexecPrepared(db, "update_email", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_phone(PGconn* db, int user_id, const char* phone) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { phone, id_str };

	PGresult* res = PQexecPrepared(db, "update_phone", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_role(PGconn* db, int user_id, const char* role) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { role, id_str };

	PGresult* res = PQexecPrepared(db, "update_role", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int admin_update_name(PGconn* db, int user_id, const char* first_name, const char* last_name) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[3] = { first_name, last_name, id_str };

	PGresult* res = PQexecPrepared(db, "update_names", 3, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int update_password(PGconn* db, int user_id, const char* current_password,
	const char* new_password) {
	int verified = verify_password(db, user_id, current_password);
	if (!verified)
		return verified;
	
	unsigned char salt[16];
	unsigned char hash[32];
	RAND_bytes(salt, sizeof(salt));
	hash_password(new_password, salt, 16, hash, 32);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[3] = { hash, salt, id_str };
	int lenghts[3] = { 32, 16, 0 };
	// 1 = двоична стойност
	int formats[3] = { 1, 1, 0 };

	PGresult* res = PQexecPrepared(db, "update_password", 3, params, lenghts, formats, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);

	return 1;
}

int update_email(PGconn* db, int user_id,
	const char* password, const char* email) {
	int verified = verify_password(db, user_id, password);
	if (!verified) return verified;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { email, id_str };

	PGresult* res = PQexecPrepared(db, "update_email", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int update_phone(PGconn* db, int user_id,
	const char* password, const char* phone) {
	int verified = verify_password(db, user_id, password);
	if (!verified) return verified;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[2] = { phone, id_str };

	PGresult* res = PQexecPrepared(db, "update_phone", 2, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int soft_delete_user(PGconn* db, int user_id, const char* password) {
	CHECK_DB(db, 0);

	int verified = verify_password(db, user_id, password);
	if (verified != 1)
		return 0;

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "soft_delete_user", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int deactivate_user(PGconn* db, int user_id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "deactivate_user", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int activate_user(PGconn* db, int user_id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "activate_user", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

int delete_user(PGconn* db, int user_id) {
	CHECK_DB(db, 0);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[1] = { id_str };

	PGresult* res = PQexecPrepared(db, "permanent_delete_user", 1, params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);

	PQclear(res);
	return 1;
}

void permanent_delete_users(PGconn* db) {
	CHECK_DB(db, NULL);

	PGresult* res = PQexecPrepared(db, "permanent_delete_users", 0, NULL, NULL, NULL, 0);
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

	PGresult* res = PQexecPrepared(db, "create_reset_token", 2, params, NULL, NULL, 0);

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

	PGresult* res = PQexecPrepared(db, "validate_token", 1, params, NULL, NULL, 0);
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
	hash_password(new_password, salt, 16, hash, 32);

	char id_str[16];
	snprintf(id_str, sizeof(id_str), "%d", user_id);
	const char* params[3] = { hash, salt, id_str };
	int lengths[3] = { 32, 16, 0 };
	int formats[3] = { 1, 1, 0 };

	PGresult* res = PQexecPrepared(db, "update_password", 3, params, lengths, formats, 0);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		PQclear(res); PQexec(db, "ROLLBACK"); return 0;
	}
	PQclear(res);

	// Изтриване на токен
	const char* del_params[1] = { token };
	res = PQexecPrepared(db, "delete_reset_token", 1, del_params, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);
	PQclear(res);

	PQexec(db, "COMMIT");
	return 1;
}

int delete_tokens(PGconn* db) {
	CHECK_DB(db, 0);
	PGresult* res = PQexecPrepared(db, "delete_reset_tokens", 0, NULL, NULL, NULL, 0);
	CHECK_COMMAND_QUERY(res, db, 0);
	return 1;
}

int get_total_users(PGconn* db) {
	CHECK_DB(db, NULL);

	PGresult* res = PQexecPrepared(db, "get_total_users", 0, NULL, NULL, NULL, 0);
	CHECK_QUERY(res, db, NULL);

	int total = atoi(PQgetvalue(res, 0, 0));

	return total;
}

int get_users_growth(PGconn* db, int type, StatGrowth** out) {
	CHECK_DB(db, NULL);
	PGresult* res = NULL;
	if (type == 0) {
		res = PQexecPrepared(db, "get_users_growth_monthly", 
			0, NULL, NULL, NULL, 0);
	}
	else if (type == 1) {
		res = PQexecPrepared(db, "get_users_growth_monthly_all", 
			0, NULL, NULL, NULL, 0);
	}
	else if (type == 2) {
		res = PQexecPrepared(db, "get_users_growth_daily", 
			0, NULL, NULL, NULL, 0);
	}

	CHECK_QUERY(res, db, 0);
	int count = PQntuples(res);

	*out = malloc(count * sizeof(StatGrowth));
	if (*out == NULL && count > 0) {
		PQclear(res);
		return 0;
	}

	for (int i = 0; i < count; i++) {
		snprintf((*out)[i].period, sizeof((*out)[i].period), "%s", PQgetvalue(res, i, 0));
		(*out)[i].count = atoi(PQgetvalue(res, i, 1));
	}

	PQclear(res);
	return count;
}

