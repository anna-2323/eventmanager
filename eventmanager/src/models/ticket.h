#pragma once
#include <libpq-fe.h>
#include <jansson.h>
#include "event.h"
#include "user.h"

typedef struct {
    int id;
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
    int user_id;
    int event_id;
    int active;
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

typedef struct {
    char period[11];  // "YYYY-MM-DD"
    double revenue;
    int count;
    int venue_id;
} StatRevenue;

int purchase_ticket(PGconn* db, TicketData* data, int* ticket_id_out);
int get_ticket(PGconn* db, int ticket_id, TicketView* out);
int get_tickets(PGconn* db, int organizer_id, TicketView** out);
int get_user_tickets(PGconn* db, int user_id, TicketView** out);
int ticket_uuid_belongs_to_user(PGconn* db, int user_id, const char* ticket_uuid);
int ticket_id_belongs_to_user(PGconn* db, int user_id, int ticket_id);

int set_ticket_active(PGconn* db, int id, int active);

int get_total_tickets(PGconn* db, int organizer_id);
int get_tickets_growth(PGconn* db, int type, int organizer_id, StatGrowth** out);

int get_revenue(PGconn* db, int type, int organizer_id, StatRevenue* out);
