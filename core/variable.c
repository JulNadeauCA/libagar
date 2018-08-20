/*
 * Copyright (c) 2008-2018 Julien Nadeau Carriere <vedge@hypertriton.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <agar/core/core.h>

#include <string.h>

const AG_VariableTypeInfo agVariableTypes[] = {
	/* Type                  indirLvl,      name,           typeTgt                       code,  size */
	{ AG_VARIABLE_NULL,		0,	"NULL",		AG_VARIABLE_NULL,		-1,	0 },
	/*
	 * Integers and floating-point numbers.
	 */
	{ AG_VARIABLE_UINT,		0,	"Uint",		AG_VARIABLE_UINT,		0,	sizeof(Uint) },
	{ AG_VARIABLE_P_UINT,		1,	"Uint *",	AG_VARIABLE_UINT,		0,	sizeof(Uint) },
	{ AG_VARIABLE_INT,		0,	"int",		AG_VARIABLE_INT,		1,	sizeof(int) },
	{ AG_VARIABLE_P_INT,		1,	"int *",	AG_VARIABLE_INT,		1,	sizeof(int) },
	{ AG_VARIABLE_ULONG,		0,	"Ulong",	AG_VARIABLE_ULONG,		14,	sizeof(Ulong) },
	{ AG_VARIABLE_P_ULONG,		1,	"Ulong *",	AG_VARIABLE_ULONG,		14,	sizeof(Ulong) },
	{ AG_VARIABLE_LONG,		0,	"long",		AG_VARIABLE_LONG,		15,	sizeof(long) },
	{ AG_VARIABLE_P_LONG,		1,	"long *",	AG_VARIABLE_LONG,		15,	sizeof(long) },

	{ AG_VARIABLE_UINT8,		0,	"Uint8",	AG_VARIABLE_UINT8,		2,	1 },
	{ AG_VARIABLE_P_UINT8,		1,	"Uint8 *",	AG_VARIABLE_UINT8,		2,	1 },
	{ AG_VARIABLE_SINT8,		0,	"Sint8",	AG_VARIABLE_SINT8,		3,	1 },
	{ AG_VARIABLE_P_SINT8,		1,	"Sint8 *",	AG_VARIABLE_SINT8,		3,	1 },
	{ AG_VARIABLE_UINT16,		0,	"Uint16",	AG_VARIABLE_UINT16,		4,	2 },
	{ AG_VARIABLE_P_UINT16,		1,	"Uint16 *",	AG_VARIABLE_UINT16,		4,	2 },
	{ AG_VARIABLE_SINT16,		0,	"Sint16",	AG_VARIABLE_SINT16,		5,	2 },
	{ AG_VARIABLE_P_SINT16,		1,	"Sint16 *",	AG_VARIABLE_SINT16,		5,	2 },
	{ AG_VARIABLE_UINT32,		0,	"Uint32",	AG_VARIABLE_UINT32,		6,	4 },
	{ AG_VARIABLE_P_UINT32,		1,	"Uint32 *" ,	AG_VARIABLE_UINT32,		6,	4 },
	{ AG_VARIABLE_SINT32,		0,	"Sint32",	AG_VARIABLE_SINT32,		7,	4 },
	{ AG_VARIABLE_P_SINT32,		1,	"Sint32 *",	AG_VARIABLE_SINT32,		7,	4 },
	{ AG_VARIABLE_UINT64,		0,	"Uint64",	AG_VARIABLE_UINT64,		8,	8 },
	{ AG_VARIABLE_P_UINT64,		1,	"Uint64 *",	AG_VARIABLE_UINT64,		8,	8 },
	{ AG_VARIABLE_SINT64,		0,	"Sint64",	AG_VARIABLE_SINT64,		9,	8 },
	{ AG_VARIABLE_P_SINT64,		1,	"Sint64 *",	AG_VARIABLE_SINT64,		9,	8 },

	{ AG_VARIABLE_FLOAT,		0,	"float",	AG_VARIABLE_FLOAT,		10,	sizeof(float) },
	{ AG_VARIABLE_P_FLOAT,		1,	"float *",	AG_VARIABLE_FLOAT,		10,	sizeof(float) },
	{ AG_VARIABLE_DOUBLE,		0,	"double",	AG_VARIABLE_DOUBLE,		11,	sizeof(double) },
	{ AG_VARIABLE_P_DOUBLE,		1,	"double *",	AG_VARIABLE_DOUBLE,		11,	sizeof(double) },
	{ AG_VARIABLE_LONG_DOUBLE,	0,	"long double",	AG_VARIABLE_LONG_DOUBLE,	12,	sizeof(long double) },
	{ AG_VARIABLE_P_LONG_DOUBLE,	1,	"long double *",AG_VARIABLE_LONG_DOUBLE,	12,	sizeof(long double) },
	/*
	 * Unbounded C strings.
	 */
	{ AG_VARIABLE_STRING,		0,	"Str",		AG_VARIABLE_STRING,		13,	sizeof(char *) },
	{ AG_VARIABLE_P_STRING,		1,	"Str *",	AG_VARIABLE_STRING,		13,	sizeof(char *) },
	/*
	 * Generic pointers.
	 */
	{ AG_VARIABLE_POINTER,		0,	"Ptr",		AG_VARIABLE_POINTER,		-1,	sizeof(void *) },
	{ AG_VARIABLE_P_POINTER,	1,	"Ptr *",	AG_VARIABLE_POINTER,		-1,	sizeof(void *) },
	/*
	 * Reference to one or more bits in a word.
	 */
	{ AG_VARIABLE_P_FLAG,		1,	"Flag *",	AG_VARIABLE_INT,		-1,	sizeof(int) },
	{ AG_VARIABLE_P_FLAG8,		1,	"Flag8 *",	AG_VARIABLE_UINT8,		-1,	1 },
	{ AG_VARIABLE_P_FLAG16,		1,	"Flag16 *",	AG_VARIABLE_UINT16,		-1,	2 },
	{ AG_VARIABLE_P_FLAG32,		1,	"Flag32 *",	AG_VARIABLE_UINT32,		-1,	4 },
	/*
	 * Serializable Object or Object:Variable reference.
	 */
	{ AG_VARIABLE_P_OBJECT,		1,	"Object *",	AG_VARIABLE_P_OBJECT,		16,	sizeof(void *) },
	{ AG_VARIABLE_P_VARIABLE,	1,	"Variable *",	AG_VARIABLE_P_VARIABLE,		17,	sizeof(void *) },
};

