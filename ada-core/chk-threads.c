#include <agar/core.h>
#include <stdio.h>

int
main (void)
{
#if defined (THREADS)
  printf ("threads\n");
#else
  printf ("no threads\n");
#endif
  return 0;
}
