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

/* Names for standard variable types */
const char *agVariableTypeNames[] = {
	"NULL",
	"Uint",
	"int",
	"Uint8",
	"Sint8",
	"Uint16",
	"Sint16",
	"Uint32",
	"Sint32",
	"Uint64",
	"Sint64",
	"float",
	"double",
	"long double",
	"String",
	"const String",
	"Pointer",
	"const Pointer",

	"Uint *",
	"int *",
	"Uint8 *",
	"Sint8 *",
	"Uint16 *",
	"Sint16 *",
	"Uint32 *",
	"Sint32 *",
	"Uint64 *",
	"Sint64 *",
	"float *",
	"double *",
	"long double *",
	"String *",
	"const String *",
	"Pointer *",
	"const Pointer *",
	"AG_Object *",
	"Flag *",
	"Flag8 *",
	"Flag16 *",
	"Flag32 *",
	
	"Real",
	"Real *",
	"Range",
	"Range *",
	"Complex",
	"Complex *",
	"Quat",
	"Quat *",
	"Rectangular",
	"Rectangular *",
	"Polar",
	"Polar *",
	"Parabolic",
	"Parabolic *",
	"Spherical",
	"Spherical *",
	"Cylindrical",
	"Cylindrical *",
	"Color",
	"Color *",
	"Vector",
	"Vector *",
	"Vector2",
	"Vector2 *",
	"Vector3",
	"Vector3 *",
	"Vector4",
	"Vector4 *",
	"Matrix",
	"Matrix *",
	"Matrix22",
	"Matrix22 *",
	"Matrix33",
	"Matrix33 *",
	"Matrix44",
	"Matrix44 *",

	NULL
};

/* Parse the given arguments and return a List of Variables. */
AG_List *
AG_VariableList(const char *argSpec, ...)
{
	char *asDup, *as, *s;
	AG_Variable V;
	AG_List *L;
	va_list ap;

	if ((L = AG_ListNew()) == NULL) {
		return (NULL);
	}
	asDup = Strdup(argSpec);
	as = &asDup[0];
	va_start(ap, argSpec);
	while ((s = AG_Strsep(&as, ":, ")) != NULL) {
		AG_VARIABLE_GET(ap, s, &V);
		AG_ListAppend(L, &V);
	}
	va_end(ap);
	Free(asDup);
	return (L);
}

/*
 * Create a new variable or return one for modification by name.
 * Object must be locked.
 */
static __inline__ AG_Variable *
AllocVariable(void *pObj, const char *name, enum ag_variable_type type)
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

/* Print the specified variable to fixed-size buffer. */
void
AG_VariablePrint(char *s, size_t len, void *obj, const char *pname)
{
	AG_Variable *V;

	if ((V = AG_GetVariable(obj, pname)) == NULL) {
		Snprintf(s, len, "<%s>", AG_GetError());
		return;
	}
	switch (V->type) {
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_CONST_STRING:
		Snprintf(s, len, "\"%s\"", V->data.s);
		break;
	case AG_VARIABLE_POINTER:
	case AG_VARIABLE_CONST_POINTER:
		Snprintf(s, len, "%p", V->data.p);
		break;
	case AG_VARIABLE_UINT:		Snprintf(s, len, "%u", V->data.u);		break;
	case AG_VARIABLE_INT:		Snprintf(s, len, "%d", V->data.i);		break;
	case AG_VARIABLE_UINT8:		Snprintf(s, len, "%u", V->data.u8);		break;
	case AG_VARIABLE_SINT8:		Snprintf(s, len, "%d", V->data.s8);		break;
	case AG_VARIABLE_UINT16:	Snprintf(s, len, "%u", V->data.u16);		break;
	case AG_VARIABLE_SINT16:	Snprintf(s, len, "%d", V->data.s16);		break;
	case AG_VARIABLE_UINT32:	Snprintf(s, len, "%lu", (Ulong)V->data.u32);	break;
	case AG_VARIABLE_SINT32:	Snprintf(s, len, "%ld", (long)V->data.s32);	break;
	case AG_VARIABLE_FLOAT:		Snprintf(s, len, "%f", V->data.flt);		break;
	case AG_VARIABLE_DOUBLE:	Snprintf(s, len, "%f", V->data.dbl);		break;
	default:
		Strlcat(s, "<?>", len);
		break;
	}
	AG_UnlockVariable(V);
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
	return AG_GetVariable(obj, varName);
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
	V = AG_GetVariable(obj, name);
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

#undef  VARIABLE_GET_FN
#define VARIABLE_GET_FN(obj,V,_field,_fname) do {		\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, (V)->name, sizeof(evName));		\
	AG_ObjectLock(obj);					\
	TAILQ_FOREACH(ev, &OBJECT(obj)->events, events) {	\
		if (strcmp(evName, ev->name) == 0)		\
			break;					\
	}							\
	if (ev != NULL) {					\
		(V)->data._field = (V)->fn._fname(ev);		\
	}							\
	AG_ObjectUnlock(obj);					\
} while (0)

