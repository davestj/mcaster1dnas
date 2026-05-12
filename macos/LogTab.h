#pragma once
/*
 * LogTab — live log file viewer.
 *
 * Mirrors Windows CLogTab exactly:
 *   - Black background QPlainTextEdit with Menlo/Consolas 10pt
 *   - QTimer fires every 2000 ms, reads new bytes from m_fileOffset
 *   - Color-codes error.log lines: EROR→red, WARN→yellow, INFO→cyan, DBUG→gray
 *   - Caps buffer at 4 MB to prevent runaway memory
 *   - Reads log file path from config->log_dir
 */

#include <QWidget>
#include <QPlainTextEdit>
#include <QTimer>
#include <QString>
#include <QFile>

enum class LogTabKind {
    Access,
    Error,
    Playlist,
    YPHealth
};

class LogTab : public QWidget
{
    Q_OBJECT
public:
    explicit LogTab(LogTabKind kind, QWidget *parent = nullptr);

public slots:
    void setLogPath(const QString &dir);
    void startTailing();
    void stopTailing();
    void clearLog();

private slots:
    void onTimer();

private:
    void appendColorLine(const QString &line);

    LogTabKind       m_kind;
    QString          m_logPath;
    QPlainTextEdit  *m_edit   = nullptr;
    QTimer          *m_timer  = nullptr;
    QFile           *m_file   = nullptr;
    qint64           m_fileOffset = 0;

    static constexpr int MAX_BUFFER_BYTES = 4 * 1024 * 1024;
    static constexpr int POLL_INTERVAL_MS = 2000;
};
