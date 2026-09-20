#include "html_controller.h"
#include "../view.h"
#include <windows.h>
#include "../../util.h"

int send_html(struct mg_connection* conn, const char* path) {
    const char* html = view_render(path);
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    mg_write(conn, html, strlen(html));
    free(html);
    return 1;
}

int html_controller(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);
    const char* uri = info->local_uri;
    Config* config = (Config*)data;

    char path[MAX_PATH];

    if (strcmp(uri, "/home") == 0) {
        snprintf(path, sizeof(path), "%s\\home.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/events") == 0) {
        snprintf(path, sizeof(path), "%s\\events.html", config->html_dir);
        return send_html(conn, path);
    }
    if ((strncmp(uri, "/events/", 8) == 0 && strlen(uri) > 8)) {
        snprintf(path, sizeof(path), "%s\\event.html", config->html_dir);
        return send_html(conn, path);
    }
    if ((strncmp(uri, "/purchase/", 8) == 0 && strlen(uri) > 8)) {
        snprintf(path, sizeof(path), "%s\\purchase.html", config->html_dir);
        return send_html(conn, path);
    }
    if ((strncmp(uri, "/confirmation/", 8) == 0 && strlen(uri) > 8)) {
        snprintf(path, sizeof(path), "%s\\confirmation.html", config->html_dir);
        return send_html(conn, path);
    }

    if (strcmp(uri, "/login") == 0) {
        snprintf(path, sizeof(path), "%s\\login.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/signup") == 0) {
        snprintf(path, sizeof(path), "%s\\signup.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/profile") == 0) {
        snprintf(path, sizeof(path), "%s\\profile.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/forgot") == 0) {
        snprintf(path, sizeof(path), "%s\\forgot.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/reset") == 0) {
        snprintf(path, sizeof(path), "%s\\reset.html", config->html_dir);
        return send_html(conn, path);
    }

    if (strcmp(uri, "/admin") == 0) {
        snprintf(path, sizeof(path), "%s\\admin.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strcmp(uri, "/admin/users") == 0) {
        snprintf(path, sizeof(path), "%s\\admin_users.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strncmp(uri, "/admin/users/", 13) == 0) {
        char* end;
        long user_id = strtol(uri + 13, &end, 10);
        if (*end == '\0') {
            snprintf(path, sizeof(path), "%s\\admin_user.html", config->html_dir);
            return send_html(conn, path);
        }
    }
    if (strcmp(uri, "/admin/events") == 0) {
        snprintf(path, sizeof(path), "%s\\admin_events.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strncmp(uri, "/admin/events/", 14) == 0) {
        if (strcmp(uri, "/admin/events/create") == 0) {
            snprintf(path, sizeof(path), "%s\\create_event.html", config->html_dir);
            return send_html(conn, path);
        }
        char* end;
        long event_id = strtol(uri + 14, &end, 10);
        if (*end == '\0') {
            snprintf(path, sizeof(path), "%s\\admin_event.html", config->html_dir);
            return send_html(conn, path);
        }
    }

    if (strcmp(uri, "/admin/venues") == 0) {
        snprintf(path, sizeof(path), "%s\\admin_venues.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strncmp(uri, "/admin/venues/", 14) == 0) {
        if (strcmp(uri, "/admin/venues/create") == 0) {
            snprintf(path, sizeof(path), "%s\\create_venue.html", config->html_dir);
            return send_html(conn, path);
        }
        char* end;
        long event_id = strtol(uri + 14, &end, 10);
        if (*end == '\0') {
            snprintf(path, sizeof(path), "%s\\admin_venue.html", config->html_dir);
            return send_html(conn, path);
        }
    }

    if (strcmp(uri, "/admin/tickets") == 0) {
        snprintf(path, sizeof(path), "%s\\admin_tickets.html", config->html_dir);
        return send_html(conn, path);
    }
    if (strncmp(uri, "/admin/tickets/", 15) == 0) {
        char* end;
        long ticket_id = strtol(uri + 15, &end, 10);
        if (*end == '\0') {
            snprintf(path, sizeof(path), "%s\\admin_ticket.html", config->html_dir);
            return send_html(conn, path);
        }
    }

    if (strcmp(uri, "/organizer") == 0) {
        snprintf(path, sizeof(path), "%s\\organizer.html", config->html_dir);
        return send_html(conn, path);
    }
}

