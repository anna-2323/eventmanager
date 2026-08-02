#include "util.h"

char* read_file_to_string(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    rewind(f);

    char* buf = malloc(len + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t read = fread(buf, 1, len, f);
    fclose(f);
    if (read != (size_t)len) { free(buf); return NULL; }

    buf[len] = '\0';
    return buf;
}

int write_string_to_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return 1;
    fputs(content, f);
    fclose(f);
    return 0;
}
