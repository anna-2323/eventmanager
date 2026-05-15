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
	char uploaded_on[255];
	char img_path[255];
	char email[255];
	Venue venue;
} Event;

int get_all_events(PGconn* db, json_t* out);
json_t* get_event(PGconn* db, int id);
json_t* event_to_json(Event e);
