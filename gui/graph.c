/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>,
 * 2019 Charles A. Daniels, <charles@cdaniels.net>
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
#include <agar/gui/graph.h>
#include <agar/gui/primitive.h>
#include <agar/gui/menu.h>
#include <agar/gui/gui_math.h>

#include <stdarg.h>
#include <string.h>

AG_Graph *
AG_GraphNew(void *parent, Uint flags)
{
	AG_Graph *gf;

	gf = Malloc(sizeof(AG_Graph));
	AG_ObjectInit(gf, &agGraphClass);
	gf->flags |= flags;

	if (flags & AG_GRAPH_HFILL) { AG_ExpandHoriz(gf); }
	if (flags & AG_GRAPH_VFILL) { AG_ExpandVert(gf); }

	AG_ObjectAttach(parent, gf);
	return (gf);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Graph *gf = AG_SELF();
	int keysym = AG_INT(1);
	const int scrollIncr = 10;

	switch (keysym) {
	case AG_KEY_LEFT:
		gf->xOffs -= scrollIncr;
		break;
	case AG_KEY_RIGHT:
		gf->xOffs += scrollIncr;
		break;
	case AG_KEY_UP:
		gf->yOffs -= scrollIncr;
		break;
	case AG_KEY_DOWN:
		gf->yOffs += scrollIncr;
		break;
	case AG_KEY_0:
		gf->xOffs = 0;
		gf->yOffs = 0;
		break;
	}
	AG_Redraw(gf);
}

static __inline__ int
MouseOverVertex(AG_GraphVertex *_Nonnull vtx, int x, int y)
{
	return (abs(x - vtx->x + vtx->graph->xOffs) <= (vtx->w >> 1) &&
	        abs(y - vtx->y + vtx->graph->yOffs) <= (vtx->h >> 1));
}

static __inline__ void
GetEdgeLabelCoords(AG_GraphEdge *edge, int *x, int *y)
{
	*x = (edge->v1->x + edge->v2->x) >> 1;
	*y = (edge->v1->y + edge->v2->y) >> 1;
}

static __inline__ int
MouseOverEdge(AG_GraphEdge *edge, int x, int y)
{
	int lx, ly;
	AG_Surface *lbl;

	if (edge->labelSu == -1) {
		return (0);
	}
	GetEdgeLabelCoords(edge, &lx, &ly);
	lbl = WSURFACE(edge->graph,edge->labelSu);
	return (abs(x - lx + edge->graph->xOffs) <= (lbl->w >> 1) &&
	        abs(y - ly + edge->graph->yOffs) <= (lbl->h >> 1));
}

static void
MouseMotion(AG_Event *event)
{
	AG_Graph *gf = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dx = AG_INT(3);
	int dy = AG_INT(4);
	AG_GraphVertex *vtx;
	AG_GraphEdge *edge;

	if (gf->flags & AG_GRAPH_PANNING) {
		gf->xOffs -= dx;
		gf->yOffs -= dy;
		AG_Redraw(gf);
		return;
	}
	if (gf->flags & AG_GRAPH_DRAGGING) {
		TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
			if (vtx->flags & AG_GRAPH_SELECTED) {
				AG_GraphVertexPosition(vtx,
				    vtx->x + dx,
				    vtx->y + dy);
			}
		}
	} else {
		TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
			AG_SETFLAGS(vtx->flags, AG_GRAPH_MOUSEOVER,
			    MouseOverVertex(vtx,x,y));
		}
		TAILQ_FOREACH(edge, &gf->edges, edges) {
			AG_SETFLAGS(edge->flags, AG_GRAPH_MOUSEOVER,
			    MouseOverEdge(edge,x,y));
		}
		if (!TAILQ_EMPTY(&gf->vertices) ||
		    !TAILQ_EMPTY(&gf->edges))
			AG_Redraw(gf);
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Graph *gf = AG_SELF();
	int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_LEFT:
		gf->flags &= ~(AG_GRAPH_DRAGGING);
		break;
	case AG_MOUSE_MIDDLE:
		gf->flags &= ~(AG_GRAPH_PANNING);
		break;
	}
}

