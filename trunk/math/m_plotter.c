/*
 * Copyright (c) 2005-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/enable_gui.h>
#ifdef ENABLE_GUI

/*
 * Graphical plotter widget.
 */

#include <core/core.h>
#include <gui/widget.h>
#include <gui/hsvpal.h>
#include <gui/table.h>
#include <gui/notebook.h>
#include <gui/separator.h>
#include <gui/radio.h>
#include <gui/numerical.h>
#include <gui/primitive.h>

#include "m.h"
#include "m_plotter.h"
#include "m_gui.h"

#include <stdarg.h>
#include <string.h>

M_Plotter *
M_PlotterNew(void *parent, Uint flags)
{
	M_Plotter *pl;

	pl = Malloc(sizeof(M_Plotter));
	AG_ObjectInit(pl, &mPlotterClass);
	pl->flags |= flags;
	
	if (flags & M_PLOTTER_HFILL) WIDGET(pl)->flags |= AG_WIDGET_HFILL;
	if (flags & M_PLOTTER_VFILL) WIDGET(pl)->flags |= AG_WIDGET_VFILL;

	AG_ObjectAttach(parent, pl);
	return (pl);
}

static void
keydown(AG_Event *event)
{
	M_Plotter *ptr = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_0:
	case SDLK_1:
		ptr->yScale = 1.0;
		break;
	case SDLK_EQUALS:
		ptr->yScale += 0.125;
		break;
	case SDLK_MINUS:
		ptr->yScale -= 0.125;
		break;
	}
	if (ptr->yScale <= 0.125) { ptr->yScale = 0.125; }
}

static __inline__ int
MouseOverPlotItem(M_Plotter *ptr, M_Plot *pl, int x, int y)
{
	AG_Surface *lbl;
	
	if (pl->label < 0) { return (0); }
	lbl = WSURFACE(ptr,pl->label);
	return (x >= pl->xLabel && x <= (pl->xLabel + lbl->w) &&
	        y >= pl->yLabel && y <= (pl->yLabel + lbl->h));
}

static void
mousemotion(AG_Event *event)
{
	M_Plotter *ptr = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dy = AG_INT(4);
	int state = AG_INT(5);
	M_Plot *pl;

	if (!AG_WidgetRelativeArea(ptr, x, y))
		return;

	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		if (pl->flags & M_PLOT_SELECTED &&
		    state & SDL_BUTTON_LEFT) {
			pl->yOffs += dy;
		}
		if (MouseOverPlotItem(ptr, pl, x, y)) {
			pl->flags |= M_PLOT_MOUSEOVER;
		} else {
			pl->flags &= ~M_PLOT_MOUSEOVER;
		}
	}
}

#if 0
static void
mousebuttonup(AG_Event *event)
{
	M_Plotter *ptr = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	M_Plot *pl;

	switch (button) {
	case SDL_BUTTON_LEFT:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			pl->flags &= ~M_PLOT_DRAGGING;
		}
		break;
	}
}
#endif

void
M_PlotUpdateLabel(M_Plot *pl)
{
	M_Plotter *ptr = pl->plotter;

	if (pl->label >= 0) {
		AG_WidgetUnmapSurface(ptr, pl->label);
	}
	AG_TextFont(ptr->font);
	AG_TextColor32(pl->color);
	pl->label = (pl->label_txt == NULL) ? -1 :
	    AG_WidgetMapSurface(ptr, AG_TextRender(pl->label_txt));
}

static void
UpdateLabel(AG_Event *event)
{
	M_PlotUpdateLabel(AG_PTR(1));
}

static void
UpdatePlotTbl(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();
	M_Plot *pl = AG_PTR(1);
	Uint i, j;

	AG_TableBegin(tbl);
	for (i = 0; i < pl->n; i++) {
		if (pl->type == M_PLOT_VECTORS) {
			M_Vector *v = pl->data.v[i];
			for (j = 0; j < v->m; j++) {
				AG_TableAddRow(tbl, "%u[%u]:%f", i, j,
				    M_VecGetElement(v,j));
			}
		} else {
			AG_TableAddRow(tbl, "%u:%f", pl->data.r[i]);
		}
	}
	AG_TableEnd(tbl);
}

