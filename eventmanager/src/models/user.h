#pragma once
#include "libpq-fe.h"
#include <jansson.h>

typedef struct {
	int id;
	char email[255];
	char first_name[255];
	char last_name[255];
	char phone[255];
	int role;
	char deleted_on[255];
	int active;
} User;

int get_all_users(PGconn* db, json_t* out);
json_t* get_user(PGconn* db, int id);
json_t* user_to_json(User* u);

int verify_user(PGconn* db, const char* email, const char* password, User* out);
int verify_email(PGconn* db, const char* email);

json_t* add_user(PGconn* db, const char* fname, const char* lname, const char* email, const char* phone, const char* password, int role);

int admin_update_email(PGconn* db, int user_id, const char* email);
int admin_update_phone(PGconn* db, int user_id, const char* phone);
int admin_update_name(PGconn* db, int user_id, const char* first_name, const char* last_name);
int admin_update_role(PGconn* db, int user_id, const char* role);

int deactivate_user(PGconn* db, int user_id);
int activate_user(PGconn* db, int user_id);

int update_email(PGconn* db, int user_id, const char* email, const char* password);
int update_phone(PGconn* db, int user_id, const char* email, const char* password);
int update_password(PGconn* db, int user_id, const char* email, const char* password);

int delete_user(PGconn* db, int user_id);
int soft_delete_user(PGconn* db, int user_id, const char* password);
void permanent_delete_users(PGconn* db);

char* create_reset_token(PGconn* db, int user_id);
int validate_reset_token(PGconn* db, const char* token);
int reset_password(PGconn* db, const char* token, const char* new_password);
int delete_tokens(PGconn* db);
