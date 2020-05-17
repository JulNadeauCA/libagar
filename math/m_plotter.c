/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/config/enable_gui.h>
#ifdef ENABLE_GUI

/*
 * Graphical plotter widget.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/table.h>
#include <agar/gui/notebook.h>
#include <agar/gui/separator.h>
#include <agar/gui/radio.h>
#include <agar/gui/numerical.h>
#include <agar/gui/primitive.h>

#include <agar/math/m.h>
#include <agar/math/m_plotter.h>
#include <agar/math/m_gui.h>

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
KeyDown(AG_Event *_Nonnull event)
{
	M_Plotter *ptr = M_PLOTTER_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_0:
	case AG_KEY_1:
		ptr->yScale = 1.0;
		AG_Redraw(ptr);
		break;
	case AG_KEY_EQUALS:
		ptr->yScale += 0.125;
		AG_Redraw(ptr);
		break;
	case AG_KEY_MINUS:
		ptr->yScale -= 0.125;
		AG_Redraw(ptr);
		break;
	}
	if (ptr->yScale <= 0.125) { ptr->yScale = 0.125; }
}

static __inline__ int
MouseOverPlotItem(M_Plotter *_Nonnull ptr, M_Plot *_Nonnull pl, int x, int y)
{
	AG_Surface *lbl;
	
	if (pl->label < 0) { return (0); }
	lbl = WSURFACE(ptr,pl->label);
	return (x >= pl->xLabel && x <= (pl->xLabel + lbl->w) &&
	        y >= pl->yLabel && y <= (pl->yLabel + lbl->h));
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	M_Plotter *ptr = M_PLOTTER_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dy = AG_INT(4);
	int state = AG_INT(5);
	M_Plot *pl;

	if (!AG_WidgetRelativeArea(ptr, x, y))
		return;

	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		if (pl->flags & M_PLOT_SELECTED &&
		    state & AG_MOUSE_LEFT) {
			pl->yOffs += dy;
			AG_Redraw(ptr);
		}
		if (MouseOverPlotItem(ptr, pl, x, y)) {
			if (!(pl->flags & M_PLOT_MOUSEOVER)) {
				pl->flags |= M_PLOT_MOUSEOVER;
				AG_Redraw(ptr);
			}
		} else {
			if (pl->flags & M_PLOT_MOUSEOVER) {
				pl->flags &= ~M_PLOT_MOUSEOVER;
				AG_Redraw(ptr);
			}
		}
	}
}

#if 0
static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	M_Plotter *ptr = M_PLOTTER_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	M_Plot *pl;

	switch (button) {
	case AG_MOUSE_LEFT:
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

	if (pl->label >= 0)
		AG_WidgetUnmapSurface(ptr, pl->label);

	AG_PushTextState();
	AG_TextFont(ptr->font);
	AG_TextColor(&pl->color);
	pl->label = (pl->label_txt[0] != '\0') ? -1 :
	    AG_WidgetMapSurface(ptr, AG_TextRender(pl->label_txt));
	AG_PopTextState();

	AG_Redraw(ptr);
}

static void
UpdateLabel(AG_Event *_Nonnull event)
{
	M_PlotUpdateLabel(AG_PTR(1));
}

#ifdef AG_TIMERS
static void
UpdatePlotTbl(AG_Event *_Nonnull event)
{
	AG_Table *tbl = AG_TABLE_SELF();
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
#endif /* AG_TIMERS */

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
	ntab = AG_NotebookAdd(nb, _("Trace"), AG_BOX_VERT);
	{
		AG_RadioNewUint(ntab, 0, type_names, (void *)&pl->type);
		M_NumericalNewReal(ntab, 0, NULL, _("X-scale: "), &pl->xScale);
		M_NumericalNewReal(ntab, 0, NULL, _("Y-scale: "), &pl->yScale);
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		AG_NumericalNewInt(ntab, 0, "px", _("X-offset: "), &pl->xOffs);
		AG_NumericalNewInt(ntab, 0, "px", _("Y-offset: "), &pl->yOffs);
	}
	ntab = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_BindUint8(pal, "RGBAv", (Uint8 *)&pl->color);
		AG_SetEvent(pal, "h-changed", UpdateLabel, "%p", pl);
		AG_SetEvent(pal, "sv-changed", UpdateLabel, "%p", pl);
	}