/* Widget must be locked */
AG_GraphEdge *
AG_GraphEdgeFind(AG_Graph *gf, void *userPtr)
{
	AG_GraphEdge *edge;

	TAILQ_FOREACH(edge, &gf->edges, edges) {
		if (edge->userPtr == userPtr)
			return (edge);
	}
	return (NULL);
}

AG_GraphEdge *
AG_GraphEdgeNew(AG_Graph *gf, AG_GraphVertex *v1, AG_GraphVertex *v2,
    void *userPtr)
{
	AG_GraphEdge *edge;

	AG_ObjectLock(gf);
	TAILQ_FOREACH(edge, &gf->edges, edges) {
		if (edge->v1 == v1 && edge->v2 == v2) {
			AG_SetError(_("Existing edge"));
			AG_ObjectUnlock(gf);
			return (NULL);
		}
	}
	edge = Malloc(sizeof(AG_GraphEdge));
	edge->labelTxt[0] = '\0';
	edge->labelSu = -1;
	AG_ColorBlack(&edge->edgeColor);
	AG_ColorBlack(&edge->labelColor);
	edge->flags = 0;
	edge->v1 = v1;
	edge->v2 = v2;
	edge->userPtr = userPtr;
	edge->graph = gf;
	edge->popupMenu = NULL;
	edge->type = AG_GRAPH_EDGE_UNDIRECTED;
	TAILQ_INSERT_TAIL(&gf->edges, edge, edges);
	gf->nedges++;

	edge->v1->edges = Realloc(edge->v1->edges,
	    (edge->v1->nedges + 1)*sizeof(AG_GraphEdge *));
	edge->v2->edges = Realloc(edge->v2->edges,
	    (edge->v2->nedges + 1)*sizeof(AG_GraphEdge *));
	edge->v1->edges[edge->v1->nedges++] = edge;
	edge->v2->edges[edge->v2->nedges++] = edge;

	AG_ObjectUnlock(gf);
	AG_Redraw(gf);
	return (edge);
}

AG_GraphEdge *
AG_DirectedGraphEdgeNew(AG_Graph *gf, AG_GraphVertex *v1, AG_GraphVertex *v2,
		void *usrPtr) {
	AG_GraphEdge *edge = AG_GraphEdgeNew(gf, v1, v2, usrPtr);
	edge->type = AG_GRAPH_EDGE_DIRECTED;
	return edge;
}

void
AG_GraphEdgeFree(AG_GraphEdge *edge)
{
	if (edge->labelSu != -1) {
		AG_WidgetUnmapSurface(edge->graph, edge->labelSu);
	}
	Free(edge);
}

void
AG_GraphEdgeLabelS(AG_GraphEdge *ge, const char *s)
{
	AG_ObjectLock(ge->graph);
	Strlcpy(ge->labelTxt, s, sizeof(ge->labelTxt));
	if (ge->labelSu >= 0) {
		AG_WidgetUnmapSurface(ge->graph, ge->labelSu);
	}
	AG_TextColor(&ge->labelColor);
	ge->labelSu = AG_WidgetMapSurface(ge->graph, AG_TextRender(ge->labelTxt));
	AG_ObjectUnlock(ge->graph);
	AG_Redraw(ge->graph);
}

void
AG_GraphEdgeLabel(AG_GraphEdge *ge, const char *fmt, ...)
{
	va_list ap;

	AG_ObjectLock(ge->graph);
	va_start(ap, fmt);
	Vsnprintf(ge->labelTxt, sizeof(ge->labelTxt), fmt, ap);
	va_end(ap);
	if (ge->labelSu >= 0) {
		AG_WidgetUnmapSurface(ge->graph, ge->labelSu);
	}
	AG_TextColor(&ge->labelColor);
	ge->labelSu = AG_WidgetMapSurface(ge->graph, AG_TextRender(ge->labelTxt));
	AG_ObjectUnlock(ge->graph);
	AG_Redraw(ge->graph);
}

