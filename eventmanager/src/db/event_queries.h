#pragma once

const char* SQL_GET_EVENTS =
"SELECT "
"    e.id, e.title, e.begins_at, e.img_path, "
"    v.venue_name, v.city, "
"    MIN(es.price) AS price, "
"    SUM(s.capacity) - COUNT(t.id) FILTER (WHERE t.active) AS seats_left, "
"    e.description, e.active "
"FROM data.events e "
"JOIN data.venues v "
"    ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es "
"    ON es.event_id = e.id "
"LEFT JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"   AND t.sector_id = es.sector_id "
"WHERE "
"    ($1::boolean = FALSE OR e.begins_at > NOW()) "
"AND ($2::text IS NULL OR "
"     e.title ILIKE '%' || $2 || '%' "
"     OR v.venue_name ILIKE '%' || $2 || '%') "
"AND ($3::text IS NULL OR v.city = $3) "
"AND ($4::int IS NULL OR e.category_id = $4) "
"AND ($5::boolean = FALSE OR e.active) "
"AND ($6::timestamptz IS NULL OR e.begins_at >= $6) "
"AND ($7::timestamptz IS NULL OR e.begins_at <= $7) "
"GROUP BY "
"    e.id, "
"    e.title, "
"    e.begins_at, "
"    e.img_path, "
"    v.venue_name, "
"    v.city "
"ORDER BY e.begins_at ASC;";

const char* SQL_GET_UPLOADED_EVENTS =
"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
"MIN(es.price) AS price, "
"SUM(s.capacity) - COUNT(t.id) FILTER (WHERE t.active) AS seats_left, "
"e.active "
"FROM data.events e "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"    AND t.sector_id = es.sector_id "
"WHERE e.organizer_id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, "
"         v.venue_name, v.city;";

const char* SQL_GET_VENUE_EVENTS =
"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
"       MIN(es.price) AS price, "
"       SUM(s.capacity) - COUNT(t.id) FILTER (WHERE t.active) AS seats_left, "
"       e.active "
"FROM data.events e "
"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"   AND t.sector_id = es.sector_id "
"JOIN data.venues v ON e.venue_id = v.id "
"WHERE e.venue_id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, e.active "
"ORDER BY e.begins_at ASC;";

const char* SQL_GET_EVENT =
"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
"MIN(es.price) AS price, "
"SUM(s.capacity) - COUNT(t.id) FILTER (WHERE t.active) AS seats_left, "
"e.description, e.active "
"FROM data.events e "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = es.sector_id "
"WHERE e.id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city; ";

const char* SQL_ADD_EVENT =
"INSERT INTO data.events "
"    (title, description, begins_at, venue_id, organizer_id, img_path) "
"VALUES ($1, $2, $3, $4, $5, $6) "
"RETURNING id";

const char* SQL_ADD_EVENT_SECTORS =
"INSERT INTO data.event_sectors "
"(event_id, sector_id, price, capacity) "
"VALUES ($1, $2, $3, $4)";

const char* SQL_UPDATE_EVENT_TITLE =
"UPDATE data.events SET title = $1 WHERE id = $2";

const char* SQL_UPDATE_EVENT_BEGINS_AT =
"UPDATE data.events SET begins_at = $1 WHERE id = $2";

const char* SQL_UPDATE_EVENT_DESCRIPTION =
"UPDATE data.events SET description = $1 WHERE id = $2";

const char* SQL_ADMIN_UPDATE_EVENT_IMAGE =
"UPDATE data.events SET img_path = $1 WHERE id = $2;";

const char* SQL_GET_EVENT_IMAGE_PATH =
"SELECT img_path FROM data.events WHERE id = $1;";

const char* SQL_ACTIVATE_EVENT =
"UPDATE data.events SET active = TRUE WHERE id = $1";

const char* SQL_DEACTIVATE_EVENT =
"UPDATE data.events SET active = FALSE WHERE id = $1";

const char* SQL_GET_CATEGORIES =
"SELECT id, title FROM data.categories;";

const char* SQL_TOTAL_EVENTS =
"SELECT COUNT(*) "
"FROM data.events e "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"AND e.active;";

const char* SQL_EVENTS_GROWTH_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', e.uploaded_at) AS month, "
"    COUNT(*) AS event_count "
"FROM data.events e "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       e.uploaded_at >= DATE_TRUNC('month', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND e.active "
"GROUP BY DATE_TRUNC('month', e.uploaded_at) "
"ORDER BY month;";

const char* SQL_EVENTS_GROWTH_DAILY =
"SELECT "
"    DATE_TRUNC('day', e.uploaded_at) AS day, "
"    COUNT(*) AS event_count "
"FROM data.events e "
"WHERE e.uploaded_at >= CURRENT_DATE - INTERVAL '30 days' "
"  AND ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       e.uploaded_at >= DATE_TRUNC('day', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND e.active "
"GROUP BY DATE_TRUNC('day', e.uploaded_at) "
"ORDER BY day;";