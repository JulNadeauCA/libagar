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
  /*@dependent@*/ /*@null@*/ const char *error_message;
};

#define INSTALL_STATUS_INIT {INSTALL_STATUS_FATAL,0,0}

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

typedef struct {
  unsigned int value;
} permissions_t;

typedef struct {
  unsigned long value;
} error_t;

enum install_file_type_t {
  INSTALL_FILE_TYPE_FILE,
  INSTALL_FILE_TYPE_CHARACTER_SPECIAL,
  INSTALL_FILE_TYPE_DIRECTORY,
  INSTALL_FILE_TYPE_SYMLINK,
  INSTALL_FILE_TYPE_SOCKET,
  INSTALL_FILE_TYPE_FIFO
};

extern char inst_exec_suffix [16];
extern char inst_dlib_suffix [16];

extern void (*install_callback_warn)(const char *, void *);
extern void (*install_callback_info)(const char *, void *);
extern void  *install_callback_data;

void install_permissions_assign (permissions_t *, int);
int  install_permissions_compare (permissions_t, permissions_t);

unsigned int install_umask (unsigned int);

int install_file_type (const char *, enum install_file_type_t *, int);
int install_file_type_lookup (const char *, enum install_file_type_t *);
int install_file_type_name_lookup (enum install_file_type_t, /*@dependent@*/ const char **);

#include "install_os.h"

void install_status_assign (struct install_status_t *, enum install_status_enum_t, /*@null@*/ /*@dependent@*/ const char *);
struct install_status_t install_file_copy (const char *, const char *, user_id_t, group_id_t, permissions_t);

struct install_platform_callbacks_t {
  struct install_status_t (*init) (void);

  const char * (*error_message) (error_t);
  const char * (*error_message_current) (void);
  error_t      (*error_current) (void);
  void         (*error_reset) (void);

  int          (*gid_compare) (group_id_t, group_id_t);
  unsigned int (*gid_format) (char *, group_id_t);
  unsigned int (*gid_scan) (const char *, group_id_t *);
  int          (*gid_current) (group_id_t *);
  int          (*gid_lookup) (const char *, group_id_t *);
  int          (*gid_valid) (group_id_t);
  void         (*gid_free) (group_id_t *);

  int          (*uid_compare) (user_id_t,  user_id_t);
  unsigned int (*uid_format) (char *, user_id_t);
  unsigned int (*uid_scan) (const char *, user_id_t *);
  int          (*uid_current) (user_id_t *);
  int          (*uid_lookup) (const char *, user_id_t *);
  int          (*uid_valid) (user_id_t);
  void         (*uid_free) (user_id_t *);

  const char * (*user_name_current) (void);

  int          (*make_dir) (const char *, permissions_t);

  int          (*file_mode_get) (const char *, permissions_t *);
  int          (*file_mode_set) (const char *file, permissions_t mode);
  int          (*file_ownership_get) (const char *, user_id_t *, group_id_t *);
  int          (*file_ownership_set) (const char *, user_id_t, group_id_t);
  int          (*file_size) (const char *, size_t *);
  int          (*file_link) (const char *, const char *);

  int          (*can_set_ownership) (user_id_t);
  int          (*supports_symlinks) (void);
  int          (*supports_posix_modes) (void);
  unsigned int (*umask) (unsigned int);
};

extern const struct install_platform_callbacks_t install_platform_posix;
extern const struct install_platform_callbacks_t install_platform_win32;

#endif /* INSTALL_IMPLEMENTATION */

#endif /* INSTALL_H */
