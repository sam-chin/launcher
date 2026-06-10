#include <afxwin.h>
#include <afxcmn.h>
#include <afxconv.h>
#include <wininet.h>
#include "resource.h"
#include "config.h"
#include "dll_loader.h"
#include "../common/file_pack/file_pack.h"
#include "../common/encrypt/encrypt.h"

// ============================================================
// 区服数据结构
// ============================================================
struct ServerInfo {
    CStringA name;
    CStringA ip;
    int      port;
};

class CServerList {
public:
    CArray<ServerInfo> m_servers;

    void Clear() { m_servers.RemoveAll(); }
    int  Count() const { return (int)m_servers.GetCount(); }
    void Add(const ServerInfo& si) { m_servers.Add(si); }
    const ServerInfo& Get(int idx) const { return m_servers[idx]; }
};

// ============================================================
// 主对话框
// ============================================================
class CLauncherApp : public CWinApp {
public:
    virtual BOOL InitInstance();
};

class CMainDlg : public CDialog {
public:
    CMainDlg(CWnd* pParent = NULL);
    enum { IDD = IDD_MAIN_DIALOG };

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnWebsiteBtn();
    afx_msg void OnRechargeBtn();
    afx_msg void OnSupportBtn();
    afx_msg void OnRegisterBtn();
    afx_msg void OnStartGame();
    DECLARE_MESSAGE_MAP();

private:
    // 网络/文件 辅助
    CStringA GetModuleDirA();
    BOOL   DownloadServerList();
    BOOL   ParseServerList(const char* text);
    BOOL   ApplyLocalPatch();

    // UI 更新
    void   LoadServerListToUI();
    void   SetStatusText(LPCTSTR text);

    // 启动游戏与注入
    BOOL   LaunchGameWithInject();
    BOOL   InjectDllToProcess(HANDLE hProcess, const CStringA& dllPath);

    // 公告文本（直接在 OnInitDialog 写死中文）
    void   InitAnnouncement();

    LauncherConfig m_config;
    CStringA       m_moduleDir;
    LoadedDll*     m_loadedDlls;
    int            m_dllCount;
    CListBox       m_serverList;
    CProgressCtrl  m_progressBar;
    CStatic        m_statusText;
    CServerList    m_servers;
};

CLauncherApp theApp;

BOOL CLauncherApp::InitInstance() {
    CWinApp::InitInstance();
    CMainDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    return FALSE;
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
    ON_BN_CLICKED(IDC_WEBSITE_BTN, &CMainDlg::OnWebsiteBtn)
    ON_BN_CLICKED(IDC_RECHARGE_BTN, &CMainDlg::OnRechargeBtn)
    ON_BN_CLICKED(IDC_SUPPORT_BTN, &CMainDlg::OnSupportBtn)
    ON_BN_CLICKED(IDC_REGISTER_BTN, &CMainDlg::OnRegisterBtn)
    ON_BN_CLICKED(IDC_START_GAME, &CMainDlg::OnStartGame)
END_MESSAGE_MAP()

CMainDlg::CMainDlg(CWnd* pParent)
    : CDialog(CMainDlg::IDD, pParent)
    , m_loadedDlls(NULL)
    , m_dllCount(0) {
}

// ============================================================
// 工具函数
// ============================================================
CStringA CMainDlg::GetModuleDirA() {
    char buf[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, buf, MAX_PATH);
    char* p = strrchr(buf, '\\');
    if (p) *p = '\0';
    return CStringA(buf);
}

void CMainDlg::SetStatusText(LPCTSTR text) {
    m_statusText.SetWindowText(text);
}

void CMainDlg::InitAnnouncement() {
    GetDlgItem(IDC_ANNOUNCEMENT)->SetWindowText(
        _T("欢迎使用游戏登录器！请选择服务器，然后点击「进入游戏」。"));
}

// ============================================================
// 区服列表下载
// ============================================================
BOOL CMainDlg::DownloadServerList() {
    if (m_config.server_list_url[0] == '\0') {
        return FALSE;
    }

    CStringA urlA(m_config.server_list_url);

    HINTERNET hInternet = InternetOpenA("GameLauncher/1.0",
                                         INTERNET_OPEN_TYPE_PRECONFIG,
                                         NULL, NULL, 0);
    if (!hInternet) {
        return FALSE;
    }

    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
                  INTERNET_FLAG_NO_UI;
    HINTERNET hFile = InternetOpenUrlA(hInternet, urlA, NULL, 0, flags, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return FALSE;
    }

    CStringA content;
    char   buffer[8192];
    DWORD  bytesRead = 0;

    while (InternetReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead)
           && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        content += buffer;
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);

    if (content.GetLength() == 0) {
        return FALSE;
    }

    return ParseServerList(content);
}

