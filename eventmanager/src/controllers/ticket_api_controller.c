#include "controller.h"
#include "ticket_api_controller.h"
#include "../email.h"
#include "../ticket_pdf.h"
#include "../qrcode.h"

// POST /api/purchase/{event_id}
int api_purchase_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->request_method, "POST") == 0) {
        const char* id_str = info->local_uri + strlen("/api/purchase/");
        int event_id = atoi(id_str);

        if (event_id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

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
    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}

// GET /api/confirmation/{ticket_id}
int api_confirm_ticket(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->request_method, "GET") == 0) {
        int ticket_id = atoi(info->local_uri + strlen("/api/confirmation/"));
        if (ticket_id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        TicketView ticket;
        if (!get_ticket(db, ticket_id, &ticket)) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        char pdf_path[128];
        snprintf(pdf_path, sizeof(pdf_path), "html/tickets/ticket_%s.pdf", ticket.token);

        // Проверка има ли вече генериран PDF билет
        FILE* check = fopen(pdf_path, "rb");
        if (check) {
            fclose(check);
        }
        else {
            char qr_path[128];
            if (!generate_ticket_qr(ticket.token, qr_path, sizeof(qr_path))) {
                mg_send_http_error(conn, 500, "Failed to generate QR");
                return 500;
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
    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}

// GET, PATCH /api/admin/tickets/{id}
int api_admin_tickets(struct mg_connection* conn, void* data) {
    if (check_role(conn, ROLE_USER)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    // /api/admin/events
    if (strcmp(info->local_uri, "/api/admin/tickets") == 0) {
        if (strcmp(info->request_method, "GET") == 0) {
            Session* s = get_session(conn);
            int organizer_id = 0;
            if (s->role == 1) organizer_id = s->user_id;
            TicketView* tickets = NULL;
            int count = get_tickets((PGconn*)data, organizer_id, &tickets);

            json_t* json = json_array();
            for (size_t i = 0; i < count; i++) {
                json_array_append_new(json, ticket_to_json(&tickets[i]));
            }

            return send_json(conn, json);
        }
        else {
            mg_send_http_error(conn, 405, "Method Not Allowed");
            return 405;
        }
    }
    // /api/admin/tickets/{id}
    const char* id_str = info->local_uri + strlen("/api/admin/tickets/");
    int id = atoi(id_str);

    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    if (strcmp(info->request_method, "GET") == 0) {
        TicketView t;
        if (get_ticket(db, id, &t))
            return send_json(conn, ticket_to_json(&t));
    }
    if (strcmp(info->request_method, "PATCH") == 0) {
        json_t* req = get_json(conn);
        if (!req)
            return 400;

        json_t* res = json_object();
        int result = 0;

        json_t* active_json = json_object_get(req, "active");

        if (active_json) {
            if (!json_is_boolean(active_json)) {
                result = 0;
            }
            else if (!check_role(conn, ROLE_ADMIN)) {
                mg_send_http_error(conn, 403, "Forbidden");
                json_decref(req);
                return 403;
            }
            else {
                result = set_ticket_active(db, id, json_boolean_value(active_json));
            }
        }

        set_result(res, result);

        json_decref(req);
        return send_json(conn, res);
    }

    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}

// GET /api/users/{id}/tickets
int api_user_tickets(struct mg_connection* conn, void* data) {
    Session* s = get_session(conn);
    // Гости не могат да преглеждат билети
    if (!s) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }

    const struct mg_request_info* info = mg_get_request_info(conn);
    if (strcmp(info->request_method, "GET") == 0) {
        const char* id_str = info->local_uri + strlen("/api/users/");
        int user_id = atoi(id_str);
        if (user_id <= 0) {
            return 400;
        }

        if (!check_role(conn, ROLE_ADMIN) && user_id != s->user_id) {
            mg_send_http_error(conn, 403, "Forbidden");
            return 403;
        }

        PGconn* db = (PGconn*)data;
        TicketView* tickets = NULL;
        int count = get_user_tickets(db, user_id, &tickets);

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

// GET /tickets/{uuid}
int api_ticket_file(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);
    Session* s = get_session(conn);

    if (!s) {
        mg_send_http_error(conn, 401, "Unauthorized");
        return 401;
    }
    if (strcmp(info->request_method, "GET") == 0) {
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
    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}
