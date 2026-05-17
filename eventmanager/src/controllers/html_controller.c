#include "html_controller.h"
#include "../view.h"

int send_html(struct mg_connection* conn, const char* path) {
    const char* html = view_render(path);
    mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    mg_write(conn, html, strlen(html));
    free(html);
    return 1;
}

int home(struct mg_connection* conn, void* data) {
    return send_html(conn, "html/home.html");
}

int events(struct mg_connection* conn, void* data) {
    const struct mg_request_info* info = mg_get_request_info(conn);

    if (strcmp(info->local_uri, "/events") == 0)
        return send_html(conn, "html/events.html");
    else
        return send_html(conn, "html/event.html");
}

int login(struct mg_connection* conn, void* data) {
    return send_html(conn, "html/login.html");
}
int signup(struct mg_connection* conn, void* data) {
    return send_html(conn, "html/signup.html");
}
