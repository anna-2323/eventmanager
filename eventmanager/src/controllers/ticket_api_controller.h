#pragma once
#include "controller.h"
#include "../models/ticket.h"

int api_purchase_ticket(struct mg_connection* conn, void* data);
int api_confirm_ticket(struct mg_connection* conn, void* data);
int api_admin_tickets(struct mg_connection* conn, void* data);
int api_user_tickets(struct mg_connection* conn, void* data);
int api_tickets(struct mg_connection* conn, void* data);
int api_ticket_file(struct mg_connection* conn, void* data);
