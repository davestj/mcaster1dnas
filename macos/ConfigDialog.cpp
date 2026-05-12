/*
 * ConfigDialog.cpp — Phase 3: Complete Configuration Manager.
 *
 * Config loading strategy:
 *   Server RUNNING  → config_get_config_unlocked() (live in-process struct).
 *   Server STOPPED  → config_parse_file() with a local mc_config_t.
 *                      Confirmed safe: no global locks needed, config_parse_file
 *                      operates only on the provided struct pointer.
 *
 * Relay traversal:  relay = relay->new_details  (NOT ->next — dual-purpose field)
 *
 * Mount storage layout (IMPORTANT):
 *   config_mount_template(name) returns 1 if name contains wildcard chars (*?[+!@$).
 *   - Wildcard template mounts  (e.g. /live*)  → config->mounts  linked list (->next)
 *   - Named static mounts       (e.g. /live)   → config->mounts_tree  AVL tree
 *   populateMounts()     traverses the linked list (wildcard mounts).
 *   populateMountsTree() traverses the AVL tree    (static named mounts).
 *   Both are called from populate() so ALL mounts appear in the UI.
 *
 * kv_pair traversal: kv  = kv->next
 */

#include "ConfigDialog.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QSplitter>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QAction>

#include <cstring>   // memset

extern "C" {
#include "server_bridge.h"  // config.h + net/sock.h
#include "global.h"
#include "cfgfile.h"
#include "avl/avl.h"        // avl_get_first(), avl_get_next() for mounts_tree traversal
#include "ssl_gen.h"        // ssl_gen_run(), ssl_gen_params_t, ssl_gen_last_error()
#include "logging.h"        // init_log_subsys() — ensures rwlock exists before config_parse_file()
#include "log/log.h"        // log_shutdown() — resets _initialized so server can re-init later

// OpenSSL for cert info display on the SSL Cert tab
#ifdef HAVE_OPENSSL
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/bio.h>
#endif
}

// ── Shared helpers ────────────────────────────────────────────────────────────

static QString qs(const char *s) { return s ? QString::fromLocal8Bit(s) : QString(); }

static QWidget *scrolled(QWidget *w)
{
    auto *sa = new QScrollArea;
    sa->setWidget(w);
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    return sa;
}

// Build a "Browse for file" row: QLineEdit + Browse button in an hbox.
static QLineEdit *fileRow(QFormLayout *fl, const QString &label,
                           const QString &def,
                           const QString &filter = "All Files (*)")
{
    auto *le  = new QLineEdit(def);
    le->setMinimumWidth(260);
    auto *btn = new QPushButton("Browse…");
    btn->setFixedWidth(72);
    QObject::connect(btn, &QPushButton::clicked, le, [le, filter] {
        QString p = QFileDialog::getOpenFileName(nullptr, "Select File",
                        QFileInfo(le->text()).absolutePath(), filter);
        if (!p.isEmpty()) le->setText(p);
    });
    auto *row = new QHBoxLayout;
    row->addWidget(le, 1);
    row->addWidget(btn);
    fl->addRow(label, row);
    return le;
}

// Build a "Browse for directory" row.
static QLineEdit *dirRow(QFormLayout *fl, const QString &label, const QString &def)
{
    auto *le  = new QLineEdit(def);
    le->setMinimumWidth(260);
    auto *btn = new QPushButton("Browse…");
    btn->setFixedWidth(72);
    QObject::connect(btn, &QPushButton::clicked, le, [le] {
        QString p = QFileDialog::getExistingDirectory(nullptr, "Select Directory",
                        le->text());
        if (!p.isEmpty()) le->setText(p);
    });
    auto *row = new QHBoxLayout;
    row->addWidget(le, 1);
    row->addWidget(btn);
    fl->addRow(label, row);
    return le;
}

static QSpinBox *mkSpin(int lo, int hi, int val, const QString &sfx = {})
{
    auto *sb = new QSpinBox;
    sb->setRange(lo, hi);
    sb->setValue(val);
    if (!sfx.isEmpty()) sb->setSuffix(sfx);
    return sb;
}

