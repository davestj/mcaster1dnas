#pragma once
/*
 * StatsTab — global server stats + sources list + per-source detail viewer.
 *
 * Mirrors Windows CMcaster1WinDlg layout:
 *   - statusTab.m_GlobalStatList  → m_globalTable  (top: 2-col stat/value)
 *   - statsTab.m_SourceListCtrl   → m_sourceList   (left: active source mounts)
 *   - statsTab.m_StatsListCtrl    → m_statsTable   (right: per-source stats)
 *
 * Populated entirely by StatsPoller::statsUpdated() signals.
 */

#include <QWidget>
#include <QSplitter>
#include <QListWidget>
#include <QTableWidget>
#include "StatsPoller.h"

class StatsTab : public QWidget
{
    Q_OBJECT
public:
    explicit StatsTab(QWidget *parent = nullptr);

public slots:
    void onStatsUpdated(GlobalStats gs, QList<SourceStats> sources);

private slots:
    void onSourceSelected(int row);

private:
    // ── Top: global server statistics (clients, listeners, uptime, etc.) ──────
    QTableWidget *m_globalTable  = nullptr;

    // ── Bottom: active source list (left) + per-source detail (right) ─────────
    QListWidget  *m_sourceList   = nullptr;
    QTableWidget *m_statsTable   = nullptr;

    QList<SourceStats> m_currentSources;
};
