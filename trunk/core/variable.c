/*
 * Copyright (c) 2008-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
	{ AG_VARIABLE_NULL,		0,	"NULL",		AG_VARIABLE_NULL,		-1,	0 },
	/*
	 * Primitive types
	 */
	{ AG_VARIABLE_UINT,		0,	"Uint",		AG_VARIABLE_UINT,		0,	sizeof(Uint) },
	{ AG_VARIABLE_P_UINT,		1,	"Uint *",	AG_VARIABLE_UINT,		0,	sizeof(Uint) },
	{ AG_VARIABLE_INT,		0,	"int",		AG_VARIABLE_INT,		1,	sizeof(int) },
	{ AG_VARIABLE_P_INT,		1,	"int *",	AG_VARIABLE_INT,		1,	sizeof(int) },
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
	{ AG_VARIABLE_UINT64,		0,	"Uint64",	AG_VARIABLE_UINT64,		8,	sizeof(Uint64) },
	{ AG_VARIABLE_P_UINT64,		1,	"Uint64 *",	AG_VARIABLE_UINT64,		8,	sizeof(Uint64) },
	{ AG_VARIABLE_SINT64,		0,	"Sint64",	AG_VARIABLE_SINT64,		9,	sizeof(Sint64) },
	{ AG_VARIABLE_P_SINT64,		1,	"Sint64 *",	AG_VARIABLE_SINT64,		9,	sizeof(Sint64) },
	{ AG_VARIABLE_FLOAT,		0,	"float",	AG_VARIABLE_FLOAT,		10,	sizeof(float) },
	{ AG_VARIABLE_P_FLOAT,		1,	"float *",	AG_VARIABLE_FLOAT,		10,	sizeof(float) },
	{ AG_VARIABLE_DOUBLE,		0,	"double",	AG_VARIABLE_DOUBLE,		11,	sizeof(double) },
	{ AG_VARIABLE_P_DOUBLE,		1,	"double *",	AG_VARIABLE_DOUBLE,		11,	sizeof(double) },
	{ AG_VARIABLE_LONG_DOUBLE,	0,	"long double",	AG_VARIABLE_LONG_DOUBLE,	12,	sizeof(long double) },
	{ AG_VARIABLE_P_LONG_DOUBLE,	1,	"long double *",AG_VARIABLE_LONG_DOUBLE,	12,	sizeof(long double) },
	{ AG_VARIABLE_STRING,		0,	"Str",		AG_VARIABLE_STRING,		13,	sizeof(char *) },
	{ AG_VARIABLE_P_STRING,		1,	"Str *",	AG_VARIABLE_STRING,		13,	sizeof(char *) },
	{ AG_VARIABLE_CONST_STRING,	0,	"Const Str",	AG_VARIABLE_CONST_STRING, 	13,	sizeof(const char *) },
	{ AG_VARIABLE_P_CONST_STRING,	1,	"Const Str *",	AG_VARIABLE_CONST_STRING, 	13,	sizeof(const char *) },
	{ AG_VARIABLE_POINTER,		0,	"Ptr",		AG_VARIABLE_POINTER,		-1,	sizeof(void *) },
	{ AG_VARIABLE_P_POINTER,	1,	"Ptr *",	AG_VARIABLE_POINTER,		-1,	sizeof(void *) },
	{ AG_VARIABLE_CONST_POINTER,	0,	"Const Ptr",	AG_VARIABLE_CONST_POINTER,	-1,	sizeof(const void *) },
	{ AG_VARIABLE_P_CONST_POINTER,	1,	"Const Ptr *",	AG_VARIABLE_CONST_POINTER,	-1,	sizeof(const void *) },
	/*
	 * Bitmask-specific types
	 */
	{ AG_VARIABLE_P_FLAG,		1,	"Flag *",	AG_VARIABLE_P_FLAG,		-1,	sizeof(int) },
	{ AG_VARIABLE_P_FLAG8,		1,	"Flag8 *",	AG_VARIABLE_P_FLAG8,		-1,	1 },
	{ AG_VARIABLE_P_FLAG16,		1,	"Flag16 *",	AG_VARIABLE_P_FLAG16,		-1,	2 },
	{ AG_VARIABLE_P_FLAG32,		1,	"Flag32 *",	AG_VARIABLE_P_FLAG32,		-1,	4 },
	/*
	 * Agar-Core specific types
	 */
	{ AG_VARIABLE_P_OBJECT,		1,	"Object *",	AG_VARIABLE_P_OBJECT,		-1,	sizeof(void *) },
	{ AG_VARIABLE_P_TEXT,		1,	"Text *",	AG_VARIABLE_P_TEXT,		-1,	sizeof(void *) },
	{ AG_VARIABLE_P_VARIABLE,	1,	"Variable *",	AG_VARIABLE_P_VARIABLE,		-1,	sizeof(void *) },
};