AG_Window *
M_PlotSettings(M_Plot *pl)
{
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	const char *type_names[] = {
		N_("Points"),
		N_("Linear interpolation"),
		N_("Cubic spline interpolation"),
		NULL
	};

	if ((win = AG_WindowNewNamed(0, "plotter%p", pl)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Plot: <%s>"), pl->label_txt);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAddTab(nb, _("Trace"), AG_BOX_VERT);
	{
		AG_RadioNewUint(ntab, 0, type_names, (void *)&pl->type);
		M_NumericalNewReal(ntab, 0, NULL, _("X-scale: "), &pl->xScale);
		M_NumericalNewReal(ntab, 0, NULL, _("Y-scale: "), &pl->yScale);
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		AG_NumericalNewInt(ntab, 0, "px", _("X-offset: "), &pl->xOffs);
		AG_NumericalNewInt(ntab, 0, "px", _("Y-offset: "), &pl->yOffs);
	}
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_BindPointer(pal, "pixel-format", (void *)&agSurfaceFmt);
		AG_BindUint32(pal, "pixel", &pl->color);
		AG_SetEvent(pal, "h-changed", UpdateLabel, "%p", pl);
		AG_SetEvent(pal, "sv-changed", UpdateLabel, "%p", pl);
	}
	ntab = AG_NotebookAddTab(nb, _("Table"), AG_BOX_VERT);
	{
		AG_Table *tbl;

		tbl = AG_TableNewPolled(ntab, AG_TABLE_MULTI|AG_TABLE_EXPAND,
		    UpdatePlotTbl, "%p", pl);
		AG_TableAddCol(tbl, _("#"), "<88888>", NULL);
		AG_TableAddCol(tbl, _("Value"), NULL, NULL);
	}
	AG_WindowShow(win);
	return (win);
}

static void
ShowPlotSettings(AG_Event *event)
{
	M_Plot *pl = AG_PTR(1);

	M_PlotSettings(pl);
}

static void
mousebuttondown(AG_Event *event)
{
	M_Plotter *ptr = AG_SELF();
	M_Plot *pl, *opl;
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	SDLMod mod = SDL_GetModState();

	switch (button) {
	case SDL_BUTTON_LEFT:
		AG_WidgetFocus(ptr);
		if (mod & (KMOD_CTRL|KMOD_SHIFT)) {
			TAILQ_FOREACH(pl, &ptr->plots, plots) {
				if (!MouseOverPlotItem(ptr, pl, x, y)) {
					continue;
				}
				AG_INVFLAGS(pl->flags, M_PLOT_SELECTED);
			}
		} else {
			TAILQ_FOREACH(pl, &ptr->plots, plots) {
				if (MouseOverPlotItem(ptr, pl, x, y))
					break;
			}
			if (pl != NULL) {
				TAILQ_FOREACH(opl, &ptr->plots, plots) {
					opl->flags &= ~M_PLOT_SELECTED;
				}
				pl->flags |= M_PLOT_SELECTED;
			}
		}
		break;
	case SDL_BUTTON_RIGHT:
		AG_WidgetFocus(ptr);
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (MouseOverPlotItem(ptr, pl, x, y)) {
				AG_PopupMenu *pm;

				pm = AG_PopupNew(ptr);
				AG_MenuUintFlags(pm->item, _("Display plot"),
				    NULL,
				    &pl->flags, M_PLOT_HIDDEN, 1);
				AG_MenuAction(pm->item, _("Plot settings"),
				    NULL,
				    ShowPlotSettings, "%p", pl);
				AG_PopupShow(pm);
				break;
			}
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (! (pl->flags & M_PLOT_SELECTED)) { continue; }
			pl->yScale -= 0.250;
		}
		break;
	case SDL_BUTTON_WHEELUP:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (! (pl->flags & M_PLOT_SELECTED)) { continue; }
			pl->yScale += 0.250;
		}
		break;
	default:
		break;
	}
	if (ptr->xScale <= 0.0625) { ptr->xScale = 0.0625; }
	if (ptr->yScale <= 0.0625) { ptr->yScale = 0.0625; }
}

