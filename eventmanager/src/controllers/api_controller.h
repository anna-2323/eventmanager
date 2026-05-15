#pragma once
#include "civetweb.h"
#include "../models/event.h"
#include "../models/user.h"

int api_events(struct mg_connection* conn, void* data);
int api_users(struct mg_connection* conn, void* data);