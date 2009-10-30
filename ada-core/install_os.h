#ifndef INSTALL_OS_H
#define INSTALL_OS_H

#define INSTALL_OS_POSIX 0x0000
#define INSTALL_OS_WIN32 0x0001

/* win32 */
#ifndef INSTALL_OS_TYPE
#  if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WIN64__) || defined(__TOS_WIN__)
#    define INSTALL_OS_TYPE INSTALL_OS_WIN32
#  endif
#endif

/* fallback OS type - posix */
#ifndef INSTALL_OS_TYPE
#  define INSTALL_OS_TYPE INSTALL_OS_POSIX
#endif

/*
 * prototypes
 */

#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
/* require >= Windows 2000 */
#ifndef WINVER
#  define WINVER 0x0500
#endif

#include <windows.h>
#include <io.h>

typedef struct { PSID value; } user_id_t;
typedef struct { PSID value; } group_id_t;

#define INSTALL_NULL_UID {NULL}
#define INSTALL_NULL_GID {NULL}

/* 1 byte revision + 1 byte sub authority count + 6 bytes authority ID,
 * variable number of 4 byte subauthorities (max 15)
 */

#define INSTALL_FMT_UID 256
#define INSTALL_FMT_GID 256
#undef INSTALL_HAVE_SYMLINKS

#endif /* INSTALL_OS_TYPE == INSTALL_OS_WIN32 */

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
#include <unistd.h>

typedef struct { int value; } user_id_t;
typedef struct { int value; } group_id_t;

#define INSTALL_NULL_UID {-1}
#define INSTALL_NULL_GID {-1}
#define INSTALL_FMT_UID (sizeof (int) << 3)
#define INSTALL_FMT_GID (sizeof (int) << 3)
#define INSTALL_HAVE_SYMLINKS

#endif /* INSTALL_OS_TYPE == INSTALL_OS_POSIX */

#endif
