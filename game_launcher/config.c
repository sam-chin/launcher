#include "config.h"
#include <string.h>

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
    
    config->dll_count = 0;
}