// ── Toolbar helper ────────────────────────────────────────────────────────────
// Returns a compact toolbar + [Add] [Edit/Dup] [Remove] buttons as QPushButton*.
static QWidget *listToolbar(const QString &addLabel,
                              QPushButton **addOut,
                              QPushButton **editOut,
                              QPushButton **dupOut,
                              QPushButton **remOut)
{
    auto *bar = new QWidget;
    auto *lay = new QHBoxLayout(bar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(4);

    *addOut  = new QPushButton(addLabel);
    *editOut = new QPushButton("Edit…");
    *dupOut  = new QPushButton("Duplicate");
    *remOut  = new QPushButton("Remove");

    (*addOut)->setStyleSheet(
        "QPushButton{background:#1a5a1a;color:white;border-radius:3px;padding:2px 10px;}");
    (*remOut)->setStyleSheet(
        "QPushButton{background:#5a1a1a;color:white;border-radius:3px;padding:2px 10px;}");

    lay->addWidget(*addOut);
    lay->addWidget(*editOut);
    if (dupOut) lay->addWidget(*dupOut);
    lay->addWidget(*remOut);
    lay->addStretch();
    return bar;
}


// ══════════════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════════════

ConfigDialog::ConfigDialog(const QString &configPath,
                           bool serverRunning,
                           QWidget *parent)
    : QDialog(parent)
    , m_configPath(configPath)
    , m_serverRunning(serverRunning)
{
    setWindowTitle("Configuration Manager — Mcaster1DNAS");
    setMinimumSize(820, 600);
    resize(960, 700);

    // ── Tab widget ────────────────────────────────────────────────────────────
    auto *tabs = new QTabWidget(this);
    tabs->addTab(scrolled(buildGlobalTab()),   "Global");
    tabs->addTab(buildMountsTab(),             "Mounts");
    tabs->addTab(buildRelaysTab(),             "Relays");
    tabs->addTab(scrolled(buildSystemTab()),   "System");
    tabs->addTab(scrolled(buildSecurityTab()), "Security");
    tabs->addTab(buildSslCertTab(),            "SSL Cert \u2605");

    // ── Bottom status + format selector ──────────────────────────────────────
    m_configLabel = new QLabel(this);
    m_configLabel->setStyleSheet("color:#888; font-size:11px;");

    m_fmtYaml = new QRadioButton("YAML", this);
    m_fmtXml  = new QRadioButton("XML",  this);

    // Auto-detect format from path extension
    ConfigFormat fmt = detectFormat(configPath);
    m_fmtXml->setChecked(fmt == ConfigFormat::Xml);
    m_fmtYaml->setChecked(fmt != ConfigFormat::Xml);

    auto *fmtGrp = new QButtonGroup(this);
    fmtGrp->addButton(m_fmtYaml);
    fmtGrp->addButton(m_fmtXml);

    auto *saveBtn     = new QPushButton("Save");
    auto *saveYamlBtn = new QPushButton("Save as YAML…");
    auto *saveXmlBtn  = new QPushButton("Save as XML…");
    auto *closeBtn    = new QPushButton("Close");
    closeBtn->setDefault(true);

    saveBtn->setStyleSheet(
        "QPushButton{background:#1a6a1a;color:white;padding:4px 14px;"
        "border-radius:4px;font-weight:bold;}");
    saveYamlBtn->setStyleSheet(
        "QPushButton{background:#1a4a6a;color:white;padding:4px 14px;"
        "border-radius:4px;font-weight:bold;}");
    saveXmlBtn->setStyleSheet(
        "QPushButton{background:#3a3a1a;color:white;padding:4px 14px;"
        "border-radius:4px;font-weight:bold;}");

    connect(saveBtn,     &QPushButton::clicked, this, &ConfigDialog::onSave);
    connect(saveYamlBtn, &QPushButton::clicked, this, &ConfigDialog::onSaveYaml);
    connect(saveXmlBtn,  &QPushButton::clicked, this, &ConfigDialog::onSaveXml);
    connect(closeBtn,    &QPushButton::clicked, this, &QDialog::accept);

    auto *fmtLbl = new QLabel("Format:");
    auto *botLay = new QHBoxLayout;
    botLay->addWidget(m_configLabel, 1);
    botLay->addWidget(fmtLbl);
    botLay->addWidget(m_fmtYaml);
    botLay->addWidget(m_fmtXml);
    botLay->addSpacing(12);
    botLay->addWidget(saveBtn);
    botLay->addWidget(saveYamlBtn);
    botLay->addWidget(saveXmlBtn);
    botLay->addSpacing(8);
    botLay->addWidget(closeBtn);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(tabs, 1);
    mainLay->addLayout(botLay);

    // ── Load config values ────────────────────────────────────────────────────
    // Server RUNNING  (serverRunning == true):
    //   Use the live in-process config via config_get_config_unlocked() — the
    //   global config is current and consistent with what the server is serving.
    //
    // Server STOPPED  (serverRunning == false):
    //   Always parse the config file directly.  This ensures the dialog shows
    //   exactly what is on disk — the source of truth the user edits.
    //   MainWindow only passes serverRunning = true while m_subsystemsAlive is
    //   set (i.e., initialize_subsystems() has run but shutdown_subsystems() has
    //   not yet been called), so this path is safe.
    if (serverRunning) {
        const mc_config_t *cfg = config_get_config_unlocked();
        if (cfg) populate(cfg);
    } else if (!configPath.isEmpty()) {
        loadFromFile(configPath);
    }

    updateConfigLabel();
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: Global
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildGlobalTab()
{
    auto *w   = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->setSpacing(12);

    // ── Server Identity ───────────────────────────────────────────────────────
    auto *idBox = new QGroupBox("Server Identity");
    auto *idFl  = new QFormLayout(idBox);
    idFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    idFl->setVerticalSpacing(6);

    m_hostname  = new QLineEdit("localhost");
    m_location  = new QLineEdit("Earth");
    m_admin     = new QLineEdit("admin@localhost");
    m_serverID  = new QLineEdit("Mcaster1DNAS");
    m_fileserve = new QCheckBox("Enable file serving (fileserve)");
    m_fileserve->setChecked(true);
    m_maxListeners = mkSpin(-1, 100000, -1);
    m_maxListeners->setToolTip("-1 = no global listener cap");

    idFl->addRow("Hostname:",      m_hostname);
    idFl->addRow("Location:",      m_location);
    idFl->addRow("Admin email:",   m_admin);
    idFl->addRow("Server ID:",     m_serverID);
    idFl->addRow("",               m_fileserve);
    idFl->addRow("Max listeners:", m_maxListeners);
    lay->addWidget(idBox);

    // ── Listen Sockets ────────────────────────────────────────────────────────
    auto *netBox = new QGroupBox("Listen Sockets");
    auto *netLay = new QVBoxLayout(netBox);

    auto *note = new QLabel(
        "SSL: -1 = auto-detect TLS/plain  |  0 = plain HTTP only  |  1 = SSL/TLS only");
    note->setStyleSheet("color:#888; font-size:11px;");
    netLay->addWidget(note);

    m_listenTable = new QTableWidget(0, 3);
    m_listenTable->setHorizontalHeaderLabels({"Port", "Bind Address", "SSL"});
    m_listenTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_listenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listenTable->setAlternatingRowColors(true);
    m_listenTable->setFixedHeight(130);

    // Default rows
    auto addDefRow = [this](int p, const QString &a, int s) {
        int r = m_listenTable->rowCount(); m_listenTable->insertRow(r);
        m_listenTable->setItem(r, 0, new QTableWidgetItem(QString::number(p)));
        m_listenTable->setItem(r, 1, new QTableWidgetItem(a));
        m_listenTable->setItem(r, 2, new QTableWidgetItem(QString::number(s)));
    };
    addDefRow(9330, "127.0.0.1", 0);
    addDefRow(9443, "127.0.0.1", 1);

    auto *addBtn = new QPushButton("Add");
    auto *remBtn = new QPushButton("Remove");
    connect(addBtn, &QPushButton::clicked, this, &ConfigDialog::onAddListener);
    connect(remBtn, &QPushButton::clicked, this, &ConfigDialog::onRemoveListener);
    auto *netBtnRow = new QHBoxLayout;
    netBtnRow->addWidget(addBtn); netBtnRow->addWidget(remBtn); netBtnRow->addStretch();

    netLay->addWidget(m_listenTable);
    netLay->addLayout(netBtnRow);
    lay->addWidget(netBox);

    // ── Authentication ────────────────────────────────────────────────────────
    auto *authBox = new QGroupBox("Authentication");
    auto *authFl  = new QFormLayout(authBox);
    authFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    authFl->setVerticalSpacing(6);

    m_sourcePw  = new QLineEdit("CHANGE_ME_source");
    m_relayPw   = new QLineEdit("CHANGE_ME_relay");
    m_adminUser = new QLineEdit("admin");
    m_adminPw   = new QLineEdit("CHANGE_ME_admin");
    for (auto *le : {m_sourcePw, m_relayPw, m_adminPw})
        le->setEchoMode(QLineEdit::Password);

    auto *showPw = new QCheckBox("Show passwords");
    connect(showPw, &QCheckBox::toggled, this, [this](bool show) {
        auto mode = show ? QLineEdit::Normal : QLineEdit::Password;
        m_sourcePw->setEchoMode(mode); m_relayPw->setEchoMode(mode);
        m_adminPw->setEchoMode(mode);
    });

    authFl->addRow("Source password:", m_sourcePw);
    authFl->addRow("Relay password:",  m_relayPw);
    authFl->addRow("Admin username:",  m_adminUser);
    authFl->addRow("Admin password:",  m_adminPw);
    authFl->addRow("",                 showPw);
    lay->addWidget(authBox);
    lay->addStretch();
    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: Mounts
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildMountsTab()
{
    auto *w   = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->setSpacing(6);

    // Toolbar
    QPushButton *addBtn, *edtBtn, *dupBtn, *remBtn;
    lay->addWidget(listToolbar("+ Add Mount", &addBtn, &edtBtn, &dupBtn, &remBtn));

    connect(addBtn, &QPushButton::clicked, this, &ConfigDialog::onAddMount);
    connect(edtBtn, &QPushButton::clicked, this, &ConfigDialog::onEditMount);
    connect(dupBtn, &QPushButton::clicked, this, &ConfigDialog::onDuplicateMount);
    connect(remBtn, &QPushButton::clicked, this, &ConfigDialog::onRemoveMount);

    m_mountList = new QListWidget;
    m_mountList->setAlternatingRowColors(true);
    QFont mono;
#if defined(Q_OS_MAC)
    mono = QFont("Menlo", 11);
#else
    mono = QFont("Monospace", 10);
    mono.setStyleHint(QFont::TypeWriter);
#endif
    m_mountList->setFont(mono);
    connect(m_mountList, &QListWidget::itemDoubleClicked,
            this, &ConfigDialog::onEditMount);
    lay->addWidget(m_mountList, 1);

    auto *hint = new QLabel(
        "Double-click a mount to edit. All ICY2 metadata fields are editable.");
    hint->setStyleSheet("color:#777; font-size:11px;");
    lay->addWidget(hint);
    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: Relays
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildRelaysTab()
{
    auto *w   = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(8, 8, 8, 8);
    lay->setSpacing(6);

    QPushButton *addBtn, *edtBtn, *dupBtn, *remBtn;
    lay->addWidget(listToolbar("+ Add Relay", &addBtn, &edtBtn, &dupBtn, &remBtn));

    connect(addBtn, &QPushButton::clicked, this, &ConfigDialog::onAddRelay);
    connect(edtBtn, &QPushButton::clicked, this, &ConfigDialog::onEditRelay);
    connect(dupBtn, &QPushButton::clicked, [this]{  // duplicate = add copy of selected
        int idx = m_relayList->currentRow();
        if (idx < 0 || idx >= m_relays.size()) return;
        RelayEntry copy = m_relays[idx];
        copy.localMount += "-copy";
        m_relays.append(copy);
        refreshRelayList();
        m_relayList->setCurrentRow(m_relays.size() - 1);
    });
    connect(remBtn, &QPushButton::clicked, this, &ConfigDialog::onRemoveRelay);

    m_relayList = new QListWidget;
    m_relayList->setAlternatingRowColors(true);
    connect(m_relayList, &QListWidget::itemDoubleClicked,
            this, &ConfigDialog::onEditRelay);
    lay->addWidget(m_relayList, 1);

    auto *hint = new QLabel(
        "Relays pull a remote stream and re-broadcast it on a local mount point.");
    hint->setStyleSheet("color:#777; font-size:11px;");
    lay->addWidget(hint);
    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: System (Limits + Paths + Logging)
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildSystemTab()
{
    auto *w   = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->setSpacing(12);

    // ── Performance limits ────────────────────────────────────────────────────
    auto *limBox = new QGroupBox("Performance Limits");
    auto *limFl  = new QFormLayout(limBox);
    limFl->setVerticalSpacing(6);

    m_clientLimit = mkSpin(1, 100000, 50);
    m_sourceLimit = mkSpin(1, 10000,  5);
    m_workers     = mkSpin(1, 256,    4);
    m_queueSize   = mkSpin(0, 0x7fffffff, 524288, " bytes");
    m_burstSize   = mkSpin(0, 0x7fffffff, 65535,  " bytes");

    limFl->addRow("Client limit:",   m_clientLimit);
    limFl->addRow("Source limit:",   m_sourceLimit);
    limFl->addRow("Worker threads:", m_workers);
    limFl->addRow("Queue size:",     m_queueSize);
    limFl->addRow("Burst size:",     m_burstSize);
    lay->addWidget(limBox);

    // ── Timeouts ──────────────────────────────────────────────────────────────
    auto *toBox = new QGroupBox("Timeouts & History");
    auto *toFl  = new QFormLayout(toBox);
    toFl->setVerticalSpacing(6);

    m_clientTO    = mkSpin(0, 3600, 30, " s");
    m_headerTO    = mkSpin(0, 3600, 15, " s");
    m_sourceTO    = mkSpin(0, 3600, 10, " s");
    m_songHistory = mkSpin(0, 10000, 25);
    m_songHistory->setToolTip("Max songs in /songdata history (0 = unlimited)");

    toFl->addRow("Client timeout:",     m_clientTO);
    toFl->addRow("Header timeout:",     m_headerTO);
    toFl->addRow("Source timeout:",     m_sourceTO);
    toFl->addRow("Song history limit:", m_songHistory);
    lay->addWidget(toBox);

    // ── Paths ─────────────────────────────────────────────────────────────────
    auto *pathBox = new QGroupBox("Paths");
    auto *pathFl  = new QFormLayout(pathBox);
    pathFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    pathFl->setVerticalSpacing(6);

    m_baseDir   = dirRow (pathFl, "Base directory:",  ".");
    m_logDir    = dirRow (pathFl, "Log directory:",   "./logs");
    m_webRoot   = dirRow (pathFl, "Web root:",        "./web");
    m_adminRoot = dirRow (pathFl, "Admin root:",      "./admin");
    m_pidFile   = fileRow(pathFl, "PID file:",        "./logs/mcaster1.pid",
                           "PID Files (*.pid);;All (*)");
    lay->addWidget(pathBox);

    // ── Logging ───────────────────────────────────────────────────────────────
    auto *logBox = new QGroupBox("Logging");
    auto *logFl  = new QFormLayout(logBox);
    logFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    logFl->setVerticalSpacing(6);

    m_accessLog   = new QLineEdit("access.log");
    m_errorLog    = new QLineEdit("error.log");
    m_playlistLog = new QLineEdit("playlist.log");

    m_logLevel = new QComboBox;
    for (const char *l : {"debug", "info", "warn", "error"})
        m_logLevel->addItem(l);
    m_logLevel->setCurrentText("info");

    m_logSize    = mkSpin(0, 1000000, 10000, " KB");
    m_logArchive = new QCheckBox("Archive (rotate) logs");
    m_logArchive->setChecked(true);

    logFl->addRow("Access log:",     m_accessLog);
    logFl->addRow("Error log:",      m_errorLog);
    logFl->addRow("Playlist log:",   m_playlistLog);
    logFl->addRow("Error level:",    m_logLevel);
    logFl->addRow("Rotate at:",      m_logSize);
    logFl->addRow("",                m_logArchive);
    lay->addWidget(logBox);
    lay->addStretch();
    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: Security
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildSecurityTab()
{
    auto *w   = new QWidget;
    auto *lay = new QVBoxLayout(w);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->setSpacing(12);

    // ── Process identity ──────────────────────────────────────────────────────
    auto *procBox = new QGroupBox("Process Identity");
    auto *procFl  = new QFormLayout(procBox);
    procFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    procFl->setVerticalSpacing(6);

    m_chroot    = new QCheckBox("Enable chroot (requires root)");
    m_chrootDir = new QLineEdit;
    m_chrootDir->setPlaceholderText("/srv/mcaster1dnas");
    m_runUser   = new QLineEdit;
    m_runUser->setPlaceholderText("e.g. mcaster1 (optional)");
    m_runGroup  = new QLineEdit;
    m_runGroup->setPlaceholderText("e.g. mcaster1 (optional)");

    procFl->addRow("",             m_chroot);
    procFl->addRow("Chroot dir:",  m_chrootDir);
    procFl->addRow("Run as user:", m_runUser);
    procFl->addRow("Run as group:",m_runGroup);
    lay->addWidget(procBox);

    // ── TLS / SSL ─────────────────────────────────────────────────────────────
    auto *tlsBox = new QGroupBox("TLS / SSL");
    auto *tlsFl  = new QFormLayout(tlsBox);
    tlsFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    tlsFl->setVerticalSpacing(6);

    m_sslCert = fileRow(tlsFl, "Certificate (PEM):",
                         "./ssl/localhost.pem",
                         "PEM Files (*.pem);;All (*)");
    m_sslKey  = fileRow(tlsFl, "Private key (PEM):",
                         "./ssl/localhost.pem",
                         "PEM Files (*.pem);;All (*)");
    m_sslCa   = fileRow(tlsFl, "CA bundle (optional):",
                         "",
                         "PEM Files (*.pem *.crt);;All (*)");

    m_cipherList = new QLineEdit;
    m_cipherList->setPlaceholderText("e.g. ECDHE-RSA-AES256-GCM-SHA384 (blank = OpenSSL default)");
    m_cipherList->setToolTip("OpenSSL cipher string for TLS listeners");
    tlsFl->addRow("Cipher list:", m_cipherList);
    lay->addWidget(tlsBox);

    // ── HTTP Headers (CORS etc.) — optional, checkable ───────────────────────
    // Default: UNCHECKED.  populate() enables it only when config has http-headers.
    // collectHeaders() returns empty list when unchecked → nothing written on save.
    m_headersGroup = new QGroupBox("HTTP Response Headers (optional — check to enable)");
    m_headersGroup->setCheckable(true);
    m_headersGroup->setChecked(false);
    auto *hdrLay = new QVBoxLayout(m_headersGroup);

    auto *hdrNote = new QLabel(
        "Headers added to every HTTP response. Common use: CORS for web players.\n"
        "Leave unchecked if you do not need custom headers — they will not be written to the config.");
    hdrNote->setWordWrap(true);
    hdrNote->setStyleSheet("color:#888; font-size:11px;");
    hdrLay->addWidget(hdrNote);

    m_headerTable = new QTableWidget(0, 2);
    m_headerTable->setHorizontalHeaderLabels({"Header Name", "Value"});
    m_headerTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_headerTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_headerTable->setAlternatingRowColors(true);
    m_headerTable->setFixedHeight(140);
    hdrLay->addWidget(m_headerTable);

    // No pre-populated defaults — table starts empty.
    // Use "Restore CORS Defaults" to re-insert the standard CORS set.

    auto *addHBtn   = new QPushButton("Add Header");
    auto *remHBtn   = new QPushButton("Remove");
    auto *corsBtn   = new QPushButton("Restore CORS Defaults");
    connect(addHBtn,  &QPushButton::clicked, this, &ConfigDialog::onAddHeader);
    connect(remHBtn,  &QPushButton::clicked, this, &ConfigDialog::onRemoveHeader);
    connect(corsBtn,  &QPushButton::clicked, this, &ConfigDialog::onRestoreCorsDefaults);
    auto *hdrBtnRow = new QHBoxLayout;
    hdrBtnRow->addWidget(addHBtn); hdrBtnRow->addWidget(remHBtn);
    hdrBtnRow->addStretch();
    hdrBtnRow->addWidget(corsBtn);
    hdrLay->addLayout(hdrBtnRow);
    lay->addWidget(m_headersGroup);
    lay->addStretch();
    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Tab: SSL Cert  (Phase 4 — full certificate lifecycle manager)
//
// Backed by src/ssl_gen.c  ssl_gen_run(ssl_gen_params_t*)
//   gentype   : "selfsigned" | "csr"
//   subj      : "/C=US/ST=.../CN=hostname"
//   savepath  : output directory  (ssl/ by default)
//   key_bits  : 2048 | 4096
//   days      : validity in days  (selfsigned only)
//   add_to_config : 1 = patch the config file ssl-certificate path
// ══════════════════════════════════════════════════════════════════════════════

QWidget *ConfigDialog::buildSslCertTab()
{
    auto *w      = new QWidget;
    auto *mainLy = new QVBoxLayout(w);
    mainLy->setContentsMargins(12, 12, 12, 12);
    mainLy->setSpacing(10);

    // ── Current cert info (read from the PEM path on the Security tab) ─────────
    auto *infoBox = new QGroupBox("Current Certificate  —  read from ssl-certificate path");
    auto *infoLay = new QVBoxLayout(infoBox);

    auto *certInfoLabel = new QLabel(
        "Set the certificate PEM path on the Security tab and click Refresh to inspect it.");
    certInfoLabel->setWordWrap(true);
    certInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    certInfoLabel->setStyleSheet(
        "background:#0d1117; color:#7ec8e3; font-family:Menlo,Monospace; font-size:11px;"
        "padding:10px; border-radius:4px; min-height:80px;");

    auto *refreshCertBtn = new QPushButton("Refresh");
    refreshCertBtn->setFixedWidth(80);

    // Closure: reads cert from m_sslCert path, displays subject/SAN/expiry via OpenSSL
    QObject::connect(refreshCertBtn, &QPushButton::clicked, this,
                     [this, certInfoLabel] {
        QString pemPath = m_sslCert ? m_sslCert->text() : QString();
        if (pemPath.isEmpty()) {
            certInfoLabel->setText("No PEM path set on Security tab.");
            return;
        }
#ifdef HAVE_OPENSSL
        FILE *f = fopen(pemPath.toLocal8Bit().constData(), "r");
        if (!f) {
            certInfoLabel->setText("Cannot open: " + pemPath);
            return;
        }
        X509 *cert = PEM_read_X509(f, nullptr, nullptr, nullptr);
        fclose(f);
        if (!cert) {
            certInfoLabel->setText("No certificate found in: " + pemPath);
            return;
        }
        // Subject
        char subj[512] = {};
        X509_NAME_oneline(X509_get_subject_name(cert), subj, sizeof(subj) - 1);
        // Issuer
        char issuer[512] = {};
        X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) - 1);
        // Validity
        BIO *bio = BIO_new(BIO_s_mem());
        ASN1_TIME_print(bio, X509_get_notBefore(cert));
        char notBefore[64] = {};
        BIO_read(bio, notBefore, sizeof(notBefore) - 1);
        BIO_reset(bio);
        ASN1_TIME_print(bio, X509_get_notAfter(cert));
        char notAfter[64] = {};
        BIO_read(bio, notAfter, sizeof(notAfter) - 1);
        BIO_free(bio);
        // SANs
        QString sans;
        GENERAL_NAMES *gns = (GENERAL_NAMES*)X509_get_ext_d2i(
            cert, NID_subject_alt_name, nullptr, nullptr);
        if (gns) {
            for (int i = 0; i < sk_GENERAL_NAME_num(gns); ++i) {
                GENERAL_NAME *gn = sk_GENERAL_NAME_value(gns, i);
                if (gn->type == GEN_DNS) {
                    const char *dns = (const char*)ASN1_STRING_get0_data(gn->d.dNSName);
                    sans += QString("  DNS:%1\n").arg(dns);
                } else if (gn->type == GEN_IPADD) {
                    const unsigned char *ip = ASN1_STRING_get0_data(gn->d.iPAddress);
                    if (ASN1_STRING_length(gn->d.iPAddress) == 4)
                        sans += QString("  IP:%1.%2.%3.%4\n")
                            .arg(ip[0]).arg(ip[1]).arg(ip[2]).arg(ip[3]);
                }
            }
            sk_GENERAL_NAME_pop_free(gns, GENERAL_NAME_free);
        }
        X509_free(cert);

        certInfoLabel->setText(
            QString("Path:    %1\nSubject: %2\nIssuer:  %3\n"
                    "Valid:   %4  →  %5\nSANs:\n%6")
                .arg(pemPath)
                .arg(QString::fromLocal8Bit(subj))
                .arg(QString::fromLocal8Bit(issuer))
                .arg(QString::fromLocal8Bit(notBefore))
                .arg(QString::fromLocal8Bit(notAfter))
                .arg(sans.isEmpty() ? "  (none)\n" : sans));
#else
        certInfoLabel->setText("OpenSSL not available — cannot inspect certificate.");
#endif
    });

    auto *refRow = new QHBoxLayout;
    refRow->addWidget(refreshCertBtn);
    refRow->addStretch();
    infoLay->addWidget(certInfoLabel);
    infoLay->addLayout(refRow);
    mainLy->addWidget(infoBox);

    // ── Certificate Generator ─────────────────────────────────────────────────
    auto *genBox = new QGroupBox("Generate Certificate  (ssl_gen_run)");
    auto *genFl  = new QFormLayout(genBox);
    genFl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    genFl->setVerticalSpacing(6);

    // Type selector
    auto *typeCombo = new QComboBox;
    typeCombo->addItem("Self-Signed Certificate",  "selfsigned");
    typeCombo->addItem("Certificate Signing Request (CSR)", "csr");
    genFl->addRow("Type:", typeCombo);

    // Subject fields
    auto *cnEdit  = new QLineEdit("localhost");
    auto *orgEdit = new QLineEdit;  orgEdit->setPlaceholderText("Mcaster1DNAS");
    auto *ouEdit  = new QLineEdit;  ouEdit->setPlaceholderText("Streaming");
    auto *cEdit   = new QLineEdit("US");  cEdit->setMaximumWidth(50);
    auto *stEdit  = new QLineEdit;  stEdit->setPlaceholderText("State / Province");
    auto *locEdit = new QLineEdit;  locEdit->setPlaceholderText("City");

    genFl->addRow("Common Name (CN):",  cnEdit);
    genFl->addRow("Organization (O):",  orgEdit);
    genFl->addRow("Unit (OU):",         ouEdit);
    genFl->addRow("Country (C):",       cEdit);
    genFl->addRow("State (ST):",        stEdit);
    genFl->addRow("City/Locality (L):", locEdit);

    // SANs
    auto *sanEdit = new QLineEdit;
    sanEdit->setPlaceholderText("DNS:localhost,IP:127.0.0.1  (comma-separated)");
    sanEdit->setToolTip("Subject Alternative Names added to the cert.\n"
                        "Format: DNS:host1,DNS:host2,IP:1.2.3.4");
    genFl->addRow("SANs:", sanEdit);

    // Key size / validity
    auto *keyCombo = new QComboBox;
    keyCombo->addItem("2048 bits (recommended)", 2048);
    keyCombo->addItem("4096 bits (stronger)",    4096);
    genFl->addRow("Key size:", keyCombo);

    auto *daysSpin = new QSpinBox;
    daysSpin->setRange(1, 3650);
    daysSpin->setValue(825);
    daysSpin->setSuffix(" days");
    daysSpin->setToolTip("Validity period (self-signed only); 825 days = ~2.25 years");
    genFl->addRow("Validity:", daysSpin);

    // Output path
    auto *savePathEdit = new QLineEdit("./ssl");
    savePathEdit->setToolTip("Directory where key/cert/PEM files are written");
    auto *savePathBtn  = new QPushButton("Browse…");
    savePathBtn->setFixedWidth(72);
    QObject::connect(savePathBtn, &QPushButton::clicked, savePathEdit,
                     [savePathEdit] {
        QString p = QFileDialog::getExistingDirectory(nullptr,
            "Select Output Directory", savePathEdit->text());
        if (!p.isEmpty()) savePathEdit->setText(p);
    });
    auto *spRow = new QHBoxLayout;
    spRow->addWidget(savePathEdit, 1);
    spRow->addWidget(savePathBtn);
    genFl->addRow("Save to:", spRow);

    // Patch config
    auto *patchChk = new QCheckBox(
        "Update ssl-certificate path in config file after generation");
    patchChk->setChecked(true);
    genFl->addRow("", patchChk);

    mainLy->addWidget(genBox);

    // ── Generate button + output log ─────────────────────────────────────────
    auto *genBtn = new QPushButton("Generate Certificate");
    genBtn->setStyleSheet(
        "QPushButton{background:#1a5a1a;color:white;padding:6px 16px;"
        "border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#22aa22;}"
        "QPushButton:disabled{background:#333;color:#666;}");

    auto *genLog = new QLabel;
    genLog->setWordWrap(true);
    genLog->setStyleSheet(
        "background:#0d1117; color:#ccffcc; font-family:Menlo,Monospace; font-size:11px;"
        "padding:8px; border-radius:4px; min-height:60px;");
    genLog->setTextInteractionFlags(Qt::TextSelectableByMouse);
    genLog->setText("Output will appear here after generation.");

    // typeCombo enables/disables daysSpin (CSR has no validity period)
    QObject::connect(typeCombo, &QComboBox::currentIndexChanged, this,
                     [daysSpin](int idx) { daysSpin->setEnabled(idx == 0); });

    QObject::connect(genBtn, &QPushButton::clicked, this,
                     [this, genBtn, typeCombo, cnEdit, orgEdit, ouEdit, cEdit, stEdit, locEdit,
                      sanEdit, keyCombo, daysSpin, savePathEdit, patchChk, genLog,
                      certInfoLabel, refreshCertBtn] {
#ifdef HAVE_OPENSSL
        // Build subject string: /C=.../ST=.../L=.../O=.../OU=.../CN=...
        QString subj = "/CN=" + cnEdit->text().trimmed();
        if (!orgEdit->text().trimmed().isEmpty()) subj.prepend("/O="  + orgEdit->text().trimmed());
        if (!ouEdit->text().trimmed().isEmpty())  subj.prepend("/OU=" + ouEdit->text().trimmed());
        if (!stEdit->text().trimmed().isEmpty())  subj.prepend("/ST=" + stEdit->text().trimmed());
        if (!locEdit->text().trimmed().isEmpty()) subj.prepend("/L="  + locEdit->text().trimmed());
        if (!cEdit->text().trimmed().isEmpty())   subj.prepend("/C="  + cEdit->text().trimmed());
        subj = "/" + subj.mid(1);   // ensure leading slash

        // Append SANs as an extra field ssl_gen.c will parse
        // (ssl_gen_run adds SANs when "subj" contains a SAN= marker)
        QString sanStr = sanEdit->text().trimmed();

        QByteArray subjBytes = subj.toLocal8Bit();
        QByteArray typeBytes = typeCombo->currentData().toString().toLocal8Bit();
        QByteArray pathBytes = savePathEdit->text().trimmed().toLocal8Bit();
        QByteArray cfgBytes  = m_configPath.toLocal8Bit();

        ssl_gen_params_t p;
        memset(&p, 0, sizeof(p));
        p.gentype       = typeBytes.constData();
        p.subj          = subjBytes.constData();
        p.savepath      = pathBytes.constData();
        p.key_bits      = keyCombo->currentData().toInt();
        p.days          = daysSpin->value();
        p.add_to_config = patchChk->isChecked() ? 1 : 0;
        p.config_path   = cfgBytes.constData();

        genBtn->setEnabled(false);
        genLog->setText("Generating…");
        QApplication::processEvents();

        int ret = ssl_gen_run(&p);

        genBtn->setEnabled(true);
        if (ret == 0) {
            genLog->setText(
                QString("Certificate generated successfully.\n"
                        "Output: %1/\n"
                        "  selfsigned.key   — private key\n"
                        "  selfsigned.crt   — certificate\n"
                        "  selfsigned.pem   — combined (key + cert)\n\n"
                        "If 'Update config' was checked, ssl-certificate has been patched.")
                    .arg(savePathEdit->text()));
            // Refresh cert info if the cert path was updated
            if (m_sslCert) {
                QString pemPath = savePathEdit->text().trimmed() + "/selfsigned.pem";
                m_sslCert->setText(pemPath);
                if (m_sslKey) m_sslKey->setText(pemPath);
            }
            refreshCertBtn->click();
        } else {
            genLog->setText(
                QString("Generation failed: %1")
                    .arg(QString::fromLocal8Bit(ssl_gen_last_error())));
        }
#else
        genLog->setText("OpenSSL not available — cannot generate certificates.");
        (void)certInfoLabel; (void)refreshCertBtn;
#endif
    });

    auto *genBtnRow = new QHBoxLayout;
    genBtnRow->addWidget(genBtn);
    genBtnRow->addStretch();
    mainLy->addLayout(genBtnRow);
    mainLy->addWidget(genLog);
    mainLy->addStretch();

    return w;
}


// ══════════════════════════════════════════════════════════════════════════════
// Config loading
// ══════════════════════════════════════════════════════════════════════════════

void ConfigDialog::loadFromFile(const QString &path)
{
    if (path.isEmpty()) return;

    mc_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    config_init_configuration(&cfg);   // set defaults into local struct

    // init_log_subsys() creates the logging rwlock (_logger_rwl) so that
    // config_parse_file()'s internal log_write() calls don't crash on NULL.
    // log_shutdown() resets _initialized=0 afterward so initialize_subsystems()
    // can properly re-initialize logging when the server actually starts.
    init_log_subsys();
    int ret = config_parse_file(path.toLocal8Bit().constData(), &cfg);
    log_shutdown();
    if (ret == 0) {
        populate(&cfg);

        // ── Default-header injection guard ────────────────────────────────────
        // Both cfgfile.c and cfgfile_yaml.c unconditionally prepend the full
        // built-in default_headers[] table (from params.c) into
        // config->http_headers BEFORE parsing the file content.  This means
        // cfg.http_headers is ALWAYS non-NULL after config_parse_file(),
        // regardless of whether the config file actually has an http-headers
        // section.  populate() would therefore always check the headers group
        // and show all 14 built-in defaults, which then get written on Save.
        //
        // Fix: scan the raw file text to determine if http-headers is actually
        // present.  If not, clear the table and uncheck the group so the
        // defaults are invisible and will NOT be written on Save.
        {
            bool hasUserHeaders = false;
            QFile f(path);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QString content = QTextStream(&f).readAll();
                if (detectFormat(path) == ConfigFormat::Xml)
                    hasUserHeaders = content.contains("<http-headers>",
                                                       Qt::CaseInsensitive);
                else  // YAML: top-level key always starts at column 0
                    hasUserHeaders = content.startsWith("http-headers:")
                                  || content.contains("\nhttp-headers:");
            }
            if (!hasUserHeaders) {
                qDebug() << "ConfigDialog: no http-headers section in file"
                         << "— suppressing parser-injected defaults";
                m_headerTable->setRowCount(0);
                m_headersGroup->setChecked(false);
            }
        }
    } else {
        qWarning() << "ConfigDialog: config_parse_file returned" << ret
                   << "for" << path;
        QMessageBox::warning(this, "Config Parse Error",
            QString("Failed to parse config file (code %1):\n%2\n\n"
                    "The dialog will open with default values.\n"
                    "Check your YAML/XML syntax before saving, or the "
                    "saved file may replace your config with defaults.")
                .arg(ret).arg(path));
    }

    // NOTE: Do NOT call config_clear() here.
    // config_clear() acquires global_lock() which requires the server's
    // pthread mutex to be initialized (server_init must have run).
    // When the server is stopped/never started that mutex is uninitialized
    // → thread_mutex_lock_c calls abort().
    // The cfg struct is stack-allocated; its heap members (strings, lists)
    // are a small bounded leak acceptable for an infrequently-opened dialog.
}

void ConfigDialog::populate(const mc_config_t *cfg)
{
    if (!cfg) return;

    // ── Global tab ────────────────────────────────────────────────────────────
    if (cfg->hostname)     m_hostname->setText(qs(cfg->hostname));
    if (cfg->location)     m_location->setText(qs(cfg->location));
    if (cfg->admin)        m_admin->setText(qs(cfg->admin));
    if (cfg->server_id)    m_serverID->setText(qs(cfg->server_id));
    m_fileserve->setChecked(cfg->fileserve != 0);
    if (cfg->max_listeners >= 0) m_maxListeners->setValue(cfg->max_listeners);

    if (cfg->source_password) m_sourcePw->setText(qs(cfg->source_password));
    if (cfg->relay_password)  m_relayPw->setText(qs(cfg->relay_password));
    if (cfg->admin_username)  m_adminUser->setText(qs(cfg->admin_username));
    if (cfg->admin_password)  m_adminPw->setText(qs(cfg->admin_password));

    m_listenTable->setRowCount(0);
    populateListeners(cfg->listen_sock);

    // ── System tab ────────────────────────────────────────────────────────────
    if (cfg->client_limit > 0)  m_clientLimit->setValue(cfg->client_limit);
    if (cfg->source_limit > 0)  m_sourceLimit->setValue(cfg->source_limit);
    if (cfg->workers_count > 0) m_workers->setValue(cfg->workers_count);
    if (cfg->queue_size_limit)  m_queueSize->setValue((int)cfg->queue_size_limit);
    if (cfg->burst_size)        m_burstSize->setValue((int)cfg->burst_size);
    if (cfg->client_timeout)    m_clientTO->setValue(cfg->client_timeout);
    if (cfg->header_timeout)    m_headerTO->setValue(cfg->header_timeout);
    if (cfg->source_timeout)    m_sourceTO->setValue(cfg->source_timeout);
    if (cfg->song_history_limit >= 0) m_songHistory->setValue(cfg->song_history_limit);

    if (cfg->base_dir)      m_baseDir->setText(qs(cfg->base_dir));
    if (cfg->log_dir)       m_logDir->setText(qs(cfg->log_dir));
    if (cfg->webroot_dir)   m_webRoot->setText(qs(cfg->webroot_dir));
    if (cfg->adminroot_dir) m_adminRoot->setText(qs(cfg->adminroot_dir));
    if (cfg->pidfile)       m_pidFile->setText(qs(cfg->pidfile));

    if (cfg->access_log.name)   m_accessLog->setText(qs(cfg->access_log.name));
    if (cfg->error_log.name)    m_errorLog->setText(qs(cfg->error_log.name));
    if (cfg->playlist_log.name) m_playlistLog->setText(qs(cfg->playlist_log.name));
    if (cfg->error_log.size > 0)
        m_logSize->setValue((int)(cfg->error_log.size / 1024));  // stored in KB
    m_logArchive->setChecked(cfg->error_log.archive != 0);

    // ── Security tab ──────────────────────────────────────────────────────────
    m_chroot->setChecked(cfg->chroot != 0);
    if (cfg->user)     m_runUser->setText(qs(cfg->user));
    if (cfg->group)    m_runGroup->setText(qs(cfg->group));

    // cert_file may be the combined PEM (key+cert) or just the cert
    if (cfg->cert_file) m_sslCert->setText(qs(cfg->cert_file));
    if (cfg->key_file)  m_sslKey->setText(qs(cfg->key_file));
    if (cfg->ca_file)   m_sslCa->setText(qs(cfg->ca_file));
    if (cfg->cipher_list) m_cipherList->setText(qs(cfg->cipher_list));

    // ── HTTP headers ──────────────────────────────────────────────────────────
    // Always clear first — never leave the group's pre-existing state from a
    // previous populate() or from the default-empty state.  Enable the group
    // only when the config actually has http-headers defined.
    m_headerTable->setRowCount(0);
    if (cfg->http_headers) {
        m_headersGroup->setChecked(true);
        populateHeaders(cfg->http_headers);
    } else {
        m_headersGroup->setChecked(false);
    }

    // ── Aliases (round-trip — no editor, just preserve) ──────────────────────
    m_aliases.clear();
    for (const aliases *a = cfg->aliases; a; a = a->next)
        if (a->source && a->destination)
            m_aliases.append({qs(a->source), qs(a->destination)});
    if (!m_aliases.isEmpty())
        qDebug() << "ConfigDialog: loaded" << m_aliases.size() << "path aliases";

    // ── Mounts + Relays ───────────────────────────────────────────────────────
    // NOTE: config_mount_template() returns 1 for wildcard names (*?[+!@$),
    // so wildcard mounts go to config->mounts (linked list) and plain named
    // mounts go to config->mounts_tree (AVL tree).  We must traverse BOTH.
    m_mounts.clear();
    populateMounts(cfg->mounts);          // wildcard / template mounts
    populateMountsTree(cfg->mounts_tree); // named static mounts (the common case)
    qDebug() << "ConfigDialog: loaded" << m_mounts.size() << "mount(s)";
    refreshMountList();

    m_relays.clear();
    populateRelays(cfg->relays);
    refreshRelayList();
}

void ConfigDialog::populateListeners(const _listener_t *head)
{
    for (const _listener_t *l = head; l; l = l->next) {
        int r = m_listenTable->rowCount(); m_listenTable->insertRow(r);
        m_listenTable->setItem(r, 0, new QTableWidgetItem(QString::number(l->port)));
        m_listenTable->setItem(r, 1, new QTableWidgetItem(
            l->bind_address ? qs(l->bind_address) : "0.0.0.0"));
        m_listenTable->setItem(r, 2, new QTableWidgetItem(QString::number(l->ssl)));
    }
}

void ConfigDialog::populateMounts(const _mount_proxy *head)
{
    for (const _mount_proxy *mp = head; mp; mp = mp->next) {
        MountEntry e;
        e.mountName         = qs(mp->mountname);
        e.mountType         = mp->mount_type ? qs(mp->mount_type) : "live";
        e.maxListeners      = mp->max_listeners;
        e.isPublic          = mp->yp_public;
        e.hidden            = mp->hidden;
        e.streamName        = qs(mp->stream_name);
        e.streamDescription = qs(mp->stream_description);
        e.streamUrl         = qs(mp->stream_url);
        e.genre             = qs(mp->stream_genre);
        e.bitrate           = qs(mp->bitrate);
        e.mimeType          = qs(mp->type);
        e.mimeSubtype       = qs(mp->subtype);
        e.username          = qs(mp->username);
        e.password          = qs(mp->password);
        if (mp->fallback.mount) e.fallbackMount = qs(mp->fallback.mount);
        e.fallbackWhenFull  = mp->fallback_when_full;
        e.onDemand          = (mp->fallback.flags & 1);  // on-demand flag
        e.burstSize         = mp->burst_size;
        e.waitTime          = mp->wait_time;
        e.lingerDuration    = mp->linger_duration;
        e.skipAccessLog     = mp->skip_accesslog;
        e.allowChunked      = mp->allow_chunked;
        e.maxListenerDuration = (int)mp->max_listener_duration;
        e.maxStreamDuration = (int)mp->max_stream_duration;
        e.noMount           = mp->no_mount;
        e.hijack            = mp->hijack;

        // ICY2 extra metadata kv_pair_t linked list
        for (const kv_pair_t *kv = mp->extra_meta; kv; kv = kv->next)
            if (kv->key) e.extraMeta.append({qs(kv->key), qs(kv->value)});

        qDebug() << "ConfigDialog: mount (list)" << e.mountName << "type=" << e.mountType;
        m_mounts.append(e);
    }
}

void ConfigDialog::populateMountsTree(const _avl_tree *tree)
{
    if (!tree) return;
    // AVL tree holds named static mounts (no wildcard chars in mountname).
    // Iterate in-order using avl_get_first() / avl_get_next(); each node->key
    // is a mount_proxy *.
    avl_node *node = avl_get_first((avl_tree *)tree);
    while (node) {
        const mount_proxy *mp = (const mount_proxy *)node->key;
        if (mp && mp->mountname) {
            MountEntry e;
            e.mountName         = qs(mp->mountname);
            e.mountType         = mp->mount_type ? qs(mp->mount_type) : "live";
            e.maxListeners      = mp->max_listeners;
            e.isPublic          = mp->yp_public;
            e.hidden            = mp->hidden;
            e.streamName        = qs(mp->stream_name);
            e.streamDescription = qs(mp->stream_description);
            e.streamUrl         = qs(mp->stream_url);
            e.genre             = qs(mp->stream_genre);
            e.bitrate           = qs(mp->bitrate);
            e.mimeType          = qs(mp->type);
            e.mimeSubtype       = qs(mp->subtype);
            e.username          = qs(mp->username);
            e.password          = qs(mp->password);
            if (mp->fallback.mount) e.fallbackMount = qs(mp->fallback.mount);
            e.fallbackWhenFull  = mp->fallback_when_full;
            e.onDemand          = (mp->fallback.flags & 1);
            e.burstSize         = mp->burst_size;
            e.waitTime          = mp->wait_time;
            e.lingerDuration    = mp->linger_duration;
            e.skipAccessLog     = mp->skip_accesslog;
            e.allowChunked      = mp->allow_chunked;
            e.maxListenerDuration = (int)mp->max_listener_duration;
            e.maxStreamDuration = (int)mp->max_stream_duration;
            e.noMount           = mp->no_mount;
            e.hijack            = mp->hijack;
            for (const kv_pair_t *kv = mp->extra_meta; kv; kv = kv->next)
                if (kv->key) e.extraMeta.append({qs(kv->key), qs(kv->value)});
            qDebug() << "ConfigDialog: mount (tree)" << e.mountName << "type=" << e.mountType;
            m_mounts.append(e);
        }
        node = avl_get_next(node);
    }
}

void ConfigDialog::populateRelays(const _relay_server *head)
{
    // IMPORTANT: relay linked list traverses via ->new_details (not ->next!)
    // Confirmed by cfgfile.c lines 1825-1826 + slave.c line 1166.
    for (const _relay_server *r = head; r; r = r->new_details) {
        RelayEntry e;
        e.localMount    = qs(r->localmount);
        e.username      = qs(r->username);
        e.password      = qs(r->password);
        e.onDemand      = (r->flags & RELAY_ON_DEMAND) ? 1 : 0;
        e.interval      = r->interval;
        e.icyMeta       = (r->flags & RELAY_ICY_META) ? 1 : 0;
        if (r->hosts) {
            e.remoteServer = qs(r->hosts->ip);
            e.remotePort   = r->hosts->port;
            e.remoteMount  = qs(r->hosts->mount);
            e.ssl          = r->hosts->secure;
        }
        m_relays.append(e);
    }
}

void ConfigDialog::populateHeaders(const _config_http_header_tag *head)
{
    for (const _config_http_header_tag *h = head; h; h = h->next) {
        if (!h->hdr.name) continue;
        int r = m_headerTable->rowCount(); m_headerTable->insertRow(r);
        m_headerTable->setItem(r, 0, new QTableWidgetItem(qs(h->hdr.name)));
        m_headerTable->setItem(r, 1, new QTableWidgetItem(qs(h->hdr.value)));
    }
}


// ══════════════════════════════════════════════════════════════════════════════
// List refresh helpers
// ══════════════════════════════════════════════════════════════════════════════

void ConfigDialog::refreshMountList()
{
    m_mountList->clear();
    for (const auto &mp : m_mounts) {
        QString maxStr = mp.maxListeners < 0
            ? "\u221e" : QString::number(mp.maxListeners);
        m_mountList->addItem(
            QString("%-20s  %-10s  max=%-5s  %s")
                .arg(mp.mountName, -20)
                .arg(mp.mountType, -10)
                .arg(maxStr, -5)
                .arg(mp.streamName));
    }
}

void ConfigDialog::refreshRelayList()
{
    m_relayList->clear();
    for (const auto &r : m_relays) {
        QString sslStr = r.ssl ? " [SSL]" : "";
        m_relayList->addItem(
            QString("%-20s  \u2190  %2:%3%4%5")
                .arg(r.localMount, -20)
                .arg(r.remoteServer)
                .arg(r.remotePort)
                .arg(r.remoteMount)
                .arg(sslStr));
    }
}

void ConfigDialog::updateConfigLabel()
{
    if (m_configPath.isEmpty())
        m_configLabel->setText("Config: (none)");
    else
        m_configLabel->setText("Config: " + m_configPath
            + (m_serverRunning ? "  \u25CF running" : "  \u25CB stopped"));
}


// ══════════════════════════════════════════════════════════════════════════════
// Listener Add / Remove
// ══════════════════════════════════════════════════════════════════════════════

void ConfigDialog::onAddListener()
{
    int r = m_listenTable->rowCount(); m_listenTable->insertRow(r);
    m_listenTable->setItem(r, 0, new QTableWidgetItem("9330"));
    m_listenTable->setItem(r, 1, new QTableWidgetItem("0.0.0.0"));
    m_listenTable->setItem(r, 2, new QTableWidgetItem("0"));
    m_listenTable->editItem(m_listenTable->item(r, 0));
}

void ConfigDialog::onRemoveListener()
{
    int r = m_listenTable->currentRow();
    if (r >= 0) m_listenTable->removeRow(r);
}


// ══════════════════════════════════════════════════════════════════════════════
// Mount edit sub-dialog  (comprehensive — all mount_proxy fields)
// ══════════════════════════════════════════════════════════════════════════════

static bool editMountDialog(MountEntry &e, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(e.mountName.isEmpty() ? "New Mount Point" : "Edit: " + e.mountName);
    dlg.resize(560, 680);
    dlg.setMinimumWidth(520);

    auto *tabs = new QTabWidget(&dlg);

    // ── Tab 1: Basic ──────────────────────────────────────────────────────────
    {
        auto *w  = new QWidget;
        auto *fl = new QFormLayout(w);
        fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        fl->setVerticalSpacing(6); fl->setContentsMargins(10,10,10,10);

        auto *nameEdit = new QLineEdit(e.mountName);
        auto *typeCombo= new QComboBox;
        for (const char *t : {"live","podcast","ondemand","socialmedia"})
            typeCombo->addItem(t);
        typeCombo->setCurrentText(e.mountType);
        auto *maxSpin  = mkSpin(-1,100000,e.maxListeners); maxSpin->setToolTip("-1 = global limit");
        auto *pubCheck = new QCheckBox("Public (announce to YP/directory)");
        pubCheck->setChecked(e.isPublic);
        auto *hidCheck = new QCheckBox("Hidden (do not show in XSL status pages)");
        hidCheck->setChecked(e.hidden);
        auto *noMntChk = new QCheckBox("No direct requests (indirect/fallback only)");
        noMntChk->setChecked(e.noMount);
        auto *hijackChk= new QCheckBox("Allow authenticated source to hijack existing stream");
        hijackChk->setChecked(e.hijack);

        fl->addRow("Mount name:",    nameEdit);
        fl->addRow("Mount type:",    typeCombo);
        fl->addRow("Max listeners:", maxSpin);
        fl->addRow("", pubCheck);
        fl->addRow("", hidCheck);
        fl->addRow("", noMntChk);
        fl->addRow("", hijackChk);
        tabs->addTab(scrolled(w), "Basic");

        // Store refs for collection at end
        dlg.setProperty("nameEdit",  QVariant::fromValue((void*)nameEdit));
        dlg.setProperty("typeCombo", QVariant::fromValue((void*)typeCombo));
        dlg.setProperty("maxSpin",   QVariant::fromValue((void*)maxSpin));
        dlg.setProperty("pubCheck",  QVariant::fromValue((void*)pubCheck));
        dlg.setProperty("hidCheck",  QVariant::fromValue((void*)hidCheck));
        dlg.setProperty("noMntChk",  QVariant::fromValue((void*)noMntChk));
        dlg.setProperty("hijackChk", QVariant::fromValue((void*)hijackChk));
    }

    // ── Tab 2: Stream Metadata ────────────────────────────────────────────────
    {
        auto *w  = new QWidget;
        auto *fl = new QFormLayout(w);
        fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        fl->setVerticalSpacing(6); fl->setContentsMargins(10,10,10,10);

        auto *snEdit  = new QLineEdit(e.streamName);
        auto *sdEdit  = new QLineEdit(e.streamDescription);
        auto *suEdit  = new QLineEdit(e.streamUrl);
        auto *gnEdit  = new QLineEdit(e.genre);
        auto *brEdit  = new QLineEdit(e.bitrate);
        auto *mtEdit  = new QLineEdit(e.mimeType);
        auto *stEdit  = new QLineEdit(e.mimeSubtype);

        fl->addRow("Stream name:",        snEdit);
        fl->addRow("Description:",        sdEdit);
        fl->addRow("Stream URL:",         suEdit);
        fl->addRow("Genre:",              gnEdit);
        fl->addRow("Bitrate (kbps):",     brEdit);
        fl->addRow("MIME type:",          mtEdit);
        fl->addRow("MIME subtype:",       stEdit);
        tabs->addTab(scrolled(w), "Stream Info");

        dlg.setProperty("snEdit", QVariant::fromValue((void*)snEdit));
        dlg.setProperty("sdEdit", QVariant::fromValue((void*)sdEdit));
        dlg.setProperty("suEdit", QVariant::fromValue((void*)suEdit));
        dlg.setProperty("gnEdit", QVariant::fromValue((void*)gnEdit));
        dlg.setProperty("brEdit", QVariant::fromValue((void*)brEdit));
        dlg.setProperty("mtEdit", QVariant::fromValue((void*)mtEdit));
        dlg.setProperty("stEdit", QVariant::fromValue((void*)stEdit));
    }

    // ── Tab 3: Access Control ─────────────────────────────────────────────────
    {
        auto *w  = new QWidget;
        auto *fl = new QFormLayout(w);
        fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        fl->setVerticalSpacing(6); fl->setContentsMargins(10,10,10,10);

        auto *userEdit = new QLineEdit(e.username);
        auto *pwEdit   = new QLineEdit(e.password);
        pwEdit->setEchoMode(QLineEdit::Password);
        auto *showPw   = new QCheckBox("Show password");
        QObject::connect(showPw, &QCheckBox::toggled, pwEdit, [pwEdit](bool s){
            pwEdit->setEchoMode(s ? QLineEdit::Normal : QLineEdit::Password);
        });
        auto *fbEdit  = new QLineEdit(e.fallbackMount);
        fbEdit->setPlaceholderText("/fallback-mount (optional)");
        auto *fbFull  = new QCheckBox("Fallback when max listeners reached");
        fbFull->setChecked(e.fallbackWhenFull);
        auto *odCheck = new QCheckBox("On-demand (only connect to source when listener present)");
        odCheck->setChecked(e.onDemand);
        auto *skipLog = new QCheckBox("Skip access log for this mount");
        skipLog->setChecked(e.skipAccessLog);
        auto *chunkOk = new QCheckBox("Allow chunked transfer encoding");
        chunkOk->setChecked(e.allowChunked);

        fl->addRow("Source username:",  userEdit);
        fl->addRow("Source password:",  pwEdit);
        fl->addRow("",                  showPw);
        fl->addRow("Fallback mount:",   fbEdit);
        fl->addRow("",                  fbFull);
        fl->addRow("",                  odCheck);
        fl->addRow("",                  skipLog);
        fl->addRow("",                  chunkOk);
        tabs->addTab(scrolled(w), "Access");

        dlg.setProperty("userEdit", QVariant::fromValue((void*)userEdit));
        dlg.setProperty("pwEdit",   QVariant::fromValue((void*)pwEdit));
        dlg.setProperty("fbEdit",   QVariant::fromValue((void*)fbEdit));
        dlg.setProperty("fbFull",   QVariant::fromValue((void*)fbFull));
        dlg.setProperty("odCheck",  QVariant::fromValue((void*)odCheck));
        dlg.setProperty("skipLog",  QVariant::fromValue((void*)skipLog));
        dlg.setProperty("chunkOk",  QVariant::fromValue((void*)chunkOk));
    }

    // ── Tab 4: Advanced / Timing ──────────────────────────────────────────────
    {
        auto *w  = new QWidget;
        auto *fl = new QFormLayout(w);
        fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        fl->setVerticalSpacing(6); fl->setContentsMargins(10,10,10,10);

        auto *burstSpin  = mkSpin(0,0x7fffffff,(int)e.burstSize," bytes");
        burstSpin->setToolTip("0 = use global burst-size");
        auto *waitSpin   = mkSpin(0,86400,e.waitTime," s");
        waitSpin->setToolTip("Seconds to keep mount after source disconnects");
        auto *lingerSpin = mkSpin(0,86400,e.lingerDuration," s");
        auto *maxLDSpin  = mkSpin(0,86400,e.maxListenerDuration," s");
        maxLDSpin->setToolTip("0 = unlimited");
        auto *maxSDSpin  = mkSpin(0,86400,e.maxStreamDuration," s");
        maxSDSpin->setToolTip("0 = unlimited");

        fl->addRow("Burst size (override):", burstSpin);
        fl->addRow("Wait time:",             waitSpin);
        fl->addRow("Linger duration:",       lingerSpin);
        fl->addRow("Max listener duration:", maxLDSpin);
        fl->addRow("Max stream duration:",   maxSDSpin);
        tabs->addTab(scrolled(w), "Advanced");

        dlg.setProperty("burstSpin",  QVariant::fromValue((void*)burstSpin));
        dlg.setProperty("waitSpin",   QVariant::fromValue((void*)waitSpin));
        dlg.setProperty("lingerSpin", QVariant::fromValue((void*)lingerSpin));
        dlg.setProperty("maxLDSpin",  QVariant::fromValue((void*)maxLDSpin));
        dlg.setProperty("maxSDSpin",  QVariant::fromValue((void*)maxSDSpin));
    }

    // ── Tab 5: ICY2.2 Metadata — structured form ──────────────────────────────
    // All well-known icy-meta-* keys are shown as labelled fields.
    // Values are pre-populated from e.extraMeta where a matching key exists;
    // otherwise blank.  Fields can be overridden during a live broadcast /
    // metadata push event without touching the config — these are *static*
    // defaults that the server sends until the source overrides them.
    //
    // Custom / non-standard keys are kept in the "Extra (custom)" table at
    // the bottom and round-trip through extraMeta unchanged.
    {
        // Helper: look up value in e.extraMeta for a given icy key
        auto icyVal = [&e](const QString &key) -> QString {
            for (const auto &kv : e.extraMeta)
                if (kv.first == key) return kv.second;
            return {};
        };

        auto *outer   = new QWidget;
        auto *outerLy = new QVBoxLayout(outer);
        outerLy->setContentsMargins(0,0,0,0);

        // inner scrollable widget
        auto *inner   = new QWidget;
        auto *innerLy = new QVBoxLayout(inner);
        innerLy->setContentsMargins(10,10,10,10);
        innerLy->setSpacing(8);

        auto *headerNote = new QLabel(
            "Static ICY2.2 metadata defaults for this mount. "
            "Values shown here will be broadcast until overridden by a live "
            "source connection or real-time metadata push. "
            "Leave blank to omit a field from the stream headers.");
        headerNote->setWordWrap(true);
        headerNote->setStyleSheet("color:#aaa; font-size:11px;");
        innerLy->addWidget(headerNote);

        // Map of property-name → QLineEdit* for collection at accept-time
        // Stored in dlg as "icy2_fields" via a string-keyed map
        // We use a QMap<QString,QLineEdit*>* on the heap, owned by the dialog's
        // parent chain (inner widget).
        auto *icy2Fields = new QMap<QString,QLineEdit*>;

        // Helper to add a group with labelled line-edit fields
        auto mkGroup = [&](const QString &title,
                           const QList<QPair<QString,QString>> &fields) {
            auto *box = new QGroupBox(title);
            auto *fl  = new QFormLayout(box);
            fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            fl->setVerticalSpacing(4);
            fl->setContentsMargins(8,8,8,8);
            for (const auto &f : fields) {
                const QString &key   = f.first;
                const QString &label = f.second;
                auto *le = new QLineEdit(icyVal(key));
                le->setPlaceholderText("(blank = not set)");
                le->setToolTip("ICY2.2 key: " + key);
                fl->addRow(label + ":", le);
                (*icy2Fields)[key] = le;
            }
            innerLy->addWidget(box);
        };

        // ── Station Identity ──────────────────────────────────────────────
        mkGroup("Station Identity", {
            {"icy-meta-station-id",      "Station ID"},
            {"icy-meta-name",            "Station name"},
            {"icy-meta-url",             "Station URL"},
            {"icy-meta-genre",           "Genre"},
            {"icy-meta-station-slogan",  "Slogan"},
            {"icy-meta-station-logo",    "Logo URL"},
            {"icy-meta-station-type",    "Station type (internet/terrestrial)"},
            {"icy-meta-station-country", "Country"},
            {"icy-meta-station-founded", "Founded"},
        });

        // ── DJ / Show ─────────────────────────────────────────────────────
        mkGroup("DJ / Show", {
            {"icy-meta-dj-handle",        "DJ handle"},
            {"icy-meta-dj-bio",           "DJ bio"},
            {"icy-meta-dj-photo",         "DJ photo URL"},
            {"icy-meta-dj-showrating",    "Show rating"},
            {"icy-meta-show-title",       "Show title"},
            {"icy-meta-show-episode",     "Episode"},
            {"icy-meta-show-season",      "Season"},
            {"icy-meta-show-start-time",  "Show start time (ISO 8601)"},
            {"icy-meta-show-end-time",    "Show end time (ISO 8601)"},
            {"icy-meta-playlist-name",    "Playlist name"},
            {"icy-meta-autodj",           "AutoDJ active (0/1)"},
            {"icy-meta-stream-session-id","Stream session ID"},
        });

        // ── Track ─────────────────────────────────────────────────────────
        mkGroup("Track Metadata", {
            {"icy-meta-track-title",    "Track title"},
            {"icy-meta-track-artist",   "Artist"},
            {"icy-meta-track-album",    "Album"},
            {"icy-meta-track-year",     "Year"},
            {"icy-meta-track-isrc",     "ISRC"},
            {"icy-meta-track-label",    "Label"},
            {"icy-meta-track-bpm",      "BPM"},
            {"icy-meta-track-artwork",  "Artwork URL"},
            {"icy-meta-track-buy-url",  "Buy URL"},
        });

        // ── Podcast ───────────────────────────────────────────────────────
        mkGroup("Podcast", {
            {"icy-meta-podcast-host",    "Podcast host"},
            {"icy-meta-podcast-rss",     "RSS feed URL"},
            {"icy-meta-podcast-episode", "Episode"},
            {"icy-meta-duration",        "Duration"},
            {"icy-meta-language",        "Language"},
            {"icy-meta-podcast-rating",  "Rating"},
        });

        // ── Audio / Video ─────────────────────────────────────────────────
        mkGroup("Audio / Video", {
            {"icy-meta-audio-codec",       "Audio codec (MP3/AAC/FLAC/Opus)"},
            {"icy-meta-audio-samplerate",  "Sample rate (Hz)"},
            {"icy-meta-audio-channels",    "Channels"},
            {"icy-meta-audio-quality",     "Audio quality"},
            {"icy-meta-audio-lossless",    "Lossless (0/1)"},
            {"icy-meta-video-type",        "Video type"},
            {"icy-meta-video-link",        "Video link URL"},
            {"icy-meta-video-platform",    "Video platform (YouTube/Twitch…)"},
            {"icy-meta-video-resolution",  "Resolution (e.g. 1920x1080)"},
            {"icy-meta-video-codec",       "Video codec"},
            {"icy-meta-video-framerate",   "Framerate"},
            {"icy-meta-video-bitrate",     "Video bitrate (kbps)"},
            {"icy-meta-video-aspect",      "Aspect ratio"},
            {"icy-meta-video-hdr",         "HDR (0/1)"},
            {"icy-meta-video-thumbnail",   "Thumbnail URL"},
            {"icy-meta-video-duration",    "Video duration"},
            {"icy-meta-video-rating",      "Video rating"},
        });

        // ── Social Media ──────────────────────────────────────────────────
        mkGroup("Social Media / Engagement", {
            {"icy-meta-social-twitter",   "Twitter / X handle"},
            {"icy-meta-social-ig",        "Instagram handle"},
            {"icy-meta-social-tiktok",    "TikTok handle"},
            {"icy-meta-social-facebook",  "Facebook page"},
            {"icy-meta-social-youtube",   "YouTube channel"},
            {"icy-meta-social-bluesky",   "Bluesky handle"},
            {"icy-meta-social-website",   "Website URL"},
            {"icy-meta-tip-url",          "Tip / donation URL"},
            {"icy-meta-chat-url",         "Live chat URL"},
            {"icy-meta-share-url",        "Share URL"},
            {"icy-meta-request-url",      "Song request URL"},
            {"icy-meta-hashtags",         "Hashtags"},
            {"icy-meta-emoji",            "Emoji"},
        });

        // ── Compliance / Access ───────────────────────────────────────────
        mkGroup("Compliance / Access Control", {
            {"icy-meta-nsfw",              "NSFW (0/1)"},
            {"icy-meta-ai-generated",      "AI-generated content (0/1)"},
            {"icy-meta-content-rating",    "Content rating"},
            {"icy-meta-parental-advisory", "Parental advisory (0/1)"},
            {"icy-meta-dmca-compliant",    "DMCA compliant (0/1)"},
            {"icy-meta-geo-region",        "Geo region restriction"},
            {"icy-meta-notice-board",      "Notice board message"},
            {"icy-meta-upcoming-show",     "Upcoming show info"},
            {"icy-meta-emergency-alert",   "Emergency alert"},
        });

        // ── Listener stats (read-only hints) ─────────────────────────────
        mkGroup("Listener / Broadcast Stats  (usually set by server, not config)", {
            {"icy-meta-listener-peak",      "Listener peak"},
            {"icy-meta-listener-count",     "Current listener count"},
            {"icy-meta-like-count",         "Like count"},
            {"icy-meta-cdn-region",         "CDN region"},
            {"icy-meta-relay-origin",       "Relay origin"},
            {"icy-meta-stream-failover",    "Failover URL"},
            {"icy-meta-stream-quality-tier","Quality tier"},
            {"icy-meta-verification-status","Verification status"},
        });

        // ── Custom / unknown icy-* keys (raw table for round-trip) ───────
        auto *customBox = new QGroupBox("Extra / Custom  icy-*  Keys");
        auto *customLay = new QVBoxLayout(customBox);

        // Keys NOT matching any well-known key go here
        QStringList knownKeys;
        for (auto it = icy2Fields->begin(); it != icy2Fields->end(); ++it)
            knownKeys << it.key();

        auto *kvTable = new QTableWidget(0, 2);
        kvTable->setHorizontalHeaderLabels({"Key  (icy-meta-…)", "Value"});
        kvTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        kvTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        kvTable->setAlternatingRowColors(true);

        for (const auto &kv : e.extraMeta) {
            if (knownKeys.contains(kv.first)) continue;  // already in form above
            int r = kvTable->rowCount(); kvTable->insertRow(r);
            kvTable->setItem(r, 0, new QTableWidgetItem(kv.first));
            kvTable->setItem(r, 1, new QTableWidgetItem(kv.second));
        }
        customLay->addWidget(kvTable);

        auto *kvAddBtn = new QPushButton("Add Row");
        auto *kvRemBtn = new QPushButton("Remove");
        QObject::connect(kvAddBtn, &QPushButton::clicked, kvTable, [kvTable]{
            int r = kvTable->rowCount(); kvTable->insertRow(r);
            kvTable->setItem(r, 0, new QTableWidgetItem("icy-meta-"));
            kvTable->setItem(r, 1, new QTableWidgetItem(""));
            kvTable->editItem(kvTable->item(r, 0));
        });
        QObject::connect(kvRemBtn, &QPushButton::clicked, kvTable, [kvTable]{
            int r = kvTable->currentRow();
            if (r >= 0) kvTable->removeRow(r);
        });
        auto *kvBtnRow = new QHBoxLayout;
        kvBtnRow->addWidget(kvAddBtn); kvBtnRow->addWidget(kvRemBtn); kvBtnRow->addStretch();
        customLay->addLayout(kvBtnRow);
        innerLy->addWidget(customBox);
        innerLy->addStretch();

        outerLy->addWidget(scrolled(inner));
        tabs->addTab(outer, "ICY2 Metadata");

        // Store pointers for collection at accept-time
        dlg.setProperty("icy2Fields", QVariant::fromValue((void*)icy2Fields));
        dlg.setProperty("kvTable",    QVariant::fromValue((void*)kvTable));
    }

    // ── Dialog buttons ────────────────────────────────────────────────────────
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *mainLay = new QVBoxLayout(&dlg);
    mainLay->addWidget(tabs, 1);
    mainLay->addWidget(bb);

    if (dlg.exec() != QDialog::Accepted) return false;

    // Collect all values back into e
    auto *nameEdit = (QLineEdit*)dlg.property("nameEdit").value<void*>();
    auto *typeCombo= (QComboBox*)dlg.property("typeCombo").value<void*>();
    auto *maxSpin  = (QSpinBox*) dlg.property("maxSpin").value<void*>();
    auto *pubCheck = (QCheckBox*)dlg.property("pubCheck").value<void*>();
    auto *hidCheck = (QCheckBox*)dlg.property("hidCheck").value<void*>();
    auto *noMntChk = (QCheckBox*)dlg.property("noMntChk").value<void*>();
    auto *hijackChk= (QCheckBox*)dlg.property("hijackChk").value<void*>();
    auto *snEdit   = (QLineEdit*)dlg.property("snEdit").value<void*>();
    auto *sdEdit   = (QLineEdit*)dlg.property("sdEdit").value<void*>();
    auto *suEdit   = (QLineEdit*)dlg.property("suEdit").value<void*>();
    auto *gnEdit   = (QLineEdit*)dlg.property("gnEdit").value<void*>();
    auto *brEdit   = (QLineEdit*)dlg.property("brEdit").value<void*>();
    auto *mtEdit   = (QLineEdit*)dlg.property("mtEdit").value<void*>();
    auto *stEdit   = (QLineEdit*)dlg.property("stEdit").value<void*>();
    auto *userEdit = (QLineEdit*)dlg.property("userEdit").value<void*>();
    auto *pwEdit   = (QLineEdit*)dlg.property("pwEdit").value<void*>();
    auto *fbEdit   = (QLineEdit*)dlg.property("fbEdit").value<void*>();
    auto *fbFull   = (QCheckBox*)dlg.property("fbFull").value<void*>();
    auto *odCheck  = (QCheckBox*)dlg.property("odCheck").value<void*>();
    auto *skipLog  = (QCheckBox*)dlg.property("skipLog").value<void*>();
    auto *chunkOk  = (QCheckBox*)dlg.property("chunkOk").value<void*>();
    auto *burstSpin= (QSpinBox*) dlg.property("burstSpin").value<void*>();
    auto *waitSpin = (QSpinBox*) dlg.property("waitSpin").value<void*>();
    auto *lingerSp = (QSpinBox*) dlg.property("lingerSpin").value<void*>();
    auto *maxLDSp  = (QSpinBox*) dlg.property("maxLDSpin").value<void*>();
    auto *maxSDSp  = (QSpinBox*) dlg.property("maxSDSpin").value<void*>();
    auto *icy2Fields = (QMap<QString,QLineEdit*>*)dlg.property("icy2Fields").value<void*>();
    auto *kvTable    = (QTableWidget*)dlg.property("kvTable").value<void*>();

    e.mountName         = nameEdit->text().trimmed();
    e.mountType         = typeCombo->currentText();
    e.maxListeners      = maxSpin->value();
    e.isPublic          = pubCheck->isChecked() ? 1 : 0;
    e.hidden            = hidCheck->isChecked() ? 1 : 0;
    e.noMount           = noMntChk->isChecked() ? 1 : 0;
    e.hijack            = hijackChk->isChecked() ? 1 : 0;
    e.streamName        = snEdit->text();
    e.streamDescription = sdEdit->text();
    e.streamUrl         = suEdit->text();
    e.genre             = gnEdit->text();
    e.bitrate           = brEdit->text();
    e.mimeType          = mtEdit->text();
    e.mimeSubtype       = stEdit->text();
    e.username          = userEdit->text();
    e.password          = pwEdit->text();
    e.fallbackMount     = fbEdit->text().trimmed();
    e.fallbackWhenFull  = fbFull->isChecked() ? 1 : 0;
    e.onDemand          = odCheck->isChecked() ? 1 : 0;
    e.skipAccessLog     = skipLog->isChecked() ? 1 : 0;
    e.allowChunked      = chunkOk->isChecked() ? 1 : 0;
    e.burstSize         = (uint32_t)burstSpin->value();
    e.waitTime          = waitSpin->value();
    e.lingerDuration    = lingerSp->value();
    e.maxListenerDuration = maxLDSp->value();
    e.maxStreamDuration = maxSDSp->value();

    // Collect ICY2.2 extra_meta: structured form fields first (non-empty only),
    // then custom kv table rows (preserves unknown keys round-trip).
    e.extraMeta.clear();
    if (icy2Fields) {
        for (auto it = icy2Fields->begin(); it != icy2Fields->end(); ++it) {
            QString val = it.value()->text().trimmed();
            if (!val.isEmpty())
                e.extraMeta.append({it.key(), val});
        }
        delete icy2Fields;   // heap-allocated above; we own it here
    }
    if (kvTable) {
        for (int r = 0; r < kvTable->rowCount(); ++r) {
            QString k = kvTable->item(r,0) ? kvTable->item(r,0)->text().trimmed() : QString();
            QString v = kvTable->item(r,1) ? kvTable->item(r,1)->text()           : QString();
            if (!k.isEmpty()) e.extraMeta.append({k, v});
        }
    }
    return true;
}

void ConfigDialog::onAddMount()
{
    MountEntry e; e.mountName = "/new-mount";
    if (!editMountDialog(e, this)) return;
    m_mounts.append(e); refreshMountList();
    m_mountList->setCurrentRow(m_mounts.size() - 1);
}

void ConfigDialog::onEditMount()
{
    int idx = m_mountList->currentRow();
    if (idx < 0 || idx >= m_mounts.size()) return;
    MountEntry e = m_mounts[idx];
    if (!editMountDialog(e, this)) return;
    m_mounts[idx] = e; refreshMountList();
    m_mountList->setCurrentRow(idx);
}

void ConfigDialog::onDuplicateMount()
{
    int idx = m_mountList->currentRow();
    if (idx < 0 || idx >= m_mounts.size()) return;
    MountEntry copy = m_mounts[idx];
    copy.mountName += "-copy";
    m_mounts.append(copy); refreshMountList();
    m_mountList->setCurrentRow(m_mounts.size() - 1);
}

void ConfigDialog::onRemoveMount()
{
    int idx = m_mountList->currentRow();
    if (idx < 0 || idx >= m_mounts.size()) return;
    m_mounts.removeAt(idx); refreshMountList();
}


// ══════════════════════════════════════════════════════════════════════════════
// Relay edit sub-dialog
// ══════════════════════════════════════════════════════════════════════════════

static bool editRelayDialog(RelayEntry &e, QWidget *parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(e.localMount.isEmpty() ? "New Relay" : "Edit Relay: " + e.localMount);
    dlg.resize(440, 340);

    auto *fl = new QFormLayout;
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    fl->setVerticalSpacing(7);
    fl->setContentsMargins(12, 12, 12, 12);

    auto *localEdit  = new QLineEdit(e.localMount);
    localEdit->setPlaceholderText("/relay-mount");
    auto *srvEdit    = new QLineEdit(e.remoteServer);
    srvEdit->setPlaceholderText("remote.server.com");
    auto *portSpin   = mkSpin(1, 65535, e.remotePort);
    auto *mntEdit    = new QLineEdit(e.remoteMount);
    mntEdit->setPlaceholderText("/live");
    auto *sslCombo   = new QComboBox;
    sslCombo->addItems({"0 — plain HTTP", "1 — SSL/TLS"});
    sslCombo->setCurrentIndex(e.ssl ? 1 : 0);
    auto *userEdit   = new QLineEdit(e.username);
    auto *pwEdit     = new QLineEdit(e.password);
    pwEdit->setEchoMode(QLineEdit::Password);
    auto *showPw     = new QCheckBox("Show password");
    QObject::connect(showPw, &QCheckBox::toggled, pwEdit, [pwEdit](bool s){
        pwEdit->setEchoMode(s ? QLineEdit::Normal : QLineEdit::Password);
    });
    auto *odCheck    = new QCheckBox("On-demand (connect only when listeners present)");
    odCheck->setChecked(e.onDemand);
    auto *icyCheck   = new QCheckBox("Relay ICY metadata from source");
    icyCheck->setChecked(e.icyMeta);
    auto *intSpin    = mkSpin(0, 86400, e.interval, " s");
    intSpin->setToolTip("Poll interval for on-demand relay (0 = server default)");

    fl->addRow("Local mount:",     localEdit);
    fl->addRow("Remote server:",   srvEdit);
    fl->addRow("Remote port:",     portSpin);
    fl->addRow("Remote mount:",    mntEdit);
    fl->addRow("Connection:",      sslCombo);
    fl->addRow("Username:",        userEdit);
    fl->addRow("Password:",        pwEdit);
    fl->addRow("",                 showPw);
    fl->addRow("",                 odCheck);
    fl->addRow("",                 icyCheck);
    fl->addRow("Poll interval:",   intSpin);

    auto *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    auto *mainLay = new QVBoxLayout(&dlg);
    mainLay->addLayout(fl);
    mainLay->addWidget(bb);

    if (dlg.exec() != QDialog::Accepted) return false;

    e.localMount   = localEdit->text().trimmed();
    e.remoteServer = srvEdit->text().trimmed();
    e.remotePort   = portSpin->value();
    e.remoteMount  = mntEdit->text().trimmed();
    e.ssl          = sslCombo->currentIndex();
    e.username     = userEdit->text();
    e.password     = pwEdit->text();
    e.onDemand     = odCheck->isChecked() ? 1 : 0;
    e.icyMeta      = icyCheck->isChecked() ? 1 : 0;
    e.interval     = intSpin->value();
    return true;
}

void ConfigDialog::onAddRelay()
{
    RelayEntry e;
    if (!editRelayDialog(e, this)) return;
    m_relays.append(e); refreshRelayList();
    m_relayList->setCurrentRow(m_relays.size() - 1);
}

void ConfigDialog::onEditRelay()
{
    int idx = m_relayList->currentRow();
    if (idx < 0 || idx >= m_relays.size()) return;
    RelayEntry e = m_relays[idx];
    if (!editRelayDialog(e, this)) return;
    m_relays[idx] = e; refreshRelayList();
    m_relayList->setCurrentRow(idx);
}

void ConfigDialog::onRemoveRelay()
{
    int idx = m_relayList->currentRow();
    if (idx < 0 || idx >= m_relays.size()) return;
    m_relays.removeAt(idx); refreshRelayList();
}


// ══════════════════════════════════════════════════════════════════════════════
// HTTP Header Add / Remove
// ══════════════════════════════════════════════════════════════════════════════

void ConfigDialog::onAddHeader()
{
    int r = m_headerTable->rowCount(); m_headerTable->insertRow(r);
    m_headerTable->setItem(r, 0, new QTableWidgetItem("X-Custom-Header"));
    m_headerTable->setItem(r, 1, new QTableWidgetItem("value"));
    m_headerTable->editItem(m_headerTable->item(r, 0));
}

void ConfigDialog::onRemoveHeader()
{
    int r = m_headerTable->currentRow();
    if (r >= 0) m_headerTable->removeRow(r);
}

void ConfigDialog::onRestoreCorsDefaults()
{
    m_headerTable->setRowCount(0);
    auto addRow = [this](const QString &n, const QString &v) {
        int r = m_headerTable->rowCount(); m_headerTable->insertRow(r);
        m_headerTable->setItem(r, 0, new QTableWidgetItem(n));
        m_headerTable->setItem(r, 1, new QTableWidgetItem(v));
    };
    addRow("Access-Control-Allow-Origin",  "*");
    addRow("Access-Control-Allow-Methods", "GET, OPTIONS");
    addRow("Access-Control-Allow-Headers",
           "Origin, X-Requested-With, Content-Type, Accept, Icy-MetaData");
    m_headersGroup->setChecked(true);
}


// ══════════════════════════════════════════════════════════════════════════════
// Data collection helpers
// ══════════════════════════════════════════════════════════════════════════════

QList<ListenEntry> ConfigDialog::collectListeners() const
{
    QList<ListenEntry> out;
    for (int r = 0; r < m_listenTable->rowCount(); ++r) {
        ListenEntry e;
        if (auto *it = m_listenTable->item(r,0)) e.port        = it->text().toInt();
        if (auto *it = m_listenTable->item(r,1)) e.bindAddress = it->text();
        if (auto *it = m_listenTable->item(r,2)) e.ssl         = it->text().toInt();
        out.append(e);
    }
    return out;
}

QList<HttpHeaderEntry> ConfigDialog::collectHeaders() const
{
    // Respect the group checkbox — unchecked means "no http-headers section".
    if (!m_headersGroup || !m_headersGroup->isChecked())
        return {};

    QList<HttpHeaderEntry> out;
    for (int r = 0; r < m_headerTable->rowCount(); ++r) {
        HttpHeaderEntry h;
        if (auto *it = m_headerTable->item(r,0)) h.name  = it->text().trimmed();
        if (auto *it = m_headerTable->item(r,1)) h.value = it->text().trimmed();
        if (!h.name.isEmpty()) out.append(h);
    }
    return out;
}


// ══════════════════════════════════════════════════════════════════════════════
// Format detection
// ══════════════════════════════════════════════════════════════════════════════

ConfigDialog::ConfigFormat ConfigDialog::detectFormat(const QString &path) const
{
    if (path.isEmpty()) return ConfigFormat::Yaml;
    // Extension check first
    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "xml") return ConfigFormat::Xml;
    if (ext == "yaml" || ext == "yml") return ConfigFormat::Yaml;
    // Fall back to byte sniff (same logic as cfgfile.c detect_config_format)
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        char buf[4] = {0};
        f.read(buf, 4);
        if (buf[0] == '<') return ConfigFormat::Xml;
    }
    return ConfigFormat::Yaml;
}


// ══════════════════════════════════════════════════════════════════════════════
// YAML generator — produces a complete, well-commented mcaster1dnas.yaml
// ══════════════════════════════════════════════════════════════════════════════

static QString yq(const QString &s)   // YAML-quoted string
{
    return "\"" + QString(s).replace("\\","\\\\").replace("\"","\\\"") + "\"";
}
static QString yb(bool b) { return b ? "true" : "false"; }

QString ConfigDialog::generateYaml() const
{
    QString out;
    QTextStream ts(&out);

    ts << "---\n";
    ts << "# Mcaster1DNAS Configuration\n";
    ts << "# Saved by Configuration Manager — " << QDate::currentDate().toString("yyyy-MM-dd") << "\n\n";

    ts << "location:  " << yq(m_location->text()) << "\n";
    ts << "admin:     " << yq(m_admin->text())    << "\n";
    ts << "hostname:  " << yq(m_hostname->text()) << "\n";
    if (!m_serverID->text().isEmpty())
        ts << "server-id: " << yq(m_serverID->text()) << "\n";
    ts << "fileserve: " << yb(m_fileserve->isChecked()) << "\n";
    if (m_maxListeners->value() > 0)
        ts << "max-listeners: " << m_maxListeners->value() << "\n";

    ts << "\nauthentication:\n";
    ts << "  source-password: " << yq(m_sourcePw->text())  << "\n";
    ts << "  relay-password:  " << yq(m_relayPw->text())   << "\n";
    ts << "  admin-user:      " << yq(m_adminUser->text())  << "\n";
    ts << "  admin-password:  " << yq(m_adminPw->text())    << "\n";

    ts << "\nlimits:\n";
    ts << "  clients:            " << m_clientLimit->value() << "\n";
    ts << "  sources:            " << m_sourceLimit->value() << "\n";
    ts << "  workers:            " << m_workers->value()     << "\n";
    ts << "  queue-size:         " << m_queueSize->value()   << "\n";
    ts << "  burst-size:         " << m_burstSize->value()   << "\n";
    ts << "  client-timeout:     " << m_clientTO->value()    << "\n";
    ts << "  header-timeout:     " << m_headerTO->value()    << "\n";
    ts << "  source-timeout:     " << m_sourceTO->value()    << "\n";
    ts << "  song-history-limit: " << m_songHistory->value() << "\n";

    ts << "\nlisten-sockets:\n";
    for (const auto &l : collectListeners()) {
        ts << "  - port:         " << l.port << "\n";
        ts << "    bind-address: " << yq(l.bindAddress) << "\n";
        ts << "    ssl:          " << l.ssl  << "\n";
    }

    ts << "\npaths:\n";
    ts << "  basedir:         " << yq(m_baseDir->text())   << "\n";
    ts << "  logdir:          " << yq(m_logDir->text())    << "\n";
    ts << "  webroot:         " << yq(m_webRoot->text())   << "\n";
    ts << "  adminroot:       " << yq(m_adminRoot->text()) << "\n";
    ts << "  pidfile:         " << yq(m_pidFile->text())   << "\n";
    if (!m_sslCert->text().isEmpty())
        ts << "  ssl-certificate: " << yq(m_sslCert->text()) << "\n";

    // Preserve path aliases round-trip (/ → /status.xsl etc.)
    if (!m_aliases.isEmpty()) {
        ts << "\n  aliases:\n";
        for (const auto &a : m_aliases)
            ts << "    - source:      " << yq(a.source)      << "\n"
               << "      destination: " << yq(a.destination) << "\n";
    }

    ts << "\nlogging:\n";
    ts << "  accesslog:\n";
    ts << "    name:    " << yq(m_accessLog->text()) << "\n";
    ts << "    archive: " << yb(m_logArchive->isChecked()) << "\n";
    ts << "  errorlog:\n";
    ts << "    name:    " << yq(m_errorLog->text()) << "\n";
    ts << "    level:   " << yq(m_logLevel->currentText()) << "\n";
    ts << "    archive: " << yb(m_logArchive->isChecked()) << "\n";
    ts << "  playlistlog:\n";
    ts << "    name:    " << yq(m_playlistLog->text()) << "\n";
    ts << "    archive: " << yb(m_logArchive->isChecked()) << "\n";
    ts << "  logsize:    " << m_logSize->value() << "\n";
    ts << "  logarchive: " << yb(m_logArchive->isChecked()) << "\n";

    ts << "\nsecurity:\n";
    ts << "  chroot: " << yb(m_chroot->isChecked()) << "\n";
    if (!m_runUser->text().isEmpty())  ts << "  user:   " << yq(m_runUser->text())  << "\n";
    if (!m_runGroup->text().isEmpty()) ts << "  group:  " << yq(m_runGroup->text()) << "\n";

    if (!m_cipherList->text().isEmpty())
        ts << "\nssl-cipher-list: " << yq(m_cipherList->text()) << "\n";

    const auto headers = collectHeaders();
    if (!headers.isEmpty()) {
        ts << "\nhttp-headers:\n";
        for (const auto &h : headers)
            ts << "  - name:  " << yq(h.name)  << "\n"
               << "    value: " << yq(h.value) << "\n";
    }

    // Mounts
    if (!m_mounts.isEmpty()) {
        ts << "\nmounts:\n";
        for (const auto &mp : m_mounts) {
            ts << "  - mount-name:  " << yq(mp.mountName) << "\n";
            ts << "    mount-type:  " << yq(mp.mountType) << "\n";
            ts << "    max-listeners: " << mp.maxListeners << "\n";
            ts << "    public:      " << mp.isPublic << "\n";
            if (mp.hidden)     ts << "    hidden:      true\n";
            if (mp.noMount)    ts << "    no-mount:    true\n";
            if (mp.hijack)     ts << "    hijack:      true\n";
            if (!mp.streamName.isEmpty())
                ts << "    stream-name: " << yq(mp.streamName) << "\n";
            if (!mp.streamDescription.isEmpty())
                ts << "    stream-description: " << yq(mp.streamDescription) << "\n";
            if (!mp.streamUrl.isEmpty())
                ts << "    stream-url:  " << yq(mp.streamUrl) << "\n";
            if (!mp.genre.isEmpty())
                ts << "    genre:       " << yq(mp.genre) << "\n";
            if (!mp.bitrate.isEmpty())
                ts << "    bitrate:     " << yq(mp.bitrate) << "\n";
            if (!mp.mimeType.isEmpty())
                ts << "    type:        " << yq(mp.mimeType) << "\n";
            if (!mp.username.isEmpty())
                ts << "    username:    " << yq(mp.username) << "\n";
            if (!mp.password.isEmpty())
                ts << "    password:    " << yq(mp.password) << "\n";
            if (!mp.fallbackMount.isEmpty()) {
                ts << "    fallback:    " << yq(mp.fallbackMount) << "\n";
                if (mp.fallbackWhenFull)
                    ts << "    fallback-when-full: true\n";
            }
            if (mp.onDemand)      ts << "    on-demand:   true\n";
            if (mp.skipAccessLog) ts << "    skip-access-log: true\n";
            if (!mp.allowChunked) ts << "    allow-chunked: false\n";
            if (mp.burstSize)
                ts << "    burst-size:  " << mp.burstSize << "\n";
            if (mp.waitTime)
                ts << "    wait-time:   " << mp.waitTime  << "\n";
            if (mp.lingerDuration)
                ts << "    linger-duration: " << mp.lingerDuration << "\n";
            if (mp.maxListenerDuration)
                ts << "    max-listener-duration: " << mp.maxListenerDuration << "\n";
            if (mp.maxStreamDuration)
                ts << "    max-stream-duration: " << mp.maxStreamDuration << "\n";
            for (const auto &kv : mp.extraMeta)
                ts << "    " << kv.first << ": " << yq(kv.second) << "\n";
            ts << "\n";
        }
    }

    // Relays
    if (!m_relays.isEmpty()) {
        ts << "\nrelays:\n";
        for (const auto &r : m_relays) {
            ts << "  - local-mount:  " << yq(r.localMount)   << "\n";
            ts << "    server:       " << yq(r.remoteServer)  << "\n";
            ts << "    port:         " << r.remotePort         << "\n";
            ts << "    mount:        " << yq(r.remoteMount)    << "\n";
            if (r.ssl)     ts << "    ssl:          " << r.ssl << "\n";
            if (!r.username.isEmpty())
                ts << "    username:     " << yq(r.username) << "\n";
            if (!r.password.isEmpty())
                ts << "    password:     " << yq(r.password) << "\n";
            if (r.onDemand)  ts << "    on-demand:    true\n";
            if (r.icyMeta)   ts << "    relay-shoutcast-metadata: true\n";
            if (r.interval)  ts << "    interval:     " << r.interval << "\n";
            ts << "\n";
        }
    }

    return out;
}


// ══════════════════════════════════════════════════════════════════════════════
// XML generator — Icecast2-compatible format
// ══════════════════════════════════════════════════════════════════════════════

QString ConfigDialog::generateXml() const
{
    QString out;
    QXmlStreamWriter xml(&out);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(4);
    xml.writeStartDocument();
    xml.writeComment(" Mcaster1DNAS Configuration — saved by Configuration Manager ");

    xml.writeStartElement("icecast");

    xml.writeTextElement("location",   m_location->text());
    xml.writeTextElement("admin",      m_admin->text());
    xml.writeTextElement("hostname",   m_hostname->text());
    if (!m_serverID->text().isEmpty())
        xml.writeTextElement("server-id", m_serverID->text());
    xml.writeTextElement("fileserve",  m_fileserve->isChecked() ? "1" : "0");
    if (m_maxListeners->value() > 0)
        xml.writeTextElement("max-listeners", QString::number(m_maxListeners->value()));

    xml.writeStartElement("authentication");
    xml.writeTextElement("source-password", m_sourcePw->text());
    xml.writeTextElement("relay-password",  m_relayPw->text());
    xml.writeTextElement("admin-user",      m_adminUser->text());
    xml.writeTextElement("admin-password",  m_adminPw->text());
    xml.writeEndElement();

    xml.writeStartElement("limits");
    xml.writeTextElement("clients",            QString::number(m_clientLimit->value()));
    xml.writeTextElement("sources",            QString::number(m_sourceLimit->value()));
    xml.writeTextElement("workers",            QString::number(m_workers->value()));
    xml.writeTextElement("queue-size",         QString::number(m_queueSize->value()));
    xml.writeTextElement("burst-size",         QString::number(m_burstSize->value()));
    xml.writeTextElement("client-timeout",     QString::number(m_clientTO->value()));
    xml.writeTextElement("header-timeout",     QString::number(m_headerTO->value()));
    xml.writeTextElement("source-timeout",     QString::number(m_sourceTO->value()));
    xml.writeTextElement("song-history-limit", QString::number(m_songHistory->value()));
    xml.writeEndElement();

    for (const auto &l : collectListeners()) {
        xml.writeStartElement("listen-socket");
        xml.writeTextElement("port",         QString::number(l.port));
        xml.writeTextElement("bind-address", l.bindAddress);
        xml.writeTextElement("ssl",          QString::number(l.ssl));
        xml.writeEndElement();
    }

    xml.writeStartElement("paths");
    xml.writeTextElement("basedir",         m_baseDir->text());
    xml.writeTextElement("logdir",          m_logDir->text());
    xml.writeTextElement("webroot",         m_webRoot->text());
    xml.writeTextElement("adminroot",       m_adminRoot->text());
    xml.writeTextElement("pidfile",         m_pidFile->text());
    if (!m_sslCert->text().isEmpty())
        xml.writeTextElement("ssl-certificate", m_sslCert->text());
    // Preserve path aliases round-trip
    for (const auto &a : m_aliases) {
        xml.writeStartElement("alias");
        xml.writeTextElement("source",      a.source);
        xml.writeTextElement("destination", a.destination);
        xml.writeEndElement();
    }
    xml.writeEndElement(); // paths

    xml.writeStartElement("logging");
    xml.writeStartElement("accesslog");
    xml.writeTextElement("name",    m_accessLog->text());
    xml.writeTextElement("archive", m_logArchive->isChecked() ? "1" : "0");
    xml.writeEndElement();
    xml.writeStartElement("errorlog");
    xml.writeTextElement("name",    m_errorLog->text());
    xml.writeTextElement("level",   m_logLevel->currentText());
    xml.writeTextElement("archive", m_logArchive->isChecked() ? "1" : "0");
    xml.writeEndElement();
    xml.writeStartElement("playlistlog");
    xml.writeTextElement("name",    m_playlistLog->text());
    xml.writeTextElement("archive", m_logArchive->isChecked() ? "1" : "0");
    xml.writeEndElement();
    xml.writeTextElement("logsize", QString::number(m_logSize->value()));
    xml.writeEndElement(); // logging

    xml.writeStartElement("security");
    xml.writeTextElement("chroot", m_chroot->isChecked() ? "1" : "0");
    if (!m_runUser->text().isEmpty())  xml.writeTextElement("user",  m_runUser->text());
    if (!m_runGroup->text().isEmpty()) xml.writeTextElement("group", m_runGroup->text());
    xml.writeEndElement();

    if (!m_cipherList->text().isEmpty())
        xml.writeTextElement("ssl-cipher-list", m_cipherList->text());

    for (const auto &h : collectHeaders()) {
        xml.writeStartElement("http-headers");
        xml.writeTextElement("header",
            "<name>" + h.name.toHtmlEscaped() + "</name>"
            "<value>" + h.value.toHtmlEscaped() + "</value>");
        xml.writeEndElement();
    }

    for (const auto &mp : m_mounts) {
        xml.writeStartElement("mount");
        xml.writeTextElement("mount-name",   mp.mountName);
        if (!mp.mountType.isEmpty())
            xml.writeTextElement("mount-type", mp.mountType);
        if (mp.maxListeners >= 0)
            xml.writeTextElement("max-listeners", QString::number(mp.maxListeners));
        xml.writeTextElement("public",       QString::number(mp.isPublic));
        if (mp.hidden)   xml.writeTextElement("hidden",   "1");
        if (mp.noMount)  xml.writeTextElement("no-mount", "1");
        if (mp.hijack)   xml.writeTextElement("hijack",   "1");
        if (!mp.streamName.isEmpty())
            xml.writeTextElement("stream-name", mp.streamName);
        if (!mp.streamDescription.isEmpty())
            xml.writeTextElement("stream-description", mp.streamDescription);
        if (!mp.streamUrl.isEmpty())
            xml.writeTextElement("stream-url", mp.streamUrl);
        if (!mp.genre.isEmpty())    xml.writeTextElement("genre",   mp.genre);
        if (!mp.bitrate.isEmpty())  xml.writeTextElement("bitrate", mp.bitrate);
        if (!mp.mimeType.isEmpty()) xml.writeTextElement("type",    mp.mimeType);
        if (!mp.username.isEmpty()) xml.writeTextElement("username",mp.username);
        if (!mp.password.isEmpty()) xml.writeTextElement("password",mp.password);
        if (!mp.fallbackMount.isEmpty()) {
            xml.writeStartElement("fallback");
            xml.writeTextElement("mount", mp.fallbackMount);
            if (mp.fallbackWhenFull)
                xml.writeTextElement("fallback-when-full", "1");
            xml.writeEndElement();
        }
        if (mp.onDemand)      xml.writeTextElement("on-demand",   "1");
        if (mp.skipAccessLog) xml.writeTextElement("skip-access-log", "1");
        if (!mp.allowChunked) xml.writeTextElement("allow-chunked","0");
        if (mp.burstSize)     xml.writeTextElement("burst-size",  QString::number(mp.burstSize));
        if (mp.waitTime)      xml.writeTextElement("wait-time",   QString::number(mp.waitTime));
        if (mp.lingerDuration)
            xml.writeTextElement("linger-duration",       QString::number(mp.lingerDuration));
        if (mp.maxListenerDuration)
            xml.writeTextElement("max-listener-duration", QString::number(mp.maxListenerDuration));
        if (mp.maxStreamDuration)
            xml.writeTextElement("max-stream-duration",   QString::number(mp.maxStreamDuration));
        for (const auto &kv : mp.extraMeta)
            xml.writeTextElement(kv.first, kv.second);
        xml.writeEndElement(); // mount
    }

    for (const auto &r : m_relays) {
        xml.writeStartElement("relay");
        xml.writeTextElement("local-mount", r.localMount);
        xml.writeTextElement("server",      r.remoteServer);
        xml.writeTextElement("port",        QString::number(r.remotePort));
        xml.writeTextElement("mount",       r.remoteMount);
        if (r.ssl)     xml.writeTextElement("ssl",      QString::number(r.ssl));
        if (!r.username.isEmpty()) xml.writeTextElement("username", r.username);
        if (!r.password.isEmpty()) xml.writeTextElement("password", r.password);
        if (r.onDemand)  xml.writeTextElement("on-demand", "1");
        if (!r.icyMeta)  xml.writeTextElement("relay-shoutcast-metadata", "0");
        if (r.interval)  xml.writeTextElement("interval", QString::number(r.interval));
        xml.writeEndElement(); // relay
    }

    xml.writeEndElement(); // icecast
    xml.writeEndDocument();
    return out;
}


// ══════════════════════════════════════════════════════════════════════════════
// Save logic
// ══════════════════════════════════════════════════════════════════════════════

bool ConfigDialog::writeToPath(const QString &path, const QString &content)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Save Failed",
            "Could not open file for writing:\n" + path + "\n\n" + f.errorString());
        return false;
    }
    QTextStream ts(&f);
    ts << content;
    return true;
}

