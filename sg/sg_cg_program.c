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
 * Vertex/fragment program written in the Cg language.
 */

#include <agar/config/have_cg.h>
#ifdef HAVE_CG

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

CGcontext sgCgProgramCtx = NULL;

static void
Init(void *_Nonnull obj)
{
	SG_CgProgram *prog = obj;

	prog->type = SG_VERTEX_PROGRAM;
	prog->objs = NULL;
	prog->nObjs = 0;
	prog->instObj = NULL;
	
	if (sgCgProgramCtx == NULL &&
	    (sgCgProgramCtx = cgCreateContext()) == NULL)
		AG_FatalError("cgCreateContext() failed");
}

static void
Destroy(void *_Nonnull obj)
{
	SG_CgProgram *prog = obj;
	int i;

	for (i = 0; i < prog->nObjs; i++)
		cgDestroyProgram(prog->objs[i]);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_CgProgram *prog = obj;

	prog->type = (int)AG_ReadUint32(buf);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SG_CgProgram *prog = obj;

	AG_WriteUint32(buf, (Uint32)prog->type);
	return (0);
}

static void
PollObjs(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_CgProgram *prog = AG_PTR(1);
	AG_TlistItem *it;
	int i;

	AG_TlistBegin(tl);
	AG_ObjectLock(prog);
	for (i = 0; i < prog->nObjs; i++) {
		CGprogram obj = prog->objs[i];

		it = AG_TlistAdd(tl, NULL, "%s%s",
		    cgGetProfileString(cgGetProgramProfile(obj)),
		    (obj == prog->instObj) ? _(" (installed)") : "");
		it->p1 = obj;
	}
	AG_ObjectUnlock(prog);
	AG_TlistEnd(tl);
}

static void
ViewProgramText(AG_Event *_Nonnull event)
{
	SG_CgProgram *prog = AG_PTR(1);
	AG_Textbox *tbox = AG_TEXTBOX_PTR(2);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(3);
	CGprogram cgp = (CGprogram)it->p1;
	const char *s;

	AG_ObjectLock(prog);
	s = cgGetProgramString(cgp, CG_COMPILED_PROGRAM);
	AG_ObjectUnlock(prog);

	AG_TextboxSetString(tbox, s);
}

static void *_Nonnull 
Edit(void *_Nonnull obj)
{
	SG_CgProgram *prog = obj;
	AG_Mutex *lock = &OBJECT(prog)->lock;
	AG_Window *win;
	AG_Box *hBox;
	const char *programTypeNames[] = {
		N_("Vertex Program"),
		N_("Fragment Program"),
		NULL
	};
	AG_Radio *rad;
	AG_Tlist *tl;
	AG_Textbox *tbox;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Cg Program <%s>"), OBJECT(prog)->name);

	rad = AG_RadioNew(win, 0, programTypeNames);
	AG_BindIntMp(rad, "value", &prog->type, lock);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);

	tl = AG_TlistNewPolled(hBox, AG_TLIST_VFILL, PollObjs, "%p", prog);
	AG_TlistSizeHint(tl, "fp30unlimited", 10);

	tbox = AG_TextboxNewS(hBox,
	    AG_TEXTBOX_EXPAND|AG_TEXTBOX_MULTILINE|AG_TEXTBOX_CATCH_TAB, NULL);
	AG_TextboxSizeHint(tbox,
	    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
	    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
	    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
	    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	
	AG_TlistSetDblClickFn(tl, ViewProgramText, "%p,%p", prog, tbox);
	return (win);
}

static int
Install(void *_Nonnull p, SG_View *_Nonnull view)
{
	SG_CgProgram *prog = p;
	CGprofile latest = cgGLGetLatestProfile(
	    (prog->type == SG_VERTEX_PROGRAM) ? CG_GL_VERTEX : CG_GL_FRAGMENT);
	CGprofile prof;
	int i;

	for (i = 0; i < prog->nObjs; i++) {
		prof = cgGetProgramProfile(prog->objs[i]);
		if (prof == latest) {
			prog->instObj = prog->objs[i];
			prog->instProf = prof;
			break;
		}
	}
	if (i == prog->nObjs) {
		for (i = 0; i < prog->nObjs; i++) {
			prof = cgGetProgramProfile(prog->objs[i]);
			if (cgGLIsProfileSupported(prof) == CG_TRUE) {
				prog->instObj = prog->objs[i];
				prog->instProf = prof;
				break;
			}
		}
		if (i == prog->nObjs) {
			AG_SetError(_("No supported Cg profile for %s"),
			    OBJECT(prog)->name);
			return (-1);
		}
	}

	cgGLEnableProfile(prog->instProf);
	cgGLLoadProgram(prog->instObj);
	return (0);
}

static void
Deinstall(void *_Nonnull p, SG_View *_Nonnull view)
{
	SG_CgProgram *prog = p;

	cgGLDisableProfile(prog->instProf);
	prog->instObj = NULL;
}

static void
Bind(void *_Nonnull p, SG_View *_Nonnull view)
{
	SG_CgProgram *prog = p;

	cgGLBindProgram(prog->instObj);
}

static void
Unbind(void *_Nonnull p, SG_View *_Nonnull view)
{
	SG_CgProgram *prog = p;

	cgGLUnbindProgram(prog->instProf);
}

SG_ProgramClass sgCgProgramClass = {
	{
		"SG_Program:SG_CgProgram",
		sizeof(SG_CgProgram),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		Load,
		Save,
		Edit
	},
	Install,
	Deinstall,
	Bind,
	Unbind
};

#endif /* HAVE_CG */