/*
 * Duplicate a Variable. Pointer references are preserved; internally
 * allocated strings are duplicated (references to string buffers are
 * preserved).
 */
int
AG_CopyVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	if (Vsrc->type == AG_VARIABLE_STRING &&
	    Vsrc->info.size == 0) {
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL)
			return (-1);
	}
	return (0);
}

/*
 * Duplicate a Variable with dereference. Pointer references are converted to
 * immediate values and strings are duplicated unconditionally.
 */
int
AG_DerefVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	switch (Vsrc->type) {
	case AG_VARIABLE_P_UINT:	Vdst->type = AG_VARIABLE_UINT;		Vdst->data.u = *(Uint *)Vsrc->data.p;		break;
	case AG_VARIABLE_P_INT:		Vdst->type = AG_VARIABLE_INT;		Vdst->data.i = *(int *)Vsrc->data.p;		break;
	case AG_VARIABLE_P_UINT8:	Vdst->type = AG_VARIABLE_UINT8;		Vdst->data.u8 = *(Uint8 *)Vsrc->data.p;		break;
	case AG_VARIABLE_P_SINT8:	Vdst->type = AG_VARIABLE_SINT8;		Vdst->data.s8 = *(Sint8 *)Vsrc->data.p;		break;
	case AG_VARIABLE_P_UINT16:	Vdst->type = AG_VARIABLE_UINT16;	Vdst->data.u16 = *(Uint16 *)Vsrc->data.p;	break;
	case AG_VARIABLE_P_SINT16:	Vdst->type = AG_VARIABLE_SINT16;	Vdst->data.s16 = *(Sint16 *)Vsrc->data.p;	break;
	case AG_VARIABLE_P_UINT32:	Vdst->type = AG_VARIABLE_UINT32;	Vdst->data.u32 = *(Uint32 *)Vsrc->data.p;	break;
	case AG_VARIABLE_P_SINT32:	Vdst->type = AG_VARIABLE_SINT32;	Vdst->data.s32 = *(Sint32 *)Vsrc->data.p;	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_P_UINT64:	Vdst->type = AG_VARIABLE_UINT64;	Vdst->data.u64 = *(Uint64 *)Vsrc->data.p;	break;
	case AG_VARIABLE_P_SINT64:	Vdst->type = AG_VARIABLE_SINT64;	Vdst->data.s64 = *(Sint64 *)Vsrc->data.p;	break;
#endif
	case AG_VARIABLE_P_FLOAT:	Vdst->type = AG_VARIABLE_FLOAT;		Vdst->data.flt = *(float *)Vsrc->data.p;	break;
	case AG_VARIABLE_P_DOUBLE:	Vdst->type = AG_VARIABLE_DOUBLE;	Vdst->data.dbl = *(double *)Vsrc->data.p;	break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_P_LONG_DOUBLE:	Vdst->type = AG_VARIABLE_LONG_DOUBLE;	Vdst->data.ldbl = *(long double *)Vsrc->data.p;	break;
#endif
	case AG_VARIABLE_P_POINTER:	Vdst->type = AG_VARIABLE_POINTER;	Vdst->data.p = *(void **)Vsrc->data.p;		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL) {
			return (-1);
		}
		Vdst->info.size = 0;
		break;
	default:
		break;
	}
	return (0);
}