/*
 * Duplicate a Variable. Preserve pointers, but duplicate allocated strings
 * and P_VARIABLE references.
 */
int
AG_CopyVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	if (Vsrc->type == AG_VARIABLE_STRING &&
	    Vsrc->info.size == 0) {
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL)
			return (-1);
	} else if (Vsrc->type == AG_VARIABLE_P_VARIABLE) {
		if ((Vdst->info.varName = TryStrdup(Vsrc->info.varName)) == NULL)
			return (-1);
	}
	return (0);
}

/*
 * Duplicate a Variable with dereference. Mutate reference types into discrete
 * types (e.g., P_INT=>INT, P_STRING=>STRING, P_FLAG8=>UINT8).
 *
 * Note: P_OBJECT and P_VARIABLE types are never dereferenced.
 */
int
AG_DerefVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	Vdst->type = agVariableTypes[Vsrc->type].typeTgt;

	switch (Vsrc->type) {
	case AG_VARIABLE_P_UINT:   case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_ULONG:  case AG_VARIABLE_P_LONG:
	case AG_VARIABLE_P_UINT8:  case AG_VARIABLE_P_SINT8:
	case AG_VARIABLE_P_UINT16: case AG_VARIABLE_P_SINT16:
	case AG_VARIABLE_P_UINT32: case AG_VARIABLE_P_SINT32:
	case AG_VARIABLE_P_UINT64: case AG_VARIABLE_P_SINT64:
	case AG_VARIABLE_P_FLOAT:  case AG_VARIABLE_P_DOUBLE:
	case AG_VARIABLE_P_LONG_DOUBLE:
	case AG_VARIABLE_P_FLAG:		/* to INT */
	case AG_VARIABLE_P_FLAG8:		/* to UINT8 */
	case AG_VARIABLE_P_FLAG16:		/* to UINT16 */
	case AG_VARIABLE_P_FLAG32:		/* to UINT32 */
	case AG_VARIABLE_P_POINTER:
		memcpy(&Vdst->data, Vsrc->data.p, agVariableTypes[Vsrc->type].size);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL) {
			return (-1);
		}
		Vdst->info.size = 0;
		break;
	case AG_VARIABLE_P_VARIABLE:
		if ((Vdst->info.varName = TryStrdup(Vsrc->info.varName)) == NULL) {
			return (-1);
		}
		break;
	default:
		break;
	}
	return (0);
}

/*
 * Compare two variables without dereference. Compare discrete types
 * (including STRING) by value, and compare pointers as pointers.
 */
int
AG_CompareVariables(const AG_Variable *a, const AG_Variable *b)
{
	if (a->type != b->type) {
		return (1);
	}
	switch (a->type) {
	case AG_VARIABLE_STRING:
		return strcmp(a->data.s, b->data.s);
	default:
		return memcmp(
		    (const void *)&a->data,
		    (const void *)&b->data,
		    agVariableTypes[a->type].size);
	}
}

/*
 * If V is a virtual function, invoke it and update data based on the result.
 * Return 0 on success and -1 if the routine is unavailable. If V is not a
 * virtual function, do nothing and return 0.
 */
