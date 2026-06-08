#include <afxwin.h>
#include <afxcmn.h>
#include <afxconv.h>
#include "resource.h"
#include "config.h"
#include "dll_loader.h"
#include "../common/file_pack/file_pack.h"
#include "../common/encrypt/encrypt.h"

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
    void LoadServerList();
    void UpdatePatch();
    void LaunchGame();
    void SetStatusText(const CString& text);

    LauncherConfig m_config;
    LoadedDll* m_loadedDlls;
    int m_dllCount;
    CListBox m_serverList;
    CProgressCtrl m_progressBar;
    CStatic m_statusText;
    CString m_selectedServerIP;
    int m_selectedServerPort;
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

CMainDlg::CMainDlg(CWnd* pParent) : CDialog(CMainDlg::IDD, pParent) {
    m_loadedDlls = NULL;
    m_dllCount = 0;
    m_selectedServerPort = 0;
}

BOOL CMainDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    LoadDefaultConfig(&m_config);
    m_dllCount = LoadExtensionDlls(&m_config, &m_loadedDlls, MAX_DLL_COUNT);

    m_serverList.SubclassDlgItem(IDC_SERVER_LIST, this);
    m_progressBar.SubclassDlgItem(IDC_PROGRESS_BAR, this);
    m_statusText.SubclassDlgItem(IDC_STATUS_TEXT, this);

    m_progressBar.SetRange(0, 100);
    m_progressBar.SetPos(0);

    SetStatusText(_T("Initializing..."));
    LoadServerList();

    SetStatusText(_T("Ready"));
    return TRUE;
}

void CMainDlg::LoadServerList() {
    m_serverList.ResetContent();
    
    m_serverList.AddString(_T("Server 1 - 192.168.1.100:7000 [Online]"));
    m_serverList.AddString(_T("Server 2 - 192.168.1.101:7000 [Online]"));
    m_serverList.AddString(_T("Server 3 - 192.168.1.102:7000 [Maintenance]"));
    
    m_serverList.SetCurSel(0);
    m_selectedServerIP = _T("192.168.1.100");
    m_selectedServerPort = 7000;
}

void CMainDlg::SetStatusText(const CString& text) {
    m_statusText.SetWindowText(text);
}

void CMainDlg::OnWebsiteBtn() {
    ShellExecuteA(NULL, "open", m_config.website_url, NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnRechargeBtn() {
    ShellExecuteA(NULL, "open", m_config.recharge_url, NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnSupportBtn() {
    ShellExecuteA(NULL, "open", m_config.support_url, NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnRegisterBtn() {
    ShellExecuteA(NULL, "open", m_config.register_url, NULL, NULL, SW_SHOWNORMAL);
}

void CMainDlg::OnStartGame() {
    SetStatusText(_T("Checking updates..."));
    UpdatePatch();
    
    SetStatusText(_T("Starting game..."));
    LaunchGame();
}

void CMainDlg::UpdatePatch() {
    m_progressBar.SetPos(0);
    
    FILE* f = NULL;
    if (fopen_s(&f, m_config.local_patch_path, "rb") == 0 && f) {
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        uint8_t* data = (uint8_t*)malloc(size);
        if (data) {
            fread(data, 1, size, f);
            fclose(f);
            
            SetStatusText(_T("Decrypting..."));
            m_progressBar.SetPos(30);
            
            xor_decrypt(data, size, (const uint8_t*)m_config.patch_key, strlen(m_config.patch_key));
            
            SetStatusText(_T("Unpacking..."));
            m_progressBar.SetPos(60);
            
            FilePack* pack = file_pack_unpack(data, size);
            free(data);
            
            if (pack) {
                char exe_path[MAX_PATH];
                GetModuleFileNameA(NULL, exe_path, MAX_PATH);
                char* last_slash = strrchr(exe_path, '\\');
                if (last_slash) {
                    *last_slash = '\0';
                }
                
                SetStatusText(_T("Applying patch..."));
                m_progressBar.SetPos(80);
                
                file_pack_extract(pack, exe_path);
                file_pack_destroy(pack);
            }
        }
    }
    
    m_progressBar.SetPos(100);
}

void CMainDlg::LaunchGame() {
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
    
    char client_full_path[MAX_PATH];
    sprintf_s(client_full_path, MAX_PATH, "%s\\%s", exe_path, m_config.client_path);
    
    char args[1024];
    CString ipStr = m_selectedServerIP;
    CW2A ipStrA(ipStr);
    sprintf_s(args, 1024, "ur;name=Player;ip=%s;port=%d;ra=163.com", 
              (LPCSTR)ipStrA, m_selectedServerPort);
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    if (!CreateProcessA(client_full_path, args, NULL, NULL, FALSE, 
                        CREATE_SUSPENDED, NULL, exe_path, &si, &pi)) {
        MessageBox(_T("Failed to start game!"), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }
    
    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    CDialog::OnOK();
}

void CMainDlg::OnOK() {
    if (m_loadedDlls) {
        UnloadExtensionDlls(m_loadedDlls, m_dllCount);
    }
    CDialog::OnOK();
}
