#pragma once
/*
 * ConfigDialog — Phase 3: Complete Configuration Manager
 *
 * 6 tabs:
 *   [Global]  [Mounts]  [Relays]  [System]  [Security]  [SSL Cert*]
 *                                                          *Phase 4
 *
 * Reads the current config into ALL form fields via:
 *   - config_get_config_unlocked()  if the server is running
 *   - config_parse_file()           if the server is stopped
 *     (safe: config_parse_file operates on the provided mc_config_t*
 *      without requiring global config_initialize() locks)
 *
 * Bottom row:
 *   [Save]  [Save as YAML…]  [Save as XML…]  [Close]
 *
 *   [Save] writes back to the current config file in the same format
 *   (detected from extension, or overridable via the format selector).
 *   savedConfig(path) is emitted after every successful write.
 */

#include <QDialog>
#include <QString>
#include <QList>
#include <QPair>

// ── Forward declarations ──────────────────────────────────────────────────────
struct mc_config_tag;
struct _listener_t;
struct _mount_proxy;
struct _relay_server;
struct _config_http_header_tag;
struct _avl_tree;   // for mounts_tree AVL iteration
class QTabWidget;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QTableWidget;
class QListWidget;
class QLabel;
class QRadioButton;
class QGroupBox;

// ══════════════════════════════════════════════════════════════════════════════
// Lightweight in-memory data carriers
// ══════════════════════════════════════════════════════════════════════════════

struct ListenEntry {
    int     port        = 9330;
    QString bindAddress = "127.0.0.1";
    int     ssl         = 0;       // 0=plain, 1=ssl/tls, -1=auto-detect
};

struct MountEntry {
    // ── identity ──────────────────────────────────────────────────────────────
    QString mountName;
    QString mountType           = "live";  // live|podcast|ondemand|socialmedia

    // ── listeners ─────────────────────────────────────────────────────────────
    int     maxListeners        = -1;      // -1 = use global limit
    int     isPublic            = 0;
    int     hidden              = 0;

    // ── stream metadata ───────────────────────────────────────────────────────
    QString streamName;
    QString streamDescription;
    QString streamUrl;
    QString genre;
    QString bitrate;
    QString mimeType;
    QString mimeSubtype;

    // ── source authentication ─────────────────────────────────────────────────
    QString username;
    QString password;

    // ── fallback / on-demand ──────────────────────────────────────────────────
    QString fallbackMount;
    int     fallbackWhenFull    = 0;
    int     onDemand            = 0;

    // ── advanced ──────────────────────────────────────────────────────────────
    uint32_t burstSize          = 0;       // 0 = use global
    int     waitTime            = 0;       // secs after source disconnect
    int     lingerDuration      = 0;
    int     skipAccessLog       = 0;
    int     allowChunked        = 1;
    int     maxListenerDuration = 0;       // 0 = unlimited
    int     maxStreamDuration   = 0;       // 0 = unlimited
    int     noMount             = 0;       // disallow direct requests
    int     hijack              = 0;

    // ── ICY2 extra metadata (icy-dj-handle, icy-show-title, etc.) ─────────────
    QList<QPair<QString,QString>> extraMeta;
};

struct RelayEntry {
    QString localMount;
    QString remoteServer;
    int     remotePort          = 8000;
    QString remoteMount         = "/";
    int     ssl                 = 0;
    QString username;
    QString password;
    int     onDemand            = 0;
    int     interval            = 0;   // poll interval when on-demand (secs)
    int     icyMeta             = 1;   // relay ICY metadata
};

struct HttpHeaderEntry {
    QString name;
    QString value;
};

// Alias entry — source → destination path mapping (e.g. "/" → "/status.xsl")
// No UI editor; just preserved on round-trip so user edits are not lost.
struct AliasEntry {
    QString source;
    QString destination;
};

// ══════════════════════════════════════════════════════════════════════════════
// ConfigDialog
// ══════════════════════════════════════════════════════════════════════════════

class ConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigDialog(const QString &configPath,
                          bool serverRunning,
                          QWidget *parent = nullptr);

signals:
    void savedConfig(const QString &path);

private slots:
    void onSave();
    void onSaveYaml();
    void onSaveXml();

    void onAddListener();
    void onRemoveListener();

    void onAddMount();
    void onEditMount();
    void onDuplicateMount();
    void onRemoveMount();

    void onAddRelay();
    void onEditRelay();
    void onRemoveRelay();

    void onAddHeader();
    void onRemoveHeader();
    void onRestoreCorsDefaults();

