/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
 * Copyright (c) 2019 Charles A. Daniels, <charles@cdaniels.net>
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

/*
 * Graph visualization widget. It shows vertices (nodes), and edges which
 * connect pairs of vertices.  Edges can be directed or undirected.  Edges
 * and vertices can be colored and labeled.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

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

	if (flags & AG_GRAPH_HFILL) { WIDGET(gf)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_GRAPH_VFILL) { WIDGET(gf)->flags |= AG_WIDGET_VFILL; }
	gf->flags |= flags;

	AG_ObjectAttach(parent, gf);
	return (gf);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Graph *gf = AG_GRAPH_SELF();
	const int keysym = AG_INT(1);
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
	const AG_Graph *gf = edge->graph;
	AG_Surface *lbl;
	int lx, ly;

	if (edge->labelSu == -1) {
		return (0);
	}
	GetEdgeLabelCoords(edge, &lx, &ly);
	lbl = WSURFACE(gf,edge->labelSu);
	return (abs(x - lx + gf->xOffs) <= (lbl->w >> 1) &&
	        abs(y - ly + gf->yOffs) <= (lbl->h >> 1));
}

static void
MouseMotion(AG_Event *event)
{
	AG_Graph *gf = AG_GRAPH_SELF();
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	const int dx = AG_INT(3);
	const int dy = AG_INT(4);
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
	AG_Graph *gf = AG_GRAPH_SELF();
	const int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_LEFT:
		gf->flags &= ~(AG_GRAPH_DRAGGING);
		break;
	case AG_MOUSE_MIDDLE:
		gf->flags &= ~(AG_GRAPH_PANNING);
		break;
	}
}

/* Search for an Edge by user pointer. The Graph must be locked. */
AG_GraphEdge *
AG_GraphEdgeFind(AG_Graph *gf, void *userPtr)
{
	AG_GraphEdge *edge;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");

	TAILQ_FOREACH(edge, &gf->edges, edges) {
		if (edge->userPtr == userPtr)
			return (edge);
	}
	return (NULL);
}

/* Create a new edge between two vertices. */
AG_GraphEdge *
AG_GraphEdgeNew(AG_Graph *gf, AG_GraphVertex *v1, AG_GraphVertex *v2,
    void *userPtr)
{
	AG_GraphEdge *edge, *edgeOrig;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");

	edge = Malloc(sizeof(AG_GraphEdge));
	edge->type = AG_GRAPH_EDGE_UNDIRECTED;
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

	AG_ObjectLock(gf);

	TAILQ_FOREACH(edgeOrig, &gf->edges, edges) {
		if (edgeOrig->v1 == v1 &&
		    edgeOrig->v2 == v2)
			goto edge_exists;
	}
	TAILQ_INSERT_TAIL(&gf->edges, edge, edges);
	gf->nEdges++;

	edge->v1->edges = Realloc(edge->v1->edges, (edge->v1->nEdges + 1)*sizeof(AG_GraphEdge *));
	edge->v2->edges = Realloc(edge->v2->edges, (edge->v2->nEdges + 1)*sizeof(AG_GraphEdge *));
	edge->v1->edges[edge->v1->nEdges++] = edge;
	edge->v2->edges[edge->v2->nEdges++] = edge;

	AG_ObjectUnlock(gf);
	AG_Redraw(gf);
	return (edge);
edge_exists:
	AG_SetError(_("An edge already exists between %s and %s."),
	    v1->labelTxt, v2->labelTxt);
	AG_ObjectUnlock(gf);
	free(edge);
	return (NULL);
}

AG_GraphEdge *
AG_DirectedGraphEdgeNew(AG_Graph *gf, AG_GraphVertex *v1, AG_GraphVertex *v2,
    void *usrPtr)
{
	AG_GraphEdge *edge = AG_GraphEdgeNew(gf, v1,v2, usrPtr);

	edge->type = AG_GRAPH_EDGE_DIRECTED;
	return edge;
}

void
AG_GraphEdgeFree(AG_GraphEdge *edge)
{
	if (edge->labelSu != -1) {
		AG_WidgetUnmapSurface(edge->graph, edge->labelSu);
	}
	free(edge);
}

