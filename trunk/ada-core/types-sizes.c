#include <stdio.h>
#include <stdlib.h>

#include <agar/core.h>

struct type_and_size_t {
  const char *name;
  size_t      size;
};

static const struct type_and_size_t types[] = {
  { "AG_DSO",          sizeof (AG_DSO),          },
  { "AG_DataSource",   sizeof (AG_DataSource),   },
  { "AG_Db",           sizeof (AG_Db),           },
  { "AG_Event",        sizeof (AG_Event),        },
  { "AG_List",         sizeof (AG_List),         },
  { "AG_Object",       sizeof (AG_Object),       },
  { "AG_ObjectClass",  sizeof (AG_ObjectClass),  },
  { "AG_ObjectDep",    sizeof (AG_ObjectDep),    },
  { "AG_ObjectHeader", sizeof (AG_ObjectHeader), },
  { "AG_Variable",     sizeof (AG_Variable),     },
};

int
main (int argc, char *argv[])
{
  unsigned int index;

  if (argc < 2) exit (EXIT_FAILURE);

  for (index = 0; index < sizeof (types) / sizeof (types [0]); ++index) {
    if (strcmp (types [index].name, argv [1]) == 0) {
      printf ("%lu\n", types [index].size);
      exit (EXIT_SUCCESS);
    }
  }

  exit (EXIT_FAILURE);
}
