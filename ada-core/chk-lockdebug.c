#include <agar/core.h>
#include <stdio.h>

int
main (void)
{
#if defined (AG_LOCKDEBUG)
  printf ("lockdebug\n");
#else
  printf ("no lockdebug\n");
#endif
  return 0;
}