int
AG_EvalVariable(void *pObj, AG_Variable *V)
{
	AG_Object *obj = pObj;
	char evName[AG_EVENT_NAME_MAX];
	AG_Event *ev;

	if (V->fn.fnVoid == NULL)
		return (0);

	AG_Strlcpy(evName, "get-", sizeof(evName));
	AG_Strlcat(evName, V->name, sizeof(evName));
	TAILQ_FOREACH(ev, &obj->events, events) {
		if (strcmp(evName, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		AG_SetError("get-%s is unavailable", V->name);
		return (-1);
	}
	switch (V->type) {
	case AG_VARIABLE_UINT:		V->data.u = V->fn.fnUint(ev);		break;
	case AG_VARIABLE_INT:		V->data.i = V->fn.fnInt(ev);		break;
	case AG_VARIABLE_ULONG:		V->data.uli = V->fn.fnUlong(ev);	break;
	case AG_VARIABLE_LONG:		V->data.li = V->fn.fnLong(ev);		break;
	case AG_VARIABLE_UINT8:		V->data.u8 = V->fn.fnUint8(ev);		break;
	case AG_VARIABLE_SINT8:		V->data.s8 = V->fn.fnSint8(ev);		break;
	case AG_VARIABLE_UINT16:	V->data.u16 = V->fn.fnUint16(ev);	break;
	case AG_VARIABLE_SINT16:	V->data.s16 = V->fn.fnSint16(ev);	break;
	case AG_VARIABLE_UINT32:	V->data.u32 = V->fn.fnUint32(ev);	break;
	case AG_VARIABLE_SINT32:	V->data.s32 = V->fn.fnSint32(ev);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	V->data.u64 = V->fn.fnUint64(ev);	break;
	case AG_VARIABLE_SINT64:	V->data.s64 = V->fn.fnSint64(ev);	break;
#endif
	case AG_VARIABLE_FLOAT:		V->data.flt = V->fn.fnFloat(ev);	break;
	case AG_VARIABLE_DOUBLE:	V->data.dbl = V->fn.fnDouble(ev);	break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	V->data.ldbl = V->fn.fnLongDouble(ev);	break;
#endif
	case AG_VARIABLE_POINTER:	V->data.p = V->fn.fnPointer(ev);	break;
	case AG_VARIABLE_STRING:
		V->fn.fnString(ev, V->data.s, V->info.size);
		break;
	default:
		break;
	}
	return (0);
}

/*
 * Print the specified variable to fixed-size buffer. The Variable must be
 * locked, and must have been previously evaluated if associated with a
 * function.
 */
static void
AG_PrintFlagsVariable(char *s, size_t len, Uint32 val, AG_Variable *V)
{
	Snprintf(s, len, "%s", (val & V->info.bitmask)?"Set":"UnSet");
}

void
AG_PrintVariable(char *s, size_t len, AG_Variable *V)
{
	Uint val;

	switch (V->type) {
	case AG_VARIABLE_UINT:		StrlcpyUint(s, V->data.u, len);				break;
	case AG_VARIABLE_P_UINT:	StrlcpyUint(s, *(Uint *)V->data.p, len);		break;
	case AG_VARIABLE_INT:		StrlcpyInt(s, V->data.i, len);				break;
	case AG_VARIABLE_P_INT:		StrlcpyInt(s, *(int *)V->data.p, len);			break;
	case AG_VARIABLE_ULONG:		Snprintf(s, len, "%lu", V->data.uli);			break;
	case AG_VARIABLE_P_ULONG:	Snprintf(s, len, "%lu", *(Ulong *)V->data.p);		break;
	case AG_VARIABLE_LONG:		Snprintf(s, len, "%ld", V->data.li);			break;
	case AG_VARIABLE_P_LONG:	Snprintf(s, len, "%ld", *(long *)V->data.p);		break;
	case AG_VARIABLE_UINT8:		StrlcpyUint(s, (Uint)V->data.u8, len);			break;
	case AG_VARIABLE_P_UINT8:	StrlcpyUint(s, (Uint)*(Uint8 *)V->data.p, len);		break;
	case AG_VARIABLE_SINT8:		StrlcpyInt(s, (int)V->data.s8, len);			break;
	case AG_VARIABLE_P_SINT8:	StrlcpyInt(s, (int)*(Sint8 *)V->data.p, len);		break;
	case AG_VARIABLE_UINT16:	StrlcpyUint(s, (Uint)V->data.u16, len);			break;
	case AG_VARIABLE_P_UINT16:	StrlcpyUint(s, (Uint)*(Uint16 *)V->data.p, len);	break;
	case AG_VARIABLE_SINT16:	StrlcpyInt(s, (int)V->data.s16, len);			break;
	case AG_VARIABLE_P_SINT16:	StrlcpyInt(s, (int)*(Sint16 *)V->data.p, len);		break;
	case AG_VARIABLE_UINT32:	Snprintf(s, len, "%lu", (Ulong)V->data.u32);		break;
	case AG_VARIABLE_P_UINT32:	Snprintf(s, len, "%lu", (Ulong)*(Uint32 *)V->data.p);	break;
	case AG_VARIABLE_SINT32:	Snprintf(s, len, "%ld", (long)V->data.s32);		break;
	case AG_VARIABLE_P_SINT32:	Snprintf(s, len, "%ld", (long)*(Sint32 *)V->data.p);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_SINT64:	Snprintf(s, len, "%lld", (long long)V->data.s64);		break;
	case AG_VARIABLE_P_SINT64:	Snprintf(s, len, "%lld", (long long)*(Sint64 *)V->data.p);	break;
	case AG_VARIABLE_UINT64:	Snprintf(s, len, "%llu", (unsigned long long)V->data.u64);		break;
	case AG_VARIABLE_P_UINT64:	Snprintf(s, len, "%llu", (unsigned long long)*(Sint64 *)V->data.p);	break;
#endif
	case AG_VARIABLE_FLOAT:		Snprintf(s, len, "%.2f", V->data.flt);			break;
	case AG_VARIABLE_P_FLOAT:	Snprintf(s, len, "%.2f", *(float *)V->data.p);		break;
	case AG_VARIABLE_DOUBLE:	Snprintf(s, len, "%.2f", V->data.dbl);			break;
	case AG_VARIABLE_P_DOUBLE:	Snprintf(s, len, "%.2f", *(double *)V->data.p);		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_P_LONG_DOUBLE:	Snprintf(s, len, "%.2Lf", *(long double *)V->data.p);	break;
#endif
	case AG_VARIABLE_STRING:
		Strlcpy(s, V->data.s, len);
		break;
	case AG_VARIABLE_P_STRING:
		Strlcpy(s, *(char **)V->data.p, len);
		break;
	case AG_VARIABLE_POINTER:
		Snprintf(s, len, "%p", V->data.p);
		break;
	case AG_VARIABLE_P_POINTER:
		Snprintf(s, len, "%p", *(void **)V->data.p);
		break;
	case AG_VARIABLE_P_FLAG:
		val = *(Uint *)V->data.p;
		AG_PrintFlagsVariable(s, len, val, V);
		break;
	case AG_VARIABLE_P_FLAG8:
		val = *(Uint8 *)V->data.p;
		AG_PrintFlagsVariable(s, len, val, V);
		break;
	case AG_VARIABLE_P_FLAG16:
		val = *(Uint16 *)V->data.p;
		AG_PrintFlagsVariable(s, len, val, V);
		break;
	case AG_VARIABLE_P_FLAG32:
		val = *(Uint32 *)V->data.p;
		AG_PrintFlagsVariable(s, len, val, V);
		break;
	default:
		s[0] = '?';
		s[1] = '\0';
		break;
	}
}

/*
 * Lookup a variable by name and return a generic pointer to its current value.
 * If the variable is a reference, the target is accessed. If the variable is
 * function-defined, that function is invoked.
 *
 * The variable is returned locked. Returns NULL if the variable is undefined.
 */
AG_Variable *
AG_GetVariable(void *pObj, const char *name, ...)
{
	AG_Object *obj = pObj;
	void **p;
	AG_Variable *V;
	va_list ap;
	
	va_start(ap, name);
	p = va_arg(ap, void **);
	va_end(ap);

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_SetError("<%s>: No \"%s\"", obj->name, name);
		AG_FatalError(NULL);
	}
	if (V->fn.fnVoid != NULL) {
		AG_EvalVariable(obj, V);
	}
	if (p != NULL) {
		*p = (agVariableTypes[V->type].indirLvl > 0) ?
		    V->data.p : &V->data;
	}
	AG_ObjectUnlock(obj);
	return (V);
}

/*
 * Search for a variable from an a "object-name:prop-name" string,
 * relative to the specified VFS. Returns the Variable locked.
 */
AG_Variable *
AG_GetVariableVFS(void *vfsRoot, const char *varPath)
{
	char sb[AG_OBJECT_PATH_MAX+65];
	char *s = &sb[0], *objName, *varName;
	void *obj;
	AG_Variable *V;

	Strlcpy(sb, varPath, sizeof(sb));
	objName = Strsep(&s, ":");
	varName = Strsep(&s, ":");
	if (objName == NULL || varName == NULL ||
	    objName[0] == '\0' || varName[0] == '\0') {
		AG_SetError(_("Invalid variable path: %s"), varPath);
		return (NULL);
	}
	if ((obj = AG_ObjectFindS(vfsRoot, objName)) == NULL) {
		return (NULL);
	}
	if ((V = AG_AccessVariable(obj, varName)) == NULL) {
		return (NULL);
	}
	if (V->fn.fnVoid != NULL) {
		AG_EvalVariable(obj, V);
	}
	return (V);
}

/* Unset a variable */
void
AG_Unset(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	
	Debug(obj, "Unset \"%s\"\n", name);

	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0) {
			TAILQ_REMOVE(&obj->vars, V, vars);
			AG_FreeVariable(V);
			free(V);
			break;
		}
	}
}

