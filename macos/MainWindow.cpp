/*
 * MainWindow.cpp — Qt6 macOS/Linux GUI for Mcaster1DNAS.
 *
 * Architecture mirrors Windows CMcaster1WinDlg:
 *   - ServerThread → _beginthread(StartServer) with server_init+server_process
 *   - StatsPoller on QThread → _beginthread(StartStats) with stats_get_xml
 *   - 500 ms QTimer → Windows SetTimer(1, 500) polling loop
 *   - LogTab × 3 → Windows CLogTab instances (access, error, playlist)
 *
 * Phase 2 additions:
 *   - QToolBar with Start/Stop buttons, running/stopped status image, uptime
 *   - App icon set from embedded Qt resources (converted from android-chrome-512x512.png)
 *   - Logo label from mcaster1dnaslogo2.bmp (converted to PNG)
 *   - running.bmp / stopped.bmp status indicators (converted to PNG)
 */

#include "MainWindow.h"
#include "ConfigDialog.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QFrame>
#include <QToolBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QDesktopServices>
#include <QUrl>
#include <QSvgRenderer>
#include <QPainter>
#include <cmath>

extern "C" {
#include "server_bridge.h" // config.h + net/sock.h (defines IOVEC for connection.h)
#include "global.h"        // mc_global_t global, MC_RUNNING, MC_HALTING
#include "cfgfile.h"       // config_get_config_unlocked, mc_config_t
#include "logging.h"       // init_log_subsys(), log_shutdown() — needed before config_parse_file()
#include "log/log.h"       // log_shutdown()
}

// ── Header banner with overlaid status indicator ──────────────────────────────
// Draws header-banner-v2.png at its NATURAL pixel size (no stretching), then
// paints a rounded-rect status badge over the right side of the banner.
// The badge shows an animated broadcast tower (RUNNING) or a stop-sign (STOPPED).
class OverviewHeader : public QWidget
{
public:
    explicit OverviewHeader(QWidget *parent = nullptr) : QWidget(parent)
    {
        m_px = QPixmap(":/images/header-banner-v2.png");

        // Scale the banner to half its natural height (1926×142 → ~963×71) so
        // it sits as a compact header bar rather than consuming a large block.
        // The pre-scaled pixmap is then stretched to widget width in paintEvent.
        if (!m_px.isNull()) {
            m_px = m_px.scaledToHeight(m_px.height() / 2, Qt::SmoothTransformation);
            setFixedHeight(m_px.height());   // 71 px
        } else {
            setFixedHeight(48);
        }
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // Animation timer for running state (20 fps)
        m_timer = new QTimer(this);
        m_timer->setInterval(50);
        connect(m_timer, &QTimer::timeout, this, [this]() {
            m_phase += 0.10f;
            if (m_phase > 6.2832f) m_phase -= 6.2832f;
            update();
        });
    }

    void setRunning(bool r)
    {
        m_running = r;
        if (r) m_timer->start();
        else  { m_timer->stop(); update(); }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);

        const int W = width();
        const int H = height();

        // 1. Draw banner scaled to widget width (slight horizontal scale is
        //    acceptable for a wide banner; height is fixed at 71 px).
        if (!m_px.isNull())
            p.drawPixmap(0, 0, W, H, m_px);
        else {
            p.fillRect(0, 0, W, H, QColor("#1a3a6e"));
            p.setPen(QColor("#00c9ff"));
            QFont tf; tf.setBold(true); tf.setPointSize(16);
            p.setFont(tf);
            p.drawText(rect(), Qt::AlignCenter, "Mcaster1DNAS");
        }

        // 2. Status badge — 50% scale, centred vertically in 142 px banner
        const int bW  = 50;                    // badge width  (was 100)
        const int bH  = 61;                    // badge height (was 122)
        const int bY  = (H - bH) / 2;         // vertically centred in banner
        const int bX  = W - bW - 10;          // flush to right edge
        const int bcx = bX + bW / 2;          // badge horizontal centre

        // Semi-transparent dark backing
        p.setBrush(QColor(0, 0, 0, 175));
        p.setPen(QPen(QColor(255, 255, 255, 28), 1));
        p.drawRoundedRect(bX, bY, bW, bH, 5, 5);

