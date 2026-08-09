#pragma once
#include "libpq-fe.h"
#include "civetweb.h"
#include "jansson.h"

typedef struct {
	int id;
	char venue_name[255];
	char city[255];
	char address[255];
} Venue;

typedef struct {
	int id;
	int capacity;
	int seats_left;
	float price;
	char title[255];
	char begins_at[255];
	char img_path[255];
	char uploaded_on[255];
	int organizer_id;
	int verified;
	Venue venue;
} Event;

int get_events(PGconn* db, const char* search, const char* sort, json_t* out);
json_t* get_event(PGconn* db, int id);
json_t* get_event_layout(PGconn* db, int id);
json_t* get_user_events(PGconn* db, int id);

int admin_update_title(PGconn* db, int id, const char* title);
int admin_update_begins_at(PGconn* db, int id, const char* begins_at);

int verify_event(PGconn* db, int id);
int unverify_event(PGconn* db, int id);
int delete_event(PGconn* db, int id);

json_t* event_to_json(Event e);
