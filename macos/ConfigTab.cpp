/*
 * ConfigTab.cpp — mirrors Windows CConfigTab.
 * Loads the YAML/XML config file into a plain text editor.
 * Save button writes it back.  External editor button opens it in the system
 * default editor (TextEdit on macOS, gedit/kate/nano on Linux).
 */

#include "ConfigTab.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>

ConfigTab::ConfigTab(QWidget *parent)
    : QWidget(parent)
{
    m_edit = new QPlainTextEdit(this);
    m_edit->setFont(QFont("Monospace", 11));

    m_save = new QPushButton("Save Config", this);
    m_open = new QPushButton("Open in External Editor", this);

    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_save);
    btnRow->addWidget(m_open);
    btnRow->addStretch();

    auto *lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Configuration File (YAML/XML):", this));
    lay->addWidget(m_edit);
    lay->addLayout(btnRow);

    connect(m_save, &QPushButton::clicked, this, &ConfigTab::save);
    connect(m_open, &QPushButton::clicked, this, &ConfigTab::openInEditor);
    connect(m_edit, &QPlainTextEdit::modificationChanged,
            this, [this](bool mod){ m_modified = mod; });
}

void ConfigTab::setConfigPath(const QString &path)
{
    m_path = path;
    load();
}

void ConfigTab::load()
{
    if (m_path.isEmpty()) return;

    QFile f(m_path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_edit->setPlainText(QString("Cannot open: %1").arg(m_path));
        return;
    }
    QTextStream in(&f);
    m_edit->setPlainText(in.readAll());
    m_edit->document()->setModified(false);
    m_modified = false;
}

void ConfigTab::save()
{
    if (m_path.isEmpty()) {
        QMessageBox::warning(this, "Save", "No config file path set.");
        return;
    }

    QFile f(m_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Save Error",
            QString("Cannot write to %1:\n%2").arg(m_path, f.errorString()));
        return;
    }
    QTextStream out(&f);
    out << m_edit->toPlainText();
    m_edit->document()->setModified(false);
    m_modified = false;
    emit configSaved(m_path);
}

void ConfigTab::openInEditor()
{
    if (m_path.isEmpty()) return;
    // QDesktopServices::openUrl with a file:// URL invokes the system default
    // text editor — TextEdit on macOS, whatever xdg-open selects on Linux.
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_path));
}
