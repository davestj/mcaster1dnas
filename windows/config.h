/* config.h - Windows MSVC build configuration for Mcaster1DNAS
 *
 * This file replaces the autotools-generated config.h for MSVC builds.
 * It is included when HAVE_CONFIG_H is defined (set in the .vcxproj preprocessor).
 *
 * Generated for: Windows x86/x64, MSVC (Visual Studio 2022 / v143 toolset)
 * vcpkg dependencies: libxml2, libxslt, libcurl, openssl, libogg, libvorbis,
 *                     libtheora, speex, libiconv, pthreads4w, libyaml
 */

#ifndef CONFIG_H_WINDOWS
#define CONFIG_H_WINDOWS

/* -----------------------------------------------------------------------
 * Package / version info
 * ----------------------------------------------------------------------- */
#define PACKAGE         "mcaster1dnas"
#define PACKAGE_NAME    "Mcaster1DNAS"
#define PACKAGE_VERSION "2.5.1"
#define PACKAGE_STRING  "Mcaster1DNAS 2.5.1"
#define VERSION         "2.5.1"

/* -----------------------------------------------------------------------
 * Platform
 * ----------------------------------------------------------------------- */
#define WIN32 1
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601   /* Windows 7+ */
#endif

/* -----------------------------------------------------------------------
 * Windows system headers available
 * ----------------------------------------------------------------------- */
#define HAVE_WINSOCK2_H 1
#define HAVE_MALLOC_H   1
#define HAVE_SIGNAL_H   1
#define HAVE_FCNTL_H    1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_STDINT_H   1
#define HAVE_INTTYPES_H 1
#define HAVE_STDBOOL_H  1

/* -----------------------------------------------------------------------
 * C standard library functions available on MSVC 2015+
 * ----------------------------------------------------------------------- */
#define HAVE_STRTOLL    1
#define HAVE_UINT32_T   1
#define HAVE_ISWCTYPE   1
#define HAVE_MBSRTOWCS  1

/* Windows uses FindFirstFile instead of POSIX glob */
#define HAVE_DECL_FINDFIRSTFILE 1

/* MSVC 2015+ defines isblank via <ctype.h> */
#define HAVE_DECL_ISBLANK 1

/* -----------------------------------------------------------------------
 * Core types and missing defines
 * ----------------------------------------------------------------------- */
/* File descriptor type - int on all POSIX systems and Windows */
#define FD_t int

/* Define for socklen_t if needed */
#define HAVE_SOCKLEN_T 1

/* struct timespec available in VS2015+ */
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
#endif

/* getnameinfo is in ws2tcpip.h on Windows */
#define HAVE_GETNAMEINFO 1

/* getaddrinfo is available in winsock2/ws2tcpip.h on Windows Vista+ */
/* Enables the modern socket iterator path in sock.c (required for multi-port binding) */
#define HAVE_GETADDRINFO 1

/* _WIN32_WINNT for IPv6 (already guarded above; keep for documentation) */

/* -----------------------------------------------------------------------
 * GCC-specific extensions: neutralize on MSVC
 * ----------------------------------------------------------------------- */
#ifdef _MSC_VER
/* Suppress __attribute__((format(...))) and similar GCC attrs */
#define __attribute__(x)
/* gnu_printf is a GCC printf format specifier alias */
#define gnu_printf printf
#endif

/* -----------------------------------------------------------------------
 * Feature flags: auth subsystems and YP directory
 * ----------------------------------------------------------------------- */
#define HAVE_AUTH_URL 1
#define USE_YP 1
#define HAVE_OGG 1

/* -----------------------------------------------------------------------
 * Optional codec/library features (installed via vcpkg)
 * ----------------------------------------------------------------------- */
#define HAVE_CURL       1
#define HAVE_CURL_GLOBAL_INIT 1
#define HAVE_OPENSSL    1
#define HAVE_SPEEX      1
#define HAVE_THEORA     1
#define HAVE_YAML       1

/* Kate codec - not available via vcpkg, disabled */
/* #undef HAVE_KATE */

/* -----------------------------------------------------------------------
 * libxslt: xsltSaveResultToString is present in libxslt >= 1.1.x
 * ----------------------------------------------------------------------- */
#define HAVE_XSLTSAVERESULTTOSTRING 1

/* -----------------------------------------------------------------------
 * Time functions: HAVE_FTIME for timing.c / thread.c (ftime fallback path).
 * log.c uses the time()-only fallback (struct timeval, tv_sec only).
 * struct timeval is provided by the winsock2.h inclusion below.
 * ----------------------------------------------------------------------- */
#define HAVE_FTIME          1
#define HAVE_SYS_TIMEB_H    1   /* <sys/timeb.h> exists on MSVC for ftime() */

/* Ensure CLOCK_REALTIME is visible. windows.h may include an early time.h
 * variant that does not define CLOCK_REALTIME; including time.h here and
 * providing the constant guarantees it is always available. */
#ifdef _MSC_VER
#include <time.h>
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
#endif