        if (m_running) {
            // ── Broadcast tower + animated signal arcs ───────────────────────
            const int tipY  = bY + 13;     // antenna tip (was bY+26)
            const int baseY = bY + 39;     // tower base  (was bY+78)

            // 3 cascading arcs, outermost first (drawn behind tower)
            static const int   arcR[3]  = { 11, 8, 5 };          // was 22,15,9
            static const float arcLW[3] = { 0.8f, 1.0f, 1.2f };  // was 1.4,1.8,2.2
            for (int i = 0; i < 3; ++i) {
                float ph    = m_phase + i * 2.094f;
                float opac  = 0.15f + 0.85f * (0.5f + 0.5f * std::sin(ph));
                int   alpha = static_cast<int>(opac * 255);
                int   r     = arcR[i];

                p.setPen(QPen(QColor(0, 210, 80, alpha), arcLW[i],
                             Qt::SolidLine, Qt::RoundCap));
                p.setBrush(Qt::NoBrush);
                p.drawArc(QRectF(bcx - r, tipY - r, r * 2, r * 2),
                          30 * 16, 120 * 16);
            }

            // Tower mast
            QPen mast(QColor("#00dd55"), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            p.setPen(mast);
            p.drawLine(bcx, tipY + 1, bcx, baseY - 1);
            // Base legs
            p.drawLine(bcx, baseY - 1, bcx - 6, baseY + 4);
            p.drawLine(bcx, baseY - 1, bcx + 6, baseY + 4);
            // Cross brace
            p.drawLine(bcx - 4, baseY - 6, bcx + 4, baseY - 6);

            // Antenna tip pulse dot
            float gA = 0.5f + 0.5f * std::sin(m_phase * 2.0f);
            p.setBrush(QColor(0, 255, 100, static_cast<int>((0.55f + 0.45f * gA) * 255)));
            p.setPen(Qt::NoPen);
            p.drawEllipse(bcx - 2, tipY - 2, 4, 4);

            // "ON AIR" label
            p.setPen(QColor("#00ff66"));
            QFont lf; lf.setBold(true); lf.setPointSize(7);
            lf.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
            p.setFont(lf);
            p.drawText(QRect(bX, bY + bH - 10, bW, 9),
                       Qt::AlignHCenter | Qt::AlignVCenter, "ON AIR");

        } else {
            // ── Stop-sign octagon ─────────────────────────────────────────────
            const int R   = 12;                    // octagon outer radius (was 24)
            const int ocX = bcx;
            const int ocY = bY + bH / 2 - 4;      // vertically centred in badge

            QPolygonF oct, oct2;
            for (int i = 0; i < 8; ++i) {
                double a = M_PI / 8.0 + i * M_PI / 4.0;
                oct  << QPointF(ocX +  R    * std::cos(a), ocY +  R    * std::sin(a));
                oct2 << QPointF(ocX + (R-2) * std::cos(a), ocY + (R-2) * std::sin(a));
            }

            // Drop shadow
            p.setBrush(QColor(60, 0, 0, 90));
            p.setPen(Qt::NoPen);
            p.drawPolygon(oct.translated(1, 1));

            // Main octagon body
            p.setBrush(QColor("#aa0f0f"));
            p.setPen(QPen(QColor("#ff4444"), 1.0));
            p.drawPolygon(oct);

            // Inner white ring
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::white, 0.8));
            p.drawPolygon(oct2);

            // "STOP" text
            p.setPen(Qt::white);
            QFont sf; sf.setBold(true); sf.setPointSize(7);
            sf.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
            p.setFont(sf);
            p.drawText(QRect(bX, ocY - R, bW, R * 2), Qt::AlignCenter, "STOP");

            // "STOPPED" label
            p.setPen(QColor("#dd4444"));
            QFont of; of.setBold(true); of.setPointSize(6);
            of.setLetterSpacing(QFont::AbsoluteSpacing, 0.3);
            p.setFont(of);
            p.drawText(QRect(bX, bY + bH - 10, bW, 9),
                       Qt::AlignHCenter | Qt::AlignVCenter, "STOPPED");
        }
    }

private:
    QPixmap m_px;
    QTimer *m_timer   = nullptr;
    float   m_phase   = 0.0f;
    bool    m_running = false;
};


// ── Overview tab — header banner with overlay + stats below ───────────────────
class OverviewTab : public QWidget
{
public:
    explicit OverviewTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        // ── Header (banner + overlay indicator, natural image height) ───────────
        m_header = new OverviewHeader(this);

        // ── Stats labels ────────────────────────────────────────────────────────
        m_statusLbl    = new QLabel(QString::fromUtf8("● Offline"), this);
        m_sourcesLbl   = new QLabel("Sources:   0", this);
        m_clientsLbl   = new QLabel("Clients:   0", this);
        m_listenersLbl = new QLabel("Listeners: 0", this);
        m_uptimeLbl    = new QLabel("Uptime: --", this);
        m_configLbl    = new QLabel("Config: (none)", this);
        m_versionLbl   = new QLabel("Mcaster1DNAS  v2.5.3-beta", this);

        QFont bold = m_statusLbl->font();
        bold.setBold(true);
        bold.setPointSize(bold.pointSize() + 2);
        m_statusLbl->setFont(bold);
        m_statusLbl->setStyleSheet("color: #cc2222;");

        QFont mono = m_sourcesLbl->font();
        mono.setFamily("Menlo, Monaco, Courier New, monospace");
        for (QLabel *lbl : {m_sourcesLbl, m_clientsLbl, m_listenersLbl, m_uptimeLbl})
            lbl->setFont(mono);

        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);

        auto *statsCol = new QVBoxLayout;
        statsCol->setContentsMargins(10, 8, 10, 8);
        statsCol->setSpacing(4);
        statsCol->addWidget(m_versionLbl);
        statsCol->addSpacing(8);
        statsCol->addWidget(m_statusLbl);
        statsCol->addSpacing(4);
        statsCol->addWidget(m_sourcesLbl);
        statsCol->addWidget(m_clientsLbl);
        statsCol->addWidget(m_listenersLbl);
        statsCol->addWidget(m_uptimeLbl);
        statsCol->addSpacing(10);
        statsCol->addWidget(m_configLbl);
        statsCol->addStretch();

        auto *lay = new QVBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
        lay->addWidget(m_header);
        lay->addWidget(sep);
        lay->addLayout(statsCol, 1);
    }

    void setRunning(bool r)
    {
        m_header->setRunning(r);
        if (r) {
            m_statusLbl->setText(QString::fromUtf8("● Online  —  Broadcasting"));
            m_statusLbl->setStyleSheet("color: #22aa22;");
        } else {
            m_statusLbl->setText(QString::fromUtf8("● Offline"));
            m_statusLbl->setStyleSheet("color: #cc2222;");
        }
    }

    void setSources(int n)   { m_sourcesLbl->setText(  QString("Sources:   %1").arg(n)); }
    void setClients(int n)   { m_clientsLbl->setText(  QString("Clients:   %1").arg(n)); }
    void setListeners(int n) { m_listenersLbl->setText(QString("Listeners: %1").arg(n)); }
    void setUptime(const QString &s) { m_uptimeLbl->setText("Uptime:    " + s); }
    void setConfig(const QString &s) { m_configLbl->setText("Config:    " + s); }

