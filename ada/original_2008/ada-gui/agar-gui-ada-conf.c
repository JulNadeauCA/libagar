#include <stdio.h>
#include <stdlib.h>
#include "ctxt.h"

static unsigned int flag;
static int str_diff(register const char *, register const char *);

#define FLAG_INCDIR  0x0001
#define FLAG_DLIBDIR 0x0002
#define FLAG_SLIBDIR 0x0004
#define FLAG_NEWLINE 0x0008
#define FLAG_VERSION 0x0010
#define FLAG_COMPILE 0x0020
#define FLAG_HELP    0x0040
#define FLAG_CFLAGS  0x0080
#define FLAG_LDFLAGS 0x0100

/* PROJECT SPECIFIC */

#include "_sysinfo.h"

const char progname[] = "agar-gui-ada-conf";

void flag_incdir(void)
{
  if (flag & FLAG_COMPILE)
    printf("-I%s ", ctxt_incdir);
  else
    printf("%s ", ctxt_incdir);
}
void flag_cflags(void)
{

}
void flag_dlibdir(void)
{
  if (flag & FLAG_COMPILE) printf("-L");
  printf("%s ", ctxt_dlibdir);
}
void flag_slibdir(void)
{
  if (flag & FLAG_COMPILE) printf("-L");
  printf("%s ", ctxt_slibdir);
}
void flag_ldflags(void)
{
  printf("-lagar-gui-ada ");
}

/* PROJECT SPECIFIC END */

void flag_incdir(void);
void flag_dlibdir(void);
void flag_slibdir(void);
void flag_cflags(void);
void flag_ldflags(void);

static void flag_version(void);
static void flag_help(void);
static void flag_newline(void);

static const struct {
  const char *flag;
  void (*func)(void);
  unsigned int val;
  const char *desc;
} flags[] = {
  { "help",    flag_help,    FLAG_HELP,    "this message" },
  { "compile", 0,            FLAG_COMPILE, "modify output for use as compiler flags" },
  { "incdir",  flag_incdir,  FLAG_INCDIR,  "print include directory" },
  { "cflags",  flag_cflags,  FLAG_CFLAGS,  "output required compiler flags" },
  { "dlibdir", flag_dlibdir, FLAG_DLIBDIR, "print dynamic library directory" },
  { "slibdir", flag_slibdir, FLAG_SLIBDIR, "print static library directory" },
  { "ldflags", flag_ldflags, FLAG_LDFLAGS, "output required linker flags" },
  { "version", flag_version, FLAG_VERSION, "print library version" },
  { "newline", flag_newline, FLAG_NEWLINE, "print trailing newline" },
};

static void usage(void) { printf("%s: [help] ops ...\n", progname); }
static void help(void)
{
  unsigned int ind;
  usage();
  printf("possible operators:\n");
  for (ind = 0; ind < sizeof(flags) / sizeof(flags[0]); ++ind)
    printf("%s - %s\n", flags[ind].flag, flags[ind].desc);
  exit(0);
}
static void parse_flags(int argc, char *argv[])
{
  int ind;
  unsigned int jnd;

  --argc;
  ++argv;

  if (!argc) { usage(); exit(111); }

  flag = 0;
  for (ind = 0; ind < argc; ++ind) {
    for (jnd = 0; jnd < sizeof(flags) / sizeof(flags[0]); ++jnd) {
      if (str_diff(argv[ind], flags[jnd].flag) == 0) {
        flag |= flags[jnd].val;
        break;
      }
    }
  }
}
static void call_flags(void)
{
  unsigned int ind;
  for (ind = 0; ind < sizeof(flags) / sizeof(flags[0]); ++ind) {
    if (flag & flags[ind].val)
      if (flags[ind].func)
        flags[ind].func();
  }
}
static void flag_version(void) { printf("%s ", ctxt_version); }
static void flag_newline(void) { printf("\n"); }
static void flag_help(void) { help(); }
int main(int argc, char *argv[])
{
  parse_flags(argc, argv);
  call_flags();

  if (fflush(0) != 0) return 112;
  return 0;
}

/* portability functions */
static int str_diff(register const char *s, register const char *t)
{
  register char u;
  for (;;) {
    u = *s; if (u != *t) break; if (!u) break; ++s; ++t;
    u = *s; if (u != *t) break; if (!u) break; ++s; ++t;
    u = *s; if (u != *t) break; if (!u) break; ++s; ++t;
    u = *s; if (u != *t) break; if (!u) break; ++s; ++t;
  }
  return ((int)(unsigned int)(unsigned char) u) - 
         ((int)(unsigned int)(unsigned char) *t);
}