static void
UpdateXBar(AG_Event *event)
{
	M_Plotter *ptr = AG_PTR(1);
	int value = AG_GetInt(ptr->hbar, "value");

	if (value >= ptr->xMax - WIDTH(ptr)) {
		ptr->flags |= M_PLOTTER_SCROLL;
	} else {
		ptr->flags &= ~M_PLOTTER_SCROLL;
	}
}

static void
Init(void *obj)
{
	M_Plotter *ptr = obj;

	WIDGET(ptr)->flags |= AG_WIDGET_FOCUSABLE;

	ptr->type = M_PLOT_2D;
	ptr->flags = 0;
	ptr->xMax = 0;
	ptr->yMin = 0.0;
	ptr->yMax = 0.0;
	ptr->xOffs = 0;
	ptr->yOffs = 0;
	ptr->wPre = 128;
	ptr->hPre = 64;
	ptr->xScale = 1.0;
	ptr->yScale = 1.0;
	ptr->font = AG_FetchFont(NULL, -1, -1);
	ptr->r = AG_RECT(0,0,0,0);
	TAILQ_INIT(&ptr->plots);
	
	ptr->vMin = M_New(3,1);
	ptr->vMax = M_New(3,1);
	M_SetZero(ptr->vMin);
	M_SetZero(ptr->vMax);

	ptr->curColor = 0;
	ptr->colors[0] = AG_MapRGB(agSurfaceFmt, 255, 255, 255);
	ptr->colors[1] = AG_MapRGB(agSurfaceFmt, 0, 250, 0); 
	ptr->colors[2] = AG_MapRGB(agSurfaceFmt, 250, 250, 0);
	ptr->colors[3] = AG_MapRGB(agSurfaceFmt, 0, 118, 163);
	ptr->colors[4] = AG_MapRGB(agSurfaceFmt, 175, 143, 44);
	ptr->colors[5] = AG_MapRGB(agSurfaceFmt, 169, 172, 182);
	ptr->colors[6] = AG_MapRGB(agSurfaceFmt, 255, 255, 255);
	ptr->colors[7] = AG_MapRGB(agSurfaceFmt, 59, 122, 87);
	ptr->colors[8] = AG_MapRGB(agSurfaceFmt, 163, 151, 180);
	ptr->colors[9] = AG_MapRGB(agSurfaceFmt, 249, 234, 243);
	ptr->colors[10] = AG_MapRGB(agSurfaceFmt, 157, 229, 255);
	ptr->colors[11] = AG_MapRGB(agSurfaceFmt, 223, 190, 111);
	ptr->colors[12] = AG_MapRGB(agSurfaceFmt, 79, 168, 61);
	ptr->colors[13] = AG_MapRGB(agSurfaceFmt, 234, 147, 115);
	ptr->colors[14] = AG_MapRGB(agSurfaceFmt, 127, 255, 212);
	ptr->colors[15] = AG_MapRGB(agSurfaceFmt, 218, 99, 4);
	
	ptr->hbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_HORIZ, 0);
	ptr->vbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_VERT, 0);
	AG_BindInt(ptr->hbar, "value", &ptr->xOffs);
	AG_BindInt(ptr->hbar, "visible", &ptr->r.w);
	AG_BindInt(ptr->hbar, "max", &ptr->xMax);
	AG_SetEvent(ptr->hbar, "scrollbar-changed", UpdateXBar, "%p", ptr);

	AG_BindInt(ptr->vbar, "value", &ptr->yOffs);
/*	AG_BindInt(ptr->vbar, "max", &ptr->yMax); */
	AG_SetInt(ptr->hbar, "min", 0);
	AG_SetInt(ptr->vbar, "min", 0);

	AG_SetEvent(ptr, "window-keydown", keydown, NULL);
	AG_SetEvent(ptr, "window-mousebuttondown", mousebuttondown, NULL);
/*	AG_SetEvent(ptr, "window-mousebuttonup", mousebuttonup, NULL); */
	AG_SetEvent(ptr, "window-mousemotion", mousemotion, NULL);
}

