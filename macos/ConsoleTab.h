#pragma once
/*
 * ConsoleTab — captures server stdout (startup banner, printf output).
 *
 * Before the server thread calls server_init(), startCapture() replaces
 * STDOUT_FILENO with the write end of a pipe.  A QSocketNotifier on the
 * read end drains buffered output into a terminal-style QPlainTextEdit on
 * every Qt event-loop iteration — no polling, no blocking.
 *
 * Flow:  server printf() → STDOUT_FILENO → pipe[1]
 *                                          pipe[0] → QSocketNotifier → this tab
 */

#include <QWidget>
#include <QPlainTextEdit>
#include <QSocketNotifier>

class ConsoleTab : public QWidget
{
    Q_OBJECT
public:
    explicit ConsoleTab(QWidget *parent = nullptr);
    ~ConsoleTab() override;

    // Redirect STDOUT_FILENO through a pipe into this tab.
    // Call before server_init() so the startup banner is captured.
    void startCapture();

    // Restore the original stdout fd and close the pipe.
    void stopCapture();

    // Append a UI-level informational message (not from server stdout).
    void appendInfo(const QString &msg);

private:
    QPlainTextEdit  *m_edit        = nullptr;
    QSocketNotifier *m_notifier    = nullptr;
    int              m_readFd      = -1;
    int              m_savedStdout = -1;
};