private:
    OverviewHeader *m_header      = nullptr;
    QLabel         *m_statusLbl   = nullptr;
    QLabel         *m_sourcesLbl  = nullptr;
    QLabel         *m_clientsLbl  = nullptr;
    QLabel         *m_listenersLbl = nullptr;
    QLabel         *m_uptimeLbl   = nullptr;
    QLabel         *m_configLbl   = nullptr;
    QLabel         *m_versionLbl  = nullptr;
};


// ══════════════════════════════════════════════════════════════════════════════
// SVG icon factory — broadcaster-themed toolbar icons
// Renders an SVG string into a QIcon at the requested pixel size.
// No external files needed; shapes are defined inline.
// ══════════════════════════════════════════════════════════════════════════════

static QIcon svgIcon(const char *svg, int sz = 22)
{
    QByteArray   data(svg);
    QSvgRenderer renderer(data);
    QPixmap      px(sz, sz);
    px.fill(Qt::transparent);
    QPainter p(&px);
    renderer.render(&p);
    return QIcon(px);
}

// broadcaster-themed SVG icon strings
// ── Start Server: green rounded rect + play triangle ─────────────────────────
static constexpr const char *kSvgStart = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <rect width="24" height="24" rx="5" fill="#196619"/>
  <polygon points="8,5 8,19 21,12" fill="white"/>
</svg>)svg";

// ── Stop Server: red rounded rect + stop square ───────────────────────────────
static constexpr const char *kSvgStop = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <rect width="24" height="24" rx="5" fill="#881818"/>
  <rect x="6" y="6" width="12" height="12" rx="2" fill="white"/>
</svg>)svg";

// ── Config: blue rounded rect + 8-tooth gear ──────────────────────────────────
static constexpr const char *kSvgConfig = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <rect width="24" height="24" rx="5" fill="#1a3a6a"/>
  <g fill="white" transform="translate(12,12)">
    <circle r="5.5"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(45)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(90)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(135)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(180)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(225)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(270)"/>
    <rect x="-1.5" y="-10" width="3" height="3.5" rx="0.8" transform="rotate(315)"/>
    <circle r="2.5" fill="#1a3a6a"/>
  </g>
</svg>)svg";

// ── Web UI: teal rounded rect + globe (circle + meridian + parallels) ─────────
static constexpr const char *kSvgWebUI = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <rect width="24" height="24" rx="5" fill="#1a4a5a"/>
  <g stroke="white" fill="none" transform="translate(12,12)">
    <circle r="7" stroke-width="1.3"/>
    <ellipse rx="3.5" ry="7" stroke-width="1"/>
    <line x1="-7" y1="0" x2="7" y2="0" stroke-width="1"/>
    <line x1="-6" y1="-3.5" x2="6" y2="-3.5" stroke-width="0.8"/>
    <line x1="-6" y1="3.5" x2="6" y2="3.5" stroke-width="0.8"/>
  </g>
</svg>)svg";

// ── Admin: purple rounded rect + broadcast tower + signal arcs ────────────────
static constexpr const char *kSvgAdmin = R"svg(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24">
  <rect width="24" height="24" rx="5" fill="#3a1a5a"/>
  <g stroke="white" fill="none" stroke-linecap="round">
    <line x1="12" y1="5" x2="12" y2="17" stroke-width="2"/>
    <line x1="9"  y1="17" x2="15" y2="17" stroke-width="2"/>
    <path d="M8.5,9.5 Q12,7.5 15.5,9.5"  stroke-width="1.4"/>
    <path d="M5.5,6.5 Q12,3 18.5,6.5"    stroke-width="1.4"/>
  </g>
</svg>)svg";