/* Generate "bound" event if BOUND_EVENTS flag is set */
#undef  FN_POST_BOUND_EVENT
#define FN_POST_BOUND_EVENT(obj, V) \
	if (OBJECT(obj)->flags & AG_OBJECT_BOUND_EVENTS) \
		AG_PostEvent(NULL, (obj), "bound", "%p", (V))


/* Body of AG_GetFoo() routines. */
#undef  FN_VARIABLE_GET
#define FN_VARIABLE_GET(_memb,_fn,_type,_getfn)			\
	_type rv;						\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	if ((V = AG_AccessVariable(obj,name)) == NULL) {	\
		AG_SetError("<%s>: Undefined variable \"%s\"",	\
		    OBJECT(obj)->name, name);			\
		AG_FatalError(NULL);				\
	}							\
	if (V->fn._fn != NULL) {				\
		_getfn(obj, V);					\
	}							\
	if (agVariableTypes[V->type].indirLvl > 0) {		\
		rv = *(_type *)V->data.p;			\
	} else {						\
		rv = V->data._memb;				\
	}							\
	AG_UnlockVariable(V);					\
	AG_ObjectUnlock(obj);					\
	return (rv)						\

/* Body of AG_SetFoo() routines. */
#undef  FN_VARIABLE_SET
#define FN_VARIABLE_SET(_memb,_type,ntype)			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariable(obj, name, ntype);			\
	if (agVariableTypes[V->type].indirLvl > 0) {		\
		*(_type *)V->data.p = v;			\
	} else {						\
		V->data._memb = v;				\
	}							\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_BindFoo() routines. */
#undef  FN_VARIABLE_BIND
#define FN_VARIABLE_BIND(ntype)					\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariableOfType(obj, name, ntype);		\
	V->data.p = (void *)v;					\
	FN_POST_BOUND_EVENT(obj, V);				\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_BindFooFn() routines. */
#undef  FN_VARIABLE_BIND_FN
#define FN_VARIABLE_BIND_FN(_memb,ntype)			\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
	AG_Variable *V;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, name, sizeof(evName));			\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariableOfType(obj, name, ntype);		\
	V->fn._memb = fn;					\
	ev = AG_SetEvent(obj, evName, NULL, NULL);		\
	AG_EVENT_GET_ARGS(ev, fmt);				\
	FN_POST_BOUND_EVENT(obj, V);				\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_BindFooMp() routines. */
#undef  FN_VARIABLE_BIND_MP
#define FN_VARIABLE_BIND_MP(ntype)				\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariableOfType(obj, name, ntype);		\
	V->mutex = mutex;					\
	V->data.p = (void *)v;					\
	FN_POST_BOUND_EVENT(obj, V);				\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_GetFooFn() routines. */
#define FN_VARIABLE_GETFN(_field,_fname) do {			\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, V->name, sizeof(evName));		\
	TAILQ_FOREACH(ev, &OBJECT(obj)->events, events) {	\
		if (strcmp(evName, ev->name) == 0)		\
			break;					\
	}							\
	if (ev != NULL) {					\
		V->data._field = V->fn._fname(ev);		\
	}							\
} while (0)


/*
 * Unsigned integer
 */
static void
GetUintFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(u, fnUint);
}
Uint
AG_GetUint(void *obj, const char *name)
{
	FN_VARIABLE_GET(u, fnUint, Uint, GetUintFn);
}
AG_Variable *
AG_SetUint(void *obj, const char *name, Uint v)
{
	Debug(obj, "Set \"%s\" -> (Uint) %u\n", name, v);
	FN_VARIABLE_SET(u, Uint, AG_VARIABLE_UINT);
}
void
AG_InitUint(AG_Variable *V, Uint v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT, "");
	V->data.u = v;
}
AG_Variable *
AG_BindUint(void *obj, const char *name, Uint *v)
{
	Debug(obj, "Bind \"%s\" -> *(Uint *)%p (= 0x%x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT);
}
AG_Variable *
AG_BindUintFn(void *obj, const char *name, AG_UintFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUint, AG_VARIABLE_UINT);
}
AG_Variable *
AG_BindUintMp(void *obj, const char *name, Uint *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Uint *)%p (= %u) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT);
}

/*
 * Signed integer
 */
static void
GetIntFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(i, fnInt);
}
int
AG_GetInt(void *obj, const char *name)
{
	FN_VARIABLE_GET(i, fnInt, int, GetIntFn);
}
AG_Variable *
AG_SetInt(void *obj, const char *name, int v)
{
	Debug(obj, "Set \"%s\" -> (int) %d\n", name, v);
	FN_VARIABLE_SET(i, int, AG_VARIABLE_INT);
}
void
AG_InitInt(AG_Variable *V, int v)
{
	AG_InitVariable(V, AG_VARIABLE_INT, "");
	V->data.i = v;
}
AG_Variable *
AG_BindInt(void *obj, const char *name, int *v)
{
	Debug(obj, "Bind \"%s\" -> *(int *)%p (= %d)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_INT);
}
AG_Variable *
AG_BindIntFn(void *obj, const char *name, AG_IntFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnInt, AG_VARIABLE_INT);
}
AG_Variable *
AG_BindIntMp(void *obj, const char *name, int *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(int *)%p (= %d) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_INT);
}

/*
 * Unsigned long integer
 */

static void
GetUlongFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(uli, fnUlong);
}
Ulong
AG_GetUlong(void *obj, const char *name)
{
	FN_VARIABLE_GET(uli, fnUlong, Ulong, GetUlongFn);
}
AG_Variable *
AG_SetUlong(void *obj, const char *name, Ulong v)
{
	Debug(obj, "Set \"%s\" -> (Ulong) %lu\n", name, v);
	FN_VARIABLE_SET(uli, Ulong, AG_VARIABLE_ULONG);
}
void
AG_InitUlong(AG_Variable *V, Ulong v)
{
	AG_InitVariable(V, AG_VARIABLE_ULONG, "");
	V->data.uli = v;
}
AG_Variable *
AG_BindUlong(void *obj, const char *name, Ulong *v)
{
	Debug(obj, "Bind \"%s\" -> *(Ulong *)%p (= %lu)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_ULONG);
}
AG_Variable *
AG_BindUlongFn(void *obj, const char *name, AG_UlongFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUlong, AG_VARIABLE_ULONG);
}
AG_Variable *
AG_BindUlongMp(void *obj, const char *name, Ulong *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Ulong *)%p (= %lu) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_ULONG);
}

