// Mcaster1Win.h : main header file for the MCASTER1WIN application
//

#if !defined(AFX_MCASTER1WIN_H__76A528C9_A424_4417_BFDF_0E556A9EE4F1__INCLUDED_)
#define AFX_MCASTER1WIN_H__76A528C9_A424_4417_BFDF_0E556A9EE4F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__	
#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp:
// See Mcaster1Win.cpp for the implementation of this class
//

class CMcaster1WinApp : public CWinApp
{
public:
	char m_configFile[1024];
	HICON m_pIconList[2];
	BOOL m_bAutoStart;   // -s: start server immediately on startup
	BOOL m_bMinimize;    // -m: hide to system tray on startup
	CMcaster1WinApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcaster1WinApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMcaster1WinApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

extern CMcaster1WinApp theApp;
#endif // !defined(AFX_MCASTER1WIN_H__76A528C9_A424_4417_BFDF_0E556A9EE4F1__INCLUDED_)
