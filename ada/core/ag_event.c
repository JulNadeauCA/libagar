#define AG_TYPE_SAFETY
#include <agar/core.h>

void       *ag_event_get_ptr(AG_Event *event, unsigned i)       { return AG_PTR(i); }
const void *ag_event_get_const_ptr(AG_Event *event, unsigned i) { return AG_CONST_PTR(i); }
char       *ag_event_get_string(AG_Event *event, unsigned i)    { return AG_STRING(i); }
int         ag_event_get_int(AG_Event *event, unsigned i)       { return AG_INT(i); }
unsigned    ag_event_get_uint(AG_Event *event, unsigned i)      { return AG_UINT(i); }
long        ag_event_get_long(AG_Event *event, unsigned i)      { return AG_LONG(i); }
float       ag_event_get_float(AG_Event *event, unsigned i)     { return AG_FLOAT(i); }
double      ag_event_get_double(AG_Event *event, unsigned i)    { return AG_DOUBLE(i); }

void       *ag_event_get_ptr_named(AG_Event *event, const char *s)       { return AG_PTR_NAMED(s); }
const void *ag_event_get_const_ptr_named(AG_Event *event, const char *s) { return AG_PTR_NAMED(s); }
char       *ag_event_get_string_named(AG_Event *event, const char *s)    { return AG_STRING_NAMED(s); }
int         ag_event_get_int_named(AG_Event *event, const char *s)       { return AG_INT_NAMED(s); }
unsigned    ag_event_get_uint_named(AG_Event *event, const char *s)      { return AG_UINT_NAMED(s); }
long        ag_event_get_long_named(AG_Event *event, const char *s)      { return AG_LONG_NAMED(s); }
float       ag_event_get_float_named(AG_Event *event, const char *s)     { return AG_FLOAT_NAMED(s); }
double      ag_event_get_double_named(AG_Event *event, const char *s)    { return AG_DOUBLE_NAMED(s); }

