/* win_applink.c
 * Windows-only: set up OpenSSL CRT linkage so PEM file I/O works correctly
 * when using OpenSSL as a DLL (vcpkg build).
 *
 * Must be compiled into exactly ONE compilation unit per executable.
 * Include <openssl/applink.c> here and nowhere else.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_OPENSSL
#include <openssl/applink.c>
#endif
