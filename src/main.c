/* Mcaster1DNAS - Digital Network Audio Server
 *
 * Based on Icecast-KH by Karl Heyes
 * Based on Icecast2 by Xiph.Org Foundation
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2025-2026, Saint John (David St John) <davestj@gmail.com>
 * Copyright 2010-2022, Karl Heyes <karl@kheyes.plus.com>
 * Copyright 2000-2004, Jack Moffitt <jack@xiph.org>,
 *                      Michael Smith <msmith@xiph.org>,
 *                      oddsock <oddsock@xiph.org>,
 *                      Karl Heyes <karl@xiph.org>
 *                      and others (see AUTHORS for details).
 */

/* -*- c-basic-offset: 4; indent-tabs-mode: nil; -*- */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#include <process.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef __APPLE__
# include <mach-o/dyld.h>   /* _NSGetExecutablePath */
#endif
#ifndef PATH_MAX
# define PATH_MAX 4096
#endif
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include <git_hash.h>

#include "thread/thread.h"
#include "avl/avl.h"
#include "net/sock.h"
#include "net/resolver.h"
#include "httpp/httpp.h"

#ifdef CHUID
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#endif
#ifdef HAVE_GETRLIMIT
#include <sys/resource.h>
#endif

#include "cfgfile.h"
#include "sighandler.h"

#ifdef HAVE_OPENSSL
#include "ssl_gen.h"
#endif

#include "global.h"
#include "compat.h"
#include "connection.h"
#include "refbuf.h"
#include "client.h"
#include "slave.h"
#include "stats.h"
#include "logging.h"
#include "xslt.h"
#include "fserve.h"
#include "auth.h"
#include "source.h"
#include "songdata_api.h"

#include <libxml/xmlmemory.h>

#undef CATMODULE
#define CATMODULE "main"

#if defined(WIN32) && defined(_DEBUG)
#define SI_TRACE(msg) do { FILE *_t=fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[si] " msg "\n");fclose(_t);} } while(0)
#else
#define SI_TRACE(msg) do {} while(0)
#endif

static void _ch_root_uid_setup(void);

static int background;
static char *pidfile = NULL;

#define _fatal_error fatal_error
void fatal_error (const char *perr)
{
#if defined(WIN32)
    /* MB_SERVICE_NOTIFICATION is invalid in non-service GUI processes on Win10/11.
     * Use MB_OK | MB_ICONERROR so the message actually appears. */
    MessageBox(NULL, perr, "Mcaster1DNAS Error", MB_OK | MB_ICONERROR);
#else
    ERROR1("%s", perr);
#endif
}

static void _print_usage(void)
{
    printf("%s\n\n", ICECAST_VERSION_STRING);
    printf("usage: mcaster1 [-b -v] [-c <file>]\n");
    printf("       mcaster1 --ssl-gencert [ssl options]\n\n");
    printf("Server options:\n");
    printf("\t-c <file>\tSpecify configuration file (YAML or XML)\n");
    printf("\t\t\tDefault search: <exedir>/mcaster1dnas.yaml, ./mcaster1dnas.yaml\n");
    printf("\t-v\t\tDisplay version info\n");
    printf("\t-b\t\tRun in background (Unix only)\n");
#ifdef HAVE_OPENSSL
    printf("\nSSL certificate generation (exits after completing):\n");
    printf("\t--ssl-gencert\t\t\t  Generate SSL certificates\n");
    printf("\t--ssl-gentype=selfsigned|csr\t  Output type (default: selfsigned)\n");
    printf("\t--subj=\"/C=US/ST=.../CN=host\"\t  X.509 subject fields\n");
    printf("\t--ssl-gencert-savepath=<dir>\t  Directory to save generated files\n");
    printf("\t--ssl-gencert-addtoconfig=true\t  Also update -c config with new cert paths\n");
#endif
    printf("\n");
}


/*
 * get_exe_dir() — cross-platform: return the directory containing the running exe.
 * buf is filled with a path ending in the platform path separator.
 */
static void get_exe_dir(char *buf, size_t len)
{
    buf[0] = '\0';
#if defined(_WIN32)
    {
        DWORD n = GetModuleFileNameA(NULL, buf, (DWORD)len);
        if (n > 0)
        {
            char *last = strrchr(buf, '\\');
            if (!last) last = strrchr(buf, '/');
            if (last) *(last + 1) = '\0';
        }
    }
#elif defined(__APPLE__)
    {
        uint32_t sz = (uint32_t)len;
        if (_NSGetExecutablePath(buf, &sz) == 0)
        {
            char *last = strrchr(buf, '/');
            if (last) *(last + 1) = '\0';
        }
    }
#else
    /* Linux: /proc/self/exe */
    {
        ssize_t r = readlink("/proc/self/exe", buf, len - 1);
        if (r > 0)
        {
            buf[r] = '\0';
            char *last = strrchr(buf, '/');
            if (last) *(last + 1) = '\0';
        }
    }
#endif
}


