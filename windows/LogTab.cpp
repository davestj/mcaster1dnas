// LogTab.cpp — black-background log viewer tab for Mcaster1DNAS WinUI
//
// Each tab instance tails one log file (access, error, playlist, yp-health).
// Background is black; text is white-ish; error log lines are colour-coded:
//   EROR  -> red          WARN -> yellow
//   INFO  -> cyan         DBUG -> grey

#include "stdafx.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <share.h>
#include "LogTab.h"

extern "C" {
#ifdef HAVE_OPENSSL
#undef HAVE_OPENSSL   // avoid re-defining symbols already in pch
#define HAVE_OPENSSL
#endif
#include "../src/cfgfile.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Timer ID used by all CLogTab instances (per-window, so no collision)
static const UINT LOG_POLL_TIMER = 2;
static const UINT LOG_POLL_MS    = 2000;

// Max bytes read per poll (to avoid blocking the UI on huge log files)
static const long MAX_READ_PER_POLL = 65536;

/////////////////////////////////////////////////////////////////////////////
// CLogTab

BEGIN_MESSAGE_MAP(CLogTab, CTabPageSSL)
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CLogTab::CLogTab(LogTabKind kind)
    : CTabPageSSL(IDD_LOG_VIEW)
    , m_kind(kind)
    , m_fileOffset(0)
    , m_pathResolved(false)
    , m_showedStatus(false)
{
}

void CLogTab::SetLogPath(const char *path)
{
    m_logPath       = path ? path : "";
    m_pathResolved  = true;
    m_fileOffset    = 0;
    m_showedStatus  = false;
}

BOOL CLogTab::OnInitDialog()
{
    CTabPageSSL::OnInitDialog();

    // Create monospace rich edit to fill the entire client area
    CRect rc;
    GetClientRect(&rc);

    if (!m_rich.Create(
            ES_MULTILINE | ES_READONLY | WS_VISIBLE | WS_CHILD |
            WS_VSCROLL | ES_AUTOVSCROLL | ES_NOHIDESEL,
            rc, this, IDC_LOGEDIT))
    {
        return TRUE;   // will be an empty tab — not fatal
    }

    // Black background
    m_rich.SetBackgroundColor(FALSE, RGB(0, 0, 0));

    // Default character format: Consolas 9pt white
    CHARFORMAT2 cf  = {};
    cf.cbSize       = sizeof(CHARFORMAT2);
    cf.dwMask       = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_CHARSET;
    cf.dwEffects    = 0;
    cf.crTextColor  = RGB(210, 210, 210);
    cf.yHeight      = 9 * 20;   // 9pt in twips (1/20 pt)
    cf.bCharSet     = ANSI_CHARSET;
    lstrcpyn(cf.szFaceName, _T("Consolas"), LF_FACESIZE);
    m_rich.SetDefaultCharFormat(cf);

    // Limit rich edit to 4 MB to avoid runaway memory usage
    m_rich.LimitText(4 * 1024 * 1024);

    // Start polling timer
    SetTimer(LOG_POLL_TIMER, LOG_POLL_MS, NULL);

    return TRUE;
}

void CLogTab::OnSize(UINT nType, int cx, int cy)
{
    CTabPageSSL::OnSize(nType, cx, cy);
    if (m_rich.m_hWnd && cx > 0 && cy > 0)
        m_rich.MoveWindow(0, 0, cx, cy, TRUE);
}

void CLogTab::OnDestroy()
{
    KillTimer(LOG_POLL_TIMER);
    CTabPageSSL::OnDestroy();
}

void CLogTab::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == LOG_POLL_TIMER)
    {
        if (!m_pathResolved)
            ResolvePath();

        if (m_pathResolved)
        {
            if (m_logPath.IsEmpty())
            {
                if (!m_showedStatus)
                {
                    const char *label = "";
                    switch (m_kind)
                    {
                        case LTK_ACCESS:   label = "access log";   break;
                        case LTK_ERROR:    label = "error log";    break;
                        case LTK_PLAYLIST: label = "playlist log"; break;
                        case LTK_YPHEALTH: label = "YP health log";break;
                    }
                    char msg[256];
                    _snprintf_s(msg, sizeof(msg), _TRUNCATE,
                        "\r\n  No %s configured in mcaster1dnas.yaml.", label);
                    ShowStatus(msg, RGB(100, 100, 100));
                    m_showedStatus = true;
                }
            }
            else
            {
                PollFile();
            }
        }
    }
    CTabPageSSL::OnTimer(nIDEvent);
}

/////////////////////////////////////////////////////////////////////////////
// Private helpers

