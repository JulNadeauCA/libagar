#include <agar/core.h>

int
agar_object_of_class (void *obj, const char *spec)
{
  return AG_OfClass (obj, spec);
}

AG_ObjectClass *
agar_object_superclass (const void *p)
{
  return AG_ObjectSuperclass (p);
}