void initialize_subsystems(void)
{
    init_log_subsys ();
    thread_initialize();
    global_initialize();
    sock_initialize();
    resolver_initialize();
    config_initialize();
    connection_initialize();
    refbuf_initialize();

    stats_initialize();
    xslt_initialize();
#ifdef HAVE_CURL_GLOBAL_INIT
    curl_global_init (CURL_GLOBAL_ALL);
#endif
}


void shutdown_subsystems(void)
{
    connection_shutdown();
    slave_shutdown();
    xslt_shutdown();

    config_shutdown();
    refbuf_shutdown();
    resolver_shutdown();
    sock_shutdown();

#ifdef HAVE_CURL
    curl_global_cleanup();
#endif

    /* Now that these are done, we can stop the loggers. */
    log_shutdown();
    global_shutdown();
    thread_shutdown();
}

static int _parse_config_opts(int argc, char **argv, char *filename, int size)
{
    int i = 1;
    int config_ok = 0;

    background = 0;

#ifdef HAVE_OPENSSL
    /* Check for SSL cert generation before anything else.
     * If --ssl-gencert is present, generate and exit — no server startup. */
    {
        ssl_gen_params_t ssl_params;
        if (ssl_gen_parse_args(argc, argv, &ssl_params))
        {
            if (!ssl_params.savepath || !ssl_params.savepath[0])
            {
                fprintf(stderr, "ssl-gen: --ssl-gencert-savepath=<dir> is required\n");
                exit(1);
            }
            printf("ssl-gen: type=%s  subj=%s  savepath=%s\n",
                   ssl_params.gentype  ? ssl_params.gentype  : "selfsigned",
                   ssl_params.subj     ? ssl_params.subj     : "/CN=localhost",
                   ssl_params.savepath);
            int rc = ssl_gen_run(&ssl_params);
            if (rc != 0)
                fprintf(stderr, "ssl-gen error: %s\n", ssl_gen_last_error());
            else
                printf("ssl-gen: Done.\n");
            exit(rc ? 1 : 0);
        }
    }
#endif /* HAVE_OPENSSL */

    while (i < argc) {
        if (strcmp(argv[i], "-b") == 0) {
#ifndef WIN32
            pid_t pid;
            fprintf(stdout, "Starting Mcaster1DNAS\nDetaching from the console\n");

            pid = fork();

            if (pid > 0) {
                /* exit the parent */
                exit(0);
            }
            else if(pid < 0) {
                fprintf(stderr, "FATAL: Unable to fork child!");
                exit(1);
            }
            background = 1;
#endif
        }
        if (strcmp(argv[i], "-v") == 0) {
            fprintf(stdout, "%s\n", ICECAST_VERSION_STRING "-" GIT_VERSION);
            exit(0);
        }

        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            _print_usage();
            exit(0);
        }

        if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                strncpy(filename, argv[i + 1], size-1);
                filename[size-1] = 0;
                config_ok = 1;
            } else {
                return -1;
            }
        }
        i++;
    }

    if (config_ok)
        return 1;

    /* No -c flag supplied — search for mcaster1dnas.yaml automatically. */
    {
        char exedir[PATH_MAX] = "";
        char candidate[PATH_MAX];

        get_exe_dir(exedir, sizeof(exedir));

        /* 1. <exedir>/mcaster1dnas.yaml */
        if (exedir[0])
        {
            snprintf(candidate, sizeof(candidate), "%smcaster1dnas.yaml", exedir);
#ifdef _WIN32
            if (_access(candidate, 0) == 0)
#else
            if (access(candidate, F_OK) == 0)
#endif
            {
                strncpy(filename, candidate, size - 1);
                filename[size - 1] = '\0';
                return 1;
            }
        }

        /* 2. ./mcaster1dnas.yaml */
        strncpy(candidate, "mcaster1dnas.yaml", sizeof(candidate) - 1);
#ifdef _WIN32
        if (_access(candidate, 0) == 0)
#else
        if (access(candidate, F_OK) == 0)
#endif
        {
            strncpy(filename, candidate, size - 1);
            filename[size - 1] = '\0';
            return 1;
        }

        /* 3. Prompt user (stdin) */
        fprintf(stderr,
            "\n[Mcaster1DNAS] Config file not found.\n"
            "Searched: %smcaster1dnas.yaml\n"
            "          ./mcaster1dnas.yaml\n\n"
            "Enter path to configuration file (YAML or XML): ",
            exedir);
        fflush(stderr);

        if (fgets(filename, size, stdin))
        {
            filename[strcspn(filename, "\r\n")] = '\0';
            if (filename[0])
            {
#ifdef _WIN32
                if (_access(filename, 0) == 0)
#else
                if (access(filename, F_OK) == 0)
#endif
                    return 1;
                fprintf(stderr, "Config file not accessible: %s\n", filename);
            }
        }
    }

    return -1;
}


