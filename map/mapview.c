/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/label.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/statusbar.h>

#include <agar/map/map.h>
#include <agar/map/nodesel.h>
#include <agar/map/refsel.h>

#include "rg_icons.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

enum {
	ZOOM_MIN =	 20,	/* Min zoom factor (%) */
	ZOOM_MAX =	 500,	/* Max zoom factor (%) */
	ZOOM_PROPS_MIN = 60,	/* Min zoom factor for showing properties (%) */
	ZOOM_GRID_MIN =	 20	/* Min zoom factor for showing the grid (%) */
};

int mapViewBg = 1;		/* Background tiles enable */
int mapViewBgTileSize = 8;	/* Background tile size */
int mapViewEditSelOnly = 0;	/* Restrict edition to selection */
int mapViewZoomInc = 8;

static void
UpdateCamera(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);

	MAP_ViewUpdateCamera(mv);
}

MAP_View *
MAP_ViewNew(void *parent, MAP *map, Uint flags, struct ag_toolbar *toolbar,
    struct ag_statusbar *statbar)
{
	MAP_View *mv;

	mv = Malloc(sizeof(MAP_View));
	AG_ObjectInit(mv, &mapViewClass);
	mv->flags |= flags;
	mv->map = map;
	mv->toolbar = toolbar;
	mv->statusbar = statbar;
	mv->mx = map->xOrigin;
	mv->my = map->yOrigin;

	if (statbar != NULL) {
		mv->statusbar = statbar;
		mv->status = AG_StatusbarAddLabel(statbar, "...");
	}
	if ((flags & MAP_VIEW_NO_SCROLLBARS) == 0) {
		AG_Scrollbar *sb;

		sb = mv->hbar = AG_ScrollbarNewHoriz(mv, 0);
		WIDGET(sb)->flags &= ~(AG_WIDGET_FOCUSABLE);
		WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_MOTION;
		AG_BindInt(sb, "value", &AGMCAM(mv).x);
		AG_BindUint(sb, "visible", &mv->wVis);
		AG_SetInt(sb, "inc", MAPTILESZ);
		AG_SetEvent(sb, "scrollbar-changed", UpdateCamera, "%p", mv);

		sb = mv->vbar = AG_ScrollbarNewVert(mv, 0);
		WIDGET(sb)->flags &= ~(AG_WIDGET_FOCUSABLE);
		WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_MOTION;
		AG_BindInt(sb, "value", &AGMCAM(mv).y);
		AG_BindUint(sb, "visible", &mv->hVis);
		AG_SetInt(sb, "inc", MAPTILESZ);
		AG_SetEvent(sb, "scrollbar-changed", UpdateCamera, "%p", mv);
		
		/* "min" and "max" are set by MAP_ViewUpdateCamera() */
	}

	AG_ObjectAttach(parent, mv);
	return (mv);
}

void
MAP_ViewControl(MAP_View *mv, const char *slot, void *obj)
{
#ifdef AG_DEBUG
	if (!AG_OfClass(obj, "MAP_Actor:*"))
		AG_FatalError(NULL);
#endif
	mv->actor = (MAP_Actor *)obj;
}

void
MAP_ViewSelectTool(MAP_View *mv, MAP_Tool *ntool, void *p)
{
	AG_Window *pwin;

	if (mv->curtool != NULL) {
		if (mv->curtool->trigger != NULL) {
			AG_SetBool(mv->curtool->trigger, "state", 0);
		}
		if (mv->curtool->win != NULL) {
			AG_WindowHide(mv->curtool->win);
		}
		if (mv->curtool->pane != NULL) {
			AG_ObjectFreeChildren(mv->curtool->pane);
			AG_WidgetUpdate(mv->curtool->pane);
		}
		mv->curtool->mv = NULL;

		AG_LabelTextS(mv->status,
		    _("Select a tool or double-click on an element to insert."));
	}
	mv->curtool = ntool;

	if (ntool != NULL) {
		ntool->p = p;
		ntool->mv = mv;

		if (ntool->trigger != NULL) {
			AG_SetBool(ntool->trigger, "state", 1);
		}
		if (ntool->win != NULL) {
			AG_WindowShow(ntool->win);
		}
		if (ntool->pane != NULL && ntool->ops->edit_pane != NULL) {
			ntool->ops->edit_pane(ntool, ntool->pane);
			AG_WidgetUpdate(mv->curtool->pane);
		}
		MAP_ToolUpdateStatus(ntool);
	}

	if ((pwin = AG_ParentWindow(mv)) != NULL) {
		AG_WindowFocus(pwin);
		AG_WidgetFocus(mv);
	}
}

MAP_Tool *
MAP_ViewFindTool(MAP_View *mv, const char *name)
{
	MAP_Tool *tool;

	TAILQ_FOREACH(tool, &mv->tools, tools) {
		if (strcmp(tool->ops->name, name) == 0)
			return (tool);
	}
	return (NULL);
}

static void
SelectTool(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_PTR(1);
	MAP_Tool *tool = AG_PTR(2);
	void *p = AG_PTR(3);

	if (mv->curtool == tool) {
		MAP_ViewSelectTool(mv, NULL, NULL);
	} else {
		MAP_ViewSelectTool(mv, tool, p);
	}
}

MAP_Tool *
MAP_ViewRegTool(MAP_View *mv, const MAP_ToolOps *ops, void *p)
{
	MAP_Tool *t;

	t = Malloc(ops->len);
	t->ops = ops;
	t->mv = mv;
	t->p = p;
	MAP_ToolInit(t);

	if (((ops->flags & TOOL_HIDDEN) == 0) && mv->toolbar != NULL) {
		t->trigger = AG_ToolbarButtonIcon(mv->toolbar,
		    (ops->icon != NULL) ? ops->icon->s : NULL,
		    0, SelectTool, "%p,%p,%p", mv, t, p);
	}

	TAILQ_INSERT_TAIL(&mv->tools, t, tools);
	return (t);
}

void
MAP_ViewSetDefaultTool(MAP_View *mv, MAP_Tool *tool)
{
	mv->deftool = tool;
}

void
MAP_ViewRegDrawCb(MAP_View *mv,
    void (*draw_func)(MAP_View *, void *), void *p)
{
	MAP_ViewDrawCb *dcb;

	dcb = Malloc(sizeof(MAP_ViewDrawCb));
	dcb->func = draw_func;
	dcb->p = p;
	SLIST_INSERT_HEAD(&mv->draw_cbs, dcb, draw_cbs);
}

