#pragma once
#include "civetweb.h"
#include "../session.h"
#include "../models/user.h"

#define ROLE_ADMIN       0
#define ROLE_ORGANIZATOR 1
#define ROLE_USER        2

json_t* get_json(struct mg_connection* conn);
int send_json(struct mg_connection* conn, json_t* json);
int check_role(struct mg_connection* conn, int role);
