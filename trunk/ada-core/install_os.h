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

int iwin32_compare_gid (group_id_t, group_id_t);
int iwin32_compare_uid (user_id_t, user_id_t);
int iwin32_file_get_mode (const char *, permissions_t *);
int iwin32_file_get_ownership (const char *, user_id_t *, group_id_t *);
int iwin32_file_link (const char *, const char *);
int iwin32_file_set_mode (const char *, permissions_t);
int iwin32_file_set_ownership (const char *, user_id_t, group_id_t);
int iwin32_gid_current (group_id_t *);
int iwin32_gid_lookup (const char *, group_id_t *);
int iwin32_mkdir (const char *, unsigned int);
int iwin32_uid_current (user_id_t *);
int iwin32_uid_lookup (const char *, user_id_t *);
struct install_status_t iwin32_install_init (void);
unsigned int iwin32_fmt_gid (char *, group_id_t);
unsigned int iwin32_fmt_uid (char *, user_id_t);
unsigned int iwin32_scan_gid (const char *, group_id_t *);
unsigned int iwin32_scan_uid (const char *, user_id_t *);
unsigned int iwin32_umask (unsigned int);
void iwin32_gid_free (group_id_t *);
void iwin32_uid_free (user_id_t *);
void iwin32_uidgid_current (user_id_t *, group_id_t *);
#endif

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
#include <unistd.h>

typedef struct { int value; } user_id_t;
typedef struct { int value; } group_id_t;

#define INSTALL_NULL_UID {-1}
#define INSTALL_NULL_GID {-1}
#define INSTALL_FMT_UID (sizeof (int) << 3)
#define INSTALL_FMT_GID (sizeof (int) << 3)
#define INSTALL_HAVE_SYMLINKS

int iposix_compare_gid (group_id_t, group_id_t);
int iposix_compare_uid (user_id_t, user_id_t);
int iposix_file_set_mode (const char *, permissions_t);
int iposix_file_get_mode (const char *, permissions_t *);
int iposix_file_get_ownership (const char *, user_id_t *, group_id_t *);
int iposix_file_link (const char *, const char *);
int iposix_file_set_ownership (const char *, user_id_t, group_id_t);
int iposix_gid_current (group_id_t *);
int iposix_gid_lookup (const char *, group_id_t *);
int iposix_mkdir (const char *, unsigned int);
int iposix_uid_current (user_id_t *);
int iposix_uid_lookup (const char *, user_id_t *);
struct install_status_t iposix_install_init (void);
unsigned int iposix_fmt_gid (char *, group_id_t);
unsigned int iposix_fmt_uid (char *, user_id_t);
unsigned int iposix_scan_gid (const char *, group_id_t *);
unsigned int iposix_scan_uid (const char *, user_id_t *);
unsigned int iposix_umask (unsigned int);
void iposix_uidgid_current (user_id_t *, group_id_t *);
#endif

#endif
