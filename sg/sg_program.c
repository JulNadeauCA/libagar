/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

static void
Init(void *_Nonnull obj)
{
	SG_Program *prog = obj;

	prog->flags = 0;
	prog->tag = 0;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_Program *prog = obj;

	prog->flags = (Uint)AG_ReadUint32(buf);
	prog->tag = (int)AG_ReadSint32(buf);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SG_Program *prog = obj;

	AG_WriteUint32(buf, (Uint32)prog->flags);
	AG_WriteSint32(buf, (Sint32)prog->tag);
	return (0);
}

/* Compile, verify and upload a program to the hardware. */
int
SG_ProgramInstall(SG_Program *prog, SG_View *view)
{
	int rv;

	AG_ObjectLock(prog);
	if (SG_PROGRAM_OPS(prog)->install != NULL) {
		rv = SG_PROGRAM_OPS(prog)->install(prog, view);
	} else {
		rv = 0;
	}
	AG_ObjectUnlock(prog);
	return (rv);
}

/* Uninstall a program from the hardware. */
void
SG_ProgramDeinstall(SG_Program *prog, SG_View *view)
{
	AG_ObjectLock(prog);
	if (SG_PROGRAM_OPS(prog)->deinstall != NULL) {
		SG_PROGRAM_OPS(prog)->deinstall(prog, view);
	}
	AG_ObjectUnlock(prog);
}

/* Bind a program to the current rendering context. */
void
SG_ProgramBind(SG_Program *prog, SG_View *view)
{
	AG_ObjectLock(prog);
	if (SG_PROGRAM_OPS(prog)->bind != NULL) {
		SG_PROGRAM_OPS(prog)->bind(prog, view);
	}
	AG_ObjectUnlock(prog);
}

/* Unbind a program from the current rendering context. */
void
SG_ProgramUnbind(SG_Program *prog, SG_View *view)
{
	AG_ObjectLock(prog);
	if (SG_PROGRAM_OPS(prog)->unbind != NULL) {
		SG_PROGRAM_OPS(prog)->unbind(prog, view);
	}
	AG_ObjectUnlock(prog);
}

AG_ObjectClass sgProgramClass = {
	"SG_Program",
	sizeof(SG_Program),
	{ 0,0 },
	Init,
	NULL,			/* reset */
	NULL,			/* destroy */
	Load,
	Save,
	NULL,			/* edit */
};
