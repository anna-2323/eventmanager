#pragma once
#include "civetweb.h"

int home(struct mg_connection* conn, void* data);
int events(struct mg_connection* conn, void* data);
int event(struct mg_connection* conn, void* data);