/*	$Csoft: tileview.c,v 1.9 2005/02/12 10:31:37 vedge Exp $	*/

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

#include <engine/widget/primitive.h>

#include <stdarg.h>

#include "tileview.h"

const struct widget_ops tileview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		tileview_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	tileview_draw,
	tileview_scale
};

enum {
	FRAME_COLOR,
	TILE1_COLOR,
	TILE2_COLOR,
	TEXTFG_COLOR,
	TEXTBG_COLOR
};

struct tileview *
tileview_new(void *parent, struct tileset *ts, struct tile *t, int flags)
{
	struct tileview *tv;

	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, t, flags);
	object_attach(parent, tv);
	return (tv);
}

static Uint32
zoomin_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom > 1000) {
		return (0);
	}
	tileview_set_zoom(tv, tv->zoom+20, 1);
	return (ival);
}

static Uint32
zoomout_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom < 100) {
		return (0);
	}
	tileview_set_zoom(tv, tv->zoom-20, 1);
	return (ival);
}

static __inline__ void
move_cursor(struct tileview *tv, int x, int y)
{
	tv->xms = x;
	tv->yms = y;
	tv->xsub = tv->xms%tv->pxsz;
	tv->ysub = tv->yms%tv->pxsz;
	tv->xms -= tv->xsub;
	tv->yms -= tv->ysub;
	tv->xms /= tv->pxsz;
	tv->yms /= tv->pxsz;
}

static __inline__ int
cursor_overlap(struct tileview_handle *th, int sx, int sy)
{
	return (sx >= th->x - 2 && sx <= th->x + 2 &&
	        sy >= th->y - 2 && sy <= th->y + 2);
}

static void
tileview_keydown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_EQUALS:
		timeout_set(&tv->zoom_to, zoomin_tick, NULL, 0);
		timeout_del(tv, &tv->zoom_to);
		timeout_add(tv, &tv->zoom_to, 10);
		zoomin_tick(tv, 0, NULL);
		break;
	case SDLK_MINUS:
		timeout_set(&tv->zoom_to, zoomout_tick, NULL, 0);
		timeout_del(tv, &tv->zoom_to);
		timeout_add(tv, &tv->zoom_to, 10);
		zoomout_tick(tv, 0, NULL);
		break;
	case SDLK_0:
	case SDLK_1:
		tileview_set_zoom(tv, 100, 1);
		break;
	case SDLK_r:
		tv->tile->flags |= TILE_DIRTY;
		break;
	}
}

static void
tileview_keyup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_EQUALS:
	case SDLK_MINUS:
		timeout_del(tv, &tv->zoom_to);
		break;
	}
}

static void
tileview_buttondown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct tileview_ctrl *ctrl;
	int sx, sy, i;

	widget_focus(tv);

	switch (button) {
	case SDL_BUTTON_LEFT:
		sx = (x - tv->xoffs)/tv->pxsz;
		sy = (y - tv->yoffs)/tv->pxsz;
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				struct tileview_handle *th = &ctrl->handles[i];
		
				if (cursor_overlap(th, sx, sy)) {
					th->enable = 1;
					tv->xorig = sx;
					tv->yorig = sy;
					break;
				}
			}
			if (i < ctrl->nhandles)
				break;
		}
		if (ctrl == NULL && tv->state == TILEVIEW_PIXMAP_EDIT) {
			struct tile_element *tel = tv->tv_pixmap.tel;
			struct pixmap *px = tel->tel_pixmap.px;

			if (sx >= tel->tel_pixmap.x &&
			    sx <= tel->tel_pixmap.x+px->su->w &&
			    sy >= tel->tel_pixmap.y &&
			    sy <= tel->tel_pixmap.y+px->su->h) {
				pixmap_mousebuttondown(tv, px,
				    sx - tel->tel_pixmap.x,
				    sy - tel->tel_pixmap.y,
				    button);
			}
		}
		break;
	case SDL_BUTTON_MIDDLE:
	case SDL_BUTTON_RIGHT:
		tv->scrolling++;
		break;
	case SDL_BUTTON_WHEELUP:
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom+5 : tv->zoom+100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		break;
	case SDL_BUTTON_WHEELDOWN:
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom-5 : tv->zoom-100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		break;
	}
}