/* Compare two variables (without dereference). */
int
AG_CompareVariables(const AG_Variable *a, const AG_Variable *b)
{
	size_t size;

	if (a->type != b->type) {
		return (1);
	}
	size = agVariableTypes[a->type].size;
	if (size == 0) {
		return (1);
	} else {
		return memcmp((const void *)&a->data, (const void *)&b->data,
		    agVariableTypes[a->type].size);
	}
}

/* Evaluate the value of a variable from any associated function. */
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
		AG_SetError("Missing get-%s event", V->name);
		return (-1);
	}
	switch (V->type) {
	case AG_VARIABLE_UINT:		V->data.u = V->fn.fnUint(ev);		break;
	case AG_VARIABLE_INT:		V->data.i = V->fn.fnInt(ev);		break;
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
	case AG_VARIABLE_CONST_POINTER:	V->data.Cp = V->fn.fnConstPointer(ev);	break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_CONST_STRING:
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
	case AG_VARIABLE_CONST_STRING:
		Strlcpy(s, V->data.s, len);
		break;
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		Strlcpy(s, V->data.s, len);
		break;
	case AG_VARIABLE_POINTER:
	case AG_VARIABLE_CONST_POINTER:
		Snprintf(s, len, "%p", V->data.p);
		break;
	case AG_VARIABLE_P_POINTER:
	case AG_VARIABLE_P_CONST_POINTER:
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
	if ((V = AG_GetVariableLocked(obj, name)) == NULL) {
		goto fail;
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
fail:
	AG_ObjectUnlock(obj);
	return (NULL);
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
	if ((V = AG_GetVariableLocked(obj, varName)) == NULL) {
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

	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0) {
			TAILQ_REMOVE(&obj->vars, V, vars);
			AG_FreeVariable(V);
			free(V);
			break;
		}
	}
}

