#include <afxwin.h>
#include <afxconv.h>
#include <shlobj.h>
#include "resource.h"
#include "../game_launcher/config.h"

class CGeneratorApp : public CWinApp {
public:
    virtual BOOL InitInstance();
};

class CGeneratorDlg : public CDialog {
public:
    CGeneratorDlg(CWnd* pParent = NULL);
    enum { IDD = IDD_GENERATOR_DIALOG };

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnSaveBtn();
    afx_msg void OnLoadBtn();
    afx_msg void OnGenerateBtn();
    afx_msg void OnOpenDirBtn();
    DECLARE_MESSAGE_MAP();

private:
    void LoadConfigToUI();
    void SaveConfigFromUI();
    CStringA GetAppDirA();
    BOOL CopyAssetFile(const CStringA& srcPath, const CStringA& destDir, const CStringA& fileName);

    LauncherConfig m_config;
    CEdit m_websiteEdit;
    CEdit m_rechargeEdit;
    CEdit m_supportEdit;
    CEdit m_registerEdit;
    CEdit m_serverListEdit;
    CEdit m_patchEdit;
    CEdit m_patchKeyEdit;
    CEdit m_clientEdit;
    CEdit m_injectDllEdit;
};

CGeneratorApp theApp;

BOOL CGeneratorApp::InitInstance() {
    CWinApp::InitInstance();
    CGeneratorDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    return FALSE;
}

BEGIN_MESSAGE_MAP(CGeneratorDlg, CDialog)
    ON_BN_CLICKED(IDC_SAVE_BTN, &CGeneratorDlg::OnSaveBtn)
    ON_BN_CLICKED(IDC_LOAD_BTN, &CGeneratorDlg::OnLoadBtn)
    ON_BN_CLICKED(IDC_GENERATE_BTN, &CGeneratorDlg::OnGenerateBtn)
    ON_BN_CLICKED(IDC_OPEN_DIR_BTN, &CGeneratorDlg::OnOpenDirBtn)
END_MESSAGE_MAP()

CGeneratorDlg::CGeneratorDlg(CWnd* pParent) : CDialog(CGeneratorDlg::IDD, pParent) {
    LoadDefaultConfig(&m_config);
}

CStringA CGeneratorDlg::GetAppDirA() {
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
    return CStringA(exe_path);
}

BOOL CGeneratorDlg::CopyAssetFile(const CStringA& srcPath, const CStringA& destDir, const CStringA& fileName) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(srcPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    FindClose(hFind);

    char destPath[MAX_PATH * 2];
    sprintf_s(destPath, sizeof(destPath), "%s\\%s", destDir, fileName);

    return ::CopyFileA(srcPath, destPath, FALSE);
}