// ══════════════════════════════════════════════════════════════════════════════
// MainWindow implementation
// ══════════════════════════════════════════════════════════════════════════════

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Mcaster1DNAS");
    resize(960, 680);

    // ── App icon (compiled-in resource — converted from android-chrome-512px) ──
    QIcon appIcon(":/icons/mcaster1.png");
    if (!appIcon.isNull()) {
        appIcon.addPixmap(QPixmap(":/icons/mcaster1_32.png"));
        QApplication::setWindowIcon(appIcon);
        setWindowIcon(appIcon);
    }

    // ── Cache running/stopped pixmaps for the toolbar indicator ───────────────
    m_pxRunning = QPixmap(":/images/running.png");
    m_pxStopped = QPixmap(":/images/stopped.png");

    // ── Config discovery (mirrors src/main.c _parse_config_opts) ──────────────
    m_configPath = discoverConfig(argc, argv);

    // ── Tabs ───────────────────────────────────────────────────────────────────
    m_tabs = new QTabWidget(this);

    m_overview    = new OverviewTab(this);
    m_statsTab    = new StatsTab(this);
    m_logAccess   = new LogTab(LogTabKind::Access,   this);
    m_logError    = new LogTab(LogTabKind::Error,    this);
    m_logPlaylist = new LogTab(LogTabKind::Playlist, this);
    m_configTab   = new ConfigTab(this);
    m_consoleTab  = new ConsoleTab(this);
    m_creditsTab  = new HtmlTab("CREDITS.html", this);
    m_helpTab     = new HtmlTab("index.html",   this);

    m_tabs->addTab(m_overview,    "Overview");
    m_tabs->addTab(m_statsTab,    "Stats");
    m_tabs->addTab(m_logAccess,   "Access Log");
    m_tabs->addTab(m_logError,    "Error Log");
    m_tabs->addTab(m_logPlaylist, "Playlist Log");
    m_tabs->addTab(m_configTab,   "Config");
    m_tabs->addTab(m_consoleTab,  "Console");
    m_tabs->addTab(m_creditsTab,  "Credits");
    m_tabs->addTab(m_helpTab,     "Help");

    // Right-click on the Help tab label → show the same link context menu.
    // tabBar()->tabAt(pos) identifies which tab was clicked; we only act on
    // the Help tab so other tab labels keep their default (no) context menu.
    m_tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabs->tabBar(), &QTabBar::customContextMenuRequested,
            this, [this](const QPoint &pos) {
        if (m_tabs->tabBar()->tabAt(pos) == m_tabs->indexOf(m_helpTab))
            m_helpTab->showContextMenu(m_tabs->tabBar()->mapToGlobal(pos));
    });

    setCentralWidget(m_tabs);

    // ── Toolbar (Phase 2: Start/Stop buttons + status indicator + uptime) ──────
    m_toolbar = addToolBar("Server");
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(20, 20));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Logo in toolbar
    QLabel *tbLogo = new QLabel(this);
    QPixmap logoPx(":/images/logo.png");
    if (!logoPx.isNull())
        tbLogo->setPixmap(logoPx.scaled(140, 28, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
    else
        tbLogo->setText("Mcaster1DNAS");
    tbLogo->setContentsMargins(4, 0, 8, 0);
    m_toolbar->addWidget(tbLogo);

    // Separator
    m_toolbar->addSeparator();

    // ── Shared button base style (broadcaster dark theme) ─────────────────────
    // Each button overrides the background/border colour; hover/press/disabled
    // states are shared.  Icons are rendered from inline SVG via QSvgRenderer.
    auto applyBtnStyle = [](QPushButton *btn,
                            const char *svgStr,
                            const QString &gradTop,
                            const QString &gradBot,
                            const QString &border,
                            const QString &hoverTop,
                            const QString &hoverBot,
                            const QString &hoverBorder) {
        btn->setIcon(svgIcon(svgStr, 20));
        btn->setIconSize(QSize(20, 20));
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "    stop:0 %1, stop:1 %2);"
            "  color: white;"
            "  border: 1px solid %3;"
            "  border-radius: 5px;"
            "  padding: 5px 13px 5px 9px;"
            "  font-weight: bold;"
            "  font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "    stop:0 %4, stop:1 %5);"
            "  border-color: %6;"
            "}"
            "QPushButton:pressed {"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "    stop:0 %2, stop:1 %1);"
            "}"
            "QPushButton:disabled {"
            "  background: #252525;"
            "  color: #444;"
            "  border-color: #333;"
            "}"
        ).arg(gradTop, gradBot, border, hoverTop, hoverBot, hoverBorder));
    };

    // Config button ─ steel blue
    m_configBtn = new QPushButton("Config", this);
    applyBtnStyle(m_configBtn, kSvgConfig,
                  "#2a4878", "#162840", "#3a6aaa",
                  "#3a5a9a", "#1e3a60", "#55aaff");
    m_configBtn->setMinimumWidth(88);
    m_configBtn->setToolTip("Open Configuration Manager");
    m_toolbar->addWidget(m_configBtn);
    connect(m_configBtn, &QPushButton::clicked, this, &MainWindow::onConfigManager);

    m_toolbar->addSeparator();

    // Start button ─ broadcast green
    m_startBtn = new QPushButton("Start Server", this);
    applyBtnStyle(m_startBtn, kSvgStart,
                  "#226622", "#133313", "#33aa33",
                  "#2e8a2e", "#1a4a1a", "#55dd55");
    m_startBtn->setMinimumWidth(130);
    m_toolbar->addWidget(m_startBtn);

    // Stop button ─ signal red
    m_stopBtn = new QPushButton("Stop Server", this);
    applyBtnStyle(m_stopBtn, kSvgStop,
                  "#882020", "#4a1010", "#bb3333",
                  "#aa2828", "#6a1818", "#ff5555");
    m_stopBtn->setMinimumWidth(118);
    m_stopBtn->setEnabled(false);
    m_toolbar->addWidget(m_stopBtn);

    m_toolbar->addSeparator();

    // Web UI button ─ deep teal
    m_webFrontendBtn = new QPushButton("Web UI", this);
    applyBtnStyle(m_webFrontendBtn, kSvgWebUI,
                  "#1a4a5a", "#0d2830", "#2a7a8a",
                  "#226070", "#143845", "#44bbcc");
    m_webFrontendBtn->setMinimumWidth(84);
    m_webFrontendBtn->setToolTip("Open web status page in browser");
    m_toolbar->addWidget(m_webFrontendBtn);
    connect(m_webFrontendBtn, &QPushButton::clicked, this, &MainWindow::onOpenWebFrontend);

    // Admin button ─ broadcast purple
    m_webAdminBtn = new QPushButton("Admin", this);
    applyBtnStyle(m_webAdminBtn, kSvgAdmin,
                  "#3a1a5a", "#1e0d30", "#6a3aaa",
                  "#4e2278", "#281044", "#9955dd");
    m_webAdminBtn->setMinimumWidth(78);
    m_webAdminBtn->setToolTip("Open web admin area in browser");
    m_toolbar->addWidget(m_webAdminBtn);
    connect(m_webAdminBtn, &QPushButton::clicked, this, &MainWindow::onOpenWebAdmin);

    m_toolbar->addSeparator();

    // Status indicator — text-based, blinks green "◉ ONLINE" when server runs
    m_toolbarStatus = new QLabel(this);
    m_toolbarStatus->setText("◉  OFFLINE");
    m_toolbarStatus->setStyleSheet(
        "color: #444444; font-weight: bold; font-size: 13px;"
        " padding: 0 6px; letter-spacing: 1px;");
    m_toolbarStatus->setMinimumWidth(110);
    m_toolbarStatus->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_toolbar->addWidget(m_toolbarStatus);

    // Blink timer — fires every 600 ms when server is running
    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(600);
    connect(m_blinkTimer, &QTimer::timeout, this, &MainWindow::onBlinkTimer);

    m_toolbar->addSeparator();

    // Uptime in toolbar (right-aligned via stretch)
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(spacer);

    m_toolbarUptime = new QLabel("", this);
    m_toolbarUptime->setContentsMargins(0, 0, 8, 0);
    m_toolbarUptime->setStyleSheet("color: #aaaaaa; font-size: 11px;");
    m_toolbar->addWidget(m_toolbarUptime);

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(m_stopBtn,  &QPushButton::clicked, this, &MainWindow::onStopServer);

    // ── Menu bar ───────────────────────────────────────────────────────────────
    QMenu *serverMenu = menuBar()->addMenu("Server");
    QAction *startAct = serverMenu->addAction(QString::fromUtf8("\u25B6  Start Server"));
    QAction *stopAct  = serverMenu->addAction(QString::fromUtf8("\u25FC  Stop Server"));
    serverMenu->addSeparator();
    QAction *webFrontAct = serverMenu->addAction(
        QString::fromUtf8("\U0001F310  Open Web Frontend"), this, &MainWindow::onOpenWebFrontend);
    QAction *webAdminAct = serverMenu->addAction(
        QString::fromUtf8("\U0001F527  Open Web Admin"),   this, &MainWindow::onOpenWebAdmin);
    serverMenu->addSeparator();
    serverMenu->addAction("Quit", this, &MainWindow::onQuit);

    stopAct->setEnabled(false);
    // Web items usable any time a config path is known
    webFrontAct->setEnabled(!m_configPath.isEmpty());
    webAdminAct->setEnabled(!m_configPath.isEmpty());

    connect(startAct, &QAction::triggered, this, &MainWindow::onStartServer);
    connect(stopAct,  &QAction::triggered, this, &MainWindow::onStopServer);

    // Keep menu actions in sync with toolbar buttons
    connect(m_startBtn, &QPushButton::clicked, stopAct,  [stopAct]{ stopAct->setEnabled(true); });
    connect(m_stopBtn,  &QPushButton::clicked, startAct, [startAct]{ startAct->setEnabled(true); });

    // Enable web actions whenever a config path is set (they resolve URL from config/live state)
    connect(m_startBtn, &QPushButton::clicked, webFrontAct, [webFrontAct]{ webFrontAct->setEnabled(true); });
    connect(m_startBtn, &QPushButton::clicked, webAdminAct, [webAdminAct]{ webAdminAct->setEnabled(true); });
    (void)webFrontAct; (void)webAdminAct;   // captured by lambdas above

    QMenu *editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction("Edit Config in External Editor", m_configTab, &ConfigTab::openInEditor);
    editMenu->addSeparator();
    editMenu->addAction("Configuration Manager…", this, &MainWindow::onConfigManager);

    QMenu *toolsMenu = menuBar()->addMenu("Tools");
    m_configAction = toolsMenu->addAction("Configuration Manager…",
                                          this, &MainWindow::onConfigManager);

    // ── Help menu (macOS system menu bar) ──────────────────────────────────────
    // On macOS, Qt places a menu named "Help" in the standard position at the
    // right side of the menu bar.  Each action opens a URL in the default browser.
    struct HelpLink { const char *label; const char *url; };
    static const HelpLink helpLinks[] = {
        { "Online Home",        "https://mcaster1.com/mcaster1_dnas.php"          },
        { "Online Help",        "https://mcaster1.com/mcaster1dnas/"              },
        { nullptr, nullptr },
        { "About Mcaster1",     "https://mcaster1.com/about.php"                  },
        { nullptr, nullptr },
        { "GitHub Repo",        "https://github.com/davestj/mcaster1dnas"         },
        { "Report Bug",         "https://github.com/davestj/mcaster1dnas/issues"  },
        { nullptr, nullptr },
        { "Mcaster1 Community", "https://mcaster1.com/community.php"              },
    };
    QMenu *helpMenu = menuBar()->addMenu("Help");
    for (const auto &lnk : helpLinks) {
        if (!lnk.label) { helpMenu->addSeparator(); continue; }
        QAction *act = helpMenu->addAction(lnk.label);
        const QString url = lnk.url;
        connect(act, &QAction::triggered, this, [url] {
            QDesktopServices::openUrl(QUrl(url));
        });
    }

    // ── Status bar ─────────────────────────────────────────────────────────────
    m_statusLabel = new QLabel("Stopped", this);
    m_uptimeLabel = new QLabel("", this);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_uptimeLabel);

    // ── Server thread ──────────────────────────────────────────────────────────
    m_server = new ServerThread(this);
    connect(m_server, &ServerThread::serverStarted, this, &MainWindow::onServerStarted);
    connect(m_server, &ServerThread::serverStopped, this, &MainWindow::onServerStopped);
    connect(m_server, &ServerThread::serverError,   this, &MainWindow::onServerError);

    // ── Stats poller (runs on dedicated QThread) ───────────────────────────────
    m_statsThread = new QThread(this);
    m_statsPoller = new StatsPoller;
    m_statsPoller->moveToThread(m_statsThread);

    connect(m_statsThread, &QThread::started,  m_statsPoller, &StatsPoller::startPolling);
    connect(m_statsThread, &QThread::finished, m_statsPoller, &QObject::deleteLater);
    connect(m_statsPoller, &StatsPoller::statsUpdated, this, &MainWindow::onStatsUpdated);

    // ── 500 ms UI poll timer (mirrors Windows SetTimer(1, 500)) ───────────────
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(500);
    connect(m_pollTimer, &QTimer::timeout, this, &MainWindow::onPollTimer);

    // ── Tray icon ──────────────────────────────────────────────────────────────
    m_tray = new TrayIcon(this);
    connect(m_tray, &TrayIcon::startRequested,      this, &MainWindow::onStartServer);
    connect(m_tray, &TrayIcon::stopRequested,        this, &MainWindow::onStopServer);
    connect(m_tray, &TrayIcon::showWindowRequested,  this, &MainWindow::onShowWindow);
    connect(m_tray, &TrayIcon::quitRequested,        this, &MainWindow::onQuit);

    // ── Load config into editor ────────────────────────────────────────────────
    m_configTab->setConfigPath(m_configPath);
    m_overview->setConfig(m_configPath);
    connect(m_configTab, &ConfigTab::configSaved, this, &MainWindow::onConfigSaved);

    // ── Auto-start polling timer ───────────────────────────────────────────────
    m_pollTimer->start();
}

