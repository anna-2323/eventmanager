#pragma once
#include <libpq-fe.h>
#include <jansson.h>
#include "event.h"
#include "user.h"

typedef struct {
    char event_name[256];
    char begins_at[256];
    char venue_name[256];
    char venue_city[256];
    char venue_address[256];
    char first_name[256];
    char last_name[256];
    char email[256];
    char phone[50];
    char sector[256];
    char token[256];
    float price;
} TicketView;

typedef struct {
    int event_id;
    int sector_id;
    int user_id;
    char first_name[256];
    char last_name[256];
    char email[256];
    char phone[50];
} TicketData;

int purchase_ticket(PGconn* db, TicketData* data, int* ticket_id_out);
json_t* get_ticket(PGconn* db, int ticket_id);
int generate_ticket_html(PGconn* db, int ticket_id, const char* qr_path, char* out_path, size_t out_size);

json_t* get_total_tickets(PGconn* db);
json_t* get_tickets_growth(PGconn* db, int type);

json_t* get_revenue(PGconn* db, int type);