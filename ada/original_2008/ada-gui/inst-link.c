/* $Rev$ */

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "install.h"

#define LINK "link"

char *dir;
char *src;
char *dst;

void say()
{
  printf(LINK" %s %s %s\n", dir, src, dst);
  fflush(0);
}
void complain(const char *s)
{
  if (s)
    printf("failed: %s: %s\n", s, install_error(errno));
  else
    printf("failed: %s\n", install_error(errno));
}
int create_link()
{
  if (chdir(dir) == -1) { complain("chdir"); return 113; }
  if (symlink(src, dst) == -1) { complain("symlink"); return 114; }
  return 0;
}
int main(int argc, char *argv[])
{
  --argc;
  ++argv;

  if (argc < 3) return 111;

  dir = argv[0];
  src = argv[1];
  dst = argv[2];

  say();

  if (argc < 4) return create_link();
  return 0;
}
