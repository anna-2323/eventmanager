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

const char* SQL_TOTAL_TICKETS =
"SELECT COUNT(*) "
"FROM data.tickets;";

const char* SQL_TICKETS_GROWTH_MONTHLY =
"SELECT "
"DATE_TRUNC('month', t.purchased_at) AS month, "
"COUNT(*) AS ticket_count "
"FROM data.tickets t "
"WHERE t.purchased_at >= CURRENT_DATE - INTERVAL '12 months' "
"GROUP BY DATE_TRUNC('month', t.purchased_at) "
"ORDER BY month; ";

const char* SQL_TICKETS_GROWTH_MONTHLY_ALL =
"SELECT "
"DATE_TRUNC('month', t.purchased_at) AS month, "
"COUNT(*) AS ticket_count "
"FROM data.tickets t "
"GROUP BY DATE_TRUNC('month', t.purchased_at) "
"ORDER BY month; ";

const char* SQL_TICKETS_GROWTH_DAILY =
"SELECT "
"DATE_TRUNC('day', t.purchased_at) AS day, "
"COUNT(*) AS ticket_count "
"FROM data.tickets t "
"WHERE t.purchased_at >= CURRENT_DATE - INTERVAL '30 days' "
"GROUP BY DATE_TRUNC('day', t.purchased_at) "
"ORDER BY day; ";

const char* SQL_TOTAL_REVENUE =
"SELECT SUM(es.price) AS total_revenue "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"ON es.event_id = t.event_id "
"AND es.sector_id = t.sector_id; ";

const char* SQL_REVENUE_DAILY =
"SELECT DATE_TRUNC('day', t.purchased_at) AS day, "
"SUM(es.price) AS revenue, "
"COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"ON es.event_id = t.event_id "
"WHERE t.purchased_at >= CURRENT_DATE - INTERVAL '30 days' "
"AND es.sector_id = t.sector_id "
"GROUP BY day "
"ORDER BY day; ";

const char* SQL_REVENUE_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(es.price) AS revenue, "
"	COUNT(*) AS ticket_count "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"WHERE t.purchased_at >= CURRENT_DATE - INTERVAL '12 months' "
"GROUP BY month "
"ORDER BY month;";

const char* SQL_REVENUE_MONTHLY_ALL =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(es.price) AS revenue, "
"    COUNT(*) AS tickets_sold "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"GROUP BY month "
"ORDER BY month;";

const char* SQL_REVENUE_BY_VENUE_MONTHLY =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(es.price) AS revenue, "
"    COUNT(*) AS tickets_sold, "
"    v.id AS venue_id "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"JOIN data.events e ON e.id = t.event_id "
"JOIN data.venues v ON v.id = e.venue_id "
"WHERE t.purchased_at >= CURRENT_DATE - INTERVAL '12 months' "
"GROUP BY v.id, month "
"ORDER BY v.id, month;";

const char* SQL_REVENUE_BY_VENUE_MONTHLY_ALL =
"SELECT "
"    DATE_TRUNC('month', t.purchased_at) AS month, "
"    SUM(es.price) AS revenue, "
"    COUNT(*) AS tickets_sold, "
"    v.id AS venue_id "
"FROM data.tickets t "
"JOIN data.event_sectors es "
"    ON es.event_id = t.event_id "
"   AND es.sector_id = t.sector_id "
"JOIN data.events e ON e.id = t.event_id "
"JOIN data.venues v ON v.id = e.venue_id "
"GROUP BY v.id, month "
"ORDER BY v.id, month;";