static void
Destroy(void *obj)
{
	M_Plotter *ptr = obj;
	M_Plot *plot;
	M_PlotLabel *plbl;
	Uint i;

	while ((plot = TAILQ_FIRST(&ptr->plots)) != NULL) {
		while ((plbl = TAILQ_FIRST(&plot->labels)) != NULL) {
			Free(plbl);
		}
		if (plot->type == M_PLOT_VECTORS) {
			for (i = 0; i < plot->n; i++) {
				M_VecFree(plot->data.v[i]);
			}
			free(plot->data.v);
		} else {
			free(plot->data.r);
		}
	}
	
	M_Free(ptr->vMin);
	M_Free(ptr->vMax);
}

void
M_PlotterSizeHint(M_Plotter *ptr, Uint w, Uint h)
{
	ptr->wPre = w;
	ptr->hPre = h;
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	M_Plotter *ptr = obj;

	r->w = ptr->wPre;
	r->h = ptr->hPre;
	if (ptr->flags & M_PLOTTER_SCROLL)
		ptr->xOffs = 0;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	M_Plotter *ptr = obj;
	AG_SizeAlloc aBar;

	if (a->w < 2 || a->h < 2)
		return (-1);

	ptr->r.w = a->w;
	ptr->r.h = a->h;

	aBar.x = 0;
	aBar.y = a->h - ptr->hbar->wButton;
	aBar.w = a->w;
	aBar.h = ptr->hbar->wButton;
	AG_WidgetSizeAlloc(ptr->hbar, &aBar);
	ptr->r.h -= HEIGHT(ptr->hbar);

	aBar.x = a->w - ptr->vbar->wButton;
	aBar.y = ptr->vbar->wButton;
	aBar.w = ptr->vbar->wButton;
	aBar.h = a->h - ptr->hbar->wButton;
	AG_WidgetSizeAlloc(ptr->vbar, &aBar);
	ptr->r.w -= WIDTH(ptr->vbar);
	
	return (0);
}

static __inline__ M_Real
ScaleReal(M_Plotter *ptr, M_Plot *pl, M_Real r)
{
	return (r*(ptr->yScale*pl->yScale));
}

