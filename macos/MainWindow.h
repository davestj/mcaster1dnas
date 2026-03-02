#pragma once
/*
 * MainWindow — top-level QMainWindow.
 *
 * Tab layout mirrors Windows CMcaster1WinDlg:
 *   [Overview] [Stats] [Log: Access] [Log: Error] [Log: Playlist] [Config]
 *
 * Toolbar (Phase 2):
 *   [Logo] | [Start Server ▶] [Stop Server ■] | [status image] | [uptime]
 *
 * A 500 ms QTimer polls global.running / global.sources / global.clients /
 * global.listeners to update the status bar and window title — same as
 * the Windows MFC SetTimer(1, 500, NULL) polling loop.
 *
 * The StatsPoller runs on a dedicated QThread to call stats_get_xml() every 5 s
 * without blocking the UI — same as Windows _beginthread(StartStats).
 */

#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>
#include <QLabel>
#include <QThread>
#include <QString>
#include <QDateTime>
#include <QToolBar>
#include <QPushButton>
#include <QPixmap>
#include <QAction>

#include "ServerThread.h"
#include "StatsPoller.h"
#include "LogTab.h"
#include "StatsTab.h"
#include "ConfigTab.h"
#include "ConsoleTab.h"
#include "HtmlTab.h"
#include "TrayIcon.h"

// Forward declared so the overview widget can be a simple QWidget
class OverviewTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(int argc, char *argv[], QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPollTimer();
    void onServerStarted();
    void onServerStopped();
    void onServerError(const QString &msg);
    void onStatsUpdated(GlobalStats gs, QList<SourceStats> sources);
    void onStartServer();
    void onStopServer();
    void onShowWindow();
    void onQuit();
    void onConfigSaved(const QString &path);
    void onConfigManager();
    void onOpenWebFrontend();
    void onOpenWebAdmin();
    void onBlinkTimer();

private:
    QString discoverConfig(int argc, char *argv[]) const;
    QString discoverServerUrl(bool adminPath) const;
    void    updateStatusBar(bool running, int sources, int clients, int listeners);
    void    updateUptime();
    void    setServerRunningUI(bool running);

    // ── server lifecycle ───────────────────────────────────────────────────────
    ServerThread *m_server      = nullptr;
    QThread      *m_statsThread = nullptr;
    StatsPoller  *m_statsPoller = nullptr;

    // ── 500 ms UI poll timer ───────────────────────────────────────────────────
    QTimer   *m_pollTimer = nullptr;
    QDateTime m_startTime;

    // ── config path (resolved at startup) ─────────────────────────────────────
    QString   m_configPath;

    // ── subsystem lifecycle tracking ───────────────────────────────────────────
    // True only while the server is running and initialize_subsystems() has been
    // called but shutdown_subsystems() has NOT yet run.  ConfigDialog uses this
    // to decide whether config_parse_file() is safe (logging rwlock is alive).
    bool      m_subsystemsAlive = false;

    // ── toolbar widgets ────────────────────────────────────────────────────────
    QToolBar    *m_toolbar         = nullptr;
    QPushButton *m_configBtn       = nullptr;   // ⚙ Config — opens Configuration Manager
    QAction     *m_configAction    = nullptr;   // Tools > Configuration Manager (mirrors button)
    QPushButton *m_startBtn        = nullptr;
    QPushButton *m_stopBtn         = nullptr;
    QPushButton *m_webFrontendBtn  = nullptr;   // 🌐 Web UI — opens browser to /
    QPushButton *m_webAdminBtn     = nullptr;   // ⚙ Admin  — opens browser to /admin/
    QLabel      *m_toolbarStatus   = nullptr;   // running.png / stopped.png
    QLabel      *m_toolbarUptime   = nullptr;

    // ── tabs ───────────────────────────────────────────────────────────────────
    QTabWidget  *m_tabs        = nullptr;
    OverviewTab *m_overview    = nullptr;
    StatsTab    *m_statsTab    = nullptr;
    LogTab      *m_logAccess   = nullptr;
    LogTab      *m_logError    = nullptr;
    LogTab      *m_logPlaylist = nullptr;
    ConfigTab   *m_configTab   = nullptr;
    ConsoleTab  *m_consoleTab  = nullptr;
    HtmlTab     *m_creditsTab  = nullptr;
    HtmlTab     *m_helpTab     = nullptr;

    // ── tray icon ─────────────────────────────────────────────────────────────
    TrayIcon *m_tray = nullptr;

    // ── status bar labels ─────────────────────────────────────────────────────
    QLabel *m_statusLabel  = nullptr;
    QLabel *m_uptimeLabel  = nullptr;

    // ── status indicator blink timer ───────────────────────────────────────────
    QTimer *m_blinkTimer = nullptr;
    bool    m_blinkState = false;

    // ── cached pixmaps for running/stopped indicator ──────────────────────────
    QPixmap m_pxRunning;
    QPixmap m_pxStopped;
};
