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

#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#define MC_LISTEN_QUEUE 64

#define MC_RUNNING 1
#define MC_HALTING 2

#define ICECAST_VERSION_STRING "Mcaster1DNAS " PACKAGE_VERSION
#define MCASTER1_VERSION_STRING "Mcaster1DNAS " PACKAGE_VERSION

#include <signal.h>
#include "net/sock.h"
#include "compat.h"
#include "avl/avl.h"

typedef struct mc_global_tag
{
    int server_sockets;
    sock_t *serversock;
    struct _listener_t **server_conn;

    volatile sig_atomic_t running;
    volatile sig_atomic_t schedule_config_reread;

    int new_connections_slowdown;
    int sources;
    int clients;
    int listeners;

    avl_tree *source_tree;

#ifdef MY_ALLOC
    avl_tree *alloc_tree;
#endif

    avl_tree *relays;

    /* redirection to slaves */
    unsigned int redirect_count;

    /* a copy of what is in the config xml */
    int64_t max_rate;

    struct rate_calc *out_bitrate;

    rwlock_t workers_rw;
} mc_global_t;

extern unsigned int throttle_sends;

extern void initialize_subsystems (void);
extern void shutdown_subsystems (void);
extern void server_process (void);
extern int  server_init (int argc, char *argv[]);


#ifdef MY_ALLOC
#define calloc(x,y) my_calloc(__func__,__LINE__,x,y)
#define malloc(x) my_calloc(__func__,__LINE__,1,x)
#define realloc(x,y) my_realloc(__func__,__LINE__,x,y)
#define free(x) my_free(x)
#define strdup(x) my_strdup(__func__,__LINE__,x)
char *my_strdup (const char *file, int line, const char *s);
void *my_calloc (const char *file, int line, size_t num, size_t size);
void *my_realloc (const char *file, int line, void *ptr, size_t size);
void my_free (void *freeblock);
int compare_allocs(void *arg, void *a, void *b);
int free_alloc_node(void *key);

typedef struct {
    char name[54];
    int count;
    int allocated;
} alloc_node;

#endif

extern mc_global_t global;

void global_initialize(void);
void global_shutdown(void);
void global_lock(void);
void global_unlock(void);
int  global_state(void);
void global_add_bitrates (struct rate_calc *rate, unsigned long value, uint64_t milli);
void global_reduce_bitrate_sampling (struct rate_calc *rate);
unsigned long global_getrate_avg (struct rate_calc *rate);

#endif  /* __GLOBAL_H__ */
