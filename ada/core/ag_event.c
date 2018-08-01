#define AG_TYPE_SAFETY
#include <agar/core.h>

void ag_event_push_ptr(AG_Event *e, char *k, void *v)     { AG_EventPushPointer(e,k,v); }
void ag_event_push_string(AG_Event *e, char *k, char *v)  { AG_EventPushString(e,k,v); }
void ag_event_push_int(AG_Event *e, char *k, int v)       { AG_EventPushInt(e,k,v); }
void ag_event_push_uint(AG_Event *e, char *k, unsigned v) { AG_EventPushUint(e,k,v); }
void ag_event_push_long(AG_Event *e, char *k, long v)     { AG_EventPushLong(e,k,v); }
void ag_event_push_float(AG_Event *e, char *k, float v)   { AG_EventPushFloat(e,k,v); }
void ag_event_push_double(AG_Event *e, char *k, double v) { AG_EventPushDouble(e,k,v); }
void ag_event_push_long_double(AG_Event *e, char *k, double v) { AG_EventPushLongDouble(e,k,v); }

void       *ag_event_pop_ptr(AG_Event *e)         { return AG_EventPopPointer(e); }
char       *ag_event_pop_string(AG_Event *e)      { return AG_EventPopString(e); }
int         ag_event_pop_int(AG_Event *e)         { return AG_EventPopInt(e); }
unsigned    ag_event_pop_uint(AG_Event *e)        { return AG_EventPopUint(e); }
long        ag_event_pop_long(AG_Event *e)        { return AG_EventPopLong(e); }
float       ag_event_pop_float(AG_Event *e)       { return AG_EventPopFloat(e); }
double      ag_event_pop_double(AG_Event *e)      { return AG_EventPopDouble(e); }
long double ag_event_pop_long_double(AG_Event *e) { return AG_EventPopLongDouble(e); }

void       *ag_event_get_ptr(AG_Event *event, unsigned i)    { return AG_PTR(i); }
char       *ag_event_get_string(AG_Event *event, unsigned i) { return AG_STRING(i); }
int         ag_event_get_int(AG_Event *event, unsigned i)    { return AG_INT(i); }
unsigned    ag_event_get_uint(AG_Event *event, unsigned i)   { return AG_UINT(i); }
long        ag_event_get_long(AG_Event *event, unsigned i)   { return AG_LONG(i); }
float       ag_event_get_float(AG_Event *event, unsigned i)  { return AG_FLOAT(i); }
double      ag_event_get_double(AG_Event *event, unsigned i) { return AG_DOUBLE(i); }
long double ag_event_get_long_double(AG_Event *event, unsigned i) { return AG_LONG_DOUBLE(i); }

void       *ag_event_get_ptr_named(AG_Event *event, const char *s)    { return AG_PTR_NAMED(s); }
char       *ag_event_get_string_named(AG_Event *event, const char *s) { return AG_STRING_NAMED(s); }
int         ag_event_get_int_named(AG_Event *event, const char *s)    { return AG_INT_NAMED(s); }
unsigned    ag_event_get_uint_named(AG_Event *event, const char *s)   { return AG_UINT_NAMED(s); }
long        ag_event_get_long_named(AG_Event *event, const char *s)   { return AG_LONG_NAMED(s); }
float       ag_event_get_float_named(AG_Event *event, const char *s)  { return AG_FLOAT_NAMED(s); }
double      ag_event_get_double_named(AG_Event *event, const char *s) { return AG_DOUBLE_NAMED(s); }
long double ag_event_get_long_double_named(AG_Event *event, const char *s) { return AG_LONG_DOUBLE_NAMED(s); }

