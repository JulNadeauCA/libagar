/*
 * Copyright (c) 2010-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Progress bar widget. It connects to integers representing minimum, maximum
 * and current values, and displays a progress bar (optionally with "%" label).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

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

	if (flags & AG_PROGRESS_BAR_HFILL) { WIDGET(pb)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PROGRESS_BAR_VFILL) { WIDGET(pb)->flags |= AG_WIDGET_VFILL; }
	pb->flags |= flags;

	AG_ObjectAttach(parent, pb);
	return (pb);
}

AG_ProgressBar *
AG_ProgressBarNewHoriz(void *parent, Uint flags)
{
	return AG_ProgressBarNew(parent, AG_PROGRESS_BAR_HORIZ, flags);
}

AG_ProgressBar *
AG_ProgressBarNewVert(void *parent, Uint flags)
{
	return AG_ProgressBarNew(parent, AG_PROGRESS_BAR_VERT, flags);
}

AG_ProgressBar *
AG_ProgressBarNewInt(void *parent, enum ag_progress_bar_type type, Uint flags,
    int *val, int *min, int *max)
{
	AG_ProgressBar *pb;
	
	pb = AG_ProgressBarNew(parent, type, flags);
	if (val != NULL) { AG_BindInt(pb, "value", val); }
	if (min != NULL) { AG_BindInt(pb, "min", min); }
	if (max != NULL) { AG_BindInt(pb, "max", max); }
	return (pb);
}
	
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_ProgressBar *pb = AG_PROGRESSBAR_SELF();

	if ((pb->flags & AG_PROGRESS_BAR_EXCL) == 0) {
		AG_RedrawOnChange(pb, 250, "value");
		AG_RedrawOnChange(pb, 1000, "min");
		AG_RedrawOnChange(pb, 1000, "max");
	}
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_ProgressBar *pb = AG_PROGRESSBAR_SELF();
	const AG_Font *font = WFONT(pb);

	pb->width = font->height + 10; /* XXX style */
	AG_TextCacheClear(pb->tCache);
}

static void
Init(void *_Nonnull obj)
{
	AG_ProgressBar *pb = obj;

	WIDGET(pb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP |
	                     AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_USE_TEXT;

	pb->type = AG_PROGRESS_BAR_HORIZ;
	pb->flags = 0;
	pb->value = 0;
	pb->min = 0;
	pb->max = 100;
	pb->width = agTextFontHeight+10; /* XXX style */
	pb->length = 300;
	pb->tCache = AG_TextCacheNew(pb, 100, 1);
	
	AG_BindInt(pb, "value", &pb->value);
	AG_BindInt(pb, "min", &pb->min);
	AG_BindInt(pb, "max", &pb->max);
	
	AG_AddEvent(pb, "font-changed", OnFontChange, NULL);
	AG_AddEvent(pb, "widget-shown", OnShow, NULL);
}

static void
Destroy(void *_Nonnull obj)
{
	AG_ProgressBar *pb = obj;

	AG_TextCacheDestroy(pb->tCache);
}

void
AG_ProgressBarSetWidth(AG_ProgressBar *pb, int width)
{
	AG_OBJECT_ISA(pb, "AG_Widget:AG_ProgressBar:*");
	pb->width = width;
	AG_Redraw(pb);
}

void
AG_ProgressBarSetLength(AG_ProgressBar *pb, int length)
{
	AG_OBJECT_ISA(pb, "AG_Widget:AG_ProgressBar:*");
	pb->length = length;
	AG_Redraw(pb);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_ProgressBar *pb = obj;

	r->w = WIDGET(pb)->paddingLeft + WIDGET(pb)->paddingRight;
	r->h = WIDGET(pb)->paddingTop + WIDGET(pb)->paddingBottom;

	switch (pb->type) {
	case AG_PROGRESS_BAR_HORIZ:
		r->w += pb->length;
		r->h += pb->width;
		break;
	case AG_PROGRESS_BAR_VERT:
		r->w += pb->width;
		r->h += pb->length;
		break;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	if (a->w < WIDGET(obj)->paddingLeft + 5 + WIDGET(obj)->paddingRight ||
	    a->h < WIDGET(obj)->paddingTop + 5 + WIDGET(obj)->paddingBottom) {
		return (-1);
	}
	return (0);
}

/* Return current value in %. */
int
AG_ProgressBarPercent(AG_ProgressBar *pb)
{
	int min, max, val;

	AG_OBJECT_ISA(pb, "AG_Widget:AG_ProgressBar:*");
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
DrawPercentText(AG_ProgressBar *_Nonnull pb)
{
	char s[32];
	int ent;

	StrlcpyInt(s, AG_ProgressBarPercent(pb), sizeof(s));
	Strlcat(s, "%", sizeof(s));

	if ((ent = AG_TextCacheGet(pb->tCache, s)) != -1) {
		const AG_Surface *S = WSURFACE(pb, ent);

		if (HEIGHT(pb) >= S->h && S->w <= WIDTH(pb)) {
			AG_PushBlendingMode(pb, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

			AG_WidgetBlitSurface(pb, ent,
			    (WIDTH(pb) >> 1)  - (S->w >> 1),
			    (HEIGHT(pb) >> 1) - (S->h >> 1));

			AG_PopBlendingMode(pb);
		}
	}
}

static void
Draw(void *_Nonnull obj)
{
	AG_ProgressBar *pb = obj;
	AG_Rect rd = WIDGET(pb)->r;
	const int paddingLeft   = WIDGET(pb)->paddingLeft;
	const int paddingRight  = WIDGET(pb)->paddingRight;
	const int paddingTop    = WIDGET(pb)->paddingTop;
	const int paddingBottom = WIDGET(pb)->paddingBottom;
	int min, max, val;

	min = AG_GetInt(pb, "min");
	max = AG_GetInt(pb, "max");
	val = AG_GetInt(pb, "value");
	if (val < min) { val = min; }
	if (val > max) { val = max; }

	AG_DrawBoxSunk(pb, &rd, &WCOLOR(pb,BG_COLOR));

	if ((max - min) <= 0)
		return;

	rd.x = paddingLeft;
	rd.y = paddingTop;

	switch (pb->type) {
	case AG_PROGRESS_BAR_VERT:
		rd.h -= (paddingTop + paddingBottom);
		rd.y += rd.h - (val-min)*(rd.h)/(max-min) - 1;
		rd.h = HEIGHT(pb) - paddingTop - rd.y + 1;
		rd.w -= (paddingLeft + paddingRight);
		break;
	case AG_PROGRESS_BAR_HORIZ:
	default:
		rd.w = (val-min)*(rd.w - paddingLeft - paddingRight)/(max-min);
		rd.h -= (paddingTop + paddingBottom);
		break;
	}
	AG_DrawRect(pb, &rd, &WCOLOR(pb,SELECTION_COLOR));

	if (pb->flags & AG_PROGRESS_BAR_SHOW_PCT)
		DrawPercentText(pb);
}

AG_WidgetClass agProgressBarClass = {
	{
		"Agar(Widget:ProgressBar)",
		sizeof(AG_ProgressBar),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
