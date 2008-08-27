#include <agar/core.h>
#include <agar/gui.h>
#include <stdio.h>

int
main (void)
{
#if defined (HAVE_OPENGL)
  printf ("opengl\n");
#else
  printf ("no opengl\n");
#endif
  return 0;
}