static void
Draw(void *obj)
{
	M_Plotter *ptr = obj;
	M_Plot *pl;
	M_PlotLabel *plbl;
	Uint i;
	int y0 = ptr->r.h/2;

	AG_DrawBox(ptr, ptr->r, -1, AG_COLOR(GRAPH_BG_COLOR));
	
	AG_WidgetDraw(ptr->hbar);
	AG_WidgetDraw(ptr->vbar);

	AG_PushClipRect(ptr, ptr->r);
	
	AG_DrawLineH(ptr, 1, ptr->r.w-2, y0, ptr->colors[0]);
	AG_DrawLineV(ptr, ptr->xMax-1, 30, ptr->r.h-30, ptr->colors[0]);

	/* First pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		int x = pl->xOffs - ptr->xOffs;
		int y, py = y0+pl->yOffs+ptr->yOffs;

		if (pl->label >= 0) {
			AG_Surface *su = WSURFACE(ptr,pl->label);

			if (pl->flags & M_PLOT_SELECTED) {
				AG_DrawRectOutline(ptr,
				    AG_RECT(pl->xLabel-2, pl->yLabel-2,
				            su->w+4, su->h+4),
				    AG_VideoPixel(pl->color));
			} else if (pl->flags & M_PLOT_MOUSEOVER) {
				AG_DrawRectOutline(ptr,
				    AG_RECT(pl->xLabel-2, pl->yLabel-2,
				            su->w+4, su->h+4),
				    AG_COLOR(TEXT_COLOR));
			}
			AG_WidgetBlitSurface(ptr, pl->label, pl->xLabel,
			    pl->yLabel);
		}
		if (pl->flags & M_PLOT_HIDDEN) {
			continue;
		}
		switch (pl->type) {
		case M_PLOT_POINTS:
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				if (agView->opengl) {
					/* TODO */
				} else {
					AG_LockView();
					AG_WidgetPutPixel(ptr, x,
					    y0 - y + pl->yOffs + ptr->yOffs,
					    AG_VideoPixel(pl->color));
					AG_UnlockView();
				}
				if (x > ptr->r.w) { break; }
			}
			break;
		case M_PLOT_LINEAR:
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				AG_DrawLine(ptr, x-1, py, x,
				    y0 - y + pl->yOffs + ptr->yOffs,
				    AG_VideoPixel(pl->color));
				py = y0 - y + pl->yOffs + ptr->yOffs;
				if (x > ptr->r.w) { break; }
			}
			break;
		default:
			break;
		}
	}
	/* Second pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		if (pl->flags & M_PLOT_HIDDEN) {
			continue;
		}
		TAILQ_FOREACH(plbl, &pl->labels, labels) {
			AG_Surface *su = WSURFACE(ptr,plbl->text_surface);
			int xLbl, yLbl;
			Uint8 colLine[4];
			Uint8 colBG[4];

			AG_GetRGB(AG_COLOR(GRAPH_BG_COLOR), agVideoFmt,
			    &colBG[0], &colBG[1], &colBG[2]);
			AG_GetRGBA(pl->color, agSurfaceFmt,
			    &colLine[0], &colLine[1], &colLine[2], &colLine[3]);
			colLine[3] /= 2;
			colBG[3] = 200;

			switch (plbl->type) {
			case M_LABEL_X:
				xLbl = plbl->x - ptr->xOffs - pl->xOffs;
				yLbl = ptr->r.h - su->h - 4 - plbl->y;
				AG_DrawLineBlended(ptr,
				    xLbl, 1,
				    xLbl, ptr->r.h-2,
				    colLine, AG_ALPHA_SRC);
				break;
			case M_LABEL_Y:
				xLbl = plbl->x - ptr->xOffs - pl->xOffs;
				yLbl = ptr->r.h - su->h - 4 - plbl->y;
				break;
			case M_LABEL_FREE:
				xLbl = 4 + plbl->x - ptr->xOffs - pl->xOffs;
				yLbl = 4 + plbl->y;
				break;
			default:
				xLbl = 4 + plbl->x;
				yLbl = 4 + plbl->y;
				break;
			}
			AG_DrawRectBlended(ptr,
			    AG_RECT(xLbl+2, yLbl, su->w, su->h),
			    colBG, AG_ALPHA_SRC);
			AG_WidgetBlitSurface(ptr, plbl->text_surface,
			    xLbl+2, yLbl);
		}
	}

	AG_PopClipRect();
}

void
M_PlotClear(M_Plot *pl)
{
	pl->data.r = Realloc(pl->data.r, sizeof(M_Real));
	pl->n = 0;
}

void
M_PlotReal(M_Plot *pl, M_Real v)
{
	pl->data.r = Realloc(pl->data.r, (pl->n+1)*sizeof(M_Real));
	pl->data.r[pl->n] = v;
	if (++pl->n > pl->plotter->xMax) { pl->plotter->xMax = pl->n; }
	if (v > pl->plotter->yMax) { pl->plotter->yMax = v; }
	if (v < pl->plotter->yMin) { pl->plotter->yMin = v; }
}

void
M_PlotRealv(M_Plot *pl, Uint n, const M_Real *vp)
{
	Uint i;

	pl->data.r = Realloc(pl->data.r, (pl->n+n)*sizeof(M_Real));
	memcpy(&pl->data.r[pl->n], vp, n*sizeof(M_Real));
	if ((pl->n += n) > pl->plotter->xMax) { pl->plotter->xMax = pl->n; }
	for (i = 0; i < n; i++) {
		if (vp[i] > pl->plotter->yMax) { pl->plotter->yMax = vp[i]; }
		if (vp[i] < pl->plotter->yMin) { pl->plotter->yMin = vp[i]; }
	}
}

static void
VectorMinimum(M_Vector *c, const M_Vector *a, const M_Vector *b)
{
	Uint i;
	
	for (i = 0; i < c->m; i++) {
		if (M_VecGet(a, i) < M_VecGet(b,i)) {
			M_VecSet(c, i, M_VecGet(a, i));
		} else {
			M_VecSet(c, i, M_VecGet(b, i));
		}
	}
}

static void
VectorMaximum(M_Vector *c, const M_Vector * a, const M_Vector *b)
{
	Uint i;

	for (i = 0; i < c->m; i++) {
		if (M_VecGet(a, i) > M_VecGet(b,i)) {
			M_VecSet(c, i, M_VecGet(a, i));
		} else {
			M_VecSet(c, i, M_VecGet(b, i));
		}
	}
}

void
M_PlotVector(M_Plot *pl, const M_Vector *v)
{
	int i;
	pl->data.v = Realloc(pl->data.v, (pl->n)*sizeof(M_Vector *));
	pl->data.v[pl->n] = M_VecNew(v->m);
	VectorMinimum(pl->plotter->vMin, pl->plotter->vMin, v);
	VectorMaximum(pl->plotter->vMax, pl->plotter->vMax, v);
	for (i = 0; i < v->m; i++) {
		M_Real *e = M_VecGetElement(pl->data.v[pl->n], i);
		*e = M_VecGet(v, i);
	}
	pl->n++;
}

void
M_PlotVectorv(M_Plot *pl, Uint n, const M_Vector **vp)
{
	Uint i;

	pl->data.v = Realloc(pl->data.v, (pl->n+n)*sizeof(M_Vector));
	for (i = 0; i < n; i++) {
		pl->data.v[pl->n+i] = M_VecNew(vp[i]->m);
		VectorMinimum(pl->plotter->vMin, pl->plotter->vMin, vp[i]);
		VectorMaximum(pl->plotter->vMax, pl->plotter->vMax, vp[i]);
		M_Copy(pl->data.v[pl->n+i], vp[i]);
	}
	pl->n += n;
}

static __inline__ void
M_PlotDerivative(M_Plotter *ptr, M_Plot *dp)
{
	M_Plot *p = dp->src.plot;

	if (p->n >= 2) {
		M_PlotReal(dp, p->data.r[p->n-1] - p->data.r[p->n-2]);
	} else {
		M_PlotReal(dp, p->data.r[p->n-1]);
	}
}

static __inline__ M_Real
PlotVariableVFS(M_Plotter *ptr, M_Plot *pl)
{
	AG_Variable *V;
	M_Real rv;

	if ((V = AG_GetVariableVFS(pl->src.varVFS.vfs, pl->src.varVFS.key))
	    == NULL) {
		AG_Verbose("Plot \"%s\": %s\n", pl->src.varVFS.key,
		    AG_GetError());
		return (0.0);
	}
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_FLOAT:		rv = (M_Real)V->data.flt;	break;
	case AG_VARIABLE_DOUBLE:	rv = (M_Real)V->data.dbl;	break;
	case AG_VARIABLE_UINT:		rv = (M_Real)V->data.u;		break;
	case AG_VARIABLE_INT:		rv = (M_Real)V->data.i;		break;
	case AG_VARIABLE_UINT8:		rv = (M_Real)V->data.u8;	break;
	case AG_VARIABLE_SINT8:		rv = (M_Real)V->data.s8;	break;
	case AG_VARIABLE_UINT16:	rv = (M_Real)V->data.u16;	break;
	case AG_VARIABLE_SINT16:	rv = (M_Real)V->data.s16;	break;
	case AG_VARIABLE_UINT32:	rv = (M_Real)V->data.u32;	break;
	case AG_VARIABLE_SINT32:	rv = (M_Real)V->data.s32;	break;
	default:
		AG_Verbose("Plot \"%s\": Invalid type\n", pl->src.varVFS.key);
		rv = 0.0;
		break;
	}
	AG_UnlockVariable(V);
	return (rv);
}

void
M_PlotterUpdate(M_Plotter *ptr)
{
	M_Plot *pl;
	M_Real *v;

	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		switch (pl->src_type) {
		case M_PLOT_MANUALLY:
			break;
		case M_PLOT_FROM_VARIABLE_VFS:
			M_PlotReal(pl, PlotVariableVFS(ptr, pl));
			break;
		case M_PLOT_FROM_REAL:
			M_PlotReal(pl, *pl->src.real);
			break;
		case M_PLOT_FROM_INT:
			M_PlotReal(pl, (M_Real)(*pl->src.integer));
			break;
		case M_PLOT_FROM_COMPONENT:
			if (!M_ENTRY_EXISTS(pl->src.com.A, pl->src.com.i,
			    pl->src.com.j)) {
				break;
			}
			v = M_GetElement(pl->src.com.A, pl->src.com.i,
			                                pl->src.com.j);
			M_PlotReal(pl, *v);
			break;
		case M_PLOT_DERIVATIVE:
			M_PlotDerivative(ptr, pl);
			break;
		default:
			break;
		}
	}
	if (ptr->flags & M_PLOTTER_SCROLL)
		ptr->xOffs++;
}

M_PlotLabel *
M_PlotLabelNew(M_Plot *pl, enum m_plot_label_type type, Uint x, Uint y,
    const char *fmt, ...)
{
	M_PlotLabel *plbl;
	va_list args;

	plbl = Malloc(sizeof(M_PlotLabel));
	plbl->type = type;
	plbl->x = x;
	plbl->y = y;

	va_start(args, fmt);
	Vsnprintf(plbl->text, sizeof(plbl->text), fmt, args);
	va_end(args);

	AG_PushTextState();
	AG_TextFont(pl->plotter->font);
	AG_TextColor32(pl->color);
	plbl->text_surface = AG_WidgetMapSurface(pl->plotter,
	    AG_TextRender(plbl->text));
	AG_PopTextState();

	TAILQ_INSERT_TAIL(&pl->labels, plbl, labels);
	return (plbl);
}

void
M_PlotLabelSetText(M_Plot *pl, M_PlotLabel *plbl, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	Vsnprintf(plbl->text, sizeof(plbl->text), fmt, args);
	va_end(args);

	AG_WidgetUnmapSurface(pl->plotter, plbl->text_surface);
	AG_TextFont(pl->plotter->font);
	AG_TextColor32(pl->color);
	plbl->text_surface = AG_WidgetMapSurface(pl->plotter,
	    AG_TextRender(plbl->text));
}

/* Replace plot labels matching the given text. */
M_PlotLabel *
M_PlotLabelReplace(M_Plot *pl, enum m_plot_label_type type, Uint x, Uint y,
    const char *fmt, ...)
{
	char text[M_PLOTTER_LABEL_MAX];
	M_PlotLabel *plbl;
	va_list args;
	
	va_start(args, fmt);
	Vsnprintf(text, sizeof(text), fmt, args);
	va_end(args);

	TAILQ_FOREACH(plbl, &pl->labels, labels) {
		if (strcmp(text, plbl->text) == 0)
			break;
	}
	if (plbl != NULL) {
		plbl->type = type;
		switch (plbl->type) {
		case M_LABEL_X:
			plbl->x = x;
			break;
		case M_LABEL_Y:
			plbl->y = y;
			break;
		case M_LABEL_FREE:
		case M_LABEL_OVERLAY:
			plbl->x = x;
			plbl->y = y;
			break;
		}
	} else {
		Uint nx = x;
		Uint ny = y;
		AG_Surface *su;
reposition:
		/* Do what we can to avoid overlapping labels */
		TAILQ_FOREACH(plbl, &pl->labels, labels) {
			if (plbl->x != nx || plbl->y != ny) {
				continue;
			}
			su = WSURFACE(pl->plotter,plbl->text_surface);
			switch (plbl->type) {
			case M_LABEL_X:
				ny += su->h;
				break;
			case M_LABEL_Y:
			case M_LABEL_FREE:
			case M_LABEL_OVERLAY:
				nx += su->w;
				break;
			}
			goto reposition;
		}
		plbl = M_PlotLabelNew(pl, type, nx, ny, "%s", text);
	}
	return (plbl);
}

