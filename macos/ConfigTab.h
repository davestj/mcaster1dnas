#pragma once
/*
 * ConfigTab — YAML / XML config file editor.
 * Phase 1: plain text editor with save-on-change.
 * Phase 3: replaced by full ConfigDialog wizard (tabs: Global, Mounts, Relays, Security).
 */

#include <QWidget>
#include <QPlainTextEdit>
#include <QString>
#include <QPushButton>

class ConfigTab : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigTab(QWidget *parent = nullptr);

    void setConfigPath(const QString &path);
    QString configPath() const { return m_path; }

public slots:
    void load();
    void save();
    void openInEditor();    // opens in default system text editor

signals:
    void configSaved(const QString &path);

private:
    QPlainTextEdit *m_edit   = nullptr;
    QPushButton    *m_save   = nullptr;
    QPushButton    *m_open   = nullptr;
    QString         m_path;
    bool            m_modified = false;
};
