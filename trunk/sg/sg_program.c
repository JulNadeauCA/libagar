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
 * Generic program class. This is the base class for vertex and fragment
 * shader programs.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/util.h>

#include "sg.h"

void
SG_ProgramInit(void *obj, const char *name)
{
	SG_Program *prog = obj;

	AG_ObjectInit(prog, name, &sgProgramOps);
	prog->flags = 0;
}

int
SG_ProgramLoad(void *obj, AG_Netbuf *buf)
{
	SG_Program *prog = obj;

	if (AG_ReadObjectVersion(buf, prog, NULL) != 0) {
		return (-1);
	}
	prog->flags = (Uint)AG_ReadUint32(buf);
	return (0);
}

int
SG_ProgramSave(void *obj, AG_Netbuf *buf)
{
	SG_Program *prog = obj;

	AG_WriteObjectVersion(buf, prog);
	AG_WriteUint32(buf, (Uint32)prog->flags);
	return (0);
}

void
SG_ProgramInstall(SG_Program *prog, SG_View *view)
{
	if (SG_PROGRAM_OPS(prog)->install != NULL)
		SG_PROGRAM_OPS(prog)->install(prog, view);
}

void
SG_ProgramDeinstall(SG_Program *prog, SG_View *view)
{
	if (SG_PROGRAM_OPS(prog)->deinstall != NULL)
		SG_PROGRAM_OPS(prog)->deinstall (prog, view);
}

void
SG_ProgramBind(SG_Program *prog, SG_View *view)
{
	if (SG_PROGRAM_OPS(prog)->bind != NULL)
		SG_PROGRAM_OPS(prog)->bind(prog, view);
}

void
SG_ProgramUnbind(SG_Program *prog, SG_View *view)
{
	if (SG_PROGRAM_OPS(prog)->unbind != NULL)
		SG_PROGRAM_OPS(prog)->unbind(prog, view);
}

const AG_ObjectOps sgProgramOps = {
	"SG_Program",
	sizeof(SG_Program),
	{ 0,0 },
	SG_ProgramInit,
	NULL,			/* reinit */
	NULL,			/* destroy */
	SG_ProgramLoad,
	SG_ProgramSave,
	NULL,			/* edit */
};

#endif /* HAVE_OPENGL */
