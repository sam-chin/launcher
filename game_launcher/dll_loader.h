#ifndef DLL_LOADER_H
#define DLL_LOADER_H

#include <windows.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*DllInitFunc)();
typedef void (*DllUninitFunc)();

typedef struct {
    HMODULE hModule;
    DllInitFunc init_func;
    DllUninitFunc uninit_func;
    char path[MAX_PATH_LEN];
} LoadedDll;

int LoadExtensionDlls(const LauncherConfig* config, LoadedDll** out_dlls, int max_dlls);
void UnloadExtensionDlls(LoadedDll* dlls, int count);

#ifdef __cplusplus
}
#endif

#endif
