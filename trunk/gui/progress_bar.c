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

#include <agar/core/core.h>
#include <agar/gui/progress_bar.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/text_cache.h>

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
OnShow(AG_Event *event)
{
	AG_ProgressBar *pb = AG_SELF();

	if ((pb->flags & AG_PROGRESS_BAR_EXCL) == 0) {
		AG_RedrawOnChange(pb, 250, "value");
		AG_RedrawOnChange(pb, 1000, "min");
		AG_RedrawOnChange(pb, 1000, "max");
	}
}

static void
OnFontChange(AG_Event *event)
{
	AG_ProgressBar *pb = AG_SELF();

	pb->width = WIDGET(pb)->font->height+10;
}

static void
Init(void *obj)
{
	AG_ProgressBar *pb = obj;

	WIDGET(pb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_TABLE_EMBEDDABLE|
			     AG_WIDGET_USE_TEXT;

	pb->type = AG_PROGRESS_BAR_HORIZ;
	pb->flags = 0;
	pb->value = 0;
	pb->min = 0;
	pb->max = 100;
	pb->width = agTextFontHeight+10;
	pb->length = 300;
	pb->pad = 2;
	pb->tCache = AG_TextCacheNew(pb, 100, 1);
	
	AG_BindInt(pb, "value", &pb->value);
	AG_BindInt(pb, "min", &pb->min);
	AG_BindInt(pb, "max", &pb->max);
	
	AG_AddEvent(pb, "font-changed", OnFontChange, NULL);
	AG_AddEvent(pb, "widget-shown", OnShow, NULL);

	AG_SetString(pb, "font-size", "90%");
}

static void
Destroy(void *obj)
{
	AG_ProgressBar *pb = obj;

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
DrawPercentText(AG_ProgressBar *pb)
{
	char s[32];
	int su;

	StrlcpyInt(s, AG_ProgressBarPercent(pb), sizeof(s));
	Strlcat(s, "%", sizeof(s));

	if ((su = AG_TextCacheGet(pb->tCache, s)) != -1) {
		AG_Surface *s = WSURFACE(pb,su);

		if (HEIGHT(pb) >= s->h && s->w <= WIDTH(pb)) {
			AG_WidgetBlitSurface(pb, su,
			    WIDTH(pb)/2  - s->w/2,
			    HEIGHT(pb)/2 - s->h/2);
		}
	}
}

static void
Draw(void *obj)
{
	AG_ProgressBar *pb = obj;
	AG_Rect rd;
	int min, max, val, wAvail;

	min = AG_GetInt(pb, "min");
	max = AG_GetInt(pb, "max");
	val = AG_GetInt(pb, "value");
	if (val < min) { val = min; }
	if (val > max) { val = max; }

	AG_DrawBox(pb,
	    AG_RECT(0, 0, WIDTH(pb), HEIGHT(pb)), -1,
	    WCOLOR(pb,0));

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
	AG_DrawRect(pb, rd, WCOLOR_SEL(pb,0));

	if (pb->flags & AG_PROGRESS_BAR_SHOW_PCT)
		DrawPercentText(pb);
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