BOOL CGeneratorDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    SetWindowText(_T("登录器生成器"));

    GetDlgItem(IDC_GROUPBOX1)->SetWindowText(_T("网页配置"));
    GetDlgItem(IDC_LABEL_WEBSITE)->SetWindowText(_T("官网:"));
    GetDlgItem(IDC_LABEL_RECHARGE)->SetWindowText(_T("充值:"));
    GetDlgItem(IDC_LABEL_SUPPORT)->SetWindowText(_T("客服:"));
    GetDlgItem(IDC_LABEL_REGISTER)->SetWindowText(_T("注册:"));

    GetDlgItem(IDC_GROUPBOX2)->SetWindowText(_T("补丁和服务器配置"));
    GetDlgItem(IDC_LABEL_SERVERLIST)->SetWindowText(_T("服务器列表:"));
    GetDlgItem(IDC_LABEL_PATCH)->SetWindowText(_T("补丁文件:"));
    GetDlgItem(IDC_LABEL_PATCHKEY)->SetWindowText(_T("补丁密钥:"));

    GetDlgItem(IDC_GROUPBOX3)->SetWindowText(_T("客户端配置"));
    GetDlgItem(IDC_LABEL_CLIENT)->SetWindowText(_T("客户端:"));
    GetDlgItem(IDC_LABEL_INJECT_DLL)->SetWindowText(_T("注入DLL:"));

    GetDlgItem(IDC_SAVE_BTN)->SetWindowText(_T("保存配置"));
    GetDlgItem(IDC_LOAD_BTN)->SetWindowText(_T("加载配置"));
    GetDlgItem(IDC_GENERATE_BTN)->SetWindowText(_T("生成登录器"));
    GetDlgItem(IDC_OPEN_DIR_BTN)->SetWindowText(_T("打开目录"));

    m_websiteEdit.SubclassDlgItem(IDC_WEBSITE_EDIT, this);
    m_rechargeEdit.SubclassDlgItem(IDC_RECHARGE_EDIT, this);
    m_supportEdit.SubclassDlgItem(IDC_SUPPORT_EDIT, this);
    m_registerEdit.SubclassDlgItem(IDC_REGISTER_EDIT, this);
    m_serverListEdit.SubclassDlgItem(IDC_SERVER_LIST_EDIT, this);
    m_patchEdit.SubclassDlgItem(IDC_PATCH_EDIT, this);
    m_patchKeyEdit.SubclassDlgItem(IDC_PATCH_KEY_EDIT, this);
    m_clientEdit.SubclassDlgItem(IDC_CLIENT_EDIT, this);
    m_injectDllEdit.SubclassDlgItem(IDC_INJECT_DLL_EDIT, this);

    CStringA appDir = GetAppDirA();
    char configPath[MAX_PATH * 2];
    sprintf_s(configPath, sizeof(configPath), "%s\\launcher_config.ini", (LPCSTR)appDir);
    if (!LoadConfigFromFile(&m_config, configPath)) {
        LoadDefaultConfig(&m_config);
    }
    LoadConfigToUI();

    return TRUE;
}

void CGeneratorDlg::LoadConfigToUI() {
    m_websiteEdit.SetWindowText(CString(m_config.website_url));
    m_rechargeEdit.SetWindowText(CString(m_config.recharge_url));
    m_supportEdit.SetWindowText(CString(m_config.support_url));
    m_registerEdit.SetWindowText(CString(m_config.register_url));
    m_serverListEdit.SetWindowText(CString(m_config.server_list_url));
    m_patchEdit.SetWindowText(CString(m_config.local_patch_path));
    m_patchKeyEdit.SetWindowText(CString(m_config.patch_key));
    m_clientEdit.SetWindowText(CString(m_config.client_path));
    m_injectDllEdit.SetWindowText(CString(m_config.inject_dll_path));
}

void CGeneratorDlg::SaveConfigFromUI() {
    CString websiteStr, rechargeStr, supportStr, registerStr;
    CString serverListStr, patchStr, patchKeyStr, clientStr, injectDllStr;

    m_websiteEdit.GetWindowText(websiteStr);
    m_rechargeEdit.GetWindowText(rechargeStr);
    m_supportEdit.GetWindowText(supportStr);
    m_registerEdit.GetWindowText(registerStr);
    m_serverListEdit.GetWindowText(serverListStr);
    m_patchEdit.GetWindowText(patchStr);
    m_patchKeyEdit.GetWindowText(patchKeyStr);
    m_clientEdit.GetWindowText(clientStr);
    m_injectDllEdit.GetWindowText(injectDllStr);

    strncpy_s(m_config.website_url, MAX_URL_LEN, CT2CA(websiteStr), _TRUNCATE);
    strncpy_s(m_config.recharge_url, MAX_URL_LEN, CT2CA(rechargeStr), _TRUNCATE);
    strncpy_s(m_config.support_url, MAX_URL_LEN, CT2CA(supportStr), _TRUNCATE);
    strncpy_s(m_config.register_url, MAX_URL_LEN, CT2CA(registerStr), _TRUNCATE);
    strncpy_s(m_config.server_list_url, MAX_URL_LEN, CT2CA(serverListStr), _TRUNCATE);
    strncpy_s(m_config.local_patch_path, MAX_PATH_LEN, CT2CA(patchStr), _TRUNCATE);
    strncpy_s(m_config.patch_key, MAX_KEY_LEN, CT2CA(patchKeyStr), _TRUNCATE);
    strncpy_s(m_config.client_path, MAX_PATH_LEN, CT2CA(clientStr), _TRUNCATE);
    strncpy_s(m_config.inject_dll_path, MAX_PATH_LEN, CT2CA(injectDllStr), _TRUNCATE);
}

