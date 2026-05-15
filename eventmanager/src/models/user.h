#pragma once
#include "libpq-fe.h"
#include <jansson.h>

typedef struct {
	int id;
	char email[255];
	char first_name[255];
	char last_name[255];
	int role;
} User;

int get_all_users(PGconn* db, json_t* out);
json_t* get_user(PGconn* db, int id);
json_t* user_to_json(User u);
