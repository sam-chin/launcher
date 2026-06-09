#include <afxwin.h>
#include <afxconv.h>
#include "resource.h"
#include "../game_launcher/config.h"
#include <shlobj.h>

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

    LauncherConfig m_config;
    CEdit m_websiteEdit;
    CEdit m_rechargeEdit;
    CEdit m_supportEdit;
    CEdit m_registerEdit;
    CEdit m_serverListEdit;
    CEdit m_patchEdit;
    CEdit m_patchKeyEdit;
    CEdit m_clientEdit;
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
}

void CGeneratorDlg::SaveConfigFromUI() {
    CString websiteStr;
    m_websiteEdit.GetWindowText(websiteStr);
    CW2A websiteStrA(websiteStr);
    strcpy_s(m_config.website_url, websiteStrA);

    CString rechargeStr;
    m_rechargeEdit.GetWindowText(rechargeStr);
    CW2A rechargeStrA(rechargeStr);
    strcpy_s(m_config.recharge_url, rechargeStrA);

    CString supportStr;
    m_supportEdit.GetWindowText(supportStr);
    CW2A supportStrA(supportStr);
    strcpy_s(m_config.support_url, supportStrA);

    CString registerStr;
    m_registerEdit.GetWindowText(registerStr);
    CW2A registerStrA(registerStr);
    strcpy_s(m_config.register_url, registerStrA);

    CString serverListStr;
    m_serverListEdit.GetWindowText(serverListStr);
    CW2A serverListStrA(serverListStr);
    strcpy_s(m_config.server_list_url, serverListStrA);

    CString patchStr;
    m_patchEdit.GetWindowText(patchStr);
    CW2A patchStrA(patchStr);
    strcpy_s(m_config.local_patch_path, patchStrA);

    CString patchKeyStr;
    m_patchKeyEdit.GetWindowText(patchKeyStr);
    CW2A patchKeyStrA(patchKeyStr);
    strcpy_s(m_config.patch_key, patchKeyStrA);

    CString clientStr;
    m_clientEdit.GetWindowText(clientStr);
    CW2A clientStrA(clientStr);
    strcpy_s(m_config.client_path, clientStrA);
}

void CGeneratorDlg::OnSaveBtn() {
    SaveConfigFromUI();
    MessageBox(_T("配置已保存！"), _T("信息"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnLoadBtn() {
    MessageBox(_T("加载配置功能需要实现！"), _T("信息"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnGenerateBtn() {
    SaveConfigFromUI();
    MessageBox(_T("生成登录器功能需要完整实现！"), _T("信息"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnOpenDirBtn() {
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    char* last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
    }
    ShellExecuteA(NULL, "open", exe_path, NULL, NULL, SW_SHOWNORMAL);
}