/* bind the socket and start listening */
static int server_proc_init(void)
{
    mc_config_t *config = config_get_config_unlocked();

    SI_TRACE("init_logging");
    if (init_logging (config) < 0)
        return 0;
    SI_TRACE("init_logging ok");

    songdata_init (config->song_history_limit);

    INFO2 ("%s server reading configuration from %s", ICECAST_VERSION_STRING, config->config_filename);

    SI_TRACE("connection_setup_sockets");
    if (connection_setup_sockets (config) == 0)
    {
        SI_TRACE("connection_setup_sockets FAILED");
        return 0;
    }
    SI_TRACE("connection_setup_sockets ok");

    _ch_root_uid_setup(); /* Change user id and root if requested/possible */

    /* recreate the pid file */
    if (config->pidfile)
    {
        FILE *f;
        pidfile = strdup (config->pidfile);
        if (pidfile && (f = fopen (config->pidfile, "w")) != NULL)
        {
            fprintf (f, "%d\n", (int)getpid());
            fclose (f);
        }
    }

    return 1;
}


/* this is the heart of the beast */
void server_process (void)
{
    INFO1 ("%s server started", ICECAST_VERSION_STRING);

    global.running = MC_RUNNING;

    /* Do this after logging init */
    auth_initialise ();

    /* Pre-register static mount types (podcast/ondemand/socialmedia) in the
     * stats system so they appear in the XML/JSON API without a live source */
    source_static_mounts_init (config_get_config_unlocked());

    if (background)
    {
        fclose (stdin);
        fclose (stdout);
        fclose (stderr);
    }
    slave_initialize();
    INFO0("Shutting down");
    auth_shutdown();
}


/* unix traditionally defaults to 1024 open FDs max, which is often a restriction for mcaster1
 * so here (as root) we check the current limit against clients allowed and up it while we can
 */
static void check_open_file_limit (mc_config_t *config)
{
#ifdef HAVE_GETRLIMIT
    struct rlimit rlimit;
    if (getrlimit (RLIMIT_NOFILE, &rlimit) == 0)
    {
        if (rlimit.rlim_max < config->client_limit)
        {
            rlim_t old = rlimit.rlim_max;
            rlimit.rlim_cur = rlimit.rlim_max = config->client_limit;
            if (setrlimit (RLIMIT_NOFILE, &rlimit) < 0)
                fprintf (stderr, "failed to increase max number of open files from %lu to %lu\n",
                        (unsigned long)old, (unsigned long)config->client_limit);
        }
    }
#endif
}


/* chroot the process. Watch out - we need to do this before starting other
 * threads. Change uid as well, after figuring out uid _first_ */
static void _ch_root_uid_setup(void)
{
   mc_config_t *conf = config_get_config_unlocked();
#ifdef CHUID
   struct passwd *user;
   struct group *group;
   uid_t uid=-1;
   gid_t gid=-1;

   if(conf->chuid)
   {
       if(conf->user) {
           user = getpwnam(conf->user);
           if(user)
               uid = user->pw_uid;
           else
               fprintf(stderr, "Couldn't find user \"%s\" in password file\n", conf->user);
       }
       if(conf->group) {
           group = getgrnam(conf->group);

           if(group)
               gid = group->gr_gid;
           else
               fprintf(stderr, "Couldn't find group \"%s\" in groups file\n", conf->group);
       }
   }
#endif

   check_open_file_limit (conf);

#ifdef HAVE_CHROOT
   if (conf->chroot)
   {
       if(getuid()) /* root check */
       {
           fprintf(stderr, "WARNING: Cannot change server root unless running as root.\n");
       }
       if (chroot(conf->base_dir) < 0 || chdir ("/") < 0)
       {
           fprintf(stderr,"WARNING: Couldn't change server root: %s\n", strerror(errno));
           return;
       }
       else
           fprintf(stdout, "Changed root successfully to \"%s\".\n", conf->base_dir);

   }   
#endif
#ifdef CHUID

   if(conf->chuid)
   {
       if(getuid()) /* root check */
       {
           fprintf(stderr, "WARNING: Can't change user id unless you are root.\n");
           return;
       }

       if (gid != (gid_t)-1)
       {
           if (initgroups (conf->user, gid) < 0)
               fprintf (stdout, "Error changing supplementary groups: %s.\n", strerror(errno));
           else
               fprintf (stdout, "Changed supplementary groups based on user: %s.\n", conf->user);
#ifdef HAVE_SETRESGID
           if (setresgid (gid, gid, gid) < 0)
#else
           if (setgid (gid) < 0)
#endif
               fprintf (stdout, "Error changing groupid: %s.\n", strerror(errno));
           else
               fprintf (stdout, "Changed groupid to %i.\n", (int)gid);
       }

       if (uid != (uid_t)-1)
       {
#ifdef HAVE_SETRESUID
           if (setresuid (uid, uid, uid) < 0)
#else
           if (setuid (gid) < 0)
#endif
               fprintf (stdout, "Error changing userid: %s.\n", strerror(errno));
           else
               fprintf (stdout, "Changed userid to %i.\n", (int)uid);
       }
   }
#endif
}