void ConfigDialog::onSave()
{
    if (m_configPath.isEmpty()) { onSaveYaml(); return; }

    // ── Pre-save validation ──────────────────────────────────────────────────
    if (m_hostname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
            "Hostname cannot be empty.\nPlease enter a hostname before saving.");
        return;
    }
    if (collectListeners().isEmpty()) {
        QMessageBox::warning(this, "Validation Error",
            "At least one listen socket is required.\n"
            "Add a port entry in the Listen Sockets table.");
        return;
    }
    if (m_mounts.isEmpty()) {
        auto ans = QMessageBox::question(this, "No Mounts Defined",
            "No mount points are configured.\n\n"
            "The server will accept source connections on any mountpoint.\n"
            "Save anyway?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ans != QMessageBox::Yes) return;
    }

    bool isXml = (detectFormat(m_configPath) == ConfigFormat::Xml) || m_fmtXml->isChecked();
    QString content = isXml ? generateXml() : generateYaml();
    if (writeToPath(m_configPath, content)) {
        qDebug() << "ConfigDialog: saved config to" << m_configPath
                 << "(format:" << (isXml ? "XML" : "YAML") << ")";
        QMessageBox::information(this, "Saved",
            QString("Config saved (%1):\n%2").arg(isXml ? "XML" : "YAML", m_configPath));
        emit savedConfig(m_configPath);
    }
}

