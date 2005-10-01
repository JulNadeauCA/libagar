/*	$Csoft: checkbox.c,v 1.54 2005/09/27 00:25:22 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include "checkbox.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static AG_WidgetOps agCheckboxOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_WidgetDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_CheckboxDraw,
	AG_CheckboxScale
};

#define XSPACING 6

static void checkbox_mousebutton(int , union evarg *);
static void checkbox_keydown(int , union evarg *);

AG_Checkbox *
AG_CheckboxNew(void *parent, const char *fmt, ...)
{
	char caption[AG_LABEL_MAX];
	AG_Checkbox *cb;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(caption, sizeof(caption), fmt, ap);
	va_end(ap);

	cb = Malloc(sizeof(AG_Checkbox), M_OBJECT);
	AG_CheckboxInit(cb, caption);
	AG_ObjectAttach(parent, cb);
	return (cb);
}

void
AG_CheckboxInit(AG_Checkbox *cbox, char *caption)
{
	AG_WidgetInit(cbox, "checkbox", &agCheckboxOps, AG_WIDGET_FOCUSABLE);
	AG_WidgetBind(cbox, "state", AG_WIDGET_BOOL, &cbox->state);

	cbox->state = 0;
	cbox->label_su = AG_TextRender(NULL, -1, AG_COLOR(CHECKBOX_TXT_COLOR),
	    caption);
	cbox->label_id = AG_WidgetMapSurface(cbox, cbox->label_su);
	
	AG_SetEvent(cbox, "window-mousebuttondown", checkbox_mousebutton, NULL);
	AG_SetEvent(cbox, "window-keydown", checkbox_keydown, NULL);
}

void
AG_CheckboxDraw(void *p)
{
	AG_Checkbox *cbox = p;

	agPrim.box(cbox,
	    0, 0,
	    AGWIDGET(cbox)->h, AGWIDGET(cbox)->h,
	    AG_WidgetBool(cbox, "state") ? -1 : 1,
	    AG_COLOR(CHECKBOX_COLOR));

	AG_WidgetBlitSurface(cbox, cbox->label_id, AGWIDGET(cbox)->h+XSPACING, 0);
}

static void
checkbox_mousebutton(int argc, union evarg *argv)
{
	AG_Checkbox *cbox = argv[0].p;
	int button = argv[1].i;

	AG_WidgetFocus(cbox);

	if (button == SDL_BUTTON(1))
		AG_CheckboxToggle(cbox);
}

static void
checkbox_keydown(int argc, union evarg *argv)
{
	AG_Checkbox *cbox = argv[0].p;
	SDLKey key = argv[1].i;

	switch (key) {
	case SDLK_RETURN:
	case SDLK_SPACE:
		AG_CheckboxToggle(cbox);
		break;
	default:
		break;
	}
}

void
AG_CheckboxScale(void *p, int rw, int rh)
{
	AG_Checkbox *cb = p;

	if (rh == -1)
		AGWIDGET(cb)->h = cb->label_su->h;
	if (rw == -1)
		AGWIDGET(cb)->w = AGWIDGET(cb)->h + XSPACING + cb->label_su->w;
}

/* Toggle the checkbox state. */
void
AG_CheckboxToggle(AG_Checkbox *cbox)
{
	AG_WidgetBinding *stateb;
	int *state;

	stateb = AG_WidgetGetBinding(cbox, "state", &state);
	*state = !(*state);
	AG_PostEvent(NULL, cbox, "checkbox-changed", "%i", *state);
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
}

