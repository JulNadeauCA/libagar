/*
 * Copyright (c) 2005-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <core/config.h>

#include <gui/widget.h>
#include <gui/text.h>
#include <gui/primitive.h>
#include <gui/text_cache.h>
#include <gui/opengl.h>
#include <gui/iconmgr.h>

#include "tileview.h"
#include "icons.h"

#include <stdarg.h>
#include <string.h>

RG_Tileview *
RG_TileviewNew(void *parent, RG_Tileset *ts, Uint flags)
{
	RG_Tileview *tv;

	tv = Malloc(sizeof(RG_Tileview));
	AG_ObjectInit(tv, &rgTileviewClass);
	tv->ts = ts;
	tv->flags |= flags;

	AG_ObjectAttach(parent, tv);
	return (tv);
}

static Uint32
ZoomInTimeout(void *obj, Uint32 ival, void *arg)
{
	RG_Tileview *tv = obj;

	if (tv->zoom > 1600) {
		return (0);
	}
	RG_TileviewSetZoom(tv, tv->zoom+20, 1);
	return (ival);
}

static Uint32
ZoomOutTimeout(void *obj, Uint32 ival, void *arg)
{
	RG_Tileview *tv = obj;

	if (tv->zoom < 100) {
		return (0);
	}
	RG_TileviewSetZoom(tv, tv->zoom-20, 1);
	return (ival);
}

static __inline__ void
MoveCursor(RG_Tileview *tv, int x, int y)
{
	tv->xms = x;
	tv->yms = y;
	tv->xsub = tv->xms%tv->pxsz;
	tv->ysub = tv->yms%tv->pxsz;
	tv->xms -= tv->xsub;
	tv->yms -= tv->ysub;
	tv->xms /= tv->pxsz;
	tv->yms /= tv->pxsz;
	AG_Redraw(tv);
}

static __inline__ int
CursorOver(RG_TileviewHandle *th, int sx, int sy)
{
	return (sx >= th->x - 2 && sx <= th->x + 2 &&
	        sy >= th->y - 2 && sy <= th->y + 2);
}

static void
KeyDown(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	int keysym = AG_INT(1);
	int keymod = AG_INT(2);

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapKeydown(tv, keysym);
		break;
#if 0
	case RG_TILEVIEW_SKETCH_EDIT:
		RG_SketchKeyDown(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
#endif
	default:
		break;
	}

	switch (keysym) {
	case AG_KEY_Z:
		if (keymod & AG_KEYMOD_CTRL) {
			switch (tv->state) {
			case RG_TILEVIEW_PIXMAP_EDIT:
				RG_PixmapUndo(tv, tv->tv_pixmap.tel);
				break;
#if 0
			case RG_TILEVIEW_SKETCH_EDIT:
				RG_SketchUndo(tv, tv->tv_sketch.tel);
				break;
#endif
			default:
				break;
			}
		}
		break;
	case AG_KEY_R:
		if (keymod & AG_KEYMOD_CTRL) {
			switch (tv->state) {
			case RG_TILEVIEW_PIXMAP_EDIT:
				RG_PixmapRedo(tv, tv->tv_pixmap.tel);
				break;
#if 0
			case RG_TILEVIEW_SKETCH_EDIT:
				RG_SketchRedo(tv, tv->tv_sketch.tel);
				break;
#endif
			default:
				break;
			}
		} else {
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case AG_KEY_EQUALS:
		AG_SetTimeout(&tv->zoom_to, ZoomInTimeout, NULL, 0);
		AG_ScheduleTimeout(tv, &tv->zoom_to, 10);
		ZoomInTimeout(tv, 0, NULL);
		break;
	case AG_KEY_MINUS:
		AG_SetTimeout(&tv->zoom_to, ZoomOutTimeout, NULL, 0);
		AG_ScheduleTimeout(tv, &tv->zoom_to, 10);
		ZoomOutTimeout(tv, 0, NULL);
		break;
	case AG_KEY_0:
	case AG_KEY_1:
		RG_TileviewSetZoom(tv, 100, 1);
		break;
	}
}

static void
KeyUp(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	int keysym = AG_INT(1);
/*	int keymod = AG_INT(2); */
	
	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapKeyup(tv);
		break;
#if 0
	case RG_TILEVIEW_SKETCH_EDIT:
		RG_SketchKeyUp(tv, tv->tv_sketch.tel, keysym, keymod);
		break;
#endif
	default:
		break;
	}

	switch (keysym) {
	case AG_KEY_EQUALS:
	case AG_KEY_MINUS:
		AG_DelTimeout(tv, &tv->zoom_to);
		break;
	}
}

static __inline__ int
OverPixmap(RG_TileElement *tel, int x, int y)
{
	return (x >= tel->tel_pixmap.x &&
	        x < tel->tel_pixmap.x + tel->tel_pixmap.px->su->w &&
	        y >= tel->tel_pixmap.y &&
	        y < tel->tel_pixmap.y + tel->tel_pixmap.px->su->h);
}

#if 0
static __inline__ int
OverSketch(RG_TileElement *tel, int x, int y)
{
	return (x >= tel->tel_sketch.x &&
	        x < tel->tel_sketch.x + tel->tel_sketch.sk->vg->su->w &&
	        y >= tel->tel_sketch.y &&
	        y < tel->tel_sketch.y + tel->tel_sketch.sk->vg->su->h);
}
#endif

static void
ToggleAttrib(RG_Tileview *tv, int sx, int sy)
{
	RG_Tile *t = tv->tile;
	int nx = sx/RG_TILESZ;
	int ny = sy/RG_TILESZ;
	Uint *a;
	
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
	AG_Redraw(tv);
}