#undef  VARIABLE_GET
#define VARIABLE_GET(obj,name,_memb,_fn,_type,_getfn)		\
	_type rv;						\
	AG_Variable *V;						\
								\
	if ((V = AG_GetVariable((obj),(name))) == NULL) {	\
		return (0);					\
	}							\
	if (V->fn._fn != NULL) { _getfn((obj),V); }		\
	rv = V->data._memb;					\
	AG_UnlockVariable(V);					\
	return (rv)						\

#undef  VARIABLE_SET
#define VARIABLE_SET(obj,name,_memb,type)			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AllocVariable(obj, name, type);			\
	V->data._memb = v;					\
	AG_ObjectUnlock(obj);					\
	return (V)

#undef  VARIABLE_SET_FN
#define VARIABLE_SET_FN(obj,name,_memb,type)			\
	char evName[AG_EVENT_NAME_MAX];				\
	AG_Event *ev;						\
	AG_Variable *V;						\
								\
	Strlcpy(evName, "get-", sizeof(evName));		\
	Strlcat(evName, name, sizeof(evName));			\
	AG_ObjectLock(obj);					\
	V = AllocVariable(obj, name, type);			\
	V->fn._memb = fn;					\
	ev = AG_SetEvent(obj, evName, NULL, NULL);		\
	AG_EVENT_GET_ARGS(ev, fmt);				\
	AG_ObjectUnlock(obj);					\
	return (V)

#undef  VARIABLE_SET_MP
#define VARIABLE_SET_MP(obj,name,_memb,type)			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AllocVariable(obj, name, type);			\
	V->data._memb = v;					\
	V->mutex = mutex;					\
	AG_ObjectUnlock(obj);					\
	return (V)

/*
 * Unsigned integer
 */
static void
GetUintFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, u, fnUint);
}
Uint
AG_GetUint(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u, fnUint, Uint, GetUintFn);
}
AG_Variable *
AG_SetUint(void *obj, const char *name, Uint v)
{
	VARIABLE_SET(obj, name, u, AG_VARIABLE_UINT);
}
AG_Variable *
AG_BindUintFn(void *obj, const char *name, AG_UintFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnUint, AG_VARIABLE_UINT);
}
AG_Variable *
AG_BindUint(void *obj, const char *name, Uint *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_UINT);
}
AG_Variable *
AG_BindUint_MP(void *obj, const char *name, Uint *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_UINT);
}

/*
 * Signed integer
 */
static void
GetIntFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, i, fnInt);
}
int
AG_GetInt(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u, fnInt, int, GetIntFn);
}
AG_Variable *
AG_SetInt(void *obj, const char *name, int v)
{
	VARIABLE_SET(obj, name, i, AG_VARIABLE_INT);
}
AG_Variable *
AG_BindInt(void *obj, const char *name, int *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_INT);
}
AG_Variable *
AG_BindIntFn(void *obj, const char *name, AG_IntFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnInt, AG_VARIABLE_INT);
}
AG_Variable *
AG_BindInt_MP(void *obj, const char *name, int *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_INT);
}

/*
 * Unsigned 8-bit integer
 */
static void
GetUint8Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, u8, fnUint8);
}
Uint8
AG_GetUint8(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u8, fnUint8, Uint8, GetUint8Fn);
}
AG_Variable *
AG_SetUint8(void *obj, const char *name, Uint8 v)
{
	VARIABLE_SET(obj, name, s8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindUint8(void *obj, const char *name, Uint8 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_SINT8);
}
AG_Variable *
AG_BindUint8Fn(void *obj, const char *name, AG_Uint8Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnUint8, AG_VARIABLE_UINT8);
}
AG_Variable *
AG_BindUint8_MP(void *obj, const char *name, Uint8 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_SINT8);
}

