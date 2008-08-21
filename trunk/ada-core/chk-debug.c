#include <agar/core.h>
#include <stdio.h>

int
main (void)
{
#if defined (DEBUG)
  printf ("debug\n");
#else
  printf ("no debug\n");
#endif
  return 0;
}
