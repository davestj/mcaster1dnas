// ConfigTab.cpp : implementation file
//

#include "stdafx.h"
#include "Mcaster1Win.h"
#include "ConfigTab.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigTab dialog


CConfigTab::CConfigTab(CWnd* pParent /*=NULL*/)
	: CTabPageSSL(CConfigTab::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigTab)
	m_Config = _T("");
	//}}AFX_DATA_INIT
}


void CConfigTab::DoDataExchange(CDataExchange* pDX)
{
	CTabPageSSL::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigTab)
	DDX_Control(pDX, IDC_CONFIG, m_ConfigCtrl);
	DDX_Text(pDX, IDC_CONFIG, m_Config);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigTab, CTabPageSSL)
	//{{AFX_MSG_MAP(CConfigTab)
	ON_EN_KILLFOCUS(IDC_CONFIG, OnKillfocusConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigTab message handlers

void CConfigTab::SaveConfiguration()
{
	// TODO: Add your command handler code here
	// TODO: Add your control notification handler code here
	FILE	*filep;
	char	buffer[2046] = "";
	CMcaster1WinApp	*myApp = (CMcaster1WinApp *)AfxGetApp();

	UpdateData(TRUE);
	if (m_Config != "") {
		filep = fopen(myApp->m_configFile, "w");
		if (filep) {
			fputs(LPCSTR(m_Config), filep);
			fclose(filep);
		}
	}
}

BOOL CConfigTab::OnInitDialog() 
{
	CTabPageSSL::OnInitDialog();
	
	// TODO: Add extra initialization here
//	AddAnchor(IDC_CONFIG, TOP_LEFT, BOTTOM_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigTab::OnKillfocusConfig() 
{
	// TODO: Add your control notification handler code here
	SaveConfiguration();
}
