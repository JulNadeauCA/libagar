/*	$Csoft: matview.c,v 1.2 2005/09/11 07:05:58 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>

#include "matview.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static AG_WidgetOps matview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_MatviewDrawNumerical,
	AG_MatviewScale
};

AG_Matview *
AG_MatviewNew(void *parent, struct mat *mat, u_int flags)
{
	AG_Matview *mv;

	mv = Malloc(sizeof(AG_Matview), M_OBJECT);
	AG_MatviewInit(mv, mat, flags);
	AG_ObjectAttach(parent, mv);
	return (mv);
}

static void
matview_keydown(int argc, union evarg *argv)
{
	AG_Matview *mv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_g:
		AGWIDGET_OPS(mv)->draw = AG_MatviewDrawGreyscale;
		break;
	case SDLK_n:
		AGWIDGET_OPS(mv)->draw = AG_MatviewDrawNumerical;
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
matview_mousebuttondown(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;

	AG_WidgetFocus(bu);
}

void
AG_MatviewInit(AG_Matview *mv, struct mat *mat, u_int flags)
{
	AG_WidgetInit(mv, "matview", &matview_ops,
	    AG_WIDGET_WFILL|AG_WIDGET_HFILL|AG_WIDGET_CLIPPING|
	    AG_WIDGET_FOCUSABLE);
	mv->mat = mat;
	mv->flags = flags;
	mv->hspace = 2;
	mv->vspace = 2;
	mv->hbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_HORIZ);
	mv->vbar = AG_ScrollbarNew(mv, AG_SCROLLBAR_VERT);
	mv->xoffs = 0;
	mv->yoffs = 0;
	mv->scale = 4;
	mv->pre_m = mat->m;
	mv->pre_n = mat->n;
	mv->numfmt = "%g";
	
	AG_WidgetBind(mv->hbar, "value", AG_WIDGET_INT, &mv->xoffs);
	AG_WidgetBind(mv->vbar, "value", AG_WIDGET_INT, &mv->yoffs);
	AG_WidgetBind(mv->hbar, "max", AG_WIDGET_INT, &mv->mat->m);
	AG_WidgetBind(mv->vbar, "max", AG_WIDGET_INT, &mv->mat->n);
	AG_WidgetSetInt(mv->hbar, "min", 0);
	AG_WidgetSetInt(mv->vbar, "min", 0);

	AG_TextPrescale("-00", &mv->ent_w, &mv->ent_h);
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
AG_MatviewPrescale(AG_Matview *mv, const char *text, u_int m, u_int n)
{
	mv->pre_m = m;
	mv->pre_n = n;
	AG_TextPrescale(text, &mv->ent_w, &mv->ent_h);
}

void
AG_MatviewScale(void *p, int w, int h)
{
	AG_Matview *mv = p;

	if (w == -1 && h == -1) {
		AGWIDGET(mv)->w = mv->pre_n*(mv->ent_w + mv->hspace) +
		    mv->hspace*2;
		AGWIDGET(mv)->h = mv->pre_m*(mv->ent_h + mv->vspace) +
		    mv->vspace*2;
		return;
	}

	AGWIDGET(mv->hbar)->x = 0;
	AGWIDGET(mv->hbar)->y = AGWIDGET(mv)->h - mv->hbar->button_size;
	AGWIDGET(mv->hbar)->w = AGWIDGET(mv)->w;
	AGWIDGET(mv->hbar)->h = mv->hbar->button_size;

	AGWIDGET(mv->vbar)->x = AGWIDGET(mv)->w - mv->vbar->button_size;
	AGWIDGET(mv->vbar)->y = mv->vbar->button_size;
	AGWIDGET(mv->vbar)->w = mv->vbar->button_size;
	AGWIDGET(mv->vbar)->h = AGWIDGET(mv)->h - mv->vbar->button_size;
}

void
AG_MatviewDrawNumerical(void *p)
{
	char text[8];
	AG_Matview *mv = p;
	struct mat *M = mv->mat;
	u_int m, n;
	SDL_Surface *su;
	int x, y;

	agPrim.box(mv, 0, 0, AGWIDGET(mv)->w, AGWIDGET(mv)->h, -1,
	    AG_COLOR(BG_COLOR));

	for (m = 0, y = -mv->yoffs*mv->ent_h;
	     m <= M->m && y < AGWIDGET(mv)->h;
	     m++, y += (mv->ent_h + mv->vspace)) {
		for (n = 0, x = -mv->xoffs*mv->ent_w;
		     n <= M->n && x < AGWIDGET(mv)->w;
		     n++, x += (mv->ent_w + mv->hspace)) {
			if (m == 0) {
				snprintf(text, sizeof(text), "%d", n);
			} else if (n == 0) {
				snprintf(text, sizeof(text), "%d", m);
			} else {
				agPrim.box(mv, x, y,
				    mv->ent_w, mv->ent_h, -1,
				    AG_COLOR(FRAME_COLOR));
				snprintf(text, sizeof(text), mv->numfmt,
				    M->mat[m][n]);
			}
			su = AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR),
			    text);
			AG_WidgetBlit(mv, su, x, y);
			SDL_FreeSurface(su);
		}
	}
}

void
AG_MatviewDrawGreyscale(void *p)
{
	AG_Matview *mv = p;
	struct mat *M = mv->mat;
	SDL_Surface *su;
	u_int m, n;
	int x, y;

	agPrim.box(mv, 0, 0, AGWIDGET(mv)->w, AGWIDGET(mv)->h, -1,
	    AG_COLOR(BG_COLOR));

	for (m = 1, y = -mv->yoffs*mv->scale;
	     m <= M->m && y < AGWIDGET(mv)->h;
	     m++, y += mv->scale) {
		for (n = 1, x = -mv->xoffs*mv->scale;
		     n <= M->n && x < AGWIDGET(mv)->w;
		     n++, x += mv->scale) {
		     	double dv = M->mat[m][n];
			SDL_Rect rd;
			Uint32 c;
			Uint8 v8;
			u_int vi;

			if (dv == HUGE_VAL) {
				c = SDL_MapRGB(agVideoFmt, 200, 0, 0);
			} else {
#if 0
				if (dv > 0.0) {
					vi = 30 + ((u_int)(dv) - 30);
					v8 = vi < 255 ? (Uint8)vi : 255;
					c = SDL_MapRGB(agVideoFmt, v8, v8, 0);
				} else {
					vi = 30 + ((u_int)(fabs(dv)) - 30);
					v8 = vi < 255 ? (Uint8)vi : 255;
					c = SDL_MapRGB(agVideoFmt, v8, 0, v8);
				}
#else
				c = (Uint32)dv;
#endif
			}
			if (dv != 0.0) {
				rd.x = AGWIDGET(mv)->cx+x;
				rd.y = AGWIDGET(mv)->cy+y;
				rd.w = mv->scale;
				rd.h = mv->scale;
				SDL_FillRect(agView->v, &rd, c);
			}
		}
	}
}

