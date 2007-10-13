/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "scrollable_text.h"
#include "window.h"
#include "primitive.h"

#include <stdarg.h>

AG_ScrollableText *
AG_ScrollableTextNew(void *parent, Uint flags)
{
	AG_ScrollableText *st;

	st = Malloc(sizeof(AG_ScrollableText), M_OBJECT);
	AG_ScrollableTextInit(st, flags);
	AG_ObjectAttach(parent, st);
	return (st);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 0;
	r->h = 0;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	return (0);
}

static void
Draw(void *p)
{
	AG_ScrollableText *st = p;

	if (st->text == NULL)
		return;

	AG_ScrollableDrawBegin(&st->sa);
	if (st->surface == -1) {
		st->surface = AG_WidgetMapSurface(st, AG_TextRender(st->text));
	}
	AG_WidgetBlitSurface(st, st->surface, 0, 0);
	AG_ScrollableDrawEnd(&st->sa);
}

void
AG_ScrollableTextInit(AG_ScrollableText *st, Uint flags)
{
	Uint wFlags = 0;

	if (flags & AG_SCROLLABLE_TEXT_HFILL) { wFlags |= AG_WIDGET_HFILL; }
	if (flags & AG_SCROLLABLE_TEXT_VFILL) { wFlags |= AG_WIDGET_VFILL; }

	AG_ScrollableInit(&st->sa, 0, &agScrollableTextOps);
	st->flags = flags;
	st->surface = -1;
	st->text = NULL;
	st->text_len = 0;
}

static void
Destroy(void *p)
{
	AG_ScrollableText *st = p;

	Free(st->text,0);
}

const AG_WidgetOps agScrollableTextOps = {
	{
		"AG_Widget:AG_Scrollable:AG_ScrollableText",
		sizeof(AG_ScrollableText),
		{ 0,0 },
		NULL,				/* init() */
		NULL,				/* reinit() */
		Destroy,
		NULL,				/* load() */
		NULL,				/* save() */
		NULL				/* edit() */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
