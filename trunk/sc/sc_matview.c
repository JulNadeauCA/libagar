/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "sc_matview.h"

#include <gui/window.h>
#include <gui/button.h>
#include <gui/primitive.h>
#include <gui/text_cache.h>

SC_Matview *
SC_MatviewNew(void *parent, SC_Matrix *mat, Uint flags)
{
	SC_Matview *mv;

	mv = Malloc(sizeof(SC_Matview));
	AG_ObjectInit(mv, &scMatviewClass);
	mv->flags |= flags;
	if (mat != NULL) {
		SC_MatviewSetMatrix(mv, mat);
	}
	AG_ObjectAttach(parent, mv);
	return (mv);
}

void
SC_MatviewSetMatrix(SC_Matview *mv, struct sc_matrix *mat)
{
	AG_ObjectLock(mv);
	mv->mat = mat;
	mv->pre_m = mat->m;
	mv->pre_n = mat->n;
	AG_WidgetBind(mv->hbar, "max", AG_WIDGET_INT, &mv->mat->m);
	AG_WidgetBind(mv->vbar, "max", AG_WIDGET_INT, &mv->mat->n);
	AG_ObjectUnlock(mv);
}

static void
KeyDown(AG_Event *event)
{
	SC_Matview *mv = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_g:
		SC_MatviewSetDisplayMode(mv, SC_MATVIEW_GREYSCALE);
		break;
	case SDLK_n:
		SC_MatviewSetDisplayMode(mv, SC_MATVIEW_NUMERICAL);
		break;
	case SDLK_EQUALS:
		mv->scale++;
		break;
	case SDLK_MINUS:
		if (mv->scale-1 >= 0) {
			mv->scale--;
		}
		break;
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();

	AG_WidgetFocus(bu);
}

static void
Init(void *obj)
{
	SC_Matview *mv = obj;

	WIDGET(mv)->flags |= AG_WIDGET_EXPAND|AG_WIDGET_CLIPPING|
	                     AG_WIDGET_FOCUSABLE;

	mv->mode = SC_MATVIEW_NUMERICAL;
	mv->mat = NULL;
	mv->flags = 0;
	mv->hspace = 2;
	mv->vspace = 2;
	mv->hbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_HORIZ, 0);
	mv->vbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_VERT, 0);
	mv->xoffs = 0;
	mv->yoffs = 0;
	mv->scale = 4;
	mv->pre_m = 0;
	mv->pre_n = 0;
	mv->numfmt = "%g";
	mv->tCache = AG_TextCacheNew(mv, 64, 16);
	
	AG_WidgetBind(mv->hbar, "value", AG_WIDGET_INT, &mv->xoffs);
	AG_WidgetBind(mv->vbar, "value", AG_WIDGET_INT, &mv->yoffs);
	AG_WidgetSetInt(mv->hbar, "min", 0);
	AG_WidgetSetInt(mv->vbar, "min", 0);

	AG_TextSize("-00", &mv->ent_w, &mv->ent_h);

	AG_SetEvent(mv, "window-keydown", KeyDown, NULL);
	AG_SetEvent(mv, "window-mousebuttondown", MouseButtonDown, NULL);
}

static void
Destroy(void *obj)
{
	SC_Matview *mv = obj;

	AG_TextCacheDestroy(mv->tCache);
}

void
SC_MatviewSetDisplayMode(SC_Matview *mv, enum sc_matview_mode mode)
{
	AG_ObjectLock(mv);
	mv->mode = mode;
	AG_ObjectUnlock(mv);
}

void
SC_MatviewSetNumericalFmt(SC_Matview *mv, const char *fmt)
{
	AG_ObjectLock(mv);
	mv->numfmt = fmt;
	AG_ObjectUnlock(mv);
}

void
SC_MatviewSizeHint(SC_Matview *mv, const char *text, Uint m, Uint n)
{
	AG_ObjectLock(mv);
	mv->pre_m = m;
	mv->pre_n = n;
	AG_TextSize(text, &mv->ent_w, &mv->ent_h);
	AG_ObjectUnlock(mv);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	SC_Matview *mv = p;

	r->w = mv->pre_n*(mv->ent_w + mv->hspace) + mv->hspace*2;
	r->h = mv->pre_m*(mv->ent_h + mv->vspace) + mv->vspace*2;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	SC_Matview *mv = p;
	AG_SizeAlloc aChld;

	aChld.x = 0;
	aChld.y = a->h - mv->hbar->wButton;
	aChld.w = a->w;
	aChld.h = mv->hbar->wButton;
	AG_WidgetSizeAlloc(mv->hbar, &aChld);

	aChld.x = a->w - mv->vbar->wButton;
	aChld.y = mv->vbar->wButton;
	aChld.w = mv->vbar->wButton;
	aChld.h = a->h - mv->hbar->wButton;
	AG_WidgetSizeAlloc(mv->vbar, &aChld);
	return (0);
}