MainWindow::~MainWindow()
{
    m_server->stop();
    if (m_statsThread->isRunning()) {
        m_statsPoller->stopPolling();
        m_statsThread->quit();
        m_statsThread->wait(3000);
    }
}

// ── Config discovery ──────────────────────────────────────────────────────────
// Search order:
//   1. -c <path> command-line argument
//   2. Directories near the binary (inside or alongside .app bundle)
//   3. Current working directory (rarely useful on macOS — 'open' sets CWD to /)
//   4. QFileDialog fallback
//
// macOS .app layout:  Mcaster1DNAS.app/Contents/MacOS/Mcaster1DNAS
//   exedir  = .../build-qt/Mcaster1DNAS.app/Contents/MacOS
//   +3 up   = .../build-qt              (sibling of .app)
//   +4 up   = .../macos                 (GUI source dir)
//   +5 up   = .../mcaster1dnas          (repo root — where mcaster1dnas.yaml lives)
QString MainWindow::discoverConfig(int argc, char *argv[]) const
{
    // 1. -c <path> argument
    for (int i = 1; i < argc - 1; ++i) {
        if (QString(argv[i]) == "-c") {
            QString p = QString::fromLocal8Bit(argv[i + 1]);
            if (QFileInfo::exists(p))
                return p;
        }
    }

    // 2. Build candidate search directories
    QString exedir = QFileInfo(QApplication::applicationFilePath()).absolutePath();

    QStringList searchDirs;
    searchDirs << exedir;
    searchDirs << QDir::currentPath();

    // When running inside a macOS .app bundle, walk up to find the repo root.
    if (exedir.contains(".app/Contents/MacOS", Qt::CaseInsensitive) ||
        exedir.contains(".app/Contents/MacOS/", Qt::CaseInsensitive))
    {
        QDir d(exedir);
        for (int levels = 0; levels < 6; ++levels) {
            if (!d.cdUp()) break;
            searchDirs << d.absolutePath();
        }
    } else {
        // Not inside a bundle — just walk up a few levels from the binary
        QDir d(exedir);
        for (int levels = 0; levels < 4; ++levels) {
            if (!d.cdUp()) break;
            searchDirs << d.absolutePath();
        }
    }

    // Config filenames to try, in priority order
    static const QStringList names = {
        "mcaster1dnas.yaml",
        "mcaster1dnas-macos.yaml",
        "mcaster1dnas-console.yaml",
        "mcaster1dnas.xml",
    };

    // Deduplicate while preserving order
    QStringList seen;
    for (const QString &dir : searchDirs) {
        if (seen.contains(dir)) continue;
        seen << dir;
        for (const QString &name : names) {
            QString path = dir + "/" + name;
            if (QFileInfo::exists(path))
                return path;
        }
    }

    // 3. Ask user
    return QFileDialog::getOpenFileName(
        nullptr, "Select Mcaster1DNAS Config File", QDir::homePath(),
        "Config Files (*.yaml *.yml *.xml);;All Files (*)");
}

