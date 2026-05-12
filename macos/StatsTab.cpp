/*
 * StatsTab.cpp — mirrors Windows CMcaster1WinDlg stats layout.
 *
 * Three panels:
 *   Top:          m_globalTable   — global server stats (clients, listeners, uptime…)
 *   Bottom-left:  m_sourceList    — active source mounts
 *   Bottom-right: m_statsTable    — per-source stats when a mount is selected
 *
 * Populated entirely by StatsPoller::statsUpdated() signals.
 */

#include "StatsTab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSplitter>
#include <QGroupBox>

StatsTab::StatsTab(QWidget *parent)
    : QWidget(parent)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setSpacing(6);
    outerLayout->setContentsMargins(4, 4, 4, 4);

    // ── Top: Global server statistics ─────────────────────────────────────────
    auto *globalGroup = new QGroupBox("Global Server Statistics", this);
    auto *globalGroupLayout = new QVBoxLayout(globalGroup);
    globalGroupLayout->setContentsMargins(4, 4, 4, 4);

    m_globalTable = new QTableWidget(0, 2, globalGroup);
    m_globalTable->setHorizontalHeaderLabels({"Statistic", "Value"});
    m_globalTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_globalTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_globalTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_globalTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_globalTable->verticalHeader()->setVisible(false);
    m_globalTable->setAlternatingRowColors(true);
    m_globalTable->setMaximumHeight(160);
    globalGroupLayout->addWidget(m_globalTable);
    outerLayout->addWidget(globalGroup);

    // ── Bottom: source list (left) + per-source stats (right) ─────────────────
    auto *sourcesGroup = new QGroupBox("Sources", this);
    auto *sourcesGroupLayout = new QVBoxLayout(sourcesGroup);
    sourcesGroupLayout->setContentsMargins(4, 4, 4, 4);

    auto *splitter = new QSplitter(Qt::Horizontal, sourcesGroup);

    m_sourceList = new QListWidget(splitter);
    m_sourceList->setMinimumWidth(160);
    splitter->addWidget(m_sourceList);

    m_statsTable = new QTableWidget(0, 2, splitter);
    m_statsTable->setHorizontalHeaderLabels({"Statistic", "Value"});
    m_statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_statsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_statsTable->verticalHeader()->setVisible(false);
    m_statsTable->setAlternatingRowColors(true);
    splitter->addWidget(m_statsTable);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

    sourcesGroupLayout->addWidget(new QLabel("Click a source to view its statistics:", sourcesGroup));
    sourcesGroupLayout->addWidget(splitter);
    outerLayout->addWidget(sourcesGroup, 1);

    connect(m_sourceList, &QListWidget::currentRowChanged,
            this, &StatsTab::onSourceSelected);
}

// ── Helper: add/update a row in a 2-column table ──────────────────────────────
static void setTableRow(QTableWidget *tbl, int row,
                        const QString &stat, const QString &value)
{
    auto *statItem  = new QTableWidgetItem(stat);
    auto *valueItem = new QTableWidgetItem(value);
    statItem->setFlags(statItem->flags() & ~Qt::ItemIsEditable);
    valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
    tbl->setItem(row, 0, statItem);
    tbl->setItem(row, 1, valueItem);
}

void StatsTab::onStatsUpdated(GlobalStats gs, QList<SourceStats> sources)
{
    // ── Update global stats table ──────────────────────────────────────────────
    {
        // Fixed rows first, then any extra fields from the poller
        const int fixedRows = 4;
        const int extraRows = gs.extra.size();
        const int totalRows = fixedRows + extraRows;

        m_globalTable->setRowCount(totalRows);

        setTableRow(m_globalTable, 0, "Clients (total)",   QString::number(gs.clients));
        setTableRow(m_globalTable, 1, "Listeners",         QString::number(gs.listeners));
        setTableRow(m_globalTable, 2, "Sources / Mounts",  QString::number(gs.sources));
        setTableRow(m_globalTable, 3, "Server started",    gs.serverStart.isEmpty()
                                                             ? "(not reported)"
                                                             : gs.serverStart);

        int row = fixedRows;
        for (auto it = gs.extra.cbegin(); it != gs.extra.cend(); ++it, ++row)
            setTableRow(m_globalTable, row, it.key(), it.value());
    }

    // ── Update source list, preserving current selection ──────────────────────
    QString selectedMount;
    if (m_sourceList->currentItem())
        selectedMount = m_sourceList->currentItem()->text();

    m_currentSources = sources;
    m_sourceList->clear();
    for (const SourceStats &ss : sources)
        m_sourceList->addItem(ss.mount);

    // Restore selection if possible
    for (int i = 0; i < m_sourceList->count(); ++i) {
        if (m_sourceList->item(i)->text() == selectedMount) {
            m_sourceList->setCurrentRow(i);
            return;   // onSourceSelected() will refresh m_statsTable
        }
    }

    // Selection lost (mount disconnected) — clear per-source view
    m_statsTable->setRowCount(0);
}

void StatsTab::onSourceSelected(int row)
{
    m_statsTable->setRowCount(0);
    if (row < 0 || row >= m_currentSources.size())
        return;

    const SourceStats &ss = m_currentSources[row];
    m_statsTable->setRowCount(ss.fields.size());

    int r = 0;
    for (auto it = ss.fields.cbegin(); it != ss.fields.cend(); ++it, ++r)
        setTableRow(m_statsTable, r, it.key(), it.value());
}
