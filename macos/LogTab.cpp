/*
 * LogTab.cpp — live log viewer (mirrors Windows CLogTab).
 *
 * Color scheme matches the Windows MFC version exactly:
 *   EROR → red    (0xFF, 0x40, 0x40)
 *   WARN → yellow (0xFF, 0xD7, 0x00)
 *   INFO → cyan   (0x00, 0xD7, 0xFF)
 *   DBUG → gray   (0x80, 0x80, 0x80)
 *   default → off-white (0xD2, 0xD2, 0xD2)
 */

#include "LogTab.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QFontDatabase>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QFile>

static const char *logFilename(LogTabKind kind)
{
    switch (kind) {
        case LogTabKind::Access:   return "access.log";
        case LogTabKind::Error:    return "error.log";
        case LogTabKind::Playlist: return "playlist.log";
        case LogTabKind::YPHealth: return "yp-health.log";
    }
    return "unknown.log";
}

LogTab::LogTab(LogTabKind kind, QWidget *parent)
    : QWidget(parent), m_kind(kind)
{
    m_edit = new QPlainTextEdit(this);
    m_edit->setReadOnly(true);
    m_edit->setMaximumBlockCount(10000);

    // Black background + monospace font — matches Windows Consolas 9pt
    QPalette pal = m_edit->palette();
    pal.setColor(QPalette::Base, Qt::black);
    pal.setColor(QPalette::Text, QColor(0xD2, 0xD2, 0xD2));
    m_edit->setPalette(pal);

    QFont mono;
#if defined(Q_OS_MAC)
    mono = QFont("Menlo", 10);
#else
    mono = QFont("Monospace", 10);
    mono.setStyleHint(QFont::TypeWriter);
#endif
    m_edit->setFont(mono);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->addWidget(m_edit);

    m_timer = new QTimer(this);
    m_timer->setInterval(POLL_INTERVAL_MS);
    connect(m_timer, &QTimer::timeout, this, &LogTab::onTimer);
}

void LogTab::setLogPath(const QString &dir)
{
    m_logPath = dir + "/" + logFilename(m_kind);

    // Reopen file at new path
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
    m_fileOffset = 0;

    m_file = new QFile(m_logPath, this);
    if (!m_file->open(QIODevice::ReadOnly))
    {
        // File may not exist yet — will be created when server writes to it
        delete m_file;
        m_file = nullptr;
    }
}

void LogTab::startTailing()
{
    m_timer->start();
}

void LogTab::stopTailing()
{
    m_timer->stop();
}

void LogTab::clearLog()
{
    m_edit->clear();
}

void LogTab::onTimer()
{
    // Try to (re-)open file if not open yet
    if (!m_file || !m_file->isOpen()) {
        if (m_logPath.isEmpty()) return;
        if (!m_file) m_file = new QFile(m_logPath, this);
        if (!m_file->open(QIODevice::ReadOnly)) return;
        m_fileOffset = 0;
    }

    // Read new bytes from current offset
    m_file->seek(m_fileOffset);
    QByteArray data = m_file->read(MAX_BUFFER_BYTES);
    if (data.isEmpty()) {
        // Check if file was rotated (size shrank)
        if (m_file->size() < m_fileOffset) {
            m_file->close();
            m_fileOffset = 0;
            m_file->open(QIODevice::ReadOnly);
        }
        return;
    }
    m_fileOffset = m_file->pos();

    // Split into lines and append with color coding
    QString text = QString::fromLocal8Bit(data);
    QStringList lines = text.split('\n');
    for (const QString &line : lines)
        if (!line.isEmpty())
            appendColorLine(line);

    // Auto-scroll to bottom
    QScrollBar *sb = m_edit->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogTab::appendColorLine(const QString &line)
{
    QTextCursor  cursor(m_edit->document());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat fmt;
    fmt.setForeground(QColor(0xD2, 0xD2, 0xD2));  // default off-white

    // Color coding for error.log only (matches Windows CLogTab exactly)
    if (m_kind == LogTabKind::Error) {
        if (line.contains("EROR"))
            fmt.setForeground(QColor(0xFF, 0x40, 0x40));  // red
        else if (line.contains("WARN"))
            fmt.setForeground(QColor(0xFF, 0xD7, 0x00));  // yellow
        else if (line.contains("INFO"))
            fmt.setForeground(QColor(0x00, 0xD7, 0xFF));  // cyan
        else if (line.contains("DBUG"))
            fmt.setForeground(QColor(0x80, 0x80, 0x80));  // gray
    }

    cursor.insertText(line + "\n", fmt);
}