static void
Destroy(void *_Nonnull p)
{
	MAP_View *mv = p;
	MAP_ViewDrawCb *dcb, *ndcb;

	for (dcb = SLIST_FIRST(&mv->draw_cbs);
	     dcb != SLIST_END(&mv->draw_cbs);
	     dcb = ndcb) {
		ndcb = SLIST_NEXT(dcb, draw_cbs);
		Free(dcb);
	}
}

static void
OnDetach(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP_Tool *tool, *ntool;

	for (tool = TAILQ_FIRST(&mv->tools);
	     tool != TAILQ_END(&mv->tools);
	     tool = ntool) {
		ntool = TAILQ_NEXT(tool, tools);
		MAP_ToolDestroy(tool);
		Free(tool);
	}
	TAILQ_INIT(&mv->tools);
	mv->curtool = NULL;
}

static void
ExpireDblClick(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();

	mv->dblclicked = 0;
}

/*
 * Translate widget coordinates to node coordinates.
 * The map must be locked.
 */
static void
GetNodeCoords(MAP_View *_Nonnull mv, int *_Nonnull x, int *_Nonnull y)
{
	const int tileSz = AGMCAM(mv).tilesz;

	*x -= mv->xOffs;
	*y -= mv->yOffs;
	mv->cxoffs = *x % tileSz;
	mv->cyoffs = *y % tileSz;

	*x = ((*x) - mv->cxoffs) / tileSz;
	*y = ((*y) - mv->cyoffs) / tileSz;

	mv->cx = mv->mx + *x;
	mv->cy = mv->my + *y;

	if (mv->cx < 0 || mv->cx >= (int)mv->map->w || mv->cxoffs < 0)
		mv->cx = -1;
	if (mv->cy < 0 || mv->cy >= (int)mv->map->h || mv->cyoffs < 0)
		mv->cy = -1;
}

static void
DrawMapCursor(MAP_View *_Nonnull mv)
{
	AG_Rect rd;
	AG_Color c;

	rd.w = AGMTILESZ(mv);
	rd.h = AGMTILESZ(mv);

	if (mv->msel.set) {
#if 0
		/* XXX */
		AG_MouseGetState(&msx, &msy);
		if (!agView->opengl)
			AG_SurfaceBlit(mapIconCursor.s, NULL, agView->v,
			    msx, msy);
#endif
		return;
	}

	rd.x = mv->mouse.x*AGMTILESZ(mv) + mv->xOffs;
	rd.y = mv->mouse.y*AGMTILESZ(mv) + mv->yOffs;

	if (mv->curtool == NULL)
		return;

	if (mv->curtool->ops->cursor != NULL) {
		if (mv->curtool->ops->cursor(mv->curtool, &rd) == -1)
			goto defcurs;
	}
	return;
defcurs:
	AG_ColorRGB_8(&c, 100,100,100);
	rd.x++;
	rd.y++;
	rd.w = rd.h = AGMTILESZ(mv)-1;
	AG_DrawRectOutline(mv, &rd, &c);
	rd.x++;
	rd.y++;
	rd.w = rd.h = AGMTILESZ(mv)-3;
	AG_DrawRectOutline(mv, &rd, &c);
}

static void
CenterToOrigin(MAP_View *_Nonnull mv)
{
	AGMCAM(mv).x = mv->map->xOrigin*AGMTILESZ(mv) - AGMTILESZ(mv)/2;
	AGMCAM(mv).y = mv->map->yOrigin*AGMTILESZ(mv) - AGMTILESZ(mv)/2;
	MAP_ViewUpdateCamera(mv);
}

/*
 * Render a graphical map item to absolute view coordinates rx,ry.
 * The map must be locked. Must be called from widget draw context only.
 */
static void
DrawItem(MAP_View *_Nonnull mv, MAP *_Nonnull map, MAP_Item *_Nonnull mi,
    int rx, int ry, int ncam)
{
	const MAP_ItemClass *miClass = mapItemClasses[mi->type];

	if (miClass->draw != NULL)
		miClass->draw(mv, mi, rx,ry, ncam);
}

