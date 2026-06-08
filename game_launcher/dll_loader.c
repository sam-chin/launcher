#include "dll_loader.h"
#include <stdio.h>
#include <stdlib.h>

int LoadExtensionDlls(const LauncherConfig* config, LoadedDll** out_dlls, int max_dlls) {
    if (!config || !out_dlls || config->dll_count == 0) {
        *out_dlls = NULL;
        return 0;
    }
    
    int load_count = (config->dll_count < max_dlls) ? config->dll_count : max_dlls;
    LoadedDll* dlls = (LoadedDll*)calloc(load_count, sizeof(LoadedDll));
    if (!dlls) {
        return 0;
    }
    
    int actual_count = 0;
    
    for (int i = 0; i < load_count; i++) {
        const char* dll_path = config->dll_paths[i];
        
        HMODULE hMod = LoadLibraryA(dll_path);
        if (!hMod) {
            char msg[512];
            sprintf_s(msg, 512, "无法加载DLL: %s\n错误代码: %lu", dll_path, GetLastError());
            MessageBoxA(NULL, msg, "警告", MB_OK | MB_ICONWARNING);
            continue;
        }
        
        DllInitFunc init_func = (DllInitFunc)GetProcAddress(hMod, "DllInit");
        DllUninitFunc uninit_func = (DllUninitFunc)GetProcAddress(hMod, "DllUninit");
        
        if (init_func) {
            if (!init_func()) {
                char msg[512];
                sprintf_s(msg, 512, "DLL初始化失败: %s", dll_path);
                MessageBoxA(NULL, msg, "警告", MB_OK | MB_ICONWARNING);
                FreeLibrary(hMod);
                continue;
            }
        }
        
        strcpy_s(dlls[actual_count].path, MAX_PATH_LEN, dll_path);
        dlls[actual_count].hModule = hMod;
        dlls[actual_count].init_func = init_func;
        dlls[actual_count].uninit_func = uninit_func;
        actual_count++;
    }
    
    *out_dlls = dlls;
    return actual_count;
}

void UnloadExtensionDlls(LoadedDll* dlls, int count) {
    if (!dlls) {
        return;
    }
    
    for (int i = 0; i < count; i++) {
        if (dlls[i].uninit_func) {
            dlls[i].uninit_func();
        }
        if (dlls[i].hModule) {
            FreeLibrary(dlls[i].hModule);
        }
    }
    
    free(dlls);
}
