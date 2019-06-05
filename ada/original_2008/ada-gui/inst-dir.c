/* $Rev$ */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "install.h"

#define MKDIR "mkdir"
#define MAX_PATHLEN 1024

char *dir;
int uid;
int gid;
unsigned int perm;

void die()
{
  printf("failed: %s\n", install_error(errno));
  fflush(0);
  _exit(112);
}
void say()
{
  int t_uid;
  int t_gid;
  t_uid = (uid == -1) ? (int) getuid() : uid;
  t_gid = (gid == -1) ? (int) getgid() : gid;
  printf(MKDIR" %s %d:%d %o\n", dir, t_uid, t_gid, perm);
  fflush(0);
}
int rmkdir()
{
  char pbuf[MAX_PATHLEN];
  unsigned int len;
  unsigned int pos;
  unsigned int buflen;
  unsigned int bufpos;
  int end;
  int fd;
  char *ptr;
  char *ptr2;

  buflen = MAX_PATHLEN;
  bufpos = 0;
  end = 0;
  len = strlen(dir);
  ptr = dir;

  if (len >= MAX_PATHLEN) return 113;

  for (;;) {
    if (!len) break;
    ptr2 = strchr(ptr, '/');
    if (!ptr2) {
      pos = len;
      end = 1;
    } else pos = ptr2 - ptr;
    if (buflen <= (unsigned int) pos + 1) break;
    memcpy(pbuf + bufpos, ptr, pos);
    bufpos += pos;
    buflen -= pos;
    pbuf[bufpos] = '/';
    ++bufpos;
    --buflen;
    pbuf[bufpos] = 0;
    if (mkdir(pbuf, perm) == -1) {
      if (!end) {
        if (errno != EEXIST && errno != EISDIR) die();
      } else die();
    }
    ptr += pos;
    len -= pos;
    if (len) {
      ++ptr;
      --len;
      if (!len) break;
    }
  }
  fd = open(pbuf, O_RDONLY);
  if (fd == -1) die();
  if (fchown(fd, uid, gid) == -1) die();
  if (close(fd) == -1) die();
  return 0;
}

int main(int argc, char *argv[])
{
  --argc;
  ++argv;

  if (argc < 4) return 111;

  dir = argv[0];
  if (!sscanf(argv[1], "%d", &uid)) return 111;
  if (!sscanf(argv[2], "%d", &gid)) return 111;
  if (!sscanf(argv[3], "%o", &perm)) return 111;

  say();

  if (argc < 5) return rmkdir();
  return 0;
}
