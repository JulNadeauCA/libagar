/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "sc_plotter.h"
#include "sc_gui.h"

#include <gui/primitive.h>
#include <gui/table.h>
#include <gui/notebook.h>
#include <gui/separator.h>
#include <gui/mfspinbutton.h>
#include <gui/radio.h>
#include <gui/hsvpal.h>

#include <stdarg.h>
#include <string.h>

SC_Plotter *
SC_PlotterNew(void *parent, Uint flags)
{
	SC_Plotter *pl;

	pl = Malloc(sizeof(SC_Plotter));
	AG_ObjectInit(pl, &scPlotterClass);
	pl->flags |= flags;
	
	if (flags & SC_PLOTTER_HFILL) WIDGET(pl)->flags |= AG_WIDGET_HFILL;
	if (flags & SC_PLOTTER_VFILL) WIDGET(pl)->flags |= AG_WIDGET_VFILL;

	AG_ObjectAttach(parent, pl);
	return (pl);
}

static void
keydown(AG_Event *event)
{
	SC_Plotter *ptr = AG_SELF();
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
MouseOverPlotItem(SC_Plotter *ptr, SC_Plot *pl, int x, int y)
{
	SDL_Surface *lbl;
	
	if (pl->label < 0) { return (0); }
	lbl = WSURFACE(ptr,pl->label);
	return (x >= pl->xLabel && x <= (pl->xLabel + lbl->w) &&
	        y >= pl->yLabel && y <= (pl->yLabel + lbl->h));
}

static void
mousemotion(AG_Event *event)
{
	SC_Plotter *ptr = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int dy = AG_INT(4);
	int state = AG_INT(5);
	SC_Plot *pl;

	if (!AG_WidgetRelativeArea(ptr, x, y))
		return;

	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		if (pl->flags & SC_PLOT_SELECTED &&
		    state & SDL_BUTTON_LEFT) {
			pl->yOffs += dy;
		}
		if (MouseOverPlotItem(ptr, pl, x, y)) {
			pl->flags |= SC_PLOT_MOUSEOVER;
		} else {
			pl->flags &= ~SC_PLOT_MOUSEOVER;
		}
	}
}

#if 0
static void
mousebuttonup(AG_Event *event)
{
	SC_Plotter *ptr = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	SC_Plot *pl;

	switch (button) {
	case SDL_BUTTON_LEFT:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			pl->flags &= ~SC_PLOT_DRAGGING;
		}
		break;
	}
}
#endif

void
SC_PlotUpdateLabel(SC_Plot *pl)
{
	SC_Plotter *ptr = pl->plotter;

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
	SC_PlotUpdateLabel(AG_PTR(1));
}

static void
UpdatePlotTbl(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();
	SC_Plot *pl = AG_PTR(1);
	Uint i, j;

	AG_TableBegin(tbl);
	for (i = 0; i < pl->n; i++) {
		if (pl->type == SC_PLOT_VECTORS) {
			SC_Vector *v = pl->data.v[i];
			for (j = 1; j <= v->m; j++) {
				AG_TableAddRow(tbl, "%u[%u]:%f", i, j,
				    v->mat[j][1]);
			}
		} else {
			AG_TableAddRow(tbl, "%u:%f", pl->data.r[i]);
		}
	}
	AG_TableEnd(tbl);
}