static void
tileview_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;
	struct tileview_ctrl *ctrl;
	int i;

	if (button == SDL_BUTTON_RIGHT ||
	    button == SDL_BUTTON_MIDDLE)
		tv->scrolling = 0;

	switch (button) {
	case SDL_BUTTON_RIGHT:
	case SDL_BUTTON_MIDDLE:
		tv->scrolling = 0;
		break;
	case SDL_BUTTON_LEFT:
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				struct tileview_handle *th = &ctrl->handles[i];

				if (th->enable) {
					th->enable = 0;
					break;
				}
			}
			if (i < ctrl->nhandles)
				break;
		}
		if (ctrl == NULL && tv->state == TILEVIEW_PIXMAP_EDIT) {
			struct tile_element *tel = tv->tv_pixmap.tel;
			struct pixmap *px = tel->tel_pixmap.px;

			pixmap_mousebuttonup(tv, px,
			    tv->xms - tel->tel_pixmap.x,
			    tv->yms - tel->tel_pixmap.y,
			    button);
		}
		break;
	}
}

static __inline__ void
clamp_offsets(struct tileview *tv)
{
	int lim;

	if (tv->xoffs >
	   (lim = (WIDGET(tv)->w - TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	} else if (tv->xoffs <
	   (lim = (-tv->scaled->w + TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	}

	if (tv->yoffs >
	    (lim = (WIDGET(tv)->h - TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	} else if (tv->yoffs <
	    (lim = (-tv->scaled->h + TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	}
}

void
tileview_set_int(struct tileview_ctrl *ctrl, int nval, int v)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_INT_VAL:
		ctrl->vals[nval].i = v;
		break;
	case TILEVIEW_UINT_VAL:
		ctrl->vals[nval].ui = (u_int)v;
		break;
	case TILEVIEW_INT_PTR:
		*(int *)ctrl->vals[nval].p = v;
		break;
	case TILEVIEW_UINT_PTR:
		*(u_int *)ctrl->vals[nval].p = (u_int)v;
		break;
	default:
		fatal("cannot convert");
	}
}

void
tileview_set_float(struct tileview_ctrl *ctrl, int nval, float v)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = v;
		break;
	case TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = (double)v;
		break;
	case TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = v;
		break;
	case TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = (double)v;
		break;
	default:
		fatal("cannot convert");
	}
}

void
tileview_set_double(struct tileview_ctrl *ctrl, int nval, double v)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = (float)v;
		break;
	case TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = v;
		break;
	case TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = (float)v;
		break;
	case TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = v;
		break;
	default:
		fatal("cannot convert");
	}
}

static void
move_handle(struct tileview *tv, struct tileview_ctrl *ctrl, int nhandle,
    int x2, int y2)
{
	struct tileview_handle *th = &ctrl->handles[nhandle];
	int dx = x2 - tv->xorig;
	int dy = y2 - tv->yorig;
	int xoffs = 0;
	int yoffs = 0;

	if (dx == 0 && dy == 0)
		return;

	switch (ctrl->type) {
	case TILEVIEW_RECTANGLE:
		switch (nhandle) {
		case 0:
			tileview_set_int(ctrl, 0, x2);
			tileview_set_int(ctrl, 1, y2);
			break;
		case 1:
			{
				int cy = tileview_int(ctrl, 1);
				int ch = tileview_int(ctrl, 3);
				int nh = ch-dy;

				tileview_set_int(ctrl, 1, cy+dy);
				tileview_set_int(ctrl, 3, nh>=8 ? nh : 8);
				if (dy < 0 || dy > 0)
					yoffs = -dy;
			}
			break;
		case 2:
			{
				int ch = tileview_int(ctrl, 3);
				int nh = ch+dy;

				tileview_set_int(ctrl, 3, nh>=8 ? nh : 8);
			}
			break;
		case 3:
			{
				int cw = tileview_int(ctrl, 2);
				int nw = cw+dx;

				tileview_set_int(ctrl, 2, nw>=8 ? nw : 8);
			}
			break;
		case 4:
			{
				int cx = tileview_int(ctrl, 0);
				int cw = tileview_int(ctrl, 2);
				int nw = cw-dx;

				tileview_set_int(ctrl, 0, cx+dx);
				tileview_set_int(ctrl, 2, nw>=8 ? nw : 8);
				if (dx < 0 || dx > 0)
					xoffs = -dx;
			}
			break;
		}
		break;
	case TILEVIEW_POINT:
		tileview_set_int(ctrl, 0, tileview_int(ctrl, 0)+dx);
		tileview_set_int(ctrl, 1, tileview_int(ctrl, 1)+dy);
		break;
	default:
		break;
	}

	if (ctrl->event != NULL)
		event_post(NULL, tv, ctrl->event->name, "%i,%i", xoffs, yoffs);
}

static void
tileview_mousemotion(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	int state = argv[5].i;
	struct tileview_ctrl *ctrl;
	int sx, sy, i;

	if (tv->scrolling) {
		tv->xoffs += xrel;
		tv->yoffs += yrel;
		clamp_offsets(tv);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		return;
	}

	sx = x - tv->xoffs;
	sy = y - tv->yoffs;
	move_cursor(tv, sx, sy);
	sx /= tv->pxsz;
	sy /= tv->pxsz;

	TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
		for (i = 0; i < ctrl->nhandles; i++) {
			struct tileview_handle *th = &ctrl->handles[i];

			if (th->enable) {
				move_handle(tv, ctrl, i, sx, sy);
				break;
			} else {
				if (cursor_overlap(th, sx, sy)) {
					th->over = 1;
				} else {
					th->over = 0;
				}
			}
		}
		if (i < ctrl->nhandles)
			break;
	}
	if (ctrl == NULL && tv->state == TILEVIEW_PIXMAP_EDIT) {
		struct tile_element *tel = tv->tv_pixmap.tel;
		struct pixmap *px = tel->tel_pixmap.px;

		if (sx >= tel->tel_pixmap.x &&
		    sx <= tel->tel_pixmap.x+px->su->w &&
		    sy >= tel->tel_pixmap.y &&
		    sy <= tel->tel_pixmap.y+px->su->h) {
			pixmap_mousemotion(tv, px,
			    sx - tel->tel_pixmap.x,
			    sy - tel->tel_pixmap.y,
			    sx - tv->xorig,
			    sy - tv->yorig,
			    state);
		}
	}
	
	tv->xorig = sx;
	tv->yorig = sy;
}

static Uint32
autoredraw(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	tile_generate(tv->tile);
	view_scale_surface(tv->tile->su, tv->scaled->w, tv->scaled->h,
	    &tv->scaled);
	tv->tile->flags &= ~TILE_DIRTY;
	return (ival);
}

void
tileview_init(struct tileview *tv, struct tileset *ts, struct tile *tile,
    int flags)
{
	widget_init(tv, "tileview", &tileview_ops, WIDGET_WFILL|WIDGET_HFILL|
	                                           WIDGET_FOCUSABLE|
						   WIDGET_CLIPPING);
	widget_map_color(tv, FRAME_COLOR, "frame", 40, 40, 60, 255);
	widget_map_color(tv, TILE1_COLOR, "tile1", 140, 140, 140, 255);
	widget_map_color(tv, TILE2_COLOR, "tile2", 80, 80, 80, 255);
	widget_map_color(tv, TEXTBG_COLOR, "text-bg", 0, 0, 50, 255);
	widget_map_color(tv, TEXTFG_COLOR, "text-fg", 250, 250, 250, 255);

	tv->ts = ts;
	tv->tile = tile;
	tv->scaled = NULL;
	tv->xoffs = 0;
	tv->yoffs = 0;
	tv->scrolling = 0;
	tv->flags = flags;
	tv->state = TILEVIEW_TILE_EDIT;
	tv->edit_mode = 0;
	tv->c.r = 255;
	tv->c.g = 255;
	tv->c.b = 255;
	tv->c.a = 128;
	TAILQ_INIT(&tv->ctrls);

	widget_map_surface(tv, NULL);
	tileview_set_zoom(tv, 100, 0);

	timeout_set(&tv->redraw_to, autoredraw, NULL, 0);
	
	event_new(tv, "window-keydown", tileview_keydown, NULL);
	event_new(tv, "window-keyup", tileview_keyup, NULL);
	event_new(tv, "window-mousebuttonup", tileview_buttonup, NULL);
	event_new(tv, "window-mousebuttondown", tileview_buttondown, NULL);
	event_new(tv, "window-mousemotion", tileview_mousemotion, NULL);
}

#define INSERT_VALUE(vt,memb, type,arg) do {			\
	ctrl->valtypes[ctrl->nvals] = (vt);			\
	ctrl->vals[ctrl->nvals].memb = (type)va_arg(ap, arg);	\
	ctrl->nvals++;						\
} while (/*CONSTCOND*/0)

