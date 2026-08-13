#pragma once

const char* SQL_GET_VENUES =
"SELECT v.id, v.city, v.address, v.venue_name "
"FROM data.venues v;";

const char* SQL_GET_VENUE =
"SELECT v.id, v.city, v.address, v.venue_name, v.active, v.has_sectors::int "
"FROM data.venues v "
"WHERE v.id = $1;";

const char* SQL_GET_SECTORS =
"SELECT s.id, s.name "
"FROM data.sectors s "
"WHERE s.venue_id = $1;";

const char* SQL_GET_CITIES =
"SELECT DISTINCT city "
"FROM data.venues "
"ORDER BY city;";

const char* SQL_ADD_VENUE =
"INSERT INTO data.venues (city, address, venue_name, has_sectors) "
"VALUES ($1, $2, $3, false) "
"RETURNING id";

const char* SQL_ADD_VENUE_SECTOR =
"INSERT INTO data.sectors "
"(venue_id) VALUES ($1)";

const char* SQL_UPDATE_VENUE_NAME =
"UPDATE data.venues SET venue_name = $1 WHERE id = $2;";

const char* SQL_DEACTIVATE_VENUE =
"UPDATE data.venues SET active = FALSE WHERE id = $1;";

const char* SQL_RESTORE_VENUE =
"UPDATE data.venues SET active = TRUE WHERE id = $1;";
