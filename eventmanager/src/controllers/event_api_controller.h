#pragma once
#include "controller.h"
#include "../models/event.h"

int api_events(struct mg_connection* conn, void* data);
int api_user_events(struct mg_connection* conn, void* data);
int api_categories(struct mg_connection* conn, void* data);
int api_event_seatmap(struct mg_connection* conn, void* data);
int api_admin_events(struct mg_connection* conn, void* data);
