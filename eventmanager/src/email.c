#include <curl/curl.h>
#include "email.h"

static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp) {
    MailPayload* p = (MailPayload*)userp;
    size_t len = strlen(p->data) - p->pos;
    if (len == 0) return 0;
    size_t copy = size * nmemb < len ? size * nmemb : len;
    memcpy(ptr, p->data + p->pos, copy);
    p->pos += copy;
    return copy;
}

int send_email(const char* to, const char* subject, const char* body) {
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    char message[4096];
    snprintf(message, sizeof(message),
        "From: EventMGR <noreply@yourdomain.com>\r\n"
        "To: %s\r\n"
        "Subject: %s\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "%s\r\n",
        to, subject, body);

    MailPayload payload = { message, 0 };

    struct curl_slist* recipients = curl_slist_append(NULL, to);

    char* smpt_user = NULL;
    char* smpt_pass = NULL;
    size_t len = 0;
    _dupenv_s(&smpt_user, &len, "SMTP_USER");
    _dupenv_s(&smpt_pass, &len, "SMTP_PASS");

    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(curl, CURLOPT_USERNAME, smpt_user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, smpt_pass);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, smpt_user);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "Грешка при изпращане на имейл: %s\n", curl_easy_strerror(res));

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}
