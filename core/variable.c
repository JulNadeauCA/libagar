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

#if 0
/* Parse the given arguments and return a List of Variables. */
AG_List *
AG_ParseVariableList(const char *argSpec, ...)
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
#endif

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
	Uint i;

	AG_ObjectLock(obj);

	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(obj->vars[i].name, name) == 0)
			break;
	}
	if (i == obj->nVars) {			/* Create new variable */
		obj->vars = Realloc(obj->vars,
		    (obj->nVars+1)*sizeof(AG_Variable));
		V = &obj->vars[obj->nVars++];
	}
	va_start(ap, fmt);
	AG_VARIABLE_GET(ap, fmt, V);
	va_end(ap);
	V->name = name;

	AG_PostEvent(NULL, obj, "variable-set", "%p", V);

	AG_ObjectUnlock(obj);
	return (V);
}
