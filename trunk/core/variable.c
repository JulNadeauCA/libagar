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
	{ AG_VARIABLE_P_REAL,		1,	"Real *",	AG_VARIABLE_NULL,		20 },
	{ AG_VARIABLE_P_RANGE,		1,	"Range *",	AG_VARIABLE_NULL,		21 },
	{ AG_VARIABLE_P_COMPLEX,	1,	"Complex *",	AG_VARIABLE_NULL,		22 },
	{ AG_VARIABLE_P_QUAT,		1,	"Quat *",	AG_VARIABLE_NULL,		23 },
	{ AG_VARIABLE_P_RECTANGULAR,	1,	"Rectangular *",AG_VARIABLE_NULL,		24 },
	{ AG_VARIABLE_P_POLAR,		1,	"Polar *",	AG_VARIABLE_NULL,		25 },
	{ AG_VARIABLE_P_PARABOLIC,	1,	"Parabolic *",	AG_VARIABLE_NULL,		26 },
	{ AG_VARIABLE_P_SPHERICAL,	1,	"Spherical *",	AG_VARIABLE_NULL,		27 },
	{ AG_VARIABLE_P_CYLINDRICAL,	1,	"Cylindrical *",AG_VARIABLE_NULL,		28 },
	{ AG_VARIABLE_P_COLOR,		1,	"Color *",	AG_VARIABLE_NULL,		29 },
	{ AG_VARIABLE_P_VECTOR,		1,	"Vector *",	AG_VARIABLE_NULL,		30 },
	{ AG_VARIABLE_P_VECTOR2,	1,	"Vector2 *",	AG_VARIABLE_NULL,		31 },
	{ AG_VARIABLE_P_VECTOR3,	1,	"Vector3 *",	AG_VARIABLE_NULL,		32 },
	{ AG_VARIABLE_P_VECTOR4,	1,	"Vector4 *",	AG_VARIABLE_NULL,		33 },
	{ AG_VARIABLE_P_MATRIX,		1,	"Matrix *",	AG_VARIABLE_NULL,		34 },
	{ AG_VARIABLE_P_MATRIX22,	1,	"Matrix22 *",	AG_VARIABLE_NULL,		35 },
	{ AG_VARIABLE_P_MATRIX33,	1,	"Matrix33 *",	AG_VARIABLE_NULL,		36 },
	{ AG_VARIABLE_P_MATRIX44,	1,	"Matrix44 *",	AG_VARIABLE_NULL,		37 },
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
	AG_Variable *V;
	
	for (i = 0; i < obj->nVars; i++) {
		V = &obj->vars[i];
		if (strcmp(V->name, name) == 0) {
			V->type = type;
			V->mutex = NULL;
			V->fn.fnVoid = NULL;
			return (V);
		}
	}
	obj->vars = Realloc(obj->vars, (obj->nVars+1)*sizeof(AG_Variable));
	V = &obj->vars[obj->nVars++];
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	Strlcpy(V->name, name, sizeof(V->name));
	return (V);
}

