// Mcaster1Win.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Mcaster1Win.h"
#include "Mcaster1WinDlg.h"
#include "xslt.h"
#include <git_hash.h>

// _initialize_subsystems / _shutdown_subsystems are called only by main()
// on the server thread. The GUI InitInstance must NOT call them to avoid
// double-init of pthreads, mutexes, and AVL trees which corrupts server state.
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
	m_bAutoStart = FALSE;
	m_bMinimize  = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMcaster1WinApp object

CMcaster1WinApp theApp;

/////////////////////////////////////////////////////////////////////////////
// Command-line helpers

// Print a message to the parent console (if launched from one) or a MessageBox.
static void print_to_console(const char *msg)
{
	if (AttachConsole(ATTACH_PARENT_PROCESS))
	{
		FILE *dummy;
		freopen_s(&dummy, "CONOUT$", "w", stdout);
		// Print a newline first so output appears on its own line (the shell
		// prompt is already on the current line when the GUI is launched).
		printf("\n%s", msg);
		fflush(stdout);
		FreeConsole();
	}
	else
	{
		MessageBox(NULL, msg, "Mcaster1DNAS", MB_OK | MB_ICONINFORMATION);
	}
}

// Parse Mcaster1Win command line flags.
// Returns TRUE  -> continue and show the dialog.
// Returns FALSE -> -v or -h was handled; exit immediately.
static BOOL ParseMcasterCmdLine(const char *cmdline,
                                 char *configFile, int configFileSize,
                                 BOOL &autoStart, BOOL &minimize)
{
	if (!cmdline || !cmdline[0])
		return TRUE;

	char buf[4096];
	strncpy_s(buf, sizeof(buf), cmdline, _TRUNCATE);
	char *p = buf;

	while (*p)
	{
		// Skip whitespace
		while (*p == ' ' || *p == '\t') p++;
		if (!*p) break;

		// Collect one token (handle double-quoted paths)
		char token[2048] = "";
		int  ti = 0;
		if (*p == '"')
		{
			p++;
			while (*p && *p != '"' && ti < (int)sizeof(token) - 1)
				token[ti++] = *p++;
			if (*p == '"') p++;
		}
		else
		{
			while (*p && *p != ' ' && *p != '\t' && ti < (int)sizeof(token) - 1)
				token[ti++] = *p++;
		}
		token[ti] = '\0';

		if (strcmp(token, "-c") == 0)
		{
			// Next token is the config file path
			while (*p == ' ' || *p == '\t') p++;
			char path[2048] = "";
			int  pi = 0;
			if (*p == '"')
			{
				p++;
				while (*p && *p != '"' && pi < (int)sizeof(path) - 1)
					path[pi++] = *p++;
				if (*p == '"') p++;
			}
			else
			{
				while (*p && *p != ' ' && *p != '\t' && pi < (int)sizeof(path) - 1)
					path[pi++] = *p++;
			}
			path[pi] = '\0';
			if (pi > 0)
				strncpy_s(configFile, configFileSize, path, _TRUNCATE);
		}
		else if (strcmp(token, "-s") == 0)
		{
			autoStart = TRUE;
		}
		else if (strcmp(token, "-m") == 0)
		{
			minimize = TRUE;
		}
		else if (strcmp(token, "-v") == 0)
		{
			char msg[512];
			sprintf_s(msg, sizeof(msg), "Mcaster1DNAS %s\n", GIT_VERSION);
			print_to_console(msg);
			return FALSE;
		}
		else if (strcmp(token, "-h") == 0 || strcmp(token, "--help") == 0)
		{
			char msg[1024];
			sprintf_s(msg, sizeof(msg),
				"Mcaster1DNAS %s\n\n"
				"Usage: mcaster1win.exe [options]\n\n"
				"Options:\n"
				"  -c <file>    Specify configuration file (YAML or XML auto-detected)\n"
				"  -s           Start server immediately on startup\n"
				"  -m           Minimize/hide to system tray on startup\n"
				"  -v           Display version information and exit\n"
				"  -h           Display this help and exit\n\n"
				"Default config: .\\mcaster1.yaml\n",
				GIT_VERSION);
			print_to_console(msg);
			return FALSE;
		}
		else if (token[0] != '-')
		{
			// Bare argument = config file path (backward compatibility)
			strncpy_s(configFile, configFileSize, token, _TRUNCATE);
		}
		// Unknown flags are silently ignored
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp initialization

BOOL CMcaster1WinApp::InitInstance()
{
	AfxEnableControlContainer();

	// Default config
	m_bAutoStart = FALSE;
	m_bMinimize  = FALSE;
	strcpy(m_configFile, ".\\mcaster1.yaml");

	// Parse command line; returns FALSE for -v / -h (print and exit)
	if (!ParseMcasterCmdLine(m_lpCmdLine, m_configFile, sizeof(m_configFile),
	                          m_bAutoStart, m_bMinimize))
		return FALSE;

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CMcaster1WinDlg dlg;
	m_pMainWnd = &dlg;

	m_pIconList[0] = LoadIcon(MAKEINTRESOURCE(IDR_MAINFRAME));

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
    return CWinApp::ExitInstance();
}
