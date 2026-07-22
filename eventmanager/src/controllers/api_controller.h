#pragma once
#include "civetweb.h"
#include "../models/event.h"
#include "../models/user.h"
#include "../session.h"

int api_events(struct mg_connection* conn, void* data);
int api_users(struct mg_connection* conn, void* data);
int api_purchase_ticket(struct mg_connection* conn, void* data);

// auth
int api_confirm_ticket(struct mg_connection* conn, void* data);
int api_me(struct mg_connection* conn, void* data);
int api_login(struct mg_connection* conn, void* data);
int api_signup(struct mg_connection* conn, void* data);
int api_logout(struct mg_connection* conn, void* data);

// profile
int controller_api_profile(struct mg_connection* conn, void* cbdata);
typedef int (*profile_handler_fn)(PGconn* db, Session* s, json_t* req, json_t* res);
int handle_profile_request(struct mg_connection* conn, PGconn* db, profile_handler_fn handler);

int api_forgot(struct mg_connection* conn, void* data);
int api_reset_password(struct mg_connection* conn, void* cbdata);