#pragma once
#include <libpq-fe.h>
#include <jansson.h>
#include "event.h"
#include "user.h"

typedef struct {
    int event_id;
    int user_id;
    int sector_id;
    char first_name[255];
    char last_name[255];
    char email[255];
    char phone[255];
} TicketData;

int purchase_ticket(PGconn* db, TicketData* data, int* ticket_id_out);
json_t* get_ticket(PGconn* db, int ticket_id);
int generate_ticket_html(PGconn* db, int ticket_id, const char* out_path);