/* Variant of FetchVariable() returning "new" flag. */
static __inline__ AG_Variable *
FetchVariableNew(void *pObj, const char *name, enum ag_variable_type type,
    int *newFlag)
{
	AG_Object *obj = pObj;
	Uint i;
	AG_Variable *V;
	
	for (i = 0; i < obj->nVars; i++) {
		V = &obj->vars[i];
		if (strcmp(V->name, name) == 0) {
			V->type = type;
			V->mutex = NULL;
			V->fn.fnVoid = NULL;
			*newFlag = 0;
			return (V);
		}
	}
	obj->vars = Realloc(obj->vars, (obj->nVars+1)*sizeof(AG_Variable));
	V = &obj->vars[obj->nVars++];
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	Strlcpy(V->name, name, sizeof(V->name));
	*newFlag = 1;
	return (V);
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
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_UINT:	Snprintf(s, len, "%u", V->data.u);		break;
	case AG_VARIABLE_INT:	Snprintf(s, len, "%d", V->data.i);		break;
	case AG_VARIABLE_UINT8:	Snprintf(s, len, "%u", (Uint)V->data.u8);	break;
	case AG_VARIABLE_SINT8:	Snprintf(s, len, "%d", (int)V->data.s8);	break;
	case AG_VARIABLE_UINT16:Snprintf(s, len, "%u", (Uint)V->data.u16);	break;
	case AG_VARIABLE_SINT16:Snprintf(s, len, "%d", (int)V->data.s16);	break;
	case AG_VARIABLE_UINT32:Snprintf(s, len, "%lu", (Ulong)V->data.u32);	break;
	case AG_VARIABLE_SINT32:Snprintf(s, len, "%ld", (long)V->data.s32);	break;
	case AG_VARIABLE_FLOAT:	Snprintf(s, len, "%f", V->data.flt);		break;
	case AG_VARIABLE_DOUBLE:Snprintf(s, len, "%f", V->data.dbl);		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_CONST_STRING:
		Snprintf(s, len, "\"%s\"", V->data.s);
		break;
	case AG_VARIABLE_POINTER:
	case AG_VARIABLE_CONST_POINTER:
		Snprintf(s, len, "%p", V->data.p);
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
	Uint i;
	va_list ap;
	
	va_start(ap, name);
	p = va_arg(ap, void **);
	va_end(ap);

	AG_ObjectLock(obj);
	V = AG_GetVariableLocked(obj, name);
	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(obj->vars[i].name, name) == 0)
			break;
	}
	if (i == obj->nVars) {
		goto fail;
	}
	V = &obj->vars[i];

	AG_LockVariable(V);
	if (V->fn.fnVoid != NULL) {
		(void)AG_EvalVariable(obj, V);
	}
	*p = (agVariableTypes[V->type].indirLvl > 0) ? V->data.p : &V->data;
	AG_ObjectUnlock(obj);
	return (V);
fail:
	AG_ObjectUnlock(obj);
	return (NULL);
}

/*
 * Search for a variable from an a "object-name:prop-name" string,
 * relative to the specified VFS.
 */
AG_Variable *
AG_GetVariableVFS(void *vfsRoot, const char *varPath)
{
	char sb[AG_OBJECT_PATH_MAX+65];
	char *s = &sb[0], *objName, *varName;
	void *obj;

	Strlcpy(sb, varPath, sizeof(sb));
	objName = Strsep(&s, ":");
	varName = Strsep(&s, ":");
	if (objName == NULL || varName == NULL ||
	    objName[0] == '\0' || varName[0] == '\0') {
		AG_SetError(_("Invalid variable path: %s"), varPath);
		return (NULL);
	}
	if ((obj = AG_ObjectFind(vfsRoot, objName)) == NULL) {
		return (NULL);
	}
	return AG_GetVariableLocked(obj, varName);
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
	AG_Variable *V;
	va_list ap;

	AG_ObjectLock(obj);
	V = AG_GetVariableLocked(obj, name);
	va_start(ap, fmt);
	AG_VARIABLE_GET(ap, fmt, V);
	va_end(ap);
	Strlcpy(V->name, name, sizeof(V->name));
	AG_ObjectUnlock(obj);
	return (V);
}

/*
 * Atomic get/set and bind routines.
 */

#undef  VARIABLE_GET
#define VARIABLE_GET(obj,name,_memb,_fn,_type,_getfn)		\
	_type rv;						\
	AG_Variable *V;						\
								\
	if ((V = AG_GetVariableLocked((obj),(name))) == NULL) {	\
		Debug((obj), "No such variable: %s\n", (name));	\
		return (0);					\
	}							\
	if (V->fn._fn != NULL) {				\
		_getfn((obj), V);				\
	} else if (agVariableTypes[V->type].indirLvl > 0) {	\
		rv = *(_type *)V->data.p;			\
	} else {						\
		rv = V->data._memb;				\
	}							\
	AG_UnlockVariable(V);					\
	return (rv)						\

