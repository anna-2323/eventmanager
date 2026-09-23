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
	int active;
	Venue venue;
} Event;

typedef struct {
	int organizer_id;
	int venue_id;
	char title[256];
	char begins_at[256];
	char img_path[256];
	char description[512];
	double price;
	int capacity;
} EventData;

typedef struct {
	int id;
	char title[256];
} Category;

typedef struct {
	const char* search;
	const char* city;
	const char* from;
	const char* to;

	int category_id;

	int upcoming;
	int uploaded;
	int booked;

	int active;

	int user_id;
} EventFilters;

typedef struct {
	char venue_id[16];
	char title[256];
	char description[513];
	char begins_at[256];
	char price[32];
	char capacity[32];

	char image_path[256];       // /res/...
	char image_disk_path[512];  // .\html\res\...
	int image_uploaded;

	int verified;
	int verified_set;
} CreateEventForm;

int get_events(PGconn* db, const EventFilters* filters, Event** out);
int get_event(PGconn* db, int id, Event* out);
int get_user_events(PGconn* db, int id, Event** out);
int get_events_in_venue(PGconn* db, int venue_id, Event** out);

int add_event(PGconn* db, EventData* data);

int admin_update_title(PGconn* db, int id, const char* title);
int admin_update_begins_at(PGconn* db, int id, const char* begins_at);
int admin_update_description(PGconn* db, int id, const char* description);
int admin_update_image(PGconn* db, int event_id, const char* img_path);
// Събитието не се изтрива напълно за да се предотвратят конфликти в минали записи, свързани с това събитие
int set_event_active(PGconn* db, int id, int active);

int get_categories(PGconn* db, Category** out);

int get_total_events(PGconn* db, int organizer_id);
int get_events_growth(PGconn* db, int type, int organizer_id, StatGrowth** out);