static void
IncrementLayer(RG_Tileview *tv, int sx, int sy, int inc)
{
	RG_Tile *t = tv->tile;
	int nx = sx/RG_TILESZ;
	int ny = sy/RG_TILESZ;
	int *a;

	if (nx < 0 || nx >= t->nw ||
	    ny < 0 || ny >= t->nh)
		return;

	a = &RG_TILE_LAYER2(t,nx,ny);
	(*a) += inc;

	t->flags |= RG_TILE_DIRTY;
	AG_Redraw(tv);
}

static void
MouseButtonDown(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	RG_TileviewCtrl *ctrl;
	RG_TileElement *tel = tv->tv_pixmap.tel;
	int sx = (x - tv->xoffs)/tv->pxsz;
	int sy = (y - tv->yoffs)/tv->pxsz;
	int i;

	AG_WidgetFocus(tv);

	switch (button) {
	case AG_MOUSE_WHEELUP:
		if (tv->state == RG_TILEVIEW_PIXMAP_EDIT) {
			if (RG_PixmapWheel(tv, tel, 0) == 1)
			return;
#if 0
		} else if (tv->state == RG_TILEVIEW_SKETCH_EDIT) {
			if (RG_SketchWheel(tv, tel, 0) == 1)
				return;
#endif
		}
		RG_TileviewSetZoom(tv,
		    tv->zoom<100 ? tv->zoom+5 : tv->zoom+100, 1);
		MoveCursor(tv, x - tv->xoffs, y - tv->yoffs);
		return;
	case AG_MOUSE_WHEELDOWN:
		if (tv->state == RG_TILEVIEW_PIXMAP_EDIT) {
			if (RG_PixmapWheel(tv, tel, 1) == 1)
				return;
#if 0
		} else if (tv->state == RG_TILEVIEW_SKETCH_EDIT) {
			if (RG_SketchWheel(tv, tel, 1) == 1)
				return;
#endif
		}
		RG_TileviewSetZoom(tv,
		    tv->zoom<100 ? tv->zoom-5 : tv->zoom-100, 1);
		MoveCursor(tv, x - tv->xoffs, y - tv->yoffs);
		break;
	default:
		break;
	}
	
	if (button == AG_MOUSE_LEFT &&
	    (tv->flags & RG_TILEVIEW_HIDE_CONTROLS) == 0) {
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				RG_TileviewHandle *th = &ctrl->handles[i];
		
				if (CursorOver(th, sx, sy)) {
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
					AG_Redraw(tv);
					break;
				}
			}
			if (i < ctrl->nhandles)
				return;
		}
	}

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		if (OverPixmap(tel, sx, sy)) {
			tv->tv_pixmap.xorig = sx - tel->tel_pixmap.x;
			tv->tv_pixmap.yorig = sy - tel->tel_pixmap.y;
			RG_PixmapButtondown(tv, tel, tv->tv_pixmap.xorig,
			    tv->tv_pixmap.yorig, button);
			return;
		} else {
			if (button == AG_MOUSE_RIGHT)
				tv->scrolling++;
		}
		break;
#if 0
	case RG_TILEVIEW_SKETCH_EDIT:
		if (button == AG_MOUSE_RIGHT &&
		   (tv->flags & RG_TILEVIEW_NO_SCROLLING) == 0) {
			tv->scrolling++;
		}
		if (button == AG_MOUSE_MIDDLE || button == AG_MOUSE_RIGHT ||
		   (button == AG_MOUSE_LEFT &&
		    OverSketch(tel, sx, sy)))
		{
			RG_Sketch *sk = tv->tv_sketch.sk;
			float vx, vy;

			vx = VG_VECXF(sk->vg, sx - tel->tel_sketch.x);
			vy = VG_VECYF(sk->vg, sy - tel->tel_sketch.y);
			RG_SketchButtondown(tv, tel, vx, vy, button);
			return;
		}
		break;
