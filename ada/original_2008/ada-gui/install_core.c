/* $Rev$ */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "install.h"

#define wait_exitcode(w) ((w) >> 8)

#define EXT_INST_COPY ext_tools[0]
#define EXT_INST_CHECK ext_tools[1]
#define EXT_INST_DIR ext_tools[2]
#define EXT_INST_LINK ext_tools[3]
#define EXT_INST_SOSUFFIX ext_tools[4]
char *ext_tools[] = {
  "./inst-copy",
  "./inst-check",
  "./inst-dir",
  "./inst-link",
  "./mk-sosuffix",
};

#define MAX_PATHLEN 1024
#define MAX_MSGLEN 8192

#define str_same(a,b) (strcmp((a),(b)) == 0)

extern const char progname[];
extern char **environ;

char src_name[MAX_PATHLEN];
char dst_name[MAX_PATHLEN];
char dir_name[MAX_PATHLEN];
char src_tmp[MAX_PATHLEN];
char dst_tmp[MAX_PATHLEN];
char dir_tmp[MAX_PATHLEN];
char tmp_buf[MAX_PATHLEN];
char msg_buf[MAX_MSGLEN];

char *task_args[16];
int task_pipe[2];
int uid;
int gid;

char uidbuf[16];
char gidbuf[16];
char permbuf[16];

unsigned long install_failed;

/* error functions */
int fails_sys(const char *s)
{
  printf("failed: %s: %s\n", s, install_error(errno));
  return 0;
}
int fails(const char *s)
{
  printf("failed: %s\n", s);
  return 0;
}
void fail()
{
  printf("failed: %s\n", install_error(errno));
}
void fail_noread()
{
  printf("failed: no bytes read\n");
}

/* portability functions */
void mem_copy(void *dst, const void *src, unsigned long len)
{
  char ch;
  char *cdst;
  const char *csrc;

  csrc = (const char *) src;
  cdst = (char *) dst;
  for (;;) {
    if (!len) break; ch = *csrc; *cdst = ch; ++cdst; ++csrc; --len;
    if (!len) break; ch = *csrc; *cdst = ch; ++cdst; ++csrc; --len;
    if (!len) break; ch = *csrc; *cdst = ch; ++cdst; ++csrc; --len;
    if (!len) break; ch = *csrc; *cdst = ch; ++cdst; ++csrc; --len;
  }
}
int base_name(const char *dir, char **out)
{
  static char path[MAX_PATHLEN];
  const char *s;
  const char *t;
  const char *u;
  unsigned int len;
  unsigned int nlen;

  len = strlen(dir); 

  if (!len) {
    path[0] = '.';
    path[1] = 0;
    *out = path;
    return 1;
  }

  if (len >= MAX_PATHLEN) return 0;

  s = dir;
  t = s + (len - 1);
  while ((t > s) && (t[0] == '/')) --t;

  if ((t == s) && (t[0] == '/')) {
    path[0] = '/';
    path[1] = 0;
    *out = path;
    return 1;
  }
  u = t;
  while ((u > s) && (*(u - 1) != '/')) --u;

  nlen = (t - u) + 1;
  mem_copy(path, u, nlen);
  path[nlen] = 0;

  *out = path;
  return 1;
}
int str_ends(const char *s, const char *end)
{
  register unsigned long slen;
  register unsigned long elen;
  slen = strlen(s);
  elen = strlen(end);
  if (elen > slen) elen = slen;
  s += (slen - elen);
  return str_same(s, end);
}

