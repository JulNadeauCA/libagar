#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#define INSTALL_IMPLEMENTATION
#include "install.h"

char inst_exec_suffix [16];
char inst_dlib_suffix [16];
unsigned long install_failed;

void (*install_callback_warn)(const char *, void *);
void (*install_callback_info)(const char *, void *);
void *install_callback_data;

/*@null@*/ static const char *inst_fake_root = NULL;

#ifdef INSTALL_HAVE_SYMLINKS
static int have_symlinks = 1;
#else
static int have_symlinks = 0;
#endif

/*
 * Credential functions
 */

int
install_file_set_ownership (const char *file, user_id_t uid, group_id_t gid)
{
  assert (file != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_file_set_ownership (file, uid, gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_file_set_ownership (file, uid, gid);
#endif
}

int
install_file_get_ownership (const char *file, user_id_t *uid, group_id_t *gid)
{
  assert (file != NULL);
 
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_file_get_ownership (file, uid, gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_file_get_ownership (file, uid, gid);
#endif
}

int
install_file_get_mode (const char *file, permissions_t *mode)
{
  assert (file != NULL);
  assert (mode != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_file_get_mode (file, mode);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_file_get_mode (file, mode);
#endif
}

int
install_file_link (const char *src, const char *dst)
{
  assert (src != NULL);
  assert (dst != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_file_link (src, dst);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_file_link (src, dst);
#endif
}

int
install_gid_current (group_id_t *gid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_gid_current (gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_gid_current (gid);
#endif
}

int
install_uid_current (user_id_t *uid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_uid_current (uid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_uid_current (uid);
#endif
}

int
install_gid_lookup (const char *name, group_id_t *gid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_gid_lookup (name, gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_gid_lookup (name, gid);
#endif
}

int
install_uid_lookup (const char *name, user_id_t *uid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_uid_lookup (name, uid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_uid_lookup (name, uid);
#endif
}

unsigned int
install_fmt_gid (char *buffer, group_id_t gid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_fmt_gid (buffer, gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_fmt_gid (buffer, gid);
#endif
}

unsigned int
install_fmt_uid (char *buffer, user_id_t uid)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_fmt_uid (buffer, uid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_fmt_uid (buffer, uid);
#endif
}

unsigned int
install_scan_gid (const char *buffer, group_id_t *gid)
{
  assert (buffer != NULL);
  assert (gid != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_scan_gid (buffer, gid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_scan_gid (buffer, gid);
#endif
}

unsigned int
install_scan_uid (const char *buffer, user_id_t *uid)
{
  assert (buffer != NULL);
  assert (uid != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_scan_uid (buffer, uid);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_scan_uid (buffer, uid);
#endif
}

int
install_compare_uid (user_id_t a, user_id_t b)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_compare_uid (a, b);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_compare_uid (a, b);
#endif
}

int
install_compare_gid (group_id_t a, group_id_t b)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_compare_gid (a, b);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_compare_gid (a, b);
#endif
}

void
install_gid_free (group_id_t *gid)
{
  assert (gid != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  iwin32_gid_free (gid);
#endif
}

void
install_uid_free (user_id_t *uid)
{
  assert (uid != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  iwin32_uid_free (uid);
#endif
}

int
install_compare_permissions (permissions_t a, permissions_t b)
{
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return 1;
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return (int) (a.value == b.value);
#endif
}

void
install_perms_assign (permissions_t *perm, unsigned int mode)
{
  assert (perm != NULL);
  perm->value = mode;
}

void
install_status_assign (struct install_status_t *status,
  enum install_status_enum_t code, /*@null@*/ /*@dependent@*/ const char *message)
{
  assert (status != NULL);

  status->status = code;
  status->message = message;
}

/* Portability macro */
#ifndef S_ISSOCK
#  if defined(S_IFMT) && defined(S_IFSOCK)
#    define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#  else
#    define S_ISSOCK(mode) (0)
#  endif
#endif

static int s_ifreg (mode_t m)  { return (int) S_ISREG(m); }
static int s_ifchr (mode_t m)  { return (int) S_ISCHR(m); }
static int s_ifdir (mode_t m)  { return (int) S_ISDIR(m); }
static int s_ifsock (mode_t m) { return (int) S_ISSOCK(m); }
static int s_ififo (mode_t m)  { return (int) S_ISFIFO(m); }

#ifdef INSTALL_HAVE_SYMLINKS
static int s_iflnk (mode_t m)  { return (int) S_ISLNK(m); }
#else
static int s_iflnk (mode_t m)  { return 0; }
#endif

static const struct {
  int (*check)(mode_t);
  /*@dependent@*/ const char *name;
  const enum install_file_type_t type;
} file_type_lookups [] = {
  { &s_ifreg,  "file",              INSTALL_FILE_TYPE_FILE },
  { &s_ifchr,  "character_special", INSTALL_FILE_TYPE_CHARACTER_SPECIAL },
  { &s_ifdir,  "directory",         INSTALL_FILE_TYPE_DIRECTORY },
  { &s_iflnk,  "symlink",           INSTALL_FILE_TYPE_SYMLINK },
  { &s_ifsock, "socket",            INSTALL_FILE_TYPE_SOCKET },
  { &s_ififo,  "fifo",              INSTALL_FILE_TYPE_FIFO },
};
static const size_t file_type_lookups_size =
  sizeof (file_type_lookups) / sizeof (file_type_lookups [0]);

int
install_file_type (const char *file, enum install_file_type_t *type, int nofollow)
{
  struct stat sb;
  unsigned int index;
  const unsigned int last = (unsigned int) file_type_lookups_size;

  assert (file != NULL);
  assert (type != NULL);

#ifdef INSTALL_HAVE_SYMLINKS
  if (nofollow != 0) {
    if (lstat (file, &sb) == -1) return 0;
  } else {
    if (stat (file, &sb) == -1) return 0;
  }
#else
  if (stat (file, &sb) == -1) return 0;
#endif

  for (index = 0; index < last; ++index) {
    if (file_type_lookups [index].check (sb.st_mode) != 0) {
      *type = file_type_lookups [index].type;
      return 1;
    }
  }
  return 0;
}

int
install_file_type_lookup (const char *type_name, enum install_file_type_t *type)
{
  unsigned int index;
  const unsigned int last = (unsigned int) file_type_lookups_size;

  assert (type_name != NULL);

  for (index = 0; index < last; ++index) {
    if (strcmp (file_type_lookups [index].name, type_name) == 0) {
      *type = file_type_lookups [index].type;
      return 1;
    }
  }
  return 0;
}

int
install_file_type_name_lookup (enum install_file_type_t type, const char **returned_name)
{
  unsigned int index;
  const unsigned int last = (unsigned int) file_type_lookups_size;

  for (index = 0; index < last; ++index) {
    if (file_type_lookups [index].type == type) {
      *returned_name = file_type_lookups [index].name;
      return 1;
    }
  }
  return 0;
}

int
install_file_set_mode (const char *file, permissions_t mode)
{
  assert (file != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_file_set_mode (file, mode);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_file_set_mode (file, mode);
#endif
}

struct install_status_t
install_file_copy (const char *src, const char *dst,
  user_id_t uid, group_id_t gid, permissions_t mode)
{
  static char dst_tmp [INSTALL_MAX_PATHLEN];
  static char copy_buf [65536];
  char *copy_ptr = NULL;
  FILE *fd_src = NULL;
  FILE *fd_dst = NULL;
  size_t r;
  size_t w;
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (dst != NULL);
  assert (src != NULL);

  if (snprintf (dst_tmp, sizeof (dst_tmp), "%s.tmp", dst) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format string");
    return status;
  }

  fd_src = fopen (src, "rb");
  if (fd_src == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not open source file");
    return status;
  }
  fd_dst = fopen (dst_tmp, "wb");
  if (fd_dst == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not open destination file");
    goto ERR;
  }

  for (;;) {
    r = fread (copy_buf, 1, sizeof (copy_buf), fd_src);
    if (r == 0) {
      if (feof (fd_src) != 0) break;
      if (ferror (fd_src) != 0) {
        install_status_assign (&status, INSTALL_STATUS_ERROR, "read error");
        goto ERR;
      }
    }
    copy_ptr = copy_buf;
    while (r > 0) {
      w = fwrite (copy_ptr, 1, r, fd_dst);
      if (w == 0) {
        if (feof (fd_dst) != 0) break;
        if (ferror (fd_dst) != 0) {
          install_status_assign (&status, INSTALL_STATUS_ERROR, "write error");
          goto ERR;
        }
      }
      r -= w;
      copy_ptr += w;
    }
  }

  if (fflush (fd_dst) != 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "write error");
    goto ERR;
  }
  if (install_file_set_mode (dst_tmp, mode) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not set file permissions");
    goto ERR;
  }
  if (install_file_set_ownership (dst_tmp, uid, gid) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not set file ownership");
    goto ERR;
  }
  if (fclose (fd_dst) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not close destination file");
    goto ERR;
  }
  if (fclose (fd_src) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not close source file");
    goto ERR;
  }
  if (rename (dst_tmp, dst) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not rename temporary file");
    goto ERR;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;

  ERR:
  if (fd_dst != NULL) (void) fclose (fd_dst);
  if (fd_src != NULL) (void) fclose (fd_src);
  (void) unlink (dst_tmp);
  return status;
}

int
install_file_size (const char *file, size_t *size)
{
  struct stat sb;

  assert (file != NULL);
  assert (size != NULL);

  if (stat (file, &sb) == -1) return 0;

  *size = (size_t) sb.st_size;
  return 1;
}

static struct install_status_t
install_uidgid_lookup (const char *user, user_id_t *uid,
  const char *group, group_id_t *gid)
{
  struct install_status_t status = INSTALL_STATUS_INIT;

  if (install_uid_lookup (user, uid) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not lookup user ID");
    return status;
  }
  if (install_gid_lookup (group, gid) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not lookup group ID");
    install_uid_free (uid);
    return status;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
install_file_check (const char *file_src, permissions_t mode_want,
  enum install_file_type_t type_want, user_id_t uid_want, group_id_t gid_want,
  const char *file_dst)
  /*@globals errno; @*/
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  static char uid_want_str [INSTALL_FMT_UID];
  static char uid_got_str [INSTALL_FMT_UID];
  static char gid_want_str [INSTALL_FMT_GID];
  static char gid_got_str [INSTALL_FMT_GID];
  user_id_t uid_got                 = INSTALL_NULL_UID;
  group_id_t gid_got                = INSTALL_NULL_GID;
  enum install_file_type_t type_got = INSTALL_FILE_TYPE_FILE;
  permissions_t mode_got            = INSTALL_NULL_PERMISSIONS;
  const char *type_got_name         = NULL;
  const char *type_want_name        = NULL;
  size_t size_got                   = 0;
  size_t size_want                  = 0;
  int no_follow                     = 0;
  struct install_status_t status    = INSTALL_STATUS_INIT;

  assert (file_src != NULL);
  assert (file_dst != NULL);

  /* Reset errno */
  errno = 0;

  /* Get source file types and type names */
  if (type_want == INSTALL_FILE_TYPE_SYMLINK) no_follow = 1;
  if (install_file_type_name_lookup (type_want, &type_want_name) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "unknown source filetype");
    return status;
  }

  /* Get source file size, if necessary */
  if (type_want == INSTALL_FILE_TYPE_FILE) {
    if (install_file_size (file_src, &size_want) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not determine source file size");
      return status;
    }
  }

  /* Format ownership data */
  uid_want_str [install_fmt_uid (uid_want_str, uid_want)] = (char) 0;
  gid_want_str [install_fmt_gid (gid_want_str, gid_want)] = (char) 0;

  /* Call info callback if defined */
  if (install_callback_info != NULL) {
    assert (type_want_name != NULL);
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "check %s %s %s %s %o %s %lu",
      file_src, file_dst, uid_want_str, gid_want_str, mode_want.value,
      type_want_name, (unsigned long) size_want);
    install_callback_info (msg_buffer, install_callback_data);
  }

  /* Check file type */
  if (install_file_type (file_dst, &type_got, no_follow) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not determine destination file type");
    return status;
  }
  if (install_file_type_name_lookup (type_got, &type_got_name) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "unknown destination file type");
    return status;
  }
  if ((have_symlinks == 0) && (type_want != INSTALL_FILE_TYPE_SYMLINK)) {
    if (type_want != type_got) {
      assert (type_want_name != NULL);
      assert (type_got_name != NULL);
      (void) snprintf (msg_buffer, sizeof (msg_buffer), "filetype %s not %s",
        type_got_name, type_want_name);

      install_status_assign (&status, INSTALL_STATUS_ERROR, msg_buffer);
      return status;
    }
  }

  /* Check file size */
  if (type_want == INSTALL_FILE_TYPE_FILE) {
    if (install_file_size (file_dst, &size_got) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not determine destination file size");
      return status;
    }
    if (size_want != size_got) {
      (void) snprintf (msg_buffer, sizeof (msg_buffer), "size %lu not %lu",
        (unsigned long) size_got, (unsigned long) size_want);

      install_status_assign (&status, INSTALL_STATUS_ERROR, msg_buffer);
      return status;
    }
  }

  /* Check file permissions */
  if (install_file_get_mode (file_dst, &mode_got) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not determine destination file mode");
    return status;
  }

  if (install_compare_permissions (mode_got, mode_want) == 0) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "mode %o not %o",
      mode_got.value, mode_want.value);

    install_status_assign (&status, INSTALL_STATUS_ERROR, msg_buffer);
    return status;
  }
 
  /* Check file ownership */
  if (install_file_get_ownership (file_dst, &uid_got, &gid_got) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not determine destination file ownership");
    goto END;
  }
  uid_got_str [install_fmt_uid (uid_got_str, uid_got)] = (char) 0;
  gid_got_str [install_fmt_gid (gid_got_str, gid_got)] = (char) 0;

  if (install_compare_uid (uid_want, uid_got) == 0) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "uid %s not %s",
      uid_got_str, uid_want_str);

    install_status_assign (&status, INSTALL_STATUS_ERROR, msg_buffer);
    goto END;
  }
  if (install_compare_gid (gid_want, gid_got) == 0) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "gid %s not %s",
      gid_got_str, gid_want_str);

    install_status_assign (&status, INSTALL_STATUS_ERROR, msg_buffer);
    goto END;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  END:
  install_uid_free (&uid_got);
  install_gid_free (&gid_got);
  return status; 
}

unsigned int
install_umask (unsigned int m)
{
#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_umask (m);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_umask (m);
#endif
}

int
install_mkdir (const char *dir, unsigned int mode)
{
  assert (dir != NULL);

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_mkdir (dir, mode);
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_mkdir (dir, mode);
#endif
}

static int
install_rmkdir (const char *dir, unsigned int perm)
  /*@globals errno; @*/
{
  char path_buf [INSTALL_MAX_PATHLEN];
  size_t len;
  size_t pos;
  size_t buflen;
  size_t bufpos;
  int end;
  const char *ptr;
  char *ptr2;

  assert (dir != NULL);

  buflen = sizeof (path_buf);
  bufpos = 0;
  end = 0;
  len = strlen (dir);
  ptr = dir;

  if (len >= sizeof (path_buf)) { errno = ENAMETOOLONG; return 0; }

  for (;;) {
    if (len == 0) break;
    ptr2 = strchr (ptr, '/');
    if (ptr2 == NULL) {
      pos = len;
      end = 1;
    } else pos = (size_t) (ptr2 - ptr);
    if (buflen <= (size_t) pos + 1) break;
    memcpy (path_buf + bufpos, ptr, pos);
    bufpos += pos;
    buflen -= pos;
    path_buf [bufpos] = '/';
    ++bufpos;
    --buflen;
    path_buf [bufpos] = (char) 0;
    if (install_mkdir (path_buf, perm) == -1) {
      if (end == 0) {
        if (errno != EEXIST && errno != EISDIR) return 0;
      } else return 0;
    }
    ptr += pos;
    len -= pos;
    if (len != 0) {
      ++ptr;
      --len;
      if (len == 0) break;
    }
  }

  return 1;
}

/*
 * Portability functions
 */

static int
base_name (const char *dir, char **out)
{
  static char path_buf [INSTALL_MAX_PATHLEN];
  const char *s;
  const char *t;
  const char *u;
  size_t len;
  size_t nlen;

  assert (dir != NULL);

  len = strlen (dir); 

  if (len == 0) {
    path_buf [0] = (char) '.';
    path_buf [1] = (char) 0;
    *out = path_buf;
    return 1;
  }

  if (len >= INSTALL_MAX_PATHLEN) return 0;

  s = dir;
  t = s + (len - 1);
  while ((t > s) && (t [0] == '/')) --t;

  if ((t == s) && (t [0] == '/')) {
    path_buf [0] = (char) '/';
    path_buf [1] = (char) 0;
    *out = path_buf;
    return 1;
  }
  u = t;
  while ((u > s) && (*(u - 1) != '/')) --u;

  nlen = (size_t) (t - u) + 1;
  memcpy (path_buf, u, nlen);
  path_buf [nlen] = (char) 0;

  *out = path_buf;
  return 1;
}

static int
str_same (const char *a, const char *b)
{
  return (int) (strcmp (a, b) == 0);
}

static int
str_ends (const char *s, const char *end)
{
  register size_t slen = strlen (s);
  register size_t elen = strlen (end);

  if (elen > slen) elen = slen;
  s += (slen - elen);
  return str_same (s, end);
}

/*
 * Utilities
 */

static int
libname (char *name, char *buf)
{
  static char read_buf [INSTALL_MAX_PATHLEN];
  FILE *fp;
  char *s;
  size_t r;
  int ret = 1;
  int clean;

  assert (name != NULL);
  assert (buf != NULL);

  fp = fopen (name, "rb");
  if (fp == NULL) return 0;

  r = fread (read_buf, (size_t) 1, INSTALL_MAX_PATHLEN, fp);
  if (r < INSTALL_MAX_PATHLEN) {
    if (ferror (fp) != 0) {
      ret = 0;
      goto END;
    }
  }

  /* Clean whitespace from buffer */
  s = read_buf;
  clean = 0;
  while (r != 0) {
    switch (*s) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        s [0] = (char) 0;
        clean = 1;
        break;
      default:
        break;
    }
    if (clean == 1) break;
    --r;
    ++s;
  }
  memcpy (buf, read_buf, s - read_buf);
  buf [s - read_buf] = (char) 0;

  END:
  if (fclose (fp) != 0) return 0;
  return ret;
}

/*
 * Install operator callbacks
 */

static struct install_status_t
inst_copy (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  static char uid_str [INSTALL_FMT_UID];
  static char gid_str [INSTALL_FMT_GID];
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  permissions_t perm             = INSTALL_NULL_PERMISSIONS;
  size_t size                    = 0;
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  install_perms_assign (&perm, (unsigned int) item->perm);

  status = install_uidgid_lookup (item->owner, &uid, item->group, &gid);
  if (status.status != INSTALL_STATUS_OK) return status;

  /* Defer error here (attempting to copy will give actual error) */
  (void) install_file_size (item->src, &size);

  uid_str [install_fmt_uid (uid_str, uid)] = (char) 0;
  gid_str [install_fmt_gid (gid_str, gid)] = (char) 0;

  /* Call info callback */
  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "copy %s %s %s %s %o %lu",
      item->src, item->dst, uid_str, gid_str, perm.value, (unsigned long) size);
    install_callback_info (msg_buffer, install_callback_data);
  }

  /* Copy file if not performing dry run */
  if ((flags & INSTALL_DRYRUN) == INSTALL_NO_FLAGS) {
    status = install_file_copy (item->src, item->dst, uid, gid, perm);
    if (status.status != INSTALL_STATUS_OK) goto END;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);

  END:
  install_uid_free (&uid);
  install_gid_free (&gid);
  return status;
}

static struct install_status_t
inst_link (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  static char path_buf [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  /* Call info callback */
  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "symlink %s %s %s",
      item->dir, item->src, item->dst);
    install_callback_info (msg_buffer, install_callback_data);
  }

  if (getcwd (path_buf, sizeof (path_buf)) == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not get current working directory");
    return status;
  }
  if (chdir (item->dir) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not change directory");
    return status;
  }
  if ((flags & INSTALL_DRYRUN) == INSTALL_NO_FLAGS) {
    if (install_file_link (item->src, item->dst) == 0) {
      if (chdir (path_buf) == -1) {
        install_status_assign (&status, INSTALL_STATUS_ERROR, "could not restore current working directory");
        return status;
      }
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not make symbolic link");
      return status;
    }
  }
  if (chdir (path_buf) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not restore current working directory");
    return status;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
inst_mkdir (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  static char uid_str [INSTALL_FMT_UID];
  static char gid_str [INSTALL_FMT_GID];
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = install_uidgid_lookup (item->owner, &uid, item->group, &gid);
  if (status.status != INSTALL_STATUS_OK) return status;

  uid_str [install_fmt_uid (uid_str, uid)] = (char) 0;
  gid_str [install_fmt_gid (gid_str, gid)] = (char) 0;

  /* Call info callback */
  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "mkdir %s %s %s %o", item->dir,
      uid_str, gid_str, (unsigned int) item->perm);
    install_callback_info (msg_buffer, install_callback_data);
  }

  /* Create directory if not performing dry run */
  if ((flags & INSTALL_DRYRUN) == INSTALL_NO_FLAGS) {
    if (install_rmkdir (item->dir, (unsigned int) item->perm) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not create directory");
      goto END;
    }
    if (install_file_set_ownership (item->dir, uid, gid) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not set ownership on directory");
      goto END;
    }
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  END:
  install_uid_free (&uid);
  install_gid_free (&gid);
  return status;
}

static struct install_status_t
inst_liblink (struct install_item *item, unsigned int flags)
{
  return inst_link (item, flags);
}

/*
 * Name translation callbacks
 */

static struct install_status_t
ntran_copy (struct install_item *item)
{
  static char src_name [INSTALL_MAX_PATHLEN];
  static char dst_name [INSTALL_MAX_PATHLEN];
  static char dst_tmp [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (item->src == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "source file undefined");
    return status;
  }
  if (item->dir == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "directory undefined");
    return status;
  }
  if (item->dst == NULL) item->dst = item->src;

  /* If source or destination file are virtual library files, build
   * real library names. */
  if (str_ends (item->src, ".vlb") != 0) {
    if (libname (item->src, src_name) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not build library name");
      return status;
    }
    item->src = src_name;
  }
  if (str_ends (item->dst, ".vlb") != 0) {
    if (libname (item->dst, dst_name) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not build library name");
      return status;
    }
    item->dst = dst_name;
  }

  assert (item->src != NULL);
  assert (item->dst != NULL);

  if (base_name (item->dst, &item->dst) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "invalid destination path");
    return status;
  }

  assert (item->dst != NULL);

  /* Build file names, possibly prefixed with fake root */
  if (inst_fake_root != NULL) {
    if (snprintf (dst_tmp, sizeof (dst_tmp), "%s/%s/%s",
      inst_fake_root, item->dir, item->dst) < 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
      return status;
    }
  } else {
    if (snprintf (dst_tmp, sizeof (dst_tmp), "%s/%s", item->dir, item->dst) < 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
      return status;
    }
  }

  item->dst = dst_tmp;

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_copy_exec (struct install_item *item)
{
  static char src_name [INSTALL_MAX_PATHLEN];
  static char dst_name [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  /* Format basic paths */
  status = ntran_copy (item);
  if (status.status != INSTALL_STATUS_OK) return status;

  assert (item->src != NULL);
  assert (item->dst != NULL);

  /* Append executable suffix if necessary */
  if (snprintf (src_name, sizeof (src_name), "%s%s", item->src, inst_exec_suffix) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format source path");
    return status;
  }

  /* Format destination path */
  if (snprintf (dst_name, sizeof (dst_name), "%s%s", item->dst, inst_exec_suffix) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
    return status;
  }

  item->src = src_name;
  item->dst = dst_name;

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_link (struct install_item *item)
{
  static char dir_name [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (item->src == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "source file undefined");
    return status;
  }
  if (item->dir == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "directory undefined");
    return status;
  }
  if (item->dst == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "destination file undefined");
    return status;
  }

  /* Modify path if fake root was specified */
  if (inst_fake_root != NULL) {
    if (snprintf (dir_name, INSTALL_MAX_PATHLEN, "%s/%s", inst_fake_root, item->dir) < 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
      return status;
    }
    item->dir = dir_name;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_liblink (struct install_item *item)
{
  static char dir_name [INSTALL_MAX_PATHLEN];
  static char dst_tmp [INSTALL_MAX_PATHLEN];
  static char src_name [INSTALL_MAX_PATHLEN];
  static char src_tmp [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (item->src == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "source file undefined");
    return status;
  }
  if (item->dir == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "directory undefined");
    return status;
  }
  if (item->dst == NULL) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "destination file undefined");
    return status;
  }

  /* Modify path if fake root was specified */
  if (inst_fake_root != NULL) {
    if (snprintf (dir_name, INSTALL_MAX_PATHLEN, "%s/%s", inst_fake_root, item->dir) < 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
      return status;
    }
    item->dir = dir_name;
  }

  /* If source or destination file are virtual library files, build
   * real library names. */
  if (str_ends (item->src, ".vlb") != 0) {
    if (libname (item->src, src_tmp) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not build library name");
      return status;
    }
    item->src = src_tmp;
    if (base_name (item->src, &item->src) == 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "invalid source path");
      return status;
    }
    memcpy (src_name, item->src, INSTALL_MAX_PATHLEN);
    item->src = src_name;
  }

  /* Build library name */
  if (base_name (item->dst, &item->dst) == 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "invalid destination path");
    return status;
  }

  assert (item->dst != NULL);

  if (snprintf (dst_tmp, INSTALL_MAX_PATHLEN, "%s%s", item->dst, inst_dlib_suffix) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
    return status;
  }
  item->dst = dst_tmp;

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_mkdir (struct install_item *item)
{
  static char dir_name [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (item->dst == NULL) item->dst = item->src;

  /* Modify path if fake root was specified */
  if (inst_fake_root != NULL) {
    if (snprintf (dir_name, INSTALL_MAX_PATHLEN, "%s/%s", inst_fake_root, item->dir) < 0) {
      install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
      return status;
    }
    item->dir = dir_name;
  }

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_chk_link (struct install_item *item)
{
  static char dst_name [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = ntran_link (item);
  if (status.status != INSTALL_STATUS_OK) return status;

  if (snprintf (dst_name, INSTALL_MAX_PATHLEN, "%s/%s", item->dir, item->dst) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
    return status;
  }

  item->dst = dst_name;

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
ntran_chk_liblink (struct install_item *item)
{
  static char dst_name [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = ntran_liblink (item);
  if (status.status != INSTALL_STATUS_OK) return status;

  if (snprintf (dst_name, INSTALL_MAX_PATHLEN, "%s/%s", item->dir, item->dst) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format destination path");
    return status;
  }

  item->dst = dst_name;

  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

/*
 * instchk operator callbacks
 */

static struct install_status_t
instchk_copy (struct install_item *item, /*@unused@*/ unsigned int flags)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  permissions_t perm             = INSTALL_NULL_PERMISSIONS;

  assert (item != NULL);

  install_perms_assign (&perm, (unsigned int) item->perm);

  status = install_uidgid_lookup (item->owner, &uid, item->group, &gid);
  if (status.status != INSTALL_STATUS_OK) return status;

  status = install_file_check (item->src, perm, INSTALL_FILE_TYPE_FILE,
    uid, gid, item->dst);
  if (status.status != INSTALL_STATUS_OK) ++install_failed;

  install_uid_free (&uid);
  install_gid_free (&gid);
  return status;
}

static struct install_status_t
instchk_link (struct install_item *item, /*@unused@*/ unsigned int flags)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  permissions_t perm             = INSTALL_NULL_PERMISSIONS;

  assert (item != NULL);

  install_perms_assign (&perm, (unsigned int) item->perm);

  status = install_uidgid_lookup (item->owner, &uid, item->group, &gid);
  if (status.status != INSTALL_STATUS_OK) return status;

  status = install_file_check (item->src, perm, INSTALL_FILE_TYPE_SYMLINK,
    uid, gid, item->dst);
  if (status.status != INSTALL_STATUS_OK) ++install_failed;

  install_uid_free (&uid);
  install_gid_free (&gid);
  return status;
}

static struct install_status_t
instchk_mkdir (struct install_item *item, /*@unused@*/ unsigned int flags)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  permissions_t perm             = INSTALL_NULL_PERMISSIONS;

  assert (item != NULL);

  install_perms_assign (&perm, (unsigned int) item->perm);

  status = install_uidgid_lookup (item->owner, &uid, item->group, &gid);
  if (status.status != INSTALL_STATUS_OK) return status;

  status = install_file_check (item->dir, perm, INSTALL_FILE_TYPE_DIRECTORY,
    uid, gid, item->dir);
  if (status.status != INSTALL_STATUS_OK) ++install_failed;

  install_uid_free (&uid);
  install_gid_free (&gid);
  return status;
}

static struct install_status_t
instchk_liblink (struct install_item *item, unsigned int flags)
{
  return instchk_link (item, flags);
}

/*
 * deinstall operator callbacks
 */

static struct install_status_t
deinst_copy (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  /* Call info callback */
  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "unlink %s", item->dst);
    install_callback_info (msg_buffer, install_callback_data);
  }

  if ((flags & INSTALL_DRYRUN) != INSTALL_NO_FLAGS) goto END;

  if (unlink (item->dst) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not unlink file");
    return status;
  }

  END:
  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
deinst_link (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  static char path_buf [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "unlink %s/%s", item->dir, item->dst);
    install_callback_info (msg_buffer, install_callback_data);
  }

  if ((flags & INSTALL_DRYRUN) != INSTALL_NO_FLAGS) goto END;

  if (snprintf (path_buf, sizeof (path_buf), "%s/%s", item->dir, item->dst) < 0) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not format string");
    return status;
  }
  if (unlink (path_buf) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not unlink");
    return status;
  }

  END:
  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
deinst_mkdir (struct install_item *item, unsigned int flags)
{
  static char msg_buffer [INSTALL_MAX_MSGLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  if (install_callback_info != NULL) {
    (void) snprintf (msg_buffer, sizeof (msg_buffer), "rmdir %s", item->dir);
    install_callback_info (msg_buffer, install_callback_data);
  }

  if ((flags & INSTALL_DRYRUN) != INSTALL_NO_FLAGS) goto END;

  if (rmdir (item->dir) == -1) {
    install_status_assign (&status, INSTALL_STATUS_ERROR, "could not remove directory");
    return status;
  }

  END:
  install_status_assign (&status, INSTALL_STATUS_OK, NULL);
  return status;
}

static struct install_status_t
deinst_liblink (struct install_item *item, unsigned int flags)
{
  assert (item != NULL);
  return deinst_link (item, flags);
}

/*
 * Operator callback tables
 */

struct instop {
  struct install_status_t (*oper) (struct install_item *, unsigned int);
  struct install_status_t (*trans) (struct install_item *);
};
static struct instop install_opers [] = {
  { inst_copy,    ntran_copy },
  { inst_copy,    ntran_copy_exec },
  { inst_link,    ntran_link },
  { inst_mkdir,   ntran_mkdir },
  { inst_liblink, ntran_liblink },
};
static struct instop instchk_opers [] = {
  { instchk_copy,    ntran_copy },
  { instchk_copy,    ntran_copy_exec },
  { instchk_link,    ntran_chk_link },
  { instchk_mkdir,   ntran_mkdir },
  { instchk_liblink, ntran_chk_liblink },
};
static struct instop deinst_opers [] = {
  { deinst_copy,    ntran_copy },
  { deinst_copy,    ntran_copy_exec },
  { deinst_link,    ntran_link },
  { deinst_mkdir,   ntran_mkdir },
  { deinst_liblink, ntran_liblink },
};

/*
 * Interface
 */

static void
install_suffix_sanitize (char *buffer, size_t size)
{
  char ch;
  size_t index;

  assert (buffer != NULL);

  /* Sanitize suffix */
  for (index = 0; index < size; ++index) {
    ch = buffer [index];
    if (ch == '.') continue;
    if ((ch >= '0') && (ch <= '9')) continue;
    if ((ch >= 'A') && (ch <= 'Z')) continue;
    if ((ch >= 'a') && (ch <= 'z')) continue;
    buffer [index] = (char) 0;
    break;
  }
}

struct install_status_t
install_init (const char *suffix_file)
{
  static char error_buffer [INSTALL_MAX_PATHLEN];
  struct install_status_t status = INSTALL_STATUS_INIT;
  struct stat sb;
  FILE *fp;

  assert (suffix_file != NULL);

  /* Check fake_root exists, if requested */
  if (inst_fake_root != NULL) {
    if (stat (inst_fake_root, &sb) == -1) {
      (void) snprintf (error_buffer, sizeof (error_buffer),
        "fake root %s does not exist", inst_fake_root);
      install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
      return status;
    }
    if (s_ifdir (sb.st_mode) == 0) {
     (void) snprintf (error_buffer, sizeof (error_buffer),
        "fake root %s is not a directory", inst_fake_root);
      install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
      return status;
    }
  }

  /* Open file containing dynamic library suffix */
  fp = fopen (suffix_file, "rb");
  if (fp == NULL) {
    (void) snprintf (error_buffer, sizeof (error_buffer),
      "could not open %s", suffix_file);
    install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
    return status;
  }

  memset (inst_exec_suffix, 0, sizeof (inst_exec_suffix));
  memset (inst_dlib_suffix, 0, sizeof (inst_dlib_suffix));

  /* Read file containing dynamic library suffix */
  inst_dlib_suffix [0] = '.';
  if (fread (inst_dlib_suffix + 1, 1, sizeof (inst_dlib_suffix) - 2, fp) == 0) {
    if (ferror (fp) != 0) {
      (void) snprintf (error_buffer, sizeof (error_buffer),
        "error reading %s", suffix_file);
      install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
    }
    if (feof (fp) != 0) {
      (void) snprintf (error_buffer, sizeof (error_buffer),
        "%s is empty", suffix_file);
      install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
    }
    (void) fclose (fp);
    return status;
  }

  /* Ensure suffix is alphanumeric */
  install_suffix_sanitize (inst_dlib_suffix, sizeof (inst_dlib_suffix));

  /* Close suffix file */
  if (fclose (fp) != 0) {
    (void) snprintf (error_buffer, sizeof (error_buffer),
      "could not close %s", suffix_file);
    install_status_assign (&status, INSTALL_STATUS_ERROR, error_buffer);
    return status;
  }

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX
  return iposix_install_init ();
#endif
#if INSTALL_OS_TYPE == INSTALL_OS_WIN32
  return iwin32_install_init ();
#endif
}

struct install_status_t
install (struct install_item *item, unsigned int flags)
{
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = install_opers [item->op].trans (item);
  if (status.status != INSTALL_STATUS_OK) goto CLEANUP;
  status = install_opers [item->op].oper (item, flags);

  CLEANUP:
  (void) fflush (NULL);
  return status;
}

struct install_status_t
install_check (struct install_item *item)
{
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = instchk_opers [item->op].trans (item);
  if (status.status != INSTALL_STATUS_OK) goto CLEANUP;
  status = instchk_opers [item->op].oper (item, 0);

  CLEANUP:
  (void) fflush (NULL);
  return status;
}

struct install_status_t
deinstall (struct install_item *item, unsigned int flags)
{
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (item != NULL);

  status = deinst_opers [item->op].trans (item);
  if (status.status != INSTALL_STATUS_OK) goto CLEANUP;
  status = deinst_opers [item->op].oper (item, flags);

  CLEANUP:
  (void) fflush (NULL);
  return status;
}

void
install_callback_warn_set (void (*callback)(const char *, void *))
{
  install_callback_warn = callback;
}

void
install_callback_info_set (void (*callback)(const char *, void *))
{
  install_callback_info = callback;
}

void
install_callback_data_set (void *data)
{
  install_callback_data = data;
}

void
install_fake_root (const char *path)
{
  inst_fake_root = path;
}