#ifdef AG_TIMERS
	ntab = AG_NotebookAdd(nb, _("Table"), AG_BOX_VERT);
	{
		AG_Table *tbl;

		tbl = AG_TableNewPolled(ntab, AG_TABLE_MULTI | AG_TABLE_EXPAND,
		    UpdatePlotTbl, "%p", pl);
		AG_TableAddCol(tbl, _("#"), "<88888>", NULL);
		AG_TableAddCol(tbl, _("Value"), NULL, NULL);
	}
#endif
	AG_WindowShow(win);
	return (win);
}

static void
ShowPlotSettings(AG_Event *_Nonnull event)
{
	M_PlotSettings(AG_PTR(1));
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	M_Plotter *ptr = M_PLOTTER_SELF();
	M_Plot *pl, *opl;
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	switch (button) {
	case AG_MOUSE_LEFT:
		AG_WidgetFocus(ptr);
		if (AG_GetModState(ptr) & (AG_KEYMOD_CTRL|AG_KEYMOD_SHIFT)) {
			TAILQ_FOREACH(pl, &ptr->plots, plots) {
				if (!MouseOverPlotItem(ptr, pl, x, y)) {
					continue;
				}
				AG_INVFLAGS(pl->flags, M_PLOT_SELECTED);
				AG_Redraw(ptr);
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
				AG_Redraw(ptr);
			}
		}
		break;
	case AG_MOUSE_RIGHT:
		AG_WidgetFocus(ptr);
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (MouseOverPlotItem(ptr, pl, x, y)) {
				AG_PopupMenu *pm;

				pm = AG_PopupNew(ptr);
				AG_MenuUintFlags(pm->root, _("Display plot"), NULL,
				    &pl->flags, M_PLOT_HIDDEN, 1);
				AG_MenuAction(pm->root, _("Plot settings"), NULL,
				    ShowPlotSettings, "%p", pl);
				AG_PopupShowAt(pm, x,y);
				break;
			}
		}
		break;
	case AG_MOUSE_WHEELDOWN:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (! (pl->flags & M_PLOT_SELECTED)) { continue; }
			pl->yScale -= 0.250;
			AG_Redraw(ptr);
		}
		break;
	case AG_MOUSE_WHEELUP:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (! (pl->flags & M_PLOT_SELECTED)) { continue; }
			pl->yScale += 0.250;
			AG_Redraw(ptr);
		}
		break;
	}
	if (ptr->xScale <= 0.0625) { ptr->xScale = 0.0625; }
	if (ptr->yScale <= 0.0625) { ptr->yScale = 0.0625; }
}

static void
UpdateXBar(AG_Event *_Nonnull event)
{
	M_Plotter *ptr = M_PLOTTER_PTR(1);
	int value = AG_GetInt(ptr->hbar, "value");

	if (value >= ptr->xMax - WIDTH(ptr)) {
		ptr->flags |= M_PLOTTER_SCROLL;
	} else {
		ptr->flags &= ~M_PLOTTER_SCROLL;
	}
}

