/*	Public domain	*/

#if AG_MODEL != AG_SMALL

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushPointer(AG_Event *_Nonnull ev, const char *_Nullable name,
    void *_Nullable val)
# else
void
ag_event_push_pointer(AG_Event *ev, const char *name, void *val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_POINTER, name, p, val);
	ev->argv[ev->argc - 1].info.pFlags = 0;
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushConstPointer(AG_Event *_Nonnull ev, const char *_Nullable name,
    const void *_Nullable val)
# else
void
ag_event_push_const_pointer(AG_Event *ev, const char *name, const void *val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_POINTER, name, p, (void *)val);
	ev->argv[ev->argc - 1].info.pFlags = AG_VARIABLE_P_READONLY;
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushString(AG_Event *_Nonnull ev, const char *_Nullable name,
    char *_Nonnull val)
# else
void
ag_event_push_string(AG_Event *ev, const char *name, char *val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_STRING, name, s, AG_Strdup(val));
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushInt(AG_Event *_Nonnull ev, const char *_Nullable name, int val)
# else
void
ag_event_push_int(AG_Event *ev, const char *name, int val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_INT, name, i, val);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushUint(AG_Event *_Nonnull ev, const char *_Nullable name, Uint val)
# else
void
ag_event_push_uint(AG_Event *ev, const char *name, Uint val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_UINT, name, i, val);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushLong(AG_Event *_Nonnull ev, const char *_Nullable name, long val)
# else
void
ag_event_push_long(AG_Event *_Nonnull ev, const char *_Nullable name, long val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_LONG, name, li, val);
}

# ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushUlong(AG_Event *_Nonnull ev, const char *_Nullable name, Ulong val)
# else
void
ag_event_push_ulong(AG_Event *ev, const char *name, Ulong val)
# endif
{
	AG_EVENT_PUSH_FN(ev, AG_VARIABLE_ULONG, name, uli, val)
}

# ifdef AG_HAVE_FLOAT
#  ifdef AG_INLINE_HEADER
static __inline__ void
AG_EventPushFloat(AG_Event *_Nonnull ev, const char *_Nullable name, float val)
#  else
void
ag_event_push_float(AG_Event *ev, const char *name, float val)
#  endif
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
# endif /* AG_HAVE_FLOAT */

# ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable
AG_EventPopPointer(AG_Event *_Nonnull ev)
# else
void *
ag_event_pop_pointer(AG_Event *ev)
# endif
{
	AG_Variable *V;

	AG_EVENT_POP_ARG_PRECOND(ev)
	V = &ev->argv[ev->argc--];
	AG_EVENT_POP_ARG_POSTCOND(V, AG_VARIABLE_POINTER)
# ifdef AG_DEBUG
	if (V->info.pFlags & AG_VARIABLE_P_READONLY) {
		AG_FatalErrorV("E30", "Pointer is const. "
		                      "Did you mean AG_EventPopConstPointer()?");
	}
# endif
	return (V->data.p);
}

# ifdef AG_INLINE_HEADER
static __inline__ const void *_Nullable
AG_EventPopConstPointer(AG_Event *_Nonnull ev)
# else
const void *
ag_event_pop_const_pointer(AG_Event *ev)
# endif
{
	AG_Variable *V;

	AG_EVENT_POP_ARG_PRECOND(ev)
	V = &ev->argv[ev->argc--];
	AG_EVENT_POP_ARG_POSTCOND(V, AG_VARIABLE_POINTER)
# ifdef AG_DEBUG
	if (!(V->info.pFlags & AG_VARIABLE_P_READONLY)) {
		AG_FatalErrorV("E35", "Pointer is not const. "
		                      "Did you mean AG_EventPopPointer()?");
	}
# endif
	return (const void *)V->data.p;
}

# ifdef AG_INLINE_HEADER
static __inline__ char *_Nonnull
AG_EventPopString(AG_Event *_Nonnull ev)
# else
char *
ag_event_pop_string(AG_Event *ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_STRING, s);
}

# ifdef AG_INLINE_HEADER
static __inline__ int
AG_EventPopInt(AG_Event *_Nonnull ev)
# else
int
ag_event_pop_int(AG_Event *ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_INT, i);
}

# ifdef AG_INLINE_HEADER
static __inline__ Uint
AG_EventPopUint(AG_Event *_Nonnull ev)
# else
Uint
ag_event_pop_uint(AG_Event *_Nonnull ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_UINT, u);
}

# ifdef AG_INLINE_HEADER
static __inline__ long
AG_EventPopLong(AG_Event *_Nonnull ev)
# else
long
ag_event_pop_long(AG_Event *_Nonnull ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_LONG, li);
}

# ifdef AG_INLINE_HEADER
static __inline__ Ulong
AG_EventPopUlong(AG_Event *_Nonnull ev)
# else
Ulong
ag_event_pop_ulong(AG_Event *_Nonnull ev)
# endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_ULONG, uli);
}

# ifdef AG_HAVE_FLOAT
#  ifdef AG_INLINE_HEADER
static __inline__ float
AG_EventPopFloat(AG_Event *_Nonnull ev)
#  else
float
ag_event_pop_float(AG_Event *_Nonnull ev)
#  endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_FLOAT, flt);
}

#  ifdef AG_INLINE_HEADER
static __inline__ double
AG_EventPopDouble(AG_Event *_Nonnull ev)
#  else
double
ag_event_pop_double(AG_Event *_Nonnull ev)
#  endif
{
	AG_EVENT_POP_FN(AG_VARIABLE_DOUBLE, dbl);
}
# endif  /* AG_HAVE_FLOAT */