/* Render a gimp-style background tiling. */
static void
DrawTiling(void *_Nonnull obj, const AG_Rect *_Nonnull r, int tsz, int offs,
    const AG_Color *_Nonnull c1, const AG_Color *_Nonnull c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_Rect rd;
	int alt1 = 0, alt2 = 0;
	const int x = wid->rView.x1 + r->x;
	const int y = wid->rView.y1 + r->y;
	const int x2 = x + r->w;
	const int y2 = y + r->h;
	const int tsz_offs = tsz+offs;

	rd.w = tsz+1;
	rd.h = tsz+1;

	/* XXX inelegant */
	for (rd.y = y-tsz_offs; rd.y < y2; rd.y += tsz) {
		for (rd.x = x-tsz_offs; rd.x < x2; rd.x += tsz) {
			if (alt1++ == 1) {
				wid->drvOps->drawRectFilled(drv, &rd, c1);
				alt1 = 0;
			} else {
				wid->drvOps->drawRectFilled(drv, &rd, c2);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}

static void
Draw(void *_Nonnull obj)
{
	MAP_View *mv = obj;
	MAP_ViewDrawCb *dcb;
	MAP *map = mv->map;
	MAP_Node *node;
	MAP_Item *mi;
	int mx, my, rx = 0, ry = 0;
	int layer = 0, tileSz;
	AG_Rect r, rSel, mSel, rExtent;
	AG_Color c, c2;

	rSel.x = -1; mSel.x = -1;
	rSel.y = -1; mSel.y = -1;
	rSel.w = -1; mSel.w = -1;
	rSel.h = -1; mSel.h = -1;

	AG_PushClipRect(mv, &mv->r);

	if (WIDGET(mv)->flags & AG_WIDGET_FOCUSED) {
		AG_ColorRGB_8(&c, 150,150,150);
		AG_DrawRectOutline(mv, &mv->r, &c);
	}

	SLIST_FOREACH(dcb, &mv->draw_cbs, draw_cbs)
		dcb->func(mv, dcb->p);
	
	if (mv->flags & MAP_VIEW_CENTER) {
		mv->flags &= ~(MAP_VIEW_CENTER);
		CenterToOrigin(mv);
	}

	if ((mv->flags & MAP_VIEW_NO_BG) == 0) {
		r.x = 0;
		r.y = 0;
		r.w = WIDTH(mv);
		r.h = HEIGHT(mv);
		AG_ColorRGB_8(&c, 50,50,50);
		AG_ColorRGB_8(&c2, 40,40,40);
		DrawTiling(mv, &r, mapViewBgTileSize, 0, &c, &c2);
	}

	AG_ObjectLock(map);

	if (map->map == NULL) {
		goto out;
	}
	tileSz = AGMTILESZ(mv);
draw_layer:
	if (!map->layers[layer].visible) {
		goto next_layer;
	}
	for (my = mv->my, ry = mv->yOffs;
	     ((my - mv->my) <= (int)mv->mh) && (my < (int)map->h);
	     my++, ry += tileSz) {

		for (mx = mv->mx, rx = mv->xOffs;
	     	     ((mx - mv->mx) <= (int)mv->mw) && (mx < (int)map->w);
		     mx++, rx += tileSz) {

			node = &map->map[my][mx];

			TAILQ_FOREACH(mi, &node->items, items) {
				if (mi->layer != layer)
					continue;

				DrawItem(mv, map, mi, rx,ry, mv->cam);

				if ((mi->layer == map->layerCur) &&
				    (mv->mode == MAP_VIEW_EDIT_ATTRS)) {
					MAP_ItemAttrColor(mv->edit_attr,
					    (mi->flags & mv->edit_attr), &c);
					r.x = rx;
					r.y = ry;
					r.w = tileSz;
					r.h = tileSz;
					AG_DrawRectBlended(mv, &r, &c,
					    AG_ALPHA_SRC,
					    AG_ALPHA_ONE_MINUS_SRC);
				}

				if ((mi->flags & MAP_ITEM_SELECTED) &&
				    MAP_ItemExtent(map, mi, &rExtent, mv->cam) == 0) {
					r.x = rx + rExtent.x - 1;
					r.y = ry + rExtent.y - 1;
					r.w = rExtent.w + 1;
					r.h = rExtent.h + 1;
					AG_ColorRGB_8(&c, 60,250,60);
					AG_DrawRectOutline(mv, &r, &c);
				}
			}
			if ((mv->flags & MAP_VIEW_EDIT) == 0)
				continue;
				
			if ((mv->flags & MAP_VIEW_SHOW_ORIGIN) &&
			    (mx == map->xOrigin && my == map->yOrigin)) {
				const int t2 = tileSz >> 1;
				const int rx2 = rx + tileSz;
				const int ry2 = ry + tileSz;
				const int rxt2 = rx+t2;
				const int ryt2 = ry+t2;
			
				AG_ColorRGB_8(&c, 150,150,0);
				AG_DrawCircle(mv, rxt2, ryt2, t2, &c);
				AG_DrawLine(mv, rxt2, ry,   rxt2, ry2,  &c);
				AG_DrawLine(mv, rx,   ryt2, rx2,  ryt2, &c);
			}
			if (mv->msel.set &&
			    mv->msel.x == mx && mv->msel.y == my) {
				mSel.x = rx + 1;
				mSel.y = ry + 1;
				mSel.w = mv->msel.xOffs * tileSz - 2;
				mSel.h = mv->msel.yOffs * tileSz - 2;
			}
			if (mv->esel.set &&
			    mv->esel.x == mx && mv->esel.y == my) {
				rSel.x = rx;
				rSel.y = ry;
				rSel.w = tileSz * mv->esel.w;
				rSel.h = tileSz * mv->esel.h;
			}
		}
	}
next_layer:
	if (++layer < (int)map->nLayers)
		goto draw_layer;			/* Draw next layer */

	if (mv->flags & MAP_VIEW_GRID) {
		int rx2 = rx;

		for (; ry >= mv->yOffs; ry -= tileSz) {
			AG_DrawLineBlended(mv,
			    mv->xOffs,     /* x1 */
			    ry,            /* y1 */
			    rx2,           /* x2 */
			    ry,            /* y2 */
			    &mv->color,
			    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

			for (; rx >= mv->xOffs; rx -= tileSz) {
				AG_DrawLineBlended(mv,
				    rx,            /* x1 */
				    mv->yOffs,     /* y1 */
				    rx,            /* x2 */
				    ry,            /* y2 */
				    &mv->color,
				    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
			}
		}
	}

	if (rSel.x != -1) {				/* Active selection */
		AG_ColorRGB_8(&c, 180,180,180);
		AG_DrawRectOutline(mv, &rSel, &c);
	}
	if (mSel.x != -1) {				/* Mouse selection */
		AG_ColorRGB_8(&c, 150,150,150);
		AG_DrawRectOutline(mv, &mSel, &c);
	}

	/* Draw the cursor for the current tool. */
	if ((mv->flags & MAP_VIEW_EDIT) && (mv->mode == MAP_VIEW_EDITION) &&
	    (mv->flags & MAP_VIEW_NO_CURSOR) == 0 &&
	    (mv->cx != -1 && mv->cy != -1)) {
		DrawMapCursor(mv);
	}
out:
	AG_ObjectUnlock(map);
	AG_PopClipRect(mv);

	if (mv->hbar) { AG_WidgetDraw(mv->hbar); }
	if (mv->vbar) { AG_WidgetDraw(mv->vbar); }
}

/*
 * Recalculate the offsets to be used by the rendering routine based on
 * the current camera coordinates. 
 */
void
MAP_ViewUpdateCamera(MAP_View *mv)
{
	MAP *map = mv->map;
	MAP_Camera *cam;
	int xCam, yCam, xMax, yMax, tileSz;

	AG_ObjectLock(map);
	cam = &AGMCAM(mv);
	tileSz = AGMTILESZ(mv);
	
	if (cam->x < 0) {
		cam->x = 0;
	} else if (cam->x > (xMax = (int)map->w * tileSz)) { 
		cam->x = xMax;
	}
	if (cam->y < 0) {
		cam->y = 0;
	} else if (cam->y > (yMax = (int)map->h * tileSz)) { 
		cam->y = yMax;
	}

	xCam = cam->x;
	yCam = cam->y;

	switch (cam->alignment) {
	case AG_MAP_CENTER:
		xCam -= WIDTH(mv)  >> 1;
		yCam -= HEIGHT(mv) >> 1;
		break;
	case AG_MAP_LOWER_CENTER:
		xCam -= WIDTH(mv) >> 1;
		yCam -= HEIGHT(mv);
		break;
	case AG_MAP_UPPER_CENTER:
		xCam -= WIDTH(mv) >> 1;
		break;
	case AG_MAP_UPPER_LEFT:
		break;
	case AG_MAP_MIDDLE_LEFT:
		yCam -= HEIGHT(mv) >> 1;
		break;
	case AG_MAP_LOWER_LEFT:
		yCam -= HEIGHT(mv);
		break;
	case AG_MAP_UPPER_RIGHT:
		xCam -= WIDTH(mv);
		break;
	case AG_MAP_MIDDLE_RIGHT:
		xCam -= WIDTH(mv);
		yCam -= HEIGHT(mv) >> 1;
		break;
	case AG_MAP_LOWER_RIGHT:
		xCam -= WIDTH(mv);
		yCam -= HEIGHT(mv);
		break;
	}
	
	mv->mx = xCam / tileSz;
	if (mv->mx < 0) {
		mv->mx = 0;
		mv->xOffs = -xCam - tileSz;
	} else {
		mv->xOffs = -(xCam % tileSz) - tileSz;
	}
	
	mv->my = yCam / tileSz;
	if (mv->my < 0) {
		mv->my = 0;
		mv->yOffs = -yCam - tileSz;
	} else {
		mv->yOffs = -(yCam % tileSz) - tileSz;
	}

	if (mv->hbar != NULL) {
		AG_SetInt(mv->hbar, "min", 0);
		AG_SetInt(mv->hbar, "max", (map->w + mv->mw) * tileSz);
	}
	if (mv->vbar != NULL) {
		AG_SetInt(mv->vbar, "min", 0);
		AG_SetInt(mv->vbar, "max", (map->h + mv->mh) * tileSz);
	}
	
	AG_ObjectUnlock(map);
	AG_Redraw(mv);
}

void
MAP_ViewSetScale(MAP_View *mv, Uint zoom, int adj_offs)
{
	MAP *map;
	int wPx, hPx, wPxPrev, hPxPrev, tileSz;

	AG_ObjectLock(mv);
	map = mv->map;
	AG_ObjectLock(map);
	tileSz = AGMTILESZ(mv);
	wPxPrev = map->w * tileSz;
	hPxPrev = map->h * tileSz;

	if (zoom < ZOOM_MIN) { zoom = ZOOM_MIN; }
	else if (zoom > ZOOM_MAX) { zoom = ZOOM_MAX; }
	
	AGMZOOM(mv) = zoom;
	tileSz = AGMTILESZ(mv) = zoom*MAPTILESZ/100;
	AGMPIXSZ(mv) = tileSz/MAPTILESZ;
	if (tileSz > MAP_TILESZ_MAX)
		tileSz = AGMTILESZ(mv) = MAP_TILESZ_MAX;

	mv->mw = WIDTH(mv) / tileSz + 2;
	mv->mh = HEIGHT(mv) / tileSz + 2;
	mv->wVis = mv->mw * tileSz;
	mv->hVis = mv->mh * tileSz;
	wPx = map->w * tileSz;
	hPx = map->h * tileSz;

	if (adj_offs) {
		AGMCAM(mv).x = AGMCAM(mv).x * wPx / wPxPrev;
		AGMCAM(mv).y = AGMCAM(mv).y * hPx / hPxPrev;
	}
	AG_ObjectUnlock(map);
	
	MAP_ViewUpdateCamera(mv);

	AG_ObjectUnlock(mv);
}

static __inline__ int _Pure_Attribute
InsideNodeSelection(MAP_View *_Nonnull mv, int x, int y)
{
	return (!mapViewEditSelOnly || !mv->esel.set ||
	    (x >= mv->esel.x &&
	     y >= mv->esel.y &&
	     x <  mv->esel.x + mv->esel.w &&
	     y <  mv->esel.y + mv->esel.h));
}

static void
ToggleAttribute(MAP_View *_Nonnull mv)
{
	MAP *map = mv->map;
	MAP_Item *mi;
	MAP_Node *node;
	Uint nToggled = 0;

	if (mv->attr_x == mv->cx && mv->attr_y == mv->cy)
		return;

	MAP_ModBegin(map);
	MAP_ModNodeChg(map, mv->cx, mv->cy);

	node = &map->map[mv->cy][mv->cx];
	TAILQ_FOREACH(mi, &node->items, items) {
		if (mi->layer != map->layerCur) {
			continue;
		}
		if (mi->flags & mv->edit_attr) {
			mi->flags &= ~(mv->edit_attr);
			MAP_ViewStatus(mv, "[%d,%d]: ref %p flags 0x%x -> clear 0x%x",
			    mv->cx, mv->cy, mi->flags, mv->edit_attr);
		} else {
			mi->flags |= mv->edit_attr;
			MAP_ViewStatus(mv, "[%d,%d]: ref %p flags 0x%x -> set 0x%x",
			    mv->cx, mv->cy, mi->flags, mv->edit_attr);
		}
		nToggled++;
	}

	MAP_ModEnd(map);

	if (!nToggled) {
		MAP_ViewStatus(mv, "No item at [%d,%d] layer %d",
		    mv->cx, mv->cy, map->layerCur);
	}

	mv->attr_x = mv->cx;
	mv->attr_y = mv->cy;
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP *map = mv->map;
	int x = AG_INT(1);
	int y = AG_INT(2);
	const int xrel = AG_INT(3);
	const int yrel = AG_INT(4);
	const int state = AG_INT(5);
	int xmap, ymap;
	int rv;

	AG_ObjectLock(map);

	GetNodeCoords(mv, &x,&y);
	mv->cxrel = x - mv->mouse.x;
	mv->cyrel = y - mv->mouse.y;
	xmap = mv->cx * AGMTILESZ(mv) + mv->cxoffs;
	ymap = mv->cy * AGMTILESZ(mv) + mv->cyoffs;
	mv->mouse.xmap_rel += xmap - mv->mouse.xmap;
	mv->mouse.ymap_rel += ymap - mv->mouse.ymap;
	mv->mouse.xmap = xmap;
	mv->mouse.ymap = ymap;

	if (mv->flags & MAP_VIEW_EDIT) {
		if ((state & AG_MOUSE_LEFT) &&
		    mv->cx != -1 && mv->cy != -1 &&
		    (x != mv->mouse.x || y != mv->mouse.y) &&
		    (InsideNodeSelection(mv, mv->cx, mv->cy))) {
			if (mv->flags & MAP_VIEW_SET_ATTRS) {
				ToggleAttribute(mv);
				goto out;
			}
			if (mv->mode == MAP_VIEW_EDIT_ORIGIN) {
				map->xOrigin = mv->cx;
				map->yOrigin = mv->cy;
				map->layerOrigin = map->layerCur;
				goto out;
			}
			if (mv->curtool != NULL &&
			    mv->curtool->ops->effect != NULL &&
			    (rv = mv->curtool->ops->effect(mv->curtool,
			     &map->map[mv->cy][mv->cx])) != -1) {
				map->nMods += rv;
				goto out;
			}
		}
		if (mv->curtool != NULL &&
		    mv->curtool->ops->mousemotion != NULL &&
		    mv->curtool->ops->mousemotion(mv->curtool,
		      mv->mouse.xmap, mv->mouse.ymap,
		      xrel, yrel, state) == 1) {
			goto out;
		}
		if (mv->deftool != NULL &&
		    mv->deftool->ops->mousemotion != NULL &&
		    mv->deftool->ops->mousemotion(mv->deftool,
		      mv->mouse.xmap, mv->mouse.ymap,
		      xrel, yrel, state) == 1) {
			goto out;
		}
	}
	
	if (mv->mouse.scrolling) {
		AGMCAM(mv).x -= xrel;
		AGMCAM(mv).y -= yrel;
		MAP_ViewUpdateCamera(mv);
	} else if (mv->msel.set) {
		mv->msel.xOffs += mv->cxrel;
		mv->msel.yOffs += mv->cyrel;
	} else if (mv->esel.set && mv->esel.moving) {
		MAP_NodeselUpdateMove(mv, mv->cxrel, mv->cyrel);
	} else if (mv->rsel.moving) {
//		if (abs(mv->mouse.xmap_rel) > AGMPIXSZ(mv)) {
			MAP_UpdateRefSel(mv, mv->mouse.xmap_rel < 0 ? -1 : 1, 0);
			mv->mouse.xmap_rel = 0;
//		}
//		if (abs(mv->mouse.ymap_rel) > AGMPIXSZ(mv)) {
			MAP_UpdateRefSel(mv, 0, mv->mouse.ymap_rel < 0 ? -1 : 1);
			mv->mouse.ymap_rel = 0;
//		}
	}
out:
	AG_ObjectUnlock(map);

	mv->mouse.x = x;
	mv->mouse.y = y;
	AG_Redraw(mv);
}

void
MAP_ViewSetMode(MAP_View *mv, enum map_view_mode mode)
{
	mv->mode = mode;
}

#if 0
static void
ViewPopupMenu(AG_MenuItem *menu, MAP_View *mv)
{
	AG_MenuUintFlagsMp(menu, _("Enable edition"), rgIconEdit.s,
	    &mv->flags, MAP_VIEW_EDIT, 0, &OBJECT(mv)->lock);
}
#endif

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP *map = mv->map;
	MAP_Tool *tool;
	const int button = AG_INT(1);
/*	const int mx = AG_INT(2); */
/*	const int my = AG_INT(3); */
	int x,y, rv;
	
	AG_WidgetFocus(mv);
	
	AG_ObjectLock(map);
	GetNodeCoords(mv, &x,&y);
	mv->mouse.x = x;
	mv->mouse.y = y;
	mv->mouse.xmap = mv->cx*AGMTILESZ(mv) + mv->cxoffs;
	mv->mouse.ymap = mv->cy*AGMTILESZ(mv) + mv->cyoffs;
	mv->mouse.xmap_rel = 0;
	mv->mouse.ymap_rel = 0;
	
	if (mv->actor != NULL &&
	    MAP_ACTOR_OPS(mv->actor)->mousebuttondown != NULL &&
	    MAP_ACTOR_OPS(mv->actor)->mousebuttondown(mv->actor,
	      mv->mouse.xmap, mv->mouse.ymap, button) == -1)
		goto out;

	if ((mv->flags & MAP_VIEW_EDIT) &&
	    (mv->cx >= 0 && mv->cy >= 0)) {
		if (mv->mode == MAP_VIEW_EDIT_ATTRS &&
		    button == AG_MOUSE_LEFT) {
			mv->flags |= MAP_VIEW_SET_ATTRS;
			mv->attr_x = -1;
			mv->attr_y = -1;
			ToggleAttribute(mv);
			goto out;
		}
		if (mv->flags & MAP_VIEW_SHOW_ORIGIN &&
		    button == AG_MOUSE_LEFT &&
		    mv->cx == map->xOrigin &&
		    mv->cy == map->yOrigin) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDIT_ORIGIN);
			goto out;
		}
		if (mv->curtool != NULL) {
			if (mv->curtool->ops->mousebuttondown != NULL &&
			    mv->curtool->ops->mousebuttondown(mv->curtool,
			      mv->mouse.xmap, mv->mouse.ymap, button) == 1) {
				goto out;
			}
			if (button == AG_MOUSE_LEFT &&
			    mv->curtool->ops->effect != NULL &&
			    InsideNodeSelection(mv, mv->cx, mv->cy)) {
				if ((rv = mv->curtool->ops->effect(mv->curtool,
				     &map->map[mv->cy][mv->cx])) != -1) {
					mv->map->nMods = rv;
					goto out;
				}
			}
		}

		/* Mouse bindings allow inactive tools to bind mouse events. */
		TAILQ_FOREACH(tool, &mv->tools, tools) {
			MAP_ToolMouseBinding *mbinding;

			SLIST_FOREACH(mbinding, &tool->mbindings, mbindings) {
				if (mbinding->button != button) {
					continue;
				}
				if (mbinding->edit &&
				   (OBJECT(map)->flags & AG_OBJECT_READONLY)) {
					continue;
				}
				tool->mv = mv;
				if (mbinding->func(tool, button, 1,
				                   mv->mouse.xmap,
				                   mv->mouse.ymap,
				                   mbinding->arg) == 1)
					goto out;
			}
		}

		if (mv->deftool != NULL &&
		    mv->deftool->ops->mousebuttondown != NULL &&
		    mv->deftool->ops->mousebuttondown(mv->deftool,
		                                      mv->mouse.xmap,
		                                      mv->mouse.ymap,
		                                      button) == 1) {
			goto out;
		}
	}

	switch (button) {
	case AG_MOUSE_LEFT:
		if (mv->esel.set) {
			if (mv->cx >= mv->esel.x &&
			    mv->cy >= mv->esel.y &&
			    mv->cx < mv->esel.x+mv->esel.w &&
			    mv->cy < mv->esel.y+mv->esel.h) {
				MAP_NodeselBeginMove(mv);
			} else {
				mv->esel.set = 0;
			}
			goto out;
		} else {
			const AG_KeyMod mod = AG_GetModState(mv);
			MAP_Item *mi;
			int nx, ny;
			
			if (mv->curtool != NULL &&
			    mv->curtool->ops == &mapNodeselOps &&
			    (mv->flags & MAP_VIEW_NO_NODESEL) == 0) {
				MAP_NodeselBegin(mv);
				goto out;
			}
			if ((mod & AG_KEYMOD_CTRL) == 0) {
				for (ny = 0; ny < (int)map->h; ny++) {
					for (nx = 0; nx < (int)map->w; nx++) {
						MAP_Node *node = &map->map[ny][nx];
						
						TAILQ_FOREACH(mi, &node->items, items) {
							mi->flags &=
							   ~(MAP_ITEM_SELECTED);
						}
					}
				}
			}
			if (mv->curtool != NULL &&
			    mv->curtool->ops == &mapRefselOps &&
			    (mi = MAP_ItemLocate(map,
			                         mv->mouse.xmap,
			                         mv->mouse.ymap,
						 mv->cam)) != NULL) {
				if (mi->flags & MAP_ITEM_SELECTED) {
					mi->flags &= ~(MAP_ITEM_SELECTED);
				} else {
					mi->flags |= MAP_ITEM_SELECTED;
					mv->rsel.moving = 1;
				}
			}
		}
		if (mv->dblclicked) {
			AG_PostEvent(mv, "mapview-dblclick", "%i,%i%i,%i%i",
			    button, x,y, mv->cxoffs, mv->cyoffs);
			mv->dblclicked = 0;
		} else {
			mv->dblclicked++;
			AG_SchedEvent(mv, agMouseDblclickDelay,
			    "dblclick-expire", "%s", "noname");
		}
		break;
	case AG_MOUSE_MIDDLE:
	case AG_MOUSE_RIGHT:
		mv->mouse.scrolling++;
		break;
#if 0
	case AG_MOUSE_RIGHT:
		if (mv->popup != NULL) {
			AG_PopupDestroy(mv->popup);
		}
		mv->popup = AG_PopupNew(mv);
		ViewPopupMenu(mv->popup->root, mv);
		AG_PopupShowAt(mv->popup, mx,my);
		break;
#endif
	case AG_MOUSE_WHEELDOWN:
		if ((mv->flags & MAP_VIEW_NO_BMPSCALE) == 0) {
			MAP_ViewSetScale(mv, AGMZOOM(mv) - mapViewZoomInc, 1);
			MAP_ViewStatus(mv, _("%d%% zoom"), AGMZOOM(mv));
		}
		break;
	case AG_MOUSE_WHEELUP:
		if ((mv->flags & MAP_VIEW_NO_BMPSCALE) == 0) {
			MAP_ViewSetScale(mv, AGMZOOM(mv) + mapViewZoomInc, 1);
			MAP_ViewStatus(mv, _("%d%% zoom"), AGMZOOM(mv));
		}
		break;
	}
out:
	AG_Redraw(mv);
	AG_ObjectUnlock(map);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP *map = mv->map;
	MAP_Tool *tool;
	const int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	
	AG_ObjectLock(map);
	GetNodeCoords(mv, &x,&y);

	mv->flags &= ~(MAP_VIEW_SET_ATTRS);
	
	if (mv->actor != NULL &&
	    MAP_ACTOR_OPS(mv->actor)->mousebuttonup != NULL) {
		x = mv->cx*AGMTILESZ(mv) + mv->cxoffs;
		y = mv->cy*AGMTILESZ(mv) + mv->cyoffs;
		if (MAP_ACTOR_OPS(mv->actor)->mousebuttonup(mv->actor, x, y,
		    button) == -1)
			goto out;
	}

	if ((mv->flags & MAP_VIEW_EDIT) &&
	    (mv->cx >= 0 && mv->cy >= 0)) {
	    	if (mv->mode == MAP_VIEW_EDIT_ORIGIN) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDITION);
		}
		if (mv->curtool != NULL) {
			if (mv->curtool->ops->mousebuttonup != NULL &&
			    mv->curtool->ops->mousebuttonup(mv->curtool,
			      mv->mouse.xmap, mv->mouse.ymap, button) == 1) {
				goto out;
			}
		}
		
		TAILQ_FOREACH(tool, &mv->tools, tools) {
			MAP_ToolMouseBinding *mbinding;

			SLIST_FOREACH(mbinding, &tool->mbindings, mbindings) {
				if (mbinding->button != button) {
					continue;
				}
				if (mbinding->edit &&
				    (OBJECT(map)->flags & AG_OBJECT_READONLY)) {
					continue;
				}
				tool->mv = mv;
				if (mbinding->func(tool, button, 0, x, y,
				    mbinding->arg) == 1)
					goto out;
			}
		}
		
		if (mv->deftool != NULL &&
		    mv->deftool->ops->mousebuttonup != NULL &&
		    mv->deftool->ops->mousebuttonup(mv->deftool,
		      mv->mouse.xmap, mv->mouse.ymap, button) == 1) {
			goto out;
		}
	} else {
		mv->mouse.scrolling = 0;
		if (mv->esel.set && mv->esel.moving) {
			MAP_NodeselEndMove(mv);
		}
		mv->rsel.moving = 0;
		goto out;
	}

	switch (button) {
	case AG_MOUSE_LEFT:
		if (mv->msel.set &&
		   (mv->msel.xOffs == 0 || mv->msel.yOffs == 0)) {
			mv->esel.set = 0;
			mv->msel.set = 0;
		} else {
			if (mv->msel.set) {
				MAP_NodeselEnd(mv);
				mv->msel.set = 0;
			} else if (mv->esel.set && mv->esel.moving) {
				MAP_NodeselEndMove(mv);
			}
		}
		mv->rsel.moving = 0;
		break;
	case AG_MOUSE_MIDDLE:
	case AG_MOUSE_RIGHT:
		mv->mouse.scrolling = 0;
		break;
	}
out:
	AG_Redraw(mv);
	AG_ObjectUnlock(map);
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP *map = mv->map;
	MAP_Tool *tool;
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);
	
	AG_ObjectLock(map);

	if (mv->actor != NULL &&
	    MAP_ACTOR_OPS(mv->actor)->keyup != NULL) {
		if (MAP_ACTOR_OPS(mv->actor)->keyup(mv->actor, keysym, keymod)
		    == -1)
			goto out;
	}
	
	if (mv->flags & MAP_VIEW_EDIT &&
	    mv->curtool != NULL &&
	    mv->curtool->ops->keyup != NULL &&
	    mv->curtool->ops->keyup(mv->curtool, keysym, keymod) == 1)
		goto out;
	
	TAILQ_FOREACH(tool, &mv->tools, tools) {
		MAP_ToolKeyBinding *kbinding;

		SLIST_FOREACH(kbinding, &tool->kbindings, kbindings) {
			if (kbinding->key == keysym &&
			    (kbinding->mod == AG_KEYMOD_NONE ||
			     keymod & kbinding->mod)) {
				if (kbinding->edit &&
				   (((mv->flags & MAP_VIEW_EDIT) == 0) ||
				    ((OBJECT(map)->flags & AG_OBJECT_READONLY)))) {
					continue;
				}
				tool->mv = mv;
				if (kbinding->func(tool, keysym, 0,
				    kbinding->arg) == 1)
					goto out;
			}
		}
	}
