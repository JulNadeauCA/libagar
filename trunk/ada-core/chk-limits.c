#include <stdio.h>
#include <agar/core.h>

int
main (void)
{
  printf ("  pathname_max : constant := %u;\n", AG_PATHNAME_MAX);
  printf ("  filename_max : constant := %u;\n", AG_FILENAME_MAX);
  printf ("  buffer_min   : constant := %u;\n", AG_BUFFER_MIN);
  printf ("  buffer_max   : constant := %u;\n", AG_BUFFER_MAX);
  return 0;
}
