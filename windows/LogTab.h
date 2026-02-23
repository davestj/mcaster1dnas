#pragma once
// LogTab.h — black-background log file viewer tab for Mcaster1DNAS WinUI
// Tails a log file every 2 seconds and displays new lines with colour coding.

#include "TabPageSSL.h"
#include "resource.h"

enum LogTabKind
{
    LTK_ACCESS   = 0,   // access.log
    LTK_ERROR    = 1,   // error.log  (colourises INFO/WARN/EROR/DBUG lines)
    LTK_PLAYLIST = 2,   // playlist.log
    LTK_YPHEALTH = 3,   // yp-health.log
};

class CLogTab : public CTabPageSSL
{
public:
    explicit CLogTab(LogTabKind kind);

    // Optionally force a specific path (overrides auto-detect from config).
    void SetLogPath(const char *path);

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

private:
    LogTabKind      m_kind;
    CRichEditCtrl   m_rich;
    CFont           m_font;
    CString         m_logPath;
    long            m_fileOffset;
    bool            m_pathResolved;
    bool            m_showedStatus;   // true once "not configured" has been shown

    void        ResolvePath();
    void        PollFile();
    void        ShowStatus(const char *msg, COLORREF col = RGB(110, 110, 110));
    void        AppendLine(const char *line);
    COLORREF    LineColor(const char *line);
};