#undef  VARIABLE_SET
#define VARIABLE_SET(obj,name,_memb,_type,ntype)		\
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

#undef  VARIABLE_BIND
#define VARIABLE_BIND(obj,name,_memb,type)			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, type);			\
	V->data._memb = v;					\
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

#undef  VARIABLE_BIND_FN
#define VARIABLE_BIND_FN(obj,name,_memb,type)			\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
	AG_Variable *V;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, name, sizeof(evName));			\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, type);			\
	V->fn._memb = fn;					\
	ev = AG_SetEvent(obj, evName, NULL, NULL);		\
	AG_EVENT_GET_ARGS(ev, fmt);				\
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

#undef  VARIABLE_BIND_MP
#define VARIABLE_BIND_MP(obj,name,_memb,type)			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = FetchVariable(obj, name, type);			\
	V->data._memb = v;					\
	V->mutex = mutex;					\
	AG_PostEvent(NULL, obj, "bound", "%p", V);		\
	AG_ObjectUnlock(obj);					\
	return (V)

/*
 * Unsigned integer
 */
static void
GetUintFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, u, fnUint);
}
Uint
AG_GetUint(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u, fnUint, Uint, GetUintFn);
}
AG_Variable *
AG_SetUint(void *obj, const char *name, Uint v)
{
	VARIABLE_SET(obj, name, u, Uint, AG_VARIABLE_UINT);
}
AG_Variable *
AG_BindUint(void *obj, const char *name, Uint *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_UINT);
}
AG_Variable *
AG_BindUintFn(void *obj, const char *name, AG_UintFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnUint, AG_VARIABLE_UINT);
}
AG_Variable *
AG_BindUint_MP(void *obj, const char *name, Uint *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_UINT);
}

/*
 * Signed integer
 */
static void
GetIntFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, i, fnInt);
}
int
AG_GetInt(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, i, fnInt, int, GetIntFn);
}
AG_Variable *
AG_SetInt(void *obj, const char *name, int v)
{
	VARIABLE_SET(obj, name, i, int, AG_VARIABLE_INT);
}
AG_Variable *
AG_BindInt(void *obj, const char *name, int *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_INT);
}
AG_Variable *
AG_BindIntFn(void *obj, const char *name, AG_IntFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnInt, AG_VARIABLE_INT);
}
AG_Variable *
AG_BindInt_MP(void *obj, const char *name, int *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_INT);
}

/*
 * Unsigned 8-bit integer
 */
static void
GetUint8Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, u8, fnUint8);
}
Uint8
AG_GetUint8(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u8, fnUint8, Uint8, GetUint8Fn);
}
AG_Variable *
AG_SetUint8(void *obj, const char *name, Uint8 v)
{
	VARIABLE_SET(obj, name, s8, Uint8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindUint8(void *obj, const char *name, Uint8 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_SINT8);
}
AG_Variable *
AG_BindUint8Fn(void *obj, const char *name, AG_Uint8Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnUint8, AG_VARIABLE_UINT8);
}
AG_Variable *
AG_BindUint8_MP(void *obj, const char *name, Uint8 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_SINT8);
}

/*
 * Signed 8-bit integer
 */
static void
GetSint8Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, s8, fnSint8);
}
Sint8
AG_GetSint8(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s8, fnSint8, Sint8, GetSint8Fn);
}
AG_Variable *
AG_SetSint8(void *obj, const char *name, Sint8 v)
{
	VARIABLE_SET(obj, name, s8, Sint8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindSint8(void *obj, const char *name, Sint8 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_SINT8);
}
AG_Variable *
AG_BindSint8Fn(void *obj, const char *name, AG_Sint8Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnSint8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindSint8_MP(void *obj, const char *name, Sint8 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_SINT8);
}

/*
 * Unsigned 16-bit integer
 */
static void
GetUint16Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, u16, fnUint16);
}
Uint16
AG_GetUint16(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u16, fnUint16, Uint16, GetUint16Fn);
}
AG_Variable *
AG_SetUint16(void *obj, const char *name, Uint16 v)
{
	VARIABLE_SET(obj, name, u16, Uint16, AG_VARIABLE_UINT16);
}
AG_Variable *
AG_BindUint16(void *obj, const char *name, Uint16 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_UINT16);
}
AG_Variable *
AG_BindUint16Fn(void *obj, const char *name, AG_Uint16Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnUint16, AG_VARIABLE_UINT16);
}
AG_Variable *
AG_BindUint16_MP(void *obj, const char *name, Uint16 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_UINT16);
}