/*
 * Signed long integer.
 */
static void
GetLongFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(li, fnLong);
}
long
AG_GetLong(void *obj, const char *name)
{
	FN_VARIABLE_GET(li, fnLong, long, GetLongFn);
}
AG_Variable *
AG_SetLong(void *obj, const char *name, long v)
{
	Debug(obj, "Set \"%s\" -> (long) %ld\n", name, v);
	FN_VARIABLE_SET(li, long, AG_VARIABLE_LONG);
}
void
AG_InitLong(AG_Variable *V, long v)
{
	AG_InitVariable(V, AG_VARIABLE_LONG, "");
	V->data.li = v;
}
AG_Variable *
AG_BindLong(void *obj, const char *name, long *v)
{
	Debug(obj, "Bind \"%s\" -> *(long *)%p (= %ld)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_LONG);
}
AG_Variable *
AG_BindLongFn(void *obj, const char *name, AG_LongFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnLong, AG_VARIABLE_LONG);
}
AG_Variable *
AG_BindLongMp(void *obj, const char *name, long *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(long *)%p (= %ld) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_LONG);
}

/*
 * Unsigned 8-bit integer
 */
static void
GetUint8Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(u8, fnUint8);
}
Uint8
AG_GetUint8(void *obj, const char *name)
{
	FN_VARIABLE_GET(u8, fnUint8, Uint8, GetUint8Fn);
}
AG_Variable *
AG_SetUint8(void *obj, const char *name, Uint8 v)
{
	Debug(obj, "Set \"%s\" -> (Uint8) 0x%02x\n", name, v);
	FN_VARIABLE_SET(u8, Uint8, AG_VARIABLE_UINT8);
}
void
AG_InitUint8(AG_Variable *V, Uint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT8, "");
	V->data.u8 = v;
}
AG_Variable *
AG_BindUint8(void *obj, const char *name, Uint8 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Uint8 *)%p (= 0x%02x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT8);
}
AG_Variable *
AG_BindUint8Fn(void *obj, const char *name, AG_Uint8Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUint8, AG_VARIABLE_UINT8);
}
AG_Variable *
AG_BindUint8Mp(void *obj, const char *name, Uint8 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Uint8 *)%p (= 0x%02x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT8);
}

/*
 * Signed 8-bit integer
 */
static void
GetSint8Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(s8, fnSint8);
}
Sint8
AG_GetSint8(void *obj, const char *name)
{
	FN_VARIABLE_GET(s8, fnSint8, Sint8, GetSint8Fn);
}
AG_Variable *
AG_SetSint8(void *obj, const char *name, Sint8 v)
{
	Debug(obj, "Set \"%s\" -> (Sint8) 0x%02x\n", name, v);
	FN_VARIABLE_SET(s8, Sint8, AG_VARIABLE_SINT8);
}
void
AG_InitSint8(AG_Variable *V, Sint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT8, "");
	V->data.s8 = v;
}
AG_Variable *
AG_BindSint8(void *obj, const char *name, Sint8 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Sint8 *)%p (= 0x%02x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT8);
}
AG_Variable *
AG_BindSint8Fn(void *obj, const char *name, AG_Sint8Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnSint8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindSint8Mp(void *obj, const char *name, Sint8 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Sint8 *)%p (= 0x%02x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT8);
}

/*
 * Unsigned 16-bit integer
 */
static void
GetUint16Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(u16, fnUint16);
}
Uint16
AG_GetUint16(void *obj, const char *name)
{
	FN_VARIABLE_GET(u16, fnUint16, Uint16, GetUint16Fn);
}
AG_Variable *
AG_SetUint16(void *obj, const char *name, Uint16 v)
{
	Debug(obj, "Set \"%s\" -> (Uint16) 0x%04x\n", name, v);
	FN_VARIABLE_SET(u16, Uint16, AG_VARIABLE_UINT16);
}
void
AG_InitUint16(AG_Variable *V, Uint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT16, "");
	V->data.u16 = v;
}
AG_Variable *
AG_BindUint16(void *obj, const char *name, Uint16 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Uint16 *)%p (= 0x%04x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT16);
}
AG_Variable *
AG_BindUint16Fn(void *obj, const char *name, AG_Uint16Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUint16, AG_VARIABLE_UINT16);
}
AG_Variable *
AG_BindUint16Mp(void *obj, const char *name, Uint16 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Uint16 *)%p (= 0x%04x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT16);
}

/*
 * Signed 16-bit integer
 */
static void
GetSint16Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(s16, fnSint16);
}
Sint16
AG_GetSint16(void *obj, const char *name)
{
	FN_VARIABLE_GET(s16, fnSint16, Sint16, GetSint16Fn);
}
AG_Variable *
AG_SetSint16(void *obj, const char *name, Sint16 v)
{
	Debug(obj, "Set \"%s\" -> (Sint16) 0x%04x\n", name, v);
	FN_VARIABLE_SET(s16, Sint16, AG_VARIABLE_SINT16);
}
void
AG_InitSint16(AG_Variable *V, Sint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT16, "");
	V->data.s16 = v;
}
AG_Variable *
AG_BindSint16(void *obj, const char *name, Sint16 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Sint16 *)%p (= 0x%04x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT16);
}
AG_Variable *
AG_BindSint16Fn(void *obj, const char *name, AG_Sint16Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnSint16, AG_VARIABLE_SINT16);
}
AG_Variable *
AG_BindSint16Mp(void *obj, const char *name, Sint16 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Sint16 *)%p (= 0x%04x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT16);
}

/*
 * Unsigned 32-bit integer
 */
static void
GetUint32Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(u32, fnUint32);
}
Uint32
AG_GetUint32(void *obj, const char *name)
{
	FN_VARIABLE_GET(u32, fnUint32, Uint32, GetUint32Fn);
}
AG_Variable *
AG_SetUint32(void *obj, const char *name, Uint32 v)
{
	Debug(obj, "Set \"%s\" -> (Uint32) 0x%08x\n", name, v);
	FN_VARIABLE_SET(u32, Uint32, AG_VARIABLE_UINT32);
}
void
AG_InitUint32(AG_Variable *V, Uint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT32, "");
	V->data.u32 = v;
}
AG_Variable *
AG_BindUint32(void *obj, const char *name, Uint32 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Uint32 *)%p (= 0x%08x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT32);
}
AG_Variable *
AG_BindUint32Fn(void *obj, const char *name, AG_Uint32Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUint32, AG_VARIABLE_UINT32);
}
AG_Variable *
AG_BindUint32Mp(void *obj, const char *name, Uint32 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Uint32 *)%p (= 0x%08x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT32);
}