void
AG_GraphEdgeColorLabel(AG_GraphEdge *edge, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ObjectLock(edge->graph);
	AG_ColorRGB_8(&edge->labelColor, r,g,b);
	AG_ObjectUnlock(edge->graph);
	AG_Redraw(edge->graph);
}

void
AG_GraphEdgeColor(AG_GraphEdge *edge, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ObjectLock(edge->graph);
	AG_ColorRGB_8(&edge->edgeColor, r,g,b);
	AG_ObjectUnlock(edge->graph);
	AG_Redraw(edge->graph);
}

void
AG_GraphEdgePopupMenu(AG_GraphEdge *edge, struct ag_popup_menu *pm)
{
	AG_ObjectLock(edge->graph);
	edge->popupMenu = pm;
	AG_ObjectUnlock(edge->graph);
	AG_Redraw(edge->graph);
}

static void
SetVertexStyle(AG_Event *event)
{
	AG_GraphVertex *vtx = AG_PTR(1);
	int style = AG_INT(2);

	AG_GraphVertexStyle(vtx, (enum ag_graph_vertex_style)style);
}

static void
UnselectEdge(AG_Graph *gf, AG_GraphEdge *edge)
{
	edge->flags &= ~(AG_GRAPH_SELECTED);
	AG_PostEvent(NULL, gf, "graph-edge-unselected", "%p", edge);
	AG_Redraw(gf);
}

static void
SelectEdge(AG_Graph *gf, AG_GraphEdge *edge)
{
	edge->flags |= AG_GRAPH_SELECTED;
	AG_PostEvent(NULL, gf, "graph-edge-selected", "%p", edge);
	AG_Redraw(gf);
}

static void
UnselectVertex(AG_Graph *gf, AG_GraphVertex *vtx)
{
	vtx->flags &= ~(AG_GRAPH_SELECTED);
	AG_PostEvent(NULL, gf, "graph-vertex-unselected", "%p", vtx);
	AG_Redraw(gf);
}

