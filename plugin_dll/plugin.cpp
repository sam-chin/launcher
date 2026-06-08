#include <windows.h>
#include <stdio.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL DllInit() {
    MessageBoxA(NULL, "扩展DLL加载成功！", "提示", MB_OK | MB_ICONINFORMATION);
    return TRUE;
}

extern "C" __declspec(dllexport) void DllUninit() {
}
