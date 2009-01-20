/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
AllocVariable(AG_Object *obj, const char *name, enum ag_variable_type type)
{
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
	V->name = name;
	return (V);
}

/*
 * Lookup a variable by name and return it locked.
 * Object must be locked.
 */
static __inline__ AG_Variable *
GetVariable(AG_Object *obj, const char *name)
{
	Uint i;
	AG_Variable *V;
	
	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(obj->vars[i].name, name) == 0)
			break;
	}
	if (i == obj->nVars) {
		AG_SetError("%s: No such variable: \"%s\"", obj->name, name);
		return (NULL);
	}
	V = &obj->vars[i];
	if (V->mutex != NULL) {
		AG_MutexLock(V->mutex);
	}
	return (V);
}

/* Release any mutex associated with a variable. */
static __inline__ void
UnlockVariable(AG_Variable *V)
{
	if (V->mutex != NULL)
		AG_MutexUnlock(V->mutex);
}

/* Print the specified variable to fixed-size buffer. */
void
AG_VariablePrint(char *s, size_t len, void *obj, const char *pname)
{
	AG_Variable *V;

	if ((V = GetVariable(obj, pname)) == NULL) {
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
	V = AllocVariable(obj, name, AG_VARIABLE_NULL);

	va_start(ap, fmt);
	AG_VARIABLE_GET(ap, fmt, V);
	va_end(ap);

	V->name = name;
	AG_ObjectUnlock(obj);
	return (V);
}

/*
 * Integer get/set routines.
 */

AG_Variable *
AG_SetUint(void *pObj, const char *name, Uint v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_UINT);
	V->data.u = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint(void *pObj, const char *name, Uint *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint_MP(void *pObj, const char *name, Uint *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetInt(void *pObj, const char *name, int v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_INT);
	V->data.i = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindInt(void *pObj, const char *name, int *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_INT);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindInt_MP(void *pObj, const char *name, int *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_INT);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetUint8(void *pObj, const char *name, Uint8 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_UINT8);
	V->data.u8 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint8(void *pObj, const char *name, Uint8 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT8);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint8_MP(void *pObj, const char *name, Uint8 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT8);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetSint8(void *pObj, const char *name, Sint8 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_SINT8);
	V->data.s8 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint8(void *pObj, const char *name, Sint8 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT8);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint8_MP(void *pObj, const char *name, Sint8 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT8);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetUint16(void *pObj, const char *name, Uint16 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_UINT16);
	V->data.u16 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint16(void *pObj, const char *name, Uint16 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT16);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint16_MP(void *pObj, const char *name, Uint16 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT16);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetSint16(void *pObj, const char *name, Sint16 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_SINT16);
	V->data.s16 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint16(void *pObj, const char *name, Sint16 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT16);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint16_MP(void *pObj, const char *name, Sint16 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT16);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetUint32(void *pObj, const char *name, Uint32 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_UINT32);
	V->data.u32 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint32(void *pObj, const char *name, Uint32 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT32);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindUint32_MP(void *pObj, const char *name, Uint32 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_UINT32);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetSint32(void *pObj, const char *name, Sint32 v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_SINT32);
	V->data.s32 = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint32(void *pObj, const char *name, Sint32 *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT32);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindSint32_MP(void *pObj, const char *name, Sint32 *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_SINT32);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetFloat(void *pObj, const char *name, float v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_FLOAT);
	V->data.flt = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFloat(void *pObj, const char *name, float *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLOAT);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFloat_MP(void *pObj, const char *name, float *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLOAT);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetDouble(void *pObj, const char *name, double v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_DOUBLE);
	V->data.dbl = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindDouble(void *pObj, const char *name, double *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_DOUBLE);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindDouble_MP(void *pObj, const char *name, double *v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_DOUBLE);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetString(void *pObj, const char *name, const char *fmt, ...)
{
	AG_Object *obj = pObj;
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
AG_BindString(void *pObj, const char *name, char *v, size_t size)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_STRING);
	V->data.p = v;
	V->info.size = size;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindString_MP(void *pObj, const char *name, char *v, size_t size,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
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
AG_SetConstString(void *pObj, const char *name, const char *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_CONST_STRING);
	V->data.Cs = v;
	V->info.size = strlen(v)+1;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindConstString(void *pObj, const char *name, const char **v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.p = v;
	V->info.size = strlen(*v)+1;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindConstString_MP(void *pObj, const char *name, const char **v,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_STRING);
	V->data.p = v;
	V->info.size = strlen(*v)+1;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetPointer(void *pObj, const char *name, void *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_POINTER);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindPointer(void *pObj, const char *name, void **v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_POINTER);
	V->data.p = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindPointer_MP(void *pObj, const char *name, void **v, AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_POINTER);
	V->data.p = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_SetConstPointer(void *pObj, const char *name, const void *v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_CONST_POINTER);
	V->data.Cp = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindConstPointer(void *pObj, const char *name, const void **v)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_POINTER);
	V->data.Cp = v;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindConstPointer_MP(void *pObj, const char *name, const void **v,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_CONST_POINTER);
	V->data.Cp = v;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFlag(void *pObj, const char *name, Uint *v, Uint bitmask)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFlag_MP(void *pObj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
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
AG_BindFlag8(void *pObj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFlag8_MP(void *pObj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
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
AG_BindFlag16(void *pObj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFlag16_MP(void *pObj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
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
AG_BindFlag32(void *pObj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindFlag32_MP(void *pObj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	V = AllocVariable(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask = bitmask;
	V->mutex = mutex;
	AG_ObjectUnlock(obj);
	return (V);
}
