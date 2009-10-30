#define INSTALL_IMPLEMENTATION
#include "install.h"

#if INSTALL_OS_TYPE == INSTALL_OS_WIN32

#include <assert.h>

#include <sys/stat.h>
#include <windows.h>
#include <sddl.h>
#include <lmcons.h>
#include <io.h>
#include <stdio.h>

#define BUFFER_SIZE 256

static user_id_t administrator_id = INSTALL_NULL_UID;

static char  user_name [UNLEN + 1];
static DWORD user_name_size = sizeof (user_name);
static int   user_name_fetched;

static int
iwin32_get_sid (const char *name, PSID *ext_sid)
{
  char domain [BUFFER_SIZE];
  DWORD domain_size = BUFFER_SIZE;
  DWORD sid_size    = BUFFER_SIZE;
  SID *sid;
  SID_NAME_USE account_type;

  assert (name    != NULL);
  assert (ext_sid != NULL);

  sid = malloc (sid_size);
  if (!sid) return 0;

  if (!LookupAccountName (NULL, name, sid, &sid_size,
    domain, &domain_size, &account_type)) return 0;

  *ext_sid = sid;
  return 1;
}

/*
 * User name.
 */

const char *
iwin32_user_name_current (void)
{
  if (user_name_fetched) return user_name;
  if (!GetUserName (user_name, &user_name_size)) return NULL;

  assert (user_name_size > 0);
  assert (user_name_size <= sizeof (user_name));

  user_name_fetched = 1;
  return user_name;
}

/*
 * Error.
 */

static error_t
iwin32_error_current (void)
{
  error_t e;
  e.value = GetLastError ();
  return e;
}

static const char *
iwin32_error_message (error_t error)
{
  static char buffer [8192];
  DWORD error_code = GetLastError ();

  if (error_code != 0) {
    FormatMessage
     (FORMAT_MESSAGE_FROM_SYSTEM,
      NULL,
      error_code,
      0,
      (LPTSTR) buffer,
      sizeof (buffer),
      0);
  } else {
    buffer [0] = 0;
  }

  return buffer;
}

static const char *
iwin32_error_message_current (void)
{
  return iwin32_error_message (iwin32_error_current ());
}

static void
iwin32_error_reset (void)
{
  SetLastError (0);
}

/*
 * GID.
 */

static int
iwin32_gid_compare (group_id_t a, group_id_t b)
{
  assert (a.value != NULL);
  assert (b.value != NULL);

  return EqualSid (a.value, b.value);
}

static unsigned int
iwin32_gid_format (char *buffer, group_id_t gid)
{
  char *sid_str;
  unsigned long len;

  assert (buffer    != NULL);
  assert (gid.value != NULL);

  if (!ConvertSidToStringSid (gid.value, &sid_str)) return 0;
  len = strlen (sid_str);
  memcpy (buffer, sid_str, len);
  LocalFree (sid_str);
  return len;
}

static unsigned int
iwin32_gid_scan (const char *buffer, group_id_t *gid)
{
  assert (buffer     != NULL);
  assert (gid        != NULL);
  assert (gid->value == NULL);

  if (!ConvertStringSidToSid ((char *) buffer, &gid->value)) return 0;
  return strlen (buffer);
}

static int
iwin32_gid_current (group_id_t *gid)
{
  HANDLE               thread_tok;
  DWORD                needed;
  TOKEN_PRIMARY_GROUP *group;
  DWORD                sid_size;

  assert (gid        != NULL);
  assert (gid->value == NULL);

  if (!OpenProcessToken (GetCurrentProcess(),
    STANDARD_RIGHTS_READ | READ_CONTROL | TOKEN_QUERY, &thread_tok)) return 0;

  /*
   * Is this _really_ correct?
   */

  if (!GetTokenInformation (thread_tok, TokenPrimaryGroup, NULL, 0, &needed)) {
    if (GetLastError () == ERROR_INSUFFICIENT_BUFFER) {
      group = malloc (needed);
      if (group == NULL) return 0;
      if (GetTokenInformation (thread_tok, TokenPrimaryGroup, group, needed, &needed)) {
        sid_size   = GetLengthSid (group->PrimaryGroup);
        gid->value = malloc (sid_size);
        if (gid->value == NULL) {
          free (group);
          return 0;
        }
        if (!CopySid (sid_size, gid->value, group->PrimaryGroup)) {
          free (gid->value);
          free (group);
          return 0;
        }
      }
      free (group);
    } else {
      return 0;
    }
  }

  return 1;
}

