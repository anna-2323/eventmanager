#pragma once

const char* SQL_CHECK_SEAT =
"SELECT s.capacity - COUNT(t.id) "
"FROM data.sectors s "
"LEFT JOIN data.tickets t "
"    ON t.event_id = $1 "
"   AND t.sector_id = s.id "
"WHERE s.id = $2 "
"GROUP BY s.capacity;";

const char* SQL_SECTOR_PRICE =
"SELECT price "
"FROM data.event_sectors "
"WHERE sector_id = $1;";

const char* SQL_ADD_TICKET_USER =
"INSERT INTO data.tickets (event_id, user_id, sector_id, first_name, last_name, email, phone, price) "
"VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING id";

const char* SQL_ADD_TICKET_GUEST =
"INSERT INTO data.tickets (event_id, sector_id, first_name, last_name, email, phone, price) "
"VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id";

const char* SQL_GET_TICKET =
"SELECT t.id, e.title, e.begins_at, v.venue_name, v.city, v.address, "
"       t.first_name, t.last_name, t.email, t.phone, t.price, "
"		t.access_token, e.id, u.id, t.active,  "
"       CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.users u ON t.user_id = u.id "
"LEFT JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"LEFT JOIN data.sectors s "
"    ON s.id = t.sector_id "
"WHERE t.id = $1;";

const char* SQL_GET_TICKETS =
"SELECT t.id, e.title, e.begins_at, v.venue_name, v.city, v.address, "
"       t.first_name, t.last_name, t.email, t.phone, t.price, "
"       t.access_token, e.id, u.id, t.active,  "
"       CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.sectors s "
"    ON s.id = t.sector_id "
"LEFT JOIN data.users u on t.user_id = u.id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"ORDER BY e.begins_at;";

const char* SQL_GET_USER_TICKETS =
"SELECT t.id, e.title, e.begins_at, v.venue_name, v.city, v.address, "
"       t.first_name, t.last_name, t.email, t.phone, t.price, "
"		t.access_token, e.id, u.id, t.active,  "
"       CASE WHEN v.has_sectors THEN s.name ELSE NULL END AS sector_name "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"JOIN data.venues v ON e.venue_id = v.id "
"LEFT JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"LEFT JOIN data.sectors s "
"    ON s.id = t.sector_id "
"JOIN data.users u on t.user_id = u.id "
"WHERE u.id = $1;";

const char* SQL_ACTIVATE_TICKET =
"UPDATE data.tickets SET active = TRUE WHERE id = $1";

const char* SQL_DEACTIVATE_TICKET =
"UPDATE data.tickets SET active = FALSE WHERE id = $1";

const char* SQL_TICKET_BELONGS_TO_USER_BY_ID =
"SELECT EXISTS( "
"    SELECT 1 "
"    FROM data.tickets "
"    WHERE id = $1 "
"    AND user_id = $2 "
");";

const char* SQL_TICKET_BELONGS_TO_USER_BY_UUID =
"SELECT EXISTS( "
"    SELECT 1 "
"    FROM data.tickets "
"    WHERE access_token = $1 "
"    AND user_id = $2 "
");";

const char* SQL_TOTAL_TICKETS =
"SELECT COUNT(*) "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"AND t.active;";

const char* SQL_TICKETS_GROWTH_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.events e "
"    ON e.id = t.event_id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       t.purchased_at >= DATE_TRUNC('month', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND t.active = TRUE "
"GROUP BY DATE_TRUNC('month', t.purchased_at) "
"ORDER BY month;";

const char* SQL_TICKETS_GROWTH_DAILY =
"SELECT "
"    DATE_TRUNC('day', t.purchased_at) AS day, "
"    COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.events e "
"    ON e.id = t.event_id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       t.purchased_at >= DATE_TRUNC('day', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND t.active = TRUE "
"GROUP BY DATE_TRUNC('day', t.purchased_at) "
"ORDER BY day;";

const char* SQL_TOTAL_REVENUE =
"SELECT SUM(t.price) AS total_revenue "
"FROM data.tickets t "
"JOIN data.events e ON t.event_id = e.id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"AND t.active;";

const char* SQL_REVENUE_DAILY =
"SELECT "
"    DATE_TRUNC('day', t.purchased_at) AS day, "
"    SUM(t.price) AS revenue, "
"    COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.events e "
"    ON e.id = t.event_id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       t.purchased_at >= DATE_TRUNC('day', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND t.active "
"GROUP BY DATE_TRUNC('day', t.purchased_at) "
"ORDER BY day;";

const char* SQL_REVENUE_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(t.price) AS revenue, "
"    COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.events e "
"    ON e.id = t.event_id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       t.purchased_at >= DATE_TRUNC('month', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND t.active "
"GROUP BY DATE_TRUNC('month', t.purchased_at) "
"ORDER BY month;";

const char* SQL_REVENUE_BY_VENUE_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(t.price) AS revenue, "
"    COUNT(*) AS ticket_count, "
"    v.id AS venue_id "
"FROM data.tickets t "
"JOIN data.events e "
"    ON e.id = t.event_id "
"JOIN data.venues v "
"    ON v.id = e.venue_id "
"WHERE ($1::integer IS NULL OR e.organizer_id = $1) "
"  AND ($2::integer IS NULL OR "
"       t.purchased_at >= DATE_TRUNC('month', CURRENT_DATE) "
"           - ($2::integer * INTERVAL '1 month')) "
"  AND v.active "
"  AND t.active "
"GROUP BY v.id, DATE_TRUNC('month', t.purchased_at) "
"ORDER BY v.id, month;";