void ConfigDialog::onSaveYaml()
{
    QString def = m_configPath.isEmpty()
        ? "mcaster1dnas.yaml"
        : QFileInfo(m_configPath).dir().filePath(
              QFileInfo(m_configPath).completeBaseName() + ".yaml");

    QString path = QFileDialog::getSaveFileName(this, "Save as YAML", def,
        "YAML Config (*.yaml *.yml);;All Files (*)");
    if (path.isEmpty()) return;
    if (writeToPath(path, generateYaml())) {
        m_configPath = path;
        m_fmtYaml->setChecked(true);
        updateConfigLabel();
        QMessageBox::information(this, "Saved", "YAML config saved:\n" + path);
        emit savedConfig(path);
    }
}

void ConfigDialog::onSaveXml()
{
    QString def = m_configPath.isEmpty()
        ? "mcaster1dnas.xml"
        : QFileInfo(m_configPath).dir().filePath(
              QFileInfo(m_configPath).completeBaseName() + ".xml");

    QString path = QFileDialog::getSaveFileName(this, "Save as XML", def,
        "XML Config (*.xml);;All Files (*)");
    if (path.isEmpty()) return;
    if (writeToPath(path, generateXml())) {
        m_configPath = path;
        m_fmtXml->setChecked(true);
        updateConfigLabel();
        QMessageBox::information(this, "Saved", "XML config saved:\n" + path);
        emit savedConfig(path);
    }
}
