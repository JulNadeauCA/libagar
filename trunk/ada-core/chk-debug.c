#include <agar/core.h>
#include <stdio.h>

int
main (void)
{
#if defined (AG_DEBUG)
  printf ("debug\n");
#else
  printf ("no debug\n");
#endif
  return 0;
}
