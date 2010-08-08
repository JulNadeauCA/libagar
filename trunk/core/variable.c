/*
 * Copyright (c) 2008-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "core.h"

const AG_VariableTypeInfo agVariableTypes[] = {
	{ AG_VARIABLE_NULL,		0,	"NULL",		AG_VARIABLE_NULL,		-1 },
	/*
	 * Primitive types
	 */
	{ AG_VARIABLE_UINT,		0,	"Uint",		AG_VARIABLE_UINT,		0 },
	{ AG_VARIABLE_P_UINT,		1,	"Uint *",	AG_VARIABLE_UINT,		0 },
	{ AG_VARIABLE_INT,		0,	"int",		AG_VARIABLE_INT,		1 },
	{ AG_VARIABLE_P_INT,		1,	"int *",	AG_VARIABLE_INT,		1 },
	{ AG_VARIABLE_UINT8,		0,	"Uint8",	AG_VARIABLE_UINT8,		2 },
	{ AG_VARIABLE_P_UINT8,		1,	"Uint8 *",	AG_VARIABLE_UINT8,		2 },
	{ AG_VARIABLE_SINT8,		0,	"Sint8",	AG_VARIABLE_SINT8,		3 },
	{ AG_VARIABLE_P_SINT8,		1,	"Sint8 *",	AG_VARIABLE_SINT8,		3 },
	{ AG_VARIABLE_UINT16,		0,	"Uint16",	AG_VARIABLE_UINT16,		4 },
	{ AG_VARIABLE_P_UINT16,		1,	"Uint16 *",	AG_VARIABLE_UINT16,		4 },
	{ AG_VARIABLE_SINT16,		0,	"Sint16",	AG_VARIABLE_SINT16,		5 },
	{ AG_VARIABLE_P_SINT16,		1,	"Sint16 *",	AG_VARIABLE_SINT16,		5 },
	{ AG_VARIABLE_UINT32,		0,	"Uint32",	AG_VARIABLE_UINT32,		6 },
	{ AG_VARIABLE_P_UINT32,		1,	"Uint32 *" ,	AG_VARIABLE_UINT32,		6 },
	{ AG_VARIABLE_SINT32,		0,	"Sint32",	AG_VARIABLE_SINT32,		7 },
	{ AG_VARIABLE_P_SINT32,		1,	"Sint32 *",	AG_VARIABLE_SINT32,		7 },
	{ AG_VARIABLE_UINT64,		0,	"Uint64",	AG_VARIABLE_UINT64,		8 },
	{ AG_VARIABLE_P_UINT64,		1,	"Uint64 *",	AG_VARIABLE_UINT64,		8 },
	{ AG_VARIABLE_SINT64,		0,	"Sint64",	AG_VARIABLE_SINT64,		9 },
	{ AG_VARIABLE_P_SINT64,		1,	"Sint64 *",	AG_VARIABLE_SINT64,		9 },
	{ AG_VARIABLE_FLOAT,		0,	"float",	AG_VARIABLE_FLOAT,		10 },
	{ AG_VARIABLE_P_FLOAT,		1,	"float *",	AG_VARIABLE_FLOAT,		10 },
	{ AG_VARIABLE_DOUBLE,		0,	"double",	AG_VARIABLE_DOUBLE,		11 },
	{ AG_VARIABLE_P_DOUBLE,		1,	"double *",	AG_VARIABLE_DOUBLE,		11 },
	{ AG_VARIABLE_LONG_DOUBLE,	0,	"long double",	AG_VARIABLE_LONG_DOUBLE,	12 },
	{ AG_VARIABLE_P_LONG_DOUBLE,	1,	"long double *",AG_VARIABLE_LONG_DOUBLE,	12 },
	{ AG_VARIABLE_STRING,		0,	"Str",		AG_VARIABLE_STRING,		13 },
	{ AG_VARIABLE_P_STRING,		1,	"Str *",	AG_VARIABLE_STRING,		13 },
	{ AG_VARIABLE_CONST_STRING,	0,	"Const Str",	AG_VARIABLE_CONST_STRING, 	13 },
	{ AG_VARIABLE_P_CONST_STRING,	1,	"Const Str *",	AG_VARIABLE_CONST_STRING, 	13 },
	{ AG_VARIABLE_POINTER,		0,	"Ptr",		AG_VARIABLE_POINTER,		-1 },
	{ AG_VARIABLE_P_POINTER,	1,	"Ptr *",	AG_VARIABLE_POINTER,		-1 },
	{ AG_VARIABLE_CONST_POINTER,	0,	"Const Ptr",	AG_VARIABLE_CONST_POINTER,	-1 },
	{ AG_VARIABLE_P_CONST_POINTER,	1,	"Const Ptr *",	AG_VARIABLE_CONST_POINTER,	-1 },
	/*
	 * Bitmask-specific types
	 */
	{ AG_VARIABLE_P_FLAG,		1,	"Flag *",	AG_VARIABLE_P_FLAG,		-1 },
	{ AG_VARIABLE_P_FLAG8,		1,	"Flag8 *",	AG_VARIABLE_P_FLAG8,		-1 },
	{ AG_VARIABLE_P_FLAG16,		1,	"Flag16 *",	AG_VARIABLE_P_FLAG16,		-1 },
	{ AG_VARIABLE_P_FLAG32,		1,	"Flag32 *",	AG_VARIABLE_P_FLAG32,		-1 },
	/*
	 * Agar-Core specific types
	 */
	{ AG_VARIABLE_P_OBJECT,		1,	"Object *",	AG_VARIABLE_P_OBJECT,		-1 },