// ============================================================
// 解析 serverlist.txt
// 支持的格式（每行一条）:
//     区服名,IP,端口
//     区服名|IP|端口
//     区服名 IP 端口
// ============================================================
static void TrimA(char* s) {
    if (!s) return;
    char* p = s;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t'
                       || s[len-1] == '\r' || s[len-1] == '\n')) {
        s[--len] = '\0';
    }
}

BOOL CMainDlg::ParseServerList(const char* text) {
    if (!text || !*text) return FALSE;
    m_servers.Clear();

    CStringA content(text);
    char* buf = content.GetBuffer();

    char* line = buf;
    char* p = buf;

    while (*p) {
        if (*p == '\n' || *p == '\r') {
            char saved = *p;
            *p = '\0';
            TrimA(line);
            if (*line && *line != '#' && *line != ';') {
                // 解析一行
                char sep = ',';
                char* sp = strchr(line, ',');
                if (!sp) { sp = strchr(line, '|'); if (sp) sep = '|'; }
                if (!sp) { sp = strchr(line, '\t'); if (sp) sep = '\t'; }
                if (!sp) { sp = strchr(line, ' '); if (sp) sep = ' '; }

                if (sp) {
                    char name[256] = { 0 };
                    char ip[64] = { 0 };
                    int port = 0;

                    int nameLen = (int)(sp - line);
                    if (nameLen >= 256) nameLen = 255;
                    strncpy_s(name, sizeof(name), line, nameLen);
                    TrimA(name);

                    char* rest = sp + 1;
                    char* sp2 = NULL;
                    if (sep == ',')      sp2 = strchr(rest, ',');
                    else if (sep == '|') sp2 = strchr(rest, '|');
                    else if (sep == '\t')sp2 = strchr(rest, '\t');
                    else                 sp2 = strchr(rest, ' ');

                    if (sp2) {
                        int ipLen = (int)(sp2 - rest);
                        if (ipLen >= 64) ipLen = 63;
                        strncpy_s(ip, sizeof(ip), rest, ipLen);
                        TrimA(ip);
                        port = atoi(sp2 + 1);
                    } else {
                        strncpy_s(ip, sizeof(ip), rest, _TRUNCATE);
                        TrimA(ip);
                    }

                    if (name[0] && ip[0] && port > 0) {
                        ServerInfo si;
                        si.name = name;
                        si.ip   = ip;
                        si.port = port;
                        m_servers.Add(si);
                    }
                }
            }
            p++;
            if (saved == '\r' && *p == '\n') p++;
            line = p;
        } else {
            p++;
        }
    }

    // 最后一行
    if (line && *line) {
        TrimA(line);
        char* sp = strchr(line, ',');
        if (sp) {
            char name[256] = { 0 }, ip[64] = { 0 };
            int nameLen = (int)(sp - line);
            if (nameLen >= 256) nameLen = 255;
            strncpy_s(name, sizeof(name), line, nameLen);
            TrimA(name);
            char* rest = sp + 1;
            char* sp2 = strchr(rest, ',');
            int port = 0;
            if (sp2) {
                int ipLen = (int)(sp2 - rest);
                if (ipLen >= 64) ipLen = 63;
                strncpy_s(ip, sizeof(ip), rest, ipLen);
                TrimA(ip);
                port = atoi(sp2 + 1);
            } else {
                strncpy_s(ip, sizeof(ip), rest, _TRUNCATE);
            }
            if (name[0] && ip[0] && port > 0) {
                ServerInfo si;
                si.name = name;
                si.ip   = ip;
                si.port = port;
                m_servers.Add(si);
            }
        }
    }

    content.ReleaseBuffer();
    return m_servers.Count() > 0;
}

