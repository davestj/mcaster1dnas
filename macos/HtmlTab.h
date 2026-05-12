#pragma once
/*
 * HtmlTab — QWidget wrapper around QWebEngineView.
 *
 * Loads a local HTML file from the docs/ directory that lives alongside the
 * application binary.  On macOS app bundles the binary is at:
 *   Mcaster1DNAS.app/Contents/MacOS/Mcaster1DNAS
 * so QCoreApplication::applicationDirPath() returns  "…/Contents/MacOS"
 * and docs are copied there by the CMake post-build step, giving:
 *   "…/Contents/MacOS/docs/<filename>"
 *
 * On a plain build-dir launch (make gui + run directly) the binary sits at:
 *   macos/build-qt/Mcaster1DNAS.app/Contents/MacOS/Mcaster1DNAS
 * — same logic applies.
 *
 * Right-click / two-finger trackpad tap context menu:
 *   Qt 6.10 does NOT expose QWebEnginePage::contextMenuRequested.
 *   Instead we subclass QWebEngineView (McasterWebView) and override
 *   contextMenuEvent — the method Qt 6.10 itself overrides in QWebEngineView
 *   to show the Chromium menu.  Our override fires first and shows ours.
 *   No Q_OBJECT needed on McasterWebView; std::function carries the callback.
 */

#include <QWidget>
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QUrl>
#include <QCoreApplication>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QPoint>
#include <QContextMenuEvent>
#include <functional>

// ── Private helper: QWebEngineView subclass that intercepts contextMenuEvent ──
// No Q_OBJECT — uses std::function to avoid MOC complexity in a header-only class.
class McasterWebView : public QWebEngineView
{
public:
    explicit McasterWebView(QWidget *parent = nullptr) : QWebEngineView(parent) {}

    // Set by HtmlTab to receive the global position when a context menu fires.
    std::function<void(QPoint)> contextMenuCallback;

protected:
    void contextMenuEvent(QContextMenuEvent *e) override
    {
        // Qt 6.10 QWebEngineView::contextMenuEvent() shows Chromium's menu.
        // We intercept here and show our own instead.
        if (contextMenuCallback)
            contextMenuCallback(e->globalPos());
        e->accept();   // mark handled — prevent default Chromium popup
    }
};

// ─────────────────────────────────────────────────────────────────────────────

class HtmlTab : public QWidget
{
    Q_OBJECT
public:
    explicit HtmlTab(const QString &docFilename, QWidget *parent = nullptr)
        : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        auto *view = new McasterWebView(this);
        view->contextMenuCallback = [this](QPoint globalPos) {
            showContextMenu(globalPos);
        };
        m_view = view;
        layout->addWidget(m_view);

        load(docFilename);
    }

    void reload() { m_view->reload(); }

    // Called externally (e.g. from the macOS menu bar Help menu) to show the
    // same link list without requiring a right-click.
    void showContextMenu(const QPoint &globalPos)
    {
        QMenu menu(this);

        struct Link { const char *label; const char *url; };
        static const Link items[] = {
            { "Online Home",        "https://mcaster1.com/mcaster1_dnas.php"          },
            { "Online Help",        "https://mcaster1.com/mcaster1dnas/"              },
            { nullptr, nullptr },   // ── separator ──
            { "About Mcaster1",     "https://mcaster1.com/about.php"                  },
            { nullptr, nullptr },   // ── separator ──
            { "GitHub Repo",        "https://github.com/davestj/mcaster1dnas"         },
            { "Report Bug",         "https://github.com/davestj/mcaster1dnas/issues"  },
            { nullptr, nullptr },   // ── separator ──
            { "Mcaster1 Community", "https://mcaster1.com/community.php"              },
        };

        for (const auto &item : items) {
            if (!item.label) {
                menu.addSeparator();
                continue;
            }
            QAction *act = menu.addAction(item.label);
            const QString url = item.url;
            connect(act, &QAction::triggered, this, [url] {
                QDesktopServices::openUrl(QUrl(url));
            });
        }

        menu.exec(globalPos);
    }

private:
    void load(const QString &filename)
    {
        QString binaryDir = QCoreApplication::applicationDirPath();

        // 1. macOS app bundle: binary is at Contents/MacOS/; docs go into
        //    Contents/Resources/docs/ (codesign rejects non-code in MacOS/).
        QString path = binaryDir + "/../Resources/docs/" + filename;

        if (!QFile::exists(path)) {
            // 2. Flat binary / Linux: docs/ sibling of the binary.
            path = binaryDir + "/docs/" + filename;
        }

        if (!QFile::exists(path)) {
            // 3. Developer fallback: running from project dir (make gui + direct exec).
            path = QDir::currentPath() + "/docs/" + filename;
        }

        if (QFile::exists(path)) {
            m_view->setUrl(QUrl::fromLocalFile(QDir::cleanPath(path)));
        } else {
            m_view->setHtml(
                "<html><body style='background:#0f0f14;color:#e0e0e8;"
                "font-family:system-ui;padding:2rem'>"
                "<h2 style='color:#00c9ff'>Documentation not found</h2>"
                "<p>Could not locate <code style='color:#7dd3fc'>" +
                filename.toHtmlEscaped() +
                "</code>.</p>"
                "<p style='color:#888'>Searched alongside binary and in "
                "<code style='color:#7dd3fc'>Contents/Resources/docs/</code>.</p>"
                "</body></html>");
        }
    }

    QWebEngineView *m_view = nullptr;
};
