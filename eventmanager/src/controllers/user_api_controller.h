#pragma once
#include "controller.h"
#include "../models/user.h"
#include "../models/event.h"

int api_users(struct mg_connection* conn, void* data);

int api_me(struct mg_connection* conn, void* data);
int api_login(struct mg_connection* conn, void* data);
int api_signup(struct mg_connection* conn, void* data);
int api_logout(struct mg_connection* conn, void* data);

int api_profile(struct mg_connection* conn, void* cbdata);

int api_forgot(struct mg_connection* conn, void* data);
int api_reset_password(struct mg_connection* conn, void* cbdata);
