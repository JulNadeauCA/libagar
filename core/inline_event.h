/*	Public domain	*/

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushPointer(AG_Event *_Nonnull ev, const char *_Nullable name,
    void *_Nullable val)
#else
void
ag_event_push_pointer(AG_Event *ev, const char *name, void *val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_POINTER, name, p, val);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushString(AG_Event *_Nonnull ev, const char *_Nullable name,
    char *_Nonnull val)
#else
void
ag_event_push_string(AG_Event *ev, const char *name, char *val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_STRING, name, s, AG_Strdup(val));
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushInt(AG_Event *_Nonnull ev, const char *_Nullable name, int val)
#else
void
ag_event_push_int(AG_Event *ev, const char *name, int val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_INT, name, i, val);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushUint(AG_Event *_Nonnull ev, const char *_Nullable name, Uint val)
#else
void
ag_event_push_uint(AG_Event *ev, const char *name, Uint val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_UINT, name, i, val);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushLong(AG_Event *_Nonnull ev, const char *_Nullable name, long val)
#else
void
ag_event_push_long(AG_Event *_Nonnull ev, const char *_Nullable name, long val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_LONG, name, li, val);
}

#ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushUlong(AG_Event *_Nonnull ev, const char *_Nullable name, Ulong val)
#else
void
ag_event_push_ulong(AG_Event *ev, const char *name, Ulong val)
#endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_ULONG, name, uli, val)
}

#ifdef AG_HAVE_FLOAT
# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushFloat(AG_Event *_Nonnull ev, const char *_Nullable name, float val)
# else
void
ag_event_push_float(AG_Event *ev, const char *name, float val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_FLOAT, name, flt, val);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushDouble(AG_Event *_Nonnull ev, const char *_Nullable name, double val)
# else
void
ag_event_push_double(AG_Event *ev, const char *name, double val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_DOUBLE, name, dbl, val);
}

# ifdef AG_HAVE_LONG_DOUBLE
#  ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushLongDouble(AG_Event *_Nonnull ev, const char *_Nullable name,
    long double val)
#  else
void
ag_event_push_long_double(AG_Event *ev, const char *name, long double val)
#  endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_LONG_DOUBLE, name, ldbl, val);
}
# endif
#endif /* AG_HAVE_FLOAT */

#ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable
AG_EventPopPointer(AG_Event *_Nonnull ev)
#else
void *
ag_event_pop_pointer(AG_Event *ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_POINTER, p);
}

#ifdef AG_INLINE_HEADER
static __inline__ char *_Nonnull
AG_EventPopString(AG_Event *_Nonnull ev)
#else
char *
ag_event_pop_string(AG_Event *ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_STRING, s);
}

#ifdef AG_INLINE_HEADER
static __inline__ int
AG_EventPopInt(AG_Event *_Nonnull ev)
#else
int
ag_event_pop_int(AG_Event *ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_INT, i);
}

#ifdef AG_INLINE_HEADER
static __inline__ Uint
AG_EventPopUint(AG_Event *_Nonnull ev)
#else
Uint
ag_event_pop_uint(AG_Event *_Nonnull ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_UINT, u);
}

#ifdef AG_INLINE_HEADER
static __inline__ long
AG_EventPopLong(AG_Event *_Nonnull ev)
#else
long
ag_event_pop_long(AG_Event *_Nonnull ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_LONG, li);
}

#ifdef AG_INLINE_HEADER
static __inline__ Ulong
AG_EventPopUlong(AG_Event *_Nonnull ev)
#else
Ulong
ag_event_pop_ulong(AG_Event *_Nonnull ev)
#endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_ULONG, uli);
}

#ifdef AG_HAVE_FLOAT
# ifdef AG_INLINE_HEADER
static __inline__ float
AG_EventPopFloat(AG_Event *_Nonnull ev)
# else
float
ag_event_pop_float(AG_Event *_Nonnull ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_FLOAT, flt);
}

# ifdef AG_INLINE_HEADER
static __inline__ double
AG_EventPopDouble(AG_Event *_Nonnull ev)
# else
double
ag_event_pop_double(AG_Event *_Nonnull ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_DOUBLE, dbl);
}

