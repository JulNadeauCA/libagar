/*
 * Copyright (c) 2002-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * LEGACY Interface to AG_Variable(3).
 */

#include <core/core.h>

#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

/*
 * Create a new property, or modify an existing one.
 * LEGACY Interface to AG_Variable(3).
 */
AG_Prop *
AG_SetProp(void *pObj, const char *key, enum ag_prop_type type, ...)
{
	AG_Object *obj = pObj;
	va_list ap;

	AG_ObjectLock(obj);
	va_start(ap, type);
	switch (type) {
	case AG_PROP_INT:	AG_SetInt(obj, key, va_arg(ap,int));		break;
	case AG_PROP_UINT:	AG_SetUint(obj, key, va_arg(ap,Uint));		break;
	case AG_PROP_FLOAT:	AG_SetFloat(obj, key, (float)va_arg(ap,double)); break;
	case AG_PROP_DOUBLE:	AG_SetDouble(obj, key, va_arg(ap,double));	break;
	case AG_PROP_STRING:	AG_SetString(obj, key, va_arg(ap,char *));	break;
	case AG_PROP_POINTER:	AG_SetPointer(obj, key, va_arg(ap,void *));	break;
	case AG_PROP_UINT8:	AG_SetUint8(obj, key, (Uint8)va_arg(ap,int));	break;
	case AG_PROP_SINT8:	AG_SetSint8(obj, key, (Sint8)va_arg(ap,int));	break;
	case AG_PROP_UINT16:	AG_SetUint16(obj, key, (Uint16)va_arg(ap,int));	break;
	case AG_PROP_SINT16:	AG_SetSint16(obj, key, (Sint16)va_arg(ap,int));	break;
	case AG_PROP_UINT32:	AG_SetUint32(obj, key, va_arg(ap,Uint32));	break;
	case AG_PROP_SINT32:	AG_SetSint32(obj, key, va_arg(ap,Sint32));	break;
	default:								break;
	}
	va_end(ap);
	AG_ObjectUnlock(obj);

	return AG_GetProp(obj, key, type, NULL);
}

#undef PROP_GET
#define PROP_GET(v,type) do { \
	if (p != NULL) *(type *)p = (type)V->data.v; \
} while (0)

/*
 * Lookup a property by name.
 * LEGACY interface to AG_Variable(3).
 */
AG_Variable *
AG_GetProp(void *pObj, const char *key, int t, void *p)
{
	AG_Object *obj = pObj;
	Uint i;

	AG_ObjectLock(obj);
	for (i = 0; i < obj->nVars; i++) {
		AG_Variable *V = &obj->vars[i];

		if ((t >= 0 && t != V->type) || strcmp(key, V->name) != 0) {
			continue;
		}
		switch (AG_VARIABLE_TYPE(V)) {
		case AG_VARIABLE_INT:    PROP_GET(i, int);	break;
		case AG_VARIABLE_UINT:   PROP_GET(u, unsigned);	break;
		case AG_VARIABLE_UINT8:  PROP_GET(u8, Uint8);	break;
		case AG_VARIABLE_SINT8:  PROP_GET(s8, Sint8);	break;
		case AG_VARIABLE_UINT16: PROP_GET(u16, Uint16);	break;
		case AG_VARIABLE_SINT16: PROP_GET(s16, Sint16);	break;
		case AG_VARIABLE_UINT32: PROP_GET(u32, Uint32);	break;
		case AG_VARIABLE_SINT32: PROP_GET(s32, Sint32);	break;
		case AG_VARIABLE_FLOAT:  PROP_GET(flt, float);	break;
		case AG_VARIABLE_DOUBLE: PROP_GET(dbl, double);	break;
		case AG_VARIABLE_STRING: PROP_GET(s, char *);	break;
		case AG_VARIABLE_POINTER:PROP_GET(p, void *);	break;
		default:
			AG_SetError("Bad prop %d", V->type);
			goto fail;
		}
		AG_ObjectUnlock(obj);
		return (V);
	}
	AG_SetError("%s: No such property: \"%s\" (%d)", obj->name, key, t);
fail:
	AG_ObjectUnlock(obj);
	return (NULL);
}
