/*
 * Copyright (c) 2010 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
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
#include <core/config.h>

#include "progress_bar.h"
#include "window.h"
#include "primitive.h"
#include "text.h"
#include "text_cache.h"

AG_ProgressBar *
AG_ProgressBarNew(void *parent, enum ag_progress_bar_type type, Uint flags)
{
	AG_ProgressBar *pb;

	pb = Malloc(sizeof(AG_ProgressBar));
	AG_ObjectInit(pb, &agProgressBarClass);
	pb->type = type;
	pb->flags |= flags;
	if (flags & AG_PROGRESS_BAR_HFILL) { AG_ExpandHoriz(pb); }
	if (flags & AG_PROGRESS_BAR_VFILL) { AG_ExpandVert(pb); }
	AG_ObjectAttach(parent, pb);
	return (pb);
}

AG_ProgressBar *
AG_ProgressBarNewInt(void *parent, enum ag_progress_bar_type type, Uint flags,
    int *val, int *min, int *max)
{
	AG_ProgressBar *pb = AG_ProgressBarNew(parent, type, flags);
	if (val != NULL) { AG_BindInt(pb, "value", val); }
	if (min != NULL) { AG_BindInt(pb, "min", min); }
	if (max != NULL) { AG_BindInt(pb, "max", max); }
	return (pb);
}

static void
Init(void *obj)
{
	AG_ProgressBar *pb = obj;

	WIDGET(pb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_TABLE_EMBEDDABLE;

	pb->type = AG_PROGRESS_BAR_HORIZ;
	pb->flags = 0;
	pb->value = 0;
	pb->min = 0;
	pb->max = 100;
	pb->width = 25;
	pb->length = 300;
	pb->pad = 2;
	pb->tCache = agTextCache ? AG_TextCacheNew(pb, 50, 10) : NULL;

	AG_BindInt(pb, "value", &pb->value);
	AG_BindInt(pb, "min", &pb->min);
	AG_BindInt(pb, "max", &pb->max);

	AG_RedrawOnChange(pb, 250, "value");
	AG_RedrawOnChange(pb, 1000, "min");
	AG_RedrawOnChange(pb, 1000, "max");

#ifdef AG_DEBUG
	AG_BindUint(pb, "flags", &pb->flags);
	AG_BindUint(pb, "type", &pb->type);
	AG_BindInt(pb, "width", &pb->width);
	AG_BindInt(pb, "length", &pb->length);
	AG_BindInt(pb, "pad", &pb->pad);
#endif
}

static void
Destroy(void *obj)
{
	AG_ProgressBar *pb = obj;

	if (pb->tCache != NULL)
		AG_TextCacheDestroy(pb->tCache);
}

void
AG_ProgressBarSetWidth(AG_ProgressBar *pb, int width)
{
	AG_ObjectLock(pb);
	pb->width = width;
	AG_ObjectUnlock(pb);
	AG_Redraw(pb);
}

void
AG_ProgressBarSetLength(AG_ProgressBar *pb, int length)
{
	AG_ObjectLock(pb);
	pb->length = length;
	AG_ObjectUnlock(pb);
	AG_Redraw(pb);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_ProgressBar *pb = obj;

	switch (pb->type) {
	case AG_PROGRESS_BAR_HORIZ:
		r->w = pb->length;
		r->h = pb->width;
		break;
	case AG_PROGRESS_BAR_VERT:
		r->w = pb->width;
		r->h = pb->length;
		break;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_ProgressBar *pb = obj;

	if (a->w < pb->width || a->h < pb->width) {
		return (-1);
	}
	return (0);
}

int
AG_ProgressBarPercent(AG_ProgressBar *pb)
{
	int min, max, val;

	AG_ObjectLock(pb);
	min = AG_GetInt(pb, "min");
	max = AG_GetInt(pb, "max");
	val = AG_GetInt(pb, "value");
	AG_ObjectUnlock(pb);

	if (val < min) { val = min; }
	if (val > max) { val = max; }
	
	return (val - min)*100/(max - min);
}

static void
Draw(void *obj)
{
	AG_ProgressBar *pb = obj;
	char pctText[32];
	AG_Rect rd;
	int min, max, val, wAvail;

	min = AG_GetInt(pb, "min");
	max = AG_GetInt(pb, "max");
	val = AG_GetInt(pb, "value");
	if (val < min) { val = min; }
	if (val > max) { val = max; }

	STYLE(pb)->ProgressBarBackground(pb);

	if ((max - min) <= 0) {
		return;
	}
	switch (pb->type) {
	case AG_PROGRESS_BAR_VERT:
		wAvail = WIDGET(pb)->h - pb->pad*2;
		rd.x = pb->pad;
		rd.y = pb->pad + (val - min)*wAvail/(max - min);
		rd.w = WIDGET(pb)->w - pb->pad*2;
		rd.h = WIDGET(pb)->h;
		break;
	case AG_PROGRESS_BAR_HORIZ:
	default:
		wAvail = WIDGET(pb)->w - pb->pad*2;
		rd.x = pb->pad;
		rd.y = pb->pad;
		rd.w = (val - min)*wAvail/(max - min);
		rd.h = WIDGET(pb)->h - pb->pad*2;
		break;
	}
	AG_DrawRect(pb, rd, agColors[PROGRESS_BAR_COLOR]);

	if (pb->flags & AG_PROGRESS_BAR_SHOW_PCT) {
		StrlcpyInt(pctText, AG_ProgressBarPercent(pb), sizeof(pctText));
		Strlcat(pctText, "%", sizeof(pctText));

		AG_PushTextState();
		AG_TextColor(agColors[TEXT_COLOR]);
		if (agTextCache) {
			int su = AG_TextCacheGet(pb->tCache, pctText);
			AG_WidgetBlitSurface(pb, su,
			    WIDTH(pb)/2  - WSURFACE(pb,su)->w/2,
			    HEIGHT(pb)/2 - WSURFACE(pb,su)->h/2);
		} else {
			AG_Surface *suTmp = AG_TextRender(pctText);
			AG_WidgetBlit(pb, suTmp,
			    WIDTH(pb)/2  - suTmp->w/2,
			    HEIGHT(pb)/2 - suTmp->h/2);
			AG_SurfaceFree(suTmp);
		}
		AG_PopTextState();
	}
}

AG_WidgetClass agProgressBarClass = {
	{
		"Agar(Widget:ProgressBar)",
		sizeof(AG_ProgressBar),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