void CGeneratorDlg::OnSaveBtn() {
    SaveConfigFromUI();

    CStringA appDir = GetAppDirA();
    char configPath[MAX_PATH * 2];
    sprintf_s(configPath, sizeof(configPath), "%s\\launcher_config.ini", (LPCSTR)appDir);

    if (SaveConfigToFile(&m_config, configPath)) {
        MessageBox(_T("配置已保存到 launcher_config.ini！"), _T("成功"),
                   MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(_T("保存配置失败！请检查目录权限。"), _T("错误"),
                   MB_OK | MB_ICONERROR);
    }
}

void CGeneratorDlg::OnLoadBtn() {
    CStringA appDir = GetAppDirA();
    char configPath[MAX_PATH * 2];
    sprintf_s(configPath, sizeof(configPath), "%s\\launcher_config.ini", (LPCSTR)appDir);

    if (LoadConfigFromFile(&m_config, configPath)) {
        LoadConfigToUI();
        MessageBox(_T("已从 launcher_config.ini 加载配置！"), _T("成功"),
                   MB_OK | MB_ICONINFORMATION);
    } else {
        LoadDefaultConfig(&m_config);
        LoadConfigToUI();
        MessageBox(_T("未找到 launcher_config.ini，已加载默认配置。"),
                   _T("提示"), MB_OK | MB_ICONINFORMATION);
    }
}

void CGeneratorDlg::OnGenerateBtn() {
    SaveConfigFromUI();

    CStringA appDir = GetAppDirA();

    BROWSEINFOA bi;
    ZeroMemory(&bi, sizeof(bi));
    char displayName[MAX_PATH];
    bi.hwndOwner = m_hWnd;
    bi.pszDisplayName = displayName;
    bi.lpszTitle = "请选择登录器输出目录";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (!pidl) {
        return;
    }

    char selectedPath[MAX_PATH];
    if (!SHGetPathFromIDListA(pidl, selectedPath)) {
        CoTaskMemFree(pidl);
        MessageBox(_T("获取目录失败！"), _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }
    CoTaskMemFree(pidl);

    CStringA outputDir(selectedPath);
    if (outputDir.IsEmpty()) {
        MessageBox(_T("未选择输出目录！"), _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    char configOutputPath[MAX_PATH * 2];
    sprintf_s(configOutputPath, sizeof(configOutputPath),
              "%s\\launcher_config.ini", (LPCSTR)outputDir);

    if (!SaveConfigToFile(&m_config, configOutputPath)) {
        MessageBox(_T("写入配置文件到输出目录失败！请检查目录权限。"),
                   _T("错误"), MB_OK | MB_ICONERROR);
        return;
    }

    CStringA srcLauncher;
    srcLauncher.Format("%s\\game_launcher.exe", (LPCSTR)appDir);
    BOOL copiedLauncher = CopyAssetFile(srcLauncher, outputDir, "game_launcher.exe");

    CStringA srcPlugin;
    srcPlugin.Format("%s\\plugin.dll", (LPCSTR)appDir);
    BOOL copiedPlugin = CopyAssetFile(srcPlugin, outputDir, "plugin.dll");

    CString msg;
    msg.Format(_T("登录器已生成到：\n%s\n\n包含文件:"), CString(outputDir));
    
    if (copiedLauncher) {
        msg += _T("\n- game_launcher.exe (主程序)");
    } else {
        msg += _T("\n- [警告] game_launcher.exe 未找到！请确保工具与 game_launcher.exe 在同一目录");
    }
    
    msg += _T("\n- launcher_config.ini (配置文件)");
    
    if (copiedPlugin) {
        msg += _T("\n- plugin.dll (扩展插件)");
    }

    msg += _T("\n\n使用说明:\n1. 将注入DLL（如 inject.dll）复制到登录器目录\n2. 双击 game_launcher.exe 启动登录器\n3. 登录器会自动读取 launcher_config.ini 配置");

    MessageBox(msg, _T("完成"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnOpenDirBtn() {
    CStringA appDir = GetAppDirA();
    ShellExecuteA(m_hWnd, "open", (LPCSTR)appDir, NULL, NULL, SW_SHOWNORMAL);
}
