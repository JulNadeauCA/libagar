/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Physics engine class.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/util.h>

#include <sg/sg.h>

static void
Init(void *obj)
{
	PE *pe = obj;

	pe->flags = 0;
	pe->time = 0;
}

static int
Load(void *obj, AG_DataSource *buf, const AG_Version *ver)
{
	PE *pe = obj;

	pe->flags = (Uint)AG_ReadUint32(buf);
	return (0);
}

static int
Save(void *obj, AG_DataSource *buf)
{
	PE *pe = obj;

	AG_WriteUint32(buf, (Uint32)pe->flags);
	return (0);
}

void
PE_AttachObject(PE *pe, SG_Object *so)
{
	if (PE_OPS(pe)->attachObject != NULL)
		PE_OPS(pe)->attachObject(pe, so);
}

void
PE_DetachObject(PE *pe, SG_Object *so)
{
	if (PE_OPS(pe)->detachObject != NULL)
		PE_OPS(pe)->detachObject(pe, so);
}

const AG_ObjectOps peOps = {
	"PE",
	sizeof(PE),
	{ 0,0 },
	Init,
	NULL,			/* free */
	NULL,			/* destroy */
	Load,
	Save,
	NULL,			/* edit */
};

#endif /* HAVE_OPENGL */
