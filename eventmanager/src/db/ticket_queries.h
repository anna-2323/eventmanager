#pragma once

const char* SQL_CHECK_SEAT =
"SELECT es.capacity - COUNT(t.id) "
"FROM data.event_sectors es "
"LEFT JOIN data.tickets t "
"    ON t.event_id = es.event_id "
"   AND t.sector_id = es.sector_id "
"WHERE es.event_id = $1 "
"  AND es.sector_id = $2 "
"GROUP BY es.capacity;";

const char* SQL_ADD_TICKET_USER =
"INSERT INTO data.tickets (event_id, user_id, sector_id, first_name, last_name, email, phone) "
"VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id";

const char* SQL_ADD_TICKET_GUEST =
"INSERT INTO data.tickets (event_id, sector_id, first_name, last_name, email, phone) "
"VALUES ($1, $2, $3, $4, $5, $6) RETURNING id";

const char* SQL_GET_TICKET =
"SELECT e.title, e.begins_at, v.venue_name, v.city, v.address, "
"       t.first_name, t.last_name, t.email, t.phone, t.access_token, "
"       CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"LEFT JOIN data.sectors s "
"    ON s.id = t.sector_id "
"WHERE t.id = $1;";

const char* SQL_GET_TICKET_FOR_HTML =
"SELECT e.title, e.begins_at, v.venue_name, v.city, v.address, "
"       t.first_name, t.last_name, t.email, t.phone, t.access_token, "
"       CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.sectors s ON s.id = t.sector_id "
"WHERE t.id = $1;";
