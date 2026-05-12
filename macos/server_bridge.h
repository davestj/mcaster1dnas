/*
 * server_bridge.h — Bootstrap header for C++ files that include server C headers.
 *
 * Must be the first include inside every extern "C" { } block that pulls in
 * server headers. This ensures:
 *   1. config.h is loaded → defines HAVE_SYS_UIO_H, HAVE_UNISTD_H, etc.
 *   2. net/sock.h is loaded → defines IOVEC (used by connection.h)
 *
 * Without this, the autoconf HAVE_* guards in sock.h are never satisfied
 * from C++ compilation units (which don't do #ifdef HAVE_CONFIG_H themselves).
 */
#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "net/sock.h"   /* IOVEC, sock_t — required by connection.h */