/* Create a new plot. */
M_Plot *
M_PlotNew(M_Plotter *ptr, enum m_plot_type type)
{
	M_Plot *pl, *pl2;
	
	pl = Malloc(sizeof(M_Plot));
	pl->plotter = ptr;
	pl->flags = 0;
	pl->type = type;
	pl->data.r = NULL;
	pl->n = 0;
	pl->src_type = M_PLOT_MANUALLY;
	pl->label = -1;
	pl->label_txt[0] = '\0';
	pl->color = ptr->colors[ptr->curColor++];
	pl->xOffs = 0;
	pl->yOffs = 0;
	pl->xScale = 1.0;
	pl->yScale = 1.0;
	pl->xLabel = 5;
	pl->yLabel = 5;
	TAILQ_FOREACH(pl2, &ptr->plots, plots) {
		if (pl2->label == -1) { continue; }
		pl->xLabel += WSURFACE(ptr,pl2->label)->w + 5;
	}
	TAILQ_INSERT_TAIL(&ptr->plots, pl, plots);
	TAILQ_INIT(&pl->labels);

	if (ptr->curColor >= M_PLOTTER_NDEFCOLORS) {
		ptr->curColor = 0;
	}
	return (pl);
}

/* Plot the derivative of existing Plot data. */
M_Plot *
M_PlotFromDerivative(M_Plotter *ptr, enum m_plot_type type, M_Plot *plot)
{
	M_Plot *pl;

	pl = M_PlotNew(ptr, type);
	pl->src_type = M_PLOT_DERIVATIVE;
	pl->src.plot = plot;
	M_PlotSetLabel(pl, "%s'", plot->label_txt);
	return (pl);
}

