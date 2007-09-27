/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Routines related to the AG_Object type table. The type table allows
 * archived objects to be dynamically allocated and initialized.
 */

#include <core/core.h>
#include <core/typesw.h>

#include <string.h>

extern const AG_ObjectOps agObjectOps;
AG_ObjectType *agTypes = NULL;
int agnTypes = 0;

/* Initialize the type switch and register the built-in types. */
void
AG_InitTypeSw(void)
{
	agTypes = Malloc(sizeof(AG_ObjectType), M_TYPESW);
	agnTypes = 0;
	AG_RegisterType(&agObjectOps, OBJ_ICON);
}

void
AG_DestroyTypeSw(void)
{
	Free(agTypes, M_TYPESW);
}

void
AG_RegisterType(const void *ops, int icon)
{
	AG_ObjectType *ntype;

	agTypes = Realloc(agTypes, (agnTypes+1)*sizeof(AG_ObjectType));
	ntype = &agTypes[agnTypes++];
	ntype->ops = ops != NULL ? ops : &agObjectOps;
	ntype->icon = icon;
}

AG_ObjectType *
AG_FindType(const char *type)
{
	int i;

	for (i = 0; i < agnTypes; i++) {
		if (strcmp(agTypes[i].ops->type, type) == 0)
			return (&agTypes[i]);
	}
	return (NULL);
}