out:
	AG_Redraw(mv);
	AG_ObjectUnlock(map);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	MAP_View *mv = MAP_VIEW_SELF();
	MAP *map = mv->map;
	MAP_Tool *tool;
	const int keysym = AG_INT(1);
	const int keymod = AG_INT(2);
	
	AG_ObjectLock(map);

	if (mv->actor != NULL &&
	    MAP_ACTOR_OPS(mv->actor)->keydown != NULL) {
		if (MAP_ACTOR_OPS(mv->actor)->keydown(mv->actor, keysym, keymod)
		    == -1)
			goto out;
	}
	
	if (mv->flags & MAP_VIEW_EDIT &&
	    mv->curtool != NULL &&
	    mv->curtool->ops->keydown != NULL &&
	    mv->curtool->ops->keydown(mv->curtool, keysym, keymod) == 1)
		goto out;

	TAILQ_FOREACH(tool, &mv->tools, tools) {
		MAP_ToolKeyBinding *kbinding;

		SLIST_FOREACH(kbinding, &tool->kbindings, kbindings) {
			if (kbinding->key == keysym &&
			    (kbinding->mod == AG_KEYMOD_NONE ||
			     keymod & kbinding->mod)) {
				if (kbinding->edit &&
				   (((mv->flags & MAP_VIEW_EDIT) == 0) ||
				    ((OBJECT(map)->flags &
				      AG_OBJECT_READONLY)))) {
					continue;
				}
				tool->mv = mv;
				if (kbinding->func(tool, keysym, 1,
				    kbinding->arg) == 1)
					goto out;
			}
		}
	}

	switch (keysym) {
	case AG_KEY_Z:
		if (keymod & AG_KEYMOD_CTRL) {
			MAP_Undo(map);
		}
		break;
	case AG_KEY_R:
		if (keymod & AG_KEYMOD_CTRL) {
			MAP_Redo(map);
		}
		break;
	case AG_KEY_1:
	case AG_KEY_0:
		if ((mv->flags & MAP_VIEW_NO_BMPSCALE) == 0) {
			MAP_ViewSetScale(mv, 100, 1);
			MAP_ViewStatus(mv, _("%d%% zoom"), AGMZOOM(mv));
		}
		break;
	case AG_KEY_O:
		CenterToOrigin(mv);
		break;
	case AG_KEY_G:
		if (mv->flags & MAP_VIEW_GRID) {
			mv->flags &= ~(MAP_VIEW_GRID);
		} else {
			mv->flags |= MAP_VIEW_GRID;
		}
		break;
	case AG_KEY_B:
		if (mv->flags & MAP_VIEW_NO_BG) {
			mv->flags &= ~(MAP_VIEW_NO_BG);
		} else {
			mv->flags |= MAP_VIEW_NO_BG;
		}
		break;
	case AG_KEY_W:
		if (mv->mode == MAP_VIEW_EDITION) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDIT_ATTRS);
			mv->edit_attr = MAP_ITEM_BLOCK;
		} else {
			MAP_ViewSetMode(mv, MAP_VIEW_EDITION);
		}
		break;
	case AG_KEY_C:
		if (mv->mode == MAP_VIEW_EDITION) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDIT_ATTRS);
			mv->edit_attr = MAP_ITEM_CLIMBABLE;
		} else {
			MAP_ViewSetMode(mv, MAP_VIEW_EDITION);
		}
		break;
	case AG_KEY_S:
		if (mv->mode == MAP_VIEW_EDITION) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDIT_ATTRS);
			mv->edit_attr = MAP_ITEM_SLIPPERY;
		} else {
			MAP_ViewSetMode(mv, MAP_VIEW_EDITION);
		}
		break;
	case AG_KEY_J:
		if (mv->mode == MAP_VIEW_EDITION) {
			MAP_ViewSetMode(mv, MAP_VIEW_EDIT_ATTRS);
			mv->edit_attr = MAP_ITEM_JUMPABLE;
		} else {
			MAP_ViewSetMode(mv, MAP_VIEW_EDITION);
		}
		break;
	}
