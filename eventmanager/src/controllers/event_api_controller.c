#include "event_api_controller.h"
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

static int create_event_field_found(const char* key,
    const char* filename,
    char* path,
    size_t pathlen,
    void* user_data);

static int create_event_field_get(const char* key,
    const char* value,
    size_t valuelen,
    void* user_data);

static int create_event_field_store(const char* path,
    long long file_size,
    void* user_data);

static int make_image_filename(const char* filename,
    char* path,
    size_t pathlen);

// GET /api/events
int api_events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->request_method, "GET") == 0) {
        // /api/events
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
        // /api/events/{id}
        else {
            const char* id_str = info->local_uri + strlen("/api/events/");
            int event_id = atoi(id_str);

            if (event_id <= 0) {
                mg_send_http_error(conn, 404, "Not found");
                return 404;
            }

            Event e;
            if (get_event((PGconn*)data, event_id, &e)) {
                json_t* json = event_to_json(&e);
                return send_json(conn, json);
            }
            else {
                mg_send_http_error(conn, 404, "Not found");
                return 404;
            }
        }
    }
    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}

// GET /api/users/{id}/events
int api_user_events(struct mg_connection* conn, void* data) {
    Session* s = get_session(conn);
    // Само администратори и организатори имат достъп до качени събития
    if (!s || check_role(conn, ROLE_USER)) {
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
        Event* events = NULL;
        int count = get_user_events(db, user_id, &events);

        json_t* json = json_array();
        for (size_t i = 0; i < count; i++) {
            json_array_append_new(json, event_to_json(&events[i]));
        }
        free(events);
        return send_json(conn, json);
    }

    mg_send_http_error(conn, 405, "Method Not Allowed");
    return 405;
}



