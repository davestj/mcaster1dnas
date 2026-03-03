/* mcaster1_subsys.c - Windows GUI/Service subsystem init/shutdown
 *
 * Provides _initialize_subsystems() and _shutdown_subsystems() for the
 * Mcaster1Win MFC GUI and Mcaster1Service Windows Service projects.
 *
 * These mirror initialize_subsystems() / shutdown_subsystems() in src/main.c
 * but are separate so main.c (which contains main()) is not linked into the
 * GUI/Service builds.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "log/log.h"
#include "logging.h"
#include "thread/thread.h"
#include "net/sock.h"
#include "net/resolver.h"

#include "global.h"
#include "cfgfile.h"
#include "connection.h"
#include "refbuf.h"
#include "stats.h"
#include "xslt.h"
#include "slave.h"

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif


void _initialize_subsystems (void)
{
    init_log_subsys ();
    thread_initialize ();
    global_initialize ();
    sock_initialize ();
    resolver_initialize ();
    config_initialize ();
    connection_initialize ();
    refbuf_initialize ();
    stats_initialize ();
    xslt_initialize ();
#ifdef HAVE_CURL_GLOBAL_INIT
    curl_global_init (CURL_GLOBAL_ALL);
#endif
}


void _shutdown_subsystems (void)
{
    connection_shutdown ();
    slave_shutdown ();
    xslt_shutdown ();

    config_shutdown ();
    refbuf_shutdown ();
    resolver_shutdown ();
    sock_shutdown ();

#ifdef HAVE_CURL
    curl_global_cleanup ();
#endif

    log_shutdown ();
    global_shutdown ();
    thread_shutdown ();
}
