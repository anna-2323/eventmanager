#pragma once
#include "controller.h"
#include "../models/event.h"

int api_events(struct mg_connection* conn, void* data);
int api_event_layout(struct mg_connection* conn, void* data);