static void
Init(void *_Nonnull obj)
{
	M_Plotter *ptr = obj;
	AG_Scrollbar *sb;

	WIDGET(ptr)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_TEXT;

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
	ptr->font = agDefaultFont;
	ptr->r.x = 0;
	ptr->r.y = 0;
	ptr->r.w = 0;
	ptr->r.h = 0;
	TAILQ_INIT(&ptr->plots);
	
	ptr->vMin = M_New(3,1);
	ptr->vMax = M_New(3,1);
	M_SetZero(ptr->vMin);
	M_SetZero(ptr->vMax);

	ptr->curColor = 0;

	AG_ColorRGB_8(&ptr->colors[0],  230, 230, 230);
	AG_ColorRGB_8(&ptr->colors[1],  0,   250, 0); 
	AG_ColorRGB_8(&ptr->colors[2],  250, 250, 0);
	AG_ColorRGB_8(&ptr->colors[3],  0,   118, 163);
	AG_ColorRGB_8(&ptr->colors[4],  175, 143, 44);
	AG_ColorRGB_8(&ptr->colors[5],  169, 172, 182);
	AG_ColorRGB_8(&ptr->colors[6],  255, 255, 255);
	AG_ColorRGB_8(&ptr->colors[7],  59,  122, 87);
	AG_ColorRGB_8(&ptr->colors[8],  163, 151, 180);
	AG_ColorRGB_8(&ptr->colors[9],  249, 234, 243);
	AG_ColorRGB_8(&ptr->colors[10], 157, 229, 255);
	AG_ColorRGB_8(&ptr->colors[11], 223, 190, 111);
	AG_ColorRGB_8(&ptr->colors[12], 79,  168, 61);
	AG_ColorRGB_8(&ptr->colors[13], 234, 147, 115);
	AG_ColorRGB_8(&ptr->colors[14], 127, 255, 212);
	AG_ColorRGB_8(&ptr->colors[15], 218, 99,  4);
	
	sb = ptr->hbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_EXCL);
	AG_BindInt(sb, "value",   &ptr->xOffs);
	AG_BindInt(sb, "visible", &WIDGET(ptr)->w);
	AG_SetInt(sb,  "min",     0);
	AG_BindInt(sb, "max",     &ptr->xMax);
	AG_SetEvent(sb, "scrollbar-changed", UpdateXBar, "%p", ptr);

	sb = ptr->vbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_BindInt(sb, "value",   &ptr->yOffs);
	AG_BindInt(sb, "visible", &WIDGET(ptr)->h);
	AG_SetInt(sb,  "min",     0);
	AG_SetInt(sb,  "max",     200);

	AG_SetEvent(ptr, "key-down", KeyDown, NULL);
	AG_SetEvent(ptr, "mouse-button-down", MouseButtonDown, NULL);
/*	AG_SetEvent(ptr, "mouse-button-up", MouseButtonUp, NULL); */
	AG_SetEvent(ptr, "mouse-motion", MouseMotion, NULL);
}