static void
DrawNumerical(void *p)
{
	char text[8];
	SC_Matview *mv = p;
	SC_Matrix *M = mv->mat;
	Uint m, n;
	int x, y;

	AG_DrawBox(mv,
	    AG_RECT(0, 0, WIDGET(mv)->w, WIDGET(mv)->h), -1,
	    AG_COLOR(BG_COLOR));

	AG_PushTextState();
	AG_TextColor(TEXT_COLOR);

	for (m = 0, y = -mv->yoffs*mv->ent_h;
	     m <= M->m && y < WIDGET(mv)->h;
	     m++, y += (mv->ent_h + mv->vspace)) {
		for (n = 0, x = -mv->xoffs*mv->ent_w;
		     n <= M->n && x < WIDGET(mv)->w;
		     n++, x += (mv->ent_w + mv->hspace)) {
			if (m == 0) {
				Snprintf(text, sizeof(text), "%d", n);
			} else if (n == 0) {
				Snprintf(text, sizeof(text), "%d", m);
			} else {
				AG_DrawBox(mv,
				    AG_RECT(x, y, mv->ent_w, mv->ent_h), -1,
				    AG_COLOR(FRAME_COLOR));
				Snprintf(text, sizeof(text), mv->numfmt,
				    M->mat[m][n]);
			}
			AG_WidgetBlitSurface(mv,
			    AG_TextCacheInsLookup(mv->tCache,text),
			    x, y);
		}
	}
	
	AG_PopTextState();
}

static void
DrawGreyscale(void *p)
{
	SC_Matview *mv = p;
	SC_Matrix *A = mv->mat;
	Uint m, n;
	int x, y;
	SC_Real big = 0.0, small = 0.0;

	AG_DrawBox(mv,
	    AG_RECT(0, 0, WIDGET(mv)->w, WIDGET(mv)->h), -1,
	    AG_COLOR(BG_COLOR));

	for (m = 1; m <= A->m; m++) {
		for (n = 1; n <= A->n; n++) {
			if (A->mat[m][n] > big) { big = A->mat[m][n]; }
			if (A->mat[m][n] < small) { small = A->mat[m][n]; }
		}
	}

	for (m = 1, y = -mv->yoffs*mv->scale;
	     m <= A->m && y < WIDGET(mv)->h;
	     m++, y += mv->scale) {
		for (n = 1, x = -mv->xoffs*mv->scale;
		     n <= A->n && x < WIDGET(mv)->w;
		     n++, x += mv->scale) {
		     	SC_Real dv = A->mat[m][n];
			SDL_Rect rd;
			Uint32 c;
			Uint8 v;

			if (dv == HUGE_VAL) {
				c = SDL_MapRGB(agVideoFmt, 200, 0, 0);
			} else {
				if (dv >= 0.0) {
					v = 128 + (Uint8)(dv*128.0/big);
					c = SDL_MapRGB(agVideoFmt, v, 0, 0);
				} else {
					v = 128 + (Uint8)((1.0/dv)*128.0 /
					    (1.0/small));
					c = SDL_MapRGB(agVideoFmt, 0, 0, v);
				}
			}
			if (dv != 0.0) {
				rd.x = WIDGET(mv)->cx+x;
				rd.y = WIDGET(mv)->cy+y;
				rd.w = mv->scale;
				rd.h = mv->scale;
				SDL_FillRect(agView->v, &rd, c);
			}
		}
	}
}

static void
Draw(void *obj)
{
	SC_Matview *mv = obj;

	if (mv->mode == SC_MATVIEW_NUMERICAL) {
		DrawNumerical(mv);
	} else {
		DrawGreyscale(mv);
	}
}

AG_WidgetClass scMatviewClass = {
	{
		"AG_Widget:SC_Matview",
		sizeof(SC_Matview),
		{ 0,0 },
		Init,
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

