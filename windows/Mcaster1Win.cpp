// Mcaster1Win.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Mcaster1Win.h"
#include "Mcaster1WinDlg.h"
#include "xslt.h"
#include <git_hash.h>
#include "SslGen.h"

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

// ---------------------------------------------------------------------------
// Config file discovery helpers
// ---------------------------------------------------------------------------

// Return the directory containing the running exe (ends with backslash).
static void get_exe_dir_win(char *buf, int bufsz)
{
	buf[0] = '\0';
	GetModuleFileNameA(NULL, buf, bufsz);
	char *last = strrchr(buf, '\\');
	if (!last) last = strrchr(buf, '/');
	if (last) *(last + 1) = '\0';
}

// Check whether a file exists (using Win32 attributes to avoid CRT issues).
static bool file_exists(const char *path)
{
	DWORD attr = GetFileAttributesA(path);
	return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

// Show GetOpenFileName dialog so the user can browse to their config file.
// Returns TRUE if the user selected a file, FALSE if they cancelled.
static BOOL BrowseForConfig(char *out, int outSz)
{
	OPENFILENAMEA ofn = {};
	ofn.lStructSize  = sizeof(ofn);
	ofn.hwndOwner    = NULL;
	ofn.lpstrFilter  = "Config Files\0*.yaml;*.yml;*.xml\0All Files\0*.*\0";
	ofn.lpstrFile    = out;
	ofn.nMaxFile     = (DWORD)outSz;
	ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrTitle   = "Locate mcaster1dnas.yaml (or XML config)";
	out[0]           = '\0';
	return GetOpenFileNameA(&ofn);
}

// ---------------------------------------------------------------------------
// ParseMcasterCmdLine
// Returns TRUE  -> continue and show the main dialog.
// Returns FALSE -> handled (--ssl-gencert, -v, -h); exit immediately.
// ---------------------------------------------------------------------------
static BOOL ParseMcasterCmdLine(const char *cmdline,
                                 char *configFile, int configFileSize,
                                 BOOL &autoStart, BOOL &minimize)
{
	// SSL gen params (accumulated as we scan tokens)
	SslGenParams sslp;
	bool ssl_gencert = false;

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
			{
				strncpy_s(configFile, configFileSize, path, _TRUNCATE);
				sslp.configPath = path;
			}
		}
		else if (strcmp(token, "-s") == 0)
		{
			autoStart = TRUE;
		}
		else if (strcmp(token, "-m") == 0)
		{
			minimize = TRUE;
		}
		else if (strcmp(token, "--ssl-gencert") == 0)
		{
			ssl_gencert = true;
		}
		else if (strncmp(token, "--ssl-gentype=", 14) == 0)
		{
			sslp.gentype = token + 14;
		}
		else if (strncmp(token, "--subj=", 7) == 0)
		{
			sslp.subj = token + 7;
		}
		else if (strncmp(token, "--ssl-gencert-savepath=", 23) == 0)
		{
			sslp.savepath = token + 23;
		}
		else if (strncmp(token, "--ssl-gencert-addtoconfig=", 26) == 0)
		{
			sslp.addToConfig = (strcmp(token + 26, "true") == 0);
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
			char msg[2048];
			sprintf_s(msg, sizeof(msg),
				"Mcaster1DNAS %s\n\n"
				"Usage: mcaster1win.exe [options]\n\n"
				"Server options:\n"
				"  -c <file>    Specify configuration file (YAML or XML)\n"
				"  -s           Start server immediately on startup\n"
				"  -m           Minimize/hide to system tray on startup\n"
				"  -v           Display version information and exit\n"
				"  -h           Display this help and exit\n\n"
				"  If no -c file is given, looks for mcaster1dnas.yaml beside the exe,\n"
				"  then in the current directory, then shows a file-open dialog.\n\n"
				"SSL certificate generation (app exits after completing):\n"
				"  --ssl-gencert                         Generate SSL certificates\n"
				"  --ssl-gentype=selfsigned|csr           Output type\n"
				"  --subj=\"/C=US/ST=TX/O=Org/CN=host\"   X.509 subject\n"
				"  --ssl-gencert-savepath=<dir>          Output directory\n"
				"  --ssl-gencert-addtoconfig=true        Update -c config with new paths\n",
				GIT_VERSION);
			print_to_console(msg);
			return FALSE;
		}
		else if (token[0] != '-')
		{
			// Bare argument = config file path (backward compatibility)
			strncpy_s(configFile, configFileSize, token, _TRUNCATE);
			sslp.configPath = token;
		}
		// Unknown flags are silently ignored
	}

	// Handle --ssl-gencert: run generation, show result, exit.
	if (ssl_gencert)
	{
		if (sslp.savepath.empty())
		{
			MessageBoxA(NULL, "ssl-gen: --ssl-gencert-savepath=<dir> is required.",
						"Mcaster1DNAS SSL Gen", MB_OK | MB_ICONERROR);
			return FALSE;
		}
		int rc = CSslGen::Run(sslp);
		char msg[1024];
		if (rc == 0)
		{
			sprintf_s(msg, sizeof(msg),
				"SSL generation succeeded.\n\nFiles saved to:\n  %s",
				sslp.savepath.c_str());
			MessageBoxA(NULL, msg, "Mcaster1DNAS SSL Gen", MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			sprintf_s(msg, sizeof(msg),
				"SSL generation failed:\n\n%s",
				CSslGen::GetLastError().c_str());
			MessageBoxA(NULL, msg, "Mcaster1DNAS SSL Gen", MB_OK | MB_ICONERROR);
		}
		return FALSE;   // exit app after SSL gen
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMcaster1WinApp initialization

BOOL CMcaster1WinApp::InitInstance()
{
	AfxEnableControlContainer();

	// Default config — empty; discovery runs below.
	m_bAutoStart = FALSE;
	m_bMinimize  = FALSE;
	m_configFile[0] = '\0';

	// Parse command line; returns FALSE for -v / -h / --ssl-gencert (exit immediately).
	if (!ParseMcasterCmdLine(m_lpCmdLine, m_configFile, sizeof(m_configFile),
	                          m_bAutoStart, m_bMinimize))
		return FALSE;

	// Config file discovery (only if not set by -c flag).
	if (m_configFile[0] == '\0')
	{
		char exedir[MAX_PATH] = "";
		get_exe_dir_win(exedir, sizeof(exedir));

		// 1. <exedir>\mcaster1dnas.yaml
		char candidate[MAX_PATH];
		sprintf_s(candidate, sizeof(candidate), "%smcaster1dnas.yaml", exedir);
		if (file_exists(candidate))
		{
			strncpy_s(m_configFile, sizeof(m_configFile), candidate, _TRUNCATE);
		}
		else
		{
			// 2. .\mcaster1dnas.yaml
			if (file_exists("mcaster1dnas.yaml"))
			{
				strncpy_s(m_configFile, sizeof(m_configFile), "mcaster1dnas.yaml", _TRUNCATE);
			}
			else
			{
				// 3. File-open dialog
				char picked[MAX_PATH] = "";
				if (!BrowseForConfig(picked, sizeof(picked)))
					return FALSE;   // user cancelled
				strncpy_s(m_configFile, sizeof(m_configFile), picked, _TRUNCATE);
			}
		}
	}

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
