#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <libpq-fe.h>

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

#define CHECK_COMMAND_QUERY(res, db, ret) \
    if (PQresultStatus(res) != PGRES_COMMAND_OK) { \
        fprintf(stderr, "Грешка във функцията %s: %s\n", __func__, PQerrorMessage(db)); \
        PQclear(res); \
        return 0; \
    }

typedef struct {
    char app_dir[MAX_PATH];
    char html_dir[MAX_PATH];
    char tickets_dir[MAX_PATH];

    char db_host[256];
    char db_port[16];
    char db_name[256];
    char db_user[256];
    char db_password[256];

    int server_port;
} Config;

typedef struct {
    PGconn* db;
    Config* config;
} TicketContext;

char* read_file_to_string(const char* path);
int write_string_to_file(const char* path, const char* content);