static void
SelectVertex(AG_Graph *gf, AG_GraphVertex *vtx)
{
	vtx->flags |= AG_GRAPH_SELECTED;
	AG_PostEvent(NULL, gf, "graph-vertex-selected", "%p", vtx);
	AG_Redraw(gf);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Graph *gf = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_KeyMod kmod = AG_GetModState(gf);
	AG_GraphVertex *vtx, *vtx2;
	AG_GraphEdge *edge, *edge2;
	AG_PopupMenu *pm;

	if (!AG_WidgetIsFocused(gf))
		AG_WidgetFocus(gf);

	switch (button) {
	case AG_MOUSE_MIDDLE:
		gf->flags |= AG_GRAPH_PANNING;
		break;
	case AG_MOUSE_LEFT:
		if (gf->flags & AG_GRAPH_NO_SELECT) {
			break;
		}
		if (kmod & (AG_KEYMOD_CTRL|AG_KEYMOD_SHIFT)) {
			TAILQ_FOREACH(edge, &gf->edges, edges) {
				if (!MouseOverEdge(edge, x, y)) {
					continue;
				}
				if (edge->flags & AG_GRAPH_SELECTED) {
					UnselectEdge(gf, edge);
				} else {
					SelectEdge(gf, edge);
				}
			}
			TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
				if (!MouseOverVertex(vtx, x, y)) {
					continue;
				}
				if (vtx->flags & AG_GRAPH_SELECTED) {
					UnselectVertex(gf, vtx);
				} else {
					SelectVertex(gf, vtx);
				}
			}
		} else {
			TAILQ_FOREACH(edge, &gf->edges, edges) {
				if (MouseOverEdge(edge, x, y))
					break;
			}
			if (edge != NULL) {
				TAILQ_FOREACH(edge2, &gf->edges, edges) {
					UnselectEdge(gf, edge2);
				}
				SelectEdge(gf, edge);
			}
			TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
				if (MouseOverVertex(vtx, x, y))
					break;
			}
			if (vtx != NULL) {
				TAILQ_FOREACH(vtx2, &gf->vertices, vertices) {
					UnselectVertex(gf, vtx2);
				}
				SelectVertex(gf, vtx);
			}
		}
		if (!(gf->flags & AG_GRAPH_NO_MOVE)) {
			gf->flags |= AG_GRAPH_DRAGGING;
		}
		break;
	case AG_MOUSE_RIGHT:
		if (gf->flags & AG_GRAPH_NO_MENUS) {
			break;
		}
		TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
			if (!MouseOverVertex(vtx, x, y)) {
				continue;
			}
			if (vtx->popupMenu != NULL) {
				AG_PopupShowAt(vtx->popupMenu, x,y);
			} else {
				pm = AG_PopupNew(gf);
				AG_MenuUintFlags(pm->root, _("Hide vertex"), NULL,
				    &vtx->flags, AG_GRAPH_HIDDEN, 1);
				AG_MenuSeparator(pm->root);
				AG_MenuAction(pm->root, _("Rectangular"), NULL,
				    SetVertexStyle, "%p,%i", vtx,
				    AG_GRAPH_RECTANGLE);
				AG_MenuAction(pm->root, _("Circular"), NULL,
				    SetVertexStyle, "%p,%i", vtx,
				    AG_GRAPH_CIRCLE);
				AG_PopupShowAt(pm, x,y);
			}
			break;
		}
		TAILQ_FOREACH(edge, &gf->edges, edges) {
			if (!MouseOverEdge(edge, x, y)) {
				continue;
			}
			if (edge->popupMenu != NULL) {
				AG_PopupShowAt(edge->popupMenu, x,y);
			} else {
				pm = AG_PopupNew(gf);
				AG_MenuUintFlags(pm->root, _("Hide edge"), NULL,
				    &edge->flags, AG_GRAPH_HIDDEN, 1);
				AG_PopupShowAt(pm, x,y);
			}
			break;
		}
	}
}

