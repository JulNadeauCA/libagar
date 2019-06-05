/* $Rev$ */

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "install.h"

#define COPY "copy"
#define COPYBUF_SIZE 8192

char copybuf[COPYBUF_SIZE];
char tmpdst[1024];
char *src;
char *dst;
int uid;
int gid;
unsigned int perm;

void complain(const char *s)
{
  if (s)
    printf("error: %s: %s\n", s, install_error(errno));
  else
    printf("error: %s\n", install_error(errno));
}
void say()
{
  int t_uid;
  int t_gid;
  t_uid = (uid == -1) ? (int) getuid() : uid;
  t_gid = (gid == -1) ? (int) getgid() : gid;
  printf(COPY" %s %s %d:%d %o\n", src, dst, t_uid, t_gid, perm);
  fflush(0);
}
int copy()
{
  int srcfd;
  int dstfd;
  int r;
  int w;
  int code;

  if (snprintf(tmpdst, 1024, "%s.tmp", dst) < 0) return 112;
  srcfd = open(src, O_RDONLY);
  if (srcfd == -1) { complain("open"); return 113; }
  dstfd = open(tmpdst, O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if (dstfd == -1) { complain("open"); code = 114; goto ERR; }

  for (;;) {
    r = read(srcfd, copybuf, COPYBUF_SIZE);
    if (r == -1) { complain("read"); code = 115; goto ERR; }
    if (r == 0) break;
    while (r) {
      w = write(dstfd, copybuf, r);
      if (w == -1) { complain("read"); code = 116; goto ERR; }
      if (w == 0) break;
      r -= w;
    }
  }

  if (fsync(dstfd) == -1) { complain("fsync"); code = 117; goto ERR; }
  if (chmod(tmpdst, perm) == -1) { complain("chmod"); code = 118; goto ERR; }
  if (chown(tmpdst, uid, gid) == -1) {
    complain("chown"); code = 119; goto ERR;
  }
  if (rename(tmpdst, dst) == -1) { complain("rename"); code = 120; goto ERR; }
  if (close(dstfd) == -1) complain("close");
  if (close(srcfd) == -1) complain("close");
  return 0;
  ERR:
  if (unlink(tmpdst) == -1) complain("unlink");
  return code;
}

int main(int argc, char *argv[])
{
  --argc;
  ++argv;

  if (argc < 5) return 111;

  src = argv[0];
  dst = argv[1];
  if (!sscanf(argv[2], "%d", &uid)) return 111;
  if (!sscanf(argv[3], "%d", &gid)) return 111;
  if (!sscanf(argv[4], "%o", &perm)) return 111;

  say();

  if (argc < 6) return copy();
  return 0;
}
