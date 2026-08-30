#pragma once
#include "controller.h"
#include "../models/ticket.h"

int api_purchase_ticket(struct mg_connection* conn, void* data);
int api_confirm_ticket(struct mg_connection* conn, void* data);
int api_my_tickets(struct mg_connection* conn, void* data);
int api_ticket_file(struct mg_connection* conn, void* data);
