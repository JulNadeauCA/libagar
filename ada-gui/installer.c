/* $Rev$ */

#include "install.h"

const char progname[] = "installer";

int main(int argc, char *argv[])
{
  unsigned long i;
  unsigned int flag;

  argv = 0;
  if (!check_tools()) return 112;

  flag = (argc > 1) ? INSTALL_DRYRUN : 0;
  for (i = 0; i < insthier_len; ++i)
    install(&insthier[i], flag);

  return 0;
}
