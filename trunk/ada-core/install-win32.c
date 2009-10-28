#define INSTALL_IMPLEMENTATION
#include "install.h"

#if INSTALL_OS_TYPE == INSTALL_OS_WIN32

#include <sys/stat.h>
#include <windows.h>
#include <sddl.h>
#include <io.h>
#include <stdio.h>

#define BUFFER_SIZE 256

static int
iwin32_get_sid (const char *name, PSID *ext_sid)
{
  char domain [BUFFER_SIZE];
  DWORD domain_size = BUFFER_SIZE;
  DWORD sid_size = BUFFER_SIZE;
  SID *sid;
  SID_NAME_USE account_type;

  sid = malloc (sid_size);
  if (!sid) return 0;

  if (!LookupAccountName (NULL, name, sid, &sid_size,
    domain, &domain_size, &account_type)) return 0;

  *ext_sid = sid;
  return 1;
}

static int
iwin32_user_primary_group (group_id_t *gid)
{
  HANDLE thread_tok;
  DWORD needed;
  TOKEN_PRIMARY_GROUP *group;

  if (!OpenProcessToken (GetCurrentProcess(),
    STANDARD_RIGHTS_READ | READ_CONTROL | TOKEN_QUERY, &thread_tok)) return 0;

  if (!GetTokenInformation (thread_tok, TokenPrimaryGroup, NULL, 0, &needed)) {
    if (!GetLastError () == ERROR_INSUFFICIENT_BUFFER) {
      group = malloc (needed);
      if (!group) return 0;
      if (GetTokenInformation (thread_tok, TokenPrimaryGroup, group, needed, &needed)) {
        gid->value = group->PrimaryGroup;
      }
      free (group);
    }
  }
  return 1;
}

int
iwin32_compare_gid (group_id_t a, group_id_t b)
{
  return EqualSid (a.value, b.value);
}

int
iwin32_compare_uid (user_id_t a, user_id_t b)
{
  return EqualSid (a.value, b.value);
}

int
iwin32_gid_current (group_id_t *gid)
{
  return iwin32_user_primary_group (gid);
}

int
iwin32_gid_lookup (const char *name, group_id_t *gid)
{
  if (name) {
    return iwin32_get_sid (name, &gid->value);
  } else {
    return iwin32_get_sid ("None", &gid->value);
  }
}

int
iwin32_uid_current (user_id_t *uid)
{
  return iwin32_get_sid ("Administrator", &uid->value);
}

int
iwin32_uid_lookup (const char *name, user_id_t *uid)
{
  if (name) {
    return iwin32_get_sid (name, &uid->value);
  } else {
    return iwin32_get_sid ("Administrator", &uid->value);
  }
}

int
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

int
iwin32_file_set_ownership (const char *file, user_id_t uid, group_id_t gid)
{
  if (!iwin32_file_set_sid (file, uid.value)) return 0;
  return 1;
}

int
iwin32_file_get_owner (const char *file, user_id_t *uid)
{
  char file_sd_buf [256];
  DWORD sd_size = 256;
  PSECURITY_DESCRIPTOR file_sd = (PSECURITY_DESCRIPTOR) &file_sd_buf;
  PSID sid;
  BOOL dummy;

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

int
iwin32_file_get_group (const char *file, group_id_t *gid)
{
  char file_sd_buf [256];
  DWORD sd_size = 256;
  PSECURITY_DESCRIPTOR file_sd = (PSECURITY_DESCRIPTOR) &file_sd_buf;
  PSID sid;
  BOOL dummy;

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

int
iwin32_file_get_ownership (const char *file, user_id_t *uid, group_id_t *gid)
{
  if (!iwin32_file_get_owner (file, uid)) return 0;
  if (!iwin32_file_get_group (file, gid)) return 0;
  return 1;
}

int
iwin32_file_set_mode (const char *file, permissions_t mode)
{
  return chmod (file, mode.value) == 0;
}

int
iwin32_file_get_mode (const char *file, permissions_t *mode)
{
  struct stat sb;

  if (stat (file, &sb) == -1) return 0;
  mode->value = sb.st_mode & 0755;
  return 1;
}

int
iwin32_file_link (const char *src, const char *dst)
{
  user_id_t uid;
  group_id_t gid;
  permissions_t mode;
  struct install_status_t status = INSTALL_STATUS_INIT;

  if (install_callback_warn) {
    install_callback_warn ("filesystem does not support symlinks - copying",
      install_callback_data);
  }

  /* only vista supports symlinks */
  if (!iwin32_file_get_ownership (src, &uid, &gid)) return 0;
  if (!iwin32_file_get_mode (src, &mode)) return 0;

  status = install_file_copy (src, dst, uid, gid, mode);
  return status.status == INSTALL_STATUS_OK;
}

struct install_status_t
iwin32_install_init (void)
{
  struct install_status_t status = INSTALL_STATUS_INIT;
  status.status = INSTALL_STATUS_OK;

  memcpy (inst_exec_suffix, ".exe", 4);
  return status; 
}

unsigned int
iwin32_fmt_gid (char *buffer, group_id_t gid)
{
  char *sid_str;
  unsigned long len;
  if (!ConvertSidToStringSid (gid.value, &sid_str)) return 0;
  len = strlen (sid_str);
  memcpy (buffer, sid_str, len);
  LocalFree (sid_str);
  return len;
}

unsigned int
iwin32_fmt_uid (char *buffer, user_id_t uid)
{
  char *sid_str;
  unsigned long len;
  if (!ConvertSidToStringSid (uid.value, &sid_str)) return 0;
  len = strlen (sid_str);
  memcpy (buffer, sid_str, len);
  LocalFree (sid_str);
  return len;
}

unsigned int
iwin32_scan_gid (const char *buffer, group_id_t *gid)
{
  if (!ConvertStringSidToSid ((char *) buffer, &gid->value)) return 0;
  return strlen (buffer);
}

unsigned int
iwin32_scan_uid (const char *buffer, user_id_t *uid)
{
  if (!ConvertStringSidToSid ((char *) buffer, &uid->value)) return 0;
  return strlen (buffer);
}

unsigned int
iwin32_umask (unsigned int m)
{
  return m;
}

int
iwin32_mkdir (const char *dir, unsigned int mode)
{
  return mkdir (dir);
}

void
iwin32_uid_free (user_id_t *uid)
{
  free (uid->value);
}

void
iwin32_gid_free (group_id_t *gid)
{
  free (gid->value);
}

#endif
