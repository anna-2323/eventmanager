#pragma once

typedef struct {
    const char* data;
    size_t      pos;
} MailPayload;

static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp);
int send_email(const char* to, const char* subject, const char* body);