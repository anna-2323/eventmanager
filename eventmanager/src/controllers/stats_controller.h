#pragma once

typedef enum {
    STAT_MONTHLY,
    STAT_MONTHLY_ALL,
    STAT_DAILY,
    STAT_BY_VENUE
} StatType;

int api_admin_stats(struct mg_connection* conn, void* data);
int api_admin_stats_export(struct mg_connection* conn, void* data);