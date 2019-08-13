/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Base tool class for VG_View.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>

#include <stdarg.h>

void
VG_ToolInit(VG_Tool *t)
{
	t->selected = 0;
	t->tag = 0;
	t->editWin = NULL;
	t->editArea = NULL;
	TAILQ_INIT(&t->cmds);

	if (t->ops->init != NULL)
		t->ops->init(t);
}

void
VG_ToolDestroy(VG_Tool *tool)
{
	VG_ToolCommand *cmd, *cmdNext;
	
	if (tool->editWin != NULL)
		AG_ObjectDetach(tool->editWin);
	
	if (tool->editArea != NULL) {
		AG_ObjectFreeChildren(tool->editArea);
		AG_WidgetUpdate(tool->editArea);
	}
	for (cmd = TAILQ_FIRST(&tool->cmds);
	     cmd != TAILQ_END(&tool->cmds);
	     cmd = cmdNext) {
		cmdNext = TAILQ_NEXT(cmd, cmds);
		Free(cmd->name);
		Free(cmd);
	}
	if (tool->ops->destroy != NULL)
		tool->ops->destroy(tool);
}

static void
ToolWindowClosed(AG_Event *_Nonnull event)
{
	VG_Tool *tool = AG_PTR(1);

	VG_ViewSelectTool(tool->vgv, NULL, NULL);
}

AG_Window *
VG_ToolWindow(void *obj)
{
	VG_Tool *tool = obj;
	AG_Window *win;

	win = tool->editWin = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _(tool->ops->desc));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_SetEvent(win, "window-close", ToolWindowClosed, "%p", tool);
	return (win);
}

/* Register a tool command. */
VG_ToolCommand *
VG_ToolCommandNew(void *obj, const char *name, AG_EventFn fn)
{
	VG_Tool *tool = obj;
	VG_ToolCommand *tc;

	tc = Malloc(sizeof(VG_ToolCommand));
	tc->name = Strdup(name);
	tc->descr = NULL;
	tc->kMod = AG_KEYMOD_NONE;
	tc->kSym = 0;
	tc->tool = tool;
	tc->fn = AG_SetEvent(tool->vgv, NULL, fn, NULL);

	AG_ObjectLock(tool->vgv);
	TAILQ_INSERT_HEAD(&tool->cmds, tc, cmds);
	AG_ObjectUnlock(tool->vgv);
	return (tc);
}

/* Configure a keyboard shortcut for a command. */
void
VG_ToolCommandKey(VG_ToolCommand *tc, AG_KeyMod kMod, AG_KeySym kSym)
{
	AG_ObjectLock(tc->tool->vgv);
	tc->kMod = kMod;
	tc->kSym = kSym;
	AG_ObjectUnlock(tc->tool->vgv);
}

/* Set the long description for a command. */
void
VG_ToolCommandDescr(VG_ToolCommand *tc, const char *fmt, ...)
{
	va_list ap;

	AG_ObjectLock(tc->tool->vgv);
	va_start(ap, fmt);
	Free(tc->descr);
	Vasprintf(&tc->descr, fmt, ap);
	va_end(ap);
	AG_ObjectUnlock(tc->tool->vgv);
}

/* Execute a tool command. */
int
VG_ToolCommandExec(void *obj, const char *name, const char *fmt, ...)
{
	VG_Tool *tool = obj;
	VG_ToolCommand *cmd;
	AG_Event evPost;

	AG_ObjectLock(tool->vgv);

	TAILQ_FOREACH(cmd, &tool->cmds, cmds) {
		if (strcmp(cmd->name, name) == 0)
			break;
	}
	if (cmd == NULL) {
		AG_SetError("No such command: %s", name);
		goto fail;
	}
	Debug(tool->vgv, "%s: CMD: <%s>\n", tool->ops->name, name);
	AG_EventInit(&evPost);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(&evPost, fmt, ap);
		va_end(ap);
	}
	cmd->fn->fn(&evPost);

	AG_ObjectUnlock(tool->vgv);
	return (0);
fail:
	AG_ObjectUnlock(tool->vgv);
	return (-1);
}