// GET /api/categories
int api_categories(struct mg_connection* conn, void* data) {
    Category* categories = NULL;
    int count = get_categories((PGconn*)data, &categories);
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

    if (strcmp(info->request_method, "GET") == 0) {
        const char* id_str = info->local_uri + strlen("/api/events/seatmap/");
        int event_id = atoi(id_str);

        if (event_id <= 0) {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }

        SeatMap* seatMap = NULL;
        if (get_event_seatmap((PGconn*)data, event_id, &seatMap)) {
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
    else {
        mg_send_http_error(conn, 405, "Method Not Allowed");
        return 405;
    }
}

// GET, POST, PATCH /api/admin/events
int api_admin_events(struct mg_connection* conn, void* data) {
    if (!check_role(conn, ROLE_ADMIN) && !check_role(conn, ROLE_ORGANIZATOR)) {
        mg_send_http_error(conn, 403, "Forbidden");
        return 403;
    }

    PGconn* db = (PGconn*)data;
    const struct mg_request_info* info = mg_get_request_info(conn);
    Session* s = get_session(conn);

    // /api/admin/events
    if (strcmp(info->local_uri, "/api/admin/events") == 0) {
        if (strcmp(info->request_method, "GET") == 0) {
            Event* events = NULL;
            int count = 0;
            if (s->role == 0) {
                EventFilters filters = { 0 };
                filters.upcoming = 0;
                char search[256] = "";
                char city[128] = "";
                char category[32] = "";

                count = get_events((PGconn*)data, &filters, &events);
            }
            else if (s->role == 1) {
                count = get_user_events(db, s->user_id, &events);
            }
            json_t* json = json_array();
            for (size_t i = 0; i < count; i++) {
                json_array_append_new(json, event_to_json(&events[i]));
            }

            free(events);
            return send_json(conn, json);
        }
        if (strcmp(info->request_method, "POST") == 0) {
            CreateEventForm form = { 0 };

            struct mg_form_data_handler fdh = {
                .field_found = create_event_field_found,
                .field_get = create_event_field_get,
                .field_store = create_event_field_store,
                .user_data = &form
            };

            int result = mg_handle_form_request(conn, &fdh);

            if (result < 0) {
                mg_send_http_error(conn, 400, "Invalid form data");
                return 400;
            }

            if (!form.venue_id[0] || !form.title[0] ||
                !form.begins_at[0] || !form.price[0] ||
                !form.capacity[0]) {

                if (form.image_uploaded)
                    remove(form.image_disk_path);

                mg_send_http_error(conn, 400, "Missing required fields");
                return 400;
            }

            EventData data = { 0 };

            data.organizer_id = s->user_id;
            data.venue_id = atoi(form.venue_id);
            snprintf(data.title, sizeof(data.title), "%s", form.title);
            snprintf(data.description, sizeof(data.description), "%s", form.description);
            snprintf(data.begins_at, sizeof(data.begins_at), "%s", form.begins_at);
            data.price = strtof(form.price, NULL);
            data.capacity = atoi(form.capacity);

            if (form.image_uploaded) {
                snprintf(data.img_path, sizeof(data.img_path), "%s", form.image_path);
            }
            else {
                snprintf(data.img_path, sizeof(data.img_path), "/res/default1.png");
            }

            int event_id = add_event(db, &data);
            if (!event_id) {
                if (form.image_uploaded)
                    remove(form.image_path);

                json_t* res = json_object();
                set_result(res, 0);
                return send_json(conn, res);
            }

            json_t* res = json_object();
            set_result(res, event_id);

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
        else {
            mg_send_http_error(conn, 404, "Not found");
            return 404;
        }
    }

    if (strcmp(info->request_method, "PATCH") == 0) {

        const char* content_type = mg_get_header(conn, "Content-Type");

        // Редактиране на изображение
        if (content_type &&
            strncmp(content_type, "multipart/form-data", 19) == 0) {

            CreateEventForm form = { 0 };

            struct mg_form_data_handler fdh = {
                .field_found = create_event_field_found,
                .field_get = create_event_field_get,
                .field_store = create_event_field_store,
                .user_data = &form
            };
            int result = mg_handle_form_request(conn, &fdh);

            if (result < 0) {
                mg_send_http_error(conn, 400, "Invalid form data");
                return 400;
            }
            if (!form.image_uploaded) {
                mg_send_http_error(conn, 400, "No image uploaded");
                return 400;
            }

            // Търсене на старото изображение
            char old_image[256] = { 0 };
            if (!get_event_image_path(db, id, old_image, sizeof(old_image))) {
                remove(form.image_disk_path);
                mg_send_http_error(conn, 404, "Event not found");
                return 404;
            }

            // Смяна с новото изображение
            result = admin_update_image(db, id, form.image_path);
            if (!result) {
                remove(form.image_disk_path);
                json_t* res = json_object();
                set_result(res, 0);
                return send_json(conn, res);
            }

            // Ако старото изображение не е default1.png, да се изтрие
            if (old_image[0] && strcmp(old_image, "/res/default1.png") != 0) {
                char old_disk_path[512];
                snprintf(old_disk_path, sizeof(old_disk_path), ".\\html%s", old_image);
                remove(old_disk_path);
            }

            json_t* res = json_object();
            set_result(res, 1);
            return send_json(conn, res);
        }
        // Редактиране на заглавие, дата на започване
        else {
            json_t* req = get_json(conn);
            if (!req)
                return 400;

            json_t* res = json_object();
            int result = 0;

            const char* title = json_string_value(json_object_get(req, "title"));
            const char* begins_at = json_string_value(json_object_get(req, "begins_at"));
            const char* description = json_string_value(json_object_get(req, "description"));
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
                    result = set_event_active(db, id, json_boolean_value(active_json));
                }
            }
            else {
                if (title)
                    result = admin_update_title(db, id, title);
                if (begins_at)
                    result = admin_update_begins_at(db, id, begins_at);
                if (description)
                    result = admin_update_description(db, id, description);
            }

            set_result(res, result);

            json_decref(req);
            return send_json(conn, res);
        }
    }

    mg_send_http_error(conn, 405, "Method Not Allowed");
    return 405;
}

static int create_event_field_found(const char* key, const char* filename,
    char* path, size_t pathlen, void* user_data) {

    CreateEventForm* form = user_data;

    if (strcmp(key, "image") == 0) {
        if (!filename || !filename[0])
            return MG_FORM_FIELD_STORAGE_SKIP;

        char fname[40];

        if (!make_image_filename(filename, fname, sizeof(fname)))
            return MG_FORM_FIELD_STORAGE_SKIP;

        // Път, в който civetweb ще запази файла
        snprintf(path, pathlen, ".\\html\\res\\%s", fname);

        // Път, който ще се запише в БД, спрямо document_root
        snprintf(form->image_path, sizeof(form->image_path), "/res/%s", fname);

        // Запазва се пътя към файла за remove().
        snprintf(form->image_disk_path, sizeof(form->image_disk_path), "%s", path);

        return MG_FORM_FIELD_STORAGE_STORE;
    }

    if (strcmp(key, "venue_id") == 0 ||
        strcmp(key, "title") == 0 ||
        strcmp(key, "description") == 0 ||
        strcmp(key, "begins_at") == 0 ||
        strcmp(key, "price") == 0 ||
        strcmp(key, "capacity") == 0) {
        return MG_FORM_FIELD_STORAGE_GET;
    }

    return MG_FORM_FIELD_STORAGE_SKIP;
}

static int create_event_field_get(const char* key, const char* value,
    size_t valuelen, void* user_data) {
    CreateEventForm* form = user_data;

    char* destination = NULL;
    size_t destination_size = 0;

    if (strcmp(key, "venue_id") == 0) {
        destination = form->venue_id;
        destination_size = sizeof(form->venue_id);
    }
    else if (strcmp(key, "title") == 0) {
        destination = form->title;
        destination_size = sizeof(form->title);
    }
    else if (strcmp(key, "description") == 0) {
        destination = form->description;
        destination_size = sizeof(form->description);
    }
    else if (strcmp(key, "begins_at") == 0) {
        destination = form->begins_at;
        destination_size = sizeof(form->begins_at);
    }
    else if (strcmp(key, "price") == 0) {
        destination = form->price;
        destination_size = sizeof(form->price);
    }
    else if (strcmp(key, "capacity") == 0) {
        destination = form->capacity;
        destination_size = sizeof(form->capacity);
    }
    else {
        return MG_FORM_FIELD_HANDLE_NEXT;
    }

    if (valuelen >= destination_size)
        return MG_FORM_FIELD_HANDLE_ABORT;

    memcpy(destination, value, valuelen);
    destination[valuelen] = '\0';

    return MG_FORM_FIELD_HANDLE_NEXT;
}

static int create_event_field_store(const char* path,
    long long file_size, void* user_data) {
    if (file_size <= 0 ||
        file_size > 10 * 1024 * 1024) {
        return MG_FORM_FIELD_HANDLE_ABORT;
    }

    CreateEventForm* form = user_data;

    if (!path || !path[0])
        return MG_FORM_FIELD_HANDLE_ABORT;

    form->image_uploaded = 1;

    return MG_FORM_FIELD_HANDLE_NEXT;
}

static int make_image_filename(const char* filename, char* path, size_t pathlen) {
    const char* ext = strrchr(filename, '.');
    if (!ext)
        return 0;

    if (_stricmp(ext, ".jpg") != 0 &&
        _stricmp(ext, ".jpeg") != 0 &&
        _stricmp(ext, ".png") != 0 &&
        _stricmp(ext, ".webp") != 0) {
        return 0;
    }

    unsigned char rnd[16];
    if (BCryptGenRandom(NULL, rnd, sizeof(rnd),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        return 0;
    }

    char hex_str[33]; // 16 байта -> 32 hex символа + null
    for (int i = 0; i < 16; i++) {
        snprintf(hex_str + i * 2, 3, "%02x", rnd[i]);
    }

    snprintf(path, pathlen, "%s%s", hex_str, ext);
    return 1;
}