/* Body of AG_GetFoo() routines. */
#undef  FN_VARIABLE_GET
#define FN_VARIABLE_GET(_memb,_fn,_type,_getfn)			\
	_type rv;						\
	AG_Variable *V;						\
								\
	if ((V = AG_GetVariableLocked(obj,name)) == NULL) {	\
		Debug(obj, "No such variable: %s\n", name);	\
		return (_type)(0);				\
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
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
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
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
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
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

/*
 * Body of our internal GetFooFn() routines which are invoked when
 * a function must be evaluated. We store the function arguments into
 * a "get-foo" event to avoid growing the AG_Variable structure.
 */
#define FN_VARIABLE_GETFN(_field,_fname) do {			\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, V->name, sizeof(evName));		\
	AG_ObjectLock(obj);					\
	TAILQ_FOREACH(ev, &OBJECT(obj)->events, events) {	\
		if (strcmp(evName, ev->name) == 0)		\
			break;					\
	}							\
	if (ev != NULL) {					\
		V->data._field = V->fn._fname(ev);		\
	}							\
	AG_ObjectUnlock(obj);					\
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
	FN_VARIABLE_SET(u, Uint, AG_VARIABLE_UINT);
}
void
AG_InitUint(AG_Variable *V, Uint v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT);
	V->name[0] = '\0';
	V->data.u = v;
}
AG_Variable *
AG_BindUint(void *obj, const char *name, Uint *v)
{
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
	FN_VARIABLE_SET(i, int, AG_VARIABLE_INT);
}
void
AG_InitInt(AG_Variable *V, int v)
{
	AG_InitVariable(V, AG_VARIABLE_INT);
	V->name[0] = '\0';
	V->data.i = v;
}
AG_Variable *
AG_BindInt(void *obj, const char *name, int *v)
{
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
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_INT);
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
	FN_VARIABLE_SET(u8, Uint8, AG_VARIABLE_UINT8);
}
void
AG_InitUint8(AG_Variable *V, Uint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT8);
	V->name[0] = '\0';
	V->data.u8 = v;
}
AG_Variable *
AG_BindUint8(void *obj, const char *name, Uint8 *v)
{
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
	FN_VARIABLE_SET(s8, Sint8, AG_VARIABLE_SINT8);
}
void
AG_InitSint8(AG_Variable *V, Sint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT8);
	V->name[0] = '\0';
	V->data.s8 = v;
}
AG_Variable *
AG_BindSint8(void *obj, const char *name, Sint8 *v)
{
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
	FN_VARIABLE_SET(u16, Uint16, AG_VARIABLE_UINT16);
}
void
AG_InitUint16(AG_Variable *V, Uint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT16);
	V->name[0] = '\0';
	V->data.u16 = v;
}
AG_Variable *
AG_BindUint16(void *obj, const char *name, Uint16 *v)
{
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
	FN_VARIABLE_SET(s16, Sint16, AG_VARIABLE_SINT16);
}
void
AG_InitSint16(AG_Variable *V, Sint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT16);
	V->name[0] = '\0';
	V->data.s16 = v;
}
AG_Variable *
AG_BindSint16(void *obj, const char *name, Sint16 *v)
{
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
	FN_VARIABLE_SET(u32, Uint32, AG_VARIABLE_UINT32);
}
void
AG_InitUint32(AG_Variable *V, Uint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT32);
	V->name[0] = '\0';
	V->data.u32 = v;
}
AG_Variable *
AG_BindUint32(void *obj, const char *name, Uint32 *v)
{
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
	FN_VARIABLE_SET(s32, Sint32, AG_VARIABLE_SINT32);
}
void
AG_InitSint32(AG_Variable *V, Sint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT32);
	V->name[0] = '\0';
	V->data.s32 = v;
}
AG_Variable *
AG_BindSint32(void *obj, const char *name, Sint32 *v)
{
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
	FN_VARIABLE_SET(u64, Uint64, AG_VARIABLE_UINT64);
}
void
AG_InitUint64(AG_Variable *V, Uint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT64);
	V->name[0] = '\0';
	V->data.u64 = v;
}
AG_Variable *
AG_BindUint64(void *obj, const char *name, Uint64 *v)
{
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
	FN_VARIABLE_SET(s64, Sint64, AG_VARIABLE_SINT64);
}
void
AG_InitSint64(AG_Variable *V, Sint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT64);
	V->name[0] = '\0';
	V->data.s64 = v;
}
AG_Variable *
AG_BindSint64(void *obj, const char *name, Sint64 *v)
{
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
	FN_VARIABLE_SET(flt, float, AG_VARIABLE_FLOAT);
}
void
AG_InitFloat(AG_Variable *V, float v)
{
	AG_InitVariable(V, AG_VARIABLE_FLOAT);
	V->name[0] = '\0';
	V->data.flt = v;
}
AG_Variable *
AG_BindFloat(void *obj, const char *name, float *v)
{
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
	FN_VARIABLE_SET(dbl, double, AG_VARIABLE_DOUBLE);
}
void
AG_InitDouble(AG_Variable *V, double v)
{
	AG_InitVariable(V, AG_VARIABLE_DOUBLE);
	V->name[0] = '\0';
	V->data.dbl = v;
}
AG_Variable *
AG_BindDouble(void *obj, const char *name, double *v)
{
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
	FN_VARIABLE_SET(ldbl, long double, AG_VARIABLE_LONG_DOUBLE);
}
void
AG_InitLongDouble(AG_Variable *V, long double v)
{
	AG_InitVariable(V, AG_VARIABLE_LONG_DOUBLE);
	V->name[0] = '\0';
	V->data.ldbl = v;
}
AG_Variable *
AG_BindLongDouble(void *obj, const char *name, long double *v)
{
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
	FN_VARIABLE_SET(p, void *, AG_VARIABLE_POINTER);
}
void
AG_InitPointer(AG_Variable *V, void *v)
{
	AG_InitVariable(V, AG_VARIABLE_POINTER);
	V->name[0] = '\0';
	V->data.p = v;
}
AG_Variable *
AG_BindPointer(void *obj, const char *name, void **v)
{
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
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_POINTER);
	return (V);
}

