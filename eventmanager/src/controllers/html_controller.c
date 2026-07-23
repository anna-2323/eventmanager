#include "html_controller.h"
#include "../view.h"

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

    if (strcmp(uri, "/home") == 0)
        return send_html(conn, "html/home.html");
    if (strcmp(uri, "/events") == 0)
        return send_html(conn, "html/events.html");
    //. ..
    if ((strncmp(uri, "/events/", 8) == 0 && strlen(uri) > 8))
        return send_html(conn, "html/event.html");
    if ((strncmp(uri, "/purchase/", 8) == 0 && strlen(uri) > 8))
        return send_html(conn, "html/purchase.html");
    if ((strncmp(uri, "/confirmation/", 8) == 0 && strlen(uri) > 8))
        return send_html(conn, "html/confirmation.html");

    if (strcmp(uri, "/login") == 0)
        return send_html(conn, "html/login.html");
    if (strcmp(uri, "/signup") == 0)
        return send_html(conn, "html/signup.html");
    if (strcmp(uri, "/profile") == 0)
        return send_html(conn, "html/profile.html");
    if (strcmp(uri, "/forgot") == 0)
        return send_html(conn, "html/forgot.html");
    if (strcmp(uri, "/reset") == 0)
        return send_html(conn, "html/reset.html");

    if (strcmp(uri, "/admin") == 0)
        return send_html(conn, "html/admin.html");
    if (strcmp(uri, "/admin/users") == 0)
        return send_html(conn, "html/admin_users.html");
    if (strcmp(uri, "/admin/events") == 0)
        return send_html(conn, "html/admin_events.html");
}