/*
 * Signed 16-bit integer
 */
static void
GetSint16Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, s16, fnSint16);
}
Sint16
AG_GetSint16(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s16, fnSint16, Sint16, GetSint16Fn);
}
AG_Variable *
AG_SetSint16(void *obj, const char *name, Sint16 v)
{
	VARIABLE_SET(obj, name, s16, Sint16, AG_VARIABLE_SINT16);
}
AG_Variable *
AG_BindSint16(void *obj, const char *name, Sint16 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_SINT16);
}
AG_Variable *
AG_BindSint16Fn(void *obj, const char *name, AG_Sint16Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnSint16, AG_VARIABLE_SINT16);
}
AG_Variable *
AG_BindSint16_MP(void *obj, const char *name, Sint16 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_SINT16);
}

/*
 * Unsigned 32-bit integer
 */
static void
GetUint32Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, u32, fnUint32);
}
Uint32
AG_GetUint32(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u32, fnUint32, Uint32, GetUint32Fn);
}
AG_Variable *
AG_SetUint32(void *obj, const char *name, Uint32 v)
{
	VARIABLE_SET(obj, name, u32, Uint32, AG_VARIABLE_UINT32);
}
AG_Variable *
AG_BindUint32(void *obj, const char *name, Uint32 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_UINT32);
}
AG_Variable *
AG_BindUint32Fn(void *obj, const char *name, AG_Uint32Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnUint32, AG_VARIABLE_UINT32);
}
AG_Variable *
AG_BindUint32_MP(void *obj, const char *name, Uint32 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_UINT32);
}

/*
 * Signed 32-bit integer
 */
static void
GetSint32Fn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, s32, fnSint32);
}
Sint32
AG_GetSint32(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s32, fnSint32, Sint32, GetSint32Fn);
}
AG_Variable *
AG_SetSint32(void *obj, const char *name, Sint32 v)
{
	VARIABLE_SET(obj, name, s32, Sint32, AG_VARIABLE_SINT32);
}
AG_Variable *
AG_BindSint32(void *obj, const char *name, Sint32 *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_SINT32);
}
AG_Variable *
AG_BindSint32Fn(void *obj, const char *name, AG_Sint32Fn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnSint32, AG_VARIABLE_SINT32);
}
AG_Variable *
AG_BindSint32_MP(void *obj, const char *name, Sint32 *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_SINT32);
}

/*
 * Single-precision floating-point number.
 */
static void
GetFloatFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, flt, fnFloat);
}
float
AG_GetFloat(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, flt, fnFloat, float, GetFloatFn);
}
AG_Variable *
AG_SetFloat(void *obj, const char *name, float v)
{
	VARIABLE_SET(obj, name, flt, float, AG_VARIABLE_FLOAT);
}
AG_Variable *
AG_BindFloat(void *obj, const char *name, float *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_FLOAT);
}
AG_Variable *
AG_BindFloatFn(void *obj, const char *name, AG_FloatFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnFloat, AG_VARIABLE_FLOAT);
}
AG_Variable *
AG_BindFloat_MP(void *obj, const char *name, float *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_FLOAT);
}

/*
 * Double-precision floating-point number.
 */
