#include "controller.h"
#include "ticket_api_controller.h"
#include "../email.h"
#include "../ticket_pdf.h"
#include "../qrcode.h"

// POST /api/purchase/{event_id}
int api_purchase_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    int event_id = atoi(info->local_uri + strlen("/api/purchase/"));
    if (event_id <= 0) { mg_send_http_error(conn, 400, "Invalid ID"); return 400; }

    json_t* req = get_json(conn);

    TicketData ticket = { 0 };
    ticket.event_id = event_id;

    Session* s = get_session(conn);
    if (s)
        ticket.user_id = s->user_id;
    else
        ticket.user_id = -1;

    json_t* sector = json_object_get(req, "sector_id");
    ticket.sector_id = json_integer_value(json_object_get(req, "sector_id"));

    strncpy(ticket.first_name, 
        json_string_value(json_object_get(req, "first_name")), sizeof(ticket.first_name) - 1);
    strncpy(ticket.last_name, 
        json_string_value(json_object_get(req, "last_name")), sizeof(ticket.last_name) - 1);
    strncpy(ticket.email, 
        json_string_value(json_object_get(req, "email")), sizeof(ticket.email) - 1);
    strncpy(ticket.phone, 
        json_string_value(json_object_get(req, "phone")), sizeof(ticket.phone) - 1); 

    int ticket_id;
    int result = purchase_ticket(db, &ticket, &ticket_id);

    json_t* res = json_object();
    if (result == 1) {
        json_object_set_new(res, "success", json_true());
        json_object_set_new(res, "ticket_id", json_integer(ticket_id));
    }
    else if (result == -1) {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Няма свободни места."));
    }
    else {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", json_string("Възникна грешка."));
    }

    json_decref(req);
    return send_json(conn, res);
}

// GET /api/confirmation/{ticket_id}
int api_confirm_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    int ticket_id = atoi(info->local_uri + strlen("/api/confirmation/"));
    if (ticket_id <= 0) { mg_send_http_error(conn, 400, "Invalid ID"); return 400; }

    TicketView ticket;
    if (!get_ticket(db, ticket_id, &ticket)) { mg_send_http_error(conn, 404, "Not found"); return 404; }

    char pdf_path[128];
    snprintf(pdf_path, sizeof(pdf_path), "tickets/ticket_%s.pdf", ticket.token);

    // Проверка има ли вече генериран PDF билет
    FILE* check = fopen(pdf_path, "rb");
    if (check) {
        fclose(check);
    }
    else {
        char qr_path[128];
        if (!generate_ticket_qr(ticket.token, qr_path, sizeof(qr_path))) {
            fprintf(stderr, "Failed to generate QR for ticket %d\n", ticket_id);
        }

        char html_path[128];

        if (generate_ticket_html(db, ticket_id, qr_path, html_path, sizeof(html_path)) != 0) {
            mg_send_http_error(conn, 404, "Ticket not found");
            remove(html_path);
            return 404;
        }
        if (start_pdf_process(ticket.token) != 0) {
            mg_send_http_error(conn, 500, "Failed to generate PDF");
            remove(html_path);
            return 500;
        }

        char buf[128];
        snprintf(buf, sizeof(buf), "Билет за %s", ticket.event_name);
        const char* subject = buf;

        send_ticket_email(ticket.email, subject, "Вашият билет е прикачен тук.", ticket.token);

        remove(html_path);
    }
    json_t* res = ticket_to_json(&ticket);
    return send_json(conn, res);
}

int api_my_tickets(struct mg_connection* conn, void* data) {
    Session* s = get_session(conn);
    // Само регистрирани потребители, които са клиенти могат да купуват билети
    if (!s || s->role != 2) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_method, "GET") == 0) {

        PGconn* db = (PGconn*)data;
        TicketView* tickets = NULL;
        int count = get_user_tickets(db, s->user_id, &tickets);

        json_t* json = json_array();
        for (size_t i = 0; i < count; i++) {
            json_array_append_new(json, ticket_to_json(&tickets[i]));
        }
        free(tickets);
        return send_json(conn, json);
    }

    mg_send_http_error(conn, 405, "Method Not Allowed");
    return 405;
}

int api_ticket_file(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);
    Session* s = get_session(conn);

    if (!s) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    // /tickets/ticket_<uuid>.pdf
    const char* filename =
        info->local_uri + strlen("/tickets/");

    char ticket_uuid[37];

    if (sscanf(filename,
        "ticket_%36[0-9a-fA-F-].pdf",
        ticket_uuid) != 1) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    PGconn* db = (PGconn*)data;

    if (!ticket_belongs_to_user(db, ticket_uuid, s->user_id)) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    char path[512];
    snprintf(path, sizeof(path),
        "html/tickets/%s", filename);

    mg_send_file(conn, path);

    return 200;
}
