#include <curl/curl.h>
#include "email.h"

// Използва се в send_forgot_email, но не работи със send_ticket_email
static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp) {
    MailPayload* p = (MailPayload*)userp;
    // Проверка колко байта остават в data:
    size_t len = strlen(p->data) - p->pos;
    if (len == 0)
        return 0;
    // Като количество се записва кое е по-малко - мястото в буфера или останалите байтове:
    size_t copy = size * nmemb < len ? size * nmemb : len;
    // Копира даденото количество данни
    memcpy(ptr, p->data + p->pos, copy);
    p->pos += copy;
    return copy;
}

// Всички настройки, споделени между различните видове имейли
void set_opt(CURL* curl, struct curl_slist* recipients) {
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
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
}

int send_forgot_email(const char* to, const char* subject, const char* body) {
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    struct curl_slist* recipients = curl_slist_append(NULL, to);
    set_opt(curl, recipients);

    char message[4096];
    snprintf(message, sizeof(message),
        "From: EventMGR <noreply@eventmgr.bg>\r\n"
        "To: %s\r\n"
        "Subject: %s\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "%s\r\n",
        to, subject, body);
    MailPayload payload = { message, 0 };
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "Грешка при изпращане на имейл: %s\n", curl_easy_strerror(res));

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

int send_ticket_email(const char* to, const char* subject,
    const char* body, const char* token) {
    CURL* curl = curl_easy_init();
    if (!curl) return 0;

    CURLcode res;;

    struct curl_slist* recipients = curl_slist_append(NULL, to);
    set_opt(curl, recipients);

    curl_mime* mime = curl_mime_init(curl);

    // 1. Headers
    struct curl_slist* mail_headers = NULL;
    char from_hdr[128], to_hdr[256], subj_hdr[256];
    snprintf(from_hdr, sizeof(from_hdr), "From: EventMGR <noreply@eventmgr.bg>");
    snprintf(to_hdr, sizeof(to_hdr), "To: %s", to);
    snprintf(subj_hdr, sizeof(subj_hdr), "Subject: %s", subject);
    mail_headers = curl_slist_append(mail_headers, from_hdr);
    mail_headers = curl_slist_append(mail_headers, to_hdr);
    mail_headers = curl_slist_append(mail_headers, subj_hdr);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, mail_headers);

    // 2. Текстовата част на имейла
    curl_mimepart* body_part = curl_mime_addpart(mime);
    curl_mime_data(body_part, body, CURL_ZERO_TERMINATED);
    curl_mime_type(body_part, "text/plain");

    // 3. Прикачения PDF
    curl_mimepart* attach_part = curl_mime_addpart(mime);
    char pdf_path[128];
    snprintf(pdf_path, sizeof(pdf_path), "tickets/ticket_%s.pdf", token);


    CURLcode file_res = curl_mime_filedata(attach_part, pdf_path);
    if (file_res != CURLE_OK) {
        fprintf(stderr, "Грешка при прикачване на PDF: %s\n", pdf_path);
        curl_mime_free(mime);
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
        return 1;
    }
    curl_mime_type(attach_part, "application/pdf");

    char attach_name[64];
    snprintf(attach_name, sizeof(attach_name), "ticket_%s.pdf", token);
    curl_mime_filename(attach_part, attach_name);
    curl_mime_encoder(attach_part, "base64");

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // Проектът няма CA сертификат
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); //1L
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); //2L

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "Грешка при изпращане на имейл: %s\n", curl_easy_strerror(res));

    curl_slist_free_all(mail_headers);
    curl_slist_free_all(recipients);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}