/*
 * Signed 32-bit integer
 */
static void
GetSint32Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(s32, fnSint32);
}
Sint32
AG_GetSint32(void *obj, const char *name)
{
	FN_VARIABLE_GET(s32, fnSint32, Sint32, GetSint32Fn);
}
AG_Variable *
AG_SetSint32(void *obj, const char *name, Sint32 v)
{
	Debug(obj, "Set \"%s\" -> (Sint32) 0x%08x\n", name, v);
	FN_VARIABLE_SET(s32, Sint32, AG_VARIABLE_SINT32);
}
void
AG_InitSint32(AG_Variable *V, Sint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT32, "");
	V->data.s32 = v;
}
AG_Variable *
AG_BindSint32(void *obj, const char *name, Sint32 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Sint32 *)%p (= 0x%08x)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT32);
}
AG_Variable *
AG_BindSint32Fn(void *obj, const char *name, AG_Sint32Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnSint32, AG_VARIABLE_SINT32);
}
AG_Variable *
AG_BindSint32Mp(void *obj, const char *name, Sint32 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Sint32 *)%p (= 0x%08x) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT32);
}

#ifdef HAVE_64BIT
/*
 * Unsigned 64-bit integer
 */
static void
GetUint64Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(u64, fnUint64);
}
Uint64
AG_GetUint64(void *obj, const char *name)
{
	FN_VARIABLE_GET(u64, fnUint64, Uint64, GetUint64Fn);
}
AG_Variable *
AG_SetUint64(void *obj, const char *name, Uint64 v)
{
	Debug(obj, "Set \"%s\" -> (Sint64) 0x%lx\n", name, v);
	FN_VARIABLE_SET(u64, Uint64, AG_VARIABLE_UINT64);
}
void
AG_InitUint64(AG_Variable *V, Uint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT64, "");
	V->data.u64 = v;
}
AG_Variable *
AG_BindUint64(void *obj, const char *name, Uint64 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Sint64)%p (= 0x%lx)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT64);
}
AG_Variable *
AG_BindUint64Fn(void *obj, const char *name, AG_Uint64Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnUint64, AG_VARIABLE_UINT64);
}
AG_Variable *
AG_BindUint64Mp(void *obj, const char *name, Uint64 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Sint64)%p (= 0x%lx) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT64);
}

/*
 * Signed 64-bit integer
 */
static void
GetSint64Fn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(s64, fnSint64);
}
Sint64
AG_GetSint64(void *obj, const char *name)
{
	FN_VARIABLE_GET(s64, fnSint64, Sint64, GetSint64Fn);
}
AG_Variable *
AG_SetSint64(void *obj, const char *name, Sint64 v)
{
	Debug(obj, "Set \"%s\" -> (Sint64) 0x%lx\n", name, v);
	FN_VARIABLE_SET(s64, Sint64, AG_VARIABLE_SINT64);
}
void
AG_InitSint64(AG_Variable *V, Sint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT64, "");
	V->data.s64 = v;
}
AG_Variable *
AG_BindSint64(void *obj, const char *name, Sint64 *v)
{
	Debug(obj, "Bind \"%s\" -> *(Sint64)%p (= 0x%lx)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT64);
}
AG_Variable *
AG_BindSint64Fn(void *obj, const char *name, AG_Sint64Fn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnSint64, AG_VARIABLE_SINT64);
}
AG_Variable *
AG_BindSint64Mp(void *obj, const char *name, Sint64 *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(Sint64)%p (= 0x%lx) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT64);
}
#endif /* HAVE_64BIT */

/*
 * Single-precision floating-point number.
 */
static void
GetFloatFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(flt, fnFloat);
}
float
AG_GetFloat(void *obj, const char *name)
{
	FN_VARIABLE_GET(flt, fnFloat, float, GetFloatFn);
}
AG_Variable *
AG_SetFloat(void *obj, const char *name, float v)
{
	Debug(obj, "Set \"%s\" -> (float) %f\n", name, v);
	FN_VARIABLE_SET(flt, float, AG_VARIABLE_FLOAT);
}
void
AG_InitFloat(AG_Variable *V, float v)
{
	AG_InitVariable(V, AG_VARIABLE_FLOAT, "");
	V->data.flt = v;
}
AG_Variable *
AG_BindFloat(void *obj, const char *name, float *v)
{
	Debug(obj, "Bind \"%s\" -> *(float)%p (= %f)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_FLOAT);
}
AG_Variable *
AG_BindFloatFn(void *obj, const char *name, AG_FloatFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnFloat, AG_VARIABLE_FLOAT);
}
AG_Variable *
AG_BindFloatMp(void *obj, const char *name, float *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(float)%p (= %f) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_FLOAT);
}

/*
 * Double-precision floating-point number.
 */
static void
GetDoubleFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(dbl, fnDouble);
}
double
AG_GetDouble(void *obj, const char *name)
{
	FN_VARIABLE_GET(dbl, fnDouble, double, GetDoubleFn);
}
AG_Variable *
AG_SetDouble(void *obj, const char *name, double v)
{
	Debug(obj, "Set \"%s\" -> (double) %f\n", name, v);
	FN_VARIABLE_SET(dbl, double, AG_VARIABLE_DOUBLE);
}
void
AG_InitDouble(AG_Variable *V, double v)
{
	AG_InitVariable(V, AG_VARIABLE_DOUBLE, "");
	V->data.dbl = v;
}
AG_Variable *
AG_BindDouble(void *obj, const char *name, double *v)
{
	Debug(obj, "Bind \"%s\" -> *(double)%p (= %f)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_DOUBLE);
}
AG_Variable *
AG_BindDoubleFn(void *obj, const char *name, AG_DoubleFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnDouble, AG_VARIABLE_DOUBLE);
}
AG_Variable *
AG_BindDoubleMp(void *obj, const char *name, double *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(double)%p (= %f) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_DOUBLE);
}

#ifdef HAVE_LONG_DOUBLE
/*
 * Quad-precision floating-point number.
 */
