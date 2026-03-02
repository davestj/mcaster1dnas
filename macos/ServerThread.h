#pragma once
/*
 * ServerThread — in-process server lifecycle manager.
 *
 * Mirrors the Windows MFC StartServer/_beginthread pattern:
 *   1. start()  → std::thread calls initialize_subsystems() + server_init() +
 *                  server_process() (blocks until server halts)
 *   2. stop()   → sets global.running = MC_HALTING; joins thread
 *
 * Communication with the Qt UI thread is via the shared mc_global_t struct
 * (global.running, global.sources, global.clients, global.listeners),
 * identical to how Windows MFC polls the same struct every 500 ms.
 */

#include <QObject>
#include <thread>
#include <atomic>
#include <string>

class ServerThread : public QObject
{
    Q_OBJECT
public:
    explicit ServerThread(QObject *parent = nullptr);
    ~ServerThread();

    // Start the server with the given config file path.
    // Returns false if already running.
    bool start(const std::string &configPath);

    // Signal the server to halt and join the thread.
    void stop();

    bool isRunning() const { return m_running.load(); }

signals:
    void serverStarted();
    void serverStopped();
    void serverError(const QString &message);

private:
    void threadFunc(std::string configPath);

    std::thread      m_thread;
    std::atomic_bool m_running{false};
};