// ── Helper: update all running/stopped UI elements atomically ─────────────────
void MainWindow::setServerRunningUI(bool running)
{
    m_startBtn->setEnabled(!running);
    m_stopBtn->setEnabled(running);

    // Disable config editing while the server is live — changes require a restart
    // and editing running config via the C API is not thread-safe.
    m_configBtn->setEnabled(!running);
    if (m_configAction)
        m_configAction->setEnabled(!running);

    // Drive the ONLINE/OFFLINE blink indicator
    if (running) {
        m_blinkState = true;
        m_blinkTimer->start();
        onBlinkTimer();   // set bright state immediately (no lag on first tick)
    } else {
        m_blinkTimer->stop();
        m_toolbarStatus->setText("◉  OFFLINE");
        m_toolbarStatus->setStyleSheet(
            "color: #444444; font-weight: bold; font-size: 13px;"
            " padding: 0 6px; letter-spacing: 1px;");
    }

    m_overview->setRunning(running);
    m_tray->setServerRunning(running);

    if (running) {
        setWindowTitle("Mcaster1DNAS — Online");
        m_statusLabel->setText("Online");
    } else {
        setWindowTitle("Mcaster1DNAS — Stopped");
        m_statusLabel->setText("Stopped");
        m_uptimeLabel->setText("");
        m_toolbarUptime->setText("");
    }
}

