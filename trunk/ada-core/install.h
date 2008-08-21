#ifndef INSTALL_H
#define INSTALL_H

#define INSTALL_DRYRUN 0x0001

#define INST_COPY 0
#define INST_SYMLINK 1
#define INST_MKDIR 2
#define INST_LIBLINK 3

struct install_item {
  int op;
  char *src;
  char *dst;
  char *dir;
  char *owner;
  char *group;
  int perm;
};

int install(struct install_item *, unsigned int);
int install_check(struct install_item *);
int deinstall(struct install_item *, unsigned int);

const char *install_error(int);
int check_tools(void);

extern struct install_item insthier[];
extern unsigned long insthier_len;
extern unsigned long install_failed;

#endif
