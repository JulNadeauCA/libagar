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

/* prop */

int
agar_object_prop_get_boolean (AG_Object *obj, const char *key)
{
  return AG_GetBool (obj, key);
}

float
agar_object_prop_get_float (AG_Object *obj, const char *key)
{
  return AG_GetFloat (obj, key);
}

int
agar_object_prop_get_int (AG_Object *obj, const char *key)
{
  return AG_GetInt (obj, key);
}

Sint16
agar_object_prop_get_int16 (AG_Object *obj, const char *key)
{
  return AG_GetSint16 (obj, key);
}

Sint32
agar_object_prop_get_int32 (AG_Object *obj, const char *key)
{
  return AG_GetSint32 (obj, key);
}

Sint64
agar_object_prop_get_int64 (AG_Object *obj, const char *key)
{
  return AG_GetSint64 (obj, key);
}

Sint8
agar_object_prop_get_int8 (AG_Object *obj, const char *key)
{
  return AG_GetSint8 (obj, key);
}

double
agar_object_prop_get_long_float (AG_Object *obj, const char *key)
{
  return AG_GetDouble (obj, key);
}

long double
agar_object_prop_get_long_long_float (AG_Object *obj, const char *key)
{
  return AG_GetLongDouble (obj, key);
}

void *
agar_object_prop_get_pointer (AG_Object *obj, const char *key)
{
  return AG_GetPointer (obj, key);
}

Uint16
agar_object_prop_get_uint16 (AG_Object *obj, const char *key)
{
  return AG_GetUint16 (obj, key);
}

Uint32
agar_object_prop_get_uint32 (AG_Object *obj, const char *key)
{
  return AG_GetUint32 (obj, key);
}

Uint64
agar_object_prop_get_uint64 (AG_Object *obj, const char *key)
{
  return AG_GetUint64 (obj, key);
}

Uint8
agar_object_prop_get_uint8 (AG_Object *obj, const char *key)
{
  return AG_GetUint8 (obj, key);
}

unsigned int
agar_object_prop_get_unsigned_int (AG_Object *obj, const char *key)
{
  return AG_GetUint (obj, key);
}

/* set */


AG_Prop *
agar_object_prop_set_boolean (AG_Object *obj, const char *key,
  int value)
{
  return AG_SetBool (obj, key, value);
}

AG_Prop *
agar_object_prop_set_float (AG_Object *obj, const char *key,
  float value)
{
  return AG_SetFloat (obj, key, value);
}

AG_Prop *
agar_object_prop_set_int (AG_Object *obj, const char *key,
  int value)
{
  return AG_SetInt (obj, key, value);
}

AG_Prop *
agar_object_prop_set_int16 (AG_Object *obj, const char *key,
  Sint16 value)
{
  return AG_SetSint16 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_int32 (AG_Object *obj, const char *key,
  Sint32 value)
{
  return AG_SetSint32 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_int64 (AG_Object *obj, const char *key,
  Sint64 value)
{
  return AG_SetSint64 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_int8 (AG_Object *obj, const char *key,
  Sint8 value)
{
  return AG_SetSint8 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_long_float (AG_Object *obj, const char *key,
  double value)
{
  return AG_SetDouble (obj, key, value);
}

AG_Prop *
agar_object_prop_set_long_long_float (AG_Object *obj, const char *key,
  long double value)
{
  return AG_SetLongDouble (obj, key, value);
}

AG_Prop *
agar_object_prop_set_pointer (AG_Object *obj, const char *key,
  void *value)
{
  return AG_SetPointer (obj, key, value);
}

AG_Prop *
agar_object_prop_set_uint16 (AG_Object *obj, const char *key,
  Uint16 value)
{
  return AG_SetUint16 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_uint32 (AG_Object *obj, const char *key,
  Uint32 value)
{
  return AG_SetUint32 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_uint64 (AG_Object *obj, const char *key,
  Uint64 value)
{
  return AG_SetUint64 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_uint8 (AG_Object *obj, const char *key,
  Uint8 value)
{
  return AG_SetUint8 (obj, key, value);
}

AG_Prop *
agar_object_prop_set_unsigned_int (AG_Object *obj, const char *key,
  unsigned int value)
{
  return AG_SetUint (obj, key, value);
}