// ── ONLINE indicator blink ────────────────────────────────────────────────────
void MainWindow::onBlinkTimer()
{
    m_blinkState = !m_blinkState;
    if (m_blinkState) {
        m_toolbarStatus->setText("◉  ONLINE");
        m_toolbarStatus->setStyleSheet(
            "color: #00ff55; font-weight: bold; font-size: 13px;"
            " padding: 0 6px; letter-spacing: 1px;");
    } else {
        m_toolbarStatus->setText("◉  ONLINE");
        m_toolbarStatus->setStyleSheet(
            "color: #005522; font-weight: bold; font-size: 13px;"
            " padding: 0 6px; letter-spacing: 1px;");
    }
}

// ── Server lifecycle slots ────────────────────────────────────────────────────
void MainWindow::onStartServer()
{
    if (m_configPath.isEmpty()) {
        QMessageBox::warning(this, "No Config", "No configuration file selected.");
        return;
    }

    // ── Capture stdout so the startup banner appears in the Console tab ────────
    // startCapture() redirects STDOUT_FILENO into a pipe *before* server_init()
    // writes its banner.  ServerThread::threadFunc() then calls server_init()
    // which printf()-s the banner — it ends up in the Console tab.
    m_consoleTab->startCapture();
    m_consoleTab->appendInfo("Starting server with config: " + m_configPath);
    // Switch to Console tab so the banner is visible during startup
    m_tabs->setCurrentWidget(m_consoleTab);

    // ── Start log tailing ──────────────────────────────────────────────────────
    // Use the config file's directory + "/logs" as the initial log path.
    // ServerThread::threadFunc() calls chdir(configDir) before server_init(),
    // so the server will write logs to <configDir>/logs/ — same path we use here.
    QString configDir = QFileInfo(m_configPath).absolutePath();
    QString logDir    = configDir + "/logs";
    m_logAccess->setLogPath(logDir);
    m_logError->setLogPath(logDir);
    m_logPlaylist->setLogPath(logDir);
    m_logAccess->startTailing();
    m_logError->startTailing();
    m_logPlaylist->startTailing();

    m_server->start(m_configPath.toStdString());
}

void MainWindow::onStopServer()
{
    m_server->stop();
    m_statsPoller->stopPolling();
    m_logAccess->stopTailing();
    m_logError->stopTailing();
    m_logPlaylist->stopTailing();
    m_consoleTab->stopCapture();
    m_consoleTab->appendInfo("Server stopped.");
}

void MainWindow::onServerStarted()
{
    m_startTime = QDateTime::currentDateTime();
    m_subsystemsAlive = true;   // initialize_subsystems() has run — logging rwlock is live
    setServerRunningUI(true);

    // Now that the server has initialised, get the actual log dir from config
    if (global.running == MC_RUNNING) {
        mc_config_t *cfg = config_get_config_unlocked();
        if (cfg && cfg->log_dir) {
            QString logDir = QString::fromLocal8Bit(cfg->log_dir);
            m_logAccess->setLogPath(logDir);
            m_logError->setLogPath(logDir);
            m_logPlaylist->setLogPath(logDir);
        }
    }

    if (!m_statsThread->isRunning())
        m_statsThread->start();
    else
        m_statsPoller->startPolling();
}

void MainWindow::onServerStopped()
{
    m_subsystemsAlive = false;  // shutdown_subsystems() has run — logging rwlock is gone
    setServerRunningUI(false);
}

void MainWindow::onServerError(const QString &msg)
{
    QMessageBox::critical(this, "Server Error", msg);
    onServerStopped();
}

// ── 500 ms UI poll timer ──────────────────────────────────────────────────────
void MainWindow::onPollTimer()
{
    bool running = (global.running == MC_RUNNING);
    m_overview->setRunning(running);
    m_overview->setSources(global.sources);
    m_overview->setClients(global.clients);
    m_overview->setListeners(global.listeners);
    updateStatusBar(running, global.sources, global.clients, global.listeners);
    if (running)
        updateUptime();
}

void MainWindow::updateStatusBar(bool running, int sources, int clients, int listeners)
{
    if (running)
        m_statusLabel->setText(
            QString("Running — Sources: %1  Clients: %2  Listeners: %3")
                .arg(sources).arg(clients).arg(listeners));
    else
        m_statusLabel->setText("Stopped");
}

void MainWindow::updateUptime()
{
    if (!m_startTime.isValid()) return;
    qint64 secs = m_startTime.secsTo(QDateTime::currentDateTime());
    qint64 d = secs / 86400;
    qint64 h = (secs % 86400) / 3600;
    qint64 m = (secs % 3600) / 60;
    qint64 s = secs % 60;
    QString up;
    if (d > 0) up = QString("%1d %2h %3m %4s").arg(d).arg(h).arg(m).arg(s);
    else if (h > 0) up = QString("%1h %2m %3s").arg(h).arg(m).arg(s);
    else up = QString("%1m %2s").arg(m).arg(s);
    m_uptimeLabel->setText("Uptime: " + up);
    m_toolbarUptime->setText("Up: " + up);
    m_overview->setUptime(up);
}

// ── Stats updated (from StatsPoller) ─────────────────────────────────────────
void MainWindow::onStatsUpdated(GlobalStats gs, QList<SourceStats> sources)
{
    m_overview->setSources(gs.sources);
    m_overview->setClients(gs.clients);
    m_overview->setListeners(gs.listeners);
    m_statsTab->onStatsUpdated(gs, sources);
}