# ifdef AG_HAVE_LONG_DOUBLE
#  ifdef AG_INLINE_HEADER
static __inline__ long double
AG_EventPopLongDouble(AG_Event *_Nonnull ev)
#  else
long double
ag_event_pop_long_double(AG_Event *ev)
#  endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_LONG_DOUBLE, ldbl);
}
# endif
#endif

#undef AG_EVENT_PUSH_FN
#undef AG_EVENT_POP_FN

/*
 * Extract Event argument by name (case-insensitive).
 */
#ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nonnull _Pure_Attribute
AG_GetNamedEventArg(AG_Event *_Nonnull ev, const char *_Nonnull name)
#else
AG_Variable *
ag_get_named_event_arg(AG_Event *ev, const char *name)
#endif
{
	int i;

	for (i = 0; i < ev->argc; i++) {
		if (AG_Strcasecmp(ev->argv[i].name, name) == 0)
			return (&ev->argv[i]);
	}
	AG_SetError("Illegal AG_*_NAMED() access: No \"%s\"", name);
	AG_FatalError(NULL);
	return (&ev->argv[0]);
}

#ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable _Pure_Attribute
AG_GetNamedPtr(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
void *
ag_get_named_ptr(AG_Event *event, const char *name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_POINTER)
		AG_FatalError("Illegal AG_PTR_NAMED() access");
#endif
	return (V->data.p);
}

#ifdef AG_INLINE_HEADER
static __inline__ char *_Nonnull _Pure_Attribute
AG_GetNamedString(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
char *
ag_get_named_string(AG_Event *event, const char *name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_STRING)
		AG_FatalError("Illegal AG_STRING_NAMED() access");
#endif
	return (V->data.s);
}

#ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_GetNamedInt(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
int
ag_get_named_int(AG_Event *_Nonnull event, const char *_Nonnull name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_INT) { AG_FatalError("Illegal AG_INT_NAMED() access"); }
#endif
	return (V->data.i);
}

#ifdef AG_INLINE_HEADER
static __inline__ Uint _Pure_Attribute
AG_GetNamedUint(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
Uint
ag_get_named_uint(AG_Event *event, const char *name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_UINT) { AG_FatalError("Illegal AG_UINT_NAMED() access"); }
#endif
	return (V->data.u);
}

#ifdef AG_INLINE_HEADER
static __inline__ long _Pure_Attribute
AG_GetNamedLong(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
long
ag_get_named_long(AG_Event *event, const char *name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_LONG) { AG_FatalError("Illegal AG_LONG_NAMED() access"); }
#endif
	return (V->data.li);
}

#ifdef AG_INLINE_HEADER
static __inline__ Ulong _Pure_Attribute
AG_GetNamedUlong(AG_Event *_Nonnull event, const char *_Nonnull name)
#else
Ulong
ag_get_named_ulong(AG_Event *event, const char *name)
#endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_ULONG) { AG_FatalError("Illegal AG_ULONG_NAMED() access"); }
#endif
	return (V->data.uli);
}

#ifdef AG_HAVE_FLOAT

# ifdef AG_INLINE_HEADER
static __inline__ float _Pure_Attribute
AG_GetNamedFlt(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
float
ag_get_named_flt(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_FLOAT) { AG_FatalError("Illegal AG_FLOAT_NAMED() access"); }
# endif
	return (V->data.flt);
}

# ifdef AG_INLINE_HEADER
static __inline__ double _Pure_Attribute
AG_GetNamedDbl(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
double
ag_get_named_dbl(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_DOUBLE) { AG_FatalError("Illegal AG_DOUBLE_NAMED() access"); }
# endif
	return (V->data.dbl);
}

# ifdef AG_HAVE_LONG_DOUBLE
#  ifdef AG_INLINE_HEADER
static __inline__ long double _Pure_Attribute
AG_GetNamedLongDbl(AG_Event *_Nonnull event, const char *_Nonnull name)
#  else
long double
ag_get_named_long_dbl(AG_Event *event, const char *name)
#  endif
{
	AG_Variable *V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_LONG_DOUBLE) { AG_FatalError("Illegal AG_LONG_DOUBLE_NAMED() access"); }
# endif
	return (V->data.ldbl);
}
# endif /* AG_HAVE_LONG_DOUBLE */

#endif /* AG_HAVE_FLOAT */
