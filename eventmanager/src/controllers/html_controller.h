#pragma once
#include "civetweb.h"

int home(struct mg_connection* conn, void* data);
int events(struct mg_connection* conn, void* data);
int purchase(struct mg_connection* conn, void* data);
int confirmation(struct mg_connection* conn, void* data);
int login(struct mg_connection* conn, void* data);
int signup(struct mg_connection* conn, void* data);
int profile(struct mg_connection* conn, void* data);
int forgot(struct mg_connection* conn, void* data);
int reset(struct mg_connection* conn, void* data);
