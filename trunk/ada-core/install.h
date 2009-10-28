#ifndef INSTALL_H
#define INSTALL_H

#include <errno.h>

#define INSTALL_NO_FLAGS 0x0000u
#define INSTALL_DRYRUN   0x0001u

enum install_op_t {
  INST_COPY       = 0,
  INST_COPY_EXEC  = 1,
  INST_SYMLINK    = 2,
  INST_MKDIR      = 3,
  INST_LIBLINK    = 4
};

enum install_status_enum_t {
  INSTALL_STATUS_OK    = 0,
  INSTALL_STATUS_ERROR = 1,
  INSTALL_STATUS_FATAL = 2
};

struct install_status_t {
  enum install_status_enum_t status;
  /*@dependent@*/ /*@null@*/ const char *message;
};

#define INSTALL_STATUS_INIT {INSTALL_STATUS_FATAL,0}

struct install_item {
  enum install_op_t op;
  /*@dependent@*/ char *src;
  /*@dependent@*/ char *dst;
  /*@dependent@*/ char *dir;
  /*@dependent@*/ char *owner;
  /*@dependent@*/ char *group;
  int perm;
};

struct install_status_t install_init (const char *);
struct install_status_t install (struct install_item *, unsigned int);
struct install_status_t install_check (struct install_item *);
struct install_status_t deinstall (struct install_item *, unsigned int);

void install_callback_warn_set (void (*)(const char *, void *));
void install_callback_info_set (void (*)(const char *, void *));
void install_callback_data_set (void *);

void install_fake_root (const char *);

const char *install_error (int);
extern struct install_item insthier[];
extern unsigned long insthier_len;
extern unsigned long install_failed;

#ifdef INSTALL_IMPLEMENTATION

#define INSTALL_MAX_PATHLEN      (size_t) 1024
#define INSTALL_MAX_MSGLEN       (size_t) 8192
#define INSTALL_NULL_USER_NAME   ":"
#define INSTALL_NULL_GROUP_NAME  ":"
#define INSTALL_NULL_PERMISSIONS {0}

extern char inst_exec_suffix [16];
extern char inst_dlib_suffix [16];

extern void (*install_callback_warn)(const char *, void *);
extern void (*install_callback_info)(const char *, void *);
extern void *install_callback_data;

typedef struct {
  unsigned int value;
} permissions_t;

#include "install_os.h"

enum install_file_type_t {
  INSTALL_FILE_TYPE_FILE,
  INSTALL_FILE_TYPE_CHARACTER_SPECIAL,
  INSTALL_FILE_TYPE_DIRECTORY,
  INSTALL_FILE_TYPE_SYMLINK,
  INSTALL_FILE_TYPE_SOCKET,
  INSTALL_FILE_TYPE_FIFO
};

void install_status_assign (struct install_status_t *, enum install_status_enum_t, /*@null@*/ /*@dependent@*/ const char *);
void install_perms_assign (permissions_t *, unsigned int);
int install_compare_gid (group_id_t, group_id_t);
int install_compare_uid (user_id_t, user_id_t);
int install_compare_permissions (permissions_t, permissions_t);
int install_file_get_mode (const char *, permissions_t *);
int install_file_set_mode (const char *file, permissions_t mode);
int install_file_get_ownership (const char *, user_id_t *, group_id_t *);
int install_file_link (const char *, const char *);
int install_file_set_ownership (const char *, user_id_t, group_id_t);
int install_file_size (const char *, size_t *);
int install_file_type (const char *, enum install_file_type_t *, int);
int install_file_type_lookup (const char *, enum install_file_type_t *);
int install_file_type_name_lookup (enum install_file_type_t, /*@dependent@*/ const char **);
int install_gid_current (group_id_t *);
int install_gid_lookup (const char *, group_id_t *);
int install_mkdir (const char *, unsigned int);
int install_uid_current (user_id_t *);
int install_uid_lookup (const char *, user_id_t *);
struct install_status_t install_file_copy (const char *, const char *, user_id_t, group_id_t, permissions_t);
unsigned int install_fmt_gid (char *, group_id_t);
unsigned int install_fmt_uid (char *, user_id_t);
unsigned int install_scan_gid (const char *, group_id_t *);
unsigned int install_scan_uid (const char *, user_id_t *);
unsigned int install_umask (unsigned int);
void install_gid_free (group_id_t *);
void install_uid_free (user_id_t *);
#endif

#endif
