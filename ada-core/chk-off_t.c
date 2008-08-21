#include <sys/types.h>
#include <stdio.h>

int
main (void)
{
  switch (sizeof (off_t)) {
     case 1: printf ("8  -16#7f# 16#7f#\n"); return 0;
     case 2: printf ("16 -16#7fff# 16#7fff#\n"); return 0;
     case 4: printf ("32 -16#7fffffff# 16#7fffffff#\n"); return 0;
     case 8: printf ("64 -16#7fffffffffffffff# 16#7fffffffffffffff#\n"); return 0;
    case 16: printf ("128 -16#7fffffffffffffffffffffffffffffff# 16#7fffffffffffffffffffffffffffffff#\n"); return 0;
    default: fprintf (stderr, "fatal: unknown off_t size\n"); return 1;
  }
  return 1;
}
