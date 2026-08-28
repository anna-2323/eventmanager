#include "controller.h"

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

// Помощна функция, обобщаваща действията при изпращане на JSON
int send_json(struct mg_connection* conn, json_t* json) {
    if (!json) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    char* json_str = json_dumps(json, JSON_COMPACT);
    mg_send_http_ok(conn, "application/json", strlen(json_str));
    mg_write(conn, json_str, strlen(json_str));
    free(json_str);
    json_decref(json);
    return 1;
}

int check_role(struct mg_connection* conn, int role) {
    Session* s = get_session(conn);
    if (s && s->role == role)
        return 1;
    return 0;
}

void set_result(json_t* res, int result)
{
    if (result == 1) {
        json_object_set_new(res, "success", json_true());
    }
    else {
        json_object_set_new(res, "success", json_false());
        json_object_set_new(res, "error", 
            json_string("Възникна грешка."));
    }
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
    json_object_set_new(obj, "verified", json_integer(e->verified));
    json_object_set_new(obj, "description", json_string(e->description));
    return obj;
}

json_t* ticket_to_json(TicketView* t) {
    json_t* obj = json_object();
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