# undef AG_EVENT_PUSH_FN
# undef AG_EVENT_POP_FN

#endif /* !AG_SMALL */

#ifdef AG_NAMED_ARGS
/*
 * Extract Event argument by name (case-insensitive).
 */
# ifdef AG_INLINE_HEADER
static __inline__ AG_Variable *_Nonnull _Pure_Attribute
AG_GetNamedEventArg(AG_Event *_Nonnull ev, const char *_Nonnull name)
# else
AG_Variable *
ag_get_named_event_arg(AG_Event *ev, const char *name)
# endif
{
	int i;

	for (i = 0; i < ev->argc; i++) {
		if (strcmp(ev->argv[i].name, name) == 0)
			return (&ev->argv[i]);
	}
# ifdef AG_VERBOSITY
	AG_FatalErrorF("No such argument: \"%s\"", name);
# else
	AG_FatalError("E26");
# endif
	return (&ev->argv[0]);
}

# ifdef AG_INLINE_HEADER
static __inline__ void *_Nullable _Pure_Attribute
AG_GetNamedPtr(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
void *
ag_get_named_ptr(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_POINTER) {
		AG_FatalErrorF("Argument %s is a %s (not a Pointer)",
		    name, agVariableTypes[V->type].name);
	}
	if ((V->info.pFlags & AG_VARIABLE_P_READONLY)) {
		AG_FatalErrorV("E35", "Pointer is const. "
		                      "Did you mean AG_CONST_PTR_NAMED()?");
	}
# endif
	return (V->data.p);
}

# ifdef AG_INLINE_HEADER
static __inline__ const void *_Nullable _Pure_Attribute
AG_GetNamedConstPtr(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
const void *
ag_get_named_const_ptr(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_POINTER) {
		AG_FatalErrorF("Argument %s is a %s (not a Pointer)",
		    name, agVariableTypes[V->type].name);
	}
	if (!(V->info.pFlags & AG_VARIABLE_P_READONLY)) {
		AG_FatalErrorV("E35", "Pointer is not const. "
		                      "Did you mean AG_PTR_NAMED()?");
	}
# endif
	return (const void *)V->data.p;
}

# ifdef AG_INLINE_HEADER
static __inline__ char *_Nonnull _Pure_Attribute
AG_GetNamedString(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
char *
ag_get_named_string(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_STRING)
		AG_FatalErrorF("Argument %s is a %s (not a String)",
		    name, agVariableTypes[V->type].name);
#endif
	return (V->data.s);
}

# ifdef AG_INLINE_HEADER
static __inline__ int _Pure_Attribute
AG_GetNamedInt(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
int
ag_get_named_int(AG_Event *_Nonnull event, const char *_Nonnull name)
# endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_INT)
		AG_FatalErrorF("Argument %s is a %s (not an Int)",
		    name, agVariableTypes[V->type].name);
#endif
	return (V->data.i);
}

# ifdef AG_INLINE_HEADER
static __inline__ Uint _Pure_Attribute
AG_GetNamedUint(AG_Event *_Nonnull event, const char *_Nonnull name)
# else
Uint
ag_get_named_uint(AG_Event *event, const char *name)
# endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
# ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_UINT)
		AG_FatalErrorF("Argument %s is a %s (not an Uint)",
		    name, agVariableTypes[V->type].name);
# endif
	return (V->data.u);
}

# if AG_MODEL != AG_SMALL

#  ifdef AG_INLINE_HEADER
static __inline__ long _Pure_Attribute
AG_GetNamedLong(AG_Event *_Nonnull event, const char *_Nonnull name)
#  else
long
ag_get_named_long(AG_Event *event, const char *name)
#  endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#  ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_LONG)
		AG_FatalErrorF("Argument %s is a %s (not a Long)",
		    name, agVariableTypes[V->type].name);
#  endif
	return (V->data.li);
}

#  ifdef AG_INLINE_HEADER
static __inline__ Ulong _Pure_Attribute
AG_GetNamedUlong(AG_Event *_Nonnull event, const char *_Nonnull name)
#  else
Ulong
ag_get_named_ulong(AG_Event *event, const char *name)
#  endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#  ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_ULONG)
		AG_FatalErrorF("Argument %s is a %s (not an Ulong)",
		    name, agVariableTypes[V->type].name);
#  endif
	return (V->data.uli);
}
#  endif /* !AG_SMALL */

# ifdef AG_HAVE_FLOAT
#  ifdef AG_INLINE_HEADER
static __inline__ float _Pure_Attribute
AG_GetNamedFlt(AG_Event *_Nonnull event, const char *_Nonnull name)
#  else
float
ag_get_named_flt(AG_Event *event, const char *name)
#  endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#  ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_FLOAT)
		AG_FatalErrorF("Argument %s is a %s (not a Float)",
		    name, agVariableTypes[V->type].name);
#  endif
	return (V->data.flt);
}

#  ifdef AG_INLINE_HEADER
static __inline__ double _Pure_Attribute
AG_GetNamedDbl(AG_Event *_Nonnull event, const char *_Nonnull name)
#  else
double
ag_get_named_dbl(AG_Event *event, const char *name)
#  endif
{
	AG_Variable *V;

	V = AG_GetNamedEventArg(event, name);
#  ifdef AG_TYPE_SAFETY
	if (V->type != AG_VARIABLE_DOUBLE)
		AG_FatalErrorF("Argument %s is a %s (not a Double)",
		    name, agVariableTypes[V->type].name);
#  endif
	return (V->data.dbl);
}
# endif /* AG_HAVE_FLOAT */

#endif /* AG_NAMED_ARGS */