static int
iwin32_gid_lookup (const char *group, group_id_t *gid)
{
  assert (group      != NULL);
  assert (gid        != NULL);
  assert (gid->value == NULL);

  return iwin32_get_sid (group, &gid->value);
}

static int
iwin32_gid_valid (group_id_t gid)
{
  return gid.value != NULL;
}

static void
iwin32_gid_free (group_id_t *gid)
{
  assert (gid != NULL);
  free (gid->value);
  gid->value = NULL;
}

/*
 * UID.
 */

static int
iwin32_uid_compare (user_id_t a, user_id_t b)
{
  assert (a.value != NULL);
  assert (b.value != NULL);

  return EqualSid (a.value, b.value);
}

static unsigned int
iwin32_uid_format (char *buffer, user_id_t uid)
{
  char *sid_str;
  unsigned long len;

  assert (buffer    != NULL);
  assert (uid.value != NULL);

  if (!ConvertSidToStringSid (uid.value, &sid_str)) return 0;
  len = strlen (sid_str);
  memcpy (buffer, sid_str, len);
  LocalFree (sid_str);
  return len;
}

static unsigned int
iwin32_uid_scan (const char *buffer, user_id_t *uid)
{
  assert (buffer     != NULL);
  assert (uid        != NULL);
  assert (uid->value == NULL);

  if (!ConvertStringSidToSid ((char *) buffer, &uid->value)) return 0;
  return strlen (buffer);
}

static int
iwin32_uid_lookup (const char *user, user_id_t *uid)
{
  assert (uid        != NULL);
  assert (uid->value == NULL);

  return iwin32_get_sid (user, &uid->value);
}

static int
iwin32_uid_current (user_id_t *uid)
{
  const char *name;

  assert (uid != NULL);
  assert (uid->value == NULL);

  name = iwin32_user_name_current ();
  if (name == NULL) return 0;

  return iwin32_uid_lookup (name, uid);
}

static int
iwin32_uid_valid (user_id_t uid)
{
  return uid.value != NULL;
}

static void
iwin32_uid_free (user_id_t *uid)
{
  assert (uid != NULL);
  free (uid->value);
  uid->value = NULL;
}

/*
 * Directory.
 */

static int
iwin32_mkdir (const char *dir, permissions_t mode)
{
  assert (dir != NULL);
  return mkdir (dir) == 0;
}

/*
 * File.
 */

static int
iwin32_file_mode_set (const char *file, permissions_t mode)
{
  assert (file != NULL);
  return 1;
}

static int
iwin32_file_mode_get (const char *file, permissions_t *mode)
{
  assert (file != NULL);
  assert (mode != NULL);

  mode->value = 0;
  return 1;
}

static int
iwin32_file_set_sid (const char *file, PSID sid)
{
  char file_sd_buf [256];
  PSECURITY_DESCRIPTOR file_sd = (PSECURITY_DESCRIPTOR) &file_sd_buf;

  if (!InitializeSecurityDescriptor (file_sd, SECURITY_DESCRIPTOR_REVISION)) return 0;
  if (!SetSecurityDescriptorOwner (file_sd, sid, FALSE)) return 0;
  if (!IsValidSecurityDescriptor (file_sd)) return 0;
  if (!SetFileSecurity (file, (SECURITY_INFORMATION)(OWNER_SECURITY_INFORMATION),
    file_sd)) return 0;

  return 1;
}

static int
iwin32_file_ownership_set (const char *file, user_id_t uid, group_id_t gid)
{
  assert (file != NULL);

  if (!iwin32_file_set_sid (file, uid.value)) return 0;
  return 1;
}

static int
iwin32_file_get_owner (const char *file, user_id_t *uid)
{
  char file_sd_buf [256];
  DWORD sd_size = 256;
  PSECURITY_DESCRIPTOR file_sd = (PSECURITY_DESCRIPTOR) &file_sd_buf;
  PSID sid;
  BOOL dummy;

  assert (file != NULL);
  assert (uid  != NULL);

  if (!InitializeSecurityDescriptor (file_sd, SECURITY_DESCRIPTOR_REVISION))
    return 0;
  if (!GetFileSecurity (file, (SECURITY_INFORMATION)(OWNER_SECURITY_INFORMATION),
    file_sd, sizeof (file_sd_buf), &sd_size)) return 0;
  if (!GetSecurityDescriptorOwner (file_sd, &sid, &dummy)) return 0;
  if (!IsValidSid (sid)) return 0;

  uid->value = malloc (sd_size);
  if (!uid->value) return 0;
  if (!CopySid (sd_size, uid->value, sid)) return 0;

  return 1;
}