/* utilities */
int lookup_uid(const char *user, int *uid)
{
  struct passwd *pwd;

  pwd = getpwnam(user);
  if (!pwd) return 0;

  *uid = pwd->pw_uid;
  return 1;
}
int lookup_gid(const char *group, int *gid)
{
  struct group *grp;

  grp = getgrnam(group);
  if (!grp) return 0;

  *gid = grp->gr_gid;
  return 1;
}
int lookup(struct install_item *ins, int *uid, int *gid)
{
  if (ins->owner) {
    if (!lookup_uid(ins->owner, uid)) return 0; 
  } else *uid = -1; 
  if (ins->group) {
    if (!lookup_gid(ins->group, gid)) return 0;
  } else *gid = -1;
  return 1;
}
int task_pipes()
{
  if (pipe(task_pipe) == -1) { fail(); return -1; }
  return 0;
}
int task_close()
{
  if (close(task_pipe[0]) == -1) { fail(); return -1; }
  return 0;
}
int task()
{
  int pid;
  switch (pid = fork()) {
    case 0:
      if (close(1) == -1) return 113;
      if (dup(task_pipe[1]) == -1) return 114;
      if (close(task_pipe[0]) == -1) return 115;
      if (execve(task_args[0], task_args, 0) == -1) return 116;
      break;
    case -1:
      fails_sys("fork");
      return -1;
    default:
      if (close(task_pipe[1]) == -1) return 116;
      break;
  }
  return pid;
}
int task_read(char *buf, unsigned int len)
{
  return read(task_pipe[0], buf, len);
}
void task_echo(char *buf, unsigned int len)
{
  int r;
  for (;;) {
    r = task_read(buf, len);
    if (r == -1) { fails_sys("read"); break; }
    if (r == 0) break;
    buf[r - 1] = 0;
    printf("%s\n", buf);
  }
}
int libname(char *name, char *buf)
{
  char *s;
  char rbuf[MAX_PATHLEN];
  int r;
  int fd;
  int ret;
  int clean;

  ret = 1;

  fd = open(name, O_RDONLY);
  if (fd == -1) return fails_sys(name);

  r = read(fd, rbuf, MAX_PATHLEN);
  if (r == 0) { ret = 0; goto END; }
  if (r == -1) { fails_sys(name); ret = 0; goto END; }

  s = rbuf;
  clean = 0;
  while (r) {
    switch (*s) {
      case ' ': case '\t': case '\n':
        s[0] = 0;
        clean = 1;
        break;
      default:
        break;
    }
    if (clean) break;
    --r; ++s;
  }
  mem_copy(buf, rbuf, s - rbuf);
  buf[s - rbuf] = 0;

  END:
  if (close(fd) == -1) fails_sys(name);
  return ret;
}
int uidgidperm_to_text(int uid, int gid, int perm)
{
  if (snprintf(uidbuf, 16, "%d", uid) < 0) return fails_sys("snprintf");
  if (snprintf(gidbuf, 16, "%d", gid) < 0) return fails_sys("snprintf");
  if (snprintf(permbuf, 16, "%o", perm) < 0) return fails_sys("snprintf");
  return 1;
}

/* install operator callbacks */
int inst_copy(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_COPY;
  task_args[1] = ins->src;
  task_args[2] = ins->dst;
  task_args[3] = uidbuf;
  task_args[4] = gidbuf;
  task_args[5] = permbuf;
  if (fl & INSTALL_DRYRUN) {
    task_args[6] = "dryrun";
    task_args[7] = 0;
  } else task_args[6] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");

  task_echo(msg_buf, MAX_MSGLEN);
  return (wait_exitcode(stat) == 0);
}
int inst_link(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_LINK;
  task_args[1] = ins->dir;
  task_args[2] = ins->src;
  task_args[3] = ins->dst;
  if (fl & INSTALL_DRYRUN) {
    task_args[4] = "dryrun";
    task_args[5] = 0;
  } else task_args[4] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");

  task_echo(msg_buf, MAX_MSGLEN);
  return (wait_exitcode(stat) == 0);
}
int inst_mkdir(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_DIR;
  task_args[1] = ins->dir;
  task_args[2] = uidbuf;
  task_args[3] = gidbuf;
  task_args[4] = permbuf;
  if (fl & INSTALL_DRYRUN) {
    task_args[5] = "dryrun";
    task_args[6] = 0;
  } else task_args[5] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");

  task_echo(msg_buf, MAX_MSGLEN);
  return (wait_exitcode(stat) == 0);
}
int inst_liblink(struct install_item *ins, unsigned int fl)
{
  return inst_link(ins, fl);
}