void
AG_GraphEdgeLabelS(AG_GraphEdge *edge, const char *s)
{
	AG_Graph *gf = edge->graph;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	Strlcpy(edge->labelTxt, s, sizeof(edge->labelTxt));
	if (edge->labelSu >= 0) {
		AG_WidgetUnmapSurface(gf, edge->labelSu);
	}
	AG_TextColor(&edge->labelColor);
	edge->labelSu = AG_WidgetMapSurface(gf, AG_TextRender(edge->labelTxt));

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

void
AG_GraphEdgeLabel(AG_GraphEdge *edge, const char *fmt, ...)
{
	AG_Graph *gf = edge->graph;
	va_list ap;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	va_start(ap, fmt);
	Vsnprintf(edge->labelTxt, sizeof(edge->labelTxt), fmt, ap);
	va_end(ap);

	if (edge->labelSu >= 0) {
		AG_WidgetUnmapSurface(gf, edge->labelSu);
	}
	AG_TextColor(&edge->labelColor);
	edge->labelSu = AG_WidgetMapSurface(gf, AG_TextRender(edge->labelTxt));

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

void
AG_GraphEdgeColorLabel(AG_GraphEdge *edge, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&edge->labelColor, r,g,b);
	AG_Redraw(edge->graph);
}

void
AG_GraphEdgeColor(AG_GraphEdge *edge, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&edge->edgeColor, r,g,b);
	AG_Redraw(edge->graph);
}