static int
iwin32_file_get_group (const char *file, group_id_t *gid)
{
  char file_sd_buf [256];
  DWORD sd_size = 256;
  PSECURITY_DESCRIPTOR file_sd = (PSECURITY_DESCRIPTOR) &file_sd_buf;
  PSID sid;
  BOOL dummy;

  assert (file != NULL);
  assert (gid  != NULL);

  if (!InitializeSecurityDescriptor (file_sd, SECURITY_DESCRIPTOR_REVISION))
    return 0;
  if (!GetFileSecurity (file, (SECURITY_INFORMATION)(GROUP_SECURITY_INFORMATION),
    file_sd, sizeof (file_sd_buf), &sd_size)) return 0;
  if (!GetSecurityDescriptorGroup (file_sd, &sid, &dummy)) return 0;
  if (!IsValidSid (sid)) return 0;

  gid->value = malloc (sd_size);
  if (!gid->value) return 0;
  if (!CopySid (sd_size, gid->value, sid)) return 0;

  return 1;
}

static int
iwin32_file_ownership_get (const char *file, user_id_t *uid, group_id_t *gid)
{
  assert (file != NULL);
  assert (uid  != NULL);
  assert (gid  != NULL);

  if (!iwin32_file_get_owner (file, uid)) return 0;
  if (!iwin32_file_get_group (file, gid)) return 0;
  return 1;
}

static int
iwin32_file_size (const char *file, size_t *size)
{
  struct stat sb;

  assert (file != NULL);
  assert (size != NULL);

  if (stat (file, &sb) == -1) return 0;

  *size = sb.st_size;
  return 1;
}

static int
iwin32_file_link (const char *src, const char *dst)
{
  permissions_t mode;
  user_id_t uid                  = INSTALL_NULL_UID;
  group_id_t gid                 = INSTALL_NULL_GID;
  struct install_status_t status = INSTALL_STATUS_INIT;

  assert (src != NULL);
  assert (dst != NULL);

  if (install_callback_warn) {
    install_callback_warn ("Filesystem does not support symlinks - copying",
      install_callback_data);
  }

  /* Only >= Vista supports symlinks */
  if (!iwin32_file_ownership_get (src, &uid, &gid)) return 0;
  if (!iwin32_file_mode_get      (src, &mode)) return 0;

  status = install_file_copy (src, dst, uid, gid, mode);
  return status.status == INSTALL_STATUS_OK;
}

/*
 * Misc.
 */

static unsigned int
iwin32_umask (unsigned int m)
{
  return m;
}

static int
iwin32_can_set_ownership (user_id_t uid)
{
  assert (uid.value != NULL);
  return iwin32_uid_compare (uid, administrator_id) == 1;
}

static int
iwin32_supports_symlinks (void)
{
  return 0;
}

static int
iwin32_supports_posix_modes (void)
{
  return 0;
}

static struct install_status_t
iwin32_init (void)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  status.status = INSTALL_STATUS_OK;

  if (!iwin32_uid_lookup ("Administrator", &administrator_id)) {
    install_status_assign (&status, INSTALL_STATUS_ERROR,
      "could not fetch Administrator user ID");
  }

  memcpy (inst_exec_suffix, ".exe", 4);
  return status;
}

const struct install_platform_callbacks_t install_platform_win32 = {
  &iwin32_init,

  &iwin32_error_message,
  &iwin32_error_message_current,
  &iwin32_error_current,
  &iwin32_error_reset,

  &iwin32_gid_compare,
  &iwin32_gid_format,
  &iwin32_gid_scan,
  &iwin32_gid_current,
  &iwin32_gid_lookup,
  &iwin32_gid_valid,
  &iwin32_gid_free,

  &iwin32_uid_compare,
  &iwin32_uid_format,
  &iwin32_uid_scan,
  &iwin32_uid_current,
  &iwin32_uid_lookup,
  &iwin32_uid_valid,
  &iwin32_uid_free,

  &iwin32_user_name_current,

  &iwin32_mkdir,

  &iwin32_file_mode_get,
  &iwin32_file_mode_set,
  &iwin32_file_ownership_get,
  &iwin32_file_ownership_set,
  &iwin32_file_size,
  &iwin32_file_link,

  &iwin32_can_set_ownership,
  &iwin32_supports_symlinks,
  &iwin32_supports_posix_modes,
  &iwin32_umask,
};

#endif