/*
 * AG_Text routines.
 */
static void
GetTextFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(p, fnText);
}
AG_Text *
AG_GetText(void *obj, const char *name)
{
	FN_VARIABLE_GET(p, fnText, void *, GetTextFn);
}
AG_Variable *
AG_SetText(void *obj, const char *name, AG_Text *v)
{
	FN_VARIABLE_SET(p, void *, AG_VARIABLE_P_TEXT);
}
void
AG_InitText(AG_Variable *V, AG_Text *v)
{
	AG_InitVariable(V, AG_VARIABLE_P_TEXT);
	V->name[0] = '\0';
	V->data.p = v;
}
AG_Variable *
AG_BindText(void *obj, const char *name, AG_Text *v)
{
	FN_VARIABLE_BIND(AG_VARIABLE_P_TEXT);
}
AG_Variable *
AG_BindTextFn(void *obj, const char *name, AG_TextFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnText, AG_VARIABLE_P_TEXT);
}
AG_Variable *
AG_BindTextMp(void *obj, const char *name, AG_Text *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_TEXT);
	return (V);
}


/*
 * Const pointer routines.
 */
static void
GetConstPointerFn(void *obj, AG_Variable *V)
{
	FN_VARIABLE_GETFN(Cp, fnConstPointer);
}
const void *
AG_GetConstPointer(void *obj, const char *name)
{
	FN_VARIABLE_GET(Cp, fnConstPointer, const void *, GetConstPointerFn);
}
AG_Variable *
AG_SetConstPointer(void *obj, const char *name, const void *v)
{
	FN_VARIABLE_SET(Cp, const void *, AG_VARIABLE_CONST_POINTER);
}
void
AG_InitConstPointer(AG_Variable *V, const void *v)
{
	AG_InitVariable(V, AG_VARIABLE_CONST_POINTER);
	V->name[0] = '\0';
	V->data.Cp = v;
}
AG_Variable *
AG_BindConstPointer(void *obj, const char *name, const void **v)
{
	FN_VARIABLE_BIND(AG_VARIABLE_P_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointerFn(void *obj, const char *name, AG_ConstPointerFn fn, const char *fmt, ...)
{
	FN_VARIABLE_BIND_FN(fnConstPointer, AG_VARIABLE_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointerMp(void *obj, const char *name, const void **v,
    AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_CONST_POINTER);
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

	AG_ObjectLock(obj);
	TAILQ_FOREACH(ev, &OBJECT(obj)->events, events) {
		if (strcmp(evName, ev->name) == 0)
			break;
	}
	rv = (V->fn.fnString != NULL) ?
	      V->fn.fnString(ev, dst, dstSize) : 0;
	AG_ObjectUnlock(obj);
	return (rv);
}
size_t
AG_GetString(void *pObj, const char *name, char *dst, size_t dstSize)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	size_t rv;

	if ((V = AG_GetVariableLocked(obj, name)) == NULL) {
		return (0);
	}
	if (V->fn.fnString != NULL) {
		rv = GetStringFn(obj, V, dst, dstSize);
	} else {
		Strlcpy(dst, V->data.s, dstSize);
		rv = strlen(V->data.s);
	}
	AG_UnlockVariable(V);
	return (rv);
}
char *
AG_GetStringDup(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	if ((V = AG_GetVariableLocked(obj, name)) == NULL) {
		return (NULL);
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
	return (s);
fail:
	AG_UnlockVariable(V);
	return (NULL);
}
/* Return a direct pointer to a string buffer (not free-threaded). */
char *
AG_GetStringP(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	if ((V = AG_GetVariableLocked(obj, name)) == NULL) {
		return (NULL);
	}
	s = V->data.s;
	AG_UnlockVariable(V);
	return (s);
}

/*
 * Set the value of a string variable. If the variable exists as a reference
 * to a fixed-size buffer, the string is copied to the buffer. Otherwise, the
 * string is duplicated.
 */
AG_Variable *
AG_SetString(void *pObj, const char *name, const char *s)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(AG_Variable));
		AG_InitVariable(V, AG_VARIABLE_STRING);
		Strlcpy(V->name, name, sizeof(V->name));
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
			Strlcpy(V->data.s, s, V->info.size);
			AG_UnlockVariable(V);
			break;
		default:
			AG_FreeVariable(V);
			AG_InitVariable(V, AG_VARIABLE_STRING);
			V->info.size = 0;			/* Allocated */
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
	AG_InitVariable(V, AG_VARIABLE_STRING);
	V->name[0] = '\0';
	V->data.s = Strdup(v);
	V->info.size = 0;
}
void
AG_InitStringNODUP(AG_Variable *V, char *v)
{
	AG_InitVariable(V, AG_VARIABLE_STRING);
	V->name[0] = '\0';
	V->data.s = v;
	V->info.size = 0;
}

/*
 * Variant of AG_SetString() where the string argument is taken to be
 * a dynamically-allocated string buffer which does not need to be
 * duplicated. The provided buffer will be freed automatically with
 * the parent object.
 */
AG_Variable *
AG_SetStringNODUP(void *obj, const char *name, char *s)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
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
		Strlcpy(V->data.s, s, V->info.size);
		AG_UnlockVariable(V);
		break;
	default:
		AG_FreeVariable(V);
		AG_InitVariable(V, AG_VARIABLE_STRING);
		V->data.s = s;
		V->info.size = 0;			/* Allocated */
		break;
	}
	AG_ObjectUnlock(obj);
	return (V);
}

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
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->data.s = buf;
	V->info.size = bufSize;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
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
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->mutex = mutex;
	V->data.s = v;
	V->info.size = size;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetConstString(void *obj, const char *name, const char *v)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_CONST_STRING);
	V->data.Cs = v;
	V->info.size = strlen(v)+1;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindConstString(void *obj, const char *name, const char **v)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.Cs = (const char *)v;
	V->info.size = strlen(*v)+1;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindConstStringMp(void *obj, const char *name, const char **v,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->mutex = mutex;
	V->data.Cs = (const char *)v;
	V->info.size = strlen(*v)+1;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
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
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlagMp(void *obj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8(void *obj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8Mp(void *obj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16(void *obj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16Mp(void *obj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32(void *obj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32Mp(void *obj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Create a Variable->Variable reference. */
AG_Variable *
AG_BindVariable(void *obj, const char *name, void *tgtObj, const char *tgtKey)
{
	AG_Variable *V;
	char *keyDup;

	if ((keyDup = TryStrdup(tgtKey)) == NULL)
		return (NULL);

	AG_ObjectLock(obj);
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_VARIABLE);
	V->data.p = tgtObj;
	V->info.ref.key = keyDup;
	V->info.ref.var = NULL;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
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
				if ((V = AG_GetVariableLocked(obj, key)) != NULL) {
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

/*
 * AG_ListSet(): Construct a list of AG_Variables from varargs arguments.
 */
#undef AG_VARIABLE_SETARG
#define AG_VARIABLE_SETARG(V,t,pt,dmemb,dtype,vtype,ival,fnmemb,fntype)	\
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
		} else {						\
			(V)->data.dmemb = (dtype)va_arg(ap, vtype);	\
		}							\
	} while (0)

#undef AG_VARIABLE_SETARG_STRING
#define AG_VARIABLE_SETARG_STRING(V,t,pt,dmemb,dtype,ival,fnmemb,fntype) \
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
			(V)->info.size = 0;				\
		} else {						\
			(V)->data.dmemb = va_arg(ap, dtype);		\
			(V)->info.size = strlen((V)->data.dmemb)+1;	\
		}							\
	} while (0)

#undef AG_VARIABLE_SETARG_STRING_BUFFER
#define AG_VARIABLE_SETARG_STRING_BUFFER(V,t,pt,dmemb,dtype,ival,fnmemb,fntype) \
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
			(V)->info.size = 0;				\
		} else {						\
			(V)->data.dmemb = va_arg(ap, dtype);		\
			(V)->info.size = va_arg(ap, size_t);		\
		}							\
	} while (0)

AG_List *
AG_ListSet(const char *fmt, ...)
{
	va_list ap;
	AG_List *L;
	int *argSizes = NULL, *argSizesNew;
	const char *fmtSpec;
	int nArgs = 0;

	if ((L = AG_ListNew()) == NULL) {
		return (NULL);
	}
	va_start(ap, fmt);
	for (fmtSpec = fmt; *fmtSpec != '\0'; ) {
		const char *c;
		int pFlag = 0, fnFlag = 0, lFlag = 0, LFlag = 0, isExtended = 0;
		int fmtChars, inFmt = 0;
		AG_Variable V;

		for (c = &fmtSpec[0], fmtChars = 0;
		     *c != '\0';
		     c++) {
			if (*c == '%') { inFmt = 1; }
			if (inFmt) { fmtChars++; }
			if (*c == '%') {
				continue;
			} else if (*c == '*' && c[1] != '\0') {
				pFlag++;
			} else if (*c == 'l' && c[1] != '\0') {
				lFlag++;
			} else if (*c == 'L' && c[1] != '\0') {
				LFlag++;
			} else if (*c == 'F' && c[1] != '\0') {
				fnFlag++;
			} else if (*c == '[' && c[1] != '\0') {
				isExtended++;
				break;
			} else if (inFmt && strchr("*Csdiufgp]", *c)) {
				break;
			} else if (strchr(".0123456789", *c)) {
				continue;
			} else {
				inFmt = 0;
			}
		}
		fmtSpec += fmtChars;
		if (*c == '\0') { break; }
		if (!inFmt) { continue;	}

		if ((argSizesNew = TryRealloc(argSizes, (nArgs+1)*sizeof(int)))
		    == NULL) {
			goto fail;
		}
		argSizes = argSizesNew;
		argSizes[nArgs++] = fmtChars;

		V.type = AG_VARIABLE_NULL;
		V.name[0] = '\0';
		V.mutex = NULL;
		V.fn.fnVoid = NULL;
		V.info.bitmask = 0;

		if (pFlag) {
			V.data.p = va_arg(ap, void *);
		}
		if (isExtended) {
			c++;
			if (c[0] == 's') {
				if (c[1] == '3' && c[2] == '2') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_SINT32, AG_VARIABLE_P_SINT32,
					    s32, Sint32, int, 0, fnSint32, AG_Sint32Fn);
#ifdef HAVE_64BIT
				} else if (c[1] == '6' && c[2] == '4') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_SINT64, AG_VARIABLE_P_SINT64,
					    s64, Sint64, long long, 0LL, fnSint64, AG_Sint64Fn);
#endif
				} else if (c[1] == '1' && c[2] == '6') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_SINT16, AG_VARIABLE_P_SINT16,
					    s16, Sint16, int, 0, fnSint16, AG_Sint16Fn);
				} else if (c[1] == '8') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_SINT8, AG_VARIABLE_P_SINT8,
					    s8, Sint8, int, 0, fnSint8, AG_Sint8Fn);
				}
			} else if (c[0] == 'u') {
				if (c[1] == '3' && c[2] == '2') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT32, AG_VARIABLE_P_UINT32,
					    u32, Uint32, Uint, 0, fnUint32, AG_Uint32Fn);