AG_Window *
SC_PlotSettings(SC_Plot *pl)
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
		AG_FSpinbutton *fsb;
		AG_Radio *rad;

		rad = AG_RadioNew(ntab, AG_RADIO_HFILL, type_names);
		AG_WidgetBindInt(rad, "value", &pl->type);
	
		fsb = AG_FSpinbuttonNew(ntab, 0, NULL, _("X-scale: "));
		SC_WidgetBindReal(fsb, "value", &pl->xScale);
		fsb = AG_FSpinbuttonNew(ntab, 0, NULL, _("Y-scale: "));
		SC_WidgetBindReal(fsb, "value", &pl->yScale);
	
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
	
		fsb = AG_FSpinbuttonNew(ntab, 0, "px", _("X-offset: "));
		AG_WidgetBindInt(fsb, "value", &pl->xOffs);
		fsb = AG_FSpinbuttonNew(ntab, 0, "px", _("Y-offset: "));
		AG_WidgetBindInt(fsb, "value", &pl->yOffs);
		AG_FSpinbuttonSetIncrement(fsb, 5.0);
	}
	ntab = AG_NotebookAddTab(nb, _("Color"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;

		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_WidgetBindPointer(pal, "pixel-format", &agSurfaceFmt);
		AG_WidgetBindUint32(pal, "pixel", &pl->color);
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
	SC_Plot *pl = AG_PTR(1);

	SC_PlotSettings(pl);
}

static void
mousebuttondown(AG_Event *event)
{
	SC_Plotter *ptr = AG_SELF();
	SC_Plot *pl, *opl;
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
				if (pl->flags & SC_PLOT_SELECTED) {
					pl->flags &= ~SC_PLOT_SELECTED;
				} else {
					pl->flags |= SC_PLOT_SELECTED;
				}
			}
		} else {
			TAILQ_FOREACH(pl, &ptr->plots, plots) {
				if (MouseOverPlotItem(ptr, pl, x, y))
					break;
			}
			if (pl != NULL) {
				TAILQ_FOREACH(opl, &ptr->plots, plots) {
					opl->flags &= ~SC_PLOT_SELECTED;
				}
				pl->flags |= SC_PLOT_SELECTED;
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
				    &pl->flags, SC_PLOT_HIDDEN, 1);
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
			if (! (pl->flags & SC_PLOT_SELECTED)) { continue; }
			pl->yScale -= 0.250;
		}
		break;
	case SDL_BUTTON_WHEELUP:
		TAILQ_FOREACH(pl, &ptr->plots, plots) {
			if (! (pl->flags & SC_PLOT_SELECTED)) { continue; }
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
	SC_Plotter *ptr = AG_PTR(1);
	int value = AG_INT(2);

	if (value >= ptr->xMax - WIDGET(ptr)->w) {
		ptr->flags |= SC_PLOTTER_SCROLL;
	} else {
		ptr->flags &= ~SC_PLOTTER_SCROLL;
	}
}

static void
Init(void *obj)
{
	SC_Plotter *ptr = obj;

	WIDGET(ptr)->flags |= AG_WIDGET_CLIPPING|AG_WIDGET_FOCUSABLE;

	ptr->type = SC_PLOT_2D;
	ptr->flags = 0;
	ptr->xMax = 0;
	ptr->yMin = 0.0;
	ptr->yMax = 0.0;
	ptr->vMin = SC_VectorNewZero(3);
	ptr->vMax = SC_VectorNewZero(3);
	ptr->xOffs = 0;
	ptr->yOffs = 0;
	ptr->wPre = 128;
	ptr->hPre = 64;
	ptr->xScale = 1.0;
	ptr->yScale = 1.0;
	ptr->font = AG_FetchFont(NULL, -1, -1);
	TAILQ_INIT(&ptr->plots);

	ptr->curColor = 0;
	ptr->colors[0] = SDL_MapRGB(agSurfaceFmt, 255, 255, 255);
	ptr->colors[1] = SDL_MapRGB(agSurfaceFmt, 0, 250, 0); 
	ptr->colors[2] = SDL_MapRGB(agSurfaceFmt, 250, 250, 0);
	ptr->colors[3] = SDL_MapRGB(agSurfaceFmt, 0, 118, 163);
	ptr->colors[4] = SDL_MapRGB(agSurfaceFmt, 175, 143, 44);
	ptr->colors[5] = SDL_MapRGB(agSurfaceFmt, 169, 172, 182);
	ptr->colors[6] = SDL_MapRGB(agSurfaceFmt, 255, 255, 255);
	ptr->colors[7] = SDL_MapRGB(agSurfaceFmt, 59, 122, 87);
	ptr->colors[8] = SDL_MapRGB(agSurfaceFmt, 163, 151, 180);
	ptr->colors[9] = SDL_MapRGB(agSurfaceFmt, 249, 234, 243);
	ptr->colors[10] = SDL_MapRGB(agSurfaceFmt, 157, 229, 255);
	ptr->colors[11] = SDL_MapRGB(agSurfaceFmt, 223, 190, 111);
	ptr->colors[12] = SDL_MapRGB(agSurfaceFmt, 79, 168, 61);
	ptr->colors[13] = SDL_MapRGB(agSurfaceFmt, 234, 147, 115);
	ptr->colors[14] = SDL_MapRGB(agSurfaceFmt, 127, 255, 212);
	ptr->colors[15] = SDL_MapRGB(agSurfaceFmt, 218, 99, 4);
	
	ptr->hbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_HORIZ, 0);
	ptr->vbar = AG_ScrollbarNew(ptr, AG_SCROLLBAR_VERT, 0);
	AG_WidgetBind(ptr->hbar, "value", AG_WIDGET_INT, &ptr->xOffs);
	AG_WidgetBind(ptr->hbar, "visible", AG_WIDGET_INT, &WIDGET(ptr)->w);
	AG_WidgetBind(ptr->hbar, "max", AG_WIDGET_INT, &ptr->xMax);
	AG_SetEvent(ptr->hbar, "scrollbar-changed", UpdateXBar, "%p", ptr);

	AG_WidgetBind(ptr->vbar, "value", AG_WIDGET_INT, &ptr->yOffs);
