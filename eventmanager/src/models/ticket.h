#pragma once
#include <libpq-fe.h>
#include <jansson.h>

int  purchase_ticket(PGconn* db, int event_id, const char* first_name, const char* last_name, const char* email, const char* phone, int* ticket_id_out);
json_t* confirm_ticket(PGconn* db, int ticket_id);
