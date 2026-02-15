// Mcaster1Win.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Mcaster1Win.h"
#include "Mcaster1WinDlg.h"
#include "xslt.h"

extern "C" {
void _initialize_subsystems(void);
void _shutdown_subsystems(void);
}
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp

BEGIN_MESSAGE_MAP(CMcaster1WinApp, CWinApp)
	//{{AFX_MSG_MAP(CMcaster1WinApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

#include "colors.h"
/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp construction

CMcaster1WinApp::CMcaster1WinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMcaster1WinApp object

CMcaster1WinApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp initialization

BOOL CMcaster1WinApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	_initialize_subsystems();
	if (strlen(m_lpCmdLine) > 0) {
		strcpy(m_configFile, m_lpCmdLine);
	}
	else {
		strcpy(m_configFile, ".\\mcaster1.xml");
	}



#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMcaster1WinDlg dlg;
	m_pMainWnd = &dlg;

//	SetDialogBkColor(BGCOLOR,TEXTCOLOR); 

	m_pIconList[0] = LoadIcon (MAKEINTRESOURCE(IDR_MAINFRAME));

	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CMcaster1WinApp::ExitInstance()
{
    _shutdown_subsystems();
    return CWinApp::ExitInstance();
}