//	AG_WidgetBind(ptr->vbar, "max", AG_WIDGET_INT, &ptr->yMax);
	AG_WidgetSetInt(ptr->hbar, "min", 0);
	AG_WidgetSetInt(ptr->vbar, "min", 0);

	AG_SetEvent(ptr, "window-keydown", keydown, NULL);
	AG_SetEvent(ptr, "window-mousebuttondown", mousebuttondown, NULL);
//	AG_SetEvent(ptr, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(ptr, "window-mousemotion", mousemotion, NULL);
}

void
SC_PlotterSizeHint(SC_Plotter *ptr, Uint w, Uint h)
{
	ptr->wPre = w;
	ptr->hPre = h;
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	SC_Plotter *ptr = p;

	r->w = ptr->wPre;
	r->h = ptr->hPre;
	if (ptr->flags & SC_PLOTTER_SCROLL)
		ptr->xOffs = 0;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	SC_Plotter *ptr = p;
	AG_SizeAlloc aChld;

	if (a->w < 2 || a->h < 2)
		return (-1);

	aChld.x = 0;
	aChld.y = a->h - ptr->hbar->bw;
	aChld.w = a->w;
	aChld.h = ptr->hbar->bw;
	AG_WidgetSizeAlloc(ptr->hbar, &aChld);

	aChld.x = a->w - ptr->vbar->bw;
	aChld.y = ptr->vbar->bw;
	aChld.w = ptr->vbar->bw;
	aChld.h = a->h - ptr->hbar->bw;
	AG_WidgetSizeAlloc(ptr->vbar, &aChld);
	return (0);
}

static __inline__ SC_Real
ScaleReal(SC_Plotter *ptr, SC_Plot *pl, SC_Real r)
{
	return (r*(ptr->yScale*pl->yScale));
}

