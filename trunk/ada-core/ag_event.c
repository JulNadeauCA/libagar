#include <agar/core.h>

void
agar_event_push_pointer (AG_Event *ev, char *key, void *val)
{
  AG_EventPushPointer (ev, key, val);
}

void
agar_event_push_string (AG_Event *ev, char *key, char *val)
{
  AG_EventPushString (ev, key, val);
}

void
agar_event_push_char (AG_Event *ev, char *key, char val)
{
  AG_EventPushChar (ev, key, val);
}

void
agar_event_push_unsigned_char (AG_Event *ev, char *key, unsigned char val)
{
  AG_EventPushUChar (ev, key, val);
}

void
agar_event_push_int (AG_Event *ev, char *key, int val)
{
  AG_EventPushInt (ev, key, val);
}

void
agar_event_push_unsigned_int (AG_Event *ev, char *key, unsigned int val)
{
  AG_EventPushUint (ev, key, val);
}

void
agar_event_push_long (AG_Event *ev, char *key, long val)
{
  AG_EventPushLong (ev, key, val);
}

void
agar_event_push_unsigned_long (AG_Event *ev, char *key, unsigned long val)
{
  AG_EventPushULong (ev, key, val);
}

void
agar_event_push_float (AG_Event *ev, char *key, float val)
{
  AG_EventPushFloat (ev, key, val);
}

void
agar_event_push_double (AG_Event *ev, char *key, double val)
{
  AG_EventPushDouble (ev, key, val);
}

