/* $Rev$ */

#include "install.h"

const char progname[] = "deinstaller";

int main(int argc, char *argv[])
{
  unsigned long i;
  unsigned int flag;

  argv = 0;
  if (!check_tools()) return 112;

  flag = (argc > 1) ? INSTALL_DRYRUN : 0;
  for (i = insthier_len - 1;; --i) {
    deinstall(&insthier[i], flag);
    if (i == 0) break;
  }

  return 0;
}