void
AG_GraphEdgePopupMenu(AG_GraphEdge *edge, struct ag_popup_menu *pm)
{
	AG_Graph *gf = edge->graph;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	edge->popupMenu = pm;

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

static void
SetVertexStyle(AG_Event *event)
{
	AG_GraphVertex *vtx = AG_PTR(1);
	const int style = AG_INT(2);

	AG_GraphVertexStyle(vtx, (enum ag_graph_vertex_style)style);
}

static void
UnselectEdge(AG_Graph *gf, AG_GraphEdge *edge)
{
	edge->flags &= ~(AG_GRAPH_SELECTED);
	AG_PostEvent(gf, "graph-edge-unselected", "%p", edge);
	AG_Redraw(gf);
}

static void
SelectEdge(AG_Graph *gf, AG_GraphEdge *edge)
{
	edge->flags |= AG_GRAPH_SELECTED;
	AG_PostEvent(gf, "graph-edge-selected", "%p", edge);
	AG_Redraw(gf);
}

static void
UnselectVertex(AG_Graph *gf, AG_GraphVertex *vtx)
{
	vtx->flags &= ~(AG_GRAPH_SELECTED);
	AG_PostEvent(gf, "graph-vertex-unselected", "%p", vtx);
	AG_Redraw(gf);
}

static void
SelectVertex(AG_Graph *gf, AG_GraphVertex *vtx)
{
	vtx->flags |= AG_GRAPH_SELECTED;
	AG_PostEvent(gf, "graph-vertex-selected", "%p", vtx);
	AG_Redraw(gf);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Graph *gf = AG_GRAPH_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);
	const AG_KeyMod kmod = AG_GetModState(gf);
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
		if (kmod & (AG_KEYMOD_CTRL | AG_KEYMOD_SHIFT)) {
			TAILQ_FOREACH(edge, &gf->edges, edges) {
				if (!MouseOverEdge(edge, x,y)) {
					continue;
				}
				if (edge->flags & AG_GRAPH_SELECTED) {
					UnselectEdge(gf, edge);
				} else {
					SelectEdge(gf, edge);
				}
			}
			TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
				if (!MouseOverVertex(vtx, x,y)) {
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
				if (MouseOverEdge(edge, x,y))
					break;
			}
			if (edge != NULL) {
				TAILQ_FOREACH(edge2, &gf->edges, edges) {
					UnselectEdge(gf, edge2);
				}
				SelectEdge(gf, edge);
			}
			TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
				if (MouseOverVertex(vtx, x,y))
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
			if (!MouseOverVertex(vtx, x,y)) {
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
			if (!MouseOverEdge(edge, x,y)) {
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
	gf->nVertices = 0;
	gf->nEdges = 0;
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

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
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
	gf->nVertices = 0;
	gf->nEdges = 0;
	gf->xMin = 0;
	gf->xMax = 0;
	gf->yMin = 0;
	gf->yMax = 0;
	gf->flags &= ~(AG_GRAPH_DRAGGING);

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

static void
Destroy(void *p)
{
	AG_GraphFreeVertices((AG_Graph *)p);
}

/* Set an initial size requisition in pixels. */
void
AG_GraphSizeHint(AG_Graph *gf, Uint w, Uint h)
{
	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	gf->wPre = w;
	gf->hPre = h;
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
	AG_GraphVertex *vtx;

	if (a->w < 1 || a->h < 1)
		return (-1);

	gf->r.x = 0;
	gf->r.y = 0;
	gf->r.w = a->w;
	gf->r.h = a->h;

	/* if at least one vertex is in view, we don't need to reset *Offs. */
	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		if (AG_RectInside(&gf->r, vtx->x - gf->xOffs,
		                          vtx->y - gf->yOffs))
			break;
	}
	if (vtx == NULL) {
		/* reset view position */
		gf->xOffs = -(a->w >> 1);
		gf->yOffs = -(a->h >> 1);
	}
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

	AG_PushBlendingMode(gf, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	/* Draw the edges */
	TAILQ_FOREACH(edge, &gf->edges, edges) {
		if (edge->flags & AG_GRAPH_HIDDEN)
			continue;
		{
			int xi1, yi1, xi2, yi2;
			int x1, y1, x2, y2;
			int h1, w1, h2, w2;

			/* Cache to avoid pointer chasing. */
			x1 = edge->v1->x - xOffs;
			y1 = edge->v1->y - yOffs;
			x2 = edge->v2->x - xOffs;
			y2 = edge->v2->y - yOffs;
			h1 = edge->v1->h;
			w1 = edge->v1->w;
			h2 = edge->v2->h;
			w2 = edge->v2->w;

			/* Will be over-written with clipped positions. */
			xi1 = x1;
			yi1 = y1;
			xi2 = x2;
			yi2 = y2;

			/*
			 * Perform the correct line clipping for the
			 * given vertex style.
			 */
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

			/*
			 * Draw line appropriately depending on weather the
			 * edge type is directed or undirected. If floating
			 * point support is not available, then all edges
			 * are drawn undirected.
			 */
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
			}
			AG_DrawLine(gf,
			    xi1,
			    yi1,
			    xi2,
			    yi2,
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
				    &WCOLOR_HOVER(gf, LINE_COLOR));
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

	AG_PopBlendingMode(gf);
	AG_PopClipRect(gf);
}

/* The Graph must be locked. */
AG_GraphVertex *
AG_GraphVertexFind(AG_Graph *gf, void *userPtr)
{
	AG_GraphVertex *vtx;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");

	TAILQ_FOREACH(vtx, &gf->vertices, vertices) {
		if (vtx->userPtr == userPtr)
			return (vtx);
	}
	return (NULL);
}

/* Create a new vertex. */
AG_GraphVertex *
AG_GraphVertexNew(AG_Graph *gf, void *userPtr)
{
	AG_GraphVertex *vtx;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");

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
	vtx->nEdges = 0;
	vtx->popupMenu = NULL;

	AG_ObjectLock(gf);
	TAILQ_INSERT_TAIL(&gf->vertices, vtx, vertices);
	gf->nVertices++;
	AG_Redraw(gf);
	AG_ObjectUnlock(gf);

	return (vtx);
}

/* The Graph must be locked. */
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
	AG_ColorRGB_8(&vtx->labelColor, r,g,b);
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexColorBG(AG_GraphVertex *vtx, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&vtx->bgColor, r,g,b);
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
	AG_Graph *gf = vtx->graph;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	Strlcpy(vtx->labelTxt, s, sizeof(vtx->labelTxt));
	if (vtx->labelSu >= 0) {
		AG_WidgetUnmapSurface(gf, vtx->labelSu);
	}
	AG_TextColor(&vtx->labelColor);
	vtx->labelSu = AG_WidgetMapSurface(gf, AG_TextRender(vtx->labelTxt));

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

void
AG_GraphVertexPosition(AG_GraphVertex *vtx, int x, int y)
{
	AG_Graph *gf = vtx->graph;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	vtx->x = x;
	vtx->y = y;

	if (x < gf->xMin) { gf->xMin = x; }
	if (y < gf->yMin) { gf->yMin = y; }
	if (x > gf->xMax) { gf->xMax = x; }
	if (y > gf->yMax) { gf->yMax = y; }

	AG_Redraw(gf);
	AG_ObjectUnlock(gf);
}

void
AG_GraphVertexSize(AG_GraphVertex *vtx, Uint w, Uint h)
{
	vtx->w = w;
	vtx->h = h;
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexStyle(AG_GraphVertex *vtx, enum ag_graph_vertex_style style)
{
	vtx->style = style;
	AG_Redraw(vtx->graph);
}

void
AG_GraphVertexPopupMenu(AG_GraphVertex *vtx, struct ag_popup_menu *pm)
{
	AG_Graph *gf = vtx->graph;

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	vtx->popupMenu = pm;

	AG_ObjectUnlock(gf);
}

static int
CompareVertices(const void *p1, const void *p2)
{
	const AG_GraphVertex *v1 = *(const void **)p1;
	const AG_GraphVertex *v2 = *(const void **)p2;

	return (v2->nEdges - v1->nEdges);
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

	for (i = 0; i < vtx->nEdges; i++) {
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

	AG_OBJECT_ISA(gf, "AG_Widget:AG_Graph:*");
	AG_ObjectLock(gf);

	if (gf->nVertices == 0 || gf->nEdges == 0) {
		AG_ObjectUnlock(gf);
		return;
	}

	/* Sort the vertices based on their number of connected edges. */
	vSorted = Malloc(gf->nVertices*sizeof(AG_GraphVertex *));
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

#endif /* AG_WIDGETS */