/*
 * Plot a Variable from a VFS "object:variable-name" path. This allows the
 * object or variable to safely disappear as long as the VFS root remains.
 */
M_Plot *
M_PlotFromVariableVFS(M_Plotter *ptr, enum m_plot_type type, const char *label,
    void *vfsRoot, const char *varName)
{
	M_Plot *pl;

	pl = M_PlotNew(ptr, type);
	pl->src_type = M_PLOT_FROM_VARIABLE_VFS;
	pl->src.varVFS.vfs = vfsRoot;
	pl->src.varVFS.key = Strdup(varName);
	M_PlotSetLabel(pl, "%s", label);
	return (pl);
}

M_Plot *
M_PlotFromReal(M_Plotter *ptr, enum m_plot_type type, const char *label,
    M_Real *p)
{
	M_Plot *pl;

	pl = M_PlotNew(ptr, type);
	pl->src_type = M_PLOT_FROM_REAL;
	pl->src.real = p;
	M_PlotSetLabel(pl, "%s", label);
	return (pl);
}


M_Plot *
M_PlotFromInt(M_Plotter *ptr, enum m_plot_type type, const char *label,
    int *ip)
{
	M_Plot *pl;

	pl = M_PlotNew(ptr, type);
	pl->src_type = M_PLOT_FROM_INT;
	pl->src.integer = ip;
	M_PlotSetLabel(pl, "%s", label);
	return (pl);
}