/* Create a graphically editable geometric control. */
struct tileview_ctrl *
tileview_insert_ctrl(struct tileview *tv, enum tileview_ctrl_type type,
    const char *fmt, ...)
{
	struct tileview_ctrl *ctrl;
	va_list ap;
	int i;

	ctrl = Malloc(sizeof(struct tileview_ctrl), M_WIDGET);
	ctrl->type = type;
	ctrl->r = 255;
	ctrl->g = 255;
	ctrl->b = 255;
	ctrl->a = 128;
	ctrl->vals = Malloc(sizeof(union tileview_val)*strlen(fmt), M_WIDGET);
	ctrl->valtypes = Malloc(sizeof(enum tileview_val_type)*strlen(fmt),
	    M_WIDGET);
	ctrl->nvals = 0;
	ctrl->event = NULL;
	TAILQ_INSERT_TAIL(&tv->ctrls, ctrl, ctrls);

	if (fmt == NULL)
		goto out;
	
	va_start(ap, fmt);
	for (; *fmt != '\0'; fmt++) {
		switch (*fmt) {
		case '*':
			switch (fmt[1]) {
			case 'i':
				INSERT_VALUE(TILEVIEW_INT_PTR, p, int *,
				    void *);
				break;
			case 'u':
				INSERT_VALUE(TILEVIEW_UINT_PTR, p, u_int *,
				    void *);
				break;
			case 'f':
				INSERT_VALUE(TILEVIEW_FLOAT_PTR, p, float *,
				    void *);
				break;
			case 'd':
				INSERT_VALUE(TILEVIEW_DOUBLE_PTR, p, double *,
				    void *);
				break;
			default:
				fatal("bad format");
			}
			fmt++;
			break;
		case 'i':
			INSERT_VALUE(TILEVIEW_INT_VAL, i, int, int);
			break;
		case 'u':
			INSERT_VALUE(TILEVIEW_UINT_VAL, ui, u_int, int);
			break;
		case 'f':
			INSERT_VALUE(TILEVIEW_FLOAT_VAL, f, float, double);
			break;
		case 'd':
			INSERT_VALUE(TILEVIEW_DOUBLE_VAL, d, double, double);
			break;
		default:
			break;
		}
	}
	va_end(ap);
out:
	switch (ctrl->type) {
	case TILEVIEW_POINT:
		ctrl->nhandles = 1;
		if (ctrl->nvals < 1)
			fatal("incomplete");
		break;
	case TILEVIEW_RECTANGLE:
		ctrl->nhandles = 5;
		if (ctrl->nvals < 4)
			fatal("incomplete");
		break;
	case TILEVIEW_CIRCLE:
		ctrl->nhandles = 2;
		if (ctrl->nvals < 2)
			fatal("incomplete");
		break;
	default:
		ctrl->nhandles = 0;
		break;
	}
	ctrl->handles = (ctrl->nhandles > 0) ?
	    Malloc(sizeof(struct tileview_handle)*ctrl->nhandles, M_WIDGET) :
	    NULL;
	for (i = 0; i < ctrl->nhandles; i++) {
		struct tileview_handle *th = &ctrl->handles[i];

		th->x = -1;
		th->y = -1;
		th->over = 0;
		th->enable = 0;
	}
	return (ctrl);
}

