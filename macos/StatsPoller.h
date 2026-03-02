#pragma once
/*
 * StatsPoller — background stats collection thread.
 *
 * Mirrors the Windows MFC StartStats/_beginthread pattern:
 * every 5 seconds calls stats_get_xml(STATS_PUBLIC, NULL), parses the result,
 * and emits statsUpdated() with structured data for the StatsTab to display.
 *
 * Runs on a QThread worker so the UI thread is never blocked.
 */

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QList>

struct SourceStats {
    QString mount;
    QMap<QString, QString> fields;   // stat name → value
};

struct GlobalStats {
    int sources   = 0;
    int clients   = 0;
    int listeners = 0;
    QString serverStart;
    QMap<QString, QString> extra;
};

class StatsPoller : public QObject
{
    Q_OBJECT
public:
    explicit StatsPoller(QObject *parent = nullptr);

public slots:
    void startPolling();
    void stopPolling();

signals:
    void statsUpdated(GlobalStats globalSt, QList<SourceStats> sources);

private slots:
    void poll();

private:
    QTimer *m_timer = nullptr;
};