void CLogTab::ResolvePath()
{
    // config_get_config_unlocked() is safe to call from the GUI thread;
    // it returns NULL (or a zeroed struct) until server_init() completes.
    mc_config_t *cfg = config_get_config_unlocked();
    if (!cfg || !cfg->log_dir)
        return;     // server not started yet — retry next tick

    char path[1024] = "";
    switch (m_kind)
    {
    case LTK_ACCESS:
        if (cfg->access_log.name && cfg->access_log.name[0])
            _snprintf_s(path, sizeof(path), _TRUNCATE,
                "%s/%s", cfg->log_dir, cfg->access_log.name);
        break;

    case LTK_ERROR:
        if (cfg->error_log.name && cfg->error_log.name[0])
            _snprintf_s(path, sizeof(path), _TRUNCATE,
                "%s/%s", cfg->log_dir, cfg->error_log.name);
        break;

    case LTK_PLAYLIST:
        if (cfg->playlist_log.name && cfg->playlist_log.name[0])
            _snprintf_s(path, sizeof(path), _TRUNCATE,
                "%s/%s", cfg->log_dir, cfg->playlist_log.name);
        break;

    case LTK_YPHEALTH:
        if (cfg->num_yp_directories > 0 &&
            cfg->yp_logfile[0]      &&
            cfg->yp_logfile[0][0])
        {
            strncpy_s(path, sizeof(path), cfg->yp_logfile[0], _TRUNCATE);
        }
        break;
    }

    // Replace backslashes with forward slashes for consistency
    for (char *p = path; *p; p++)
        if (*p == '\\') *p = '/';

    m_logPath      = path;   // empty string if not configured
    m_pathResolved = true;
    m_fileOffset   = 0;
}

void CLogTab::PollFile()
{
    if (!m_rich.m_hWnd) return;

    // Use _fsopen with _SH_DENYNO so the file can be read even while
    // the server has it open for writing (Windows file-share mode).
    FILE *fp = _fsopen(m_logPath, "rb", _SH_DENYNO);
    if (!fp)
    {
        if (!m_showedStatus)
        {
            char msg[512];
            _snprintf_s(msg, sizeof(msg), _TRUNCATE,
                "\r\n  Waiting for log file:\r\n  %s", (LPCSTR)m_logPath);
            ShowStatus(msg, RGB(90, 90, 90));
            m_showedStatus = true;
        }
        return;
    }

    // Check for log rotation (file shrank)
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);

    if (fileSize < m_fileOffset)
    {
        // File was rotated — restart from zero
        m_fileOffset   = 0;
        m_showedStatus = false;
        m_rich.SetWindowText(_T(""));
    }

    if (fileSize <= m_fileOffset)
    {
        fclose(fp);
        return;
    }

    // Read up to MAX_READ_PER_POLL new bytes
    long toRead = fileSize - m_fileOffset;
    if (toRead > MAX_READ_PER_POLL) toRead = MAX_READ_PER_POLL;

    char *buf = new char[toRead + 2];
    fseek(fp, m_fileOffset, SEEK_SET);
    long nRead = (long)fread(buf, 1, toRead, fp);
    fclose(fp);

    if (nRead <= 0) { delete[] buf; return; }

    buf[nRead] = '\0';
    m_fileOffset += nRead;
    m_showedStatus = true;   // we have real content now

    // Split into lines and append with colour coding
    char *p     = buf;
    char *start = buf;
    while (*p)
    {
        if (*p == '\n')
        {
            *p = '\0';
            // Strip trailing \r
            if (p > start && *(p-1) == '\r') *(p-1) = '\0';
            if (*start) AppendLine(start);
            start = p + 1;
        }
        p++;
    }
    // Partial last line — back off so we re-read it next poll
    if (*start && start < p)
        m_fileOffset -= (long)(p - start);

    delete[] buf;

    // Scroll to bottom
    m_rich.SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

void CLogTab::ShowStatus(const char *msg, COLORREF col)
{
    if (!m_rich.m_hWnd) return;
    CHARFORMAT2 cf  = {};
    cf.cbSize       = sizeof(CHARFORMAT2);
    cf.dwMask       = CFM_COLOR;
    cf.dwEffects    = 0;
    cf.crTextColor  = col;

    m_rich.SetWindowText(_T(""));
    m_rich.SetSel(0, 0);
    m_rich.SetSelectionCharFormat(cf);
    m_rich.ReplaceSel(CString(msg));
}

void CLogTab::AppendLine(const char *line)
{
    if (!m_rich.m_hWnd || !line || !*line) return;

    CHARFORMAT2 cf  = {};
    cf.cbSize       = sizeof(CHARFORMAT2);
    cf.dwMask       = CFM_COLOR;
    cf.dwEffects    = 0;
    cf.crTextColor  = LineColor(line);

    long len = m_rich.GetTextLength();
    m_rich.SetSel(len, len);
    m_rich.SetSelectionCharFormat(cf);

    CString s(line);
    s += "\r\n";
    m_rich.ReplaceSel(s);
}

COLORREF CLogTab::LineColor(const char *line)
{
    if (m_kind != LTK_ERROR) return RGB(210, 210, 210);

    // Log format: [DATE  TIME] LEVEL module/function message
    if (strstr(line, "] EROR ")) return RGB(255,  70,  70);   // red
    if (strstr(line, "] WARN ")) return RGB(255, 210,   0);   // yellow
    if (strstr(line, "] INFO ")) return RGB( 80, 200, 255);   // cyan
    if (strstr(line, "] DBUG ")) return RGB(140, 140, 140);   // grey
    return RGB(210, 210, 210);                                  // default white-ish
}