static void
tileview_free_ctrl(struct tileview_ctrl *ctrl)
{
	Free(ctrl->valtypes, M_WIDGET);
	Free(ctrl->vals, M_WIDGET);
	Free(ctrl, M_WIDGET);
}

void
tileview_remove_ctrl(struct tileview *tv, struct tileview_ctrl *ctrl)
{
	TAILQ_REMOVE(&tv->ctrls, ctrl, ctrls);
	tileview_free_ctrl(ctrl);
}

void
tileview_set_autoredraw(struct tileview *tv, int ena, int rate)
{
	if (ena) {
		timeout_add(tv, &tv->redraw_to, rate);
		dprintf("enabled autoredraw\n");
	} else {
		timeout_del(tv, &tv->redraw_to);
		dprintf("disabled autoredraw\n");
	}
}

void
tileview_set_zoom(struct tileview *tv, int z2, int adj_offs)
{
	struct tile *t = tv->tile;
	int z1 = tv->zoom;
	int pxsz1 = tv->pxsz;

	if (z2 < 100 ||
	    z2 > 1000)
		return;

	tv->zoom = z2;
	tv->pxsz = z2/100;
	if (tv->pxsz < 1)
		tv->pxsz = 1;

	tv->scaled = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    z2>=100 ? t->su->w*tv->pxsz : t->su->w*z2/100,
	    z2>=100 ? t->su->h*tv->pxsz : t->su->h*z2/100,
	    t->su->format->BitsPerPixel,
	    t->su->format->Rmask, t->su->format->Gmask,
	    t->su->format->Bmask, t->su->format->Amask);
	tv->scaled->format->alpha = t->su->format->alpha;
	tv->scaled->format->colorkey = t->su->format->colorkey;

	if (tv->scaled == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	widget_replace_surface(tv, 0, tv->scaled);

	if (adj_offs) {
		int dx, dy;
		if (z2 >= 100) {
			dx = (t->su->w*pxsz1 - t->su->w*tv->pxsz)/2;
			dy = (t->su->h*pxsz1 - t->su->h*tv->pxsz)/2;
		} else {
			dx = (t->su->w*z1 - t->su->w*z2)/100/2;
			dy = (t->su->h*z1 - t->su->h*z2)/100/2;
		}
		tv->xoffs += dx;
		tv->yoffs += dy;
		clamp_offsets(tv);
	}

	t->flags |= TILE_DIRTY;
}

