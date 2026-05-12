/*
 * ConsoleTab.cpp — terminal-style stdout capture for the server startup banner.
 *
 * startCapture() saves the original STDOUT_FILENO via dup(), then replaces it
 * with the write end of a pipe.  A QSocketNotifier fires whenever data is
 * available on the read end.  The read is non-blocking so we drain all queued
 * bytes in a tight loop without stalling the Qt event loop.
 */

#include "ConsoleTab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QFont>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QDateTime>

#include <unistd.h>
#include <fcntl.h>

ConsoleTab::ConsoleTab(QWidget *parent)
    : QWidget(parent)
{
    m_edit = new QPlainTextEdit(this);
    m_edit->setReadOnly(true);
    m_edit->setMaximumBlockCount(8000);

    // Dark terminal look — distinct from the black log tabs
    QPalette pal = m_edit->palette();
    pal.setColor(QPalette::Base, QColor(0x0d, 0x10, 0x17));   // very dark blue
    pal.setColor(QPalette::Text, QColor(0xcc, 0xff, 0xcc));   // light green
    m_edit->setPalette(pal);

    QFont mono;
#if defined(Q_OS_MAC)
    mono = QFont("Menlo", 10);
#else
    mono = QFont("Monospace", 10);
    mono.setStyleHint(QFont::TypeWriter);
#endif
    m_edit->setFont(mono);

    // Clear button in a thin toolbar row above the text
    auto *clearBtn = new QPushButton("Clear", this);
    clearBtn->setFixedWidth(60);
    clearBtn->setStyleSheet(
        "QPushButton { font-size:10px; padding:1px 6px; }"
    );
    connect(clearBtn, &QPushButton::clicked, m_edit, &QPlainTextEdit::clear);

    auto *topRow = new QHBoxLayout;
    topRow->setContentsMargins(2, 2, 2, 0);
    topRow->addWidget(clearBtn);
    topRow->addStretch();

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addLayout(topRow);
    lay->addWidget(m_edit);

    appendInfo("Console ready — server stdout will appear here after Start Server.");
}

ConsoleTab::~ConsoleTab()
{
    stopCapture();
}

// ── startCapture ─────────────────────────────────────────────────────────────

void ConsoleTab::startCapture()
{
    if (m_readFd != -1) return;   // already capturing

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        appendInfo("[ConsoleTab] pipe() failed — stdout not captured.");
        return;
    }

    // Make the read end non-blocking so the drain loop never stalls.
    int fl = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, fl | O_NONBLOCK);

    // Save original stdout before overwriting it.
    m_savedStdout = dup(STDOUT_FILENO);

    // Point STDOUT_FILENO at the write end of the pipe.
    dup2(pipefd[1], STDOUT_FILENO);
    ::close(pipefd[1]);   // we only need the alias through STDOUT_FILENO now

    m_readFd = pipefd[0];

    // QSocketNotifier fires each time the read-end has data available.
    m_notifier = new QSocketNotifier(m_readFd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, [this](QSocketDescriptor fd, QSocketNotifier::Type) {
        char buf[4096];
        ssize_t n;
        // Non-blocking drain — read until EAGAIN.
        while ((n = ::read(static_cast<int>(fd), buf, sizeof(buf) - 1)) > 0) {
            QString text = QString::fromLocal8Bit(buf, static_cast<int>(n));
            const QStringList lines = text.split('\n');
            for (const QString &line : lines) {
                if (!line.isEmpty()) {
                    QTextCursor cur(m_edit->document());
                    cur.movePosition(QTextCursor::End);
                    QTextCharFormat fmt;
                    fmt.setForeground(QColor(0xcc, 0xff, 0xcc));
                    cur.insertText(line + "\n", fmt);
                }
            }
        }
        m_edit->verticalScrollBar()->setValue(
            m_edit->verticalScrollBar()->maximum());
    });

    appendInfo("[ConsoleTab] stdout capture active — server output below:");
    appendInfo(QString(60, QChar('-')));
}

// ── stopCapture ──────────────────────────────────────────────────────────────

void ConsoleTab::stopCapture()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        delete m_notifier;
        m_notifier = nullptr;
    }
    // Restore the original stdout so the terminal/log stays functional.
    if (m_savedStdout != -1) {
        dup2(m_savedStdout, STDOUT_FILENO);
        ::close(m_savedStdout);
        m_savedStdout = -1;
    }
    if (m_readFd != -1) {
        ::close(m_readFd);
        m_readFd = -1;
    }
}

// ── appendInfo ───────────────────────────────────────────────────────────────

void ConsoleTab::appendInfo(const QString &msg)
{
    QTextCursor cur(m_edit->document());
    cur.movePosition(QTextCursor::End);
    QTextCharFormat fmt;
    fmt.setForeground(QColor(0x88, 0x88, 0xff));   // dim blue for system msgs
    cur.insertText(msg + "\n", fmt);
    m_edit->verticalScrollBar()->setValue(
        m_edit->verticalScrollBar()->maximum());
}