private:
    // ── tab builders ─────────────────────────────────────────────────────────
    QWidget *buildGlobalTab();
    QWidget *buildMountsTab();
    QWidget *buildRelaysTab();
    QWidget *buildSystemTab();
    QWidget *buildSecurityTab();
    QWidget *buildSslCertTab();

    // ── config loading ────────────────────────────────────────────────────────
    void populate(const mc_config_tag *cfg);
    void loadFromFile(const QString &path);   // calls config_parse_file()
    void populateListeners(const _listener_t *head);
    void populateMounts(const _mount_proxy *head);      // wildcard mounts (linked list)
    void populateMountsTree(const _avl_tree *tree);     // named static mounts (AVL tree)
    void populateRelays(const _relay_server *head);     // traverses via new_details
    void populateHeaders(const _config_http_header_tag *head);

    // ── data collection ───────────────────────────────────────────────────────
    QList<ListenEntry>      collectListeners()  const;
    QList<HttpHeaderEntry>  collectHeaders()    const;

    // ── generators ────────────────────────────────────────────────────────────
    QString generateYaml() const;
    QString generateXml()  const;
    bool    writeToPath(const QString &path, const QString &content);

    // ── format detection ──────────────────────────────────────────────────────
    enum class ConfigFormat { Yaml, Xml, Unknown };
    ConfigFormat detectFormat(const QString &path) const;

    // ── helpers ───────────────────────────────────────────────────────────────
    void refreshMountList();
    void refreshRelayList();
    void updateConfigLabel();

    // ══════════════════════════════════════════════════════════════════════════
    // Global tab widgets
    // ══════════════════════════════════════════════════════════════════════════
    QLineEdit    *m_hostname     = nullptr;
    QLineEdit    *m_location     = nullptr;
    QLineEdit    *m_admin        = nullptr;
    QLineEdit    *m_serverID     = nullptr;
    QCheckBox    *m_fileserve    = nullptr;
    QSpinBox     *m_maxListeners = nullptr;

    QTableWidget *m_listenTable  = nullptr;

    QLineEdit    *m_sourcePw     = nullptr;
    QLineEdit    *m_relayPw      = nullptr;
    QLineEdit    *m_adminUser    = nullptr;
    QLineEdit    *m_adminPw      = nullptr;

    // ══════════════════════════════════════════════════════════════════════════
    // System tab widgets
    // ══════════════════════════════════════════════════════════════════════════
    QSpinBox  *m_clientLimit    = nullptr;
    QSpinBox  *m_sourceLimit    = nullptr;
    QSpinBox  *m_workers        = nullptr;
    QSpinBox  *m_queueSize      = nullptr;
    QSpinBox  *m_burstSize      = nullptr;
    QSpinBox  *m_clientTO       = nullptr;
    QSpinBox  *m_headerTO       = nullptr;
    QSpinBox  *m_sourceTO       = nullptr;
    QSpinBox  *m_songHistory    = nullptr;

    QLineEdit *m_baseDir        = nullptr;
    QLineEdit *m_logDir         = nullptr;
    QLineEdit *m_webRoot        = nullptr;
    QLineEdit *m_adminRoot      = nullptr;
    QLineEdit *m_pidFile        = nullptr;

    QLineEdit *m_accessLog      = nullptr;
    QLineEdit *m_errorLog       = nullptr;
    QLineEdit *m_playlistLog    = nullptr;
    QComboBox *m_logLevel       = nullptr;
    QSpinBox  *m_logSize        = nullptr;
    QCheckBox *m_logArchive     = nullptr;

    // ══════════════════════════════════════════════════════════════════════════
    // Security tab widgets
    // ══════════════════════════════════════════════════════════════════════════
    QCheckBox *m_chroot         = nullptr;
    QLineEdit *m_chrootDir      = nullptr;
    QLineEdit *m_runUser        = nullptr;
    QLineEdit *m_runGroup       = nullptr;

    QLineEdit *m_sslCert        = nullptr;
    QLineEdit *m_sslKey         = nullptr;
    QLineEdit *m_sslCa          = nullptr;
    QLineEdit *m_cipherList     = nullptr;

    QTableWidget *m_headerTable  = nullptr;   // HTTP headers
    QGroupBox    *m_headersGroup = nullptr;   // checkable — unchecked = no headers written

    // ══════════════════════════════════════════════════════════════════════════
    // Mount / Relay lists (data in m_mounts / m_relays)
    // ══════════════════════════════════════════════════════════════════════════
    QListWidget *m_mountList    = nullptr;
    QListWidget *m_relayList    = nullptr;
    QList<MountEntry>  m_mounts;
    QList<RelayEntry>  m_relays;
    QList<AliasEntry>  m_aliases;   // preserved from config — written back on save, no editor UI

    // ══════════════════════════════════════════════════════════════════════════
    // Bottom bar
    // ══════════════════════════════════════════════════════════════════════════
    QLabel        *m_configLabel  = nullptr;
    QRadioButton  *m_fmtYaml      = nullptr;
    QRadioButton  *m_fmtXml       = nullptr;

    // ══════════════════════════════════════════════════════════════════════════
    // State
    // ══════════════════════════════════════════════════════════════════════════
    QString       m_configPath;
    bool          m_serverRunning = false;
};