/* PATH_MAX: MSVC does not define POSIX PATH_MAX; map to _MAX_PATH */
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif

/* -----------------------------------------------------------------------
 * POSIX compatibility macros for MSVC
 * ----------------------------------------------------------------------- */
#ifdef _MSC_VER
/* String comparison - object-like macros so they work as function pointers too */
#define HAVE_STRCASECMP 1       /* tell httpp.c not to redefine strcasecmp */
#define strcasecmp   _stricmp
#define strncasecmp  _strnicmp

/* File access */
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#define access  _access         /* POSIX access() -> MSVC _access() */
#include <io.h>                 /* declares _access() */

/* localtime_r / gmtime_r: util.c provides Windows-compatible implementations
 * when HAVE_LOCALTIME_R / HAVE_GMTIME_R are not defined (uses localtime/gmtime_s) */
#endif

/* -----------------------------------------------------------------------
 * POSIX features NOT available on Windows (do not define)
 * ----------------------------------------------------------------------- */
/* #undef HAVE_UNISTD_H       - use _unistd.h shim or omit */
/* #undef HAVE_ALLOCA_H       - use <malloc.h> _alloca on Windows */
/* #undef HAVE_ARPA_INET_H    - use winsock2.h */
/* #undef HAVE_NETDB_H        - use ws2tcpip.h */
/* #undef HAVE_NETINET_IN_H   - use winsock2.h */
/* #undef HAVE_SYS_SOCKET_H   - use winsock2.h */
/* #undef HAVE_SYS_TIME_H     - use time.h or Windows equivalents */
/* #undef HAVE_SYS_WAIT_H     - not applicable */
/* #undef HAVE_CHROOT         - not applicable */
/* #undef HAVE_SETRESUID      - not applicable */
/* #undef HAVE_SETRESGID      - not applicable */
/* #undef HAVE_SIGACTION      - not applicable */
/* #undef HAVE_SIGNALFD       - not applicable */
/* #undef HAVE_PIPE2          - not applicable */
/* #undef HAVE_POLL           - use WSAPoll via winsock2.h */
/* #undef HAVE_GLOB           - use FindFirstFile */
/* #undef HAVE_GLOB_H         - use FindFirstFile */
/* #undef HAVE_FSYNC          - use FlushFileBuffers */
/* #undef HAVE_PREAD          - not available */
/* #undef HAVE_GETRLIMIT      - not applicable */
/* #undef HAVE_GMTIME_R       - use gmtime_s instead */
/* #undef HAVE_LOCALTIME_R    - use localtime_s instead */
/* #undef HAVE_MEMMEM         - not in MSVC CRT; our own impl used */
/* #undef HAVE_STRSEP         - not in MSVC CRT */
/* #undef HAVE_MEMPCPY        - not in MSVC CRT */
/* #undef HAVE_WMEMPCPY       - not in MSVC CRT */
/* #undef HAVE_STRINGS_H      - use string.h */
/* #undef HAVE_FNMATCH_H      - not available; use windows/fnmatch.h shim */
/* #undef HAVE_FNMATCH_GNU    - not applicable */
/* #undef HAVE_FSEEKO         - not in MSVC; _fseeki64 used instead */

/* -----------------------------------------------------------------------
 * ssize_t: MSVC does not define ssize_t; provide it via basetsd.h SSIZE_T
 * ----------------------------------------------------------------------- */
#ifdef _MSC_VER
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#endif

/* -----------------------------------------------------------------------
 * MSVC compatibility: suppress common POSIX deprecation warnings
 * ----------------------------------------------------------------------- */
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif

/* -----------------------------------------------------------------------
 * pthreads via pthreads4w (vcpkg)
 * The pthreads4w package provides pthreadVSE3.lib / pthreads.h
 * ----------------------------------------------------------------------- */
/* Define which pthreads4w ABI we link against */
#define PTW32_STATIC_LIB 0  /* using DLL variant */

/* -----------------------------------------------------------------------
 * Additional MSVC compatibility: GCC intrinsics, POSIX stat macros,
 * stack allocation, and Winsock types (struct timeval for log.c fallback).
 * winsock2.h MUST appear before any windows.h inclusion.
 * ----------------------------------------------------------------------- */
#ifdef _MSC_VER

/* GCC built-in optimization hint — no-op on MSVC */
#ifndef __builtin_expect
#define __builtin_expect(exp, c)    (exp)
#endif

/* POSIX S_ISREG stat macro — _S_IFMT/_S_IFREG defined in <sys/stat.h> */
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & _S_IFMT) == _S_IFREG)
#endif

/* Stack allocation: MSVC uses _alloca() from <malloc.h> */
#include <malloc.h>
#ifndef alloca
#define alloca  _alloca
#endif

/* winsock2.h provides struct timeval, SOCKET, etc.
 * Guard prevents re-inclusion if already pulled in transitively. */
#ifndef _WINSOCK2API_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#endif  /* _MSC_VER */

#endif /* CONFIG_H_WINDOWS */
