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

#include <core/core.h>
#include <core/view.h>

#include "separator.h"

#include "window.h"
#include "primitive.h"

const AG_WidgetOps agSeparatorOps = {
	{
		"AG_Widget:AG_Separator",
		sizeof(AG_Separator),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_SeparatorDraw,
	AG_SeparatorScale
};

AG_Separator *
AG_SeparatorNew(void *parent, enum ag_separator_type type)
{
	AG_Separator *sep;

	sep = Malloc(sizeof(AG_Separator), M_OBJECT);
	AG_SeparatorInit(sep, type);
	AG_ObjectAttach(parent, sep);
	return (sep);
}

void
AG_SeparatorInit(AG_Separator *sep, enum ag_separator_type type)
{
	AG_WidgetInit(sep, "separator", &agSeparatorOps,
	    (type == AG_SEPARATOR_HORIZ) ? AG_WIDGET_HFILL : AG_WIDGET_VFILL);
	sep->type = type;
	sep->padding = 4;
}

void
AG_SeparatorScale(void *p, int w, int h)
{
	AG_Separator *sep = p;

	if (w == -1 && h == -1) {
		AGWIDGET(sep)->w = sep->padding*2 + 2;
		AGWIDGET(sep)->h = sep->padding*2 + 2;
	}
}

void
AG_SeparatorDraw(void *p)
{
	AG_Separator *sep = p;

	switch (sep->type) {
	case AG_SEPARATOR_HORIZ:
		agPrim.hline(sep, 0, AGWIDGET(sep)->w, sep->padding,
		    AG_COLOR(SEPARATOR_LINE1_COLOR));
		agPrim.hline(sep, 0, AGWIDGET(sep)->w, sep->padding+1,
		    AG_COLOR(SEPARATOR_LINE2_COLOR));
		break;
	case AG_SEPARATOR_VERT:
		agPrim.vline(sep, sep->padding, 0, AGWIDGET(sep)->h,
		    AG_COLOR(SEPARATOR_LINE1_COLOR));
		agPrim.vline(sep, sep->padding+1, 0, AGWIDGET(sep)->h,
		    AG_COLOR(SEPARATOR_LINE2_COLOR));
		break;
	}
}

void
AG_SeparatorSetPadding(AG_Separator *sep, Uint pixels)
{
	sep->padding = pixels;
}
