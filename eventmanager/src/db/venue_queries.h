#pragma once

const char* SQL_GET_VENUES =
"SELECT v.id, v.city, v.address, v.venue_name, "
"v.has_sectors, v.active "
"FROM data.venues v "
"WHERE ($1::boolean = FALSE OR v.active);";

const char* SQL_GET_VENUE =
"SELECT v.id, v.city, v.address, v.venue_name, "
"	v.has_sectors, v.active, "
"   COALESCE(SUM(s.capacity), 0) AS capacity "
"FROM data.venues v "
"LEFT JOIN data.sectors s ON s.venue_id = v.id "
"WHERE v.id = $1 " 
"GROUP BY v.id; ";

const char* SQL_GET_SECTORS =
"SELECT s.id, s.name "
"FROM data.sectors s "
"WHERE s.venue_id = $1;";

const char* SQL_EVENT_HAS_SEATMAP =
"SELECT v.has_sectors, v.background_svg, v.viewBox "
"FROM data.events e "
"JOIN data.venues v ON v.id = e.venue_id "
"WHERE e.id = $1;";

const char* SQL_VENUE_HAS_SEATMAP =
"SELECT v.has_sectors, v.background_svg, v.viewBox "
"FROM data.venues v "
"WHERE v.id = $1; ";

const char* SQL_EVENT_NO_SEATMAP =
"SELECT es.sector_id "
"FROM data.event_sectors es "
"WHERE es.event_id = $1;";

const char* SQL_VENUE_NO_SEATMAP =
"SELECT s.id AS sector_id "
"FROM data.sectors s "
"WHERE s.venue_id = $1;";

const char* SQL_GET_EVENT_SEATMAP =
"SELECT s.id, s.name, s.capacity, s.color, s.svg_path, es.price, "
"       s.capacity - COALESCE(t.sold, 0) AS available "
"FROM data.event_sectors es "
"JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN ( "
"    SELECT sector_id, "
"	 COUNT(*) FILTER (WHERE active) AS sold "
"    FROM data.tickets "
"    WHERE event_id = $1 "
"    GROUP BY sector_id "
") t ON t.sector_id = es.sector_id "
"WHERE es.event_id = $1 "
"ORDER BY s.display_order;";

const char* SQL_GET_VENUE_SEATMAP =
"SELECT s.id, s.name, s.capacity, "
"    s.color, s.svg_path "
"FROM data.sectors s "
"WHERE s.venue_id = $1 "
"ORDER BY s.display_order;";

const char* SQL_GET_CITIES =
"SELECT DISTINCT city "
"FROM data.venues "
"WHERE active "
"ORDER BY city;";

const char* SQL_ADD_VENUE =
"INSERT INTO data.venues (city, address, venue_name, has_sectors) "
"VALUES ($1, $2, $3, false) "
"RETURNING id";

const char* SQL_ADD_VENUE_SECTOR =
"INSERT INTO data.sectors "
"(venue_id, capacity) VALUES ($1, $2)";

const char* SQL_UPDATE_VENUE_NAME =
"UPDATE data.venues SET venue_name = $1 WHERE id = $2;";

const char* SQL_UPDATE_VENUE_ADDRESS =
"UPDATE data.venues SET address = $1 WHERE id = $2;";

const char* SQL_ACTIVATE_VENUE =
"UPDATE data.venues SET active = TRUE WHERE id = $1;";

const char* SQL_DEACTIVATE_VENUE =
"UPDATE data.venues SET active = FALSE WHERE id = $1;";

const char* SQL_RESTORE_VENUE =
"UPDATE data.venues SET active = TRUE WHERE id = $1;";

const char* SQL_TOTAL_VENUES =
"SELECT COUNT(*) "
"FROM data.venues "
"WHERE active;";
