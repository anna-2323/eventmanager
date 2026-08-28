#pragma once
#include "civetweb.h"
#include "../session.h"
#include "../models/user.h"
#include "../models/event.h"
#include "../models/ticket.h"

#define ROLE_ADMIN       0
#define ROLE_ORGANIZATOR 1
#define ROLE_USER        2

json_t* get_json(struct mg_connection* conn);
int send_json(struct mg_connection* conn, json_t* json);
int check_role(struct mg_connection* conn, int role);
void set_result(json_t* res, int result);

json_t* event_to_json(Event* e);
json_t* ticket_to_json(TicketView* t);
json_t* venue_to_json(Venue* v);
json_t* user_to_json(User* u);
json_t* growth_to_json(StatGrowth* s);
json_t* revenue_to_json(StatRevenue* s);