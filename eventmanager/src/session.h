#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include "models/user.h"

#define SESSION_MAX 256
#define TOKEN_LEN   64

typedef struct {
    char token[TOKEN_LEN + 1];
    int user_id;
    char email[255];
    char phone[255];
    char first_name[255];
    char last_name[255];
    int role;
    time_t expires;
} Session;

void session_init(void);
Session* session_create(int user_id, const char* email, const char* first_name, const char* last_name, const char* phone, int role);
Session* session_from_token(const char* token);
Session* get_session(struct mg_connection* conn);
void session_delete(const char* token);