void
tileview_scale(void *p, int rw, int rh)
{
	struct tileview *tv = p;
	int lim;

	if (rw == -1 && rh == -1) {
		WIDGET(tv)->w = tv->tile->su->w + TILEVIEW_MIN_W;
		WIDGET(tv)->h = tv->tile->su->h + TILEVIEW_MIN_H;
	} else {
		if (tv->xoffs >
		   (lim = (WIDGET(tv)->w - TILEVIEW_MIN_W))) {
			tv->xoffs = lim;
		}
		if (tv->yoffs >
		   (lim	= (WIDGET(tv)->h - TILEVIEW_MIN_H))) {
			tv->yoffs = lim;
		}
	}
}

void
tileview_pixel2i(struct tileview *tv, int x, int y)
{
	int x1 = WIDGET(tv)->cx + tv->xoffs + x*tv->pxsz;
	int y1 = WIDGET(tv)->cy + tv->yoffs + y*tv->pxsz;
	int dx, dy;

	for (dy = 0; dy < tv->pxsz; dy++) {
		for (dx = 0; dx < tv->pxsz; dx++) {
			int cx = x1+dx;
			int cy = y1+dy;

			if (cx < WIDGET(tv)->cx ||
			    cy < WIDGET(tv)->cy ||
			    cx > WIDGET(tv)->cx+WIDGET(tv)->w ||
			    cy > WIDGET(tv)->cy+WIDGET(tv)->h)
				continue;

			if (tv->c.a < 255) {
				view_alpha_blend(view->v, cx, cy,
				    tv->c.r, tv->c.g, tv->c.b, tv->c.a);
			} else {
				VIEW_PUT_PIXEL(view->v, cx, cy, tv->c.pc);
			}
		}
	}
}