#if 0
	/*
	 * Agar-Math specific types
	 */
	{ AG_VARIABLE_P_REAL,		1,	"Real *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_RANGE,		1,	"Range *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_COMPLEX,	1,	"Complex *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_QUAT,		1,	"Quat *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_RECTANGULAR,	1,	"Rectangular *",AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_POLAR,		1,	"Polar *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_PARABOLIC,	1,	"Parabolic *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_SPHERICAL,	1,	"Spherical *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_CYLINDRICAL,	1,	"Cylindrical *",AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_COLOR,		1,	"Color *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_VECTOR,		1,	"Vector *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_VECTOR2,	1,	"Vector2 *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_VECTOR3,	1,	"Vector3 *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_VECTOR4,	1,	"Vector4 *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_MATRIX,		1,	"Matrix *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_MATRIX22,	1,	"Matrix22 *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_MATRIX33,	1,	"Matrix33 *",	AG_VARIABLE_NULL,		-1 },
	{ AG_VARIABLE_P_MATRIX44,	1,	"Matrix44 *",	AG_VARIABLE_NULL,		-1 },
#endif
};

/*
 * Create a new variable or return one for modification by name.
 * Object must be locked.
 */
static __inline__ AG_Variable *
FetchVariable(void *pObj, const char *name, enum ag_variable_type type)
{
	AG_Object *obj = pObj;
	Uint i;
	AG_Variable *V = NULL;
	
	for (i = 0; i < obj->nVars; i++) {
		V = &obj->vars[i];
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (i == obj->nVars) {
		obj->vars = Realloc(obj->vars, (obj->nVars+1) *
		                               sizeof(AG_Variable));
		V = &obj->vars[obj->nVars++];
		Strlcpy(V->name, name, sizeof(V->name));
		V->type = type;
	}
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	return (V);
}

/* Variant of FetchVariable() returning "new" flag. */
static __inline__ AG_Variable *
FetchVariableNew(void *pObj, const char *name, int *newFlag)
{
	AG_Object *obj = pObj;
	Uint i;
	AG_Variable *V;
	
	for (i = 0; i < obj->nVars; i++) {
		V = &obj->vars[i];
		if (strcmp(V->name, name) == 0) {
			*newFlag = 0;
			return (V);
		}
	}
	obj->vars = Realloc(obj->vars, (obj->nVars+1)*sizeof(AG_Variable));
	V = &obj->vars[obj->nVars++];
	Strlcpy(V->name, name, sizeof(V->name));
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	*newFlag = 1;
	return (V);
}

/* Duplicate a Variable contents. */
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
	case AG_VARIABLE_FLOAT:		V->data.flt = V->fn.fnFloat(ev);	break;
	case AG_VARIABLE_DOUBLE:	V->data.dbl = V->fn.fnDouble(ev);	break;
	case AG_VARIABLE_POINTER:	V->data.p = V->fn.fnPointer(ev);	break;
	case AG_VARIABLE_CONST_POINTER:	V->data.Cp = V->fn.fnConstPointer(ev);	break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_CONST_STRING:
		V->fn.fnString(ev, V->data.s, V->info.size);
		break;
	default:									break;
		break;
	}
	return (0);
}

/*
 * Print the specified variable to fixed-size buffer. The Variable must be
 * locked, and must have been previously evaluated if associated with a
 * function.
 */