// ── Misc slots ─────────────────────────────────────────────────────────────────
void MainWindow::onShowWindow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::onQuit()
{
    onStopServer();
    QApplication::quit();
}

void MainWindow::onConfigSaved(const QString &path)
{
    statusBar()->showMessage("Config saved: " + path, 3000);
}

void MainWindow::onConfigManager()
{
    // Guard: button/menu should be disabled when running, but protect defensively.
    if (m_subsystemsAlive) {
        statusBar()->showMessage("Stop the server before editing the configuration.", 4000);
        return;
    }

    // m_subsystemsAlive is false: server is stopped or was never started.
    // ConfigDialog will parse the config file directly (loadFromFile path) —
    // safe because logging subsystem is either uninitialized (never started)
    // or the config file is the authoritative source the user wants to edit.
    ConfigDialog dlg(m_configPath, false, this);
    connect(&dlg, &ConfigDialog::savedConfig, this, [this](const QString &path) {
        if (path != m_configPath) {
            // Config saved to a NEW file — ask the user to activate it.
            auto ans = QMessageBox::question(this, "Reload Config",
                QString("Config saved to:\n%1\n\n"
                        "Reload this as the active config?\n"
                        "The server will use it when you click \342\226\266 Start.")
                    .arg(path),
                QMessageBox::Yes | QMessageBox::No);
            if (ans == QMessageBox::Yes) {
                m_configPath = path;
                m_configTab->setConfigPath(path);
                m_overview->setConfig(path);
                statusBar()->showMessage("Active config reloaded: " + path, 5000);
            }
        } else {
            // Config saved to the SAME file — confirm it's staged for next start.
            QMessageBox::information(this, "Config Saved",
                QString("Config saved to:\n%1\n\n"
                        "Click \342\226\266 Start to launch the server with the updated settings.")
                    .arg(path));
            statusBar()->showMessage("Config updated — ready to start server.", 5000);
        }
    });
    dlg.exec();
}

// ── URL discovery ─────────────────────────────────────────────────────────────
// Priority: SSL listener (https) > plain HTTP listener.
// If bind_address is a wildcard/unspecified, substitute 127.0.0.1 for browser.
// Works from live config (server running) OR by parsing the config file
// (server stopped) — same safe pattern as ConfigDialog::loadFromFile().
QString MainWindow::discoverServerUrl(bool adminPath) const
{
    int   httpPort  = -1;
    int   httpsPort = -1;
    QString httpHost, httpsHost;

    // Helper: normalise a bind address to something a browser can reach
    auto normaliseHost = [](const char *raw) -> QString {
        if (!raw) return "127.0.0.1";
        QString a = QString::fromLocal8Bit(raw);
        if (a.isEmpty() || a == "0.0.0.0" || a == "::" || a == "*")
            return "127.0.0.1";
        return a;
    };

    auto scanListeners = [&](const _listener_t *head) {
        for (const _listener_t *l = head; l; l = l->next) {
            // ssl==1  → explicitly forced SSL
            // ssl==-1 → auto-detect; treat ports ending in 443 (e.g. 9443, 8443, 443)
            //           as the HTTPS endpoint — matches the universal convention
            bool looksLikeSSL = (l->ssl == 1) ||
                                 (l->ssl == -1 && (l->port % 1000 == 443));
            if (looksLikeSSL && httpsPort < 0) {
                httpsPort = l->port;
                httpsHost = normaliseHost(l->bind_address);
            } else if (!looksLikeSSL && httpPort < 0) {
                httpPort = l->port;
                httpHost = normaliseHost(l->bind_address);
            }
        }
    };

    if (global.running == MC_RUNNING) {
        // Server is up — read from live in-process config (no parse needed)
        const mc_config_t *cfg = config_get_config_unlocked();
        if (cfg) scanListeners(cfg->listen_sock);
    } else if (!m_configPath.isEmpty()) {
        // Server stopped — parse the file into a local struct.
        // IMPORTANT: do NOT call config_clear() — see loadFromFile() note.
        // init_log_subsys() ensures the logging rwlock is created before
        // config_parse_file() calls log_write() internally.  log_shutdown()
        // resets _initialized=0 so initialize_subsystems() can properly
        // reinitialize logging when the server actually starts later.
        mc_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        config_init_configuration(&cfg);
        init_log_subsys();
        int parseRet = config_parse_file(m_configPath.toLocal8Bit().constData(), &cfg);
        log_shutdown();
        if (parseRet == 0)
            scanListeners(cfg.listen_sock);
    }

    // Prefer HTTPS; fall back to HTTP; fall back to well-known default
    QString scheme, host, portStr;
    if (httpsPort > 0) {
        scheme  = "https";
        host    = httpsHost;
        portStr = QString::number(httpsPort);
    } else if (httpPort > 0) {
        scheme  = "http";
        host    = httpHost;
        portStr = QString::number(httpPort);
    } else {
        // No listeners parsed — use conventional default
        scheme  = "http";
        host    = "127.0.0.1";
        portStr = "9330";
    }

    QString path = adminPath ? "/admin/stats.xsl" : "/status.xsl";
    return QString("%1://%2:%3%4").arg(scheme, host, portStr, path);
}

void MainWindow::onOpenWebFrontend()
{
    QString url = discoverServerUrl(false);
    m_statusLabel->setText("Opening: " + url);
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::onOpenWebAdmin()
{
    QString url = discoverServerUrl(true);
    m_statusLabel->setText("Opening: " + url);
    QDesktopServices::openUrl(QUrl(url));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // On macOS, close hides the window (server keeps running in tray/menu bar).
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        hide();
        event->ignore();
    } else {
        onQuit();
        event->accept();
    }
}
