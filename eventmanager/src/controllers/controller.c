#include "controller.h"

const char* status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

// Помощна функция, обобщаваща действията при получаване на JSON
json_t* get_json(struct mg_connection* conn) {
    char body[1024] = "";
    mg_read(conn, body, sizeof(body) - 1);

    json_error_t err;
    json_t* req = json_loads(body, 0, &err);

    if (!req) {
        mg_send_http_error(conn, 400, "Invalid JSON");
        return NULL;
    }
    return req;
}

int check_role(struct mg_connection* conn, int role) {
    Session* s = get_session(conn);
    if (s && s->role == role)
        return 1;
    return 0;
}

// Помощна функция, обобщаваща действията при изпращане на JSON
int send_result(struct mg_connection* conn, int result,
    int status, const char* message, json_t* data) {
    
    json_t* response = json_object();

    json_object_set_new(response, "success", json_boolean(result));

    // Успех
    if (result) {
        if(message)
            json_object_set_new(response, "message",
                json_string(message ? message : "Успех"));
    }
    // Грешка
    else {
        json_object_set_new(response, "message",
            json_string(message ? message : "Възникна грешка."));
    }

    if (data) {
        json_object_set(response, "data", data);
    }

    char* json = json_dumps(response, JSON_COMPACT);

    mg_printf(conn,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        status,
        status_text(status),
        strlen(json),
        json
    );

    free(json);
    json_decref(response);

    return 1;
}

int is_valid_email(const char* email) {
    // Няма @ или започва с @
    const char* at = strchr(email, '@');
    if (!at || at == email) return 0;

    // Няма . след @
    const char* dot = strrchr(at, '.');
    if (!dot || dot == at + 1) return 0;

    // Домейнът е повече от 2 символа
    size_t tld_len = strlen(dot + 1);
    if (tld_len < 2) return 0;

    // Празни места
    for (const char* c = email; *c; c++) {
        if (isspace((unsigned char)*c)) return 0;
    }

    return 1;
}

int is_valid_phone(const char* phone) {
    size_t len = strlen(phone);
    // Нормално количество символи
    if (len < 7 || len > 15) return 0;

    int digit_count = 0;
    for (size_t i = 0; i < len; i++) {
        char c = phone[i];
        if (isdigit((unsigned char)c)) {
            digit_count++;
        }
        // Разрешени символи
        else if (c != '+' && c != ' ' && c != '-' && c != '(' && c != ')') {
            return 0;
        }
    }
    // Достатъчно цифри
    return digit_count >= 7;
}

int is_valid_password(const char* password) {
    if (!password) return 0;
    return strlen(password) >= 8;
}

json_t* event_to_json(Event* e) {
    json_t* obj = json_object();
    json_object_set_new(obj, "id", json_integer(e->id));
    json_object_set_new(obj, "title", json_string(e->title));
    json_object_set_new(obj, "price", json_real(e->price));
    json_object_set_new(obj, "begins_at", json_string(e->begins_at));
    json_object_set_new(obj, "img_path", json_string(e->img_path));
    json_object_set_new(obj, "venue_name", json_string(e->venue.venue_name));
    json_object_set_new(obj, "city", json_string(e->venue.city));
    json_object_set_new(obj, "seats_left", json_integer(e->seats_left));
    json_object_set_new(obj, "active", json_integer(e->active));
    json_object_set_new(obj, "description", json_string(e->description));
    return obj;
}

json_t* ticket_to_json(TicketView* t) {
    json_t* obj = json_object();
    json_object_set_new(obj, "id", json_integer(t->id));
    json_object_set_new(obj, "event_name", json_string(t->event_name));
    json_object_set_new(obj, "begins_at", json_string(t->begins_at));
    json_object_set_new(obj, "venue_name", json_string(t->venue_name));
    json_object_set_new(obj, "venue_city", json_string(t->venue_city));
    json_object_set_new(obj, "venue_address", json_string(t->venue_address));
    json_object_set_new(obj, "first_name", json_string(t->first_name));
    json_object_set_new(obj, "last_name", json_string(t->last_name));
    json_object_set_new(obj, "email", json_string(t->email));
    json_object_set_new(obj, "phone", json_string(t->phone));
    json_object_set_new(obj, "sector", json_string(t->sector));
    json_object_set_new(obj, "token", json_string(t->token));
    json_object_set_new(obj, "price", json_real(t->price));
    json_object_set_new(obj, "user_id", json_integer(t->user_id));
    json_object_set_new(obj, "event_id", json_integer(t->event_id));
    json_object_set_new(obj, "active", json_integer(t->active));
    return obj;
}

json_t* venue_to_json(Venue* v) {
    json_t* obj = json_object();
    json_object_set_new(obj, "id", json_integer(v->id));
    json_object_set_new(obj, "city", json_string(v->city));
    json_object_set_new(obj, "address", json_string(v->address));
    json_object_set_new(obj, "venue_name", json_string(v->venue_name));
    json_object_set_new(obj, "active", json_integer(v->active));
    json_object_set_new(obj, "has_sectors", json_integer(v->has_sectors));
    json_object_set_new(obj, "capacity", json_integer(v->capacity));
    return obj;
}

json_t* user_to_json(User* u) {
    json_t* obj = json_object();
    json_object_set_new(obj, "id", json_integer(u->id));
    json_object_set_new(obj, "email", json_string(u->email));
    json_object_set_new(obj, "first_name", json_string(u->first_name));
    json_object_set_new(obj, "last_name", json_string(u->last_name));
    json_object_set_new(obj, "phone", json_string(u->phone));
    json_object_set_new(obj, "role", json_integer(u->role));
    json_object_set_new(obj, "deleted_on", json_string(u->deleted_on));
    json_object_set_new(obj, "active", json_integer(u->active));
    return obj;
}

json_t* growth_to_json(StatGrowth* s) {
    json_t* obj = json_object();
    json_object_set_new(obj, "period", json_string(s->period));
    json_object_set_new(obj, "count", json_integer(s->count));
    return obj;
}

json_t* revenue_to_json(StatRevenue* s) {
    json_t* obj = json_object();
    json_object_set_new(obj, "period", json_string(s->period));
    json_object_set_new(obj, "revenue", json_real(s->revenue));
    json_object_set_new(obj, "count", json_integer(s->count));
    json_object_set_new(obj, "venue_id", json_integer(s->venue_id));
    return obj;
}