void
AG_PrintVariable(char *s, size_t len, AG_Variable *V)
{
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
	case AG_VARIABLE_FLOAT:		Snprintf(s, len, "%.2f", V->data.flt);			break;
	case AG_VARIABLE_P_FLOAT:	Snprintf(s, len, "%.2f", *(float *)V->data.p);		break;
	case AG_VARIABLE_DOUBLE:	Snprintf(s, len, "%.2f", V->data.dbl);			break;
	case AG_VARIABLE_P_DOUBLE:	Snprintf(s, len, "%.2f", *(double *)V->data.p);		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_CONST_STRING:
		Strlcpy(s, V->data.s, len);
		break;
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		Strlcpy(s, (char *)V->data.p, len);
		break;
	case AG_VARIABLE_POINTER:
	case AG_VARIABLE_CONST_POINTER:
		Snprintf(s, len, "%p", V->data.p);
		break;
	case AG_VARIABLE_P_POINTER:
	case AG_VARIABLE_P_CONST_POINTER:
		Snprintf(s, len, "%p", *(void **)V->data.p);
		break;
	default:
		s[0] = '?';
		s[1] = '\0';
		break;
	}
}

/*
 * Lookup a variable by name and return a generic pointer to its current value
 * after having evaluated the associated function if there is one. Variable is
 * returned locked. Return NULL if the variable does not exist.
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

/*
 * Set the value of the specified Object variable. If there is no variable
 * of the given name, it is created. The format string specifies the type
 * in standard argument format, followed by the relevant parameters.
 */
AG_Variable *
AG_Set(void *pObj, const char *name, const char *fmt, ...)
{
	AG_Object *obj = pObj;
	AG_List *vList = AG_ListNew();
	int *argSizes = NULL;
	AG_Variable *V;
	va_list ap;

	va_start(ap, fmt);
	AG_PARSE_VARIABLE_ARGS(ap, fmt, vList, argSizes);
	va_end(ap);
#ifdef DEBUG
	if (vList->n != 1)
		AG_FatalError("Invalid AG_Set() format");
#endif
	
	AG_ObjectLock(obj);
	V = AG_GetVariableLocked(obj, name);
	Strlcpy(V->name, name, sizeof(V->name));
	if (AG_CopyVariable(V, &vList->v[0]) == -1) {
		AG_FatalError(NULL);
	}
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);

	AG_ListDestroy(vList);
	Free(argSizes);
	return (V);
}

/* Unset a variable */
void
AG_Unset(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	Uint i;

	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(obj->vars[i].name, name) != 0) {
			continue;
		}
		if (i < obj->nVars-1) {
			memmove(&obj->vars[i], &obj->vars[i+1],
			    (obj->nVars-1)*sizeof(AG_Variable));
		}
		obj->nVars--;
		break;
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
	V = FetchVariable(obj, name, ntype);			\
	if (agVariableTypes[V->type].indirLvl > 0) {		\
		*(_type *)V->data.p = v;			\
	} else {						\
		V->data._memb = v;				\
	}							\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_InitFoo() routines. */
#undef  FN_VARIABLE_INIT
#define FN_VARIABLE_INIT(_type,ntype)				\
	V->name[0] = '\0';					\
	V->type = ntype;					\
	V->mutex = NULL;					\
	V->info.size = 0;					\
	V->fn.fnVoid = NULL

/* Body of AG_BindFoo() routines. */
#undef  FN_VARIABLE_BIND
#define FN_VARIABLE_BIND(t)					\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, (t));			\
	V->type = t;						\
	V->data.p = (void *)v;					\
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_BindFooFn() routines. */
#undef  FN_VARIABLE_BIND_FN
#define FN_VARIABLE_BIND_FN(_memb,t)				\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
	AG_Variable *V;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, name, sizeof(evName));			\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, (t));			\
	V->type = (t);						\
	V->fn._memb = fn;					\
	ev = AG_SetEvent(obj, evName, NULL, NULL);		\
	AG_EVENT_GET_ARGS(ev, fmt);				\
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

