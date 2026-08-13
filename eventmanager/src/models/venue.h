#pragma once
#include "libpq-fe.h"
#include <jansson.h>
#include "../util.h"

typedef struct {
	int id;
	char venue_name[255];
	char city[255];
	char address[255];
} Venue;

int get_venues(PGconn* db, json_t* out);
json_t* get_venue(PGconn* db, int id);

json_t* get_sectors(PGconn* db, int venue_id);
json_t* get_cities(PGconn* db);

int add_venue(PGconn* db, const char* city, const char* address, const char* venue_name);
int update_venue_name(PGconn* db, int id, const char* venue_name);
int soft_delete_venue(PGconn* db, int id);
int restore_venue(PGconn* db, int id);

json_t* get_total_venues(PGconn* db);