static void
Init(void *obj)
{
	AG_Graph *gf = obj;

	WIDGET(gf)->flags |= AG_WIDGET_FOCUSABLE;

	gf->flags = 0;
	gf->wPre = 128;
	gf->hPre = 128;
	gf->xOffs = 0;
	gf->yOffs = 0;
	gf->xMin = 0;
	gf->yMin = 0;
	gf->xMax = 0;
	gf->yMax = 0;
	TAILQ_INIT(&gf->vertices);
	TAILQ_INIT(&gf->edges);
	gf->nvertices = 0;
	gf->nedges = 0;
	gf->pxMin = 0;
	gf->pxMax = 0;
	gf->pyMin = 0;
	gf->pyMax = 0;
	gf->r.x = 0;
	gf->r.y = 0;
	gf->r.w = 0;
	gf->r.h = 0;

	AG_SetEvent(gf, "key-down", KeyDown, NULL);
	AG_SetEvent(gf, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(gf, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(gf, "mouse-motion", MouseMotion, NULL);
}

void
AG_GraphFreeVertices(AG_Graph *gf)
{
	AG_GraphVertex *vtx, *vtxNext;
	AG_GraphEdge *edge, *edgeNext;

	AG_ObjectLock(gf);

	for (vtx = TAILQ_FIRST(&gf->vertices);
	     vtx != TAILQ_END(&gf->vertices);
	     vtx = vtxNext) {
		vtxNext = TAILQ_NEXT(vtx, vertices);
		AG_GraphVertexFree(vtx);
	}
	for (edge = TAILQ_FIRST(&gf->edges);
	     edge != TAILQ_END(&gf->edges);
	     edge = edgeNext) {
		edgeNext = TAILQ_NEXT(edge, edges);
		AG_GraphEdgeFree(edge);
	}
	TAILQ_INIT(&gf->vertices);
	TAILQ_INIT(&gf->edges);
	gf->nvertices = 0;
	gf->nedges = 0;
	gf->xMin = 0;
	gf->xMax = 0;
	gf->yMin = 0;
	gf->yMax = 0;
	gf->flags &= ~(AG_GRAPH_DRAGGING);

	AG_ObjectUnlock(gf);
	AG_Redraw(gf);
}

static void
Destroy(void *p)
{
	AG_GraphFreeVertices((AG_Graph *)p);
}

void
AG_GraphSizeHint(AG_Graph *gf, Uint w, Uint h)
{
	AG_ObjectLock(gf);
	gf->wPre = w;
	gf->hPre = h;
	AG_ObjectUnlock(gf);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Graph *gf = obj;

	r->w = gf->wPre;
	r->h = gf->hPre;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Graph *gf = obj;

	if (a->w < 1 || a->h < 1)
		return (-1);

	gf->xOffs = -(a->w >> 1);
	gf->yOffs = -(a->h >> 1);

	gf->r.x = 0;
	gf->r.y = 0;
	gf->r.w = a->w;
	gf->r.h = a->h;

	return (0);
}

static void
Draw(void *obj)
{
	AG_Graph *gf = obj;
	AG_GraphVertex *vtx;
	AG_GraphEdge *edge;
	AG_Rect r;
	AG_Color c;
	int xOffs = gf->xOffs;
	int yOffs = gf->yOffs;

	AG_PushClipRect(gf, &gf->r);

	/* Draw the bounding box */
	r.x = gf->pxMin - xOffs;
	r.y = gf->pyMin - yOffs;
	r.w = gf->pxMax - gf->pxMin;
	r.h = gf->pyMax - gf->pyMin;
	AG_ColorRGB_8(&c, 128,128,128);
	AG_DrawRectOutline(gf, &r, &c);

	/* Draw the edges */
	TAILQ_FOREACH(edge, &gf->edges, edges) {
		if (edge->flags & AG_GRAPH_HIDDEN) {
			continue;
		}

#ifdef HAVE_FLOAT
		int xi1, yi1, xi2, yi2;
		int x1, y1, x2, y2;
		int h1, w1, h2, w2;

		/* cache to avoid pointer chasing */
		x1 = edge->v1->x - xOffs;
		y1 = edge->v1->y - yOffs;
		x2 = edge->v2->x - xOffs;
		y2 = edge->v2->y - yOffs;
		h1 = edge->v1->h;
		w1 = edge->v1->w;
		h2 = edge->v2->h;
		w2 = edge->v2->w;

		/* will be over-written with clipped positions */
		xi1 = x1;
		yi1 = y1;
		xi2 = x2;
		yi2 = y2;

		/* perform the correct line clipping for the given vertex
		 * style */
		if (edge->v2->style == AG_GRAPH_CIRCLE) {
			AG_ClipLineCircle(
				x2,
				y2,
				MAX(w2, h2) >> 1,
				x1,
				y1,
				x2,
				y2,
				&xi2,
				&yi2);

		} else {
			AG_ClipLine(
				x2 - (w2 >> 1),
				y2 - (h2 >> 1),
				w2,
				h2,
				x1,
				y1,
				&xi2,
				&yi2);
		}

		if (edge->v1->style == AG_GRAPH_CIRCLE) {
			AG_ClipLineCircle(
				x1,
				y1,
				MAX(w1, h1) >> 1,
				x2,
				y2,
				x1,
				y1,
				&xi1,
				&yi1);
		} else {
			AG_ClipLine(
				x1 - (w1 >> 1),
				y1 - (h1 >> 1),
				w1,
				h1,
				x2,
				y2,
				&xi1,
				&yi1);
		}

		/* Draw line appropriately depending on weather the edge type
		 * is directed or undirected. If floating point support is
		 * not available, then all edges are drawn undirected */
		if (edge->type == AG_GRAPH_EDGE_DIRECTED) {
			AG_DrawArrowLine(gf,
			    xi1,
			    yi1,
			    xi2,
			    yi2,
			    AG_ARROWLINE_FORWARD,
			    20,
			    0.5,
			    &edge->edgeColor);
		} else {
#endif /* HAVE_FLOAT */
			AG_DrawLine(gf,
#ifdef HAVE_FLOAT
			    xi1,
			    yi1,
			    xi2,
			    yi2,
#else		/* failover if we don't have float support */
			    edge->v1->x - xOffs, /* only have cache if
			    edge->v1->y - yOffs,  * HAVE_FLOAT */
			    edge->v2->x - xOffs,
			    edge->v2->y - yOffs,
#endif /* HAVE_FLOAT */
			    &edge->edgeColor);
		}


		if (edge->labelSu >= 0) {
			AG_Surface *su = WSURFACE(gf,edge->labelSu);
			int lblX, lblY;

			GetEdgeLabelCoords(edge, &lblX, &lblY);
			lblX -= xOffs + (su->w >> 1);
			lblY -= yOffs + (su->h >> 1);

			if (edge->flags & AG_GRAPH_SELECTED) {
				r.x = lblX - 1;
				r.y = lblY - 1;
				r.w = su->w + 2;
				r.h = su->h + 2;
				AG_DrawRectOutline(gf, &r, &edge->labelColor);
			}
			if (edge->flags & AG_GRAPH_MOUSEOVER) {
				r.x = lblX - 2;
				r.y = lblY - 2;
				r.w = su->w + 4;
				r.h = su->h + 4;
				AG_DrawRectOutline(gf, &r,
				    &WCOLOR_HOV(gf,LINE_COLOR));
			}
			r.x = lblX;
			r.y = lblY;
			r.w = su->w;
			r.h = su->h;
			AG_ColorRGBA_8(&c, 128,128,128, 128);
			AG_DrawRect(gf, &r, &c);
			AG_WidgetBlitSurface(gf, edge->labelSu, lblX,lblY);
		}
	}

	/* Draw the vertices. */
	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		int x,y, w,h;

		if (vtx->flags & AG_GRAPH_HIDDEN) {
			continue;
		}
		x = vtx->x;
		y = vtx->y;
		w = vtx->w;
		h = vtx->h;
		switch (vtx->style) {
		case AG_GRAPH_RECTANGLE:
			r.x = x - (w >> 1) - xOffs;
			r.y = y - (h >> 1) - yOffs;
			r.w = w;
			r.h = h;
			AG_DrawRect(gf, &r, &vtx->bgColor);

			if (vtx->flags & AG_GRAPH_SELECTED) {
				r.x--;
				r.y--;
				r.w += 2;
				r.h += 2;
				AG_ColorRGB_8(&c, 0,0,255);
				AG_DrawRectOutline(gf, &r, &c);
			}
			if (vtx->flags & AG_GRAPH_MOUSEOVER) {
				r.x--;
				r.y--;
				r.w += 2;
				r.h += 2;
				AG_ColorRGB_8(&c, 255,0,0);
				AG_DrawRectOutline(gf, &r, &c);
			}
			break;
		case AG_GRAPH_CIRCLE:
			AG_DrawCircle(gf,
			    x - xOffs,
			    y - yOffs,
			    MAX(w,h) >> 1,
			    &vtx->bgColor);
			break;
		}
		if (vtx->labelSu >= 0) {
			AG_Surface *lbl = WSURFACE(gf,vtx->labelSu);

			AG_WidgetBlitSurface(gf, vtx->labelSu,
			    x - (lbl->w >> 1) - xOffs,
			    y - (lbl->h >> 1) - yOffs);
		}
	}

	AG_PopClipRect(gf);
}

/* Graph must be locked. */
AG_GraphVertex *
AG_GraphVertexFind(AG_Graph *gf, void *userPtr)
{
	AG_GraphVertex *vtx;

	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		if (vtx->userPtr == userPtr)
			return (vtx);
	}
	return (NULL);
}

