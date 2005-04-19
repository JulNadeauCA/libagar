/*	$Csoft: tileview.c,v 1.31 2005/04/14 02:49:26 vedge Exp $	*/

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

struct tileview *
tileview_new(void *parent, struct tileset *ts, int flags)
{
	struct tileview *tv;

	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, flags);
	object_attach(parent, tv);
	return (tv);
}

static Uint32
zoomin_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom > 1600) {
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
keydown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;

	switch (tv->state) {
	case TILEVIEW_PIXMAP_EDIT:
		pixmap_keydown(tv, tv->tv_pixmap.tel, keysym, keymod);
		break;
	case TILEVIEW_SKETCH_EDIT:
		sketch_keydown(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
	default:
		break;
	}

	switch (keysym) {
	case SDLK_z:
		if (keymod & KMOD_CTRL) {
			switch (tv->state) {
			case TILEVIEW_PIXMAP_EDIT:
				pixmap_undo(tv, tv->tv_pixmap.tel);
				break;
			case TILEVIEW_SKETCH_EDIT:
				sketch_undo(tv, tv->tv_sketch.tel);
				break;
			default:
				break;
			}
		}
		break;
	case SDLK_r:
		if (keymod & KMOD_CTRL) {
			switch (tv->state) {
			case TILEVIEW_PIXMAP_EDIT:
				pixmap_redo(tv, tv->tv_pixmap.tel);
				break;
			case TILEVIEW_SKETCH_EDIT:
				sketch_redo(tv, tv->tv_sketch.tel);
				break;
			default:
				break;
			}
		} else {
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
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
	}
}

static void
keyup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	
	switch (tv->state) {
	case TILEVIEW_PIXMAP_EDIT:
		pixmap_keyup(tv, tv->tv_pixmap.tel, keysym, keymod);
		break;
	case TILEVIEW_SKETCH_EDIT:
		sketch_keyup(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
	default:
		break;
	}

	switch (keysym) {
	case SDLK_EQUALS:
	case SDLK_MINUS:
		timeout_del(tv, &tv->zoom_to);
		break;
	}
}

static __inline__ int
pixmap_coincident(struct tile_element *tel, int x, int y)
{
	return (x >= tel->tel_pixmap.x &&
	        x < tel->tel_pixmap.x + tel->tel_pixmap.px->su->w &&
	        y >= tel->tel_pixmap.y &&
	        y < tel->tel_pixmap.y + tel->tel_pixmap.px->su->h);
}

static __inline__ int
sketch_coincident(struct tile_element *tel, int x, int y)
{
	return (x >= tel->tel_sketch.x &&
	        x < tel->tel_sketch.x + tel->tel_sketch.sk->vg->su->w &&
	        y >= tel->tel_sketch.y &&
	        y < tel->tel_sketch.y + tel->tel_sketch.sk->vg->su->h);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct tileview_ctrl *ctrl;
	struct tile_element *tel = tv->tv_pixmap.tel;
	int sx = (x - tv->xoffs)/tv->pxsz;
	int sy = (y - tv->yoffs)/tv->pxsz;
	int i;

	widget_focus(tv);

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		if (tv->state == TILEVIEW_PIXMAP_EDIT) {
			if (pixmap_mousewheel(tv, tel, 0) == 1)
				return;
		} else if (tv->state == TILEVIEW_SKETCH_EDIT) {
			if (sketch_mousewheel(tv, tel, 0) == 1)
				return;
		}
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom+5 : tv->zoom+100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		return;
	case SDL_BUTTON_WHEELDOWN:
		if (tv->state == TILEVIEW_PIXMAP_EDIT) {
			if (pixmap_mousewheel(tv, tel, 1) == 1)
				return;
		} else if (tv->state == TILEVIEW_SKETCH_EDIT) {
			if (sketch_mousewheel(tv, tel, 1) == 1)
				return;
		}
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom-5 : tv->zoom-100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		break;
	default:
		break;
	}

	switch (tv->state) {
	case TILEVIEW_PIXMAP_EDIT:
		if (pixmap_coincident(tel, sx, sy)) {
			pixmap_mousebuttondown(tv, tel,
			    sx - tel->tel_pixmap.x,
			    sy - tel->tel_pixmap.y,
			    button);
			return;
		} else {
			if (button == SDL_BUTTON_RIGHT)
				tv->scrolling++;
		}
		break;
	case TILEVIEW_SKETCH_EDIT:
		if (button == SDL_BUTTON_RIGHT &&
		   (tv->flags & TILEVIEW_NO_SCROLLING) == 0) {
			tv->scrolling++;
		}
		if (button == SDL_BUTTON_MIDDLE || button == SDL_BUTTON_RIGHT ||
		   (button == SDL_BUTTON_LEFT &&
		    sketch_coincident(tel, sx, sy)))
		{
			struct sketch *sk = tv->tv_sketch.sk;
			double vx, vy;

			vg_vcoords2(sk->vg,
			    sx - tel->tel_sketch.x,
			    sy - tel->tel_sketch.y,
			    0, 0, &vx, &vy);
			sketch_mousebuttondown(tv, tel,
			    vx/TILESZ, vy/TILESZ,
			    button);
			return;
		}
		break;
	case TILEVIEW_FEATURE_EDIT:
		if (button == SDL_BUTTON_MIDDLE) {
			if (tv->tv_feature.ft->ops->menu != NULL) {
				feature_open_menu(tv,
				    WIDGET(tv)->cx + x,
				    WIDGET(tv)->cy + y);
			}
		} else if (button == SDL_BUTTON_RIGHT) {
			tv->scrolling++;
		}
		break;
	case TILEVIEW_TILE_EDIT:
		if (button == SDL_BUTTON_RIGHT) {
			tv->scrolling++;
		}
		break;
	}

	if (button == SDL_BUTTON_LEFT) {
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				struct tileview_handle *th = &ctrl->handles[i];
		
				if (cursor_overlap(th, sx, sy)) {
					th->enable = 1;
					tv->xorig = sx;
					tv->yorig = sy;
					ctrl->xoffs = 0;
					ctrl->yoffs = 0;
					if (ctrl->buttondown != NULL) {
						event_post(NULL, tv,
						    ctrl->buttondown->name,
						    "%i,%i", sx, sy);
					}
					break;
				}
			}
			if (i < ctrl->nhandles)
				break;
		}
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
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
					if (ctrl->buttonup != NULL) {
						event_post(NULL, tv,
						    ctrl->buttonup->name, NULL);
					}
					break;
				}
			}
			if (i < ctrl->nhandles)
				break;
		}
		if (ctrl == NULL) {
			switch (tv->state) {
			case TILEVIEW_PIXMAP_EDIT:
				{
					struct tile_element *tel =
					    tv->tv_pixmap.tel;
					struct pixmap *px = tel->tel_pixmap.px;

					pixmap_mousebuttonup(tv, tel,
					    tv->xms - tel->tel_pixmap.x,
					    tv->yms - tel->tel_pixmap.y,
					    button);
				}
				break;
			case TILEVIEW_SKETCH_EDIT:
				{
					struct tile_element *tel =
					    tv->tv_sketch.tel;
					struct sketch *sk = tel->tel_sketch.sk;
					double vx, vy;
				
					vg_vcoords2(tel->tel_sketch.sk->vg,
					    tv->xms - tel->tel_sketch.x,
					    tv->yms - tel->tel_sketch.y,
					    0, 0, &vx, &vy);

					sketch_mousebuttonup(tv, tel, vx, vy,
					    button);
				}
				break;
			default:
				break;
			}
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
			tileview_set_int(ctrl, 0,
			    tileview_int(ctrl,0)+dx);
			tileview_set_int(ctrl, 1,
			    tileview_int(ctrl,1)+dy);
			break;
		case 1:						/* Top */
			{
				int ch = tileview_int(ctrl, 3);
				int cy = tileview_int(ctrl, 1);
				int nh = ch-dy;

				tileview_set_int(ctrl, 1, cy+dy);
				tileview_set_int(ctrl, 3, nh>=1 ? nh : 1);
				if (dy < 0 || dy > 0)
					yoffs = -dy;
			}
			break;
		case 4:						/* Left */
			{
				int cw = tileview_int(ctrl, 2);
				int cx = tileview_int(ctrl, 0);
				int nw = cw-dx;


				tileview_set_int(ctrl, 0, cx+dx);
				tileview_set_int(ctrl, 2, nw>=1 ? nw : 1);
				if (dx < 0 || dx > 0)
					xoffs = -dx;
			}
			break;
		}
		/* FALLTHROUGH */
	case TILEVIEW_RDIMENSIONS:
		switch (nhandle) {
		case 2:						/* Bottom */
			{
				int ch = tileview_int(ctrl, 3);
				int nh = ch+dy;

				tileview_set_int(ctrl, 3, nh>=1 ? nh : 1);
			}
			break;
		case 3:						/* Right */
			{
				int cw = tileview_int(ctrl, 2);
				int nw = cw+dx;

				tileview_set_int(ctrl, 2, nw>=1 ? nw : 1);
			}
			break;
		case 5:						/* Bot right */
			{
				int cw = tileview_int(ctrl, 2);
				int ch = tileview_int(ctrl, 3);
				int nw = cw+dx;
				int nh = ch+dy;

				tileview_set_int(ctrl, 2, nw>=1 ? nw : 1);
				tileview_set_int(ctrl, 3, nh>=1 ? nh : 1);
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

	ctrl->xoffs += xoffs;
	ctrl->yoffs += yoffs;

	if (ctrl->motion != NULL)
		event_post(NULL, tv, ctrl->motion->name, "%i,%i", xoffs, yoffs);
}

static void
mousemotion(int argc, union evarg *argv)
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
	if (ctrl == NULL) {
		switch (tv->state) {
		case TILEVIEW_PIXMAP_EDIT:
			{
				struct tile_element *tel = tv->tv_pixmap.tel;

				if (pixmap_coincident(tel, sx, sy)) {
					pixmap_mousemotion(tv, tel,
					    sx - tel->tel_pixmap.x,
					    sy - tel->tel_pixmap.y,
					    sx - tv->xorig,
					    sy - tv->yorig,
					    state);
				}
			}
			break;
		case TILEVIEW_SKETCH_EDIT:
			{
				struct tile_element *tel = tv->tv_sketch.tel;
				double vx, vy, vxrel, vyrel;

				vg_vcoords2(tel->tel_sketch.sk->vg,
				    sx - tel->tel_sketch.x,
				    sy - tel->tel_sketch.y,
				    0, 0, &vx, &vy);
				vg_vcoords2(tel->tel_sketch.sk->vg,
				    sx - tv->xorig,
				    sy - tv->yorig,
				    0, 0, &vxrel, &vyrel);
				sketch_mousemotion(tv, tel,
				    vx/TILESZ, vy/TILESZ,
				    vxrel/TILESZ, vyrel/TILESZ,
				    state);
			}
			break;
		default:
			break;
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

struct tileview_tool *
tileview_reg_tool(struct tileview *tv, const void *p)
{
	const struct tileview_tool_ops *ops = p;
	struct tileview_tool *tvt;

	tvt = Malloc(ops->len, M_RG);
	tvt->ops = ops;
	tvt->tv = tv;
	tvt->flags = ops->flags;
	tvt->win = NULL;
	if (ops->init != NULL) {
		ops->init(tvt);
	}
	TAILQ_INSERT_TAIL(&tv->tools, tvt, tools);
	return (tvt);
}

void
tileview_set_tile(struct tileview *tv, struct tile *t)
{
	if (tv->tile != NULL) {
		tv->tile->nrefs--;
	}
	tv->tile = t;
	if (t != NULL) {
		t->nrefs++;
		tileview_set_zoom(tv, 100, 0);
	}
}

void
tileview_init(struct tileview *tv, struct tileset *ts, int flags)
{
	widget_init(tv, "tileview", &tileview_ops, WIDGET_WFILL|WIDGET_HFILL|
	                                           WIDGET_FOCUSABLE|
						   WIDGET_CLIPPING);
	tv->ts = ts;
	tv->tile = NULL;
	tv->scaled = NULL;
	tv->zoom = 100;
	tv->pxsz = 1;
	tv->pxlen = 0;
	tv->xoffs = 0;
	tv->yoffs = 0;
	tv->scrolling = 0;
	tv->flags = flags;
	tv->state = TILEVIEW_TILE_EDIT;
	tv->tv_tile.geo_ctrl = NULL;
	tv->tv_tile.orig_ctrl = NULL;
	tv->edit_mode = 0;
	tv->c.r = 255;
	tv->c.g = 255;
	tv->c.b = 255;
	tv->c.a = 128;
	tv->cur_tool = NULL;
	tv->tel_box = NULL;
	tv->tel_tbar = NULL;
	TAILQ_INIT(&tv->tools);
	TAILQ_INIT(&tv->ctrls);

	widget_map_surface(tv, NULL);

	timeout_set(&tv->redraw_to, autoredraw, NULL, 0);
	
	event_new(tv, "window-keydown", keydown, NULL);
	event_new(tv, "window-keyup", keyup, NULL);
	event_new(tv, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(tv, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(tv, "window-mousemotion", mousemotion, NULL);
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
	ctrl->vals = Malloc(sizeof(union tileview_val)*strlen(fmt), M_WIDGET);
	ctrl->valtypes = Malloc(sizeof(enum tileview_val_type)*strlen(fmt),
	    M_WIDGET);
	ctrl->nvals = 0;
	ctrl->motion = NULL;
	ctrl->buttondown = NULL;
	ctrl->buttonup = NULL;
	ctrl->xoffs = 0;
	ctrl->yoffs = 0;
	
	ctrl->c.r = 255;
	ctrl->c.g = 255;
	ctrl->c.b = 255;
	ctrl->a = 255;
	ctrl->cIna.r = 128;
	ctrl->cIna.g = 128;
	ctrl->cIna.b = 128;
	ctrl->aIna = 255;
	ctrl->cEna.r = 250;
	ctrl->cEna.g = 250;
	ctrl->cEna.b = 250;
	ctrl->aEna = 255;
	ctrl->cOver.r = 200;
	ctrl->cOver.g = 200;
	ctrl->cOver.b = 200;
	ctrl->aOver = 255;
	ctrl->cHigh.r = 200;
	ctrl->cHigh.g = 200;
	ctrl->cHigh.b = 200;
	ctrl->cLow.r = 60;
	ctrl->cLow.g = 60;
	ctrl->cLow.b = 60;

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
		if (ctrl->nvals < 1) goto missingvals;
		break;
	case TILEVIEW_RECTANGLE:
	case TILEVIEW_RDIMENSIONS:
		ctrl->nhandles = 6;
		if (ctrl->nvals < 4) goto missingvals;
		break;
	case TILEVIEW_CIRCLE:
		ctrl->nhandles = 2;
		if (ctrl->nvals < 2) goto missingvals;
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
missingvals:
	fatal("missing values");
	return (NULL);
}

static void
tileview_free_ctrl(struct tileview_ctrl *ctrl)
{
	Free(ctrl->valtypes, M_WIDGET);
	Free(ctrl->vals, M_WIDGET);
	Free(ctrl, M_WIDGET);
}

static void
tileview_free_tool(struct tileview_tool *t)
{
	if (t->ops->destroy != NULL) {
		t->ops->destroy(t);
	}
	Free(t, M_RG);
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
	    z2 > 1600)
		return;

	tv->zoom = z2;
	tv->pxsz = z2/100;
	if (tv->pxsz < 1)
		tv->pxsz = 1;

	tv->scaled = SDL_CreateRGBSurface(
	    SDL_SWSURFACE|
	    (t->su->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
	    z2>=100 ? t->su->w*tv->pxsz : t->su->w*z2/100,
	    z2>=100 ? t->su->h*tv->pxsz : t->su->h*z2/100,
	    t->su->format->BitsPerPixel,
	    t->su->format->Rmask, t->su->format->Gmask,
	    t->su->format->Bmask, t->su->format->Amask);
	if (tv->scaled == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	tv->scaled->format->alpha = t->su->format->alpha;
	tv->scaled->format->colorkey = t->su->format->colorkey;
	tv->pxlen = tv->pxsz*tv->scaled->format->BytesPerPixel;

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
		if (tv->tile != NULL) {
			WIDGET(tv)->w = tv->tile->su->w + TILEVIEW_MIN_W;
			WIDGET(tv)->h = tv->tile->su->h + TILEVIEW_MIN_H;
		} else {
			WIDGET(tv)->w = TILEVIEW_MIN_W;
			WIDGET(tv)->h = TILEVIEW_MIN_H;
		}
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
				BLEND_RGBA2_CLIPPED(view->v, cx, cy,
				    tv->c.r, tv->c.g, tv->c.b, tv->c.a);
			} else {
				VIEW_PUT_PIXEL2_CLIPPED(cx, cy, tv->c.pc);
			}
		}
	}
}

static void
draw_status_text(struct tileview *tv, const char *label)
{
	SDL_Surface *su;

	/* XXX pointless colorkey blit */
	su = text_render(NULL, -1, COLOR(TILEVIEW_TEXT_COLOR), label);
	primitives.rect_filled(tv,
	    (su->w >= WIDGET(tv)->w) ? 0 : (WIDGET(tv)->w - su->w - 2),
	    WIDGET(tv)->h - su->h - 2,
	    WIDGET(tv)->w,
	    WIDGET(tv)->h,
	    COLOR(TILEVIEW_TEXTBG_COLOR));
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
tileview_sdl_color(struct tileview *tv, SDL_Color *c, Uint8 a)
{
	tv->c.r = c->r;
	tv->c.g = c->g;
	tv->c.b = c->b;
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(vfmt, c->r, c->g, c->b, a);
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
draw_handle(struct tileview *tv, struct tileview_ctrl *ctrl,
    struct tileview_handle *th)
{
	int x = th->x;
	int y = th->y;

	/* Draw the inner circle. */
	if (th->over && !th->enable) {
		tileview_sdl_color(tv, &ctrl->cOver, ctrl->aOver);
	} else {
		tileview_sdl_color(tv,
		    th->enable ? &ctrl->cEna : &ctrl->cIna,
		    th->enable ? ctrl->aEna : ctrl->aIna);
	}
	tileview_rect2(tv, x-1, y-1, 3, 3);

	/* Draw the central point. */
	tileview_color4i(tv, 255, 255, 255,
	    th->enable ? ctrl->aEna : ctrl->aIna);
	tileview_pixel2i(tv, x, y);

	/* Draw the highlights. */
	tileview_sdl_color(tv,
	    th->enable ? &ctrl->cLow : &ctrl->cHigh,
	    255);
	tileview_pixel2i(tv, x-2, y+1);			/* Highlight left */
	tileview_pixel2i(tv, x-2, y);
	tileview_pixel2i(tv, x-2, y-1);
	tileview_pixel2i(tv, x-1, y-2);			/* Highlight top */
	tileview_pixel2i(tv, x,   y-2);
	tileview_pixel2i(tv, x+1, y-2);
	
	tileview_sdl_color(tv,
	    th->enable ? &ctrl->cHigh : &ctrl->cLow,
	    255);
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

	tileview_sdl_color(tv, &ctrl->c, ctrl->a);

	switch (ctrl->type) {
	case TILEVIEW_RECTANGLE:
	case TILEVIEW_RDIMENSIONS:
		{
			int x = tileview_int(ctrl, 0);
			int y = tileview_int(ctrl, 1);
			u_int w = tileview_uint(ctrl, 2);
			u_int h = tileview_uint(ctrl, 3);

			tileview_rect2o(tv, x-1, y-1, w+1, h+1);

			ctrl->handles[0].x = x - 3;		/* Position */
			ctrl->handles[0].y = y - 3;
			ctrl->handles[1].x = x + w/2;		/* Top */
			ctrl->handles[1].y = y - 3;
			ctrl->handles[2].x = x + w/2;		/* Bottom */
			ctrl->handles[2].y = y + h + 2;
			ctrl->handles[3].x = x + w + 2;		/* Right */
			ctrl->handles[3].y = y + h/2;
			ctrl->handles[4].x = x - 3;		/* Left */
			ctrl->handles[4].y = y + h/2;
			ctrl->handles[5].x = x + w + 2;		/* Bot right */
			ctrl->handles[5].y = y + h + 2;
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
		draw_handle(tv, ctrl, &ctrl->handles[i]);
}

/* Plot a scaled pixel on the tileview's cache surface. */
void
tileview_scaled_pixel(struct tileview *tv, int x, int y, Uint8 r, Uint8 g,
    Uint8 b)
{
	int sx = x*tv->pxsz;
	int sy = y*tv->pxsz;
	Uint32 pixel;
	Uint8 *dst;

	if (sx < 0 || sy < 0 || sx >= tv->scaled->w || sy >= tv->scaled->h)
		return;

	pixel = SDL_MapRGB(tv->scaled->format, r, g, b);

	if (SDL_MUSTLOCK(tv->scaled))
		SDL_LockSurface(tv->scaled);

	if (tv->pxsz == 1) {
		dst = (Uint8 *)tv->scaled->pixels + y*tv->scaled->pitch +
		    x*tv->scaled->format->BytesPerPixel;
		*(Uint32 *)dst = pixel;
	} else {
		int px, py;
		
		dst = (Uint8 *)tv->scaled->pixels +
		    sy*tv->scaled->pitch +
		    sx*tv->scaled->format->BytesPerPixel;

		for (py = 0; py < tv->pxsz; py++) {
			for (px = 0; px < tv->pxsz; px++) {
				*(Uint32 *)dst = pixel;
				dst += tv->scaled->format->BytesPerPixel;
			}
			dst += tv->scaled->pitch - tv->pxlen;
		}
	}

	if (SDL_MUSTLOCK(tv->scaled))
		SDL_UnlockSurface(tv->scaled);
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

	if (t == NULL)
		return;

	if (tv->state == TILEVIEW_SKETCH_EDIT) {
		struct sketch *sk = tv->tv_sketch.sk;

		if (sk->vg->redraw) {
			vg_rasterize(sk->vg);
			t->flags |= TILE_DIRTY;
		}
	}
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
		primitives.tiling(tv, rtiling, 9, 0,
		    COLOR(TILEVIEW_TILE1_COLOR),
		    COLOR(TILEVIEW_TILE2_COLOR));
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
			switch (tv->tv_pixmap.state) {
			case TILEVIEW_PIXMAP_IDLE:
				break;
			case TILEVIEW_PIXMAP_FREEHAND:
				strlcat(status, _(" (free hand)"),
				    sizeof(status));
				break;
			}
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
	struct tileview_tool *tool, *ntool;
	
	if (tv->tile != NULL) {
		tv->tile->nrefs--;
	}
	for (ctrl = TAILQ_FIRST(&tv->ctrls);
	     ctrl != TAILQ_END(&tv->ctrls);
	     ctrl = nctrl) {
		nctrl = TAILQ_NEXT(ctrl, ctrls);
		tileview_free_ctrl(ctrl);
	}
	for (tool = TAILQ_FIRST(&tv->tools);
	     tool != TAILQ_END(&tv->tools);
	     tool = ntool) {
		ntool = TAILQ_NEXT(tool, tools);
		tileview_free_tool(tool);
	}
	widget_destroy(tv);
}

static void
close_tool_win(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	
	view_detach(tv->cur_tool->win);
	tv->cur_tool->win = NULL;
}

void
tileview_select_tool(struct tileview *tv, struct tileview_tool *tvt)
{
	struct window *pwin = widget_parent_window(tv);

#ifdef DEBUG
	if (pwin == NULL)
		fatal("%s has no parent window", OBJECT(tv)->name);
#endif

	if (tv->cur_tool != NULL && tv->cur_tool->win != NULL) {
		view_detach(tv->cur_tool->win);
		tv->cur_tool->win = NULL;
	}

	if (tvt->ops->selected != NULL) {
		tvt->ops->selected(tvt);
	}
	if (tvt->ops->edit != NULL) {
		tvt->win = tvt->ops->edit(tvt);
		window_set_caption(tvt->win, _(tvt->ops->name));
		window_set_position(tvt->win, WINDOW_LOWER_LEFT, 0);
		window_attach(pwin, tvt->win);
		window_show(tvt->win);
		event_new(tvt->win, "window-close", close_tool_win, "%p", tv);
	}
	
	tv->cur_tool = tvt;
}

void
tileview_unselect_tool(struct tileview *tv)
{
	if (tv->cur_tool != NULL) {
		if (tv->cur_tool->win != NULL) {
			view_detach(tv->cur_tool->win);
			tv->cur_tool->win = NULL;
		}
		if (tv->cur_tool->ops->unselected != NULL)
			tv->cur_tool->ops->unselected(tv->cur_tool);
	}
	tv->cur_tool = NULL;
}
