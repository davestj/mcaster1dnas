/*
 * TrayIcon.cpp — macOS menu bar / Linux system tray icon.
 */

#include "TrayIcon.h"
#include <QIcon>
#include <QApplication>

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
{
    m_menu  = new QMenu;
    m_start = m_menu->addAction("Start Server");
    m_stop  = m_menu->addAction("Stop Server");
    m_menu->addSeparator();
    m_show  = m_menu->addAction("Open Window");
    m_menu->addSeparator();
    m_quit  = m_menu->addAction("Quit Mcaster1DNAS");

    m_stop->setEnabled(false);

    connect(m_start, &QAction::triggered, this, &TrayIcon::startRequested);
    connect(m_stop,  &QAction::triggered, this, &TrayIcon::stopRequested);
    connect(m_show,  &QAction::triggered, this, &TrayIcon::showWindowRequested);
    connect(m_quit,  &QAction::triggered, this, &TrayIcon::quitRequested);

    // Use embedded resource icon (converted from android-chrome-512×512)
    // Falls back to app window icon or theme icon if resources aren't available
    QIcon icon(":/icons/mcaster1.png");
    if (icon.isNull())
        icon = QApplication::windowIcon();
    if (icon.isNull())
        icon = QIcon::fromTheme("audio-x-generic",
                                QIcon::fromTheme("multimedia-player"));

    m_tray = new QSystemTrayIcon(icon, this);
    m_tray->setContextMenu(m_menu);
    m_tray->setToolTip("Mcaster1DNAS — stopped");
    m_tray->show();

    connect(m_tray, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger ||
            reason == QSystemTrayIcon::DoubleClick)
            emit showWindowRequested();
    });
}

void TrayIcon::setServerRunning(bool running)
{
    m_start->setEnabled(!running);
    m_stop->setEnabled(running);
    m_tray->setToolTip(running
        ? "Mcaster1DNAS — running"
        : "Mcaster1DNAS — stopped");
}
