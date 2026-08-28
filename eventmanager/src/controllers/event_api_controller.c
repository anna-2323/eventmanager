#include "event_api_controller.h"

// GET /api/events
int api_events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/api/events") == 0) {
        EventFilters filters = { 0 };

        filters.upcoming = 1;

        char search[256] = "";
        char city[128] = "";
        char category[32] = "";

        if (info->query_string) {
            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "search",
                search,
                sizeof(search)
            );

            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "city",
                city,
                sizeof(city)
            );

            mg_get_var(
                info->query_string,
                strlen(info->query_string),
                "category",
                category,
                sizeof(category)
            );
        }

        filters.search = search;
        filters.city = city;
        filters.category_id = atoi(category);

        Event* events;
        int count = get_events((PGconn*)data, &filters, &events);
        json_t* json = json_array();
        for (size_t i = 0; i < count; i++) {
            json_array_append_new(json, event_to_json(&events[i]));
        }
        free(events);
        return send_json(conn, json);
    }
    else {
        const char* id_str = info->local_uri + strlen("/api/events/");
        int id = atoi(id_str);
        if (id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        Event e;
        if (get_event((PGconn*)data, id, &e)) {
            json_t* json = event_to_json(&e);
            return send_json(conn, json);
        }
    }

    return 0;
}

// GET /api/categories
int api_categories(struct mg_connection* conn, void* data) {
    Category* categories = NULL;
    int count = get_categories((PGconn*)data, &categories);
    if (!count)
        return 500;
    json_t* res = json_array();
    for (int i = 0; i < count; i++) {
        json_t* c = json_object();
        json_object_set_new(c, "id", json_integer(categories[i].id));
        json_object_set_new(c, "title", json_string(categories[i].title));
        json_array_append_new(res, c);
    }
    return send_json(conn, res);
}

// GET /api/events/seatmap/{id}
int api_event_seatmap(struct mg_connection* conn, void* data) {
    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    const char* id_str = info->local_uri + strlen("/api/events/seatmap/");
    int id = atoi(id_str);
    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    SeatMap* seatMap = NULL;
    if (get_event_seatmap((PGconn*)data, id, &seatMap)) {
        json_t* res = json_object();
        json_object_set_new(res, "has_sectors", json_boolean(seatMap->has_sectors));
        if (seatMap->has_sectors) {
            json_object_set_new(res, "background_svg", 
                seatMap->background_svg ? json_string(seatMap->background_svg) : json_null());
            json_object_set_new(res, "viewbox", 
                seatMap->viewbox ? json_string(seatMap->viewbox) : json_null());
            json_t* sectors = json_array();
            for (int i = 0; i < seatMap->sector_count; i++) {
                json_t* sector = json_object();
                json_object_set_new(sector, "id", json_integer(seatMap->sectors[i].id));
                json_object_set_new(sector, "name", json_string(seatMap->sectors[i].name));
                json_object_set_new(sector, "capacity", json_integer(seatMap->sectors[i].capacity));
                json_object_set_new(sector, "price", json_real(seatMap->sectors[i].price));
                json_object_set_new(sector, "color", json_string(seatMap->sectors[i].color));
                json_object_set_new(sector, "svg_path", json_string(seatMap->sectors[i].svg_path));
                json_object_set_new(sector, "available", json_integer(seatMap->sectors[i].available));
                json_array_append(sectors, sector);
            }
            json_object_set_new(res, "sectors", sectors);
        }
        else {
            json_object_set_new(res, "no_sector_id", json_integer(seatMap->sectors[0].id));
            json_object_set_new(res, "sectors", json_null());
        }
        return send_json(conn, res);
    }
}

// GET/PATCH/DELETE /api/admin/events
int api_admin_events(struct mg_connection* conn, void* data) {
    if (check_role(conn, ROLE_USER)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);

    // /api/admin/events
    if (strcmp(info->local_uri, "/api/admin/events") == 0) {
        if (strcmp(info->request_method, "GET") == 0) {
            EventFilters filters = { 0 };

            filters.upcoming = 0;

            char search[256] = "";
            char city[128] = "";
            char category[32] = "";

            if (info->query_string) {
                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "search",
                    search,
                    sizeof(search)
                );

                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "city",
                    city,
                    sizeof(city)
                );

                mg_get_var(
                    info->query_string,
                    strlen(info->query_string),
                    "category",
                    category,
                    sizeof(category)
                );
            }

            filters.search = search;
            filters.city = city;
            filters.category_id = atoi(category);

            Event* events;
            int count = get_events((PGconn*)data, &filters, &events);
            json_t* json = json_array();
            for (size_t i = 0; i < count; i++) {
                json_array_append_new(json, event_to_json(&events[i]));
            }

            return send_json(conn, json);
        }
        if (strcmp(info->request_method, "POST") == 0) {
            if (check_role(conn, ROLE_USER)) {
                mg_send_http_error(conn, 403, "Forbidden");
                return 403;
            }

            json_t* req = get_json(conn);
            if (!req) return 400;

            json_t* res = json_object();

            Session* s = get_session(conn);

            EventData data;
            data.organizer_id = s->user_id;
            data.venue_id = json_integer_value(json_object_get(req, "venue_id"));
            snprintf(data.title, sizeof(data.title), "%s", json_string_value(json_object_get(req, "title")));
            snprintf(data.begins_at, sizeof(data.begins_at), "%s", json_string_value(json_object_get(req, "begins_at")));
            data.price = json_number_value(json_object_get(req, "price"));
            data.capacity = json_integer_value(json_object_get(req, "capacity"));
            int result = add_event(db, &data);

            set_result(res, result);
            json_decref(req);
            return send_json(conn, res);
        }

    }

    // /api/admin/events/{id}
    const char* id_str = info->local_uri + strlen("/api/admin/events/");
    int id = atoi(id_str);

    if (id <= 0) {
        mg_send_http_error(conn, 404, "Not found");
        return 404;
    }

    if (strcmp(info->request_method, "GET") == 0) {
        Event e;
        if(get_event(db, id, &e))
            return send_json(conn, event_to_json(&e));
    }

    if (strcmp(info->request_method, "PATCH") == 0) {
        json_t* req = get_json(conn);
        if (!req)
            return 400;

        json_t* res = json_object();
        int result = 0;

        const char* title = json_string_value(json_object_get(req, "title"));
        if (title)
            result = admin_update_title(db, id, title);

        const char* begins_at = json_string_value(json_object_get(req, "begins_at"));
        if (begins_at)
            result = admin_update_begins_at(db, id, begins_at);

        json_t* verified_json = json_object_get(req, "verified");
        if (!json_is_boolean(verified_json))
            result = 0;
        else if (json_boolean_value(verified_json))
            result = verify_event(db, id);
        else
            result = unverify_event(db, id);

        set_result(res, result);

        json_decref(req);
        return send_json(conn, res);
    }

    if (strcmp(info->request_method, "DELETE") == 0) {
        json_t* res = json_object();
        set_result(res, delete_event(db, id));
        return send_json(conn, res);
    }

    mg_send_http_error(conn, 405, "Method Not Allowed");
    return 405;
}
