/* $Rev$ */

#include <stdio.h>
#include "install.h"

const char progname[] = "instchk";

int main()
{
  unsigned long i;
  if (!check_tools()) return 112;

  for (i = 0; i < insthier_len; ++i)
    install_check(&insthier[i]);

  if (install_failed) {
    printf("%lu of %lu files failed\n", install_failed, insthier_len);
    return 1;
  }
  return 0;
}
