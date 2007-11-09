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
 * Vertex/fragment program object.
 */

#include <config/have_opengl.h>
#include <config/have_cg.h>
#if defined(HAVE_OPENGL) && defined(HAVE_CG)

#include <core/core.h>
#include <core/util.h>

#include "sg.h"
#include "sg_gui.h"

#include <gui/window.h>
#include <gui/notebook.h>
#include <gui/box.h>
#include <gui/label.h>
#include <gui/tlist.h>
#include <gui/button.h>
#include <gui/file_dlg.h>
#include <gui/radio.h>

CGcontext sgCgProgramCtx = NULL;

static void
Init(void *obj)
{
	SG_CgProgram *prog = obj;

	prog->type = SG_VERTEX_PROGRAM;
	prog->objs = NULL;
	prog->nObjs = 0;
	prog->instObj = NULL;
	
	if (sgCgProgramCtx == NULL &&
	    (sgCgProgramCtx = cgCreateContext()) == NULL)
		fatal("cgCreateContext() failed");
}

static void
Destroy(void *obj)
{
	SG_CgProgram *prog = obj;
	int i;

	for (i = 0; i < prog->nObjs; i++)
		cgDestroyProgram(prog->objs[i]);
}

static int
Load(void *obj, AG_DataSource *buf)
{
	SG_CgProgram *prog = obj;

	if (AG_ReadObjectVersion(buf, prog, NULL) != 0) {
		return (-1);
	}
	prog->type = (int)AG_ReadUint32(buf);
	return (0);
}

static int
Save(void *obj, AG_DataSource *buf)
{
	SG_CgProgram *prog = obj;

	AG_WriteObjectVersion(buf, prog);
	AG_WriteUint32(buf, (Uint32)prog->type);
	return (0);
}

static void
PollObjs(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SG_CgProgram *prog = AG_PTR(1);
	AG_TlistItem *it;
	int i;
	
	AG_TlistBegin(tl);
	for (i = 0; i < prog->nObjs; i++) {
		CGprogram obj = prog->objs[i];

		it = AG_TlistAdd(tl, NULL, "%s%s",
		    cgGetProfileString(cgGetProgramProfile(obj)),
		    (obj == prog->instObj) ? _(" (installed)") : "");
		it->p1 = obj;
	}
	AG_TlistEnd(tl);
}

static void
ViewProgramText(AG_Event *event)
{
	CGprogram cgp = AG_TLIST_ITEM(0);
	AG_Textbox *tbox = AG_PTR(1);
	const char *s;

	s = cgGetProgramString(cgp, CG_COMPILED_PROGRAM);
	AG_TextboxPrintf(tbox, "%s", s);
}

static void *
Edit(void *obj)
{
	SG_CgProgram *prog = obj;
	AG_Window *win;
	AG_Box *hBox;
	AG_Radio *rad;
	const char *programTypeNames[] = {
		N_("Vertex Program"),
		N_("Fragment Program"),
		NULL
	};

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Cg Program <%s>"), OBJECT(prog)->name);

	rad = AG_RadioNew(win, 0, programTypeNames);
	AG_WidgetBindInt(rad, "value", &prog->type);

	hBox = AG_BoxNewHoriz(win, AG_BOX_EXPAND);
	{
		AG_Tlist *tl;
		AG_Textbox *tbox;

		tl = AG_TlistNewPolled(hBox, AG_TLIST_VFILL,
		    PollObjs, "%p", prog);
		AG_TlistSizeHint(tl, "fp30unlimited", 10);

		tbox = AG_TextboxNew(hBox,
		    AG_TEXTBOX_EXPAND|AG_TEXTBOX_MULTILINE|
		    AG_TEXTBOX_CATCH_TAB, NULL);
		AG_TextboxSizeHint(tbox,
		    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
		    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
		    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n"
		    "XXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	
		AG_TlistSetDblClickFn(tl, ViewProgramText, "%p", tbox);
	}
	return (win);
}

static int
Install(void *p, SG_View *view)
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
Deinstall(void *p, SG_View *view)
{
	SG_CgProgram *prog = p;

	cgGLDisableProfile(prog->instProf);
	prog->instObj = NULL;
}

static void
Bind(void *p, SG_View *view)
{
	SG_CgProgram *prog = p;

	cgGLBindProgram(prog->instObj);
}

static void
Unbind(void *p, SG_View *view)
{
	SG_CgProgram *prog = p;

	cgGLUnbindProgram(prog->instProf);
}

const SG_ProgramOps sgCgProgramOps = {
	{
		"SG_Program:SG_CgProgram",
		sizeof(SG_CgProgram),
		{ 0,0 },
		Init,
		NULL,		/* free */
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

#endif /* HAVE_OPENGL and HAVE_CG */
