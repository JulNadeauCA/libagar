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
#include <core/view.h>

#include "checkbox.h"

#include "window.h"
#include "primitive.h"

#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define LABEL_SPACING 6

static void mousebuttondown(AG_Event *);
static void keydown(AG_Event *);

AG_Checkbox *
AG_CheckboxNew(void *parent, Uint flags, const char *label)
{
	AG_Checkbox *cb;

	cb = Malloc(sizeof(AG_Checkbox), M_OBJECT);
	AG_CheckboxInit(cb, flags, label);
	AG_ObjectAttach(parent, cb);
	if (flags & AG_CHECKBOX_FOCUS) {
		AG_WidgetFocus(cb);
	}
	return (cb);
}

AG_Checkbox *
AG_CheckboxNewFlag(void *parent, Uint *pFlags, Uint bitmask, const char *label)
{
	AG_Checkbox *cb;

	cb = AG_CheckboxNew(parent, AG_CHECKBOX_HFILL, label);
	AG_WidgetBindFlag(cb, "state", pFlags, bitmask);
	return (cb);
}

/* Create a set of checkboxes for the given set of flags. */
void
AG_CheckboxSetFromFlags(void *parent, Uint *pFlags, const AG_FlagDescr *fdSet)
{
	const AG_FlagDescr *fd;
	AG_Checkbox *cb;
	int i;

	for (i = 0; fdSet[i].bitmask != 0; i++) {
		fd = &fdSet[i];
		cb = AG_CheckboxNewFlag(parent, pFlags, fd->bitmask, fd->descr);
		if (!fd->writeable)
			AG_WidgetDisable(cb);
	}
}

void
AG_CheckboxInit(AG_Checkbox *cbox, Uint flags, const char *label)
{
	AG_WidgetInit(cbox, &agCheckboxOps, AG_WIDGET_FOCUSABLE);
	AG_WidgetBind(cbox, "state", AG_WIDGET_BOOL, &cbox->state);

	cbox->state = 0;
	cbox->labelTxt = (label != NULL) ? Strdup(label) : NULL;
	cbox->label = -1;
	
	AG_SetEvent(cbox, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(cbox, "window-keydown", keydown, NULL);
}

static void
Destroy(void *p)
{
	AG_Checkbox *cbox = p;

	Free(cbox->labelTxt,0);
	AG_WidgetDestroy(cbox);
}

static void
Draw(void *obj)
{
	AG_Checkbox *cbox = obj;
	AG_WidgetBinding *stateb;
	void *p;
	int state;
	
	stateb = AG_WidgetGetBinding(cbox, "state", &p);
	switch (stateb->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
		state = *(int *)p;
		break;
	case AG_WIDGET_FLAG:
		state = *(int *)p & (int)stateb->data.bitmask;
		break;
	case AG_WIDGET_FLAG8:
		state = *(Uint8 *)p & (Uint8)stateb->data.bitmask;
		break;
	case AG_WIDGET_FLAG16:
		state = *(Uint16 *)p & (Uint16)stateb->data.bitmask;
		break;
	case AG_WIDGET_FLAG32:
		state = *(Uint32 *)p & (Uint32)stateb->data.bitmask;
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

	if (AG_WidgetEnabled(cbox)) {
		agPrim.box(cbox,
		    0, 0,
		    WIDGET(cbox)->h, WIDGET(cbox)->h,
		    state ? -1 : 1,
		    AG_COLOR(CHECKBOX_COLOR));
	} else {
		agPrim.box_dithered(cbox,
		    0, 0,
		    WIDGET(cbox)->h, WIDGET(cbox)->h,
		    state ? -1 : 1,
		    AG_COLOR(CHECKBOX_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	}
	AG_WidgetUnlockBinding(stateb);

	if (cbox->labelTxt == NULL || cbox->labelTxt[0] == '\0') {
		return;
	}
	if (cbox->label == -1) {
		AG_TextColor(CHECKBOX_TXT_COLOR);
		cbox->label = AG_WidgetMapSurface(cbox,
		    AG_TextRender(cbox->labelTxt));
	}
	AG_WidgetBlitSurface(cbox, cbox->label,
	    WIDGET(cbox)->h + LABEL_SPACING,
	    0);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Checkbox *cbox = AG_SELF();
	int button = AG_INT(1);

	if (!AG_WidgetEnabled(cbox))
		return;

	if (button == SDL_BUTTON(1)) {
		AG_CheckboxToggle(cbox);
	}
	AG_WidgetFocus(cbox);
}

static void
keydown(AG_Event *event)
{
	AG_Checkbox *cbox = AG_SELF();
	
	if (!AG_WidgetEnabled(cbox))
		return;

	switch (AG_SDLKEY(1)) {
	case SDLK_RETURN:
	case SDLK_SPACE:
		AG_CheckboxToggle(cbox);
		break;
	default:
		break;
	}
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Checkbox *cb = p;

	r->h = agTextFontHeight;
	r->w = agTextFontHeight;
	
	if (cb->labelTxt != NULL) {
		AG_TextSize(cb->labelTxt, &r->w, NULL);
		r->w += agTextFontHeight + LABEL_SPACING;	/* Square */
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Checkbox *cb = p;

	if (a->w < agTextFontHeight || a->h < agTextFontHeight) {
		return (-1);
	}
	if (cb->labelTxt != NULL) {
		int wLbl, hLbl;

		AG_TextSize(cb->labelTxt, &wLbl, &hLbl);
		if (a->w < agTextFontHeight + LABEL_SPACING + wLbl ||
		    a->h < agTextFontHeight) {
			WIDGET(cb)->flags |= AG_WIDGET_CLIPPING;
		} else {
			WIDGET(cb)->flags &= ~(AG_WIDGET_CLIPPING);
		}
	}
	return (0);
}

/* Toggle the checkbox state. */
void
AG_CheckboxToggle(AG_Checkbox *cbox)
{
	AG_WidgetBinding *stateb;
	void *p;

	stateb = AG_WidgetGetBinding(cbox, "state", &p);
	switch (stateb->vtype) {
	case AG_WIDGET_BOOL:
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
		{
			int *state = (int *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    *state);
		}
		break;
	case AG_WIDGET_FLAG:
		{
			int *state = (int *)p;
			if (*state & (int)stateb->data.bitmask) {
				*state &= ~(int)stateb->data.bitmask;
			} else {
				*state |= (int)stateb->data.bitmask;
			}
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_FLAG8:
		{
			Uint8 *state = (Uint8 *)p;
			if (*state & (Uint8)stateb->data.bitmask) {
				*state &= ~(Uint8)stateb->data.bitmask;
			} else {
				*state |= (Uint8)stateb->data.bitmask;
			}
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    (Uint8)*state);
		}
		break;
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
		{
			Uint8 *state = (Uint8 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
		{
			Uint16 *state = (Uint16 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		{
			Uint32 *state = (Uint32 *)p;
			*state = !(*state);
			AG_PostEvent(NULL, cbox, "checkbox-changed", "%i",
			    (int)*state);
		}
		break;
	}
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
}

const AG_WidgetOps agCheckboxOps = {
	{
		"AG_Widget:AG_Checkbox",
		sizeof(AG_Checkbox),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