AG_GraphVertex *
AG_GraphVertexNew(AG_Graph *gf, void *userPtr)
{
	AG_GraphVertex *vtx;

	vtx = Malloc(sizeof(AG_GraphVertex));
	vtx->labelTxt[0] = '\0';
	vtx->labelSu = -1;
	AG_ColorBlack(&vtx->labelColor);
	AG_ColorRGBA_8(&vtx->bgColor, 255,255,255, 128);
	vtx->style = AG_GRAPH_RECTANGLE;
	vtx->flags = 0;
	vtx->x = 0;
	vtx->y = 0;
	vtx->w = 32;
	vtx->h = 32;
	vtx->graph = gf;
	vtx->userPtr = userPtr;
	vtx->edges = NULL;
	vtx->nedges = 0;
	vtx->popupMenu = NULL;

	AG_ObjectLock(gf);
	TAILQ_INSERT_TAIL(&gf->vertices, vtx, vertices);
	gf->nvertices++;
	AG_ObjectUnlock(gf);

	AG_Redraw(gf);
	return (vtx);
}

void
AG_GraphVertexFree(AG_GraphVertex *vtx)
{
	if (vtx->labelSu != -1) {
		AG_WidgetUnmapSurface(vtx->graph, vtx->labelSu);
	}
	Free(vtx->edges);
	Free(vtx);
}