int server_init (int argc, char *argv[])
{
    int  ret;
    char filename[512];
    char pbuf[1024];

    SI_TRACE("_parse_config_opts");
    switch (_parse_config_opts (argc, argv, filename, 512))
    {
        case -1:
            _print_usage();
            return -1;
        default:
            /* parse the config file */
            SI_TRACE("config_initial_parse_file");
            config_get_config();
            ret = config_initial_parse_file(filename);
            config_release_config();
            if (ret < 0)
            {
                snprintf (pbuf, sizeof(pbuf),
                        "FATAL: error parsing config file (%s)", filename);
                _fatal_error (pbuf);
                switch (ret)
                {
                    case CONFIG_EINSANE:
                        _fatal_error("filename was null or blank");
                        break;
                    case CONFIG_ENOROOT:
                        _fatal_error("no root element found");
                        break;
                    case CONFIG_EBADROOT:
                        _fatal_error("root element is not <mcaster1>");
                        break;
                    default:
                        _fatal_error("XML config parsing error");
                        break;
                }
                return -1;
            }
    }

    SI_TRACE("config_parse_cmdline");
    /* override config file options with commandline options */
    config_parse_cmdline(argc, argv);

    SI_TRACE("server_proc_init");
    /* Bind socket, before we change userid */
    if (server_proc_init() == 0)
    {
        _fatal_error("Server startup failed. Exiting");
        return -1;
    }
    SI_TRACE("fserve_initialize");
    fserve_initialize();

#ifdef CHUID
    /* We'll only have getuid() if we also have setuid(), it's reasonable to
     * assume */
    if (getuid() == 0) /* Running as root! Don't allow this */
    {
        fprintf (stderr, "ERROR: You should not run mcaster1dnas as root\n");
        fprintf (stderr, "Use the changeowner directive in the config file\n");
        return -1;
    }
#endif
    SI_TRACE("sighandler_initialize");
    /* setup default signal handlers */
    sighandler_initialize();

    SI_TRACE("start_logging");
    if (start_logging (config_get_config_unlocked()) < 0)
    {
        _fatal_error("FATAL: Could not start logging");
        return -1;
    }
    SI_TRACE("server_init done -> returning 0");
    return 0;
}


#ifndef WIN32_SERVICE
int main (int argc, char *argv[])
{
#if defined(WIN32) && defined(_DEBUG)
    { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[1] initialize_subsystems\n");fclose(_t);} }
#endif
    initialize_subsystems();

#if defined(WIN32) && defined(_DEBUG)
    { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[2] server_init\n");fclose(_t);} }
#endif
    if (server_init (argc, argv) == 0)
    {
#if defined(WIN32) && defined(_DEBUG)
        { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[3] server_process starting\n");fclose(_t);} }
#endif
        server_process();
#if defined(WIN32) && defined(_DEBUG)
        { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[4] server_process done\n");fclose(_t);} }
#endif
    }
    else
    {
#if defined(WIN32) && defined(_DEBUG)
        { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[2b] server_init failed\n");fclose(_t);} }
#endif
    }

#if defined(WIN32) && defined(_DEBUG)
    { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[5] shutdown_subsystems\n");fclose(_t);} }
#endif
    shutdown_subsystems();

#if defined(WIN32) && defined(_DEBUG)
    { FILE *_t = fopen("mcaster1win_start.log","a"); if(_t){fprintf(_t,"[6] main done\n");fclose(_t);} }
#endif
    if (pidfile)
    {
        remove (pidfile);
        free (pidfile);
    }
    return 0;
}
#endif /* WIN32_SERVICE */

