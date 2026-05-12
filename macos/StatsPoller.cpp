/*
 * StatsPoller.cpp — calls stats_get_xml() in-process every 5 s.
 *
 * stats_get_xml() is declared in src/stats.h and compiled into this binary.
 * Its XML output looks like:
 *
 *   <icestats>
 *     <source mount="/live">
 *       <listeners>3</listeners>
 *       <title>Song Title</title>
 *       ...
 *     </source>
 *     <clients>3</clients>
 *     <sources>1</sources>
 *     ...
 *   </icestats>
 */

#include "StatsPoller.h"

#include <QDomDocument>
#include <QTimer>
#include <QDebug>

extern "C" {
#include "server_bridge.h" // config.h + net/sock.h (defines IOVEC for connection.h)
#include "global.h"
#include "stats.h"
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
}

static QString xmlContent(xmlDocPtr doc)
{
    // Dump the libxml2 doc to an in-memory buffer and convert to QString
    xmlChar *buf = nullptr;
    int      size = 0;
    xmlDocDumpMemory(doc, &buf, &size);
    QString result = QString::fromUtf8(reinterpret_cast<const char *>(buf), size);
    xmlFree(buf);
    return result;
}

StatsPoller::StatsPoller(QObject *parent)
    : QObject(parent)
{
}

void StatsPoller::startPolling()
{
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(5000);
        connect(m_timer, &QTimer::timeout, this, &StatsPoller::poll);
    }
    m_timer->start();
    poll();  // immediate first poll
}

void StatsPoller::stopPolling()
{
    if (m_timer)
        m_timer->stop();
}

void StatsPoller::poll()
{
    if (global.running != MC_RUNNING)
        return;

    xmlDocPtr doc = stats_get_xml(STATS_PUBLIC, nullptr);
    if (!doc)
        return;

    // Parse via Qt's DOM (avoids libxml2 tree-walking in Qt code)
    QString xml = xmlContent(doc);
    xmlFreeDoc(doc);

    QDomDocument dom;
    if (!dom.setContent(xml))
        return;

    GlobalStats  gs;
    QList<SourceStats> sources;

    QDomElement root = dom.documentElement();  // <icestats>
    QDomNodeList children = root.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        QDomElement el = children.at(i).toElement();
        if (el.isNull()) continue;

        if (el.tagName() == "source") {
            SourceStats ss;
            ss.mount = el.attribute("mount", "(unknown)");
            QDomNodeList fields = el.childNodes();
            for (int j = 0; j < fields.count(); ++j) {
                QDomElement f = fields.at(j).toElement();
                if (!f.isNull())
                    ss.fields[f.tagName()] = f.text();
            }
            sources.append(ss);
        } else {
            // global fields
            QString tag = el.tagName();
            QString val = el.text();
            if      (tag == "clients")  gs.clients   = val.toInt();
            else if (tag == "sources")  gs.sources   = val.toInt();
            else if (tag == "listeners") gs.listeners = val.toInt();
            else if (tag == "server_start") gs.serverStart = val;
            else gs.extra[tag] = val;
        }
    }

    emit statsUpdated(gs, sources);
}
