/*	$Csoft: matview.c,v 1.1 2005/09/11 02:33:45 vedge Exp $	*/

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

static struct widget_ops matview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	matview_draw_numerical,
	matview_scale
};

struct matview *
matview_new(void *parent, struct mat *mat, u_int flags)
{
	struct matview *mv;

	mv = Malloc(sizeof(struct matview), M_OBJECT);
	matview_init(mv, mat, flags);
	object_attach(parent, mv);
	return (mv);
}

static void
matview_keydown(int argc, union evarg *argv)
{
	struct matview *mv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_g:
		WIDGET_OPS(mv)->draw = matview_draw_greyscale;
		break;
	case SDLK_n:
		WIDGET_OPS(mv)->draw = matview_draw_numerical;
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
	struct button *bu = argv[0].p;

	widget_focus(bu);
}

void
matview_init(struct matview *mv, struct mat *mat, u_int flags)
{
	widget_init(mv, "matview", &matview_ops, WIDGET_WFILL|WIDGET_HFILL|
	                                         WIDGET_CLIPPING|
						 WIDGET_FOCUSABLE);
	mv->mat = mat;
	mv->flags = flags;
	mv->hspace = 2;
	mv->vspace = 2;
	mv->hbar = scrollbar_new(mv, SCROLLBAR_HORIZ);
	mv->vbar = scrollbar_new(mv, SCROLLBAR_VERT);
	mv->xoffs = 0;
	mv->yoffs = 0;
	mv->scale = 4;
	mv->pre_m = mat->m;
	mv->pre_n = mat->n;
	mv->numfmt = "%g";
	
	widget_bind(mv->hbar, "value", WIDGET_INT, &mv->xoffs);
	widget_bind(mv->vbar, "value", WIDGET_INT, &mv->yoffs);
	widget_bind(mv->hbar, "max", WIDGET_INT, &mv->mat->m);
	widget_bind(mv->vbar, "max", WIDGET_INT, &mv->mat->n);
	widget_set_int(mv->hbar, "min", 0);
	widget_set_int(mv->vbar, "min", 0);

	text_prescale("-00", &mv->ent_w, &mv->ent_h);
	event_new(mv, "window-keydown", matview_keydown, NULL);
	event_new(mv, "window-mousebuttondown", matview_mousebuttondown, NULL);
}

void
matview_set_numfmt(struct matview *mv, const char *fmt)
{
	mv->numfmt = fmt;
}

void
matview_prescale(struct matview *mv, const char *text, u_int m, u_int n)
{
	mv->pre_m = m;
	mv->pre_n = n;
	text_prescale(text, &mv->ent_w, &mv->ent_h);
}

void
matview_scale(void *p, int w, int h)
{
	struct matview *mv = p;

	if (w == -1 && h == -1) {
		WIDGET(mv)->w = mv->pre_n*(mv->ent_w + mv->hspace) +
		    mv->hspace*2;
		WIDGET(mv)->h = mv->pre_m*(mv->ent_h + mv->vspace) +
		    mv->vspace*2;
		return;
	}

	WIDGET(mv->hbar)->x = 0;
	WIDGET(mv->hbar)->y = WIDGET(mv)->h - mv->hbar->button_size;
	WIDGET(mv->hbar)->w = WIDGET(mv)->w;
	WIDGET(mv->hbar)->h = mv->hbar->button_size;

	WIDGET(mv->vbar)->x = WIDGET(mv)->w - mv->vbar->button_size;
	WIDGET(mv->vbar)->y = mv->vbar->button_size;
	WIDGET(mv->vbar)->w = mv->vbar->button_size;
	WIDGET(mv->vbar)->h = WIDGET(mv)->h - mv->vbar->button_size;
}

void
matview_draw_numerical(void *p)
{
	char text[8];
	struct matview *mv = p;
	struct mat *M = mv->mat;
	u_int m, n;
	SDL_Surface *su;
	int x, y;

	primitives.box(mv, 0, 0, WIDGET(mv)->w, WIDGET(mv)->h, -1,
	    COLOR(BG_COLOR));

	for (m = 0, y = -mv->yoffs*mv->ent_h;
	     m <= M->m && y < WIDGET(mv)->h;
	     m++, y += (mv->ent_h + mv->vspace)) {
		for (n = 0, x = -mv->xoffs*mv->ent_w;
		     n <= M->n && x < WIDGET(mv)->w;
		     n++, x += (mv->ent_w + mv->hspace)) {
			if (m == 0) {
				snprintf(text, sizeof(text), "%d", n);
			} else if (n == 0) {
				snprintf(text, sizeof(text), "%d", m);
			} else {
				primitives.box(mv, x, y,
				    mv->ent_w, mv->ent_h, -1,
				    COLOR(FRAME_COLOR));
				snprintf(text, sizeof(text), mv->numfmt,
				    M->mat[m][n]);
			}
			su = text_render(NULL, -1, COLOR(TEXT_COLOR), text);
			widget_blit(mv, su, x, y);
			SDL_FreeSurface(su);
		}
	}
}

void
matview_draw_greyscale(void *p)
{
	struct matview *mv = p;
	struct mat *M = mv->mat;
	SDL_Surface *su;
	u_int m, n;
	int x, y;

	primitives.box(mv, 0, 0, WIDGET(mv)->w, WIDGET(mv)->h, -1,
	    COLOR(BG_COLOR));

	for (m = 1, y = -mv->yoffs*mv->scale;
	     m <= M->m && y < WIDGET(mv)->h;
	     m++, y += mv->scale) {
		for (n = 1, x = -mv->xoffs*mv->scale;
		     n <= M->n && x < WIDGET(mv)->w;
		     n++, x += mv->scale) {
		     	double dv = M->mat[m][n];
			SDL_Rect rd;
			Uint32 c;
			Uint8 v8;
			u_int vi;

			if (dv == HUGE_VAL) {
				c = SDL_MapRGB(vfmt, 200, 0, 0);
			} else {
#if 0
				if (dv > 0.0) {
					vi = 30 + ((u_int)(dv) - 30);
					v8 = vi < 255 ? (Uint8)vi : 255;
					c = SDL_MapRGB(vfmt, v8, v8, 0);
				} else {
					vi = 30 + ((u_int)(fabs(dv)) - 30);
					v8 = vi < 255 ? (Uint8)vi : 255;
					c = SDL_MapRGB(vfmt, v8, 0, v8);
				}
#else
				c = (Uint32)dv;
#endif
			}
			if (dv != 0.0) {
				rd.x = WIDGET(mv)->cx+x;
				rd.y = WIDGET(mv)->cy+y;
				rd.w = mv->scale;
				rd.h = mv->scale;
				SDL_FillRect(view->v, &rd, c);
			}
		}
	}
}

