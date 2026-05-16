#include "civetweb.h"
#include "session.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static Session sessions[SESSION_MAX];

void session_init(void) {
    memset(sessions, 0, sizeof(sessions));
}

static void generate_token(char* out) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < TOKEN_LEN; i++)
        out[i] = charset[rand() % (sizeof(charset) - 1)];
    out[TOKEN_LEN] = '\0';
}

Session* session_create(int user_id, const char* email, int role) {
    for (int i = 0; i < SESSION_MAX; i++) {
        if (sessions[i].token[0] == '\0') {

            generate_token(sessions[i].token);

            sessions[i].user_id = user_id;
            strncpy_s(sessions[i].email, sizeof(sessions[i].email) - 1, email, sizeof(sessions[i].email) - 1);
            sessions[i].role = role;
            sessions[i].expires = time(NULL) + 86400;  // изтича след 24 часа

            return &sessions[i];
        }
    }
    return NULL;
}

Session* session_from_token(const char* token) {
    if (!token) return NULL;
    for (int i = 0; i < SESSION_MAX; i++) {
        if (sessions[i].token[0] != '\0' &&
            strcmp(sessions[i].token, token) == 0) {
            if (time(NULL) > sessions[i].expires) {
                session_delete(token);
                return NULL;
            }
            return &sessions[i];
        }
    }
    return NULL;
}

Session* get_session(struct mg_connection* conn) {
    const struct mg_request_info* info = mg_get_request_info(conn);
    if (!info) return NULL;

    for (int i = 0; i < info->num_headers; i++) {
        if (mg_strcasecmp(info->http_headers[i].name, "Cookie") == 0) {
            const char* cookies = info->http_headers[i].value;
            if (!cookies || cookies[0] == '\0') return NULL;

            char token[TOKEN_LEN + 1] = "";
            int ret = mg_get_cookie(cookies, "session", token, sizeof(token));
            if (ret < 0) return NULL;

            return session_from_token(token);
        }
    }
    return NULL;
}

void session_delete(const char* token) {
    for (int i = 0; i < SESSION_MAX; i++)
        if (strcmp(sessions[i].token, token) == 0)
            memset(&sessions[i], 0, sizeof(Session));
}
