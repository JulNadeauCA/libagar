/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

	switch (type) {
	case AG_PROGRESS_BAR_HORIZ:
		AG_ExpandHoriz(pb);
		break;
	case AG_PROGRESS_BAR_VERT:
		AG_ExpandVert(pb);
		break;
	default:
		break;
	}
	AG_ObjectAttach(parent, pb);
	return (pb);
}

AG_ProgressBar *
AG_ProgressBarNewInt(void *parent, enum ag_progress_bar_type type, Uint flags,
    int *val, int *min, int *max)
{
	AG_ProgressBar *pb = AG_ProgressBarNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindInt(pb, "value", val); }
	if (min != NULL) { AG_WidgetBindInt(pb, "min", min); }
	if (max != NULL) { AG_WidgetBindInt(pb, "max", max); }
	return (pb);
}

static void
Init(void *obj)
{
	AG_ProgressBar *pb = obj;

	WIDGET(pb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION;

	AG_WidgetBindInt(pb, "value", &pb->value);
	AG_WidgetBindInt(pb, "min", &pb->min);
	AG_WidgetBindInt(pb, "max", &pb->max);

	pb->type = AG_PROGRESS_BAR_HORIZ;
	pb->flags = 0;
	pb->value = 0;
	pb->min = 0;
	pb->max = 100;
	pb->width = 25;
	pb->pad = 2;
	pb->tCache = agTextCache ? AG_TextCacheNew(pb, 50, 10) : NULL;
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
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_ProgressBar *pb = obj;

	switch (pb->type) {
	case AG_PROGRESS_BAR_HORIZ:
		r->w = 32;
		r->h = pb->width;
		break;
	case AG_PROGRESS_BAR_VERT:
		r->w = pb->width;
		r->h = 32;
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
	min = AG_WidgetInt(pb, "min");
	max = AG_WidgetInt(pb, "max");
	val = AG_WidgetInt(pb, "value");
	AG_ObjectUnlock(pb);

	if (val < min) { val = min; }
	if (val > max) { val = max; }
	
	return (val - min)*100/(max - min);
}

static void
Draw(void *obj)
{
	AG_ProgressBar *pb = obj;
	int min, max, val;
	int wAvail;
	AG_Rect rd;

	min = AG_WidgetInt(pb, "min");
	max = AG_WidgetInt(pb, "max");
	val = AG_WidgetInt(pb, "value");
	if (val < min) { val = min; }
	if (val > max) { val = max; }

	STYLE(pb)->ProgressBarBackground(pb);

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
	AG_DrawRectFilled(pb, rd, AG_COLOR(PROGRESS_BAR_COLOR));

	if (pb->flags & AG_PROGRESS_BAR_SHOW_PCT) {
		char pctText[32];

		Snprintf(pctText, sizeof(pctText), "%d%%",
		    AG_ProgressBarPercent(pb));
		AG_PushTextState();
		AG_TextColor(TEXT_COLOR);
		if (agTextCache) {
			int su = AG_TextCacheInsLookup(pb->tCache, pctText);
			AG_WidgetBlitSurface(pb, su,
			    WIDTH(pb)/2  - WSURFACE(pb,su)->w/2,
			    HEIGHT(pb)/2 - WSURFACE(pb,su)->h/2);
		} else {
			SDL_Surface *suTmp = AG_TextRender(pctText);
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