static void
GetLongDoubleFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(ldbl, fnLongDouble);
}
long double
AG_GetLongDouble(void *obj, const char *name)
{
	FN_VARIABLE_GET(ldbl, fnLongDouble, long double, GetLongDoubleFn);
}
AG_Variable *
AG_SetLongDouble(void *obj, const char *name, long double v)
{
	Debug(obj, "Set \"%s\" -> (long double) %Lf\n", name, v);
	FN_VARIABLE_SET(ldbl, long double, AG_VARIABLE_LONG_DOUBLE);
}
void
AG_InitLongDouble(AG_Variable *V, long double v)
{
	AG_InitVariable(V, AG_VARIABLE_LONG_DOUBLE, "");
	V->data.ldbl = v;
}
AG_Variable *
AG_BindLongDouble(void *obj, const char *name, long double *v)
{
	Debug(obj, "Bind \"%s\" -> *(long double)%p (= %Lf)\n", name, v, *v);
	FN_VARIABLE_BIND(AG_VARIABLE_P_LONG_DOUBLE);
}
AG_Variable *
AG_BindLongDoubleFn(void *obj, const char *name, AG_LongDoubleFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnLongDouble, AG_VARIABLE_LONG_DOUBLE);
}
AG_Variable *
AG_BindLongDoubleMp(void *obj, const char *name, long double *v, AG_Mutex *mutex)
{
	Debug(obj, "Bind \"%s\" -> *(long double)%p (= %Lf) mutex %p\n", name, v, *v, mutex);
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_LONG_DOUBLE);
}
#endif /* HAVE_LONG_DOUBLE */

/*
 * Pointer routines.
 */
static void
GetPointerFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(p, fnPointer);
}
void *
AG_GetPointer(void *obj, const char *name)
{
	FN_VARIABLE_GET(p, fnPointer, void *, GetPointerFn);
}
AG_Variable *
AG_SetPointer(void *obj, const char *name, void *v)
{
	Debug(obj, "Set \"%s\" -> (void *)%p\n", name, v);
	FN_VARIABLE_SET(p, void *, AG_VARIABLE_POINTER);
}
void
AG_InitPointer(AG_Variable *V, void *v)
{
	AG_InitVariable(V, AG_VARIABLE_POINTER, "");
	V->data.p = v;
}
AG_Variable *
AG_BindPointer(void *obj, const char *name, void **v)
{
#ifdef AG_DEBUG
	if (v != NULL) {
		Debug(obj, "Bind \"%s\" -> *(void *)%p (= %p)\n", name, v, *v);
	} else {
		Debug(obj, "Bind \"%s\" -> *(void *)%p (= NULL)\n", name, v);
	}
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_POINTER);
}
AG_Variable *
AG_BindPointerFn(void *obj, const char *name, AG_PointerFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnPointer, AG_VARIABLE_POINTER);
}
AG_Variable *
AG_BindPointerMp(void *obj, const char *name, void **v, AG_Mutex *mutex)
{
#ifdef AG_DEBUG
	if (v != NULL) {
		Debug(obj, "Bind \"%s\" -> *(void *)%p (= %p) mutex %p\n", name, v, *v, mutex);
	} else {
		Debug(obj, "Bind \"%s\" -> *(void *)%p (= NULL) mutex %p\n", name, v, mutex);
	}
#endif
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_POINTER);
	return (V);
}

/*
 * String get/set routines.
 */
static size_t
GetStringFn(void *obj, AG_Variable *V, char *dst, size_t dstSize)
{
	char evName[AG_EVENT_NAME_MAX];
	AG_Event *ev;
	size_t rv;

	Strlcpy(evName, "get-", sizeof(evName));
	Strlcat(evName, V->name, sizeof(evName));
	TAILQ_FOREACH(ev, &OBJECT(obj)->events, events) {
		if (strcmp(evName, ev->name) == 0)
			break;
	}
	rv = (V->fn.fnString != NULL) ?
	      V->fn.fnString(ev, dst, dstSize) : 0;
	return (rv);
}
size_t
AG_GetString(void *pObj, const char *name, char *dst, size_t dstSize)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	size_t rv;

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_SetError("<%s>: Undefined variable \"%s\"", obj->name, name);
		AG_FatalError(NULL);
	}
	if (V->fn.fnString != NULL) {
		rv = GetStringFn(obj, V, dst, dstSize);
	} else {
		Strlcpy(dst, V->data.s, dstSize);
		rv = strlen(V->data.s);
	}
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (rv);
}
char *
AG_GetStringDup(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_SetError("<%s>: Undefined variable \"%s\"", obj->name, name);
		AG_FatalError(NULL);
	}
	if (V->fn.fnString != NULL) {
		if ((s = TryMalloc(V->info.size)) == NULL) {
			goto fail;
		}
		(void)GetStringFn(obj, V, s, V->info.size);
	} else {
		s = TryStrdup(V->data.s);
	}
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (s);
fail:
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (NULL);
}
/*
 * Return a direct pointer to a string buffer.
 * Not free-threaded. The object must be locked.
 */
char *
AG_GetStringP(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_SetError("<%s>: Undefined variable \"%s\"", obj->name, name);
		AG_FatalError(NULL);
	}
	s = V->data.s;
	AG_UnlockVariable(V);
	return (s);
}

/*
 * Set the value of a string variable.
 *
 * - If var exists as an unbounded string, then replace it with a copy of s.
 * - If var references a bounded string buffer, then safe copy s into it.
 * - If var is a different type, then mutate into a STRING with a copy of s.
 */
AG_Variable *
AG_SetString(void *pObj, const char *name, const char *s)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Set \"%s\" -> \"%s\"\n", name, s);
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(AG_Variable));
		AG_InitVariable(V, AG_VARIABLE_STRING, name);
		TAILQ_INSERT_TAIL(&obj->vars, V, vars);

		V->info.size = 0;				/* Allocated */
		V->data.s = Strdup(s);
	} else {
		switch (V->type) {
		case AG_VARIABLE_STRING:
			if (V->data.s != NULL && V->info.size == 0) {
				free(V->data.s);
			}
			V->data.s = Strdup(s);
			V->info.size = 0;
			break;
		case AG_VARIABLE_P_STRING:
			AG_LockVariable(V);
#ifdef AG_DEBUG
			if (strlen(s) >= V->info.size)
				Debug(obj, "P_STRING: >= %lu bytes", V->info.size);
#endif
			Strlcpy(V->data.s, s, V->info.size);
			AG_UnlockVariable(V);
			break;
		default:
			Debug(obj, "Mutating \"%s\": From (%s) to (%s)\n", name,
			    agVariableTypes[V->type].name,
			    agVariableTypes[AG_VARIABLE_STRING].name);
			AG_FreeVariable(V);
			AG_InitVariable(V, AG_VARIABLE_STRING, name);
			V->info.size = 0;
			V->data.s = Strdup(s);
			break;
		}
	}
	AG_ObjectUnlock(obj);
	return (V);
}
void
AG_InitString(AG_Variable *V, const char *v)
{
	AG_InitVariable(V, AG_VARIABLE_STRING, "");
	V->data.s = Strdup(v);
	V->info.size = 0;
}
void
AG_InitStringNODUP(AG_Variable *V, char *v)
{
	AG_InitVariable(V, AG_VARIABLE_STRING, "");
	V->data.s = v;
	V->info.size = 0;
}

