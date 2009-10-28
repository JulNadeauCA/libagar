#define INSTALL_IMPLEMENTATION
#include "install.h"

#if INSTALL_OS_TYPE == INSTALL_OS_POSIX

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int
iposix_uid_lookup (const char *user, user_id_t *uid)
{
  struct passwd *pwd;

  if (user) {
    pwd = getpwnam (user);
    if (!pwd) return 0;
    uid->value = pwd->pw_uid;
  } else {
    uid->value = getuid ();
  }

  return 1;
}

int
iposix_gid_lookup (const char *group, group_id_t *gid)
{
  struct group *grp;

  if (group) {
    grp = getgrnam (group);
    if (!grp) return 0;
    gid->value = grp->gr_gid;
  } else {
    gid->value = getgid ();
  }
  return 1;
}

int
iposix_gid_current (group_id_t *gid)
{
  gid->value = getgid ();
  return 1;
}

int
iposix_uid_current (user_id_t *uid)
{
  uid->value = getuid ();
  return 1;
}

int
iposix_file_set_ownership (const char *file, user_id_t uid, group_id_t gid)
{
  if (chown (file, uid.value, gid.value) == -1) return 0;
  return 1;
}

int
iposix_file_get_ownership (const char *file, user_id_t *uid, group_id_t *gid)
{
  struct stat sb;

  if (stat (file, &sb) == -1) return 0;

  uid->value = sb.st_uid;
  gid->value = sb.st_gid;
  return 1;
}

int
iposix_file_set_mode (const char *file, permissions_t mode)
{
  return chmod (file, mode.value) == 0;
}

int
iposix_file_get_mode (const char *file, permissions_t *mode)
{
  struct stat sb;

  if (stat (file, &sb) == -1) return 0;
  mode->value = sb.st_mode & 0755;
  return 1;
}

int
iposix_file_link (const char *src, const char *dst)
{
  if (symlink (src, dst) == -1) return 0;
  return 1;
}

int
iposix_compare_uid (user_id_t a, user_id_t b)
{
  return a.value == b.value;
}

int
iposix_compare_gid (group_id_t a, group_id_t b)
{
  return a.value == b.value;
}

struct install_status_t
iposix_install_init (void)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  status.status = INSTALL_STATUS_OK;

  inst_exec_suffix [0] = 0;
  return status;
}

unsigned int
iposix_fmt_gid (char *buffer, group_id_t gid)
{
  unsigned int size = snprintf (buffer, INSTALL_FMT_GID, "%d", gid.value);
  return size;
}

unsigned int
iposix_fmt_uid (char *buffer, user_id_t uid)
{
  unsigned int size = snprintf (buffer, INSTALL_FMT_UID, "%d", uid.value);
  return size;
}

unsigned int
iposix_scan_gid (const char *buffer, group_id_t *gid)
{
  unsigned int size = sscanf (buffer, "%d", &gid->value);
  return size;
}

unsigned int
iposix_scan_uid (const char *buffer, user_id_t *uid)
{
  unsigned int size = sscanf (buffer, "%d", &uid->value);
  return size;
}

unsigned int
iposix_umask (unsigned int m)
{
  return umask (m);
}

int
iposix_mkdir (const char *dir, unsigned int mode)
{
  return mkdir (dir, mode) == 0;
}

#endif
