#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>

#define MAX_PATH_LEN 512
#define MAX_URL_LEN 1024
#define MAX_KEY_LEN 256
#define MAX_DLL_COUNT 16

typedef struct {
    char website_url[MAX_URL_LEN];
    char recharge_url[MAX_URL_LEN];
    char support_url[MAX_URL_LEN];
    char register_url[MAX_URL_LEN];
    int register_mode;
    
    char server_list_url[MAX_URL_LEN];
    char patch_url[MAX_URL_LEN];
    char patch_backup_url[MAX_URL_LEN];
    int timeout;
    
    int patch_source;
    char local_patch_path[MAX_PATH_LEN];
    char patch_key[MAX_KEY_LEN];
    int use_crc;
    int auto_create_dir;
    
    char client_path[MAX_PATH_LEN];
    char client_args[MAX_PATH_LEN];
    
    char dll_paths[MAX_DLL_COUNT][MAX_PATH_LEN];
    int dll_count;
} LauncherConfig;

void LoadDefaultConfig(LauncherConfig* config);

#endif