/*
 * Variant of AG_SetString() where s is assumed to be a freeable,
 * auto-allocated string (which must not be freed externally).
 */
AG_Variable *
AG_SetStringNODUP(void *obj, const char *name, char *s)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Set \"%s\" -> *(char *)%p<NODUP> (= \"%s\")\n", name, s, s);
	V = AG_FetchVariable(obj, name, AG_VARIABLE_STRING);
	switch (V->type) {
	case AG_VARIABLE_STRING:
		if (V->data.s != NULL && V->info.size == 0) {
			free(V->data.s);
		}
		V->data.s = s;
		V->info.size = 0;
		break;
	case AG_VARIABLE_P_STRING:
		AG_LockVariable(V);
#ifdef AG_DEBUG
		if (strlen(s) >= V->info.size)
			Debug(obj, "P_STRING: >= %lu bytes", V->info.size);
#endif
		Strlcpy(V->data.s, s, V->info.size);
		AG_UnlockVariable(V);
		break;
	default:
		Debug(obj, "Mutating \"%s\": From (%s) to (%s)\n", name,
		    agVariableTypes[V->type].name,
		    agVariableTypes[AG_VARIABLE_STRING].name);
		AG_FreeVariable(V);
		AG_InitVariable(V, AG_VARIABLE_STRING, name);
		V->data.s = s;
		V->info.size = 0;
		break;
	}
	AG_ObjectUnlock(obj);
	return (V);
}

/* Printf-style variant of AG_SetString() */
AG_Variable *
AG_PrtString(void *obj, const char *name, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return AG_SetStringNODUP(obj, name, s);
}

AG_Variable *
AG_BindString(void *obj, const char *name, char *buf, size_t bufSize)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> (char *)%p[+%lu] (= \"%s\")\n", name, buf, bufSize, buf);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->data.s = buf;
	V->info.size = bufSize;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindStringFn(void *obj, const char *name, AG_StringFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnString, AG_VARIABLE_STRING);
}
AG_Variable *
AG_BindStringMp(void *obj, const char *name, char *v, size_t size,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> (char *)%p[+%lu] (= \"%s\") mutex %p\n",
	    name, v, size, v, mutex);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->mutex = mutex;
	V->data.s = v;
	V->info.size = size;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

/*
 * Bitwise flag routines.
 */
AG_Variable *
AG_BindFlag(void *obj, const char *name, Uint *v, Uint bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(int *)%p (= 0x%x) mask 0x%x\n", name, v, *v, bitmask);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlagMp(void *obj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(int *)%p (= 0x%x) mask 0x%x mutex %p\n", name, v, *v, bitmask, mutex);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8(void *obj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint8 *)%p (= 0x%02x) mask 0x%02x\n", name, v, *v, bitmask);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8Mp(void *obj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint8 *)%p (= 0x%02x) mask 0x%02x mutex %p\n", name, v, *v, bitmask, mutex);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16(void *obj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint16 *)%p (= 0x%04x) mask 0x%04x\n", name, v, *v, bitmask);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16Mp(void *obj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint16 *)%p (= 0x%04x) mask 0x%04x mutex %p\n", name, v, *v, bitmask, mutex);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32(void *obj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint32 *)%p (= 0x%08x) mask 0x%08x\n", name, v, *v, bitmask);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32Mp(void *obj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(Uint32 *)%p (0x%08x) mask 0x%08x mutex %p\n", name, v, *v, bitmask, mutex);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Create a Variable to Object reference. */
AG_Variable *
AG_BindObject(void *obj, const char *name, void *tgtObj)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> *(%s *)%p (= <%s>)\n", name,
	    OBJECT_CLASS(tgtObj)->name, tgtObj, OBJECT(tgtObj)->name);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_OBJECT);
	V->data.p = tgtObj;
	V->info.objName = NULL;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Create a Variable to Object:[Variable] reference. */
AG_Variable *
AG_BindVariable(void *obj, const char *name, void *tgtObj, const char *tgtKey)
{
	AG_Variable *V;
	char *keyDup;

	if ((keyDup = TryStrdup(tgtKey)) == NULL)
		return (NULL);

	AG_ObjectLock(obj);
	Debug(obj, "Bind \"%s\" -> [*(%s *)%p (= %s)]:%s\n", name,
	    OBJECT_CLASS(tgtObj)->name, tgtObj, OBJECT(tgtObj)->name,
	    tgtKey);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_VARIABLE);
	V->data.p = tgtObj;
	V->info.varName = keyDup;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Substitute variable references of the form "$(foo)" in a string. */
void
AG_VariableSubst(void *obj, const char *s, char *dst, size_t len)
{
	char key[AG_VARIABLE_NAME_MAX], val[64];
	const char *c, *cEnd;
	AG_Variable *V;
	size_t kLen;

	if (len < 1) {
		return;
	}
	AG_ObjectLock(obj);
	dst[0] = '\0';
	for (c = &s[0]; *c != '\0'; ) {
		if (c[0] == '$' && c[1] == '(' &&
		    (cEnd = strchr(&c[2], ')')) != NULL) {
			if ((kLen = cEnd-&c[1]) >= sizeof(key)) {
				c = &cEnd[1];
				continue;
			}
			memcpy(key, &c[2], kLen);
			key[kLen-1] = '\0';
			if (strcmp(key, "name") == 0) {
				Strlcat(dst, OBJECT(obj)->name, len);
			} else if (strcmp(key, "class") == 0) {
				Strlcat(dst, OBJECT(obj)->cls->name, len);
			} else {
				if ((V = AG_AccessVariable(obj, key)) != NULL) {
					AG_PrintVariable(val, sizeof(val), V);
					Strlcat(dst, val, len);
					AG_UnlockVariable(V);
				}
#if 0
				} else {
					Strlcat(dst, "$(", len);
					Strlcat(dst, key, len);
					Strlcat(dst, ")", len);
				}
#endif
			}
			c = cEnd+1;
			continue;
		}
		val[0] = *c;
		val[1] = '\0';
		Strlcat(dst, val, len);
		c++;
	}
	AG_ObjectUnlock(obj);
}
