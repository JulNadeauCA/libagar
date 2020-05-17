/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic framework for the implementation of SK(3) tools.
 */ 

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

void
SK_ToolInit(SK_Tool *t)
{
	t->win = NULL;
	t->trigger = NULL;
	SLIST_INIT(&t->kbindings);
	SLIST_INIT(&t->mbindings);

	if (t->ops->init != NULL)
		t->ops->init(t);
}

void
SK_ToolDestroy(SK_Tool *tool)
{
	SK_ToolKeyBinding *kbinding, *nkbinding;
	SK_ToolMouseBinding *mbinding, *nmbinding;
	
	if (tool->win != NULL) {
		AG_ObjectDetach(tool->win);
	}
	for (kbinding = SLIST_FIRST(&tool->kbindings);
	     kbinding != SLIST_END(&tool->kbindings);
	     kbinding = nkbinding) {
		nkbinding = SLIST_NEXT(kbinding, kbindings);
		Free(kbinding);
	}
	for (mbinding = SLIST_FIRST(&tool->mbindings);
	     mbinding != SLIST_END(&tool->mbindings);
	     mbinding = nmbinding) {
		nmbinding = SLIST_NEXT(mbinding, mbindings);
		Free(mbinding);
	}
	if (tool->ops->destroy != NULL)
		tool->ops->destroy(tool);
}

static void
SK_ToolWindowClosed(AG_Event *_Nonnull event)
{
	SK_Tool *tool = AG_PTR(1);

	SK_ViewSelectTool(tool->skv, NULL, NULL);
}

AG_Window *
SK_ToolWindow(void *p, const char *name)
{
	SK_Tool *tool = p;
	AG_Window *win;

	if ((win = tool->win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, _(tool->ops->desc));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_SetEvent(win, "window-close", SK_ToolWindowClosed, "%p", tool);
	return (win);
}

void
SK_ToolBindMouseButton(void *p, int button,
    int (*func)(SK_Tool *, int, int, M_Vector3, void *), void *arg)
{
	SK_Tool *tool = p;
	SK_ToolMouseBinding *mb;
	
	mb = Malloc(sizeof(SK_ToolMouseBinding));
	mb->button = button;
	mb->func = func;
	mb->edit = 0;
	mb->arg = arg;
	SLIST_INSERT_HEAD(&tool->mbindings, mb, mbindings);
}

void
SK_ToolBindKey(void *p, AG_KeyMod keymod, AG_KeySym keysym,
    int (*func)(SK_Tool *, AG_KeySym, int, void *), void *arg)
{
	SK_Tool *tool = p;
	SK_ToolKeyBinding *kb;

	kb = Malloc(sizeof(SK_ToolKeyBinding));
	kb->key = keysym;
	kb->mod = keymod;
	kb->func = func;
	kb->edit = 0;
	kb->arg = arg;
	SLIST_INSERT_HEAD(&tool->kbindings, kb, kbindings);
}

void
SK_ToolUnbindKey(void *p, AG_KeyMod keymod, AG_KeySym keysym)
{
	SK_Tool *tool = p;
	SK_ToolKeyBinding *kb;

	SLIST_FOREACH(kb, &tool->kbindings, kbindings) {
		if (kb->mod == keymod &&
		    kb->key == keysym)
			break;
	}
	if (kb == NULL) {
		return;
	}
	SLIST_REMOVE(&tool->kbindings, kb, sk_tool_keybinding, kbindings);
	Free(kb);
}
