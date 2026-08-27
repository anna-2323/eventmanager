#pragma once
#include "libpq-fe.h"
#include "civetweb.h"
#include "jansson.h"
#include "venue.h"

typedef struct {
	int id;
	int capacity;
	int seats_left;
	float price;
	char title[255];
	char begins_at[255];
	char img_path[255];
	char uploaded_on[255];
	char description[512];
	int organizer_id;
	int verified;
	Venue venue;
} Event;

typedef struct {
	const char* search;
	const char* city;
	int category_id;

	int upcoming;
	int uploaded;
	int booked;

	int user_id;
} EventFilters;

int get_events(PGconn* db, const EventFilters* filters, json_t* out);
json_t* get_event(PGconn* db, int id);
json_t* get_event_seatmap(PGconn* db, int id);
json_t* get_user_events(PGconn* db, int id);
json_t* get_events_in_venue(PGconn* db, int venue_id);

int admin_update_title(PGconn* db, int id, const char* title);
int admin_update_begins_at(PGconn* db, int id, const char* begins_at);

int verify_event(PGconn* db, int id);
int unverify_event(PGconn* db, int id);
int delete_event(PGconn* db, int id);

json_t* get_categories(PGconn* db);

json_t* get_total_events(PGconn* db);
json_t* get_events_growth(PGconn* db, int type);


json_t* event_to_json(Event e);
