/*	$Csoft: tileview.c,v 1.54 2005/09/22 02:30:26 vedge Exp $	*/

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

const AG_WidgetOps tileview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		RG_TileviewDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	RG_TileviewDraw,
	RG_TileviewScale
};

RG_Tileview *
RG_TileviewNew(void *parent, RG_Tileset *ts, int flags)
{
	RG_Tileview *tv;

	tv = Malloc(sizeof(RG_Tileview), M_OBJECT);
	RG_TileviewInit(tv, ts, flags);
	AG_ObjectAttach(parent, tv);
	return (tv);
}

static Uint32
zoomin_tick(void *obj, Uint32 ival, void *arg)
{
	RG_Tileview *tv = obj;

	if (tv->zoom > 1600) {
		return (0);
	}
	RG_TileviewSetZoom(tv, tv->zoom+20, 1);
	return (ival);
}

static Uint32
zoomout_tick(void *obj, Uint32 ival, void *arg)
{
	RG_Tileview *tv = obj;

	if (tv->zoom < 100) {
		return (0);
	}
	RG_TileviewSetZoom(tv, tv->zoom-20, 1);
	return (ival);
}

static __inline__ void
move_cursor(RG_Tileview *tv, int x, int y)
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
cursor_overlap(struct rg_tileview_handle *th, int sx, int sy)
{
	return (sx >= th->x - 2 && sx <= th->x + 2 &&
	        sy >= th->y - 2 && sy <= th->y + 2);
}

