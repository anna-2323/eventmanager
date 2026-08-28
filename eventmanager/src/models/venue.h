#pragma once
#include "libpq-fe.h"
#include <jansson.h>
#include "../util.h"
#include "model.h"

typedef struct {
	int id;
	char venue_name[256];
	char city[256];
	char address[256];
	int active;
	int has_sectors;
} Venue;

int get_venues(PGconn* db, Venue** out);
int get_venue(PGconn* db, int id, Venue* out);

int get_sectors(PGconn* db, int venue_id, Sector** out);
int get_cities(PGconn* db, char** out);

int add_venue(PGconn* db, Venue* v);
int update_venue_name(PGconn* db, int id, const char* venue_name);
int soft_delete_venue(PGconn* db, int id);
int restore_venue(PGconn* db, int id);

int get_total_venues(PGconn* db);