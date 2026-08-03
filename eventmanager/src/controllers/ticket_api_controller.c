#include "controller.h"
#include "ticket_api_controller.h"
#include "../email.h"
#include "../ticket_pdf.h"

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
    free(s);

    json_t* sector = json_object_get(req, "sector_id");

    printf("exists: %s\n", sector ? "yes" : "no");
    printf("is integer: %s\n", json_is_integer(sector) ? "yes" : "no");

    if (sector) {
        printf("integer value: %lld\n", json_integer_value(sector));
    }

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

    json_t* ticket = get_ticket(db, ticket_id);
    if (!ticket) { mg_send_http_error(conn, 404, "Not found"); return 404; }

    const char* token = json_string_value(json_object_get(ticket, "token"));
    char pdf_path[128];
    snprintf(pdf_path, sizeof(pdf_path), "tickets/ticket_%s.pdf", token);

    // Проверка има ли вече генериран PDF билет
    FILE* check = fopen(pdf_path, "rb");
    if (check) {
        fclose(check);
    }
    else {
        char html_path[128];

        if (generate_ticket_html(db, ticket_id, html_path) != 0) {
            mg_send_http_error(conn, 404, "Ticket not found");
            remove(html_path);
            return 404;
        }
        if (start_pdf_process(token) != 0) {
            mg_send_http_error(conn, 500, "Failed to generate PDF");
            remove(html_path);
            return 500;
        }

        const char* to = json_string_value(json_object_get(ticket, "email"));
        char buf[128];
        snprintf(buf, sizeof(buf), "Билет за %s", json_string_value(json_object_get(ticket, "event_name")));
        const char* subject = buf;

        send_ticket_email(to, subject, "Вашият билет е прикачен тук.", token);

        remove(html_path);

    }
    printf("%s\n", json_dumps(ticket, JSON_INDENT(2)));
    return send_json(conn, ticket);
}
