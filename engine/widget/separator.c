/*	$Csoft: separator.c,v 1.3 2005/03/09 06:39:21 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <stdarg.h>

#include "separator.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

const struct widget_ops separator_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		widget_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	separator_draw,
	separator_scale
};

struct separator *
separator_new(void *parent, enum separator_type type)
{
	struct separator *sep;

	sep = Malloc(sizeof(struct separator), M_OBJECT);
	separator_init(sep, type);
	object_attach(parent, sep);
	return (sep);
}

void
separator_init(struct separator *sep, enum separator_type type)
{
	widget_init(sep, "separator", &separator_ops,
	    (type == SEPARATOR_HORIZ) ? WIDGET_WFILL : WIDGET_HFILL);
	sep->type = type;
}

void
separator_scale(void *p, int w, int h)
{
	struct separator *sep = p;

	if (w == -1 && h == -1) {
		WIDGET(sep)->w = 2;
		WIDGET(sep)->h = 2;
	}
}

void
separator_draw(void *p)
{
	struct separator *sep = p;

	switch (sep->type) {
	case SEPARATOR_HORIZ:
		primitives.hline(sep, 0, WIDGET(sep)->w, 0,
		    COLOR(SEPARATOR_LINE1_COLOR));
		primitives.hline(sep, 0, WIDGET(sep)->w, 1,
		    COLOR(SEPARATOR_LINE2_COLOR));
		break;
	case SEPARATOR_VERT:
		primitives.vline(sep, 0, 0, WIDGET(sep)->h,
		    COLOR(SEPARATOR_LINE1_COLOR));
		primitives.vline(sep, 1, 0, WIDGET(sep)->h,
		    COLOR(SEPARATOR_LINE2_COLOR));
		break;
	}
}