out:	
	AG_Redraw(mv);
	AG_ObjectUnlock(map);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	MAP_View *mv = obj;
	const int sbLen = WFONT(mv)->lineskip;

	r->w = mv->wPre*MAPTILESZ + sbLen;
	r->h = mv->hPre*MAPTILESZ + sbLen;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	MAP_View *mv = obj;
	AG_SizeAlloc aBar;
	const int sbLen = WFONT(mv)->lineskip;

	mv->r.w = a->w;
	mv->r.h = a->h;

	if (mv->hbar) {
		aBar.x = 0;
		aBar.y = a->h - sbLen;
		aBar.w = a->w - sbLen;
		aBar.h = sbLen;
		AG_WidgetSizeAlloc(mv->hbar, &aBar);
		mv->r.h -= HEIGHT(mv->hbar);
	}
	if (mv->vbar) {
		aBar.x = a->w - sbLen;
		aBar.y = 0;
		aBar.w = sbLen;
		aBar.h = a->h;
		AG_WidgetSizeAlloc(mv->vbar, &aBar);
		mv->r.w -= WIDTH(mv->vbar);
	}
	MAP_ViewSetScale(mv, AGMZOOM(mv), 0);

	if (a->w < MAPTILESZ || a->h < MAPTILESZ) {
		return (-1);
	}
	return (0);
}