// ============================================================
// 本地补丁解密 + 解包（不改逻辑）
// ============================================================
BOOL CMainDlg::ApplyLocalPatch() {
    FILE* f = NULL;
    if (fopen_s(&f, m_config.local_patch_path, "rb") != 0 || !f) {
        return FALSE;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return FALSE; }

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) { fclose(f); return FALSE; }
    fread(data, 1, size, f);
    fclose(f);

    xor_decrypt(data, size, (const uint8_t*)m_config.patch_key,
                strlen(m_config.patch_key));

    FilePack* pack = file_pack_unpack(data, size);
    free(data);
    if (!pack) return FALSE;

    CStringA dir = GetModuleDirA();
    file_pack_extract(pack, dir);
    file_pack_destroy(pack);
    return TRUE;
}

// ============================================================
// UI: 把区服加载到列表
// ============================================================
void CMainDlg::LoadServerListToUI() {
    m_serverList.ResetContent();
    if (m_servers.Count() == 0) {
        m_serverList.AddString(_T("（无可用区服）"));
        return;
    }
    for (int i = 0; i < m_servers.Count(); i++) {
        const ServerInfo& si = m_servers.Get(i);
        CString display;
        display.Format(_T("%s - %s:%d"),
                       CA2T(si.name), CA2T(si.ip), si.port);
        m_serverList.AddString(display);
    }
    m_serverList.SetCurSel(0);
}

// ============================================================
// 远程线程注入 DLL
// ============================================================
BOOL CMainDlg::InjectDllToProcess(HANDLE hProcess, const CStringA& dllPath) {
    if (!hProcess || dllPath.IsEmpty()) return FALSE;

    SIZE_T pathLen = dllPath.GetLength() + 1;

    // 1. 在目标进程内分配空间
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, pathLen,
                                      MEM_COMMIT | MEM_RESERVE,
                                      PAGE_READWRITE);
    if (!remoteMem) {
        return FALSE;
    }

    // 2. 写入 DLL 完整路径到目标进程
    SIZE_T written = 0;
    BOOL ok = WriteProcessMemory(hProcess, remoteMem,
                                 (LPCVOID)(LPCSTR)dllPath,
                                 pathLen, &written);
    if (!ok || written != pathLen) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }

    // 3. 获取 LoadLibraryA 地址（在 kernel32.dll 的地址与进程无关）
    FARPROC loadLibAddr = GetProcAddress(GetModuleHandle(_T("kernel32")),
                                          "LoadLibraryA");
    if (!loadLibAddr) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }

    // 4. 创建远程线程调用 LoadLibraryA(dllPath)
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
                         (LPTHREAD_START_ROUTINE)loadLibAddr,
                         remoteMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return FALSE;
    }

    // 5. 等待线程执行完毕，释放资源
    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);

    return (exitCode != 0);
}