/*
 * Signed 8-bit integer
 */
static void
GetSint8Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, s8, fnSint8);
}
Sint8
AG_GetSint8(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s8, fnSint8, Sint8, GetSint8Fn);
}
AG_Variable *
AG_SetSint8(void *obj, const char *name, Sint8 v)
{
	VARIABLE_SET(obj, name, s8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindSint8(void *obj, const char *name, Sint8 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_SINT8);
}
AG_Variable *
AG_BindSint8Fn(void *obj, const char *name, AG_Sint8Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnSint8, AG_VARIABLE_SINT8);
}
AG_Variable *
AG_BindSint8_MP(void *obj, const char *name, Sint8 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_SINT8);
}

/*
 * Unsigned 16-bit integer
 */
static void
GetUint16Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, u16, fnUint16);
}
Uint16
AG_GetUint16(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u16, fnUint16, Uint16, GetUint16Fn);
}
AG_Variable *
AG_SetUint16(void *obj, const char *name, Uint16 v)
{
	VARIABLE_SET(obj, name, u16, AG_VARIABLE_UINT16);
}
AG_Variable *
AG_BindUint16(void *obj, const char *name, Uint16 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_UINT16);
}
AG_Variable *
AG_BindUint16Fn(void *obj, const char *name, AG_Uint16Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnUint16, AG_VARIABLE_UINT16);
}
AG_Variable *
AG_BindUint16_MP(void *obj, const char *name, Uint16 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_UINT16);
}

/*
 * Signed 16-bit integer
 */
static void
GetSint16Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, s16, fnSint16);
}
Sint16
AG_GetSint16(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s16, fnSint16, Sint16, GetSint16Fn);
}
AG_Variable *
AG_SetSint16(void *obj, const char *name, Sint16 v)
{
	VARIABLE_SET(obj, name, s16, AG_VARIABLE_SINT16);
}
AG_Variable *
AG_BindSint16(void *obj, const char *name, Sint16 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_SINT16);
}
AG_Variable *
AG_BindSint16Fn(void *obj, const char *name, AG_Sint16Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnSint16, AG_VARIABLE_SINT16);
}
AG_Variable *
AG_BindSint16_MP(void *obj, const char *name, Sint16 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_SINT16);
}

/*
 * Unsigned 32-bit integer
 */
static void
GetUint32Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, u32, fnUint32);
}
Uint32
AG_GetUint32(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, u32, fnUint32, Uint32, GetUint32Fn);
}
AG_Variable *
AG_SetUint32(void *obj, const char *name, Uint32 v)
{
	VARIABLE_SET(obj, name, u32, AG_VARIABLE_UINT32);
}
AG_Variable *
AG_BindUint32(void *obj, const char *name, Uint32 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_UINT32);
}
AG_Variable *
AG_BindUint32Fn(void *obj, const char *name, AG_Uint32Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnUint32, AG_VARIABLE_UINT32);
}
AG_Variable *
AG_BindUint32_MP(void *obj, const char *name, Uint32 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_UINT32);
}

/*
 * Signed 32-bit integer
 */
static void
GetSint32Fn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, s32, fnSint32);
}
Sint32
AG_GetSint32(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, s32, fnSint32, Sint32, GetSint32Fn);
}
AG_Variable *
AG_SetSint32(void *obj, const char *name, Sint32 v)
{
	VARIABLE_SET(obj, name, s32, AG_VARIABLE_SINT32);
}
AG_Variable *
AG_BindSint32(void *obj, const char *name, Sint32 *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_SINT32);
}
AG_Variable *
AG_BindSint32Fn(void *obj, const char *name, AG_Sint32Fn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnSint32, AG_VARIABLE_SINT32);
}
AG_Variable *
AG_BindSint32_MP(void *obj, const char *name, Sint32 *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_SINT32);
}

/*
 * Single-precision floating-point number.
 */