static void
keydown(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapKeyDown(tv, tv->tv_pixmap.tel, keysym, keymod);
		break;
	case RG_RG_TILEVIEW_SKETCH_EDIT:
		RG_SketchKeyDown(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
	default:
		break;
	}

	switch (keysym) {
	case SDLK_z:
		if (keymod & KMOD_CTRL) {
			switch (tv->state) {
			case RG_TILEVIEW_PIXMAP_EDIT:
				RG_PixmapUndo(tv, tv->tv_pixmap.tel);
				break;
			case RG_RG_TILEVIEW_SKETCH_EDIT:
				RG_SketchUndo(tv, tv->tv_sketch.tel);
				break;
			default:
				break;
			}
		}
		break;
	case SDLK_r:
		if (keymod & KMOD_CTRL) {
			switch (tv->state) {
			case RG_TILEVIEW_PIXMAP_EDIT:
				RG_PixmapRedo(tv, tv->tv_pixmap.tel);
				break;
			case RG_RG_TILEVIEW_SKETCH_EDIT:
				RG_SketchRedo(tv, tv->tv_sketch.tel);
				break;
			default:
				break;
			}
		} else {
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case SDLK_EQUALS:
		AG_SetTimeout(&tv->zoom_to, zoomin_tick, NULL, 0);
		AG_DelTimeout(tv, &tv->zoom_to);
		AG_AddTimeout(tv, &tv->zoom_to, 10);
		zoomin_tick(tv, 0, NULL);
		break;
	case SDLK_MINUS:
		AG_SetTimeout(&tv->zoom_to, zoomout_tick, NULL, 0);
		AG_DelTimeout(tv, &tv->zoom_to);
		AG_AddTimeout(tv, &tv->zoom_to, 10);
		zoomout_tick(tv, 0, NULL);
		break;
	case SDLK_0:
	case SDLK_1:
		RG_TileviewSetZoom(tv, 100, 1);
		break;
	}
}

static void
keyup(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;
	
	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapKeyUp(tv, tv->tv_pixmap.tel, keysym, keymod);
		break;
	case RG_RG_TILEVIEW_SKETCH_EDIT:
		RG_SketchKeyUp(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
	default:
		break;
	}

	switch (keysym) {
	case SDLK_EQUALS:
	case SDLK_MINUS:
		AG_DelTimeout(tv, &tv->zoom_to);
		break;
	}
}

static __inline__ int
pixmap_coincident(RG_TileElement *tel, int x, int y)
{
	return (x >= tel->tel_pixmap.x &&
	        x < tel->tel_pixmap.x + tel->tel_pixmap.px->su->w &&
	        y >= tel->tel_pixmap.y &&
	        y < tel->tel_pixmap.y + tel->tel_pixmap.px->su->h);
}

static __inline__ int
sketch_coincident(RG_TileElement *tel, int x, int y)
{
	return (x >= tel->tel_sketch.x &&
	        x < tel->tel_sketch.x + tel->tel_sketch.sk->vg->su->w &&
	        y >= tel->tel_sketch.y &&
	        y < tel->tel_sketch.y + tel->tel_sketch.sk->vg->su->h);
}

static void
toggle_attrib(RG_Tileview *tv, int sx, int sy)
{
	RG_Tile *t = tv->tile;
	int nx = sx/AGTILESZ;
	int ny = sy/AGTILESZ;
	u_int *a;
	
	if (nx < 0 || nx >= t->nw ||
	    ny < 0 || ny >= t->nh ||
	    (tv->tv_attrs.nx == nx && tv->tv_attrs.ny == ny))
		return;

	a = &RG_TILE_ATTR2(t,nx,ny);
	if (*a & tv->edit_attr) {
		*a &= ~(tv->edit_attr);
	} else {
		*a |= tv->edit_attr;
	}

	tv->tv_attrs.nx = nx;
	tv->tv_attrs.ny = ny;

	t->flags |= RG_TILE_DIRTY;
}

static void
increment_layer(RG_Tileview *tv, int sx, int sy, int inc)
{
	RG_Tile *t = tv->tile;
	int nx = sx/AGTILESZ;
	int ny = sy/AGTILESZ;
	int *a;

	if (nx < 0 || nx >= t->nw ||
	    ny < 0 || ny >= t->nh)
		return;

	a = &RG_TILE_LAYER2(t,nx,ny);
	(*a) += inc;

	t->flags |= RG_TILE_DIRTY;
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	RG_TileviewCtrl *ctrl;
	RG_TileElement *tel = tv->tv_pixmap.tel;
	int sx = (x - tv->xoffs)/tv->pxsz;
	int sy = (y - tv->yoffs)/tv->pxsz;
	int i;

	AG_WidgetFocus(tv);

	switch (button) {
	case SDL_BUTTON_WHEELUP:
		if (tv->state == RG_TILEVIEW_PIXMAP_EDIT) {
			if (RG_PixmapMouseWheel(tv, tel, 0) == 1)
				return;
		} else if (tv->state == RG_RG_TILEVIEW_SKETCH_EDIT) {
			if (RG_SketchMouseWheel(tv, tel, 0) == 1)
				return;
		}
		RG_TileviewSetZoom(tv,
		    tv->zoom<100 ? tv->zoom+5 : tv->zoom+100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		return;
	case SDL_BUTTON_WHEELDOWN:
		if (tv->state == RG_TILEVIEW_PIXMAP_EDIT) {
			if (RG_PixmapMouseWheel(tv, tel, 1) == 1)
				return;
		} else if (tv->state == RG_RG_TILEVIEW_SKETCH_EDIT) {
			if (RG_SketchMouseWheel(tv, tel, 1) == 1)
				return;
		}
		RG_TileviewSetZoom(tv,
		    tv->zoom<100 ? tv->zoom-5 : tv->zoom-100, 1);
		move_cursor(tv, x - tv->xoffs, y - tv->yoffs);
		break;
	default:
		break;
	}
	
	if (button == SDL_BUTTON_LEFT &&
	    (tv->flags & RG_TILEVIEW_HIDE_CONTROLS) == 0) {
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				struct rg_tileview_handle *th =
				    &ctrl->handles[i];
		
				if (cursor_overlap(th, sx, sy)) {
					th->enable = 1;
					tv->xorig = sx;
					tv->yorig = sy;
					ctrl->xoffs = 0;
					ctrl->yoffs = 0;
					if (ctrl->buttondown != NULL) {
						AG_PostEvent(NULL, tv,
						    ctrl->buttondown->name,
						    "%i,%i", sx, sy);
					}
					break;
				}
			}
			if (i < ctrl->nhandles)
				return;
		}
	}

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		if (pixmap_coincident(tel, sx, sy)) {
			tv->tv_pixmap.xorig = sx - tel->tel_pixmap.x;
			tv->tv_pixmap.yorig = sy - tel->tel_pixmap.y;
			RG_PixmapMousebuttonDown(tv, tel, tv->tv_pixmap.xorig,
			    tv->tv_pixmap.yorig, button);
			return;
		} else {
			if (button == SDL_BUTTON_RIGHT)
				tv->scrolling++;
		}
		break;
	case RG_RG_TILEVIEW_SKETCH_EDIT:
		if (button == SDL_BUTTON_RIGHT &&
		   (tv->flags & RG_TILEVIEW_NO_SCROLLING) == 0) {
			tv->scrolling++;
		}
		if (button == SDL_BUTTON_MIDDLE || button == SDL_BUTTON_RIGHT ||
		   (button == SDL_BUTTON_LEFT &&
		    sketch_coincident(tel, sx, sy)))
		{
			RG_Sketch *sk = tv->tv_sketch.sk;
			double vx, vy;

			vx = VG_VECXF(sk->vg, sx - tel->tel_sketch.x);
			vy = VG_VECYF(sk->vg, sy - tel->tel_sketch.y);
			RG_SketchMouseButtonDown(tv, tel, vx, vy, button);
			return;
		}
		break;
	case RG_TILEVIEW_FEATURE_EDIT:
		if (button == SDL_BUTTON_MIDDLE) {
			if (tv->tv_feature.ft->ops->menu != NULL) {
				RG_FeatureOpenMenu(tv,
				    AGWIDGET(tv)->cx + x,
				    AGWIDGET(tv)->cy + y);
			}
		} else if (button == SDL_BUTTON_RIGHT) {
			tv->scrolling++;
		}
		break;
	case RG_TILEVIEW_TILE_EDIT:
		if (button == SDL_BUTTON_RIGHT) {
			tv->scrolling++;
		}
		break;
	case RG_TILEVIEW_ATTRIB_EDIT:
		if (button == SDL_BUTTON_RIGHT) {
			tv->scrolling++;
		} else if (button ==  SDL_BUTTON_LEFT) {
			tv->flags |= RG_TILEVIEW_SET_ATTRIBS;
			tv->tv_attrs.nx = -1;
			tv->tv_attrs.ny = -1;
			toggle_attrib(tv, sx, sy);
		}
		break;
	case RG_TILEVIEW_LAYERS_EDIT:
		if (button == SDL_BUTTON_LEFT) {
			increment_layer(tv, sx, sy, +1);
		} else if (button == SDL_BUTTON_RIGHT) {
			increment_layer(tv, sx, sy, -1);
			tv->scrolling++;
		}
		break;
	}

	if (button == SDL_BUTTON_MIDDLE &&
	    tv->state == RG_TILEVIEW_TILE_EDIT) {
		RG_TileOpenMenu(tv,
		    AGWIDGET(tv)->cx+x,
		    AGWIDGET(tv)->cy+y);
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[0].p;
	int button = argv[1].i;
	RG_TileviewCtrl *ctrl;
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
				struct rg_tileview_handle *th =
				    &ctrl->handles[i];

				if (th->enable) {
					th->enable = 0;
					if (ctrl->buttonup != NULL) {
						AG_PostEvent(NULL, tv,
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
			case RG_TILEVIEW_PIXMAP_EDIT:
				{
					RG_TileElement *tel =
					    tv->tv_pixmap.tel;
					RG_Pixmap *px = tel->tel_pixmap.px;

					RG_PixmapMousebuttonUp(tv, tel,
					    tv->xms - tel->tel_pixmap.x,
					    tv->yms - tel->tel_pixmap.y,
					    button);
				}
				break;
			case RG_RG_TILEVIEW_SKETCH_EDIT:
				{
					RG_TileElement *tel =
					    tv->tv_sketch.tel;
					RG_Sketch *sk = tel->tel_sketch.sk;
					double vx, vy;
				
					VG_Vcoords2(tel->tel_sketch.sk->vg,
					    tv->xms - tel->tel_sketch.x,
					    tv->yms - tel->tel_sketch.y,
					    0, 0, &vx, &vy);

					RG_SketchMouseButtonUp(tv, tel, vx, vy,
					    button);
				}
				break;
			case RG_TILEVIEW_ATTRIB_EDIT:
				tv->flags &= ~(RG_TILEVIEW_SET_ATTRIBS);
				break;
			default:
				break;
			}
		}
		break;
	}
}

