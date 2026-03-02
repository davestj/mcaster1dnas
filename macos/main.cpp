/*
 * Mcaster1DNAS — macOS / Linux Qt6 GUI
 *
 * Entry point.  The server's main() is excluded from compilation via
 * -DMCASTER_BUILD_GUI (see src/main.c  #if !defined(MCASTER_BUILD_GUI)).
 * We call initialize_subsystems(), server_init(), server_process() ourselves
 * from a background thread, mirroring the Windows MFC _beginthread(StartServer)
 * pattern exactly.
 *
 * Pre-flight CLI handling:
 *   -h / --help         Print usage and exit (before Qt window opens)
 *   -v / --version      Print version and exit
 *   --ssl-gencert ...   Generate SSL certificate via ssl_gen_run() and exit
 *
 * Copyright 2025-2026, Saint John (David St John) <davestj@gmail.com>
 * License: GPL-2.0
 */

#include <cstdio>
#include <cstring>

extern "C" {
#include "server_bridge.h"
#include "config.h"
#include "ssl_gen.h"
}

#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"

static void print_usage(const char *prog)
{
    printf("Usage: %s [OPTIONS]\n"
           "\n"
           "Options:\n"
           "  -c <path>                         YAML or XML configuration file\n"
           "  -v, --version                     Print version string and exit\n"
           "  -h, --help                        Print this help and exit\n"
           "  --ssl-gencert                     Generate an SSL certificate and exit\n"
           "    --ssl-gencert-savepath=<dir>    Output directory (required)\n"
           "    --ssl-gentype=selfsigned|csr    Certificate type (default: selfsigned)\n"
           "    --subj=<subject>                Subject string (default: /CN=localhost)\n"
           "                                    e.g. \"/C=US/ST=TX/O=MyOrg/CN=localhost\"\n"
           "    --ssl-gencert-addtoconfig=true  Patch -c config with new cert paths\n"
           "\n"
           "Examples:\n"
           "  %s --ssl-gencert --ssl-gencert-savepath=./ssl\n"
           "  %s --ssl-gencert --ssl-gencert-savepath=./ssl --subj=/CN=myserver --ssl-gentype=csr\n"
           "\n"
           "When launched as a .app bundle, -c accepts an absolute path:\n"
           "  open Mcaster1DNAS.app --args -c /path/to/mcaster1dnas.yaml\n"
           "\n",
           prog, prog, prog);
}

int main(int argc, char *argv[])
{
    // ── Pre-flight: handle non-GUI flags BEFORE Qt initialises ──────────────
    // Qt's QApplication may consume args or change the environment; handle
    // exit-early flags here so they work even when there is no display.
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Mcaster1DNAS %s\n", PACKAGE_VERSION);
            return 0;
        }
        if (strcmp(argv[i], "--ssl-gencert") == 0) {
            ssl_gen_params_t p;
            memset(&p, 0, sizeof(p));
            if (ssl_gen_parse_args(argc, argv, &p)) {
                int rc = ssl_gen_run(&p);
                if (rc == 0)
                    printf("SSL certificate generated successfully.\n");
                else
                    fprintf(stderr, "ssl-gencert failed: %s\n"
                            "Use -h for usage.\n",
                            ssl_gen_last_error());
                return rc;
            }
            fprintf(stderr,
                "ssl-gencert: missing --ssl-gencert-savepath=<dir>.\n"
                "Use -h for usage.\n");
            return 1;
        }
    }

    // ── Qt application ───────────────────────────────────────────────────────
    QApplication app(argc, argv);

    app.setApplicationName("Mcaster1DNAS");
    app.setApplicationVersion(PACKAGE_VERSION);
    app.setOrganizationName("MediaCast1");
    app.setOrganizationDomain("mcaster1.com");

    // Use Fusion style for consistent cross-platform look
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow win(argc, argv);
    win.show();

    return app.exec();
}
