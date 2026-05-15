// view.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"

static char* load_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

char* view_render(const char* path) {
    char* tmpl = load_file(path);
    if (!tmpl) return _strdup("<h1>Шаблонът не е намерен</h1>");
    return tmpl;
}