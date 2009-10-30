#define INSTALL_IMPLEMENTATION
#include "install.h"

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX

#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static struct install_status_t
iposix_init (void)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  status.status = INSTALL_STATUS_OK;

  inst_exec_suffix [0] = 0;
  return status;
}

/*
 * Error.
 */

static error_t
iposix_error_current (void)
{
  error_t e;
  e.value = (unsigned long) errno;
  return e;
}

static const char *
iposix_error_message (error_t error)
{
  return strerror (error.value);
}

static const char *
iposix_error_message_current (void)
{
  return iposix_error_message (iposix_error_current ());
}

static void
iposix_error_reset (void)
{
  errno = 0;
}

/*
 * GID.
 */

static int
iposix_gid_compare (group_id_t a, group_id_t b)
{
  return a.value == b.value;
}

static unsigned int
iposix_gid_format (char *buffer, group_id_t gid)
{
  assert (buffer != NULL);

  unsigned int size = snprintf (buffer, INSTALL_FMT_GID, "%d", gid.value);
  return size;
}

static unsigned int
iposix_gid_scan (const char *buffer, group_id_t *gid)
{
  assert (buffer != NULL);
  assert (gid    != NULL);

  unsigned int size = sscanf (buffer, "%d", &gid->value);
  return size;
}

static int
iposix_gid_current (group_id_t *gid)
{
  assert (gid != NULL);

  gid->value = getgid ();
  return 1;
}

static int
iposix_gid_lookup (const char *group, group_id_t *gid)
{
  struct group *grp;

  assert (group != NULL);
  assert (gid   != NULL);

  grp = getgrnam (group);
  if (!grp) return 0;
  gid->value = grp->gr_gid;

  return 1;
}

static int
iposix_gid_valid (group_id_t gid)
{
  return gid.value != -1;
}

static void
iposix_gid_free (group_id_t *gid)
{
  assert (gid != NULL);
}

/*
 * UID.
 */

static int
iposix_uid_compare (user_id_t a, user_id_t b)
{
  return a.value == b.value;
}

static unsigned int
iposix_uid_format (char *buffer, user_id_t uid)
{
  assert (buffer != NULL);

  unsigned int size = snprintf (buffer, INSTALL_FMT_UID, "%d", uid.value);
  return size;
}

static unsigned int
iposix_uid_scan (const char *buffer, user_id_t *uid)
{
  assert (buffer != NULL);
  assert (uid    != NULL);

  unsigned int size = sscanf (buffer, "%d", &uid->value);
  return size;
}

static int
iposix_uid_current (user_id_t *uid)
{
  assert (uid != NULL);

  uid->value = getuid ();
  return 1;
}

static int
iposix_uid_lookup (const char *user, user_id_t *uid)
{
  struct passwd *pwd;

  assert (user != NULL);
  assert (uid  != NULL);

  pwd = getpwnam (user);
  if (!pwd) return 0;
  uid->value = pwd->pw_uid;

  return 1;
}

static int
iposix_uid_valid (user_id_t uid)
{
  return uid.value != -1;
}

static void
iposix_uid_free (user_id_t *uid)
{
  assert (uid != NULL);
}

/*
 * User name.
 */

const char *
iposix_user_name_current (void)
{
  return getlogin ();
}

/*
 * Directory.
 */

static int
iposix_mkdir (const char *dir, permissions_t mode)
{
  assert (dir != NULL);
  return mkdir (dir, mode.value) == 0;
}

/*
 * File.
 */

static int
iposix_file_mode_set (const char *file, permissions_t mode)
{
  assert (file != NULL);

  return chmod (file, mode.value) == 0;
}

static int
iposix_file_mode_get (const char *file, permissions_t *mode)
{
  struct stat sb;

  assert (file != NULL);
  assert (mode != NULL);

  if (stat (file, &sb) == -1) return 0;
  mode->value = sb.st_mode & 0755;
  return 1;
}

static int
iposix_file_ownership_set (const char *file, user_id_t uid, group_id_t gid)
{
  assert (file != NULL);

  if (chown (file, uid.value, gid.value) == -1) return 0;
  return 1;
}

static int
iposix_file_ownership_get (const char *file, user_id_t *uid, group_id_t *gid)
{
  struct stat sb;

  assert (file != NULL);
  assert (uid  != NULL);
  assert (gid  != NULL);

  if (stat (file, &sb) == -1) return 0;

  uid->value = sb.st_uid;
  gid->value = sb.st_gid;
  return 1;
}

static int
iposix_file_size (const char *file, size_t *size)
{
  struct stat sb;

  assert (file != NULL);
  assert (size != NULL);

  if (stat (file, &sb) == -1) return 0;

  *size = sb.st_size;
  return 1;
}

static int
iposix_file_link (const char *src, const char *dst)
{
  assert (src != NULL);
  assert (dst != NULL);

  if (symlink (src, dst) == -1) return 0;
  return 1;
}

/*
 * Misc.
 */

static unsigned int
iposix_umask (unsigned int m)
{
  return umask (m);
}

static int
iposix_can_set_ownership (user_id_t uid)
{
  assert (uid.value != -1);
  return uid.value == 0; /* Only the superuser can chown() files. */
}

static int
iposix_supports_symlinks (void)
{
  return 1;
}

static int
iposix_supports_posix_modes (void)
{
  return 1;
}

const struct install_platform_callbacks_t install_platform_posix = {
  &iposix_init,

  &iposix_error_message,
  &iposix_error_message_current,  
  &iposix_error_current,
  &iposix_error_reset,

  &iposix_gid_compare,
  &iposix_gid_format,
  &iposix_gid_scan,
  &iposix_gid_current,
  &iposix_gid_lookup,
  &iposix_gid_valid,
  &iposix_gid_free,

  &iposix_uid_compare,
  &iposix_uid_format,
  &iposix_uid_scan,
  &iposix_uid_current,
  &iposix_uid_lookup,
  &iposix_uid_valid,
  &iposix_uid_free,

  &iposix_user_name_current,

  &iposix_mkdir,

  &iposix_file_mode_get,
  &iposix_file_mode_set,
  &iposix_file_ownership_get,
  &iposix_file_ownership_set,
  &iposix_file_size,
  &iposix_file_link,

  &iposix_can_set_ownership,
  &iposix_supports_symlinks,
  &iposix_supports_posix_modes,
  &iposix_umask,
};

#endif
