/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Simple cosmetic separator / spacer widget.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/separator.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

AG_Separator *
AG_SeparatorNewHoriz(void *parent)
{
	return AG_SeparatorNew(parent, AG_SEPARATOR_HORIZ);
}

AG_Separator *
AG_SeparatorNewVert(void *parent)
{
	return AG_SeparatorNew(parent, AG_SEPARATOR_VERT);
}

AG_Separator *
AG_SeparatorNew(void *parent, enum ag_separator_type type)
{
	AG_Separator *sep;

	sep = Malloc(sizeof(AG_Separator));
	AG_ObjectInit(sep, &agSeparatorClass);

	if (type == AG_SEPARATOR_HORIZ) {
		WIDGET(sep)->flags |= AG_WIDGET_HFILL;
	} else {
		WIDGET(sep)->flags |= AG_WIDGET_VFILL;
	}

	sep->type = type;

	AG_ObjectAttach(parent, sep);
	return (sep);
}

AG_Separator *
AG_SpacerNewHoriz(void *parent)
{
	return AG_SpacerNew(parent, AG_SEPARATOR_HORIZ);
}

AG_Separator *
AG_SpacerNewVert(void *parent)
{
	return AG_SpacerNew(parent, AG_SEPARATOR_VERT);
}

AG_Separator *
AG_SpacerNew(void *parent, enum ag_separator_type type)
{
	AG_Separator *sep;

	sep = Malloc(sizeof(AG_Separator));
	AG_ObjectInit(sep, &agSeparatorClass);

	WIDGET(sep)->flags |= AG_WIDGET_HIDE;

	sep->type = type;

	if (type == AG_SEPARATOR_HORIZ) {
		WIDGET(sep)->flags |= AG_WIDGET_HFILL;
	} else {
		WIDGET(sep)->flags |= AG_WIDGET_VFILL;
	}

	AG_ObjectAttach(parent, sep);
	return (sep);
}

static void
Init(void *_Nonnull obj)
{
	AG_Separator *sep = obj;

	sep->type = AG_SEPARATOR_HORIZ;
	sep->minLen = 0;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Separator *sep = obj;

	if (sep->minLen > 0) {
		switch (sep->type) {
		case AG_SEPARATOR_HORIZ:
			r->w = sep->minLen;
			r->h = WIDGET(sep)->paddingTop +
		               WIDGET(sep)->paddingBottom;
			break;
		case AG_SEPARATOR_VERT:
			r->w = WIDGET(sep)->paddingLeft +
			       WIDGET(sep)->paddingRight;
			r->h = sep->minLen;
			break;
		}
	} else {
		switch (sep->type) {
		case AG_SEPARATOR_HORIZ:
			r->w = WIDGET(sep)->paddingLeft + WIDGET(sep)->paddingRight;
			r->h = WIDGET(sep)->paddingTop + 2 + WIDGET(sep)->paddingBottom;
			break;
		case AG_SEPARATOR_VERT:
			r->w = WIDGET(sep)->paddingLeft + 2 + WIDGET(sep)->paddingRight;
			r->h = WIDGET(sep)->paddingTop + WIDGET(sep)->paddingBottom;
			break;
		}
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Separator *sep = obj;

	if (a->w < WIDGET(sep)->paddingLeft + WIDGET(sep)->paddingRight ||
	    a->h < WIDGET(sep)->paddingTop + WIDGET(sep)->paddingBottom) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Separator *sep = obj;
	AG_Color c1 = WCOLOR(sep, LINE_COLOR);
	AG_Color c2 = c1;

	AG_ColorDarken(&c1, 2);
	AG_ColorLighten(&c2, 3);

	switch (sep->type) {
	case AG_SEPARATOR_HORIZ:
		AG_DrawLineH(sep,
		    WIDGET(sep)->paddingLeft,                /* x1 */
		    WIDTH(sep) - WIDGET(sep)->paddingRight,  /* x2 */
		    WIDGET(sep)->paddingTop,                 /* y */
		    &c1);
		AG_DrawLineH(sep,
		    WIDGET(sep)->paddingLeft,
		    WIDTH(sep) - WIDGET(sep)->paddingRight,
		    WIDGET(sep)->paddingTop+1,
		    &c2);
		break;
	case AG_SEPARATOR_VERT:
		AG_DrawLineV(sep,
		    WIDGET(sep)->paddingLeft,                 /* x */
		    WIDGET(sep)->paddingTop,                  /* y1 */
		    HEIGHT(sep) - WIDGET(sep)->paddingBottom, /* y2 */
		    &c1);
		AG_DrawLineV(sep,
		    WIDGET(sep)->paddingLeft+1,
		    WIDGET(sep)->paddingTop,
		    HEIGHT(sep) - WIDGET(sep)->paddingBottom,
		    &c2);
		break;
	}
}

void
AG_SeparatorSetLength(AG_Separator *sep, Uint minLen)
{
	AG_OBJECT_ISA(sep, "AG_Widget:AG_Separator:*");
	sep->minLen = minLen;
	AG_Redraw(sep);
}

#ifdef AG_LEGACY
void
AG_SeparatorSetPadding(AG_Separator *sep, Uint pixels)
{
	AG_SetStyleF(sep, "padding", "%d", pixels);
}
#endif

AG_WidgetClass agSeparatorClass = {
	{
		"Agar(Widget:Separator)",
		sizeof(AG_Separator),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