static void
draw_status_text(struct tileview *tv, const char *label)
{
	SDL_Surface *su;

	/* XXX pointless colorkey blit */
	su = text_render(NULL, -1, WIDGET_COLOR(tv, TEXTFG_COLOR), label);
	primitives.rect_filled(tv,
	    (su->w >= WIDGET(tv)->w) ? 0 : (WIDGET(tv)->w - su->w - 2),
	    WIDGET(tv)->h - su->h - 2,
	    WIDGET(tv)->w,
	    WIDGET(tv)->h,
	    TEXTBG_COLOR);
	widget_blit(tv, su,
	    WIDGET(tv)->w - su->w - 1,
	    WIDGET(tv)->h - su->h - 1);
	SDL_FreeSurface(su);
}

void
tileview_color3i(struct tileview *tv, Uint8 r, Uint8 g, Uint8 b)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.pc = SDL_MapRGB(vfmt, r, g, b);
}

void
tileview_color4i(struct tileview *tv, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(vfmt, r, g, b, a);
}

void
tileview_alpha(struct tileview *tv, Uint8 a)
{
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(vfmt, tv->c.r, tv->c.g, tv->c.b, a);
}

void
tileview_rect2(struct tileview *tv, int x1, int y1, int w, int h)
{
	int x, y;

	for (y = y1; y < y1+h; y++)
		for (x = x1; x < x1+w; x++)
			tileview_pixel2i(tv, x, y);
}

void
tileview_circle2o(struct tileview *tv, int x0, int y0, int r)
{
	int v = 2*r - 1;
	int e = 0;
	int u = 1;
	int x = 0;
	int y = r;

	while (x < y) {
		tileview_pixel2i(tv, x0+x, y0+y);
		tileview_pixel2i(tv, x0+x, y0-y);
		tileview_pixel2i(tv, x0-x, y0+y);
		tileview_pixel2i(tv, x0-x, y0-y);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;

		tileview_pixel2i(tv, x0+y, y0+x);
		tileview_pixel2i(tv, x0+y, y0-x);
		tileview_pixel2i(tv, x0-y, y0+x);
		tileview_pixel2i(tv, x0-y, y0-x);
	}
	tileview_pixel2i(tv, x0-r, y0);
	tileview_pixel2i(tv, x0+r, y0);
}

void
tileview_rect2o(struct tileview *tv, int x1, int y1, int w, int h)
{
	int x, y;

	for (y = y1+1; y < y1+h; y++) {
		tileview_pixel2i(tv, x1, y);
		tileview_pixel2i(tv, x1+w, y);
	}
	for (x = x1; x < x1+w+1; x++) {
		tileview_pixel2i(tv, x, y1);
		tileview_pixel2i(tv, x, y1+h);
	}
}

int
tileview_int(struct tileview_ctrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_INT_VAL:
		return (ctrl->vals[nval].i);
	case TILEVIEW_INT_PTR:
		return (*(int *)ctrl->vals[nval].p);
	case TILEVIEW_UINT_VAL:
		return ((int)ctrl->vals[nval].ui);
	case TILEVIEW_UINT_PTR:
		return (*(u_int *)ctrl->vals[nval].p);
	default:
		fatal("cannot convert");
	}
}

float
tileview_float(struct tileview_ctrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_FLOAT_VAL:
		return (ctrl->vals[nval].f);
	case TILEVIEW_FLOAT_PTR:
		return (*(float *)ctrl->vals[nval].p);
	case TILEVIEW_DOUBLE_VAL:
		return ((float)ctrl->vals[nval].d);
	case TILEVIEW_DOUBLE_PTR:
		return ((float)(*(double *)ctrl->vals[nval].p));
	default:
		fatal("cannot convert");
	}
}