/* name translation callbacks */
int ntran_copy(struct install_item *ins)
{
  if (!ins->src) return fails("src file undefined");
  if (!ins->dir) return fails("directory unefined");
  if (!ins->dst) ins->dst = ins->src;

  if (str_ends(ins->src, ".vlb")) {
    if (!libname(ins->src, src_name)) return 0;
    ins->src = src_name;
  }
  if (str_ends(ins->dst, ".vlb")) {
    if (!libname(ins->dst, dst_name)) return 0;
    ins->dst = dst_name;
  }

  if (!base_name(ins->dst, &ins->dst)) return fails("invalid path");
  if (snprintf(dst_tmp, MAX_PATHLEN, "%s/%s", ins->dir, ins->dst) < 0)
    return fails_sys("snprintf");

  ins->dst = dst_tmp;
  return 1;
}
int ntran_link(struct install_item *ins)
{
  if (!ins->src) return fails("src file undefined");
  if (!ins->dir) return fails("directory unefined");
  if (!ins->dst) return fails("dst name undefined");
  return 1; 
}
int ntran_mkdir(struct install_item *ins)
{
  if (!ins->dst) ins->dst = ins->src;
  return 1;
}
int ntran_liblink(struct install_item *ins)
{
  int pid;
  int stat;
  int r;

  if (!ins->src) return fails("src file undefined");
  if (!ins->dir) return fails("directory unefined");
  if (!ins->dst) return fails("dst name undefined");

  if (str_ends(ins->src, ".vlb")) {
    if (!libname(ins->src, src_tmp)) return 0;
    ins->src = src_tmp;
    if (!base_name(ins->src, &ins->src)) return fails("invalid path");
    mem_copy(src_name, ins->src, MAX_PATHLEN);
    ins->src = src_name;
  }

  task_args[0] = EXT_INST_SOSUFFIX;
  task_args[1] = 0;
  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");

  r = task_read(tmp_buf, MAX_PATHLEN);
  if (r == -1) return fails_sys("read");
  if (r == 0) { fail_noread(); return 0; }
  tmp_buf[r - 1] = 0;

  if (!base_name(ins->dst, &ins->dst)) return fails("invalid path");
  if (snprintf(dst_tmp, MAX_PATHLEN, "%s.%s", ins->dst, tmp_buf) < 0)
    return fails_sys("snprintf");
  ins->dst = dst_tmp;

  if (task_close() == -1) return 0;
  if (task_pipes() == -1) return 0;
  return 1;
}
int ntran_chk_link(struct install_item *ins)
{
  if (!ntran_link(ins)) return 0;
  if (snprintf(dst_name, MAX_PATHLEN, "%s/%s", ins->dir, ins->dst) < 0)
    return fails_sys("sprintf");
  ins->dst = dst_name;
  return 1;
}
int ntran_chk_liblink(struct install_item *ins)
{
  if (!ntran_liblink(ins)) return 0;
  if (snprintf(dst_name, MAX_PATHLEN, "%s/%s", ins->dir, ins->dst) < 0)
    return fails_sys("sprintf");
  ins->dst = dst_name;
  return 1;
}

/* instchk operator callbacks */
int instchk_copy(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_CHECK;
  task_args[1] = ins->dst;
  task_args[2] = uidbuf;
  task_args[3] = gidbuf;
  task_args[4] = permbuf;
  task_args[5] = "file";
  task_args[6] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");
  task_echo(msg_buf, MAX_MSGLEN);

  if (wait_exitcode(stat)) ++install_failed;
  return (wait_exitcode(stat) == 0);
}
int instchk_link(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_CHECK;
  task_args[1] = ins->dst;
  task_args[2] = uidbuf;
  task_args[3] = gidbuf;
  task_args[4] = permbuf;
  task_args[5] = "symlink";
  task_args[6] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");
  task_echo(msg_buf, MAX_MSGLEN);

  if (wait_exitcode(stat)) ++install_failed;
  return (wait_exitcode(stat) == 0);
}
int instchk_mkdir(struct install_item *ins, unsigned int fl)
{
  int pid;
  int stat;

  task_args[0] = EXT_INST_CHECK;
  task_args[1] = ins->dir;
  task_args[2] = uidbuf;
  task_args[3] = gidbuf;
  task_args[4] = permbuf;
  task_args[5] = "directory";
  task_args[6] = 0;

  pid = task();
  if (pid == -1) return 0;
  if (waitpid(pid, &stat, 0) == -1) return fails_sys("task");
  task_echo(msg_buf, MAX_MSGLEN);

  if (wait_exitcode(stat)) ++install_failed;
  return (wait_exitcode(stat) == 0);
}
int instchk_liblink(struct install_item *ins, unsigned int fl)
{
  return instchk_link(ins, fl);
}

