/*	$Csoft: checkbox.c,v 1.39 2003/05/24 15:53:44 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/vsnprintf.h>
#include <engine/compat/strlcpy.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/primitive.h>
#include <engine/widget/text.h>
#include <engine/widget/region.h>

#include "checkbox.h"

#include <engine/widget/window.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops checkbox_ops = {
	{
		NULL,		/* init */
		checkbox_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	checkbox_draw,
	NULL		/* update */
};

enum {
	PADDING =	6
};

enum {
	BOX_COLOR,
	TEXT_COLOR
};

static void	checkbox_scaled(int , union evarg *);
static void	checkbox_event(int , union evarg *);

struct checkbox *
checkbox_new(struct region *reg, const char *fmt, ...)
{
	char caption[CHECKBOX_CAPTION_MAX];
	struct checkbox *cb;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(caption, sizeof(caption), fmt, ap);
	va_end(ap);

	cb = Malloc(sizeof(struct checkbox));
	checkbox_init(cb, caption);
	region_attach(reg, cb);
	return (cb);
}

void
checkbox_init(struct checkbox *cbox, char *caption)
{
	widget_init(&cbox->wid, "checkbox", &checkbox_ops, -1, -1);
	WIDGET(cbox)->flags |= WIDGET_CLIPPING;
	widget_map_color(cbox, TEXT_COLOR, "text", 250, 250, 250);
	widget_map_color(cbox, BOX_COLOR, "box", 100, 100, 100);

	widget_bind(cbox, "state", WIDGET_BOOL, NULL, &cbox->def.state);
	cbox->def.state = 0;
	
	strlcpy(cbox->caption, caption, sizeof(cbox->caption));
	cbox->label_s = text_render(NULL, -1, WIDGET_COLOR(cbox, TEXT_COLOR),
	    caption);
	
	event_new(cbox, "window-mousebuttondown", checkbox_event, "%i",
	    WINDOW_MOUSEBUTTONDOWN);
	event_new(cbox, "window-keydown", checkbox_event, "%i", WINDOW_KEYDOWN);
	event_new(cbox, "widget-scaled", checkbox_scaled, NULL);
}

void
checkbox_destroy(void *p)
{
	struct checkbox *cbox = p;

	SDL_FreeSurface(cbox->label_s);
	widget_destroy(cbox);
}

void
checkbox_draw(void *p)
{
	struct checkbox *cbox = p;

	primitives.box(cbox, 0, 0, WIDGET(cbox)->h, WIDGET(cbox)->h,
	    widget_get_bool(cbox, "state") ? -1 : 1,
	    WIDGET_COLOR(cbox, BOX_COLOR));
	widget_blit(cbox, cbox->label_s, WIDGET(cbox)->h + PADDING, 0);
}

static void
checkbox_event(int argc, union evarg *argv)
{
	struct checkbox *cbox = argv[0].p;
	struct widget_binding *stateb;
	int *state;

	switch (argv[1].i) {
	case WINDOW_MOUSEBUTTONDOWN:
		if (argv[2].i == SDL_BUTTON(1)) {
			goto changed;
		} else {
			WIDGET_FOCUS(cbox);
		}
		break;
	case WINDOW_KEYDOWN:
		if (argv[2].i == SDLK_RETURN || argv[2].i == SDLK_SPACE)
			goto changed;
		break;
	}
	return;
changed:
	stateb = widget_binding_get_locked(cbox, "state", &state);
	*state = !(*state);
	event_post(cbox, "checkbox-changed", "%i", *state);
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
}

static void
checkbox_scaled(int argc, union evarg *argv)
{
	struct checkbox *cb = argv[0].p;

	if (WIDGET(cb)->rh == -1)
		WIDGET(cb)->h = cb->label_s->h;
	if (WIDGET(cb)->rw == -1)
		WIDGET(cb)->w = cb->label_s->w + PADDING + WIDGET(cb)->h;
}

