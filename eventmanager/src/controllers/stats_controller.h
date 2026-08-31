#pragma once

typedef enum {
    STAT_MONTHLY,
    STAT_MONTHLY_ALL,
    STAT_DAILY,
    STAT_BY_VENUE,
    STAT_BY_VENUE_ALL
} StatType;

int api_stats(struct mg_connection* conn, void* data);
int api_stats_export(struct mg_connection* conn, void* data);