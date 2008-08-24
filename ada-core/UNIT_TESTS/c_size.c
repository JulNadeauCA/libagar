#include <agar/core.h>

#include <stdio.h>
#include <string.h>

struct {
  const char *type_name;
  unsigned int type_size;
} types[] = {
  /* auto generated - do not edit */
  { "enum ag_byte_order", sizeof (enum ag_byte_order) },
  { "AG_DataSource", sizeof (AG_DataSource) },
  { "struct ag_data_source", sizeof (struct ag_data_source) },
  { "AG_IOStatus", sizeof (AG_IOStatus) },
  { "enum ag_io_status", sizeof (enum ag_io_status) },
  { "enum ag_seek_mode", sizeof (enum ag_seek_mode) },
  { "AG_DSO *", sizeof (AG_DSO *) },
  { "AG_DSO", sizeof (AG_DSO) },
  { "struct ag_dso", sizeof (struct ag_dso) },
  { "AG_DSOSym *", sizeof (AG_DSOSym *) },
  { "AG_DSOSym", sizeof (AG_DSOSym) },
  { "struct ag_dso_sym", sizeof (struct ag_dso_sym) },
  { "AG_EvArg", sizeof (AG_EvArg) },
  { "union evarg", sizeof (union evarg) },
  { "enum ag_evarg_type", sizeof (enum ag_evarg_type) },
  { "AG_Event", sizeof (AG_Event) },
  { "struct ag_event", sizeof (struct ag_event) },
  { "enum ag_object_checksum_alg", sizeof (enum ag_object_checksum_alg) },
  { "AG_ObjectClass", sizeof (AG_ObjectClass) },
  { "struct ag_object_class", sizeof (struct ag_object_class) },
  { "AG_ObjectDep", sizeof (AG_ObjectDep) },
  { "struct ag_object_dep", sizeof (struct ag_object_dep) },
  { "AG_Object", sizeof (AG_Object) },
  { "struct ag_object", sizeof (struct ag_object) },
  { "AG_PropClass", sizeof (AG_PropClass) },
  { "struct ag_prop_class", sizeof (struct ag_prop_class) },
  { "enum ag_prop_type", sizeof (enum ag_prop_type) },
  { "AG_Cond", sizeof (AG_Cond) },
  { "AG_ThreadKey", sizeof (AG_ThreadKey) },
  { "AG_MutexAttr", sizeof (AG_MutexAttr) },
  { "AG_Mutex", sizeof (AG_Mutex) },
  { "AG_Thread", sizeof (AG_Thread) },
  { "AG_Timeout", sizeof (AG_Timeout) },
  { "struct ag_timeout", sizeof (struct ag_timeout) },
  { "Sint16", sizeof (Sint16) },
  { "Sint32", sizeof (Sint32) },
  { "Sint64", sizeof (Sint64) },
  { "Sint8", sizeof (Sint8) },
  { "Uint16", sizeof (Uint16) },
  { "Uint32", sizeof (Uint32) },
  { "Uint64", sizeof (Uint64) },
  { "Uint8", sizeof (Uint8) },
  { "AG_Version", sizeof (AG_Version) },
  { "struct ag_version", sizeof (struct ag_version) },
  { "AG_AgarVersion", sizeof (AG_AgarVersion) },
  { "struct ag_agar_version", sizeof (struct ag_agar_version) },
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