#ifdef HAVE_64BIT
				} else if (c[1] == '6' && c[2] == '4') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT64, AG_VARIABLE_P_UINT64,
					    u64, Uint64, unsigned long long, 0ULL, fnSint64, AG_Sint64Fn);
#endif
				} else if (c[1] == '1' && c[2] == '6') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT16, AG_VARIABLE_P_UINT16,
					    u16, Uint16, Uint, 0, fnUint16, AG_Uint16Fn);
				} else if (c[1] == '8') {
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT8, AG_VARIABLE_P_UINT8,
					    u8, Uint8, int, 0, fnUint8, AG_Uint8Fn);
				}
			} else if (c[0] == 'C') {
				switch (c[1]) {
				case 'p':
					AG_VARIABLE_SETARG(&V, AG_VARIABLE_CONST_POINTER, AG_VARIABLE_P_CONST_POINTER,
					    Cp, const void *, const void *, NULL, fnConstPointer, AG_ConstPointerFn);
					break;
				case 's':
					AG_VARIABLE_SETARG_STRING(&V, AG_VARIABLE_CONST_STRING, AG_VARIABLE_P_CONST_STRING,
					    Cs, const char *, NULL, fnString, AG_StringFn);
					break;
				}
			} else if (c[0] == 'B') {
				AG_VARIABLE_SETARG_STRING_BUFFER(&V, AG_VARIABLE_STRING,	AG_VARIABLE_P_STRING,
				    s, char *, NULL, fnString, AG_StringFn);
			}
			break;
		}
		switch (c[0]) {
		case 'p':
			AG_VARIABLE_SETARG(&V, AG_VARIABLE_POINTER, AG_VARIABLE_P_POINTER,
			    p, void *, void *, NULL, fnPointer, AG_PointerFn);
			break;
		case 's':
			AG_VARIABLE_SETARG_STRING(&V, AG_VARIABLE_STRING, AG_VARIABLE_P_STRING,
			    s, char *, NULL, fnString, AG_StringFn);
			break;
		case 't':
			V.type = AG_VARIABLE_P_TEXT;
			break;
		case 'd':
		case 'i':
			if (lFlag == 0) {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_INT, AG_VARIABLE_P_INT,
				    i, int, int, 0, fnInt, AG_IntFn);
			} else {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_SINT32, AG_VARIABLE_P_SINT32,
				    s32, Sint32, Sint32, 0, fnSint32, AG_Sint32Fn);
			}
			break;
		case 'u':
			if (lFlag == 0) {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT, AG_VARIABLE_P_UINT,
				    u, Uint, Uint, 0, fnUint, AG_UintFn);
			} else {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_UINT32, AG_VARIABLE_P_UINT32,
				    u32, Uint32, Uint32, 0, fnUint32, AG_Uint32Fn);
			}
			break;
		case 'f':
		case 'g':
			if (lFlag) {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_DOUBLE, AG_VARIABLE_P_DOUBLE,
				    dbl, double, double, 0.0, fnDouble, AG_DoubleFn);
#ifdef HAVE_LONG_DOUBLE
			} else if (LFlag) {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_LONG_DOUBLE, AG_VARIABLE_P_LONG_DOUBLE,
				    ldbl, long double, long double, 0.0L, fnLongDouble, AG_LongDoubleFn);
#endif
			} else {
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_FLOAT, AG_VARIABLE_P_FLOAT,
				    flt, float, double, 0.0f, fnFloat, AG_FloatFn);
			}
			break;
		}
		AG_ListAppend(L, &V);
	}
	va_end(ap);
	return (L);
fail:
	va_end(ap);
	AG_ListDestroy(L);
	free(argSizes);
	return (NULL);
}
