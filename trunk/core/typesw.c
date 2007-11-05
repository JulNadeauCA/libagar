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
 * Routines related to the Object class information table. This table allows,
 * notably, archived objects to be dynamically allocated and initialized.
 */

#include <core/core.h>
#include <core/typesw.h>

#include <string.h>

extern const AG_ObjectOps agObjectOps;

AG_ObjectOps **agClassTbl = NULL;
int            agClassCount = 0;

void
AG_InitClassTbl(void)
{
	agClassTbl = Malloc(sizeof(AG_ObjectOps *));
	agClassCount = 0;
	AG_RegisterClass(&agObjectOps);
}

void
AG_DestroyClassTbl(void)
{
	Free(agClassTbl);
	agClassTbl = NULL;
	agClassCount = 0;
}

void
AG_RegisterClass(const void *cl)
{
	agClassTbl = Realloc(agClassTbl,
	    (agClassCount+1)*sizeof(AG_ObjectOps *));
	agClassTbl[agClassCount++] = (AG_ObjectOps *)cl;
}
