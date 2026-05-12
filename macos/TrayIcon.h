#pragma once
/*
 * TrayIcon — macOS menu bar / Linux system tray icon.
 * On macOS: appears in the menu bar.
 * On Linux: appears in the system tray (if a tray is available).
 * Mirrors Windows CTrayNot with Start / Stop / Open Window / Quit actions.
 */

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class TrayIcon : public QObject
{
    Q_OBJECT
public:
    explicit TrayIcon(QObject *parent = nullptr);
    void setServerRunning(bool running);

signals:
    void startRequested();
    void stopRequested();
    void showWindowRequested();
    void quitRequested();

private:
    QSystemTrayIcon *m_tray   = nullptr;
    QMenu           *m_menu   = nullptr;
    QAction         *m_start  = nullptr;
    QAction         *m_stop   = nullptr;
    QAction         *m_show   = nullptr;
    QAction         *m_quit   = nullptr;
};