static __inline__ void
clamp_offsets(RG_Tileview *tv)
{
	int lim;

	if (tv->xoffs >
	   (lim = (AGWIDGET(tv)->w - RG_TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	} else if (tv->xoffs <
	   (lim = (-tv->scaled->w + RG_TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	}

	if (tv->yoffs >
	    (lim = (AGWIDGET(tv)->h - RG_TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	} else if (tv->yoffs <
	    (lim = (-tv->scaled->h + RG_TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	}
}

void
RG_TileviewSetInt(RG_TileviewCtrl *ctrl, int nval, int v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_INT_VAL:
		ctrl->vals[nval].i = v;
		break;
	case RG_TILEVIEW_UINT_VAL:
		ctrl->vals[nval].ui = (u_int)v;
		break;
	case RG_TILEVIEW_INT_PTR:
		*(int *)ctrl->vals[nval].p = v;
		break;
	case RG_TILEVIEW_UINT_PTR:
		*(u_int *)ctrl->vals[nval].p = (u_int)v;
		break;
	default:
		fatal("cannot convert");
	}
}

void
RG_TileviewSetFloat(RG_TileviewCtrl *ctrl, int nval, float v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = v;
		break;
	case RG_TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = (double)v;
		break;
	case RG_TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = v;
		break;
	case RG_TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = (double)v;
		break;
	default:
		fatal("cannot convert");
	}
}

void
RG_TileviewSetDouble(RG_TileviewCtrl *ctrl, int nval, double v)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		ctrl->vals[nval].f = (float)v;
		break;
	case RG_TILEVIEW_DOUBLE_VAL:
		ctrl->vals[nval].d = v;
		break;
	case RG_TILEVIEW_FLOAT_PTR:
		*(float *)ctrl->vals[nval].p = (float)v;
		break;
	case RG_TILEVIEW_DOUBLE_PTR:
		*(double *)ctrl->vals[nval].p = v;
		break;
	default:
		fatal("cannot convert");
	}
}

static void
move_handle(RG_Tileview *tv, RG_TileviewCtrl *ctrl, int nhandle,
    int x2, int y2)
{
	struct rg_tileview_handle *th = &ctrl->handles[nhandle];
	int dx = x2 - tv->xorig;
	int dy = y2 - tv->yorig;
	int xoffs = 0;
	int yoffs = 0;

	if (dx == 0 && dy == 0)
		return;

	switch (ctrl->type) {
	case RG_TILEVIEW_RECTANGLE:
		switch (nhandle) {
		case 0:
			RG_TileviewSetInt(ctrl, 0,
			    RG_TileviewInt(ctrl,0)+dx);
			RG_TileviewSetInt(ctrl, 1,
			    RG_TileviewInt(ctrl,1)+dy);
			break;
		case 1:						/* Top */
			{
				int ch = RG_TileviewInt(ctrl, 3);
				int cy = RG_TileviewInt(ctrl, 1);
				int nh = ch-dy;

				RG_TileviewSetInt(ctrl, 1, cy+dy);
				RG_TileviewSetInt(ctrl, 3, nh>=1 ? nh : 1);
				if (dy < 0 || dy > 0)
					yoffs = -dy;
			}
			break;
		case 4:						/* Left */
			{
				int cw = RG_TileviewInt(ctrl, 2);
				int cx = RG_TileviewInt(ctrl, 0);
				int nw = cw-dx;


				RG_TileviewSetInt(ctrl, 0, cx+dx);
				RG_TileviewSetInt(ctrl, 2, nw>=1 ? nw : 1);
				if (dx < 0 || dx > 0)
					xoffs = -dx;
			}
			break;
		}
		/* FALLTHROUGH */
	case RG_TILEVIEW_RDIMENSIONS:
		switch (nhandle) {
		case 2:						/* Bottom */
			{
				int ch = RG_TileviewInt(ctrl, 3);
				int nh = ch+dy;

				RG_TileviewSetInt(ctrl, 3, nh>=1 ? nh : 1);
			}
			break;
		case 3:						/* Right */
			{
				int cw = RG_TileviewInt(ctrl, 2);
				int nw = cw+dx;

				RG_TileviewSetInt(ctrl, 2, nw>=1 ? nw : 1);
			}
			break;
		case 5:						/* Bot right */
			{
				int cw = RG_TileviewInt(ctrl, 2);
				int ch = RG_TileviewInt(ctrl, 3);
				int nw = cw+dx;
				int nh = ch+dy;

				RG_TileviewSetInt(ctrl, 2, nw>=1 ? nw : 1);
				RG_TileviewSetInt(ctrl, 3, nh>=1 ? nh : 1);
			}
			break;
		}
		break;
	case RG_TILEVIEW_POINT:
		RG_TileviewSetInt(ctrl, 0, RG_TileviewInt(ctrl, 0)+dx);
		RG_TileviewSetInt(ctrl, 1, RG_TileviewInt(ctrl, 1)+dy);
		break;
	case RG_TILEVIEW_VERTEX:
		RG_TileviewSetDouble(ctrl, 0,
		    RG_TileviewDouble(ctrl, 0)+VG_VECXF(ctrl->vg,dx));
		RG_TileviewSetDouble(ctrl, 1,
		    RG_TileviewDouble(ctrl, 1)+VG_VECYF(ctrl->vg,dy));
		break;
	default:
		break;
	}

	ctrl->xoffs += xoffs;
	ctrl->yoffs += yoffs;

	if (ctrl->motion != NULL)
		AG_PostEvent(NULL, tv, ctrl->motion->name, "%i,%i",
		    xoffs, yoffs);
}

static void
mousemotion(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	int state = argv[5].i;
	RG_TileviewCtrl *ctrl;
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
			struct rg_tileview_handle *th = &ctrl->handles[i];

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
		case RG_TILEVIEW_PIXMAP_EDIT:
			{
				RG_TileElement *tel = tv->tv_pixmap.tel;

				if (pixmap_coincident(tel, sx, sy)) {
					RG_PixmapMouseMotion(tv, tel,
					    sx - tel->tel_pixmap.x,
					    sy - tel->tel_pixmap.y,
					    sx - tv->xorig,
					    sy - tv->yorig,
					    state);
				}
			}
			break;
		case RG_RG_TILEVIEW_SKETCH_EDIT:
			{
				RG_TileElement *tel = tv->tv_sketch.tel;
				double vx, vy, vxrel, vyrel;

				VG_Vcoords2(tel->tel_sketch.sk->vg,
				    sx - tel->tel_sketch.x,
				    sy - tel->tel_sketch.y,
				    0, 0, &vx, &vy);
				VG_Vcoords2(tel->tel_sketch.sk->vg,
				    sx - tv->xorig,
				    sy - tv->yorig,
				    0, 0, &vxrel, &vyrel);
				RG_SketchMouseMotion(tv, tel,
				    vx/AGTILESZ, vy/AGTILESZ,
				    vxrel/AGTILESZ, vyrel/AGTILESZ,
				    state);
			}
			break;
		case RG_TILEVIEW_ATTRIB_EDIT:
			if (tv->flags & RG_TILEVIEW_SET_ATTRIBS) {
				toggle_attrib(tv, sx, sy);
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
	RG_Tileview *tv = obj;

	tv->tile->flags |= RG_TILE_DIRTY;
	return (ival);
}

RG_TileviewTool *
RG_TileviewRegTool(RG_Tileview *tv, const void *p)
{
	const RG_TileviewToolOps *ops = p;
	RG_TileviewTool *tvt;

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
RG_TileviewSetTile(RG_Tileview *tv, RG_Tile *t)
{
	if (tv->tile != NULL) {
		tv->tile->nrefs--;
	}
	tv->tile = t;
	if (t != NULL) {
		t->nrefs++;
		RG_TileviewSetZoom(tv, 100, 0);
	}
}

void
RG_TileviewInit(RG_Tileview *tv, RG_Tileset *ts, int flags)
{
	AG_WidgetInit(tv, "tileview", &tileview_ops,
	    AG_WIDGET_WFILL|AG_WIDGET_HFILL|AG_WIDGET_FOCUSABLE|
	    AG_WIDGET_CLIPPING);
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
	tv->state = RG_TILEVIEW_TILE_EDIT;
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
	tv->menu = NULL;
	tv->menu_item = NULL;
	tv->menu_win = NULL;
	TAILQ_INIT(&tv->tools);
	TAILQ_INIT(&tv->ctrls);

	AG_WidgetMapSurface(tv, NULL);

	AG_SetTimeout(&tv->redraw_to, autoredraw, NULL, 0);
	
	AG_SetEvent(tv, "window-keydown", keydown, NULL);
	AG_SetEvent(tv, "window-keyup", keyup, NULL);
	AG_SetEvent(tv, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(tv, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(tv, "window-mousemotion", mousemotion, NULL);
}

#define INSERT_VALUE(vt,memb, type,arg) do {			\
	ctrl->valtypes[ctrl->nvals] = (vt);			\
	ctrl->vals[ctrl->nvals].memb = (type)va_arg(ap, arg);	\
	ctrl->nvals++;						\
} while (/*CONSTCOND*/0)

/* Create a graphically editable geometric control. */
RG_TileviewCtrl *
RG_TileviewAddCtrl(RG_Tileview *tv, enum rg_tileview_ctrl_type type,
    const char *fmt, ...)
{
	RG_TileviewCtrl *ctrl;
	va_list ap;
	int i;

	ctrl = Malloc(sizeof(RG_TileviewCtrl), M_WIDGET);
	ctrl->type = type;
	ctrl->vals = Malloc(sizeof(union rg_tileview_val)*strlen(fmt),
	    M_WIDGET);
	ctrl->valtypes = Malloc(sizeof(enum tileview_val_type)*strlen(fmt),
	    M_WIDGET);
	ctrl->nvals = 0;
	ctrl->vg = NULL;
	ctrl->vge = NULL;
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
				INSERT_VALUE(RG_TILEVIEW_INT_PTR, p, int *,
				    void *);
				break;
			case 'u':
				INSERT_VALUE(RG_TILEVIEW_UINT_PTR, p, u_int *,
				    void *);
				break;
			case 'f':
				INSERT_VALUE(RG_TILEVIEW_FLOAT_PTR, p, float *,
				    void *);
				break;
			case 'd':
				INSERT_VALUE(RG_TILEVIEW_DOUBLE_PTR, p,
				    double *, void *);
				break;
			default:
				fatal("bad format");
			}
			fmt++;
			break;
		case 'i':
			INSERT_VALUE(RG_TILEVIEW_INT_VAL, i, int, int);
			break;
		case 'u':
			INSERT_VALUE(RG_TILEVIEW_UINT_VAL, ui, u_int, int);
			break;
		case 'f':
			INSERT_VALUE(RG_TILEVIEW_FLOAT_VAL, f, float, double);
			break;
		case 'd':
			INSERT_VALUE(RG_TILEVIEW_DOUBLE_VAL, d, double, double);
			break;
		default:
			break;
		}
	}
	va_end(ap);
out:
	switch (ctrl->type) {
	case RG_TILEVIEW_POINT:
		ctrl->nhandles = 1;
		if (ctrl->nvals < 1) goto missingvals;
		break;
	case RG_TILEVIEW_VERTEX:
		ctrl->nhandles = 1;
		if (ctrl->nvals < 1) goto missingvals;
		break;
	case RG_TILEVIEW_RECTANGLE:
	case RG_TILEVIEW_RDIMENSIONS:
		ctrl->nhandles = 6;
		if (ctrl->nvals < 4) goto missingvals;
		break;
	case RG_TILEVIEW_CIRCLE:
		ctrl->nhandles = 2;
		if (ctrl->nvals < 2) goto missingvals;
		break;
	default:
		ctrl->nhandles = 0;
		break;
	}
	ctrl->handles = (ctrl->nhandles > 0) ?
	    Malloc(sizeof(struct rg_tileview_handle)*ctrl->nhandles, M_WIDGET) :
	    NULL;
	for (i = 0; i < ctrl->nhandles; i++) {
		struct rg_tileview_handle *th = &ctrl->handles[i];

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
tileview_free_ctrl(RG_TileviewCtrl *ctrl)
{
	Free(ctrl->valtypes, M_WIDGET);
	Free(ctrl->vals, M_WIDGET);
	Free(ctrl, M_WIDGET);
}

static void
tileview_free_tool(RG_TileviewTool *t)
{
	if (t->ops->destroy != NULL) {
		t->ops->destroy(t);
	}
	Free(t, M_RG);
}

void
RG_TileviewDelCtrl(RG_Tileview *tv, RG_TileviewCtrl *ctrl)
{
	TAILQ_REMOVE(&tv->ctrls, ctrl, ctrls);
	tileview_free_ctrl(ctrl);
}

void
RG_TileviewSetAutoRefresh(RG_Tileview *tv, int ena, int rate)
{
	if (ena) {
		AG_AddTimeout(tv, &tv->redraw_to, rate);
		dprintf("enabled autoredraw\n");
	} else {
		AG_DelTimeout(tv, &tv->redraw_to);
		dprintf("disabled autoredraw\n");
	}
}

void
RG_TileviewSetZoom(RG_Tileview *tv, int z2, int adj_offs)
{
	RG_Tile *t = tv->tile;
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
	AG_WidgetReplaceSurface(tv, 0, tv->scaled);

	if (adj_offs) {
		tv->xoffs += (t->su->w*pxsz1 - t->su->w*tv->pxsz)/2;
		tv->yoffs += (t->su->h*pxsz1 - t->su->h*tv->pxsz)/2;
		clamp_offsets(tv);
	}
	t->flags |= RG_TILE_DIRTY;
}

void
RG_TileviewScale(void *p, int rw, int rh)
{
	RG_Tileview *tv = p;
	int lim;

	if (rw == -1 && rh == -1) {
		if (tv->tile != NULL) {
			AGWIDGET(tv)->w = tv->tile->su->w + RG_TILEVIEW_MIN_W;
			AGWIDGET(tv)->h = tv->tile->su->h + RG_TILEVIEW_MIN_H;
		} else {
			AGWIDGET(tv)->w = RG_TILEVIEW_MIN_W;
			AGWIDGET(tv)->h = RG_TILEVIEW_MIN_H;
		}
	} else {
		if (tv->xoffs >
		   (lim = (AGWIDGET(tv)->w - RG_TILEVIEW_MIN_W))) {
			tv->xoffs = lim;
		}
		if (tv->yoffs >
		   (lim	= (AGWIDGET(tv)->h - RG_TILEVIEW_MIN_H))) {
			tv->yoffs = lim;
		}
	}
}

void
RG_TileviewPixel2i(RG_Tileview *tv, int x, int y)
{
	int x1 = RG_TILEVIEW_SCALED_X(tv,x);
	int y1 = RG_TILEVIEW_SCALED_Y(tv,y);
	int dx, dy;

	if (!agView->opengl) {
		for (dy = 0; dy < tv->pxsz; dy++) {
			for (dx = 0; dx < tv->pxsz; dx++) {
				int cx = x1+dx;
				int cy = y1+dy;

				if (cx < AGWIDGET(tv)->cx ||
				    cy < AGWIDGET(tv)->cy ||
				    cx > AGWIDGET(tv)->cx2 ||
				    cy > AGWIDGET(tv)->cy2)
					continue;

				if (tv->c.a < 255) {
					AG_BLEND_RGBA2_CLIPPED(agView->v,
					    cx, cy,
					    tv->c.r, tv->c.g, tv->c.b, tv->c.a,
					    AG_ALPHA_OVERLAY);
				} else {
					AG_VIEW_PUT_PIXEL2_CLIPPED(cx, cy,
					    tv->c.pc);
				}
			}
		}
	} else {
#ifdef HAVE_OPENGL
		int x2 = x1 + tv->pxsz;
		int y2 = y1 + tv->pxsz;

		if (x1 > AGWIDGET(tv)->cx2)	return;
		if (y1 > AGWIDGET(tv)->cy2)	return;
		if (x1 < AGWIDGET(tv)->cx)	x1 = AGWIDGET(tv)->cx;
		if (y1 < AGWIDGET(tv)->cy)	y1 = AGWIDGET(tv)->cy;
		if (x1 >= x2 || y1 >= y2)	return;
		if (x2 > AGWIDGET(tv)->cx2)	x2 = AGWIDGET(tv)->cx2;
		if (y2 > AGWIDGET(tv)->cy2)	y2 = AGWIDGET(tv)->cy2;

		glBegin(GL_POLYGON);
		if (tv->c.a < 255) {
			glColor4ub(tv->c.r, tv->c.g, tv->c.b, tv->c.a);
		} else {
			glColor3ub(tv->c.r, tv->c.g, tv->c.b);
		}
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();
#endif
	}
}

static void
draw_status_text(RG_Tileview *tv, const char *label)
{
	SDL_Surface *su;

	/* XXX pointless colorkey blit */
	su = AG_TextRender(NULL, -1, AG_COLOR(TILEVIEW_TEXT_COLOR), label);
	agPrim.rect_filled(tv,
	    (su->w >= AGWIDGET(tv)->w) ? 0 : (AGWIDGET(tv)->w - su->w - 2),
	    AGWIDGET(tv)->h - su->h - 2,
	    AGWIDGET(tv)->w,
	    AGWIDGET(tv)->h,
	    AG_COLOR(TILEVIEW_TEXTBG_COLOR));
	AG_WidgetBlit(tv, su,
	    AGWIDGET(tv)->w - su->w - 1,
	    AGWIDGET(tv)->h - su->h - 1);
	SDL_FreeSurface(su);
}

void
RG_TileviewColor3i(RG_Tileview *tv, Uint8 r, Uint8 g, Uint8 b)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.pc = SDL_MapRGB(agVideoFmt, r, g, b);
}

void
RG_TileviewColor4i(RG_Tileview *tv, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(agVideoFmt, r, g, b, a);
}

void
RG_TileviewSDLColor(RG_Tileview *tv, SDL_Color *c, Uint8 a)
{
	tv->c.r = c->r;
	tv->c.g = c->g;
	tv->c.b = c->b;
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(agVideoFmt, c->r, c->g, c->b, a);
}

void
RG_TileviewAlpha(RG_Tileview *tv, Uint8 a)
{
	tv->c.a = a;
	tv->c.pc = SDL_MapRGBA(agVideoFmt, tv->c.r, tv->c.g, tv->c.b, a);
}

void
RG_TileviewRect2(RG_Tileview *tv, int x, int y, int w, int h)
{
	Uint8 r, g, b;

	if (!agView->opengl) {
		int xi, yi;
		int x2 = x+w;
		int y2 = y+h;

		for (yi = y; yi < y2; yi++)
			for (xi = x; xi < x2; xi++)
				RG_TileviewPixel2i(tv, xi, yi);
	} else {
#ifdef HAVE_OPENGL
		int x1 = RG_TILEVIEW_SCALED_X(tv,x);
		int y1 = RG_TILEVIEW_SCALED_Y(tv,y);
		int x2 = x1 + w*tv->pxsz;
		int y2 = y1 + h*tv->pxsz;

		if (x1 > AGWIDGET(tv)->cx2)	return;
		if (y1 > AGWIDGET(tv)->cy2)	return;
		if (x1 < AGWIDGET(tv)->cx)	x1 = AGWIDGET(tv)->cx;
		if (y1 < AGWIDGET(tv)->cy)	y1 = AGWIDGET(tv)->cy;
		if (x1 >= x2 || y1 >= y2)	return;
		if (x2 > AGWIDGET(tv)->cx2)	x2 = AGWIDGET(tv)->cx2;
		if (y2 > AGWIDGET(tv)->cy2)	y2 = AGWIDGET(tv)->cy2;

		glBegin(GL_POLYGON);
		if (tv->c.a < 255) {
			glColor4ub(tv->c.r, tv->c.g, tv->c.b, tv->c.a);
		} else {
			glColor3ub(tv->c.r, tv->c.g, tv->c.b);
		}
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();
#endif /* HAVE_OPENGL */
	}
}

void
RG_TileviewCircle2o(RG_Tileview *tv, int x0, int y0, int r)
{
	int v = 2*r - 1;
	int e = 0;
	int u = 1;
	int x = 0;
	int y = r;

	while (x < y) {
		RG_TileviewPixel2i(tv, x0+x, y0+y);
		RG_TileviewPixel2i(tv, x0+x, y0-y);
		RG_TileviewPixel2i(tv, x0-x, y0+y);
		RG_TileviewPixel2i(tv, x0-x, y0-y);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;

		RG_TileviewPixel2i(tv, x0+y, y0+x);
		RG_TileviewPixel2i(tv, x0+y, y0-x);
		RG_TileviewPixel2i(tv, x0-y, y0+x);
		RG_TileviewPixel2i(tv, x0-y, y0-x);
	}
	RG_TileviewPixel2i(tv, x0-r, y0);
	RG_TileviewPixel2i(tv, x0+r, y0);
}

void
RG_TileviewVLine(RG_Tileview *tv, int x, int y1, int y2)
{
	int y;

	/* TODO opengl */

	for (y = y1; y < y2; y++)
		RG_TileviewPixel2i(tv, x, y);
}

void
RG_TileviewHLine(RG_Tileview *tv, int x1, int x2, int y)
{
	int x;

	/* TODO opengl */

	for (x = x1; x < x2; x++)
		RG_TileviewPixel2i(tv, x, y);
}

void
RG_TileviewRect2o(RG_Tileview *tv, int x, int y, int w, int h)
{
	if (!agView->opengl) {
		int xi, yi;

		for (yi = y+1; yi < y+h; yi++) {
			RG_TileviewPixel2i(tv, x, yi);
			RG_TileviewPixel2i(tv, x+w, yi);
		}
		for (xi = x; xi < x+w+1; xi++) {
			RG_TileviewPixel2i(tv, xi, y);
			RG_TileviewPixel2i(tv, xi, y+h);
		}
	} else {
#ifdef HAVE_OPENGL
		int x1 = RG_TILEVIEW_SCALED_X(tv,x);
		int y1 = RG_TILEVIEW_SCALED_Y(tv,y);
		int x2 = x1 + w*tv->pxsz;
		int y2 = y1 + h*tv->pxsz;
		GLfloat saved_width;

		glGetFloatv(GL_LINE_WIDTH, &saved_width);
		glLineWidth(tv->pxsz);
		glBegin(GL_LINES);
		if (tv->c.a < 255) {
			glColor4ub(tv->c.r, tv->c.g, tv->c.b, tv->c.a);
		} else {
			glColor3ub(tv->c.r, tv->c.g, tv->c.b);
		}

		if (y1 > AGWIDGET(tv)->cy && y1 < AGWIDGET(tv)->cy2 &&
		    x1 < AGWIDGET(tv)->cx2 && x2 > AGWIDGET(tv)->cx) {
			glVertex2i(MAX(x1, AGWIDGET(tv)->cx), y1);
			glVertex2i(MIN(x2, AGWIDGET(tv)->cx2), y1);
		}
		if (x2 < AGWIDGET(tv)->cx2 && x2 > AGWIDGET(tv)->cx &&
		    y1 < AGWIDGET(tv)->cy2 && y2 > AGWIDGET(tv)->cy) {
			glVertex2i(x2, MAX(y1, AGWIDGET(tv)->cy));
			glVertex2i(x2, MIN(y2, AGWIDGET(tv)->cy2));
		}
		if (y2 > AGWIDGET(tv)->cy && y2 < AGWIDGET(tv)->cy2 &&
		    x1 < AGWIDGET(tv)->cx2 && x2 > AGWIDGET(tv)->cx) {
			glVertex2i(MIN(x2, AGWIDGET(tv)->cx2), y2);
			glVertex2i(MAX(x1, AGWIDGET(tv)->cx), y2);
		}
		if (x1 > AGWIDGET(tv)->cx && x1 < AGWIDGET(tv)->cx2 &&
		    y1 < AGWIDGET(tv)->cy2 && y2 > AGWIDGET(tv)->cy) {
			glVertex2i(x1, MIN(y2, AGWIDGET(tv)->cy2));
			glVertex2i(x1, MAX(y1, AGWIDGET(tv)->cy));
		}

		glEnd();
		glLineWidth(saved_width);
#endif /* HAVE_OPENGL */
	}
}

int
RG_TileviewInt(RG_TileviewCtrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_INT_VAL:
		return (ctrl->vals[nval].i);
	case RG_TILEVIEW_INT_PTR:
		return (*(int *)ctrl->vals[nval].p);
	case RG_TILEVIEW_UINT_VAL:
		return ((int)ctrl->vals[nval].ui);
	case RG_TILEVIEW_UINT_PTR:
		return (*(u_int *)ctrl->vals[nval].p);
	default:
		fatal("cannot convert");
	}
}

float
RG_TileviewFloat(RG_TileviewCtrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		return (ctrl->vals[nval].f);
	case RG_TILEVIEW_FLOAT_PTR:
		return (*(float *)ctrl->vals[nval].p);
	case RG_TILEVIEW_DOUBLE_VAL:
		return ((float)ctrl->vals[nval].d);
	case RG_TILEVIEW_DOUBLE_PTR:
		return ((float)(*(double *)ctrl->vals[nval].p));
	default:
		fatal("cannot convert");
	}
}

double
RG_TileviewDouble(RG_TileviewCtrl *ctrl, int nval)
{
	switch (ctrl->valtypes[nval]) {
	case RG_TILEVIEW_FLOAT_VAL:
		return ((double)ctrl->vals[nval].f);
	case RG_TILEVIEW_FLOAT_PTR:
		return ((double)(*(float *)ctrl->vals[nval].p));
	case RG_TILEVIEW_DOUBLE_VAL:
		return (ctrl->vals[nval].d);
	case RG_TILEVIEW_DOUBLE_PTR:
		return (*(double *)ctrl->vals[nval].p);
	default:
		fatal("cannot convert");
	}
}

static void
draw_handle(RG_Tileview *tv, RG_TileviewCtrl *ctrl,
    struct rg_tileview_handle *th)
{
	int x = th->x;
	int y = th->y;

	if (th->over && !th->enable) {
		RG_TileviewSDLColor(tv, &ctrl->cOver, ctrl->aOver);
	} else {
		RG_TileviewSDLColor(tv,
		    th->enable ? &ctrl->cEna : &ctrl->cIna,
		    th->enable ? ctrl->aEna : ctrl->aIna);
	}
	
	/* Draw the inner circle. */
	RG_TileviewRect2(tv, x-1, y-1, 3, 3);

	/* Draw the central point. */
	RG_TileviewColor4i(tv, 255, 255, 255,
	    th->enable ? ctrl->aEna : ctrl->aIna);
	RG_TileviewPixel2i(tv, x, y);

	/* Draw the highlights. */
	RG_TileviewSDLColor(tv,
	    th->enable ? &ctrl->cLow : &ctrl->cHigh,
	    255);
	RG_TileviewPixel2i(tv, x-2, y+1);		/* Highlight left */
	RG_TileviewPixel2i(tv, x-2, y);
	RG_TileviewPixel2i(tv, x-2, y-1);
	RG_TileviewPixel2i(tv, x-1, y-2);		/* Highlight top */
	RG_TileviewPixel2i(tv, x,   y-2);
	RG_TileviewPixel2i(tv, x+1, y-2);
	
	RG_TileviewSDLColor(tv,
	    th->enable ? &ctrl->cHigh : &ctrl->cLow,
	    255);
	RG_TileviewPixel2i(tv, x-1, y+2);		/* Occlusion bottom */
	RG_TileviewPixel2i(tv, x,   y+2);
	RG_TileviewPixel2i(tv, x+1, y+2);
	RG_TileviewPixel2i(tv, x+2, y-1);
	RG_TileviewPixel2i(tv, x+2, y);
	RG_TileviewPixel2i(tv, x+2, y+1);
}

static void
draw_control(RG_Tileview *tv, RG_TileviewCtrl *ctrl)
{
	int i;

	if (tv->flags & RG_TILEVIEW_HIDE_CONTROLS)
		return;

	RG_TileviewSDLColor(tv, &ctrl->c, ctrl->a);

	switch (ctrl->type) {
	case RG_TILEVIEW_RECTANGLE:
	case RG_TILEVIEW_RDIMENSIONS:
		{
			int x = RG_TileviewInt(ctrl, 0);
			int y = RG_TileviewInt(ctrl, 1);
			u_int w = RG_TileviewUint(ctrl, 2);
			u_int h = RG_TileviewUint(ctrl, 3);

			RG_TileviewRect2o(tv, x-1, y-1, w+1, h+1);

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
	case RG_TILEVIEW_POINT:
		{
			int x = RG_TileviewInt(ctrl, 0);
			int y = RG_TileviewInt(ctrl, 1);

			RG_TileviewCircle2o(tv, x, y, 2);
			ctrl->handles[0].x = x;
			ctrl->handles[0].y = y;
		}
		break;
	case RG_TILEVIEW_VERTEX:
		{
			double x = RG_TileviewDouble(ctrl, 0);
			double y = RG_TileviewDouble(ctrl, 1);

			ctrl->handles[0].x = VG_RASXF(ctrl->vg,x);
			ctrl->handles[0].y = VG_RASYF(ctrl->vg,y);

			RG_TileviewCircle2o(tv,
			    ctrl->handles[0].x,
			    ctrl->handles[0].y,
			    2);
		}
		break;
	case RG_TILEVIEW_CIRCLE:
		{
			int x = RG_TileviewInt(ctrl, 0);
			int y = RG_TileviewInt(ctrl, 1);
			u_int r = RG_TileviewUint(ctrl, 2);

			RG_TileviewCircle2o(tv, x, y, r);
			RG_TileviewPixel2i(tv, x, y);

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
RG_TileviewScaledPixel(RG_Tileview *tv, int x, int y, Uint8 r, Uint8 g,
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

	AG_WidgetUpdateSurface(tv, 0);
}

void
RG_TileviewDraw(void *p)
{
	char status[64];
	RG_Tileview *tv = p;
	RG_Tile *t = tv->tile;
	RG_TileviewCtrl *ctrl;
	SDL_Rect rsrc, rdst, rtiling;
	int dxoffs, dyoffs;
	int x, y, n;

	if (t == NULL)
		return;

	if (tv->state == RG_RG_TILEVIEW_SKETCH_EDIT) {
		RG_Sketch *sk = tv->tv_sketch.sk;
		int nredraw = sk->vg->redraw - 1;

		VG_Rasterize(sk->vg);
		t->flags |= RG_TILE_DIRTY;

		/* Multiple tileviews may be rendering the same sketch. */
		sk->vg->redraw = nredraw;
	}
	if (tv->flags & RG_TILEVIEW_READONLY) {
		RG_TileGenerate(t);
		AG_ScaleSurface(t->su, tv->scaled->w, tv->scaled->h,
		    &tv->scaled);
		AG_WidgetUpdateSurface(tv, 0);
	} else {
		if (t->flags & RG_TILE_DIRTY) {
			t->flags &= ~RG_TILE_DIRTY;
			RG_TileGenerate(t);
			AG_ScaleSurface(t->su, tv->scaled->w, tv->scaled->h,
			    &tv->scaled);
			AG_WidgetUpdateSurface(tv, 0);
		}
	}
	rtiling.x = 0;
	rtiling.y = 0;
	rtiling.w = AGWIDGET(tv)->w;
	rtiling.h = AGWIDGET(tv)->h;
	if ((tv->flags & RG_TILEVIEW_NO_TILING) == 0) {
		agPrim.tiling(tv, rtiling, 9, 0,
		    AG_COLOR(TILEVIEW_TILE1_COLOR),
		    AG_COLOR(TILEVIEW_TILE2_COLOR));
	}

	rsrc.x = 0;
	rsrc.y = 0;
	rsrc.w = tv->scaled->w;
	rsrc.h = tv->scaled->h;
	rdst.x = tv->xoffs;
	rdst.y = tv->yoffs;

	if (!agView->opengl) {
		if (tv->xoffs > 0 &&
		    tv->xoffs + tv->scaled->w > AGWIDGET(tv)->w) {
			rsrc.w = AGWIDGET(tv)->w - tv->xoffs;
		} else if (tv->xoffs < 0 && -tv->xoffs < tv->scaled->w) {
			rdst.x = 0;
			rsrc.x = -tv->xoffs;
			rsrc.w = tv->scaled->w - (-tv->xoffs);
			if (rsrc.w > AGWIDGET(tv)->w)
				rsrc.w = AGWIDGET(tv)->w;
		}
		if (tv->yoffs > 0 &&
		    tv->yoffs + tv->scaled->h > AGWIDGET(tv)->h) {
			rsrc.h = AGWIDGET(tv)->h - tv->yoffs;
		} else if (tv->yoffs < 0 && -tv->yoffs < tv->scaled->h) {
			rdst.y = 0;
			rsrc.y = -tv->yoffs;
			rsrc.h = tv->scaled->h - (-tv->yoffs);
			if (rsrc.h > AGWIDGET(tv)->h)
				rsrc.h = AGWIDGET(tv)->h;
		}
	}
	AG_WidgetBlitFrom(tv, tv, 0, &rsrc, rdst.x, rdst.y);

#ifdef HAVE_OPENGL
	if (agView->opengl)
		glEnable(GL_BLEND);
#endif
	RG_TileviewColor4i(tv, 255, 255, 255, 32);
	if ((tv->flags & RG_TILEVIEW_NO_GRID) == 0) {
		for (y = 0; y < t->su->h; y += AGTILESZ)
			RG_TileviewHLine(tv, 0, t->su->w, y);
		for (x = 0; x < t->su->w; x += AGTILESZ)
			RG_TileviewVLine(tv, x, 0, t->su->h);
	}

	RG_TileviewColor4i(tv, 255, 255, 255, 128);
	if (tv->state != RG_TILEVIEW_TILE_EDIT &&
	   (tv->flags & RG_TILEVIEW_NO_EXTENT) == 0) {
		RG_TileviewRect2o(tv, 0, 0, t->su->w-1, t->su->h-1);
	}
	RG_TileviewPixel2i(tv, tv->xms, tv->yms);

	/* XXX inefficient */
	switch (tv->state) {
	case RG_TILEVIEW_ATTRIB_EDIT:
		{
			int tsz = AGTILESZ*tv->pxsz;
			int tw = t->su->w*tv->pxsz;
			int th = t->su->h*tv->pxsz;
			int nx, ny;
		
			n = 0;
			for (y = 0, ny = 0;
			     y < th;
			     y += tsz, ny++) {
				for (x = 0, nx = 0;
				     x < tw;
				     x += tsz, nx++) {
					Uint8 c[4];
					int w = tsz;
					int h = tsz;
					int d;

					AG_NitemAttrColor(tv->edit_attr,
					    (RG_TILE_ATTR2(t,nx,ny) &
					     tv->edit_attr), c);

					if ((d = (tsz - (tw - x))) > 0) {
						w -= d;
					}
					if ((d = (tsz - (th - y))) > 0) {
						h -= d;
					}
					agPrim.rect_blended(tv,
					    tv->xoffs+x,
					    tv->yoffs+y,
					    w, h, c, AG_ALPHA_OVERLAY);
	
					n++;
				}
			}
			
			strlcpy(status, _("Editing node attributes"),
			    sizeof(status));
			draw_status_text(tv, status);
		}
		break;
	case RG_TILEVIEW_LAYERS_EDIT:
		{
			int tsz = AGTILESZ*tv->pxsz;
			int tw = t->su->w*tv->pxsz;
			int th = t->su->h*tv->pxsz;
			char text[16];
			int nx, ny;

			n = 0;
			for (y = 0, ny = 0;
			     y < th;
			     y += tsz, ny++) {
				for (x = 0, nx = 0;
				     x < tw;
				     x += tsz, nx++) {
					SDL_Surface *tsu;
					int l = RG_TILE_LAYER2(t,nx,ny);
					Uint8 c[4] = { 255, 255, 255, 128 };

					snprintf(text, sizeof(text), "%s%d",
					    (l > 0) ? "+" : "", l);
					tsu = AG_TextRender(NULL, 9,
					    AG_COLOR(BG_COLOR), text);
					agPrim.rect_blended(tv,
					    tv->xoffs+x,
					    tv->yoffs+y,
					    tsu->w, tsu->h, c,
					    AG_ALPHA_OVERLAY);
					AG_WidgetBlit(tv, tsu,
					    tv->xoffs+x,
					    tv->yoffs+y);
					
					SDL_FreeSurface(tsu);
				}
			}
			
			strlcpy(status, _("Editing node layers"),
			    sizeof(status));
			draw_status_text(tv, status);
		}
		break;
	case RG_TILEVIEW_FEATURE_EDIT:
		strlcpy(status, _("Editing feature: "), sizeof(status));
		strlcat(status, tv->tv_feature.ft->name, sizeof(status));
		draw_status_text(tv, status);
		break;
	case RG_RG_TILEVIEW_SKETCH_EDIT:
		strlcpy(status, _("Editing sketch: "), sizeof(status));
		strlcat(status, tv->tv_sketch.sk->name, sizeof(status));
		draw_status_text(tv, status);
		break;
	case RG_TILEVIEW_PIXMAP_EDIT:
		{
			extern const char *pixmap_state_names[];

			strlcpy(status, _("Editing pixmap: "), sizeof(status));
			strlcat(status, tv->tv_pixmap.px->name, sizeof(status));
			strlcat(status, pixmap_state_names[tv->tv_pixmap.state],
			    sizeof(status));
			draw_status_text(tv, status);
		}
		break;
	default:
		break;
	}
	
	TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
		draw_control(tv, ctrl);
	}
#ifdef HAVE_OPENGL
	if (agView->opengl)
		glDisable(GL_BLEND);
#endif
}

void
RG_TileviewDestroy(void *p)
{
	RG_Tileview *tv = p;
	RG_TileviewCtrl *ctrl, *nctrl;
	RG_TileviewTool *tool, *ntool;
	
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
	AG_WidgetDestroy(tv);
}

static void
close_tool_win(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	
	AG_ViewDetach(tv->cur_tool->win);
	tv->cur_tool->win = NULL;
}

void
RG_TileviewSelectTool(RG_Tileview *tv, RG_TileviewTool *tvt)
{
	AG_Window *pwin = AG_WidgetParentWindow(tv);

#ifdef DEBUG
	if (pwin == NULL)
		fatal("%s has no parent window", AGOBJECT(tv)->name);
#endif

	if (tv->cur_tool != NULL && tv->cur_tool->win != NULL) {
		AG_ViewDetach(tv->cur_tool->win);
		tv->cur_tool->win = NULL;
	}

	if (tvt->ops->selected != NULL) {
		tvt->ops->selected(tvt);
	}
	if (tvt->ops->edit != NULL) {
		tvt->win = tvt->ops->edit(tvt);
		AG_WindowSetCaption(tvt->win, _(tvt->ops->name));
		AG_WindowSetPosition(tvt->win, AG_WINDOW_LOWER_LEFT, 0);
		AG_WindowAttach(pwin, tvt->win);
		AG_WindowShow(tvt->win);
		AG_SetEvent(tvt->win, "window-close", close_tool_win, "%p", tv);
	}
	
	tv->cur_tool = tvt;
}

void
RG_TileviewUnselectTool(RG_Tileview *tv)
{
	if (tv->cur_tool != NULL) {
		if (tv->cur_tool->win != NULL) {
			AG_ViewDetach(tv->cur_tool->win);
			tv->cur_tool->win = NULL;
		}
		if (tv->cur_tool->ops->unselected != NULL)
			tv->cur_tool->ops->unselected(tv->cur_tool);
	}
	tv->cur_tool = NULL;
}

void
RG_TileviewGenericMenu(RG_Tileview *tv, AG_MenuItem *mi)
{
	AG_MenuIntFlags(mi, _("Show tile grid"), RG_CONTROLS_ICON,
	    &tv->flags, RG_TILEVIEW_NO_GRID, 1);
	AG_MenuIntFlags(mi, _("Show tile extent"), RG_CONTROLS_ICON,
	    &tv->flags, RG_TILEVIEW_NO_EXTENT, 1);
	AG_MenuIntFlags(mi, _("Show controls"), RG_CONTROLS_ICON,
	    &tv->flags, RG_TILEVIEW_HIDE_CONTROLS, 1);
	AG_MenuIntFlags(mi, _("Show background"), SNAP_GRID_ICON,
	    &tv->flags, RG_TILEVIEW_NO_TILING, 1);
}