/* Body of AG_BindFooMp() routines. */
#undef  FN_VARIABLE_BIND_MP
#define FN_VARIABLE_BIND_MP(t)					\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, (t));			\
	V->type = (t);						\
	V->data.p = (void *)v;					\
	V->mutex = mutex;					\
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
	FN_VARIABLE_INIT(Uint, AG_VARIABLE_UINT);
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
	FN_VARIABLE_INIT(int, AG_VARIABLE_INT);
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
	FN_VARIABLE_INIT(Uint8, AG_VARIABLE_UINT8);
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
	FN_VARIABLE_INIT(Sint8, AG_VARIABLE_SINT8);
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
	FN_VARIABLE_INIT(Uint16, AG_VARIABLE_UINT16);
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
	FN_VARIABLE_INIT(Sint16, AG_VARIABLE_SINT16);
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
	FN_VARIABLE_INIT(Uint32, AG_VARIABLE_UINT32);
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
	FN_VARIABLE_INIT(Sint32, AG_VARIABLE_SINT32);
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
	FN_VARIABLE_INIT(float, AG_VARIABLE_FLOAT);
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
	FN_VARIABLE_INIT(double, AG_VARIABLE_DOUBLE);
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
	FN_VARIABLE_INIT(void *, AG_VARIABLE_POINTER);
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
	FN_VARIABLE_INIT(const void *, AG_VARIABLE_CONST_POINTER);
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
		s = Malloc(V->info.size);
		(void)GetStringFn(obj, V, s, V->info.size);
	} else {
		s = Strdup(V->data.s);
	}
	AG_UnlockVariable(V);
	return (s);
}
AG_Variable *
AG_SetString(void *obj, const char *name, const char *s)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, &new);
	if (new) {
		V->type = AG_VARIABLE_STRING;
		V->info.size = 0;				/* Allocated */
		V->data.s = Strdup(s);
	} else {
		AG_LockVariable(V);
		if (V->type == AG_VARIABLE_P_STRING) {
			Strlcpy(*(char **)V->data.p, s, V->info.size);
		} else {
			if (V->info.size == 0) { Free(V->data.s); }
			V->data.s = Strdup(s);
		}
		AG_UnlockVariable(V);
	}
	AG_ObjectUnlock(obj);
	return (V);
}
void
AG_InitString(AG_Variable *V, const char *v)
{
	FN_VARIABLE_INIT(char *, AG_VARIABLE_STRING);
	V->data.s = Strdup(v);
	V->info.size = 0;
}
void
AG_InitStringNODUP(AG_Variable *V, char *v)
{
	FN_VARIABLE_INIT(char *, AG_VARIABLE_STRING);
	V->data.s = v;
	V->info.size = 0;
}
AG_Variable *
AG_SetStringNODUP(void *obj, const char *name, char *s)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, &new);
	if (new) {
		V->type = AG_VARIABLE_STRING;
		V->info.size = 0;
		V->data.s = s;
	} else {
		AG_LockVariable(V);
		if (V->type == AG_VARIABLE_P_STRING) {
			/* NODUP is meaningless is this case */
			Strlcpy(V->data.s, s, V->info.size);
		} else {
			if (V->info.size == 0) { Free(V->data.s); }
			V->data.s = s;
		}
		AG_UnlockVariable(V);
	}
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetStringFixed(void *obj, const char *name, char *buf, size_t bufSize)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	V->type = AG_VARIABLE_STRING;
	V->data.s = buf;
	V->info.size = bufSize;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	AG_ObjectUnlock(obj);
	return (V);
}
void
AG_InitStringFixed(AG_Variable *V, char *v, size_t bufSize)
{
	FN_VARIABLE_INIT(char *, AG_VARIABLE_STRING);
	V->data.s = v;
	V->info.size = bufSize;
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
AG_BindString(void *obj, const char *name, char *v, size_t size)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, &new);
	V->type = AG_VARIABLE_P_STRING;
	if (!new) {
		if (V->info.size == 0) {
			Free(V->data.s);
		}
		V->mutex = NULL;
		V->fn.fnVoid = NULL;
	}
	V->data.p = v;
	V->info.size = size;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_STRING);
	V->type = AG_VARIABLE_P_STRING;
	V->data.p = v;
	V->info.size = size;
	V->mutex = mutex;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetConstString(void *obj, const char *name, const char *v)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_CONST_STRING);
	V->type = AG_VARIABLE_CONST_STRING;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->type = AG_VARIABLE_P_CONST_STRING;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->type = AG_VARIABLE_P_CONST_STRING;
	V->data.Cs = (const char *)v;
	V->info.size = strlen(*v)+1;
	V->mutex = mutex;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG);
	V->type = AG_VARIABLE_P_FLAG;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG);
	V->type = AG_VARIABLE_P_FLAG;
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8(void *obj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG8);
	V->type = AG_VARIABLE_P_FLAG8;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG8);
	V->type = AG_VARIABLE_P_FLAG8;
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16(void *obj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG16);
	V->type = AG_VARIABLE_P_FLAG16;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG16);
	V->type = AG_VARIABLE_P_FLAG16;
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32(void *obj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->type = AG_VARIABLE_P_FLAG32;
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
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->type = AG_VARIABLE_P_FLAG32;
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
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
