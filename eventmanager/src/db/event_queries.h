#pragma once

const char* SQL_GET_EVENTS =
"SELECT "
"    e.id, "
"    e.title, "
"    e.begins_at, "
"    e.img_path, "
"    v.venue_name, "
"    v.city, "
"    MIN(es.price) AS price, "
"    SUM(es.capacity) - COUNT(t.id) AS seats_left "
"FROM data.events e "
"JOIN data.venues v "
"    ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es "
"    ON es.event_id = e.id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"   AND t.sector_id = es.sector_id "
"WHERE "
"    ($1::boolean = FALSE OR e.begins_at > NOW()) "
"AND ($2::text IS NULL OR "
"     e.title ILIKE '%' || $2 || '%' "
"     OR v.venue_name ILIKE '%' || $2 || '%') "
"AND ($3::text IS NULL OR v.city = $3) "
"AND ($4::int IS NULL OR EXISTS ("
"    SELECT 1 "
"    FROM data.events_categories ec "
"    WHERE ec.event_id = e.id "
"      AND ec.category_id = $4 "
")) "
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
"SUM(es.capacity) - COUNT(t.id) AS seats_left "
"FROM data.events e "
"JOIN data.venues v ON e.venue_id = v.id "
"JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"    AND t.sector_id = es.sector_id "
"WHERE e.organizer_id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, "
"         v.venue_name, v.city;";

const char* SQL_GET_BOOKED_EVENTS =
"SELECT t.event_id, e.title, e.begins_at, e.img_path, "
"v.venue_name, v.city, es.price, s.name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"    AND es.sector_id = t.sector_id "
"JOIN data.sectors s ON es.sector_id = s.id "
"WHERE t.user_id = $1 ";

const char* SQL_GET_VENUE_EVENTS =
"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
"       MIN(es.price) AS price, "
"       SUM(es.capacity) - COUNT(t.id) AS seats_left, "
"       e.verified::int "
"FROM data.events e "
"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.tickets t "
"    ON t.event_id = e.id "
"   AND t.sector_id = es.sector_id "
"JOIN data.venues v ON e.venue_id = v.id "
"WHERE e.venue_id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, e.verified "
"ORDER BY e.begins_at ASC;";

const char* SQL_GET_EVENT =
"SELECT e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city, "
"MIN(es.price) AS price, "
"SUM(es.capacity) - COUNT(t.id) AS seats_left, "
"e.verified::int "
"FROM data.events e "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es ON es.event_id = e.id "
"LEFT JOIN data.tickets t ON t.event_id = e.id AND t.sector_id = es.sector_id "
"WHERE e.id = $1 "
"GROUP BY e.id, e.title, e.begins_at, e.img_path, v.venue_name, v.city; ";

const char* SQL_HAS_SEATMAP =
"SELECT v.has_sectors, v.background_svg, v.viewBox "
"FROM data.events e "
"JOIN data.venues v ON v.id = e.venue_id "
"WHERE e.id = $1;";

const char* SQL_NO_SEATMAP =
"SELECT es.sector_id "
"FROM data.event_sectors es "
"WHERE es.event_id = $1;";

const char* SQL_GET_SEATMAP =
"SELECT s.id, s.name, es.capacity, es.price, s.color, s.svg_path, "
"       es.capacity - COALESCE(t.sold, 0) AS available "
"FROM data.event_sectors es "
"JOIN data.sectors s ON s.id = es.sector_id "
"LEFT JOIN ( "
"    SELECT sector_id, COUNT(*) AS sold "
"    FROM data.tickets "
"    WHERE event_id = $1 "
"    GROUP BY sector_id "
") t ON t.sector_id = es.sector_id "
"WHERE es.event_id = $1 "
"ORDER BY s.display_order;";

const char* SQL_ADD_EVENT =
"INSERT INTO data.events (title, begins_at, venue_id, organizer_id, img_path) "
"VALUES ($1, $2, $3, $4, '/res/default1.png') "
"RETURNING id";

const char* SQL_ADD_EVENT_SECTORS =
"INSERT INTO data.event_sectors "
"(event_id, sector_id, price, capacity) "
"VALUES ($1, $2, $3, $4)";

const char* SQL_UPDATE_EVENT_TITLE =
"UPDATE data.events SET title = $1 WHERE id = $2";

const char* SQL_UPDATE_EVENT_BEGINS_AT =
"UPDATE data.events SET begins_at = $1 WHERE id = $2";

const char* SQL_VERIFY_EVENT =
"UPDATE data.events SET verified = TRUE WHERE id = $1";

const char* SQL_UNVERIFY_EVENT =
"UPDATE data.events SET verified = FALSE WHERE id = $1";

const char* SQL_DELETE_EVENT =
"DELETE FROM data.events WHERE id = $1 ";

const char* SQL_TOTAL_EVENTS =
"SELECT COUNT(*) "
"FROM data.events;";

const char* SQL_EVENTS_GROWTH_MONTHLY =
"SELECT DATE_TRUNC('month', e.uploaded_at) AS month, "
"COUNT(*) AS event_count "
"FROM data.events e "
"WHERE e.uploaded_at >= CURRENT_DATE - INTERVAL '12 months' "
"GROUP BY DATE_TRUNC('month', e.uploaded_at) "
"ORDER BY month; ";

const char* SQL_EVENTS_GROWTH_DAILY =
"SELECT DATE_TRUNC('day', e.uploaded_at) AS day, "
"COUNT(*) AS event_count "
"FROM data.events e "
"WHERE e.uploaded_at >= CURRENT_DATE - INTERVAL '30 days' "
"GROUP BY DATE_TRUNC('day', e.uploaded_at) "
"ORDER BY day; ";

const char* SQL_EVENTS_GROWTH_MONTHLY_ALL =
"SELECT DATE_TRUNC('month', e.uploaded_at) AS month, "
"COUNT(*) AS event_count "
"FROM data.events e "
"GROUP BY DATE_TRUNC('month', e.uploaded_at) "
"ORDER BY month; ";