double
tileview_double(struct tileview_ctrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case TILEVIEW_FLOAT_VAL:
		return ((double)ctrl->vals[nval].f);
	case TILEVIEW_FLOAT_PTR:
		return ((double)(*(float *)ctrl->vals[nval].p));
	case TILEVIEW_DOUBLE_VAL:
		return (ctrl->vals[nval].d);
	case TILEVIEW_DOUBLE_PTR:
		return (*(double *)ctrl->vals[nval].p);
	default:
		fatal("cannot convert");
	}
}

static void
draw_handle(struct tileview *tv, struct tileview_handle *th)
{
	int x = th->x;
	int y = th->y;

	if (th->over) {
		tileview_color4i(tv, 200, 200, 200, 255);
	} else {
		tileview_color4i(tv, 110, 110, 110, 255);
	}
	tileview_rect2(tv, x-1, y-1, 3, 3);		/* Inner circle */

	tileview_color4i(tv, 255, 255, 255, 128);
	tileview_pixel2i(tv, x, y);			/* Origin */

	if (!th->enable)
		tileview_color4i(tv, 200, 200, 200, 200);
	else
		tileview_color4i(tv, 60, 60, 60, 200);
	
	tileview_pixel2i(tv, x-2, y+1);			/* Highlight left */
	tileview_pixel2i(tv, x-2, y);
	tileview_pixel2i(tv, x-2, y-1);
	tileview_pixel2i(tv, x-1, y-2);			/* Highlight top */
	tileview_pixel2i(tv, x,   y-2);
	tileview_pixel2i(tv, x+1, y-2);
	
	if (th->enable)
		tileview_color4i(tv, 200, 200, 200, 200);
	else
		tileview_color4i(tv, 60, 60, 60, 200);
		
	tileview_pixel2i(tv, x-1, y+2);			/* Occlusion bottom */
	tileview_pixel2i(tv, x,   y+2);
	tileview_pixel2i(tv, x+1, y+2);
	tileview_pixel2i(tv, x+2, y-1);
	tileview_pixel2i(tv, x+2, y);
	tileview_pixel2i(tv, x+2, y+1);
}

static void
draw_control(struct tileview *tv, struct tileview_ctrl *ctrl)
{
	int i;

	tileview_color4i(tv, ctrl->r, ctrl->g, ctrl->b, ctrl->a);

	switch (ctrl->type) {
	case TILEVIEW_RECTANGLE:
		{
			int x = tileview_int(ctrl, 0);
			int y = tileview_int(ctrl, 1);
			u_int w = tileview_uint(ctrl, 2);
			u_int h = tileview_uint(ctrl, 3);

			tileview_rect2o(tv, x-1, y-1, w+1, h+1);

			ctrl->handles[0].x = x - 1;
			ctrl->handles[0].y = y - 1;
			ctrl->handles[1].x = x + w/2;
			ctrl->handles[1].y = y - 1;
			ctrl->handles[2].x = x + w/2;
			ctrl->handles[2].y = y + h;
			ctrl->handles[3].x = x + w;
			ctrl->handles[3].y = y + h/2;
			ctrl->handles[4].x = x - 1;
			ctrl->handles[4].y = y + h/2;
		}
		break;
	case TILEVIEW_POINT:
		{
			int x = tileview_int(ctrl, 0);
			int y = tileview_int(ctrl, 1);

			tileview_circle2o(tv, x, y, 2);
			ctrl->handles[0].x = x;
			ctrl->handles[0].y = y;
		}
		break;
	case TILEVIEW_CIRCLE:
		{
			int x = tileview_int(ctrl, 0);
			int y = tileview_int(ctrl, 1);
			u_int r = tileview_uint(ctrl, 2);

			tileview_circle2o(tv, x, y, r);
			tileview_pixel2i(tv, x, y);

			ctrl->handles[0].x = x;
			ctrl->handles[0].y = y;
			ctrl->handles[1].x = x+r;
			ctrl->handles[1].y = y;
		}
		break;
	}

	for (i = 0; i < ctrl->nhandles; i++)
		draw_handle(tv, &ctrl->handles[i]);
}