void
M_PlotSetColor(M_Plot *pl, Uint8 r, Uint8 g, Uint8 b)
{
	pl->color = AG_MapRGB(agSurfaceFmt, r,g,b);
}

void
M_PlotSetLabel(M_Plot *pl, const char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	Vsnprintf(pl->label_txt, sizeof(pl->label_txt), fmt, args);
	va_end(args);

	M_PlotUpdateLabel(pl);
}

void
M_PlotSetScale(M_Plot *pl, M_Real xScale, M_Real yScale)
{
	if (xScale > 0.0) { pl->xScale = xScale; }
	if (yScale > 0.0) { pl->yScale = yScale; }
}

void
M_PlotSetXoffs(M_Plot *pl, int xOffs)
{
	pl->xOffs = xOffs;
}

void
M_PlotSetYoffs(M_Plot *pl, int yOffs)
{
	pl->yOffs = yOffs;
}

void
M_PlotterSetDefaultFont(M_Plotter *ptr, const char *face, int size)
{
	ptr->font = AG_FetchFont(face, size, 0);
}

void
M_PlotterSetDefaultColor(M_Plotter *ptr, int i, Uint8 r, Uint8 g, Uint8 b)
{
	ptr->colors[i] = AG_MapRGB(agSurfaceFmt, r,g,b);
}

void
M_PlotterSetDefaultScale(M_Plotter *ptr, M_Real xScale, M_Real yScale)
{
	ptr->xScale = xScale;
	ptr->yScale = yScale;
}

AG_WidgetClass mPlotterClass = {
	{
		"AG_Widget:M_Plotter",
		sizeof(M_Plotter),
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

#endif /* ENABLE_GUI */
