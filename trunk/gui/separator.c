/*
 * Copyright (c) 2005-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "separator.h"
#include "window.h"
#include "primitive.h"

AG_Separator *
AG_SeparatorNew(void *parent, enum ag_separator_type type)
{
	AG_Separator *sep;

	sep = Malloc(sizeof(AG_Separator));
	AG_ObjectInit(sep, &agSeparatorClass);
	sep->type = type;
	sep->visible = 1;

	if (type == AG_SEPARATOR_HORIZ) {
		AG_ExpandHoriz(sep);
	} else {
		AG_ExpandVert(sep);
	}
	AG_ObjectAttach(parent, sep);
	return (sep);
}

AG_Separator *
AG_SpacerNew(void *parent, enum ag_separator_type type)
{
	AG_Separator *sep;

	sep = Malloc(sizeof(AG_Separator));
	AG_ObjectInit(sep, &agSeparatorClass);
	sep->type = type;
	sep->visible = 0;

	if (type == AG_SEPARATOR_HORIZ) {
		AG_ExpandHoriz(sep);
	} else {
		AG_ExpandVert(sep);
	}
	AG_ObjectAttach(parent, sep);
	return (sep);
}

static void
Init(void *obj)
{
	AG_Separator *sep = obj;

	sep->type = AG_SEPARATOR_HORIZ;
	sep->padding = 4;
	sep->visible = 1;
#ifdef AG_DEBUG
	AG_BindUint(sep, "type", &sep->type);
	AG_BindUint(sep, "padding", &sep->padding);
	AG_BindInt(sep, "visible", &sep->visible);
#endif
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Separator *sep = obj;

	r->w = sep->padding*2 + 2;
	r->h = sep->padding*2 + 2;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Separator *sep = obj;

	if (a->w < sep->padding*2 + 2 ||
	    a->h < sep->padding*2 + 2) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Separator *sep = obj;

	if (!sep->visible)
		return;
	
	switch (sep->type) {
	case AG_SEPARATOR_HORIZ:
		STYLE(sep)->SeparatorHoriz(sep);
		break;
	case AG_SEPARATOR_VERT:
		STYLE(sep)->SeparatorVert(sep);
		break;
	}
}

void
AG_SeparatorSetPadding(AG_Separator *sep, Uint pixels)
{
	AG_ObjectLock(sep);
	sep->padding = pixels;
	AG_ObjectUnlock(sep);
	AG_Redraw(sep);
}

AG_WidgetClass agSeparatorClass = {
	{
		"Agar(Widget:Separator)",
		sizeof(AG_Separator),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