/* deinstall operator callbacks */
int deinst_copy(struct install_item *ins, unsigned int fl)
{
  printf("unlink %s\n", ins->dst);
  if (fl & INSTALL_DRYRUN) return 1;
  if (unlink(ins->dst) == -1) return fails_sys("unlink");
  return 1;
}
int deinst_link(struct install_item *ins, unsigned int fl)
{
  printf("unlink %s/%s\n", ins->dir, ins->dst);
  if (snprintf(tmp_buf, MAX_PATHLEN, "%s/%s", ins->dir, ins->dst) < 0)
    return fails_sys("snprintf");
  ins->dst = tmp_buf;

  if (fl & INSTALL_DRYRUN) return 1;
  if (unlink(ins->dst) == -1) return fails_sys("unlink");
  return 1;
}
int deinst_mkdir(struct install_item *ins, unsigned int fl)
{
  printf("rmdir %s\n", ins->dir);
  if (fl & INSTALL_DRYRUN) return 1;
  if (rmdir(ins->dir) == -1) return fails_sys("rmdir");
  return 1;
}
int deinst_liblink(struct install_item *ins, unsigned int fl)
{
  return deinst_link(ins, fl);
}

/* operator callback tables */
struct instop {
  int (*oper)(struct install_item *, unsigned int);
  int (*trans)(struct install_item *);
};
struct instop install_opers[] = {
  { inst_copy, ntran_copy },
  { inst_link, ntran_link },
  { inst_mkdir, ntran_mkdir },
  { inst_liblink, ntran_liblink },
};
struct instop instchk_opers[] = {
  { instchk_copy, ntran_copy },
  { instchk_link, ntran_chk_link },
  { instchk_mkdir, ntran_mkdir },
  { instchk_liblink, ntran_chk_liblink },
};
struct instop deinst_opers[] = {
  { deinst_copy, ntran_copy },
  { deinst_link, ntran_link },
  { deinst_mkdir, ntran_mkdir },
  { deinst_liblink, ntran_liblink },
};

/* interface */
int check_tools()
{
  unsigned int i;
  int r;
  r = 1;
  for (i = 0; i < (sizeof(ext_tools) / sizeof(const char *)); ++i) {
    if (access(ext_tools[i], X_OK) == -1) {
      printf("%s: fatal: %s missing or not executable\n", progname,
              ext_tools[i]);
      r = 0;
    }
  }
  return r;
}

int install(struct install_item *ins, unsigned int fl)
{
  int r;

  r = 1;
  if (task_pipes() == -1) return 0;
  if (!lookup(ins, &uid, &gid)) { fail(); goto CLEANUP; }
  if (!uidgidperm_to_text(uid, gid, ins->perm)) goto CLEANUP;

  r = install_opers[ins->op].trans(ins);
  if (!r) goto CLEANUP;
  r = install_opers[ins->op].oper(ins, fl);

  CLEANUP:
  fflush(0);
  task_close();
  return r;
}

int install_check(struct install_item *ins)
{
  int r;

  r = 1;
  if (task_pipes() == -1) return 0;
  if (!lookup(ins, &uid, &gid)) { fail(); goto CLEANUP; }
  if (!uidgidperm_to_text(uid, gid, ins->perm)) goto CLEANUP;

  r = instchk_opers[ins->op].trans(ins);
  if (!r) goto CLEANUP;
  r = instchk_opers[ins->op].oper(ins, 0);

  CLEANUP:
  fflush(0);
  task_close();
  return r;
}

int deinstall(struct install_item *ins, unsigned int fl)
{
  int r;

  r = 1;
  if (task_pipes() == -1) return 0;
  if (!lookup(ins, &uid, &gid)) { fail(); goto CLEANUP; }
  if (!uidgidperm_to_text(uid, gid, ins->perm)) goto CLEANUP;

  r = deinst_opers[ins->op].trans(ins);
  if (!r) goto CLEANUP;
  r = deinst_opers[ins->op].oper(ins, fl);

  CLEANUP:
  fflush(0);
  task_close();
  return r;
}