static void
GetFloatFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, flt, fnFloat);
}
float
AG_GetFloat(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, flt, fnFloat, float, GetFloatFn);
}
AG_Variable *
AG_SetFloat(void *obj, const char *name, float v)
{
	VARIABLE_SET(obj, name, flt, AG_VARIABLE_FLOAT);
}
AG_Variable *
AG_BindFloat(void *obj, const char *name, float *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_FLOAT);
}
AG_Variable *
AG_BindFloatFn(void *obj, const char *name, AG_FloatFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnFloat, AG_VARIABLE_FLOAT);
}
AG_Variable *
AG_BindFloat_MP(void *obj, const char *name, float *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_FLOAT);
}

/*
 * Double-precision floating-point number.
 */
static void
GetDoubleFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, dbl, fnDouble);
}
double
AG_GetDouble(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, dbl, fnDouble, double, GetDoubleFn);
}
AG_Variable *
AG_SetDouble(void *obj, const char *name, double v)
{
	VARIABLE_SET(obj, name, dbl, AG_VARIABLE_DOUBLE);
}
AG_Variable *
AG_BindDouble(void *obj, const char *name, double *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_DOUBLE);
}
AG_Variable *
AG_BindDoubleFn(void *obj, const char *name, AG_DoubleFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnDouble, AG_VARIABLE_DOUBLE);
}
AG_Variable *
AG_BindDouble_MP(void *obj, const char *name, double *v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_DOUBLE);
}

/*
 * Pointer routines.
 */
static void
GetPointerFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, p, fnPointer);
}
void *
AG_GetPointer(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, p, fnPointer, void *, GetPointerFn);
}
AG_Variable *
AG_SetPointer(void *obj, const char *name, void *v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_POINTER);
}
AG_Variable *
AG_BindPointer(void *obj, const char *name, void **v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_POINTER);
}
AG_Variable *
AG_BindPointerFn(void *obj, const char *name, AG_PointerFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnPointer, AG_VARIABLE_POINTER);
}
AG_Variable *
AG_BindPointer_MP(void *obj, const char *name, void **v, AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_POINTER);
	return (V);
}

/*
 * Const pointer routines.
 */
static void
GetConstPointerFn(void *obj, AG_Variable *V)
{
	VARIABLE_GET_FN(obj, V, Cp, fnConstPointer);
}
const void *
AG_GetConstPointer(void *obj, const char *name)
{
	VARIABLE_GET(obj, name, Cp, fnConstPointer, const void *, GetConstPointerFn);
}
AG_Variable *
AG_SetConstPointer(void *obj, const char *name, const void *v)
{
	VARIABLE_SET(obj, name, Cp, AG_VARIABLE_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointer(void *obj, const char *name, const void **v)
{
	VARIABLE_SET(obj, name, p, AG_VARIABLE_P_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointerFn(void *obj, const char *name, AG_ConstPointerFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnConstPointer, AG_VARIABLE_CONST_POINTER);
}
AG_Variable *
AG_BindConstPointer_MP(void *obj, const char *name, const void **v,
    AG_Mutex *mutex)
{
	VARIABLE_SET_MP(obj, name, p, AG_VARIABLE_P_CONST_POINTER);
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

	if ((V = AG_GetVariable(obj, name)) == NULL) {
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

	if ((V = AG_GetVariable(obj, name)) == NULL) {
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
AG_SetString(void *obj, const char *name, const char *fmt, ...)
{
	AG_Variable *V;
	va_list ap;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_STRING);
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

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_STRING);
	V->data.p = v;
	V->info.size = size;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindStringFn(void *obj, const char *name, AG_StringFn fn, const char *fmt, ...)
{
	VARIABLE_SET_FN(obj, name, fnString, AG_VARIABLE_STRING);
}
AG_Variable *
AG_BindString_MP(void *obj, const char *name, char *v, size_t size,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_STRING);
	V->data.p = v;
	V->info.size = size;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_SetConstString(void *obj, const char *name, const char *v)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_CONST_STRING);
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
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.p = v;
	V->info.size = strlen(*v)+1;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindConstString_MP(void *obj, const char *name, const char **v,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.p = v;
	V->info.size = strlen(*v)+1;
	V->mutex = mutex;
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
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag_MP(void *obj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8(void *obj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag8_MP(void *obj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16(void *obj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag16_MP(void *obj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32(void *obj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}
AG_Variable *
AG_BindFlag32_MP(void *obj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

