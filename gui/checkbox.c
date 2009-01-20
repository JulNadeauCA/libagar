/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "checkbox.h"

#include "window.h"
#include "primitive.h"
#include "text.h"

#include <stdarg.h>
#include <string.h>

AG_Checkbox *
AG_CheckboxNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Checkbox *cb;
	va_list args;

	cb = Malloc(sizeof(AG_Checkbox));
	AG_ObjectInit(cb, &agCheckboxClass);
	cb->flags |= flags;

	if (fmt != NULL) {
		char text[AG_LABEL_MAX];

		va_start(args, fmt);
		Vsnprintf(text, sizeof(text), fmt, args);
		va_end(args);
		cb->lbl = AG_LabelNewString(cb, 0, text);
		AG_LabelValign(cb->lbl, AG_TEXT_MIDDLE);
	}
	
	if (flags & AG_CHECKBOX_SET) {
		cb->state = 1;
	}
	AG_ObjectAttach(parent, cb);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewFn(void *parent, Uint flags, const char *label, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Checkbox *cb;
	AG_Event *ev;

	cb = AG_CheckboxNew(parent, flags, label);
	ev = AG_SetEvent(cb, "checkbox-changed", fn, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewInt(void *parent, Uint flags, const char *label, int *pVal)
{
	AG_Checkbox *cb;
	cb = AG_CheckboxNew(parent, flags, label);
	AG_BindInt(cb, "state", pVal);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewFlag(void *parent, Uint flags, const char *label, Uint *pFlags,
    Uint bitmask)
{
	AG_Checkbox *cb;
	cb = AG_CheckboxNew(parent, flags, label);
	AG_BindFlag(cb, "state", pFlags, bitmask);
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewFlag32(void *parent, Uint flags, const char *label,
    Uint32 *pFlags, Uint32 bitmask)
{
	AG_Checkbox *cb;
	cb = AG_CheckboxNew(parent, flags, label);
	AG_BindFlag32(cb, "state", pFlags, bitmask);
	return (cb);
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags(void *parent, Uint flags, Uint *pFlags,
    const AG_FlagDescr *fdSet)
{
	const AG_FlagDescr *fd;
	AG_Checkbox *cb;
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		fd = &fdSet[i];
		cb = AG_CheckboxNewFlag(parent, flags, fd->descr,
		    pFlags, fd->bitmask);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags32(void *parent, Uint flags, Uint32 *pFlags,
    const AG_FlagDescr *fdSet)
{
	const AG_FlagDescr *fd;
	AG_Checkbox *cb;
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		fd = &fdSet[i];
		cb = AG_CheckboxNewFlag32(parent, flags, fd->descr,
		    pFlags, (Uint32)fd->bitmask);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Checkbox *cb = AG_SELF();
	int button = AG_INT(1);

	if (!AG_WidgetEnabled(cb))
		return;

	if (button == SDL_BUTTON(1)) {
		AG_WidgetFocus(cb);
		AG_CheckboxToggle(cb);
	}
}

static void
KeyDown(AG_Event *event)
{
	AG_Checkbox *cb = AG_SELF();
	
	if (!AG_WidgetEnabled(cb))
		return;

	switch (AG_SDLKEY(1)) {
	case SDLK_RETURN:
	case SDLK_SPACE:
		AG_CheckboxToggle(cb);
		break;
	default:
		break;
	}
}

static void
Init(void *obj)
{
	AG_Checkbox *cb = obj;

	WIDGET(cb)->flags |= AG_WIDGET_FOCUSABLE;

	AG_BindInt(cb, "state", &cb->state);

	cb->flags = 0;
	cb->state = 0;
	cb->lbl = NULL;
	cb->spacing = 6;
	cb->btnSize = agTextFontHeight;
	
	AG_SetEvent(cb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(cb, "window-keydown", KeyDown, NULL);
}

static void
Draw(void *obj)
{
	AG_Checkbox *cb = obj;
	AG_WidgetBinding *stateb;
	void *p;
	int state;
	
	stateb = AG_WidgetGetBinding(cb, "state", &p);
	switch (stateb->type) {
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
		state = *(int *)p;
		break;
	case AG_WIDGET_FLAG:
		state = *(int *)p & (int)stateb->info.bitmask;
		break;
	case AG_WIDGET_FLAG8:
		state = *(Uint8 *)p & (Uint8)stateb->info.bitmask;
		break;
	case AG_WIDGET_FLAG16:
		state = *(Uint16 *)p & (Uint16)stateb->info.bitmask;
		break;
	case AG_WIDGET_FLAG32:
		state = *(Uint32 *)p & (Uint32)stateb->info.bitmask;
		break;
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
		state = *(Uint8 *)p;
		break;
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
		state = *(Uint16 *)p;
		break;
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		state = *(Uint32 *)p;
		break;
	default:
		state = 0;
		break;
	}

	STYLE(cb)->CheckboxButton(cb, state, cb->btnSize);
	if (cb->lbl != NULL) {
		AG_WidgetDraw(cb->lbl);
	}
	AG_WidgetUnlockBinding(stateb);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Checkbox *cb = obj;
	AG_SizeReq rLbl;

	r->h = cb->btnSize;
	r->w = cb->btnSize;
	
	if (cb->lbl != NULL) {
		AG_WidgetSizeReq(cb->lbl, &rLbl);
		r->w += cb->spacing + rLbl.w;
		r->h = agTextFontHeight;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Checkbox *cb = obj;
	AG_SizeAlloc aLbl;

	if (a->w < cb->btnSize || a->h < cb->btnSize) {
		return (-1);
	}
	if (cb->lbl != NULL) {
		aLbl.x = cb->btnSize + cb->spacing;
		aLbl.y = 0;
		aLbl.w = a->w - aLbl.x;
		aLbl.h = a->h;
		AG_WidgetSizeAlloc(cb->lbl, &aLbl);
	}
	return (0);
}

/* Toggle the checkbox state. */
void
AG_CheckboxToggle(AG_Checkbox *cb)
{
	AG_WidgetBinding *stateb;
	void *p;

	AG_ObjectLock(cb);
	stateb = AG_WidgetGetBinding(cb, "state", &p);
	switch (stateb->type) {
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
		{
			int *state = (int *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    *state);
		}
		break;
	case AG_WIDGET_FLAG:
		{
			int *state = (int *)p;
			AG_INVFLAGS(*state, (int)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			AG_INVFLAGS(*state, (Uint8)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint8)*state);
		}
		break;
	case AG_WIDGET_FLAG16:
		{
			Uint16 *state = (Uint16 *)p;
			AG_INVFLAGS(*state, (Uint16)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint16)*state);
		}
		break;
	case AG_WIDGET_FLAG32:
		{
			Uint32 *state = (Uint32 *)p;
			AG_INVFLAGS(*state, (Uint32)stateb->info.bitmask);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (Uint32)*state);
		}
		break;
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
		{
			Uint8 *state = (Uint8 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
		{
			Uint16 *state = (Uint16 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		{
			Uint32 *state = (Uint32 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cb, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	default:
		break;
	}
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
	AG_ObjectUnlock(cb);
}

AG_WidgetClass agCheckboxClass = {
	{
		"Agar(Widget:Checkbox)",
		sizeof(AG_Checkbox),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