#endif
	case RG_TILEVIEW_FEATURE_EDIT:
		if (button == AG_MOUSE_MIDDLE) {
			if (tv->tv_feature.ft->ops->menu != NULL) {
				RG_FeatureOpenMenu(tv,
				    WIDGET(tv)->rView.x1 + x,
				    WIDGET(tv)->rView.y1 + y);
			}
		} else if (button == AG_MOUSE_RIGHT) {
			tv->scrolling++;
		}
		break;
	case RG_TILEVIEW_TILE_EDIT:
		if (button == AG_MOUSE_RIGHT) {
			tv->scrolling++;
		}
		break;
	case RG_TILEVIEW_ATTRIB_EDIT:
		if (button == AG_MOUSE_RIGHT) {
			tv->scrolling++;
		} else if (button ==  AG_MOUSE_LEFT) {
			tv->flags |= RG_TILEVIEW_SET_ATTRIBS;
			tv->tv_attrs.nx = -1;
			tv->tv_attrs.ny = -1;
			ToggleAttrib(tv, sx, sy);
		}
		break;
	case RG_TILEVIEW_LAYERS_EDIT:
		if (button == AG_MOUSE_LEFT) {
			IncrementLayer(tv, sx, sy, +1);
		} else if (button == AG_MOUSE_RIGHT) {
			IncrementLayer(tv, sx, sy, -1);
			tv->scrolling++;
		}
		break;
	}

	if (button == AG_MOUSE_MIDDLE &&
	    tv->state == RG_TILEVIEW_TILE_EDIT) {
		RG_TileOpenMenu(tv,
		    WIDGET(tv)->rView.x1 + x,
		    WIDGET(tv)->rView.y1 + y);
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	int button = AG_INT(1);
	RG_TileviewCtrl *ctrl;
	int i;

	if (button == AG_MOUSE_RIGHT ||
	    button == AG_MOUSE_MIDDLE)
		tv->scrolling = 0;

	switch (button) {
	case AG_MOUSE_RIGHT:
	case AG_MOUSE_MIDDLE:
		tv->scrolling = 0;
		break;
	case AG_MOUSE_LEFT:
		TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
			for (i = 0; i < ctrl->nhandles; i++) {
				RG_TileviewHandle *th = &ctrl->handles[i];

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

					RG_PixmapButtonup(tv, tel,
					    tv->xms - tel->tel_pixmap.x,
					    tv->yms - tel->tel_pixmap.y,
					    button);
				}
				break;
#if 0
			case RG_TILEVIEW_SKETCH_EDIT:
				{
					RG_TileElement *tel =
					    tv->tv_sketch.tel;
					float vx, vy;
				
					VG_Vcoords2(tel->tel_sketch.sk->vg,
					    tv->xms - tel->tel_sketch.x,
					    tv->yms - tel->tel_sketch.y,
					    &vx, &vy);

					RG_SketchButtonup(tv, tel, vx, vy,
					    button);
				}
				break;
#endif
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
ClampOffsets(RG_Tileview *tv)
{
	int lim;

	if (tv->xoffs >
	   (lim = (WIDTH(tv) - RG_TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	} else if (tv->xoffs <
	   (lim = (-((int)tv->scaled->w) + RG_TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	}

	if (tv->yoffs >
	    (lim = (HEIGHT(tv) - RG_TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	} else if (tv->yoffs <
	    (lim = (-((int)tv->scaled->h) + RG_TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	}
}

static void
MoveHandle(RG_Tileview *tv, RG_TileviewCtrl *ctrl, int nhandle,
    int x2, int y2)
{
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
#if 0
	case RG_TILEVIEW_VERTEX:
		RG_TileviewSetDouble(ctrl, 0,
		    RG_TileviewDouble(ctrl, 0)+VG_VECXF(ctrl->vg,dx));
		RG_TileviewSetDouble(ctrl, 1,
		    RG_TileviewDouble(ctrl, 1)+VG_VECYF(ctrl->vg,dy));
		break;
#endif
	default:
		break;
	}

	ctrl->xoffs += xoffs;
	ctrl->yoffs += yoffs;

	if (ctrl->motion != NULL)
		AG_PostEvent(NULL, tv, ctrl->motion->name, "%i,%i",
		    xoffs, yoffs);
	
	AG_Redraw(tv);
}

static void
MouseMotion(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int state = AG_INT(5);
	RG_TileviewCtrl *ctrl;
	int sx, sy, i;

	if (tv->scrolling) {
		tv->xoffs += xrel;
		tv->yoffs += yrel;
		ClampOffsets(tv);
		MoveCursor(tv, x - tv->xoffs, y - tv->yoffs);
		return;
	}

	sx = x - tv->xoffs;
	sy = y - tv->yoffs;
	MoveCursor(tv, sx, sy);
	sx /= tv->pxsz;
	sy /= tv->pxsz;

	TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
		for (i = 0; i < ctrl->nhandles; i++) {
			RG_TileviewHandle *th = &ctrl->handles[i];

			if (th->enable) {
				MoveHandle(tv, ctrl, i, sx, sy);
				break;
			} else {
				th->over = CursorOver(th, sx, sy);
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

				if (OverPixmap(tel, sx, sy)) {
					RG_PixmapMotion(tv, tel,
					    sx - tel->tel_pixmap.x,
					    sy - tel->tel_pixmap.y,
					    sx - tv->xorig,
					    sy - tv->yorig,
					    state);
				}
			}
			break;
#if 0
		case RG_TILEVIEW_SKETCH_EDIT:
			{
				RG_TileElement *tel = tv->tv_sketch.tel;
				float vx, vy, vxrel, vyrel;

				VG_Vcoords2(tel->tel_sketch.sk->vg,
				    sx - tel->tel_sketch.x,
				    sy - tel->tel_sketch.y,
				    &vx, &vy);
				VG_Vcoords2(tel->tel_sketch.sk->vg,
				    sx - tv->xorig,
				    sy - tv->yorig,
				    &vxrel, &vyrel);
				RG_SketchMotion(tv, tel,
				    vx/RG_TILESZ, vy/RG_TILESZ,
				    vxrel/RG_TILESZ, vyrel/RG_TILESZ,
				    state);
			}
			break;
#endif
		case RG_TILEVIEW_ATTRIB_EDIT:
			if (tv->flags & RG_TILEVIEW_SET_ATTRIBS) {
				ToggleAttrib(tv, sx, sy);
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
RedrawTimeout(void *obj, Uint32 ival, void *arg)
{
	RG_Tileview *tv = obj;

	tv->tile->flags |= RG_TILE_DIRTY;
	AG_Redraw(tv);
	return (ival);
}

RG_TileviewTool *
RG_TileviewRegTool(RG_Tileview *tv, const void *p)
{
	const RG_TileviewToolOps *ops = p;
	RG_TileviewTool *tvt;

	tvt = Malloc(ops->len);
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
	AG_Redraw(tv);
}

static void
Init(void *obj)
{
	RG_Tileview *tv = obj;
	
	WIDGET(tv)->flags |= AG_WIDGET_HFILL|AG_WIDGET_VFILL|
	                     AG_WIDGET_FOCUSABLE;
	tv->flags = RG_TILEVIEW_NO_EXTENT|RG_TILEVIEW_NO_TILING;
	tv->ts = NULL;
	tv->tile = NULL;
	tv->scaled = NULL;
	tv->zoom = 100;
	tv->pxsz = 1;
	tv->pxlen = 0;
	tv->xoffs = 0;
	tv->yoffs = 0;
	tv->xms = 0;
	tv->yms = 0;
	tv->scrolling = 0;
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
	tv->tCache = agTextCache ? AG_TextCacheNew(tv, 64, 16) : NULL;
	TAILQ_INIT(&tv->tools);
	TAILQ_INIT(&tv->ctrls);

	AG_WidgetMapSurface(tv, NULL);

	AG_SetTimeout(&tv->redraw_to, RedrawTimeout, NULL, 0);
	
	AG_SetEvent(tv, "key-down", KeyDown, NULL);
	AG_SetEvent(tv, "key-up", KeyUp, NULL);
	AG_SetEvent(tv, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(tv, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tv, "mouse-motion", MouseMotion, NULL);
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

	ctrl = Malloc(sizeof(RG_TileviewCtrl));
	ctrl->type = type;
	ctrl->vals = Malloc(sizeof(union rg_tileview_val) * strlen(fmt));
	ctrl->valtypes = Malloc(sizeof(enum tileview_val_type) * strlen(fmt));
	ctrl->nvals = 0;
#if 0
	ctrl->vg = NULL;
	ctrl->vge = NULL;
#endif
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
				INSERT_VALUE(RG_TILEVIEW_UINT_PTR, p, Uint *,
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
				AG_FatalError("RG_TileviewAddCtrl: Bad format");
			}
			fmt++;
			break;
		case 'i':
			INSERT_VALUE(RG_TILEVIEW_INT_VAL, i, int, int);
			break;
		case 'u':
			INSERT_VALUE(RG_TILEVIEW_UINT_VAL, ui, Uint, int);
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
#if 0
	case RG_TILEVIEW_VERTEX:
		ctrl->nhandles = 1;
		if (ctrl->nvals < 1) goto missingvals;
		break;
#endif
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
	    Malloc(sizeof(RG_TileviewHandle)*ctrl->nhandles) : NULL;
	for (i = 0; i < ctrl->nhandles; i++) {
		RG_TileviewHandle *th = &ctrl->handles[i];

		th->x = -1;
		th->y = -1;
		th->over = 0;
		th->enable = 0;
	}
	AG_Redraw(tv);
	return (ctrl);
missingvals:
	AG_FatalError("RG_TileviewAddCtrl: Missing values");
	return (NULL);
}

static void
FreeCtrl(RG_TileviewCtrl *ctrl)
{
	Free(ctrl->valtypes);
	Free(ctrl->vals);
	Free(ctrl);
}

static void
FreeTool(RG_TileviewTool *t)
{
	if (t->ops->destroy != NULL) {
		t->ops->destroy(t);
	}
	Free(t);
}

void
RG_TileviewDelCtrl(RG_Tileview *tv, RG_TileviewCtrl *ctrl)
{
	TAILQ_REMOVE(&tv->ctrls, ctrl, ctrls);
	FreeCtrl(ctrl);
	AG_Redraw(tv);
}

void
RG_TileviewSetAutoRefresh(RG_Tileview *tv, int ena, int rate)
{
	if (ena) {
		AG_ScheduleTimeout(tv, &tv->redraw_to, rate);
	} else {
		AG_DelTimeout(tv, &tv->redraw_to);
	}
}

void
RG_TileviewSetZoom(RG_Tileview *tv, int z2, int adj_offs)
{
	RG_Tile *t = tv->tile;
	int pxsz1 = tv->pxsz;

	if (z2 < 100 ||
	    z2 > 1600)
		return;

	tv->zoom = z2;
	tv->pxsz = z2/100;
	if (tv->pxsz < 1)
		tv->pxsz = 1;

	tv->scaled = AG_SurfaceRGBA(
	    z2>=100 ? t->su->w*tv->pxsz : t->su->w*z2/100,
	    z2>=100 ? t->su->h*tv->pxsz : t->su->h*z2/100,
	    t->su->format->BitsPerPixel,
	    (t->su->flags & (AG_SRCALPHA|AG_SRCCOLORKEY)),
	    t->su->format->Rmask, t->su->format->Gmask,
	    t->su->format->Bmask, t->su->format->Amask);
	if (tv->scaled == NULL) {
		AG_FatalError(NULL);
	}
	tv->scaled->format->alpha = t->su->format->alpha;
	tv->scaled->format->colorkey = t->su->format->colorkey;
	tv->pxlen = tv->pxsz*tv->scaled->format->BytesPerPixel;
	AG_WidgetReplaceSurface(tv, 0, tv->scaled);

	if (adj_offs) {
		tv->xoffs += (t->su->w*pxsz1 - t->su->w*tv->pxsz)/2;
		tv->yoffs += (t->su->h*pxsz1 - t->su->h*tv->pxsz)/2;
		ClampOffsets(tv);
	}
	t->flags |= RG_TILE_DIRTY;
	AG_Redraw(tv);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	RG_Tileview *tv = obj;

	if (tv->tile != NULL) {
		r->w = tv->tile->su->w + RG_TILEVIEW_MIN_W;
		r->h = tv->tile->su->h + RG_TILEVIEW_MIN_H;
	} else {
		r->w = RG_TILEVIEW_MIN_W;
		r->h = RG_TILEVIEW_MIN_H;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	RG_Tileview *tv = obj;
	int lim;

	if (a->w < RG_TILEVIEW_MIN_W ||
	    a->h < RG_TILEVIEW_MIN_H)
		return (-1);

	if (tv->xoffs > (lim = (a->w - RG_TILEVIEW_MIN_W)))
		tv->xoffs = lim;
	if (tv->yoffs > (lim = (a->h - RG_TILEVIEW_MIN_H)))
		tv->yoffs = lim;

	return (0);
}

/*
 * Plot a "scaled pixel" at x,y.
 * Must be called from widget draw context.
 */
void
RG_TileviewPixel2i(RG_Tileview *tv, int x, int y)
{
	AG_Driver *drv = WIDGET(tv)->drv;
	int x1 = RG_TILEVIEW_SCALED_X(tv,x);
	int y1 = RG_TILEVIEW_SCALED_Y(tv,y);
	int dx, dy;

	if (!(AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)) {
		for (dy = 0; dy < tv->pxsz; dy++) {
			for (dx = 0; dx < tv->pxsz; dx++) {
				int xView = x1+dx;
				int yView = y1+dy;

				if (xView < 0 || yView < 0 ||
				    xView > WIDTH(tv) ||
				    yView > HEIGHT(tv))
					continue;

				if (tv->c.a < 255) {
					AG_BlendPixelRGBA(tv,
					    xView, yView, (Uint8 *)&tv->c,
					    AG_ALPHA_OVERLAY);
				} else {
					AG_PutPixel32(tv,
					    xView, yView,
					    tv->c.pc);
				}
			}
		}
	} else {
#ifdef HAVE_OPENGL
		int x2 = x1 + tv->pxsz;
		int y2 = y1 + tv->pxsz;
		GLboolean svBlendBit;
		GLint svBlendSrc, svBlendDst;

		if (x1 > WIDGET(tv)->rView.x2)	return;
		if (y1 > WIDGET(tv)->rView.y2)	return;
		if (x1 < 0)			x1 = 0;
		if (y1 < 0)			y1 = 0;
		if (x1 >= x2 || y1 >= y2)	return;
		if (x2 > WIDGET(tv)->rView.x2)	x2 = WIDTH(tv);
		if (y2 > WIDGET(tv)->rView.y2)	y2 = HEIGHT(tv);
		
		if (tv->c.a < 255) {
			glGetBooleanv(GL_BLEND, &svBlendBit);
			glGetIntegerv(GL_BLEND_SRC, &svBlendSrc);
			glGetIntegerv(GL_BLEND_DST, &svBlendDst);
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ZERO);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		glBegin(GL_QUADS);
		glColor4ub(tv->c.r, tv->c.g, tv->c.b, tv->c.a);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();
		if (tv->c.a < 255) {
			if (!svBlendBit) {
				glDisable(GL_BLEND);
			}
			glBlendFunc(GL_SRC_ALPHA, svBlendSrc);
			glBlendFunc(GL_DST_ALPHA, svBlendDst);
		}
#endif /* HAVE_OPENGL */
	}
}

static void
DrawStatusText(RG_Tileview *tv, const char *label)
{
	AG_Surface *suTmp = NULL;		/* Make compiler happy */
	int su = -1;
	int wSu, hSu;

	AG_PushTextState();
	AG_TextColor(agColors[TILEVIEW_TEXT_COLOR]);
	if (agTextCache) {
		su = AG_TextCacheGet(tv->tCache, label);
		wSu = WSURFACE(tv,su)->w;
		hSu = WSURFACE(tv,su)->h;
	} else {
		suTmp = AG_TextRender(label);
		wSu = suTmp->w;
		hSu = suTmp->h;
	}
	AG_PopTextState();

	AG_DrawRect(tv,
	    AG_RECT((wSu >= WIDTH(tv)) ? 0 : (WIDTH(tv)-wSu-2),
	            HEIGHT(tv)-hSu-2, WIDTH(tv), HEIGHT(tv)),
	    agColors[TILEVIEW_TEXTBG_COLOR]);

	if (agTextCache) {
		AG_WidgetBlitSurface(tv, su, WIDTH(tv)-wSu-1, HEIGHT(tv)-hSu-1);
	} else {
		AG_WidgetBlit(tv, suTmp, WIDTH(tv)-wSu-1, HEIGHT(tv)-hSu-1);
		AG_SurfaceFree(suTmp);
	}
}

void
RG_TileviewColor3i(RG_Tileview *tv, Uint8 r, Uint8 g, Uint8 b)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.pc = AG_MapPixelRGB(WIDGET(tv)->drv->videoFmt, r,g,b);
}

void
RG_TileviewColor4i(RG_Tileview *tv, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	tv->c.r = r;
	tv->c.g = g;
	tv->c.b = b;
	tv->c.a = a;
	tv->c.pc = AG_MapPixelRGBA(WIDGET(tv)->drv->videoFmt, r,g,b,a);
}

void
RG_TileviewColor(RG_Tileview *tv, AG_Color *c, Uint8 a)
{
	tv->c.r = c->r;
	tv->c.g = c->g;
	tv->c.b = c->b;
	tv->c.a = a;
	tv->c.pc = AG_MapPixelRGBA(WIDGET(tv)->drv->videoFmt,
	    c->r, c->g, c->b, a);
}

void
RG_TileviewAlpha(RG_Tileview *tv, Uint8 a)
{
	tv->c.a = a;
	tv->c.pc = AG_MapPixelRGBA(WIDGET(tv)->drv->videoFmt,
	    tv->c.r, tv->c.g, tv->c.b, a);
}

/* Must be called from widget draw context. */
void
RG_TileviewRect2(RG_Tileview *tv, int x, int y, int w, int h)
{
	AG_Driver *drv = WIDGET(tv)->drv;

	if (!(AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)) {
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
		GLboolean svBlendBit;
		GLint svBlendSrc, svBlendDst;

		if (x1 > WIDTH(tv))		return;
		if (y1 > HEIGHT(tv))		return;
		if (x1 < 0)			x1 = 0;
		if (y1 < 0)			y1 = 0;
		if (x1 >= x2 || y1 >= y2)	return;
		if (x2 > WIDTH(tv))		x2 = WIDTH(tv);
		if (y2 > HEIGHT(tv))		y2 = HEIGHT(tv);

		if (tv->c.a < 255) {
			glGetBooleanv(GL_BLEND, &svBlendBit);
			glGetIntegerv(GL_BLEND_SRC, &svBlendSrc);
			glGetIntegerv(GL_BLEND_DST, &svBlendDst);
			glEnable(GL_BLEND);
			glBlendFunc(GL_DST_ALPHA, GL_ZERO);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		glBegin(GL_QUADS);
		glColor4ub(tv->c.r, tv->c.g, tv->c.b, tv->c.a);
		glVertex2i(x1, y1);
		glVertex2i(x2, y1);
		glVertex2i(x2, y2);
		glVertex2i(x1, y2);
		glEnd();
		if (tv->c.a < 255) {
			if (!svBlendBit) {
				glDisable(GL_BLEND);
			}
			glBlendFunc(GL_SRC_ALPHA, svBlendSrc);
			glBlendFunc(GL_DST_ALPHA, svBlendDst);
		}
#endif /* HAVE_OPENGL */
	}
}

/* Must be called from widget draw context. */
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

/* Must be called from widget draw context. */
void
RG_TileviewVLine(RG_Tileview *tv, int x, int y1, int y2)
{
	int y;

	/* TODO opengl */

	for (y = y1; y < y2; y++)
		RG_TileviewPixel2i(tv, x, y);
}

/* Must be called from widget draw context. */
void
RG_TileviewHLine(RG_Tileview *tv, int x1, int x2, int y)
{
	int x;

	/* TODO opengl */

	for (x = x1; x < x2; x++)
		RG_TileviewPixel2i(tv, x, y);
}

/* Must be called from widget draw context. */
void
RG_TileviewRect2o(RG_Tileview *tv, int x, int y, int w, int h)
{
	AG_Driver *drv = WIDGET(tv)->drv;

	if (!(AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)) {
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

		if (y1 > 0 && y1 < HEIGHT(tv) && x1 < WIDTH(tv) && x2 > 0) {
			glVertex2i(MAX(x1,0), y1);
			glVertex2i(MIN(x2,0), y1);
		}
		if (x2 < WIDTH(tv) && x2 > 0 && y1 < HEIGHT(tv) && y2 > 0) {
			glVertex2i(x2, MAX(y1,0));
			glVertex2i(x2, MIN(y2,HEIGHT(tv)));
		}
		if (y2 > 0 && y2 < HEIGHT(tv) && x1 < WIDTH(tv) && x2 > 0) {
			glVertex2i(MIN(x2,WIDTH(tv)), y2);
			glVertex2i(MAX(x1,0), y2);
		}
		if (x1 > 0 && x1 < WIDTH(tv) && y1 < HEIGHT(tv) && y2 > 0) {
			glVertex2i(x1, MIN(y2,HEIGHT(tv)));
			glVertex2i(x1, MAX(y1,0));
		}

		glEnd();
		glLineWidth(saved_width);
#endif /* HAVE_OPENGL */
	}
}

/* Must be called from widget draw context. */
static void
DrawHandle(RG_Tileview *tv, RG_TileviewCtrl *ctrl, RG_TileviewHandle *th)
{
	int x = th->x;
	int y = th->y;

	if (th->over && !th->enable) {
		RG_TileviewColor(tv, &ctrl->cOver, ctrl->aOver);
	} else {
		RG_TileviewColor(tv,
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
	RG_TileviewColor(tv,
	    th->enable ? &ctrl->cLow : &ctrl->cHigh,
	    255);
	RG_TileviewPixel2i(tv, x-2, y+1);		/* Highlight left */
	RG_TileviewPixel2i(tv, x-2, y);
	RG_TileviewPixel2i(tv, x-2, y-1);
	RG_TileviewPixel2i(tv, x-1, y-2);		/* Highlight top */
	RG_TileviewPixel2i(tv, x,   y-2);
	RG_TileviewPixel2i(tv, x+1, y-2);
	
	RG_TileviewColor(tv,
	    th->enable ? &ctrl->cHigh : &ctrl->cLow,
	    255);
	RG_TileviewPixel2i(tv, x-1, y+2);		/* Occlusion bottom */
	RG_TileviewPixel2i(tv, x,   y+2);
	RG_TileviewPixel2i(tv, x+1, y+2);
	RG_TileviewPixel2i(tv, x+2, y-1);
	RG_TileviewPixel2i(tv, x+2, y);
	RG_TileviewPixel2i(tv, x+2, y+1);
}

/* Must be called from widget draw context. */
static void
DrawControl(RG_Tileview *tv, RG_TileviewCtrl *ctrl)
{
	int i;

	if (tv->flags & RG_TILEVIEW_HIDE_CONTROLS)
		return;

	RG_TileviewColor(tv, &ctrl->c, ctrl->a);

	switch (ctrl->type) {
	case RG_TILEVIEW_RECTANGLE:
	case RG_TILEVIEW_RDIMENSIONS:
		{
			int x = RG_TileviewInt(ctrl, 0);
			int y = RG_TileviewInt(ctrl, 1);
			Uint w = RG_TileviewUint(ctrl, 2);
			Uint h = RG_TileviewUint(ctrl, 3);

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
#if 0
		{
			float x = RG_TileviewDouble(ctrl, 0);
			float y = RG_TileviewDouble(ctrl, 1);

			ctrl->handles[0].x = VG_RASXF(ctrl->vg,x);
			ctrl->handles[0].y = VG_RASYF(ctrl->vg,y);

			RG_TileviewCircle2o(tv,
			    ctrl->handles[0].x,
			    ctrl->handles[0].y,
			    2);
		}
#endif
		break;
	case RG_TILEVIEW_CIRCLE:
		{
			int x = RG_TileviewInt(ctrl, 0);
			int y = RG_TileviewInt(ctrl, 1);
			Uint r = RG_TileviewUint(ctrl, 2);

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
		DrawHandle(tv, ctrl, &ctrl->handles[i]);
}

static void
GetAttrColor(Uint flag, int state, AG_Color *c)
{
	switch (flag) {
	case RG_TILE_BLOCK:
		if (state) {
			*c = AG_ColorRGBA(255,0,0,64);
		} else {
			*c = AG_ColorRGBA(0,255,0,32);
		}
		break;
	case RG_TILE_CLIMBABLE:
		if (state) {
			*c = AG_ColorRGBA(255,255,0,64);
		} else {
			*c = AG_ColorRGBA(255,0,0,32);
		}
		break;
	case RG_TILE_SLIPPERY:
		if (state) {
			*c = AG_ColorRGBA(0,0,255,64);
		} else {
			*c = AG_ColorRGBA(0,0,0,0);
		}
		break;
	case RG_TILE_JUMPABLE:
		if (state) {
			*c = AG_ColorRGBA(255,0,255,64);
		} else {
			*c = AG_ColorRGBA(0,0,0,0);
		}
		break;
	}
}

static void
Draw(void *obj)
{
	RG_Tileview *tv = obj;
	AG_Driver *drv = WIDGET(tv)->drv;
	RG_Tile *t = tv->tile;
	char status[64];
	RG_TileviewCtrl *ctrl;
	AG_Rect rsrc, rdst;
	int x, y, n;

	if (t == NULL)
		return;

	AG_PushClipRect(tv, AG_RECT(0, 0, WIDTH(tv), HEIGHT(tv)));
#if 0
	if (tv->state == RG_TILEVIEW_SKETCH_EDIT) {
		VG_Rasterize(rv->tv_sketch.sk->vg);
		t->flags |= RG_TILE_DIRTY;
	}
#endif
	if (tv->flags & RG_TILEVIEW_READONLY) {
		RG_TileGenerate(t);
		if (AG_ScaleSurface(t->su, tv->scaled->w, tv->scaled->h,
		    &tv->scaled) == -1) {
			AG_FatalError(NULL);
		}
		AG_WidgetUpdateSurface(tv, 0);
	} else {
		if (t->flags & RG_TILE_DIRTY) {
			t->flags &= ~RG_TILE_DIRTY;
			RG_TileGenerate(t);
			if (AG_ScaleSurface(t->su, tv->scaled->w, tv->scaled->h,
			    &tv->scaled) == -1) {
				AG_FatalError(NULL);
			}
			AG_WidgetUpdateSurface(tv, 0);
		}
	}
	if ((tv->flags & RG_TILEVIEW_NO_TILING) == 0) {
		AG_DrawTiling(tv,
		    AG_RECT(0, 0, WIDTH(tv), HEIGHT(tv)), 9, 0,
		    agColors[TILEVIEW_TILE1_COLOR],
		    agColors[TILEVIEW_TILE2_COLOR]);
	}

	rsrc.x = 0;
	rsrc.y = 0;
	rsrc.w = tv->scaled->w;
	rsrc.h = tv->scaled->h;
	rdst.x = tv->xoffs;
	rdst.y = tv->yoffs;

	if (!(AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)) {
		if (tv->xoffs > 0 &&
		    tv->xoffs + tv->scaled->w > WIDTH(tv)) {
			rsrc.w = WIDTH(tv) - tv->xoffs;
		} else if (tv->xoffs < 0 && -tv->xoffs < tv->scaled->w) {
			rdst.x = 0;
			rsrc.x = -tv->xoffs;
			rsrc.w = tv->scaled->w - (-tv->xoffs);
			if (rsrc.w > WIDTH(tv))
				rsrc.w = WIDTH(tv);
		}
		if (tv->yoffs > 0 &&
		    tv->yoffs + tv->scaled->h > HEIGHT(tv)) {
			rsrc.h = HEIGHT(tv) - tv->yoffs;
		} else if (tv->yoffs < 0 && -tv->yoffs < tv->scaled->h) {
			rdst.y = 0;
			rsrc.y = -tv->yoffs;
			rsrc.h = tv->scaled->h - (-tv->yoffs);
			if (rsrc.h > HEIGHT(tv))
				rsrc.h = HEIGHT(tv);
		}
	}
	AG_WidgetBlitFrom(tv, tv, 0, &rsrc, rdst.x, rdst.y);

#ifdef HAVE_OPENGL
	if (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)
		glEnable(GL_BLEND);
#endif
	RG_TileviewColor4i(tv, 255, 255, 255, 32);
	if ((tv->flags & RG_TILEVIEW_NO_GRID) == 0) {
		for (y = 0; y < t->su->h; y += RG_TILESZ)
			RG_TileviewHLine(tv, 0, t->su->w, y);
		for (x = 0; x < t->su->w; x += RG_TILESZ)
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
			int tsz = RG_TILESZ*tv->pxsz;
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
					AG_Color c = AG_ColorRGBA(0,0,0,0);
					int w = tsz;
					int h = tsz;
					int d;

					GetAttrColor(tv->edit_attr,
					    (RG_TILE_ATTR2(t,nx,ny) &
					     tv->edit_attr), &c);

					if ((d = (tsz - (tw - x))) > 0) {
						w -= d;
					}
					if ((d = (tsz - (th - y))) > 0) {
						h -= d;
					}
					AG_DrawRectBlended(tv,
					    AG_RECT(tv->xoffs+x,
					            tv->yoffs+y,
					            w, h),
					    c, AG_ALPHA_OVERLAY);

					n++;
				}
			}
			
			Strlcpy(status, _("Editing node attributes"),
			    sizeof(status));
			DrawStatusText(tv, status);
		}
		break;
	case RG_TILEVIEW_LAYERS_EDIT:
		{
			int tsz = RG_TILESZ*tv->pxsz;
			int tw = t->su->w*tv->pxsz;
			int th = t->su->h*tv->pxsz;
			char text[16];
			int nx, ny;

			AG_PushTextState();
			n = 0;
			for (y = 0, ny = 0;
			     y < th;
			     y += tsz, ny++) {
				for (x = 0, nx = 0;
				     x < tw;
				     x += tsz, nx++) {
					AG_Surface *tsu;
					int l = RG_TILE_LAYER2(t,nx,ny);

					Strlcpy(text, (l > 0) ? "+" : "",
					    sizeof(text));
					StrlcatInt(text, l, sizeof(text));
					AG_TextColorRGB(0,0,0);
					tsu = AG_TextRender(text);
					AG_DrawRectBlended(tv,
					    AG_RECT(tv->xoffs+x,
					            tv->yoffs+y,
					            tsu->w, tsu->h),
					    AG_ColorRGBA(255,255,255,128),
					    AG_ALPHA_OVERLAY);
					AG_WidgetBlit(tv, tsu,
					    tv->xoffs+x,
					    tv->yoffs+y);
					
					AG_SurfaceFree(tsu);
				}
			}
			
			Strlcpy(status, _("Editing node layers"),
			    sizeof(status));
			DrawStatusText(tv, status);
			AG_PopTextState();
		}
		break;
	case RG_TILEVIEW_FEATURE_EDIT:
		Strlcpy(status, _("Editing feature: "), sizeof(status));
		Strlcat(status, tv->tv_feature.ft->name, sizeof(status));
		DrawStatusText(tv, status);
		break;
#if 0
	case RG_TILEVIEW_SKETCH_EDIT:
		Strlcpy(status, _("Editing sketch: "), sizeof(status));
		Strlcat(status, tv->tv_sketch.sk->name, sizeof(status));
		DrawStatusText(tv, status);
		break;
#endif
	case RG_TILEVIEW_PIXMAP_EDIT:
		{
			extern const char *pixmap_state_names[];

			Strlcpy(status, _("Editing pixmap: "), sizeof(status));
			Strlcat(status, tv->tv_pixmap.px->name, sizeof(status));
			Strlcat(status, pixmap_state_names[tv->tv_pixmap.state],
			    sizeof(status));
			DrawStatusText(tv, status);
		}
		break;
	default:
		break;
	}
	
	TAILQ_FOREACH(ctrl, &tv->ctrls, ctrls) {
		DrawControl(tv, ctrl);
	}
#ifdef HAVE_OPENGL
	if (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL)
		glDisable(GL_BLEND);
#endif
	AG_PopClipRect(tv);
}

static void
Destroy(void *p)
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
		FreeCtrl(ctrl);
	}
	for (tool = TAILQ_FIRST(&tv->tools);
	     tool != TAILQ_END(&tv->tools);
	     tool = ntool) {
		ntool = TAILQ_NEXT(tool, tools);
		FreeTool(tool);
	}
	if (tv->tCache != NULL)
		AG_TextCacheDestroy(tv->tCache);
}

static void
CloseToolWindow(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	
	AG_ObjectDetach(tv->cur_tool->win);
	tv->cur_tool->win = NULL;
}

void
RG_TileviewSelectTool(RG_Tileview *tv, RG_TileviewTool *tvt)
{
	AG_Window *pwin = AG_ParentWindow(tv);

#ifdef AG_DEBUG
	if (pwin == NULL)
		AG_FatalError("RG_TileviewSelectTool: %s has no parent window",
		    OBJECT(tv)->name);
#endif

	if (tv->cur_tool != NULL && tv->cur_tool->win != NULL) {
		AG_ObjectDetach(tv->cur_tool->win);
		tv->cur_tool->win = NULL;
	}

	if (tvt->ops->selected != NULL) {
		tvt->ops->selected(tvt);
	}
	if (tvt->ops->edit != NULL) {
		tvt->win = tvt->ops->edit(tvt);
		AG_WindowSetCaptionS(tvt->win, _(tvt->ops->name));
		AG_WindowSetPosition(tvt->win, AG_WINDOW_LOWER_LEFT, 0);
		AG_WindowAttach(pwin, tvt->win);
		AG_WindowShow(tvt->win);
		AG_SetEvent(tvt->win, "window-close", CloseToolWindow,
		    "%p", tv);
	}
	
	tv->cur_tool = tvt;
	AG_Redraw(tv);
}

void
RG_TileviewUnselectTool(RG_Tileview *tv)
{
	if (tv->cur_tool != NULL) {
		if (tv->cur_tool->win != NULL) {
			AG_ObjectDetach(tv->cur_tool->win);
			tv->cur_tool->win = NULL;
		}
		if (tv->cur_tool->ops->unselected != NULL)
			tv->cur_tool->ops->unselected(tv->cur_tool);
	}
	tv->cur_tool = NULL;
	AG_Redraw(tv);
}

void
RG_TileviewGenericMenu(RG_Tileview *tv, AG_MenuItem *mi)
{
	AG_MenuIntFlags(mi, _("Show tile grid"), rgIconControls.s,
	    &tv->flags, RG_TILEVIEW_NO_GRID, 1);
	AG_MenuIntFlags(mi, _("Show tile extent"), rgIconControls.s,
	    &tv->flags, RG_TILEVIEW_NO_EXTENT, 1);
	AG_MenuIntFlags(mi, _("Show controls"), rgIconControls.s,
	    &tv->flags, RG_TILEVIEW_HIDE_CONTROLS, 1);
	AG_MenuIntFlags(mi, _("Show background"), rgIconTiling.s,
	    &tv->flags, RG_TILEVIEW_NO_TILING, 1);
}

AG_WidgetClass rgTileviewClass = {
	{
		"Agar(Widget):RG(Tileview)",
		sizeof(RG_Tileview),
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