static void
LostFocus(AG_Event *_Nonnull event)
{
//	MAP_View *mv = MAP_VIEW_SELF();

//	if (mv->actor != NULL)
//		AG_ObjectCancelTimeouts(mv->actor, 0);
}

/* Set the coordinates and geometry of the selection rectangle. */
void
MAP_ViewSetSelection(MAP_View *mv, int x, int y, int w, int h)
{
	AG_ObjectLock(mv);
	mv->msel.set = 0;
	mv->esel.set = 1;
	mv->esel.x = x;
	mv->esel.y = y;
	mv->esel.w = w;
	mv->esel.h = h;
	AG_ObjectUnlock(mv);
}

/* Fetch the coordinates and geometry of the selection rectangle. */
int
MAP_ViewGetSelection(MAP_View *mv, int *x, int *y, int *w, int *h)
{
	AG_ObjectLock(mv);
	if (mv->esel.set) {
		if (x != NULL) { *x = mv->esel.x; }
		if (y != NULL) { *y = mv->esel.y; }
		if (w != NULL) { *w = mv->esel.w; }
		if (h != NULL) { *h = mv->esel.h; }
		AG_ObjectUnlock(mv);
		return (1);
	}
	AG_ObjectUnlock(mv);
	return (0);
}

void	
MAP_ViewSizeHint(MAP_View *mv, int w, int h)
{
	mv->wPre = w;
	mv->hPre = h;
}