static void
GetDoubleFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, dbl, fnDouble);
}
double
AG_GetDouble(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, dbl, fnDouble, double, GetDoubleFn);
}
AG_Variable *
AG_SetDouble(void *obj, const char *name, double v)
{
	VARIABLE_SET(obj, name, dbl, double, AG_VARIABLE_DOUBLE);
}
AG_Variable *
AG_BindDouble(void *obj, const char *name, double *v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_DOUBLE);
}
AG_Variable *
AG_BindDoubleFn(void *obj, const char *name, AG_DoubleFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnDouble, AG_VARIABLE_DOUBLE);
}
AG_Variable *
AG_BindDouble_MP(void *obj, const char *name, double *v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_DOUBLE);
}

/*
 * Pointer routines.
 */
static void
GetPointerFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, p, fnPointer);
}
void *
AG_GetPointer(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, p, fnPointer, void *, GetPointerFn);
}
AG_Variable *
AG_SetPointer(void *obj, const char *name, void *v)
{
	VARIABLE_SET(obj, name, p, void *, AG_VARIABLE_POINTER);
}
AG_Variable *
AG_BindPointer(void *obj, const char *name, void **v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_POINTER);
}
AG_Variable *
AG_BindPointerFn(void *obj, const char *name, AG_PointerFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnPointer, AG_VARIABLE_POINTER);
}
AG_Variable *
AG_BindPointer_MP(void *obj, const char *name, void **v, AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_POINTER);
	return (V);
}

/*
 * Const pointer routines.
 */
static void
GetConstPointerFn(void *obj, AG_Variable *V)
{
	AG_VARIABLE_GET_FN(obj, V, Cp, fnConstPointer);
}
const void *
AG_GetConstPointer(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, Cp, fnConstPointer, const void *, GetConstPointerFn);
}
AG_Variable *
AG_SetConstPointer(void *obj, const char *name, const void *v)
{
	VARIABLE_SET(obj, name, Cp, const void *, AG_VARIABLE_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointer(void *obj, const char *name, const void **v)
{
	VARIABLE_BIND(obj, name, p, AG_VARIABLE_P_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointerFn(void *obj, const char *name, AG_ConstPointerFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnConstPointer, AG_VARIABLE_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointer_MP(void *obj, const char *name, const void **v,
    AG_Mutex *mutex)
{
	VARIABLE_BIND_MP(obj, name, p, AG_VARIABLE_P_CONST_POINTER);
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
		return (0);
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
	V = FetchVariableNew(obj, name, AG_VARIABLE_STRING, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	V->data.s = Strdup(s);
	V->info.size = 0;				/* Allocated */
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetStringNODUP(void *obj, const char *name, char *s)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, AG_VARIABLE_STRING, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	V->data.s = s;
	V->info.size = 0;				/* Allocated */
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetStringFixed(void *obj, const char *name, char *buf, size_t bufSize)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, AG_VARIABLE_STRING, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	V->data.s = buf;
	V->info.size = bufSize;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_PrtString(void *obj, const char *name, const char *fmt, ...)
{
	AG_Variable *V;
	va_list ap;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, AG_VARIABLE_STRING, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	va_start(ap, fmt);
	Vasprintf(&V->data.s, fmt, ap);
	va_end(ap);
	V->info.size = 0;				/* Allocated */
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindString(void *obj, const char *name, char *v, size_t size)
{
	AG_Variable *V;
	int new;

	AG_ObjectLock(obj);
	V = FetchVariableNew(obj, name, AG_VARIABLE_P_STRING, &new);
	if (!new && V->info.size == 0) { Free(V->data.s); }
	V->data.p = v;
	V->info.size = size;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindStringFn(void *obj, const char *name, AG_StringFn fn, const char *fmt, ...)
{
	VARIABLE_BIND_FN(obj, name, fnString, AG_VARIABLE_STRING);
}
AG_Variable *
AG_BindString_MP(void *obj, const char *name, char *v, size_t size,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_STRING);
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
	V->data.p = v;
	V->info.size = strlen(*v)+1;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindConstString_MP(void *obj, const char *name, const char **v,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.p = v;
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
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag_MP(void *obj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG);
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
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8_MP(void *obj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG8);
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
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16_MP(void *obj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG16);
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
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32_MP(void *obj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = FetchVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}
