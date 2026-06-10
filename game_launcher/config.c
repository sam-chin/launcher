#include "config.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>

void LoadDefaultConfig(LauncherConfig* config) {
    memset(config, 0, sizeof(LauncherConfig));

    strcpy_s(config->website_url, MAX_URL_LEN, "https://www.example.com");
    strcpy_s(config->recharge_url, MAX_URL_LEN, "https://www.example.com/recharge");
    strcpy_s(config->support_url, MAX_URL_LEN, "https://www.example.com/support");
    strcpy_s(config->register_url, MAX_URL_LEN, "https://www.example.com/register");
    config->register_mode = 0;

    strcpy_s(config->server_list_url, MAX_URL_LEN, "https://www.example.com/serverlist.txt");
    strcpy_s(config->patch_url, MAX_URL_LEN, "https://www.example.com/patch.pack");
    strcpy_s(config->patch_backup_url, MAX_URL_LEN, "https://backup.example.com/patch.pack");
    config->timeout = 30;

    config->patch_source = 0;
    strcpy_s(config->local_patch_path, MAX_PATH_LEN, "patch.pack");
    strcpy_s(config->patch_key, MAX_KEY_LEN, "defaultkey");
    config->use_crc = 1;
    config->auto_create_dir = 1;

    strcpy_s(config->client_path, MAX_PATH_LEN, "client.exe");
    strcpy_s(config->client_args, MAX_PATH_LEN, "");
    strcpy_s(config->inject_dll_path, MAX_PATH_LEN, "inject.dll");

    config->dll_count = 0;
}

static void write_ini_string(FILE* f, const char* key, const char* value) {
    fprintf(f, "%s=%s\n", key, value);
}

static void write_ini_int(FILE* f, const char* key, int value) {
    fprintf(f, "%s=%d\n", key, value);
}

static void read_ini_line(const char* line, char* out_key, size_t key_size,
                          char* out_value, size_t value_size) {
    out_key[0] = '\0';
    out_value[0] = '\0';
    const char* eq = strchr(line, '=');
    if (!eq) return;
    size_t klen = (size_t)(eq - line);
    if (klen >= key_size) klen = key_size - 1;
    strncpy_s(out_key, key_size, line, klen);
    const char* val = eq + 1;
    strncpy_s(out_value, value_size, val, value_size - 1);
}

int SaveConfigToFile(const LauncherConfig* config, const char* file_path) {
    FILE* f = NULL;
    if (fopen_s(&f, file_path, "w") != 0 || !f) {
        return 0;
    }

    fprintf(f, "[Launcher]\n");
    write_ini_string(f, "website_url", config->website_url);
    write_ini_string(f, "recharge_url", config->recharge_url);
    write_ini_string(f, "support_url", config->support_url);
    write_ini_string(f, "register_url", config->register_url);
    write_ini_int(f, "register_mode", config->register_mode);

    write_ini_string(f, "server_list_url", config->server_list_url);
    write_ini_string(f, "patch_url", config->patch_url);
    write_ini_string(f, "patch_backup_url", config->patch_backup_url);
    write_ini_int(f, "timeout", config->timeout);

    write_ini_int(f, "patch_source", config->patch_source);
    write_ini_string(f, "local_patch_path", config->local_patch_path);
    write_ini_string(f, "patch_key", config->patch_key);
    write_ini_int(f, "use_crc", config->use_crc);
    write_ini_int(f, "auto_create_dir", config->auto_create_dir);

    write_ini_string(f, "client_path", config->client_path);
    write_ini_string(f, "client_args", config->client_args);
    write_ini_string(f, "inject_dll_path", config->inject_dll_path);

    write_ini_int(f, "dll_count", config->dll_count);
    for (int i = 0; i < config->dll_count && i < MAX_DLL_COUNT; i++) {
        char key[64];
        sprintf_s(key, sizeof(key), "dll_path_%d", i);
        write_ini_string(f, key, config->dll_paths[i]);
    }

    fclose(f);
    return 1;
}

int LoadConfigFromFile(LauncherConfig* config, const char* file_path) {
    LoadDefaultConfig(config);

    FILE* f = NULL;
    if (fopen_s(&f, file_path, "r") != 0 || !f) {
        return 0;
    }

    char line[4096];
    char key[256];
    char value[2048];

    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
            line[--len] = '\0';
        }
        if (len == 0 || line[0] == '[' || line[0] == ';' || line[0] == '#') {
            continue;
        }

        read_ini_line(line, key, sizeof(key), value, sizeof(value));
        if (key[0] == '\0') continue;

        if (strcmp(key, "website_url") == 0) {
            strncpy_s(config->website_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "recharge_url") == 0) {
            strncpy_s(config->recharge_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "support_url") == 0) {
            strncpy_s(config->support_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "register_url") == 0) {
            strncpy_s(config->register_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "register_mode") == 0) {
            config->register_mode = atoi(value);
        } else if (strcmp(key, "server_list_url") == 0) {
            strncpy_s(config->server_list_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "patch_url") == 0) {
            strncpy_s(config->patch_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "patch_backup_url") == 0) {
            strncpy_s(config->patch_backup_url, MAX_URL_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "timeout") == 0) {
            config->timeout = atoi(value);
        } else if (strcmp(key, "patch_source") == 0) {
            config->patch_source = atoi(value);
        } else if (strcmp(key, "local_patch_path") == 0) {
            strncpy_s(config->local_patch_path, MAX_PATH_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "patch_key") == 0) {
            strncpy_s(config->patch_key, MAX_KEY_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "use_crc") == 0) {
            config->use_crc = atoi(value);
        } else if (strcmp(key, "auto_create_dir") == 0) {
            config->auto_create_dir = atoi(value);
        } else if (strcmp(key, "client_path") == 0) {
            strncpy_s(config->client_path, MAX_PATH_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "client_args") == 0) {
            strncpy_s(config->client_args, MAX_PATH_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "inject_dll_path") == 0) {
            strncpy_s(config->inject_dll_path, MAX_PATH_LEN, value, _TRUNCATE);
        } else if (strcmp(key, "dll_count") == 0) {
            config->dll_count = atoi(value);
            if (config->dll_count > MAX_DLL_COUNT) config->dll_count = MAX_DLL_COUNT;
        } else if (strncmp(key, "dll_path_", 9) == 0) {
            int idx = atoi(key + 9);
            if (idx >= 0 && idx < MAX_DLL_COUNT) {
                strncpy_s(config->dll_paths[idx], MAX_PATH_LEN, value, _TRUNCATE);
            }
        }
    }

    fclose(f);
    return 1;
}