void
MAP_ViewStatus(MAP_View *mv, const char *fmt, ...)
{
	char status[AG_LABEL_MAX];
	va_list ap;
	
	AG_ObjectLock(mv);

	if (mv->status == NULL) {
		AG_ObjectUnlock(mv);
		return;
	}

	va_start(ap, fmt);
	Vsnprintf(status, sizeof(status), fmt, ap);
	va_end(ap);

	AG_WidgetReplaceSurface(mv->status, mv->status->surface,
	    AG_TextRender(status));
	
	AG_ObjectUnlock(mv);
}

static void
Init(void *_Nonnull obj)
{
	MAP_View *mv = obj;

	WIDGET(mv)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_EXPAND |
	                     AG_WIDGET_USE_TEXT;

	mv->flags = MAP_VIEW_CENTER;
	mv->mode = MAP_VIEW_EDITION;
	mv->edit_attr = 0;
	mv->map = NULL;
	mv->actor = NULL;
	mv->cam = 0;
	mv->mw = 0;					/* Set on scale */
	mv->mh = 0;
	mv->wVis = 0;					/* Set on scale */
	mv->hVis = 0;
	mv->wPre = 8;
	mv->hPre = 8;
	mv->r.x = 0;
	mv->r.y = 0;
	mv->r.w = 0;
	mv->r.h = 0;

	mv->mouse.scrolling = 0;
	mv->mouse.x = 0;
	mv->mouse.y = 0;
	mv->mouse.xmap = 0;
	mv->mouse.ymap = 0;
	mv->mouse.xmap_rel = 0;
	mv->mouse.ymap_rel = 0;

	mv->dblclicked = 0;
	mv->hbar = NULL;
	mv->vbar = NULL;
	mv->popup = NULL;
	mv->toolbar = NULL;
	mv->statusbar = NULL;
	mv->status = NULL;
	mv->lib_tl = NULL;
	mv->objs_tl = NULL;
	mv->layers_tl = NULL;
	mv->curtool = NULL;
	mv->deftool = NULL;
	TAILQ_INIT(&mv->tools);
	SLIST_INIT(&mv->draw_cbs);
	
	mv->cx = -1;
	mv->cy = -1;
	mv->cxrel = 0;
	mv->cyrel = 0;
	mv->cxoffs = 0;
	mv->cyoffs = 0;
	mv->xOffs = 0;
	mv->yOffs = 0;
	
	mv->msel.set = 0;
	mv->msel.x = 0;
	mv->msel.y = 0;
	mv->msel.xOffs = 0;
	mv->msel.yOffs = 0;
	mv->esel.set = 0;
	mv->esel.moving = 0;
	mv->esel.x = 0;
	mv->esel.y = 0;
	mv->esel.w = 0;
	mv->esel.h = 0;
	mv->rsel.moving = 0;

	AG_ColorRGBA_8(&mv->color, 255,255,255,32);

	AG_SetEvent(mv, "widget-lostfocus",	LostFocus, NULL);
	AG_SetEvent(mv, "widget-hidden",	LostFocus, NULL);
	AG_SetEvent(mv, "key-up",		KeyUp, NULL);
	AG_SetEvent(mv, "key-down",		KeyDown, NULL);
	AG_SetEvent(mv, "mouse-motion",		MouseMotion, NULL);
	AG_SetEvent(mv, "mouse-button-down",	MouseButtonDown, NULL);
	AG_SetEvent(mv, "mouse-button-up",	MouseButtonUp, NULL);
	AG_AddEvent(mv, "detached",		OnDetach, NULL);
	AG_SetEvent(mv, "dblclick-expire",	ExpireDblClick, NULL);
	AG_SetEvent(mv, "map-resized",		UpdateCamera, "%p", mv);
}

AG_WidgetClass mapViewClass = {
	{
		"Agar(Widget):MAP(View)",
		sizeof(MAP_View),
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
