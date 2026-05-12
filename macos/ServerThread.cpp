/*
 * ServerThread.cpp — in-process server lifecycle.
 *
 * Calls the server C API directly (same process, shared global struct).
 * initialize_subsystems / server_init / server_process are declared in
 * src/global.h which is compiled into this binary.
 */

#include "ServerThread.h"

#include <QDebug>
#include <QFileInfo>
#include <QString>

#include <unistd.h>

extern "C" {
#include "server_bridge.h"   // config.h + net/sock.h (defines IOVEC, size_t via HAVE_* guards)
#include "global.h"          // MC_RUNNING, MC_HALTING, global, server_init, server_process
                              // initialize_subsystems, shutdown_subsystems
}

ServerThread::ServerThread(QObject *parent)
    : QObject(parent)
{
}

ServerThread::~ServerThread()
{
    stop();
}

bool ServerThread::start(const std::string &configPath)
{
    if (m_running.load())
        return false;

    // Join any stale thread from a previous self-terminated run.
    // m_thread.joinable() stays true after threadFunc() returns until
    // join() or detach() is called.  Assigning a new std::thread over a
    // joinable m_thread calls std::terminate() per the C++ standard —
    // the crash seen when server_init() fails and the user clicks Start again.
    if (m_thread.joinable())
        m_thread.join();

    m_running = true;
    m_thread = std::thread(&ServerThread::threadFunc, this, configPath);
    return true;
}

void ServerThread::stop()
{
    // Signal halt only if server is actually running.
    if (m_running.load())
        global.running = MC_HALTING;

    // Always join if joinable — even when m_running is already false.
    // threadFunc() sets m_running=false before returning, but m_thread
    // stays joinable() until join() is called.  Destroying a joinable
    // std::thread calls std::terminate() (C++ standard §thread.thread.destr),
    // which is the SIGABRT crash seen on app quit after a natural server stop.
    if (m_thread.joinable())
        m_thread.join();

    m_running = false;
}

void ServerThread::threadFunc(std::string configPath)
{
    // ── Step 1: chdir() to the config file's directory ────────────────────────
    // When launched as a .app bundle via Finder/Dock, macOS sets CWD to $HOME.
    // All paths in the YAML are relative (./ssl/localhost.pem, ./logs, ./web).
    // Without this chdir(), server_init() can't find the SSL cert, log dir, etc.
    // This mirrors Mcaster1Win.cpp's SetCurrentDirectoryA(exedir) call.
    {
        QString qConfig = QString::fromStdString(configPath);
        QString cfgDir  = QFileInfo(qConfig).absolutePath();
        if (!cfgDir.isEmpty()) {
            if (chdir(cfgDir.toLocal8Bit().constData()) != 0)
                qWarning() << "ServerThread: chdir to" << cfgDir << "failed";
        }
    }

    // ── Step 2: build argv for server_init() ──────────────────────────────────
    // Mirrors Windows _beginthread(StartServer): {progname, "-c", configFile}.
    // server_init() parses these args via config_parse_cmdline().
    const char *args[4];
    args[0] = "mcaster1";
    args[1] = "-c";
    args[2] = configPath.c_str();
    args[3] = nullptr;
    int argc = 3;

    initialize_subsystems();

    if (server_init(argc, const_cast<char **>(args)) == 0)
    {
        emit serverStarted();
        server_process();   // blocks until global.running == MC_HALTING
    }
    else
    {
        emit serverError("server_init() failed — check config file and logs.");
    }

    shutdown_subsystems();
    m_running = false;
    emit serverStopped();
}