void
tileview_draw(void *p)
{
	struct tileview *tv = p;
	struct tile *t = tv->tile;
	struct tileview_ctrl *ctrl;
	SDL_Rect rsrc, rdst;
	int dxoffs, dyoffs;
	int drawbg = 2;

	if (t->flags & TILE_DIRTY) {
		t->flags &= ~TILE_DIRTY;
		tile_generate(t);
		view_scale_surface(t->su, tv->scaled->w, tv->scaled->h,
		    &tv->scaled);
	}

	rsrc.x = 0;
	rsrc.y = 0;
	rsrc.w = tv->scaled->w;
	rsrc.h = tv->scaled->h;
	rdst.x = tv->xoffs;
	rdst.y = tv->yoffs;

	if (tv->xoffs > 0 &&
	    tv->xoffs + tv->scaled->w > WIDGET(tv)->w) {
		rsrc.w = WIDGET(tv)->w - tv->xoffs;
	} else if (tv->xoffs < 0 &&
	          -tv->xoffs < tv->scaled->w) {
		rdst.x = 0;
		rsrc.x = -tv->xoffs;
		rsrc.w = tv->scaled->w - (-tv->xoffs);
		if (rsrc.w > WIDGET(tv)->w) {
			rsrc.w = WIDGET(tv)->w;
			drawbg--;
		}
	}

	if (tv->yoffs > 0 &&
	    tv->yoffs + tv->scaled->h > WIDGET(tv)->h) {
		rsrc.h = WIDGET(tv)->h - tv->yoffs;
	} else if (tv->yoffs < 0 &&
	          -tv->yoffs < tv->scaled->h) {
		rdst.y = 0;
		rsrc.y = -tv->yoffs;
		rsrc.h = tv->scaled->h - (-tv->yoffs);
		if (rsrc.h > WIDGET(tv)->h) {
			rsrc.h = WIDGET(tv)->h;
			drawbg--;
		}
	}
	
	if (drawbg) {
		SDL_Rect rtiling;
	
		rtiling.x = 0;
		rtiling.y = 0;
		rtiling.w = WIDGET(tv)->w;
		rtiling.h = WIDGET(tv)->h;
		primitives.tiling(tv, rtiling, 9, 0, TILE1_COLOR, TILE2_COLOR);
	}

	widget_blit_from(tv, tv, 0, &rsrc, rdst.x, rdst.y);

	tileview_color4i(tv, 255, 255, 255, 128);
	tileview_pixel2i(tv, tv->xms, tv->yms);

	TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls)
		draw_control(tv, ctrl);

	if (tv->edit_mode) {
		char status[64];

		switch (tv->state) {
		case TILEVIEW_FEATURE_EDIT:
			strlcpy(status, _("Editing feature: "), sizeof(status));
			strlcat(status, tv->tv_feature.ft->name,
			    sizeof(status));
			draw_status_text(tv, status);
			break;
		case TILEVIEW_SKETCH_EDIT:
			strlcpy(status, _("Editing sketch: "), sizeof(status));
			strlcat(status, tv->tv_sketch.sk->name,
			    sizeof(status));
			draw_status_text(tv, status);
			break;
		case TILEVIEW_PIXMAP_EDIT:
			strlcpy(status, _("Editing pixmap: "), sizeof(status));
			strlcat(status, tv->tv_pixmap.px->name, sizeof(status));
			draw_status_text(tv, status);
			break;
		default:
			break;
		}
	}
}

void
tileview_destroy(void *p)
{
	struct tileview *tv = p;
	struct tileview_ctrl *ctrl, *nctrl;

	for (ctrl = TAILQ_FIRST(&tv->ctrls);
	     ctrl != TAILQ_END(&tv->ctrls);
	     ctrl = nctrl) {
		nctrl = TAILQ_NEXT(ctrl, ctrls);
		tileview_free_ctrl(ctrl);
	}
	widget_destroy(tv);
}

