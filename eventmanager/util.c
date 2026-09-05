#include "util.h"

int load_config(Config* config) {
    if (!config)
        return 0;

    DWORD len = GetModuleFileNameA(
        NULL,
        config->app_dir,
        sizeof(config->app_dir)
    );
    if (len == 0 || len >= sizeof(config->app_dir))
        return 0;

    // Премахва се името на файла
    char* slash = strrchr(config->app_dir, '\\');
    if (!slash)
        return 0;
    *slash = '\0';

    // Пътят на конфигурационния файл е същият като на програмата
    char config_path[MAX_PATH];
    snprintf(
        config_path,
        sizeof(config_path),
        "%s\\config.txt",
        config->app_dir);

    snprintf(config->html_dir,
        sizeof(config->html_dir),
        "%s\\html",
        config->app_dir);

    snprintf(config->tickets_dir,
        sizeof(config->tickets_dir),
        "%s\\html\\tickets",
        config->app_dir);

    FILE* file = fopen(config_path, "r");
    if (!file) {
        fprintf(stderr, "Грешка при отваряне на файл: %s\n",
            config_path);
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0' || line[0] == '#')
            continue;

        // Къде е символът =
        char* equals = strchr(line, '=');
        if (!equals)
            continue;
        *equals = '\0';

        const char* key = line;
        const char* value = equals + 1;

        if (strcmp(key, "db_host") == 0) {
            snprintf(config->db_host,
                sizeof(config->db_host),
                "%s", value);
        }
        else if (strcmp(key, "db_port") == 0) {
            snprintf(config->db_port,
                sizeof(config->db_port),
                "%s", value);
        }
        else if (strcmp(key, "db_name") == 0) {
            snprintf(config->db_name,
                sizeof(config->db_name),
                "%s", value);
        }
        else if (strcmp(key, "db_user") == 0) {
            snprintf(config->db_user,
                sizeof(config->db_user),
                "%s", value);
        }
        else if (strcmp(key, "db_password") == 0) {
            snprintf(config->db_password,
                sizeof(config->db_password),
                "%s", value);
        }
        else if (strcmp(key, "server_port") == 0) {
            config->server_port = atoi(value);
        }
    }
    fclose(file);

    return 1;
}

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
