#pragma once

typedef struct {
    const char* data;
    size_t      pos; // Позицията на следващия неизпратен байт
} MailPayload;

static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp);
int send_forgot_email(const char* to, const char* subject, const char* body);
int send_ticket_email(const char* to, const char* subject, const char* body, const char* token);