#pragma once
#include "libpq-fe.h"
#include "civetweb.h"
#include "jansson.h"
#include "venue.h"
#include "model.h"

typedef struct {
	int id;
	int capacity;
	int seats_left;
	float price;
	char title[256];
	char begins_at[256];
	char img_path[256];
	char uploaded_on[256];
	char description[512];
	int organizer_id;
	int verified;
	Venue venue;
} Event;

typedef struct {
	int organizer_id;
	int venue_id;
	char title[256];
	char begins_at[256];
	double price;
	int capacity;
} EventData;

typedef struct {
	int has_sectors;
	char background_svg[256];
	char viewbox[50];
	Sector* sectors;
	int sector_count;
} SeatMap;

typedef struct {
	int id;
	char title[256];
} Category;

typedef struct {
	const char* search;
	const char* city;
	int category_id;

	int upcoming;
	int uploaded;
	int booked;

	int user_id;
} EventFilters;

int get_events(PGconn* db, const EventFilters* filters, Event** out);
int get_event(PGconn* db, int id, Event* out);
int get_event_seatmap(PGconn* db, int id, SeatMap* out);
int get_user_events(PGconn* db, int id, Event** out);
int get_events_in_venue(PGconn* db, int venue_id, Event** out);

int add_event(PGconn* db, EventData* data);

int admin_update_title(PGconn* db, int id, const char* title);
int admin_update_begins_at(PGconn* db, int id, const char* begins_at);

int verify_event(PGconn* db, int id);
int unverify_event(PGconn* db, int id);
int delete_event(PGconn* db, int id);

int get_categories(PGconn* db, Category** out);

int get_total_events(PGconn* db);
int get_events_growth(PGconn* db, int type, StatGrowth** out);
