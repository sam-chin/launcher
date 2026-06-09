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
    MessageBox(_T("Config saved!"), _T("Info"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnLoadBtn() {
    MessageBox(_T("Load config function needs implementation!"), _T("Info"), MB_OK | MB_ICONINFORMATION);
}

void CGeneratorDlg::OnGenerateBtn() {
    SaveConfigFromUI();
    MessageBox(_T("Generate launcher function needs full implementation!"), _T("Info"), MB_OK | MB_ICONINFORMATION);
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