// ============================================================
// 启动游戏：CREATE_SUSPENDED -> 注入 -> 恢复
// ============================================================
BOOL CMainDlg::LaunchGameWithInject() {
    int sel = m_serverList.GetCurSel();
    if (sel < 0 || sel >= m_servers.Count()) {
        MessageBox(_T("请先选择区服！"), _T("提示"),
                   MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    const ServerInfo& si = m_servers.Get(sel);

    // --- 拼接参数: ur;name=区服名;ip=IP;port=端口;ra=163.com ---
    CStringA args;
    args.Format("ur;name=%s;ip=%s;port=%d;ra=163.com",
                si.name, si.ip, si.port);

    // --- 客户端路径与工作目录 ---
    CStringA workDir = GetModuleDirA();
    CStringA clientPath;
    clientPath.Format("%s\\%s", workDir, m_config.client_path);

    // --- 注入 DLL 完整路径（相对登录器目录） ---
    CStringA dllFullPath;
    if (strchr(m_config.inject_dll_path, ':') != NULL
        || m_config.inject_dll_path[0] == '\\') {
        dllFullPath = m_config.inject_dll_path;   // 已绝对路径
    } else {
        dllFullPath.Format("%s\\%s", workDir, m_config.inject_dll_path);
    }

    // --- STARTUPINFO / PROCESS_INFORMATION 初始化 ---
    STARTUPINFOA siStart;
    PROCESS_INFORMATION pi;
    ZeroMemory(&siStart, sizeof(siStart));
    siStart.cb = sizeof(siStart);
    ZeroMemory(&pi, sizeof(pi));

    // 非可修改的字符缓冲区用于 CreateProcessA 的第二个参数
    char* argsBuf = args.GetBuffer();

    BOOL created = CreateProcessA(
        clientPath,        // lpApplicationName
        argsBuf,           // lpCommandLine
        NULL,              // lpProcessAttributes
        NULL,              // lpThreadAttributes
        FALSE,             // bInheritHandles
        CREATE_SUSPENDED,  // dwCreationFlags: 挂起进程以进行注入
        NULL,              // lpEnvironment
        workDir,           // lpCurrentDirectory
        &siStart,          // lpStartupInfo
        &pi                // lpProcessInformation
    );

    args.ReleaseBuffer();

    if (!created) {
        DWORD err = GetLastError();
        CString msg;
        msg.Format(_T("无法启动游戏客户端！\n路径: %s\n错误代码: %lu"),
                   CA2T(clientPath), err);
        MessageBox(msg, _T("错误"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // --- 注入 DLL ---
    BOOL injectOK = TRUE;
    CString injectErrMsg;

    if (strlen(m_config.inject_dll_path) > 0) {
        if (GetFileAttributesA(dllFullPath) == INVALID_FILE_ATTRIBUTES) {
            // DLL 不存在，但继续启动进程，只给提示
            injectOK = FALSE;
            injectErrMsg.Format(
                _T("找不到注入 DLL：\n%s\n\n将正常启动游戏，但不执行注入。"),
                CA2T(dllFullPath));
        } else if (!InjectDllToProcess(pi.hProcess, dllFullPath)) {
            injectOK = FALSE;
            DWORD err = GetLastError();
            injectErrMsg.Format(
                _T("DLL 注入失败！错误代码: %lu\n将恢复进程继续运行。"), err);
        }
    }

    // --- 恢复主线程 ---
    DWORD resume = ResumeThread(pi.hThread);
    if (resume == (DWORD)-1) {
        DWORD err = GetLastError();
        // 记录但不阻塞主流程
    }

    // --- 释放句柄 ---
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // --- 注入失败弹窗（在恢复后再弹，避免进程卡死） ---
    if (!injectOK && !injectErrMsg.IsEmpty()) {
        MessageBox(injectErrMsg, _T("警告"), MB_OK | MB_ICONWARNING);
    }

    return TRUE;
}

// ============================================================
// OnInitDialog
// ============================================================
BOOL CMainDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    m_moduleDir = GetModuleDirA();

    // UI 文本
    SetWindowText(_T("游戏登录器"));
    GetDlgItem(IDC_GROUPBOX1)->SetWindowText(_T("快捷功能"));
    GetDlgItem(IDC_WEBSITE_BTN)->SetWindowText(_T("官网"));
    GetDlgItem(IDC_RECHARGE_BTN)->SetWindowText(_T("充值"));
    GetDlgItem(IDC_SUPPORT_BTN)->SetWindowText(_T("客服"));
    GetDlgItem(IDC_REGISTER_BTN)->SetWindowText(_T("注册"));
    GetDlgItem(IDC_GROUPBOX2)->SetWindowText(_T("公告"));
    InitAnnouncement();
    GetDlgItem(IDC_GROUPBOX3)->SetWindowText(_T("区服列表"));
    GetDlgItem(IDC_START_GAME)->SetWindowText(_T("进入游戏"));

    // 配置
    char cfgPath[MAX_PATH * 2];
    sprintf_s(cfgPath, sizeof(cfgPath), "%s\\launcher_config.ini",
              (LPCSTR)m_moduleDir);
    if (!LoadConfigFromFile(&m_config, cfgPath)) {
        LoadDefaultConfig(&m_config);
    }

    // 扩展 DLL（旧式 LoadLibrary 扩展，与注入无关）
    m_dllCount = LoadExtensionDlls(&m_config, &m_loadedDlls, MAX_DLL_COUNT);

    m_serverList.SubclassDlgItem(IDC_SERVER_LIST, this);
    m_progressBar.SubclassDlgItem(IDC_PROGRESS_BAR, this);
    m_statusText.SubclassDlgItem(IDC_STATUS_TEXT, this);

    m_progressBar.SetRange(0, 100);
    m_progressBar.SetPos(0);

    // --- 区服列表 ---
    SetStatusText(_T("正在获取区服列表..."));
    if (!DownloadServerList()) {
        // 下载失败时，回退到默认区服，避免列表为空
        ServerInfo si1;
        si1.name = "默认区服";
        si1.ip   = "127.0.0.1";
        si1.port = 7777;
        m_servers.Add(si1);
    }
    LoadServerListToUI();

    // --- 本地补丁 ---
    SetStatusText(_T("正在检查补丁..."));
    if (strlen(m_config.local_patch_path) > 0) {
        char patchPath[MAX_PATH * 2];
        sprintf_s(patchPath, sizeof(patchPath), "%s\\%s",
                  (LPCSTR)m_moduleDir, m_config.local_patch_path);
        FILE* ftest = NULL;
        if (fopen_s(&ftest, patchPath, "rb") == 0 && ftest) {
            fclose(ftest);
            m_progressBar.SetPos(30);
            SetStatusText(_T("正在解密与解包补丁..."));
            // 简单尝试：不阻塞主流程，失败则静默跳过
            char old[MAX_PATH];
            strcpy_s(old, sizeof(old), m_config.local_patch_path);
            strcpy_s(m_config.local_patch_path, MAX_PATH_LEN, patchPath);
            ApplyLocalPatch();
            strcpy_s(m_config.local_patch_path, MAX_PATH_LEN, old);
            m_progressBar.SetPos(80);
        }
    }
    m_progressBar.SetPos(100);

    SetStatusText(_T("就绪，请选择区服后进入游戏"));
    return TRUE;
}

// ============================================================
// 按钮响应
// ============================================================
void CMainDlg::OnWebsiteBtn() {
    if (m_config.website_url[0])
        ShellExecuteA(NULL, "open", m_config.website_url,
                      NULL, NULL, SW_SHOWNORMAL);
}
void CMainDlg::OnRechargeBtn() {
    if (m_config.recharge_url[0])
        ShellExecuteA(NULL, "open", m_config.recharge_url,
                      NULL, NULL, SW_SHOWNORMAL);
}
void CMainDlg::OnSupportBtn() {
    if (m_config.support_url[0])
        ShellExecuteA(NULL, "open", m_config.support_url,
                      NULL, NULL, SW_SHOWNORMAL);
}
void CMainDlg::OnRegisterBtn() {
    if (m_config.register_url[0])
        ShellExecuteA(NULL, "open", m_config.register_url,
                      NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnStartGame() {
    SetStatusText(_T("正在启动游戏..."));
    if (LaunchGameWithInject()) {
        SetStatusText(_T("游戏已启动"));
    } else {
        SetStatusText(_T("启动失败"));
    }
}

void CMainDlg::OnOK() {
    if (m_loadedDlls) {
        UnloadExtensionDlls(m_loadedDlls, m_dllCount);
    }
    CDialog::OnOK();
}