static void
Destroy(void *_Nonnull obj)
{
	M_Plotter *ptr = obj;
	M_Plot *plot, *plotNext;
	M_PlotLabel *plbl, *plblNext;
	Uint i;

	for (plot = TAILQ_FIRST(&ptr->plots);
	     plot != TAILQ_END(&ptr->plots);
	     plot = plotNext) {
		plotNext = TAILQ_NEXT(plot, plots);
		for (plbl = TAILQ_FIRST(&plot->labels);
		     plbl != TAILQ_END(&plot->labels);
		     plbl = plblNext) {
			plblNext = TAILQ_NEXT(plbl, labels);
			free(plbl);
		}
		if (plot->type == M_PLOT_VECTORS) {
			for (i = 0; i < plot->n; i++) {
				M_VecFree(plot->data.v[i]);
			}
			Free(plot->data.v);
		} else {
			Free(plot->data.r);
		}
		free(plot);
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
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	M_Plotter *ptr = obj;

	r->w = ptr->wPre;
	r->h = ptr->hPre;
	if (ptr->flags & M_PLOTTER_SCROLL)
		ptr->xOffs = 0;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	M_Plotter *ptr = obj;
	const AG_Font *font = WIDGET(ptr)->font;
	const int sbThick = font->lineskip;
	AG_SizeAlloc aBar;

	if (a->w < 2 || a->h < 2)
		return (-1);

	ptr->r.w = a->w;
	ptr->r.h = a->h;

	aBar.x = 0;
	aBar.y = a->h - sbThick;
	aBar.w = a->w;
	aBar.h = sbThick;;
	AG_WidgetSizeAlloc(ptr->hbar, &aBar);
	ptr->r.h -= HEIGHT(ptr->hbar);

	aBar.x = a->w - sbThick;
	aBar.y = 0;
	aBar.w = sbThick;
	aBar.h = a->h - sbThick;
	AG_WidgetSizeAlloc(ptr->vbar, &aBar);
	ptr->r.w -= WIDTH(ptr->vbar);
	
	return (0);
}

static __inline__ M_Real
ScaleReal(M_Plotter *_Nonnull ptr, M_Plot *_Nonnull pl, M_Real r)
{
	return (r*(ptr->yScale * pl->yScale));
}

static void
Draw(void *_Nonnull obj)
{
	M_Plotter *ptr = obj;
	const AG_Color *cBg = &WCOLOR(ptr, BG_COLOR);
	const AG_Color *cText = &WCOLOR(ptr, TEXT_COLOR);
	M_Plot *pl;
	M_PlotLabel *plbl;
	AG_Rect r;
	Uint i;
	int h = ptr->r.h, h_2 = (h >> 1);
	int w = ptr->r.w;
	int y0 = h_2;

	r.x = 1;
	r.y = 1;
	r.w = WIDTH(ptr)-2;
	r.h = HEIGHT(ptr)-2;

	AG_DrawBoxSunk(ptr, &r, cBg);

	AG_PushClipRect(ptr, &r);
	
	AG_DrawLineH(ptr, 1, w-2, y0 - ptr->yOffs, &ptr->colors[0]);
	AG_DrawLineV(ptr, ptr->xMax-1, r.y, r.h,   &ptr->colors[0]);

	AG_PushBlendingMode(ptr, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	/* First pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		AG_Color color = pl->color;
		int yOffs = pl->yOffs - ptr->yOffs;
		int x = pl->xOffs - ptr->xOffs, y;
		int py = y0 + yOffs;

		if (pl->label >= 0) {
			AG_Surface *su = WSURFACE(ptr,pl->label);
			int xLabel = pl->xLabel;
			int yLabel = pl->yLabel;

			r.x = xLabel - 2;
			r.y = yLabel - 2;
			r.w = su->w + 4;
			r.h = su->h + 4;

			if (pl->flags & M_PLOT_SELECTED) {
				AG_DrawRectOutline(ptr, &r, &color);
			} else if (pl->flags & M_PLOT_MOUSEOVER) {
				AG_DrawRectOutline(ptr, &r, cText);
			}
			AG_WidgetBlitSurface(ptr, pl->label, xLabel, yLabel);
		}
		if (pl->flags & M_PLOT_HIDDEN) {
			continue;
		}
		switch (pl->type) {
		case M_PLOT_POINTS: {
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				AG_PutPixel(ptr, x, y0-y+yOffs, &color);
				if (x > w) { break; }
			}
			break;
		}
		case M_PLOT_LINEAR:
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				AG_DrawLine(ptr, x-1, py, x, y0-y+yOffs, &color);
				py = y0-y+yOffs;
				if (x > w) { break; }
			}
			break;
		default:
			break;
		}
	}
	/* Second pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		int xOffs;

		if (pl->flags & M_PLOT_HIDDEN) {
			continue;
		}
		xOffs = ptr->xOffs - pl->xOffs;

		TAILQ_FOREACH(plbl, &pl->labels, labels) {
			AG_Surface *su = WSURFACE(ptr,plbl->text_surface);
			AG_Color cLblBg, cLine;
			int xLbl, yLbl;

			switch (plbl->type) {
			case M_LABEL_X:
				xLbl = plbl->x - xOffs;
				yLbl = h - su->h - 4 - plbl->y;
				cLine = pl->color;
				cLine.a >>= 1;
				AG_DrawLineV(ptr, xLbl, 1, h-2, &cLine);
				break;
			case M_LABEL_Y:
				xLbl = plbl->x - xOffs;
				yLbl = h - su->h - 4 - plbl->y;
				break;
			case M_LABEL_FREE:
				xLbl = 4 + plbl->x - xOffs;
				yLbl = 4 + plbl->y;
				break;
			default:
				xLbl = 4 + plbl->x;
				yLbl = 4 + plbl->y;
				break;
			}
			cLblBg = *cBg;
			cLblBg.a = AG_8toH(200);

			r.x = xLbl + 2;
			r.y = yLbl;
			r.w = su->w;
			r.h = su->h;
			AG_DrawRect(ptr, &r, &cLblBg);
			AG_WidgetBlitSurface(ptr, plbl->text_surface, r.x, r.y);
		}
	}

	AG_PopBlendingMode(ptr);
	AG_PopClipRect(ptr);
	
	AG_WidgetDraw(ptr->hbar);
	AG_WidgetDraw(ptr->vbar);
}

void
M_PlotClear(M_Plot *pl)
{
	pl->data.r = Realloc(pl->data.r, sizeof(M_Real));
	pl->n = 0;
	AG_Redraw(pl->plotter);
}

void
M_PlotReal(M_Plot *pl, M_Real v)
{
	M_Plotter *ptr = pl->plotter;

	pl->data.r = Realloc(pl->data.r, (pl->n+1)*sizeof(M_Real));
	pl->data.r[pl->n] = v;
	if (++pl->n > ptr->xMax) { ptr->xMax = pl->n; }
	if (v > ptr->yMax) { ptr->yMax = v; }
	if (v < ptr->yMin) { ptr->yMin = v; }
	AG_Redraw(ptr);
}

void
M_PlotRealv(M_Plot *pl, Uint n, const M_Real *vp)
{
	M_Plotter *ptr = pl->plotter;
	Uint i;

	pl->data.r = Realloc(pl->data.r, (pl->n+n)*sizeof(M_Real));
	memcpy(&pl->data.r[pl->n], vp, n*sizeof(M_Real));
	if ((pl->n += n) > ptr->xMax) { ptr->xMax = pl->n; }
	for (i = 0; i < n; i++) {
		if (vp[i] > ptr->yMax) { ptr->yMax = vp[i]; }
		if (vp[i] < ptr->yMin) { ptr->yMin = vp[i]; }
	}
	AG_Redraw(ptr);
}

static void
VectorMinimum(M_Vector *_Nonnull c, const M_Vector *_Nonnull a,
    const M_Vector *b)
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
VectorMaximum(M_Vector *_Nonnull c, const M_Vector *_Nonnull a,
    const M_Vector *_Nonnull b)
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
M_PlotVector(M_Plot *_Nonnull pl, const M_Vector *_Nonnull v)
{
	M_Plotter *ptr = pl->plotter;
	int i;

	pl->data.v = Realloc(pl->data.v, (pl->n)*sizeof(M_Vector *));
	pl->data.v[pl->n] = M_VecNew(v->m);
	VectorMinimum(ptr->vMin, ptr->vMin, v);
	VectorMaximum(ptr->vMax, ptr->vMax, v);
	for (i = 0; i < v->m; i++) {
		M_Real *e = M_VecGetElement(pl->data.v[pl->n], i);
		*e = M_VecGet(v, i);
	}
	pl->n++;
	AG_Redraw(ptr);
}

void
M_PlotVectorv(M_Plot *_Nonnull pl, Uint n, const M_Vector *_Nonnull *_Nonnull vp)
{
	M_Plotter *ptr = pl->plotter;
	Uint i;

	pl->data.v = Realloc(pl->data.v, (pl->n+n)*sizeof(M_Vector));
	for (i = 0; i < n; i++) {
		pl->data.v[pl->n+i] = M_VecNew(vp[i]->m);
		VectorMinimum(ptr->vMin, ptr->vMin, vp[i]);
		VectorMaximum(ptr->vMax, ptr->vMax, vp[i]);
		M_Copy(pl->data.v[pl->n+i], vp[i]);
	}
	pl->n += n;
	AG_Redraw(ptr);
}

static __inline__ void
M_PlotDerivative(M_Plotter *_Nonnull ptr, M_Plot *_Nonnull dp)
{
	M_Plot *p = dp->src.plot;

	if (p->n >= 2) {
		M_PlotReal(dp, p->data.r[p->n-1] - p->data.r[p->n-2]);
	} else {
		M_PlotReal(dp, p->data.r[p->n-1]);
	}
}

static __inline__ M_Real
PlotVariableVFS(M_Plotter *_Nonnull ptr, M_Plot *_Nonnull pl)
{
	char key[AG_OBJECT_PATH_MAX + 65];
	char *s, *objName, *varName;
	AG_Variable *V;
	M_Real rv;
	void *obj;

	Strlcpy(key, pl->src.varVFS.key, sizeof(key));
	s = &key[0];
	objName = Strsep(&s, ":");
	varName = Strsep(&s, ":");
	if (objName == NULL || varName == NULL ||
	    objName[0] == '\0' || varName[0] == '\0') {
		AG_Verbose("M_Plotter: Bad key \"%s\"\n", pl->src.varVFS.key);
		return (0.0);
	}
	if ((obj = AG_ObjectFindS(pl->src.varVFS.vfs, objName)) == NULL) {
		return (0.0);
	}
	if ((V = AG_AccessVariable(obj, varName)) == NULL) {
		AG_Verbose("M_Plotter (\"%s\"): %s\n", pl->src.varVFS.key,
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
		AG_Verbose("M_Plotter(\"%s\"): Unimplemented type\n",
		    pl->src.varVFS.key);
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
		}
	}
	if (ptr->flags & M_PLOTTER_SCROLL) {
		ptr->xOffs++;
		AG_Redraw(ptr);
	}
}

M_PlotLabel *
M_PlotLabelNew(M_Plot *pl, enum m_plot_label_type type, Uint x, Uint y,
    const char *fmt, ...)
{
	M_Plotter *ptr = pl->plotter;
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
	AG_TextFont(ptr->font);
	AG_TextColor(&pl->color);
	plbl->text_surface = AG_WidgetMapSurface(ptr, AG_TextRender(plbl->text));
	AG_PopTextState();

	TAILQ_INSERT_TAIL(&pl->labels, plbl, labels);

	AG_Redraw(ptr);
	return (plbl);
}

void
M_PlotLabelSetText(M_Plot *pl, M_PlotLabel *plbl, const char *fmt, ...)
{
	M_Plotter *ptr = pl->plotter;
	va_list args;

	va_start(args, fmt);
	Vsnprintf(plbl->text, sizeof(plbl->text), fmt, args);
	va_end(args);

	AG_WidgetUnmapSurface(ptr, plbl->text_surface);

	AG_PushTextState();
	AG_TextFont(ptr->font);
	AG_TextColor(&pl->color);
	plbl->text_surface = AG_WidgetMapSurface(ptr, AG_TextRender(plbl->text));
	AG_PopTextState();

	AG_Redraw(ptr);
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
		/* XXX */
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
	AG_Redraw(ptr);
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
	AG_ColorRGB_8(&pl->color, r,g,b);
	AG_Redraw(pl->plotter);
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
	AG_Redraw(pl->plotter);
}

void
M_PlotSetXoffs(M_Plot *pl, int xOffs)
{
	pl->xOffs = xOffs;
	AG_Redraw(pl->plotter);
}

void
M_PlotSetYoffs(M_Plot *pl, int yOffs)
{
	pl->yOffs = yOffs;
	AG_Redraw(pl->plotter);
}

void
M_PlotterSetDefaultFont(M_Plotter *ptr, const char *face, float sizePts)
{
	if (ptr->font != agDefaultFont) {
		AG_UnusedFont(ptr->font);
	}
	ptr->font = AG_FetchFont(face, sizePts, 0);
	AG_Redraw(ptr);
}

void
M_PlotterSetDefaultColor(M_Plotter *ptr, int i, Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&ptr->colors[i], r,g,b);
	AG_Redraw(ptr);
}

void
M_PlotterSetDefaultScale(M_Plotter *ptr, M_Real xScale, M_Real yScale)
{
	ptr->xScale = xScale;
	ptr->yScale = yScale;
	AG_Redraw(ptr);
}

AG_WidgetClass mPlotterClass = {
	{
		"AG_Widget:M_Plotter",
		sizeof(M_Plotter),
		{ 0,0 },
		Init,
		NULL,			/* reset */
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
