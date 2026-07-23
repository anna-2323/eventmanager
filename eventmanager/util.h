#pragma once

#define CHECK_DB(db, ret) \
    if ((db) == NULL) { \
        fprintf(stderr, "Няма връзка с БД във функцията %s\n", __func__); \
        return (ret); \
    }

#define CHECK_QUERY(res, db, ret) \
    if (PQresultStatus(res) != PGRES_TUPLES_OK) { \
        fprintf(stderr, "Грешка във функцията %s: %s\n", __func__, PQerrorMessage(db)); \
        PQclear(res); \
        return 0; \
    }
