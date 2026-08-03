#pragma once
#include "controller.h"
#include "../models/ticket.h"

int api_purchase_ticket(struct mg_connection* conn, void* data);
int api_confirm_ticket(struct mg_connection* conn, void* data);
