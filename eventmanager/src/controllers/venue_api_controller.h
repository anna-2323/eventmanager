#pragma once
#include "controller.h"
#include "../models/venue.h"
#include "../models/event.h"

int api_cities(struct mg_connection* conn, void* data);
int api_venues(struct mg_connection* conn, void* data);
int api_admin_venues(struct mg_connection* conn, void* data);