#pragma once
#include "civetweb.h"
#include "../models/event.h"
#include "../models/user.h"

int api_events(struct mg_connection* conn, void* data);
int api_users(struct mg_connection* conn, void* data);
int api_purchase_ticket(struct mg_connection* conn, void* data);
int api_confirm_ticket(struct mg_connection* conn, void* data);
int api_me(struct mg_connection* conn, void* data);
int api_login(struct mg_connection* conn, void* data);
int api_signup(struct mg_connection* conn, void* data);
int api_logout(struct mg_connection* conn, void* data);