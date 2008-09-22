#!/bin/sh

if [ $# -ne 1 ]
then
  echo "usage: typemap" 1>&2
  exit 111
fi

map="$1"

(cat <<EOF
/* auto generated, do not edit */

#include <agar/core.h>
#include <agar/gui.h>

/* XXX: private */
typedef unsigned int Uint;
#include <agar/gui/titlebar.h>

#include <stdio.h>
#include <string.h>

struct {
  const char *type_name;
  unsigned int type_size;
} types[] = {
EOF
) || exit 112

./types-c-size.sh "${map}" || exit 112

(cat <<EOF
};
const unsigned int types_size = sizeof (types) / sizeof (types[0]);

void
find (const char *name)
{
  unsigned int pos;
  for (pos = 0; pos < types_size; ++pos) {
    if (strcmp (types[pos].type_name, name) == 0) {
      printf ("%u\n", types[pos].type_size * 8);
      return;
    }
  }
  fprintf (stderr, "fatal: unknown C type\n");
  exit (112);
}

int
main (int argc, char *argv[])
{
  if (argc != 2) exit (111);
  find (argv[1]);
  return 0;
}
EOF
) || exit 112
