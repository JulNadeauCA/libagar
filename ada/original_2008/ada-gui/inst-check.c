/* $Rev$ */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "install.h"

#define CHECK "check"
#define str_same(s,t) (strcmp((s),(t)) == 0)
#define str_nsame(s,t,n) (strncmp((s),(t),(n)) == 0)

#ifndef S_ISSOCK
  #if defined(S_IFMT) && defined(S_IFSOCK)
    #define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
  #else
    #define S_ISSOCK(mode)
  #endif
#endif

char *file;
char *type;
int uid;
int gid;
unsigned int perm;

int s_ifreg(int);
int s_ifchr(int);
int s_ifdir(int);
int s_iflnk(int);
int s_ifsock(int);
int s_ififo(int);

static const struct {
  int (*check)(int);
  const char *name;
} types[] = {
  { &s_ifreg, "file" },
  { &s_ifchr, "character_special" },
  { &s_ifdir, "directory" },
  { &s_iflnk, "symlink" },
  { &s_ifsock, "socket" },
  { &s_ififo, "fifo" },
};

void die()
{
  printf("failed: %s\n", install_error(errno));
  fflush(0);
  _exit(112);
}
void say()
{
  printf(CHECK" %s %d:%d %o %s\n", file, uid, gid, perm, type);
  fflush(0);
}

int s_ifreg(int m) { return S_ISREG(m); }
int s_ifchr(int m) { return S_ISCHR(m); }
int s_ifdir(int m) { return S_ISDIR(m); }
int s_iflnk(int m) { return S_ISLNK(m); }
int s_ifsock(int m) { return S_ISSOCK(m); }
int s_ififo(int m) { return S_ISFIFO(m); }

int check_type(int mode)
{
  return 1;
}
int check()
{
  struct stat sb;
  int fd;
  unsigned int got_mode;
  unsigned int want_mode;

  if (!str_nsame(type, "symlink", strlen("symlink"))) {
    fd = open(file, O_RDONLY);
    if (fd == -1) die();
    if (fstat(fd, &sb) == -1) die();
  } else if (lstat(file, &sb) == -1) die();

  if (!check_type(sb.st_mode)) return 1;

  want_mode = perm;
  got_mode = (unsigned int) sb.st_mode & 07777;

  if (!S_ISLNK(sb.st_mode)) {
    if (got_mode != want_mode) {
      printf("failed: mode %o not %o\n", got_mode, want_mode);
      return 1;
    }
    if (uid >= 0) {
      if (sb.st_uid != (unsigned) uid) {
        printf("failed: uid %d not %d\n", sb.st_uid, uid);
        return 1;
      }
    }
    if (gid >= 0) {
      if (sb.st_uid != (unsigned) uid) {
        printf("failed: gid %d not %d\n", sb.st_gid, gid);
        return 1;
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[])
{
  --argc;
  ++argv;

  if (argc < 5) return 111;

  file = argv[0];
  if (!sscanf(argv[1], "%d", &uid)) return 111;
  if (!sscanf(argv[2], "%d", &gid)) return 111;
  if (!sscanf(argv[3], "%o", &perm)) return 111;
  type = argv[4];

  say();

  if (argc < 6) return check();
  return 0;
}