static void
Draw(void *p)
{
	SC_Plotter *ptr = p;
	SC_Plot *pl;
	SC_PlotLabel *plbl;
	int y0 = WIDGET(ptr)->h/2;
	Uint i;

	AG_DrawBox(ptr,
	    AG_RECT(0, 0, WIDGET(ptr)->w, WIDGET(ptr)->h), -1,
	    AG_COLOR(GRAPH_BG_COLOR));
	AG_DrawLineH(ptr, 1, WIDGET(ptr)->w-2, y0, ptr->colors[0]);
	AG_DrawLineV(ptr, ptr->xMax-1, 30, WIDGET(ptr)->h-30, ptr->colors[0]);

	/* First pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		int x = pl->xOffs - ptr->xOffs;
		int y, py = y0+pl->yOffs+ptr->yOffs;

		if (pl->label >= 0) {
			SDL_Surface *su = WSURFACE(ptr,pl->label);

			if (pl->flags & SC_PLOT_SELECTED) {
				AG_DrawRectOutline(ptr,
				    AG_RECT(pl->xLabel-2, pl->yLabel-2,
				            su->w+4, su->h+4),
				    AG_VideoPixel(pl->color));
			} else if (pl->flags & SC_PLOT_MOUSEOVER) {
				AG_DrawRectOutline(ptr,
				    AG_RECT(pl->xLabel-2, pl->yLabel-2,
				            su->w+4, su->h+4),
				    AG_COLOR(TEXT_COLOR));
			}
			AG_WidgetBlitSurface(ptr, pl->label, pl->xLabel,
			    pl->yLabel);
		}
		if (pl->flags & SC_PLOT_HIDDEN) {
			continue;
		}
		switch (pl->type) {
		case SC_PLOT_POINTS:
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				if (agView->opengl) {
					/* TODO */
				} else {
					AG_WidgetPutPixel(ptr, x,
					    y0 - y + pl->yOffs + ptr->yOffs,
					    AG_VideoPixel(pl->color));
				}
				if (x > WIDGET(ptr)->w) { break; }
			}
			break;
		case SC_PLOT_LINEAR:
			for (i = 0; i < pl->n; i++, x++) {
				if (x < 0) { continue; }
				y = ScaleReal(ptr, pl, pl->data.r[i]);
				AG_DrawLine(ptr, x-1, py, x,
				    y0 - y + pl->yOffs + ptr->yOffs,
				    AG_VideoPixel(pl->color));
				py = y0 - y + pl->yOffs + ptr->yOffs;
				if (x > WIDGET(ptr)->w) { break; }
			}
			break;
		default:
			break;
		}
	}
	/* Second pass */
	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		if (pl->flags & SC_PLOT_HIDDEN) {
			continue;
		}
		TAILQ_FOREACH(plbl, &pl->labels, labels) {
			SDL_Surface *su = WSURFACE(ptr,plbl->text_surface);
			int xLbl, yLbl;
			Uint8 colLine[4];
			Uint8 colBG[4];

			SDL_GetRGB(AG_COLOR(GRAPH_BG_COLOR), agVideoFmt,
			    &colBG[0], &colBG[1], &colBG[2]);
			SDL_GetRGBA(pl->color, agSurfaceFmt,
			    &colLine[0], &colLine[1], &colLine[2], &colLine[3]);
			colLine[3] /= 2;
			colBG[3] = 200;

			switch (plbl->type) {
			case SC_LABEL_X:
				xLbl = plbl->x - ptr->xOffs - pl->xOffs;
				yLbl = WIDGET(ptr)->h -
				       WIDGET(ptr->hbar)->h -
				       su->h - 4 - plbl->y;
				AG_DrawLineBlended(ptr,
				    xLbl, 1,
				    xLbl, WIDGET(ptr)->h-2,
				    colLine, AG_ALPHA_SRC);
				break;
			case SC_LABEL_Y:
				xLbl = plbl->x - ptr->xOffs - pl->xOffs;
				yLbl = WIDGET(ptr)->h -
				       WIDGET(ptr->hbar)->h -
				       su->h - 4 - plbl->y;
				break;
			case SC_LABEL_FREE:
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
}

void
SC_PlotClear(SC_Plot *pl)
{
	pl->data.r = Realloc(pl->data.r, sizeof(SC_Real));
	pl->n = 0;
}

void
SC_PlotReal(SC_Plot *pl, SC_Real v)
{
	pl->data.r = Realloc(pl->data.r, (pl->n+1)*sizeof(SC_Real));
	pl->data.r[pl->n] = v;
	if (++pl->n > pl->plotter->xMax) { pl->plotter->xMax = pl->n; }
	if (v > pl->plotter->yMax) { pl->plotter->yMax = v; }
	if (v < pl->plotter->yMin) { pl->plotter->yMin = v; }
}

void
SC_PlotRealv(SC_Plot *pl, Uint n, const SC_Real *vp)
{
	Uint i;

	pl->data.r = Realloc(pl->data.r, (pl->n+n)*sizeof(SC_Real));
	memcpy(&pl->data.r[pl->n], vp, n*sizeof(SC_Real));
	if ((pl->n += n) > pl->plotter->xMax) { pl->plotter->xMax = pl->n; }
	for (i = 0; i < n; i++) {
		if (vp[i] > pl->plotter->yMax) { pl->plotter->yMax = vp[i]; }
		if (vp[i] < pl->plotter->yMin) { pl->plotter->yMin = vp[i]; }
	}
}

void
SC_PlotVector(SC_Plot *pl, const SC_Vector *v)
{
	pl->data.v = Realloc(pl->data.v, (pl->n+1)*sizeof(SC_Vector));
	pl->data.v[pl->n] = SC_VectorNew(v->n);
	SC_VectorMinimum(pl->plotter->vMin, pl->plotter->vMin, v);
	SC_VectorMaximum(pl->plotter->vMax, pl->plotter->vMax, v);
	SC_VectorCopy(v, pl->data.v[pl->n]);
	pl->n++;
}

void
SC_PlotVectorv(SC_Plot *pl, Uint n, const SC_Vector **vp)
{
	Uint i;

	pl->data.v = Realloc(pl->data.v, (pl->n+n)*sizeof(SC_Vector));
	for (i = 0; i < n; i++) {
		pl->data.v[pl->n+i] = SC_VectorNew(vp[i]->n);
		SC_VectorMinimum(pl->plotter->vMin, pl->plotter->vMin, vp[i]);
		SC_VectorMaximum(pl->plotter->vMax, pl->plotter->vMax, vp[i]);
		SC_VectorCopy(vp[i], pl->data.v[pl->n+i]);
	}
	pl->n += n;
}

static __inline__ void
SC_PlotDerivative(SC_Plotter *ptr, SC_Plot *dp)
{
	SC_Plot *p = dp->src.plot;

	if (p->n >= 2) {
		SC_PlotReal(dp, p->data.r[p->n-1] - p->data.r[p->n-2]);
	} else {
		SC_PlotReal(dp, p->data.r[p->n-1]);
	}
}

static __inline__ SC_Real
SC_PlotPropVal(SC_Plotter *ptr, SC_Plot *pl)
{
	AG_Prop *prop;

	if ((prop = AG_FindProp(pl->src.prop, -1, NULL)) == NULL) {
		printf("%s\n", AG_GetError());
		return (0.0);
	}
	switch (prop->type) {
	case AG_PROP_FLOAT:	return ((SC_Real)prop->data.f);
	case AG_PROP_DOUBLE:	return ((SC_Real)prop->data.d);
	case AG_PROP_INT:	return ((SC_Real)prop->data.i);
	case AG_PROP_UINT:	return ((SC_Real)prop->data.u);
	}
	return (0.0);
}

void
SC_PlotterUpdate(SC_Plotter *ptr)
{
	SC_Plot *pl;

	TAILQ_FOREACH(pl, &ptr->plots, plots) {
		switch (pl->src_type) {
		case SC_PLOT_MANUALLY:
			break;
		case SC_PLOT_FROM_PROP:
			SC_PlotReal(pl, SC_PlotPropVal(ptr, pl));
			break;
		case SC_PLOT_FROM_REAL:
			SC_PlotReal(pl, *pl->src.real);
			break;
		case SC_PLOT_FROM_INT:
			SC_PlotReal(pl, (SC_Real)(*pl->src.integer));
			break;
		case SC_PLOT_FROM_COMPONENT:
			if (!SC_MatrixEntryExists(pl->src.com.A,
			    pl->src.com.i, pl->src.com.j)) {
				break;
			}
			SC_PlotReal(pl, pl->src.com.A->mat[pl->src.com.i]
			                                       [pl->src.com.j]);
			break;
		case SC_PLOT_DERIVATIVE:
			SC_PlotDerivative(ptr, pl);
			break;
		default:
			break;
		}
	}
	if (ptr->flags & SC_PLOTTER_SCROLL)
		ptr->xOffs++;
}

SC_PlotLabel *
SC_PlotLabelNew(SC_Plot *pl, enum sc_plot_label_type type, Uint x, Uint y,
    const char *fmt, ...)
{
	SC_PlotLabel *plbl;
	va_list args;

	plbl = Malloc(sizeof(SC_PlotLabel));
	plbl->type = type;
	plbl->x = x;
	plbl->y = y;

	va_start(args, fmt);
	vsnprintf(plbl->text, sizeof(plbl->text), fmt, args);
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
SC_PlotLabelSetText(SC_Plot *pl, SC_PlotLabel *plbl, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(plbl->text, sizeof(plbl->text), fmt, args);
	va_end(args);

	AG_WidgetUnmapSurface(pl->plotter, plbl->text_surface);
	AG_TextFont(pl->plotter->font);
	AG_TextColor32(pl->color);
	plbl->text_surface = AG_WidgetMapSurface(pl->plotter,
	    AG_TextRender(plbl->text));
}

/* Replace plot labels matching the given text. */
SC_PlotLabel *
SC_PlotLabelReplace(SC_Plot *pl, enum sc_plot_label_type type, Uint x, Uint y,
    const char *fmt, ...)
{
	char text[SC_PLOTTER_LABEL_MAX];
	SC_PlotLabel *plbl;
	va_list args;
	
	va_start(args, fmt);
	vsnprintf(text, sizeof(text), fmt, args);
	va_end(args);

	TAILQ_FOREACH(plbl, &pl->labels, labels) {
		if (strcmp(text, plbl->text) == 0)
			break;
	}
	if (plbl != NULL) {
		plbl->type = type;
		switch (plbl->type) {
		case SC_LABEL_X:
			plbl->x = x;
			break;
		case SC_LABEL_Y:
			plbl->y = y;
			break;
		case SC_LABEL_FREE:
		case SC_LABEL_OVERLAY:
			plbl->x = x;
			plbl->y = y;
			break;
		}
	} else {
		Uint nx = x;
		Uint ny = y;
		SDL_Surface *su;
reposition:
		/* Do what we can to avoid overlapping labels */
		TAILQ_FOREACH(plbl, &pl->labels, labels) {
			if (plbl->x != nx || plbl->y != ny) {
				continue;
			}
			su = WSURFACE(pl->plotter,plbl->text_surface);
			switch (plbl->type) {
			case SC_LABEL_X:
				ny += su->h;
				break;
			case SC_LABEL_Y:
			case SC_LABEL_FREE:
			case SC_LABEL_OVERLAY:
				nx += su->w;
				break;
			}
			goto reposition;
		}
		plbl = SC_PlotLabelNew(pl, type, nx, ny, "%s", text);
	}
	return (plbl);
}

SC_Plot *
SC_PlotNew(SC_Plotter *ptr, enum sc_plot_type type)
{
	SC_Plot *pl, *pl2;
	
	pl = Malloc(sizeof(SC_Plot));
	pl->plotter = ptr;
	pl->flags = 0;
	pl->type = type;
	pl->data.r = NULL;
	pl->n = 0;
	pl->src_type = SC_PLOT_MANUALLY;
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

	if (ptr->curColor >= SC_PLOTTER_NDEFCOLORS) {
		ptr->curColor = 0;
	}
	return (pl);
}

SC_Plot *
SC_PlotFromDerivative(SC_Plotter *ptr, enum sc_plot_type type, SC_Plot *plot)
{
	SC_Plot *pl;

	pl = SC_PlotNew(ptr, type);
	pl->src_type = SC_PLOT_DERIVATIVE;
	pl->src.plot = plot;
	SC_PlotSetLabel(pl, "%s'", plot->label_txt);
	return (pl);
}

SC_Plot *
SC_PlotFromProp(SC_Plotter *ptr, enum sc_plot_type type, const char *label,
    const char *prop)
{
	SC_Plot *pl;

	pl = SC_PlotNew(ptr, type);
	pl->src_type = SC_PLOT_FROM_PROP;
	pl->src.prop = Strdup(prop);
	SC_PlotSetLabel(pl, "%s", label);
	return (pl);
}

SC_Plot *
SC_PlotFromReal(SC_Plotter *ptr, enum sc_plot_type type, const char *label,
    SC_Real *p)
{
	SC_Plot *pl;

	pl = SC_PlotNew(ptr, type);
	pl->src_type = SC_PLOT_FROM_REAL;
	pl->src.real = p;
	SC_PlotSetLabel(pl, "%s", label);
	return (pl);
}


SC_Plot *
SC_PlotFromInt(SC_Plotter *ptr, enum sc_plot_type type, const char *label,
    int *ip)
{
	SC_Plot *pl;

	pl = SC_PlotNew(ptr, type);
	pl->src_type = SC_PLOT_FROM_INT;
	pl->src.integer = ip;
	SC_PlotSetLabel(pl, "%s", label);
	return (pl);
}

void
SC_PlotSetColor(SC_Plot *pl, Uint8 r, Uint8 g, Uint8 b)
{
	pl->color = SDL_MapRGB(agSurfaceFmt, r, g, b);
}

void
SC_PlotSetLabel(SC_Plot *pl, const char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	vsnprintf(pl->label_txt, sizeof(pl->label_txt), fmt, args);
	va_end(args);

	SC_PlotUpdateLabel(pl);
}

void
SC_PlotSetScale(SC_Plot *pl, SC_Real xScale, SC_Real yScale)
{
	if (xScale > 0.0) { pl->xScale = xScale; }
	if (yScale > 0.0) { pl->yScale = yScale; }
}

void
SC_PlotSetXoffs(SC_Plot *pl, int xOffs)
{
	pl->xOffs = xOffs;
}

void
SC_PlotSetYoffs(SC_Plot *pl, int yOffs)
{
	pl->yOffs = yOffs;
}

void
SC_PlotterSetDefaultFont(SC_Plotter *ptr, const char *face, int size)
{
	ptr->font = AG_FetchFont(face, size, 0);
}

void
SC_PlotterSetDefaultColor(SC_Plotter *ptr, int i, Uint8 r, Uint8 g, Uint8 b)
{
	ptr->colors[i] = SDL_MapRGB(agSurfaceFmt, r, g, b);
}

void
SC_PlotterSetDefaultScale(SC_Plotter *ptr, SC_Real xScale, SC_Real yScale)
{
	ptr->xScale = xScale;
	ptr->yScale = yScale;
}

const AG_WidgetClass scPlotterClass = {
	{
		"AG_Widget:SC_Plotter",
		sizeof(SC_Plotter),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
