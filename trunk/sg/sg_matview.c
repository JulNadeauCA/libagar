/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Viewer widget for 4x4 matrices used in SG.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"
#include "sg_matview.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static void DrawGreyscale(void *);
static void DrawNumerical(void *);

AG_Matview *
AG_MatviewNew(void *parent, SG_Matrix *mat, Uint flags)
{
	AG_Matview *mv;

	mv = Malloc(sizeof(AG_Matview), M_OBJECT);
	AG_MatviewInit(mv, mat, flags);
	AG_ObjectAttach(parent, mv);
	return (mv);
}

static void
matview_keydown(AG_Event *event)
{
	AG_Matview *mv = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_g:
		WIDGET_OPS(mv)->draw = DrawGreyscale;
		break;
	case SDLK_n:
		WIDGET_OPS(mv)->draw = DrawNumerical;
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
matview_mousebuttondown(AG_Event *event)
{
	AG_Button *bu = AG_SELF();

	AG_WidgetFocus(bu);
}

void
AG_MatviewInit(AG_Matview *mv, SG_Matrix *mat, Uint flags)
{
	static int max_m = 4, max_n = 4;

	AG_WidgetInit(mv, &agMatviewOps, AG_WIDGET_EXPAND|AG_WIDGET_CLIPPING|
	                                 AG_WIDGET_FOCUSABLE);
	mv->mat = mat;
	mv->flags = flags;
	mv->hspace = 2;
	mv->vspace = 2;
	mv->hbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_HORIZ, 0);
	mv->vbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_VERT, 0);
	mv->xoffs = 0;
	mv->yoffs = 0;
	mv->scale = 10;
	mv->pre_m = 4;
	mv->pre_n = 4;
	mv->numfmt = "%g";
	
	AG_WidgetBind(mv->hbar, "value", AG_WIDGET_INT, &mv->xoffs);
	AG_WidgetBind(mv->vbar, "value", AG_WIDGET_INT, &mv->yoffs);
	AG_WidgetBind(mv->hbar, "max", AG_WIDGET_INT, &max_m);
	AG_WidgetBind(mv->vbar, "max", AG_WIDGET_INT, &max_n);
	AG_WidgetSetInt(mv->hbar, "min", 0);
	AG_WidgetSetInt(mv->vbar, "min", 0);

	AG_TextSize("-00", &mv->ent_w, &mv->ent_h);
	AG_SetEvent(mv, "window-keydown", matview_keydown, NULL);
	AG_SetEvent(mv, "window-mousebuttondown", matview_mousebuttondown,
	    NULL);
}

void
AG_MatviewSetNumericalFmt(AG_Matview *mv, const char *fmt)
{
	mv->numfmt = fmt;
}

void
AG_MatviewSizeHint(AG_Matview *mv, const char *text, Uint m, Uint n)
{
	mv->pre_m = m;
	mv->pre_n = n;
	AG_TextSize(text, &mv->ent_w, &mv->ent_h);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Matview *mv = p;

	r->w = mv->pre_n*(mv->ent_w + mv->hspace) + mv->hspace*2;
	r->h = mv->pre_m*(mv->ent_h + mv->vspace) + mv->vspace*2;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Matview *mv = p;
	AG_SizeAlloc aChld;

	aChld.x = 0;
	aChld.y = a->h - mv->hbar->bw;
	aChld.w = a->w;
	aChld.h = mv->hbar->bw;
	AG_WidgetSizeAlloc(mv->hbar, &aChld);

	aChld.x = a->w - mv->vbar->bw;
	aChld.y = mv->vbar->bw;
	aChld.w = mv->vbar->bw;
	aChld.h = a->h - mv->hbar->bw;
	AG_WidgetSizeAlloc(mv->vbar, &aChld);
	return (0);
}

static void
DrawNumerical(void *p)
{
	char text[8];
	AG_Matview *mv = p;
	SG_Matrix *M = mv->mat;
	int m, n;
	SDL_Surface *su;
	int x, y;

	agPrim.box(mv, 0, 0, WIDGET(mv)->w, WIDGET(mv)->h, -1,
	    AG_COLOR(BG_COLOR));

	for (m = -1, y = -mv->yoffs*mv->ent_h;
	     m < 4 && y < WIDGET(mv)->h;
	     m++, y += (mv->ent_h + mv->vspace)) {
		for (n = -1, x = -mv->xoffs*mv->ent_w;
		     n < 4 && x < WIDGET(mv)->w;
		     n++, x += (mv->ent_w + mv->hspace)) {
			if (m == -1) {
				snprintf(text, sizeof(text), "%d", n);
			} else if (n == -1) {
				snprintf(text, sizeof(text), "%d", m);
			} else {
				agPrim.box(mv, x, y,
				    mv->ent_w, mv->ent_h, -1,
				    AG_COLOR(FRAME_COLOR));
				snprintf(text, sizeof(text), mv->numfmt,
				    M->m[m][n]);
			}
			AG_TextColor(TEXT_COLOR);
			su = AG_TextRender(text);
			AG_WidgetBlit(mv, su, x, y);
			SDL_FreeSurface(su);
		}
	}
}

static void
DrawGreyscale(void *p)
{
	AG_Matview *mv = p;
	SG_Matrix *A = mv->mat;
	Uint m, n;
	int x, y;
	SG_Real big = 0.0, small = 0.0;

	agPrim.box(mv, 0, 0, WIDGET(mv)->w, WIDGET(mv)->h, -1,
	    AG_COLOR(BG_COLOR));

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++) {
			if (A->m[m][n] > big) { big = A->m[m][n]; }
			if (A->m[m][n] < small) { small = A->m[m][n]; }
		}
	}

	for (m = 0, y = -mv->yoffs*mv->scale;
	     m < 4 && y < WIDGET(mv)->h;
	     m++, y += mv->scale) {
		for (n = 0, x = -mv->xoffs*mv->scale;
		     n < 4 && x < WIDGET(mv)->w;
		     n++, x += mv->scale) {
		     	SG_Real dv = A->m[m][n];
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

const AG_WidgetOps agMatviewOps = {
	{
		"AG_Widget:AG_Matview",
		sizeof(AG_Matview),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	DrawNumerical,
	SizeRequest,
	SizeAllocate
};

#endif /* HAVE_OPENGL */