void
AG_GraphVertexColorLabel(AG_GraphVertex *vtx, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ObjectLock(vtx->graph);
	AG_ColorRGB_8(&vtx->labelColor, r,g,b);
	AG_ObjectUnlock(vtx->graph);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexColorBG(AG_GraphVertex *vtx, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ObjectLock(vtx->graph);
	AG_ColorRGB_8(&vtx->bgColor, r,g,b);
	AG_ObjectUnlock(vtx->graph);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexLabel(AG_GraphVertex *vtx, const char *fmt, ...)
{
	char s[AG_GRAPH_LABEL_MAX];
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);

	AG_GraphVertexLabelS(vtx, s);
}

void
AG_GraphVertexLabelS(AG_GraphVertex *vtx, const char *s)
{
	AG_ObjectLock(vtx->graph);
	Strlcpy(vtx->labelTxt, s, sizeof(vtx->labelTxt));
	if (vtx->labelSu >= 0) {
		AG_WidgetUnmapSurface(vtx->graph, vtx->labelSu);
	}
	AG_TextColor(&vtx->labelColor);
	vtx->labelSu = AG_WidgetMapSurface(vtx->graph, AG_TextRender(vtx->labelTxt));
	AG_ObjectUnlock(vtx->graph);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexPosition(AG_GraphVertex *vtx, int x, int y)
{
	AG_Graph *gf = vtx->graph;

	AG_ObjectLock(gf);

	vtx->x = x;
	vtx->y = y;

	if (x < gf->xMin) { gf->xMin = x; }
	if (y < gf->yMin) { gf->yMin = y; }
	if (x > gf->xMax) { gf->xMax = x; }
	if (y > gf->yMax) { gf->yMax = y; }

	AG_ObjectUnlock(gf);
	AG_Redraw(gf);
}

void
AG_GraphVertexSize(AG_GraphVertex *vtx, Uint w, Uint h)
{
	AG_ObjectLock(vtx->graph);
	vtx->w = w;
	vtx->h = h;
	AG_ObjectUnlock(vtx->graph);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexStyle(AG_GraphVertex *vtx, enum ag_graph_vertex_style style)
{
	AG_ObjectLock(vtx->graph);
	vtx->style = style;
	AG_ObjectUnlock(vtx->graph);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexPopupMenu(AG_GraphVertex *vtx, struct ag_popup_menu *pm)
{
	AG_ObjectLock(vtx->graph);
	vtx->popupMenu = pm;
	AG_ObjectUnlock(vtx->graph);
}

static int
CompareVertices(const void *p1, const void *p2)
{
	const AG_GraphVertex *v1 = *(const void **)p1;
	const AG_GraphVertex *v2 = *(const void **)p2;

	return (v2->nedges - v1->nedges);
}

static AG_GraphVertex *
VertexAtCoords(AG_Graph *gf, int x, int y)
{
	AG_GraphVertex *vtx;

	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		if (vtx->x == x && vtx->y == y)
			return (vtx);
	}
	return (NULL);
}

#ifdef HAVE_FLOAT

/* TODO: an integer-only version of this */
static void
PlaceVertex(AG_Graph *gf, AG_GraphVertex *vtx, AG_GraphVertex **vSorted,
    int x, int y)
{
	int ox, oy;
	int i;

	vtx->x = x;
	vtx->y = y;
	vtx->flags |= AG_GRAPH_AUTOPLACED;

	if (x < gf->pxMin) { gf->pxMin = x; }
	if (x > gf->pxMax) { gf->pxMax = x; }
	if (y < gf->pyMin) { gf->pyMin = y; }
	if (y > gf->pyMax) { gf->pyMax = y; }

	for (i = 0; i < vtx->nedges; i++) {
		AG_GraphEdge *edge = vtx->edges[i];
		AG_GraphVertex *oVtx;
		float r = 128.0;
		float theta = 0.0;

		if (edge->v1 == vtx) { oVtx = edge->v2; }
		else { oVtx = edge->v1; }

		if (oVtx->flags & AG_GRAPH_AUTOPLACED) {
			continue;
		}
		for (;;) {
			ox = (int)(r*Cos(theta));
			oy = (int)(r*Sin(theta));
			if (VertexAtCoords(gf, ox, oy) == NULL) {
				PlaceVertex(gf, oVtx, vSorted, ox, oy);
				break;
			}
			theta += (float)(AG_PI*2.0)/6;
			if (theta >= (AG_PI*2.0)) {
				r += 64.0;
			}
		}
	}
}

/*
 * Try to position the vertices such that they are distributed evenly
 * throughout a given bounding box, with a minimal number of edges
 * crossing each other.
 */
void
AG_GraphAutoPlace(AG_Graph *gf, Uint w, Uint h)
{
	AG_GraphVertex **vSorted, *vtx;
	Uint i, nSorted=0;
	int tx, ty;

	AG_ObjectLock(gf);

	if (gf->nvertices == 0 || gf->nedges == 0) {
		AG_ObjectUnlock(gf);
		return;
	}

	/* Sort the vertices based on their number of connected edges. */
	vSorted = Malloc(gf->nvertices*sizeof(AG_GraphVertex *));
	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		vtx->flags &= ~(AG_GRAPH_AUTOPLACED);
		vSorted[nSorted++] = vtx;
	}
	qsort(vSorted, nSorted, sizeof(AG_GraphVertex *), CompareVertices);
	gf->pxMin = 0;
	gf->pxMax = 0;
	gf->pyMin = 0;
	gf->pyMax = 0;
	for (i = 0; i < nSorted; i++) {
		if (vSorted[i]->flags & AG_GRAPH_AUTOPLACED) {
			continue;
		}
		for (tx = gf->pxMax+128, ty = 0;
		     ty <= gf->pyMax;
		     ty += 64) {
			if (VertexAtCoords(gf, tx, ty) == NULL) {
				PlaceVertex(gf, vSorted[i], vSorted, tx, ty);
				break;
			}
		}
		if (ty <= gf->pyMax) {
			continue;
		}
		for (tx = gf->pxMin-128, ty = 0;
		     ty <= gf->pyMax;
		     ty += 64) {
			if (VertexAtCoords(gf, tx, ty) == NULL) {
				PlaceVertex(gf, vSorted[i], vSorted, tx, ty);
				break;
			}
		}
		if (ty <= gf->pyMax) {
			continue;
		}
	}
	AG_ObjectUnlock(gf);
	AG_Redraw(gf);
	Free(vSorted);
}

#endif /* HAVE_FLOAT */

AG_WidgetClass agGraphClass = {
	{
		"Agar(Widget:Graph)",
		sizeof(AG_Graph),
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
