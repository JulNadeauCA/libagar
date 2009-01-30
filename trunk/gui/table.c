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

#include <core/core.h>
#include <core/config.h>

#include "table.h"
#include "primitive.h"
#include "cursors.h"

#include <string.h>
#include <stdarg.h>

#define COLUMN_RESIZE_RANGE	10	/* Range in pixels for resize ctrls */
#define LAST_VISIBLE(t) ((t)->m - (t)->mVis + 1)

AG_Table *
AG_TableNew(void *parent, Uint flags)
{
	AG_Table *t;

	t = Malloc(sizeof(AG_Table));
	AG_ObjectInit(t, &agTableClass);
	t->flags |= flags;

	if (flags & AG_TABLE_HFILL) { AG_ExpandHoriz(t); }
	if (flags & AG_TABLE_VFILL) { AG_ExpandVert(t); }

	AG_ObjectAttach(parent, t);
	return (t);
}

AG_Table *
AG_TableNewPolled(void *parent, Uint flags, void (*fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Table *t;

	t = AG_TableNew(parent, flags);
	AG_ObjectLock(t);
	t->flags |= AG_TABLE_POLL;
	t->poll_ev = AG_SetEvent(t, "table-poll", fn, NULL);
	AG_EVENT_GET_ARGS(t->poll_ev, fmt);
	AG_ObjectUnlock(t);
	return (t);
}

void
AG_TableSizeHint(AG_Table *t, int w, int nrows)
{
	AG_ObjectLock(t);
	if (w != -1) { t->wHint = w; }
	if (nrows != -1) { t->hHint = nrows*agTextFontHeight; }
	AG_ObjectUnlock(t);
}

void
AG_TableSetSelectionMode(AG_Table *t, enum ag_table_selmode mode)
{
	AG_ObjectLock(t);
	t->selMode = mode;
	AG_ObjectUnlock(t);
}

void
AG_TableSetSelectionColor(AG_Table *t, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_ObjectLock(t);
	t->selColor[0] = r;
	t->selColor[1] = g;
	t->selColor[2] = b;
	t->selColor[3] = a;
	AG_ObjectUnlock(t);
}

/* Change the set of recognized field separators (defaults to ":") */
void
AG_TableSetSeparator(AG_Table *t, const char *sep)
{
	AG_ObjectLock(t);
	t->sep = sep;
	AG_ObjectUnlock(t);
}

/* Set column height in pixels */
void
AG_TableSetColHeight(AG_Table *t, int h)
{
	AG_ObjectLock(t);
	t->hCol = h;
	AG_WindowUpdate(AG_ParentWindow(t));
	AG_ObjectUnlock(t);
}

/* Set row height in pixels */
void
AG_TableSetRowHeight(AG_Table *t, int h)
{
	AG_ObjectLock(t);
	t->hRow = h;
	AG_WindowUpdate(AG_ParentWindow(t));
	AG_ObjectUnlock(t);
}

/* Specify the minimum allowed column width in pixels. */
void
AG_TableSetColMin(AG_Table *t, int w)
{
	int n;

	AG_ObjectLock(t);
	t->wColMin = w;
	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		if (tc->w < t->wColMin)
			tc->w = t->wColMin;
	}
	AG_ObjectUnlock(t);
}

/*
 * Specify a default column width in pixels to use in size requisition,
 * for "%" and FILL columns.
 */
void
AG_TableSetDefaultColWidth(AG_Table *t, int w)
{
	AG_ObjectLock(t);
	t->wColDefault = w;
	AG_ObjectUnlock(t);
}

/*
 * If some column sizes were specified using percents, or use the FILL
 * option, expand them to their effective pixel sizes. This is only done
 * once, during initial sizing.
 */
static void
SizeColumns(AG_Table *t)
{
	AG_TableCol *tc, *tcFill = NULL;
	Uint n;

	t->wTot = 0;
	for (n = 0; n < t->n; n++) {
		tc = &t->cols[n];
		if (tc->wPct != -1) {
			tc->w = tc->wPct*t->r.w/100;
			t->wTot += tc->w;
			tc->wPct = -1;
			continue;
		}
		if (tc->flags & AG_TABLE_COL_FILL) {
			tcFill = tc;
			tcFill->flags &= ~(AG_TABLE_COL_FILL);
		} else {
			t->wTot += tc->w;
		}
	}
	if (tcFill != NULL) {
		tcFill->w = t->r.w;
		for (n = 0; n < t->n; n++) {
			tc = &t->cols[n];
			if (tc != tcFill)
				tcFill->w -= tc->w;
		}
		if (tcFill->w < t->wColMin) {
			tcFill->w = t->wColMin;
		}
		t->wTot += tcFill->w;
	}
}

/*
 * Compute scrollbar values from the current geometry and visible
 * row count. Table must be locked.
 */
static void
UpdateScrollbars(AG_Table *t)
{
	AG_Variable *bValue, *bMax;
	int *value, *max;

	t->mVis = t->r.h/t->hRow;
	if (t->r.h % t->hRow) { t->mVis++; }

	if (t->vbar != NULL) {
		bMax = AG_GetVariable(t->vbar, "max", &max);
		bValue = AG_GetVariable(t->vbar, "value", &value);
		if ((*max = LAST_VISIBLE(t)) < 0) {
			*max = 0;
		}
		if (*value > *max) {
			*value = *max;
		} else if (*value < 0) {
			*value = 0;
		}
		if (t->m > 0 && t->mVis > 0 && (t->mVis-1) < t->m) {
			AG_ScrollbarSetBarSize(t->vbar,
			    (t->mVis-1)*(HEIGHT(t->vbar) - t->vbar->wButton*2) /
			    t->m);
		} else {
			AG_ScrollbarSetBarSize(t->vbar, -1);
		}
		AG_UnlockVariable(bValue);
		AG_UnlockVariable(bMax);
	}
	if (t->hbar != NULL) {
		SizeColumns(t);
		if (t->wTot == 0 || t->wTot <= t->r.w || t->r.w == 0) {
			AG_ScrollbarSetBarSize(t->hbar, -1);
		} else {
			AG_ScrollbarSetBarSize(t->hbar,
			    t->r.w*(WIDTH(t->hbar) - t->hbar->wButton*2) /
			    t->wTot);
		}
		bValue = AG_GetVariable(t->hbar, "value", &value);
		if ((t->wTot - t->r.w - *value) < 0) {
			*value = MAX(0, t->wTot - t->r.w);
		}
		AG_UnlockVariable(bValue);
	}
}

/* Set the effective position of all embedded widgets in the table. */
static void
UpdateEmbeddedWidgets(AG_Table *t)
{
	AG_SizeAlloc wa;
	AG_Widget *wt;
	AG_Rect rd;
	AG_TableCol *col;
	AG_TableCell *c;
	Uint m, n;
	int update = 0;

	rd.h = t->hRow;

	for (n = 0, rd.x = -t->xOffs;
	     n < t->n;
	     n++) {
		col = &t->cols[n];
		rd.w = col->w;
		rd.y = t->hCol - t->mOffs*t->hRow;
		for (m = 0;
		     m < t->m;
		     m++) {
			c = &t->cells[m][n];
			if (c->type != AG_CELL_WIDGET) {
				continue;
			}
			wt = c->data.p;

			if (wt->x != rd.x || wt->y != rd.y ||
			    wt->w != rd.w || wt->h != rd.h) {
				wa.x = rd.x;
				wa.y = rd.y;
				wa.w = rd.w;
				wa.h = rd.h;
				AG_WidgetSizeAlloc(wt, &wa);
				update++;
			}

			/*
			 * Adjust sensitivity rectangle if widget is
			 * partially visible.
			 */
			wt->rSens.w = (rd.x+rd.w > t->r.w) ?
			    (t->r.w - rd.x - 4) : rd.w;
			wt->rSens.x2 = wt->rSens.x1 + wt->rSens.w;
	
			wt->rSens.h = (rd.y+rd.h > t->r.h+t->hCol) ?
			    (t->r.h + t->hCol - rd.y - 4) : rd.h;
			wt->rSens.y2 = wt->rSens.y1 + wt->rSens.h;

			rd.y += t->hRow;
		}
		rd.x += col->w;
	}

	/* Apply any changes to widget size allocations. */
	if (update) {
		AG_WidgetUpdateCoords(t,
		    WIDGET(t)->rView.x1,
		    WIDGET(t)->rView.y1);
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Table *t = obj;
	AG_SizeReq rBar;
	int n;

	r->h = t->hHint;
	if (t->wHint != -1) {
		r->w = t->wHint;
	} else {
		for (n = 0; n < t->n; n++) {
			AG_TableCol *tc = &t->cols[n];
		
			if (tc->flags & AG_TABLE_COL_FILL ||
			    tc->wPct != -1) {
				r->w += t->wColDefault;
			} else {
				r->w += tc->w;
			}
		}
	}
	
	if (t->vbar != NULL) {
		AG_WidgetSizeReq(t->vbar, &rBar);
		r->w += rBar.w;
	}
	if (t->hbar != NULL) {
		AG_WidgetSizeReq(t->hbar, &rBar);
		r->h += rBar.h;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Table *t = obj;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	
	t->r.w = a->w;
	t->r.h = a->h - t->hCol;
	t->r.y = t->hCol;

	if (t->r.h <= 0)
		return (-1);

	if (t->vbar) {
		AG_WidgetSizeReq(t->vbar, &rBar);
		if (rBar.w > a->w/2) { rBar.w = a->w/2; }
		aBar.x = a->w - rBar.w;
		aBar.y = 0;
		aBar.w = rBar.w;
		aBar.h = a->h - rBar.w;
		AG_WidgetSizeAlloc(t->vbar, &aBar);
		t->r.w -= WIDTH(t->vbar);
	}
	if (t->hbar) {
		AG_WidgetSizeReq(t->hbar, &rBar);
		if (rBar.h > a->h/2) { rBar.h = a->h/2; }
		aBar.x = 0;
		aBar.y = a->h - rBar.h;
		aBar.w = a->w - (t->vbar ? WIDTH(t->vbar) : 0);
		aBar.h = rBar.h;
		AG_WidgetSizeAlloc(t->hbar, &aBar);
		t->r.h -= HEIGHT(t->hbar);
	}

	SizeColumns(t);
	UpdateScrollbars(t);
	return (0);
}

static __inline__ void
PrintCell(AG_Table *t, AG_TableCell *c, char *buf, size_t bufsz)
{
	switch (c->type) {
	case AG_CELL_INT:
	case AG_CELL_UINT:
		Snprintf(buf, bufsz, c->fmt, c->data.i);
		break;
	case AG_CELL_LONG:
	case AG_CELL_ULONG:
		Snprintf(buf, bufsz, c->fmt, c->data.l);
		break;
	case AG_CELL_PINT:
		Snprintf(buf, bufsz, c->fmt, *(int *)c->data.p);
		break;
	case AG_CELL_PUINT:
		Snprintf(buf, bufsz, c->fmt, *(Uint *)c->data.p);
		break;
	case AG_CELL_PLONG:
		Snprintf(buf, bufsz, c->fmt, *(long *)c->data.p);
		break;
	case AG_CELL_PULONG:
		Snprintf(buf, bufsz, c->fmt, *(Ulong *)c->data.p);
		break;
	case AG_CELL_FLOAT:
		Snprintf(buf, bufsz, c->fmt, (float)c->data.f);
		break;
	case AG_CELL_PFLOAT:
		Snprintf(buf, bufsz, c->fmt, *(float *)c->data.p);
		break;
	case AG_CELL_DOUBLE:
		Snprintf(buf, bufsz, c->fmt, c->data.f);
		break;
	case AG_CELL_PDOUBLE:
		Snprintf(buf, bufsz, c->fmt, *(double *)c->data.p);
		break;
#ifdef HAVE_64BIT
	case AG_CELL_INT64:
	case AG_CELL_UINT64:
		Snprintf(buf, bufsz, c->fmt, c->data.f);
		break;
	case AG_CELL_PUINT64:
		Snprintf(buf, bufsz, c->fmt, *(Uint64 *)c->data.p);
		break;
	case AG_CELL_PINT64:
		Snprintf(buf, bufsz, c->fmt, *(Sint64 *)c->data.p);
		break;
#endif
	case AG_CELL_PUINT32:
		Snprintf(buf, bufsz, c->fmt, *(Uint32 *)c->data.p);
		break;
	case AG_CELL_PSINT32:
		Snprintf(buf, bufsz, c->fmt, *(Sint32 *)c->data.p);
		break;
	case AG_CELL_PUINT16:
		Snprintf(buf, bufsz, c->fmt, *(Uint16 *)c->data.p);
		break;
	case AG_CELL_PSINT16:
		Snprintf(buf, bufsz, c->fmt, *(Sint16 *)c->data.p);
		break;
	case AG_CELL_PUINT8:
		Snprintf(buf, bufsz, c->fmt, *(Uint8 *)c->data.p);
		break;
	case AG_CELL_PSINT8:
		Snprintf(buf, bufsz, c->fmt, *(Sint8 *)c->data.p);
		break;
	case AG_CELL_STRING:
		Strlcpy(buf, c->data.s, bufsz);
		break;
	case AG_CELL_PSTRING:
		Strlcpy(buf, (char *)c->data.p, bufsz);
		break;
	case AG_CELL_FN_TXT:
		c->fnTxt(c->data.p, buf, bufsz);
		break;
	case AG_CELL_FN_SU:
		Strlcpy(buf, "<image>", bufsz);
		break;
	case AG_CELL_POINTER:
		Snprintf(buf, bufsz, c->fmt, c->data.p);
		break;
	case AG_CELL_NULL:
		if (c->fmt[0] == '\0') {
			Strlcpy(buf, "<null>", bufsz);
		} else {
			Strlcpy(buf, c->fmt, bufsz);
		}
		break;
	case AG_CELL_WIDGET:
		Strlcpy(buf, "<widget>", bufsz);
		break;
	}
}

static __inline__ void
DrawCell(AG_Table *t, AG_TableCell *c, AG_Rect *rd)
{
	char txt[AG_TABLE_TXT_MAX];

	if (c->surface >= 0) {
		if (t->flags & AG_TABLE_REDRAW_CELLS) {
			AG_WidgetUnmapSurface(t, c->surface);
		} else {
			goto blit;
		}
	}

	switch (c->type) {
	case AG_CELL_STRING:					/* Avoid copy */
		AG_TextColor(TEXT_COLOR);
		c->surface = AG_WidgetMapSurface(t, AG_TextRender(c->data.s));
		goto blit;
	case AG_CELL_PSTRING:					/* Avoid copy */
		AG_TextColor(TEXT_COLOR);
		c->surface = AG_WidgetMapSurface(t, AG_TextRender((char *)
		                                                  c->data.p));
		goto blit;
	case AG_CELL_FN_SU:
		c->surface = AG_WidgetMapSurface(t,
		    c->fnSu(c->data.p, rd->x, rd->y));
		goto blit;
	case AG_CELL_WIDGET:
		if (WIDGET_OPS(c->data.p)->draw != NULL) {
			AG_WidgetDraw(c->data.p);
		}
		c->surface = -1;
		return;
	case AG_CELL_NULL:
		if (c->fmt[0] != '\0') {
			AG_TextColor(TEXT_COLOR);
			c->surface = AG_WidgetMapSurface(t,
			    AG_TextRender(c->fmt));
			goto blit;
		} else {
			return;
		}
		break;
	default:
		PrintCell(t, c, txt, sizeof(txt));
		break;
	}
	AG_TextColor(TEXT_COLOR);
	c->surface = AG_WidgetMapSurface(t, AG_TextRender(txt));
blit:
	AG_WidgetBlitSurface(t, c->surface,
	    rd->x,
	    rd->y + (t->hRow>>1) - (WSURFACE(t,c->surface)->h>>1));
}

static void
ScrollToSelection(AG_Table *t)
{
	Uint m;
	int offs;

	if (t->n < 1) {
		return;
	}
	for (m = 0; m < t->m; m++) {
		if (!t->cells[m][0].selected) {
			continue;
		}
		if (t->mOffs > m) {
			AG_SetInt(t->vbar, "value", m);
		} else {
			offs = m - t->mVis + 2;
			if (offs < 0) { offs = 0; }
			AG_SetInt(t->vbar, "value", offs);
		}
		return;
	}
}

#if 0
static void
SelectionToScroll(AG_Table *t)
{
	Uint m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++) {
			if (!t->cells[m][n].selected) {
				continue;
			}
			AG_TableDeselectRow(t, m);
			if (m < t->mOffs) {
				AG_TableSelectRow(t, t->mOffs);
			} else {
				AG_TableSelectRow(t, t->mOffs + t->mVis - 2);
			}
			return;
		}
	}
}
#endif

static int
SelectionVisible(AG_Table *t)
{
	int n, m, mOffs;
	int x, y;
	
	mOffs = AG_GetInt(t->vbar, "value");
	if (t->poll_ev != NULL) {
		t->poll_ev->handler(t->poll_ev);
	}
	for (n = 0, x = -t->xOffs;
	     n < t->n && x < t->r.w;
	     n++) {
		AG_TableCol *col = &t->cols[n];
		int cw;
	
		if (col->w <= 0) {
			continue;
		}
		cw = ((x + col->w) < t->r.w) ? col->w: t->r.w - x;
		for (m = mOffs, y = t->hCol;
		     m < MIN(t->m, mOffs+t->mVis) && (y < t->hCol+t->r.h);
		     m++) {
			if (t->cells[m][n].selected &&
			    m < (mOffs + t->mVis - 1)) {
				return (1);
			}
			y += t->hRow;
		}
		x += col->w;
	}
	return (0);
}

static void
Draw(void *obj)
{
	AG_Table *t = obj;
	AG_Rect rCol, rCell;
	Uint n, m;

	STYLE(t)->TableBackground(t, t->r);
	
	AG_WidgetDraw(t->vbar);
	AG_WidgetDraw(t->hbar);
	
	t->mOffs = AG_GetInt(t->vbar, "value");
	if (t->poll_ev != NULL) {
		t->poll_ev->handler(t->poll_ev);
	}
	if (t->flags & AG_TABLE_WIDGETS)
		UpdateEmbeddedWidgets(t);

	rCol.y = 0;
	rCol.h = t->hCol + t->r.h - 2;
	rCell.h = t->hRow;

	for (n = 0, rCell.x = -t->xOffs;
	     n < t->n && rCell.x < t->r.w;
	     n++) {
		AG_TableCol *col = &t->cols[n];
	
		if (col->w <= 0) {
			continue;
		}
		rCell.w = col->w;
		rCol.w = ((rCell.x + col->w) < t->r.w) ?
		         col->w : (t->r.w - rCell.x);
		rCol.x = rCell.x;

		/* Column header and separator */
		if (rCol.x > 0 && rCol.x < t->r.w) {
			AG_DrawLineV(t,
			    rCol.x - 1,
			    t->hCol - 1,
			    rCol.h,
			    AG_COLOR(TABLE_LINE_COLOR));
		}
		
		AG_PushClipRect(t, rCol);

		STYLE(t)->TableColumnHeaderBackground(t, n,
		    AG_RECT(rCol.x, 0, rCol.w, t->hCol),
		    col->selected);

		/* Column header label */
		if (col->surface != -1) {
			AG_WidgetBlitSurface(t, col->surface,
			    rCell.x + col->w/2 - WSURFACE(t,col->surface)->w/2,
			    t->hCol/2 - WSURFACE(t,col->surface)->h/2);
		}

		/* Rows of this column */
		for (m = t->mOffs, rCell.y = t->hCol;
		     m < t->m && (rCell.y < rCol.h);
		     m++) {
			AG_TableCell *c = &t->cells[m][n];

			AG_DrawLineH(t, 0, t->r.w, rCell.y,
			    AG_COLOR(TABLE_LINE_COLOR));

			DrawCell(t, c, &rCell);
			if (c->selected) {
				AG_DrawRectBlended(t, rCell, t->selColor,
				    AG_ALPHA_SRC);
			}
			rCell.y += t->hRow;
		}

		AG_DrawLineH(t, 0, t->r.w, rCell.y,
		    AG_COLOR(TABLE_LINE_COLOR));

		/* Indicate column selection. */
		if ((t->flags & AG_TABLE_HIGHLIGHT_COLS) && col->selected) {
			STYLE(t)->TableSelectedColumnBackground(t, n, rCol);
		}
		
		AG_PopClipRect();
		rCell.x += col->w;
	}
	if (rCell.x > 0 &&
	    rCell.x < t->r.w) {
		AG_DrawLineV(t,
		    rCell.x - 1,
		    t->hCol - 1,
		    rCol.h,
		    AG_COLOR(TABLE_LINE_COLOR));
	}
	t->flags &= ~(AG_TABLE_REDRAW_CELLS);
}

AG_MenuItem *
AG_TableSetPopup(AG_Table *t, int m, int n)
{
	AG_TablePopup *tp;
	AG_MenuItem *rv;

	AG_ObjectLock(t);
	SLIST_FOREACH(tp, &t->popups, popups) {
		if (tp->m == m && tp->n == n) {
			AG_MenuItemFree(tp->item);
			AG_ObjectUnlock(t);
			return (tp->item);
		}
	}
	tp = Malloc(sizeof(AG_TablePopup));
	tp->m = m;
	tp->n = n;
	tp->panel = NULL;
	tp->menu = AG_MenuNew(NULL, 0);
	tp->item = tp->menu->root;			/* XXX redundant */
	SLIST_INSERT_HEAD(&t->popups, tp, popups);
	rv = tp->item;
	AG_ObjectUnlock(t);

	return (rv);
}

void
AG_TableSetRowDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickRowEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickRowEv, fmt);
	AG_ObjectUnlock(t);
}

void
AG_TableSetColDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickColEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickColEv, fmt);
	AG_ObjectUnlock(t);
}

void
AG_TableSetCellDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickCellEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickCellEv, fmt);
	AG_ObjectUnlock(t);
}

void
AG_TableRedrawCells(AG_Table *t)
{
	AG_ObjectLock(t);
	t->flags |= AG_TABLE_REDRAW_CELLS;
	AG_ObjectUnlock(t);
}

/* Table must be locked. */
void
AG_TableFreeCell(AG_Table *t, AG_TableCell *c)
{
	if (c->widget != NULL) {
		AG_ObjectDetach(c->widget);
		AG_ObjectDestroy(c->widget);
	}
	if (c->surface >= 0)
		AG_WidgetUnmapSurface(t, c->surface);
}

/* Table must be locked. */
int
AG_TablePoolAdd(AG_Table *t, Uint m, Uint n)
{
	AG_TableCol *tc = &t->cols[n];
	
	tc->pool = Realloc(tc->pool, (tc->mpool+1)*sizeof(AG_TableCell));
	memcpy(&tc->pool[tc->mpool], &t->cells[m][n], sizeof(AG_TableCell));
	return (tc->mpool++);
}

/* Table must be locked. */
void
AG_TablePoolFree(AG_Table *t, Uint n)
{
	AG_TableCol *tc = &t->cols[n];
	Uint m;

	for (m = 0; m < tc->mpool; m++) {
		AG_TableFreeCell(t, &tc->pool[m]);
	}
	Free(tc->pool);
	tc->pool = NULL;
	tc->mpool = 0;
}

/*
 * Clear the items on the table and save the selection state. The function
 * returns with the table locked.
 */
void
AG_TableBegin(AG_Table *t)
{
	Uint m, n;

	AG_ObjectLock(t);
	/* Copy the existing cells to the column pools and free the table. */
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++) {
			AG_TablePoolAdd(t, m, n);
		}
		Free(t->cells[m]);
	}
	Free(t->cells);
	t->cells = NULL;
	t->m = 0;
	t->flags &= ~(AG_TABLE_WIDGETS);
	AG_SetInt(t->vbar, "max", 0);
}

int
AG_TableCompareCells(const AG_TableCell *c1, const AG_TableCell *c2)
{
	if (c1->type != c2->type || strcmp(c1->fmt, c2->fmt) != 0) {
		return (1);
	}
	switch (c1->type) {
	case AG_CELL_STRING:
		return (strcmp(c1->data.s, c2->data.s));
	case AG_CELL_PSTRING:
		return (strcmp((char *)c1->data.p, (char *)c2->data.p));
	case AG_CELL_INT:
	case AG_CELL_UINT:
		return (c1->data.i - c2->data.i);
	case AG_CELL_LONG:
	case AG_CELL_ULONG:
		return (c1->data.l - c2->data.l);
	case AG_CELL_FLOAT:
	case AG_CELL_DOUBLE:
		return (c1->data.f != c2->data.f);
#ifdef HAVE_64BIT
	case AG_CELL_INT64:
	case AG_CELL_UINT64:
		return (c1->data.u64 - c2->data.u64);
	case AG_CELL_PUINT64:
	case AG_CELL_PINT64:
		return (*(Uint64 *)c1->data.p - *(Uint64 *)c2->data.p);
#endif
	case AG_CELL_PULONG:
	case AG_CELL_PLONG:
		return (*(long *)c1->data.p - *(long *)c2->data.p);
	case AG_CELL_PUINT:
	case AG_CELL_PINT:
		return (*(int *)c1->data.p - *(int *)c2->data.p);
	case AG_CELL_PUINT8:
	case AG_CELL_PSINT8:
		return (*(Uint8 *)c1->data.p - *(Uint8 *)c2->data.p);
	case AG_CELL_PUINT16:
	case AG_CELL_PSINT16:
		return (*(Uint16 *)c1->data.p - *(Uint16 *)c2->data.p);
	case AG_CELL_PUINT32:
	case AG_CELL_PSINT32:
		return (*(Uint32 *)c1->data.p - *(Uint32 *)c2->data.p);
	case AG_CELL_PFLOAT:
		return (*(float *)c1->data.p - *(float *)c2->data.p);
	case AG_CELL_PDOUBLE:
		return (*(double *)c1->data.p - *(double *)c2->data.p);
	case AG_CELL_POINTER:
		return (c1->data.p != c2->data.p);
	case AG_CELL_FN_SU:
		return ((c1->data.p != c2->data.p) || (c1->fnSu != c2->fnSu));
	case AG_CELL_FN_TXT:
		{
			char b1[AG_TABLE_TXT_MAX];
			char b2[AG_TABLE_TXT_MAX];

			c1->fnTxt(c1->data.p, b1, sizeof(b1));
			c2->fnTxt(c2->data.p, b2, sizeof(b2));
			return (strcmp(b1, b2));
		}
	case AG_CELL_WIDGET:
		/* XXX TODO hooks */
		return (1);
	case AG_CELL_NULL:
		return (0);
	}
	return (1);
}

/*
 * Restore the selection state and recover the surfaces of matching
 * items in the column pool. Unlock the table.
 */
void
AG_TableEnd(AG_Table *t)
{
	Uint n, m;

	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];

		for (m = 0; m < tc->mpool; m++) {
			AG_TableCell *cPool = &tc->pool[m];

			if (m < t->m) {
				AG_TableCell *cDst = &t->cells[m][n];

				if (AG_TableCompareCells(cDst, cPool) == 0) {
					cDst->surface = cPool->surface;
					cDst->selected = cPool->selected;
					cPool->surface = -1;
				}
			}
			AG_TableFreeCell(t, cPool);
		}
		Free(tc->pool);
		tc->pool = NULL;
		tc->mpool = 0;
	}
	if (t->r.h > 0 && t->r.w > 0) {
		UpdateScrollbars(t);
	}
	AG_ObjectUnlock(t);
}

/* Return true if multiple item selection is enabled. */
static __inline__ int
SelectingMultiple(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTITOGGLE) ||
	        ((t->flags & AG_TABLE_MULTI) && SDL_GetModState() & KMOD_CTRL));
}

/* Return true if a range of items are being selected. */
static __inline__ int
SelectingRange(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTI) &&
	        (SDL_GetModState() & KMOD_SHIFT));
}

/* Display the popup menu. */
static void
ShowPopup(AG_TablePopup *tp)
{
	int x, y;

	AG_MouseGetState(&x, &y);
	if (tp->panel != NULL) {
		AG_MenuCollapse(tp->menu, tp->item);
		tp->panel = NULL;
	}
	tp->menu->itemSel = tp->item;
	tp->panel = AG_MenuExpand(tp->menu, tp->item, x+4, y+4);
}

/* Right click on a column header; display the column's popup menu. */
static void
ColumnRightClick(AG_Table *t, int px)
{
	Uint n;
	int cx;
	int x = px - (COLUMN_RESIZE_RANGE/2);

	for (n = 0, cx = -t->xOffs;
	     n < t->n;
	     n++) {
		AG_TableCol *tc = &t->cols[n];
		int x2 = cx+tc->w;
		AG_TablePopup *tp;

		if (x > cx && x < x2) {
			SLIST_FOREACH(tp, &t->popups, popups) {
				if (tp->m == -1 && tp->n == n) {
					ShowPopup(tp);
					return;
				}
			}
		}
		cx += tc->w;
	}
}

/* Left click on a column header; resize or set the sorting mode. */
static void
ColumnLeftClick(AG_Table *t, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2), x1;
	int multi = SelectingMultiple(t);
	Uint n;

	for (n = 0, x1 = -t->xOffs;
	     n < t->n;
	     n++) {
		AG_TableCol *tc = &t->cols[n];
		int x2 = x1+tc->w;

		if (x > x1 && x < x2) {
			if ((x2 - x) < COLUMN_RESIZE_RANGE) {
				if (t->nResizing == -1) {
					t->nResizing = n;
				}
			} else {
				if (multi) {
					tc->selected = !tc->selected;
				} else {
					tc->selected = 1;
				}
				if (t->dblClickedCol != -1 &&
				    t->dblClickedCol == n) {
					AG_CancelEvent(t,
					    "dblclick-col-expire");
					if (t->dblClickColEv != NULL) {
						AG_PostEvent(NULL, t,
						    t->dblClickColEv->name,
						    "%i", n);
					}
					AG_PostEvent(NULL, t,
					    "table-dblclick-col", "%i", n);
					t->dblClickedCol = -1;
				} else {
					t->dblClickedCol = n;
					AG_SchedEvent(NULL, t,
					    agMouseDblclickDelay,
					    "dblclick-col-expire", NULL);
				}
				goto cont;
			}
		}
		if (!multi)
			tc->selected = 0;
cont:
		x1 += tc->w;
	}
}

/* Process left click on a cell. */
static void
CellLeftClick(AG_Table *t, int mc, int x)
{
	AG_TableCell *c;
	AG_TableCol *tc;
	Uint m, n, i, j, nc;
	
	for (nc=0; nc < t->n; nc++) {
		tc = &t->cols[nc];
		if ((x + t->xOffs) > tc->x &&
		    (x + t->xOffs) < tc->x+tc->w)
			break;
	}
	if (nc == t->n) { nc = t->n-1; }

	switch (t->selMode) {
	case AG_TABLE_SEL_ROWS:
		if (SelectingRange(t)) {
			for (m=0; m < t->m; m++) {
				if (AG_TableRowSelected(t,m))
					break;
			}
			if (m == t->m) {
				break;
			}
			if (m < mc) {
				for (i=m; i <= mc; i++)
					AG_TableSelectRow(t, i);
			} else if (m > mc) {
				for (i=mc; i <= m; i++)
					AG_TableSelectRow(t, i);
			} else {
				AG_TableSelectRow(t, mc);
			}
		} else if (SelectingMultiple(t)) {
			for (n=0; n < t->n; n++) {
				c = &t->cells[mc][n];
				c->selected = !c->selected;
			}
		} else {
			for (m=0; m < t->m; m++) {
				for (n=0; n < t->n; n++) {
					c = &t->cells[m][n];
					c->selected = ((int)m == mc);
				}
			}
			if (t->dblClickedRow != -1 &&
			    t->dblClickedRow == mc) {
				AG_CancelEvent(t, "dblclick-row-expire");
				if (t->dblClickRowEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickRowEv->name,
					    "%i", mc);
				}
				AG_PostEvent(NULL, t,
				    "table-dblclick-row", "%i", mc);
				t->dblClickedRow = -1;
			} else {
				t->dblClickedRow = mc;
				AG_SchedEvent(NULL, t, agMouseDblclickDelay,
				    "dblclick-row-expire", NULL);
			}
		}
		break;
	case AG_TABLE_SEL_CELLS:
		if (SelectingRange(t)) {
			for (m=0, n=0; m < t->m; m++) {
				for (n=0; n < t->n; n++) {
					if (AG_TableCellSelected(t,m,n))
						break;
				}
				if (n == t->n)
					break;
			}
			if (m == t->m) {
				break;
			}
			if (m < mc && n < nc) {
				for (i=n; i <= nc; i++)
					for (j = m; j <= mc; j++)
						AG_TableSelectCell(t, j,i);
			} else if (m > mc && n > nc) {
				for (i=nc; i <= n; i++)
					for (j = mc; j <= m; j++)
						AG_TableSelectCell(t, j,i);
			} else {
				AG_TableSelectCell(t, mc, nc);
			}
		} else if (SelectingMultiple(t)) {
			c = &t->cells[mc][nc];
			c->selected = !c->selected;
		} else {
			for (m=0; m < t->m; m++) {
				for (n=0; n < t->n; n++) {
					c = &t->cells[m][n];
					c->selected = ((int)m == mc) &&
					              ((int)n == nc);
				}
			}
			if (t->dblClickedCell != -1 &&
			    t->dblClickedCell == mc) {
				AG_CancelEvent(t, "dblclick-cell-expire");
				if (t->dblClickCellEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickCellEv->name,
					    "%i", mc);
				}
				AG_PostEvent(NULL, t,
				    "table-dblclick-cell", "%i,%i", mc, nc);
				t->dblClickedCell = -1;
			} else {
				t->dblClickedRow = mc;
				AG_SchedEvent(NULL, t, agMouseDblclickDelay,
				    "dblclick-cell-expire", NULL);
			}
		}
		break;
	case AG_TABLE_SEL_COLS:
		if (SelectingRange(t)) {
			for (n=0; n < t->n; n++) {
				if (AG_TableColSelected(t,n))
					break;
			}
			if (n == t->n) {
				break;
			}
			if (n < nc) {
				for (i=n; i <= nc; i++)
					AG_TableSelectCol(t, i);
			} else if (n > nc) {
				for (i=nc; i <= n; i++)
					AG_TableSelectCol(t, i);
			} else {
				AG_TableSelectCol(t, nc);
			}
		} else if (SelectingMultiple(t)) {
			for (n=0; n < t->n; n++) {
				tc = &t->cols[n];
				tc->selected = !tc->selected;
			}
		} else {
			for (n=0; n < t->n; n++) {
				tc = &t->cols[n];
				tc->selected = ((int)n == nc);
			}
			if (t->dblClickedCol != -1 &&
			    t->dblClickedCol == nc) {
				AG_CancelEvent(t, "dblclick-col-expire");
				if (t->dblClickColEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickColEv->name,
					    "%i", nc);
				}
				AG_PostEvent(NULL, t,
				    "table-dblclick-col", "%i", nc);
				t->dblClickedCol = -1;
			} else {
				t->dblClickedCol = nc;
				AG_SchedEvent(NULL, t, agMouseDblclickDelay,
				    "dblclick-col-expire", NULL);
			}
		}
		break;
	}
}

/* Right click on cell; show the cell's popup menu. */
static void
CellRightClick(AG_Table *t, int m, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2), cx;
	Uint n;

	for (n = 0, cx = -t->xOffs;
	     n < t->n;
	     n++) {
		AG_TableCol *tc = &t->cols[n];
		AG_TablePopup *tp;

		if (x < cx ||
		    x > cx+tc->w) {
			continue;
		}
		SLIST_FOREACH(tp, &t->popups, popups) {
			if ((tp->m == m || tp->m == -1) &&
			    (tp->n == n || tp->n == -1)) {
				ShowPopup(tp);
				return;
			}
		}
		cx += tc->w;
	}
}

/* Cursor is over column header? */
static __inline__ int
OverColumnHeader(AG_Table *t, int y)
{
	return (y <= t->hCol);
}

/* Cursor is over column resize control? */
static __inline__ int
OverColumnResizeControl(AG_Table *t, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2);
	Uint n;
	int cx;

	if (px < 0 || px > t->r.w) {
		return (0);
	}
	for (n = 0, cx = -t->xOffs;
	     n < t->n;
	     n++) {
		int x2 = cx + t->cols[n].w;
		if (x > cx && x < x2) {
			if ((x2 - x) < COLUMN_RESIZE_RANGE)
				return (1);
		}
		cx += t->cols[n].w;
	}
	return (0);
}

/* Return the row at the given y-coordinate. */
static __inline__ int
RowAtY(AG_Table *t, int y)
{
	int m;
	
	m = AG_GetInt(t->vbar, "value");
	if (y > t->hCol) {
		m += (y - t->hCol)/t->hRow;
	}
	if (m < 0) { m = 0; }
	if (m >= (int)t->m) { m = (t->m > 0) ? ((int)t->m - 1) : 0; }
	return (m);
}

static void
DecrementSelection(AG_Table *t, int inc)
{
	int m;

	if (t->m < 1) {
		return;
	}
	for (m = 0; m < t->m; m++) {
		if (AG_TableRowSelected(t,m)) {
			m -= inc;
			if (m < 0) { m = 0; }
			break;
		}
	}
	if (m < t->m) {
		AG_TableDeselectAllRows(t);
		AG_TableSelectRow(t, m);
	} else {
		AG_TableSelectRow(t, 0);
	}
	if (!SelectionVisible(t))
		ScrollToSelection(t);
}

static void
IncrementSelection(AG_Table *t, int inc)
{
	int m;

	if (t->m < 1) {
		return;
	}
	for (m = t->m-1; m >= 0; m--) {
		if (AG_TableRowSelected(t,m)) {
			m += inc;
			if (m >= t->m) { m = t->m-1; }
			break;
		}
	}
	if (m >= 0) {
		AG_TableDeselectAllRows(t);
		AG_TableSelectRow(t, m);
	} else {
		AG_TableSelectRow(t, 0);
	}
	if (!SelectionVisible(t))
		ScrollToSelection(t);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	int m;
	
	switch (button) {
	case SDL_BUTTON_WHEELUP:
		{
			AG_Variable *offsb;
			int *offs;

			offsb = AG_GetVariable(t->vbar, "value", &offs);
			(*offs) -= AG_WidgetScrollDelta(&t->wheelTicks);
			if (*offs < 0) {
				*offs = 0;
			}
			AG_UnlockVariable(offsb);
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		{
			AG_Variable *offsb;
			int *offs;

			offsb = AG_GetVariable(t->vbar, "value", &offs);
			(*offs) += AG_WidgetScrollDelta(&t->wheelTicks);
			if (*offs > LAST_VISIBLE(t)) {
				*offs = LAST_VISIBLE(t);
			}
			AG_UnlockVariable(offsb);
		}
		break;
	case SDL_BUTTON_LEFT:
		if (OverColumnHeader(t, y)) {
			ColumnLeftClick(t, x);
			break;
		}
		if (t->m == 0 || t->n == 0) {
			goto out;
		}
		m = RowAtY(t, y);
		CellLeftClick(t, m, x);
		break;
	case SDL_BUTTON_RIGHT:
		if (OverColumnHeader(t, y)) {
			ColumnRightClick(t, x);
			break;
		}
		if (t->m == 0 || t->n == 0) {
			goto out;
		}
		m = RowAtY(t, y);
		CellRightClick(t, m, x);
		break;
	default:
		break;
	}
out:
	AG_WidgetFocus(t);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int button = AG_INT(1);

	switch (button) {
	case SDL_BUTTON_LEFT:
		if (t->nResizing >= 0) {
			t->nResizing = -1;
		}
		break;
	}
}

static void
KeyDown(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_UP:
		DecrementSelection(t, 1);
		AG_DelTimeout(t, &t->incTo);
		AG_ScheduleTimeout(t, &t->decTo, agKbdDelay);
		break;
	case SDLK_DOWN:
		IncrementSelection(t, 1);
		AG_DelTimeout(t, &t->decTo);
		AG_ScheduleTimeout(t, &t->incTo, agKbdDelay);
		break;
	case SDLK_PAGEUP:
		DecrementSelection(t, agPageIncrement);
		AG_DelTimeout(t, &t->incTo);
		AG_ScheduleTimeout(t, &t->decTo, agKbdDelay);
		break;
	case SDLK_PAGEDOWN:
		IncrementSelection(t, agPageIncrement);
		AG_DelTimeout(t, &t->decTo);
		AG_ScheduleTimeout(t, &t->incTo, agKbdDelay);
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int xrel = AG_INT(3);

	if (x < 0 || y < 0 || x >= WIDTH(t) || y >= HEIGHT(t))
		return;

	if (t->nResizing >= 0 && (Uint)t->nResizing < t->n) {
		AG_TableCol *tc = &t->cols[t->nResizing];

		if ((tc->w += xrel) < t->wColMin) {
			tc->w = t->wColMin;
		}
		SizeColumns(t);
		if (t->r.h > 0 && t->r.w > 0) {
			UpdateScrollbars(t);
		}
		AG_SetCursor(AG_HRESIZE_CURSOR);
	} else {
		if (OverColumnHeader(t, y) &&
		    OverColumnResizeControl(t, x))
			AG_SetCursor(AG_HRESIZE_CURSOR);
	}
}

static void
KeyUp(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_UP:
	case SDLK_PAGEUP:
		AG_DelTimeout(t, &t->decTo);
		break;
	case SDLK_DOWN:
	case SDLK_PAGEDOWN:
		AG_DelTimeout(t, &t->incTo);
		break;
	}
}

static void
LostFocus(AG_Event *event)
{
	AG_Table *t = AG_SELF();

	AG_DelTimeout(t, &t->incTo);
	AG_DelTimeout(t, &t->decTo);
	AG_CancelEvent(t, "dblclick-row-expire");
	AG_CancelEvent(t, "dblclick-col-expire");
	AG_CancelEvent(t, "dblclick-cell-expire");
	if (t->nResizing >= 0)
		t->nResizing = -1;
}

int
AG_TableRowSelected(AG_Table *t, Uint m)
{
	Uint n;

	AG_ObjectLock(t);
	if (m >= t->m) {
		goto out;
	}
	for (n = 0; n < t->n; n++) {
		if (t->cells[m][n].selected) {
			AG_ObjectUnlock(t);
			return (1);
		}
	}
out:
	AG_ObjectUnlock(t);
	return (0);
}

void
AG_TableSelectRow(AG_Table *t, Uint m)
{
	Uint n;

	AG_ObjectLock(t);
	if (m < t->m) {
		for (n = 0; n < t->n; n++) {
			t->cells[m][n].selected = 1;
		}
	}
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectRow(AG_Table *t, Uint m)
{
	Uint n;

	AG_ObjectLock(t);
	if (m < t->m) {
		for (n = 0; n < t->n; n++) {
			t->cells[m][n].selected = 0;
		}
	}
	AG_ObjectUnlock(t);
}

void
AG_TableSelectAllRows(AG_Table *t)
{
	Uint m, n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 1;
	}
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectAllRows(AG_Table *t)
{
	Uint m, n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 0;
	}
	AG_ObjectUnlock(t);
}

void
AG_TableSelectAllCols(AG_Table *t)
{
	Uint n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		t->cols[n].selected = 1;
	}
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectAllCols(AG_Table *t)
{
	Uint n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		t->cols[n].selected = 0;
	}
	AG_ObjectUnlock(t);
}

static void
ExpireRowDblClick(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	t->dblClickedRow = -1;
}

static void
ExpireColDblClick(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	t->dblClickedCol = -1;
}

static void
ExpireCellDblClick(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	t->dblClickedCell = -1;
}

int
AG_TableAddCol(AG_Table *t, const char *name, const char *size_spec,
    int (*sort_fn)(const void *, const void *))
{
	AG_TableCol *tc, *lc;
	Uint m, n;

	AG_ObjectLock(t);

	/* Initialize the column information structure. */
	t->cols = Realloc(t->cols, (t->n+1)*sizeof(AG_TableCol));
	tc = &t->cols[t->n];
	if (name != NULL) {
		Strlcpy(tc->name, name, sizeof(tc->name));
	} else {
		tc->name[0] = '\0';
	}
	tc->flags = AG_TABLE_SORT_ASCENDING;
	tc->sort_fn = sort_fn;
	tc->selected = 0;
	tc->w = 0;
	tc->wPct = -1;
	tc->pool = NULL;
	tc->mpool = 0;

	/* XXX CONTEXT */
	AG_TextColor(TEXT_COLOR);
	tc->surface = (name == NULL) ? -1 :
	    AG_WidgetMapSurface(t, AG_TextRender(name));
	if (t->n > 0) {
		lc = &t->cols[t->n - 1];
		tc->x = lc->x+lc->w;
	} else {
		tc->x = 0;
	}
	if (size_spec != NULL) {
		switch (AG_WidgetParseSizeSpec(size_spec, &tc->w)) {
		case AG_WIDGET_PERCENT:
			tc->wPct = tc->w;
			tc->w = 0;
			break;
		case AG_WIDGET_FILL:
			tc->flags |= AG_TABLE_COL_FILL;
			break;
		default:
			break;
		}
	} else {
		tc->flags |= AG_TABLE_COL_FILL;
	}

	/* Resize the row arrays. */
	for (m = 0; m < t->m; m++) {
		t->cells[m] = Realloc(t->cells[m],
		    (t->n+1)*sizeof(AG_TableCell));
		AG_TableInitCell(t, &t->cells[m][t->n]);
	}
	n = t->n++;
	AG_ObjectUnlock(t);
	return (n);
}

void
AG_TableInitCell(AG_Table *t, AG_TableCell *c)
{
	c->type = AG_CELL_NULL;
	c->fmt[0] = '\0';
	c->fnSu = NULL;
	c->fnTxt = NULL;
	c->selected = 0;
	c->surface = -1;
	c->widget = NULL;
}

int
AG_TableAddRow(AG_Table *t, const char *fmtp, ...)
{
	char fmt[64], *sp = &fmt[0];
	va_list ap;
	Uint n, rv;

	Strlcpy(fmt, fmtp, sizeof(fmt));

	AG_ObjectLock(t);

	va_start(ap, fmtp);
	t->cells = Realloc(t->cells, (t->m+1)*sizeof(AG_TableCell));
	t->cells[t->m] = Malloc(t->n*sizeof(AG_TableCell));
	for (n = 0; n < t->n; n++) {
		AG_TableCell *c = &t->cells[t->m][n];
		char *s = AG_Strsep(&sp, ":"), *sc;
		int ptr = 0, lflag = 0, ptr_long = 0;
		int infmt = 0;

		AG_TableInitCell(t, c);
		Strlcpy(c->fmt, s, sizeof(c->fmt));
		for (sc = &s[0]; *sc != '\0'; sc++) {
			if (*sc == '%') {
				infmt = 1;
				continue;
			}
			if (*sc == '*' && sc[1] != '\0') {
				ptr++;
			} else if (*sc == '[' && sc[1] != '\0') {
				ptr_long++;
				break;
			} else if (*sc == 'l') {
				lflag++;
			} else if (infmt && strchr("sdiufgp]", *sc) != NULL) {
				break;
			} else if (strchr(".0123456789", *sc)) {
				continue;
			} else {
				infmt = 0;
			}
		}
		if (*sc == '\0' || !infmt) {
			c->type = AG_CELL_NULL;
			continue;
		}
		if (ptr) {
			c->data.p = va_arg(ap, void *);
		} else if (ptr_long) {
			sc++;
			c->data.p = va_arg(ap, void *);
			if (sc[0] == 's') {
				if (sc[1] == '3' && sc[2] == '2') {
					c->type = AG_CELL_PSINT32;
				} else if (s[1] == '1' && sc[2] == '6') {
					c->type = AG_CELL_PSINT16;
				} else if (s[1] == '8') {
					c->type = AG_CELL_PSINT8;
				}
			} else if (sc[0] == 'u') {
				if (sc[1] == '3' && sc[2] == '2') {
					c->type = AG_CELL_PUINT32;
				} else if (s[1] == '1' && sc[2] == '6') {
					c->type = AG_CELL_PUINT16;
				} else if (s[1] == '8') {
					c->type = AG_CELL_PUINT8;
				}
			} else if (sc[0] == 'F' && sc[1] == 't') {
				c->type = AG_CELL_FN_TXT;
				c->fnTxt = c->data.p;
				c->data.p = c;
			} else if (sc[0] == 'F' && sc[1] == 's') {
				c->type = AG_CELL_FN_SU;
				c->fnSu = c->data.p;
				c->data.p = c;
			} else if (sc[0] == 'W') {
				AG_SizeAlloc a;

				a.x = 0;
				a.y = 0;
				a.w = 0;
				a.h = 0;
				c->type = AG_CELL_WIDGET;
				c->widget = c->data.p;
				AG_ObjectAttach(t, c->widget);
				AG_WidgetSizeAlloc(c->widget, &a);
				t->flags |= AG_TABLE_WIDGETS;
			}
		}
		switch (sc[0]) {
		case 's':
			if (ptr) {
				c->type = AG_CELL_PSTRING;
			} else {
				c->type = AG_CELL_STRING;
				Strlcpy(c->data.s, va_arg(ap, char *),
				    sizeof(c->data.s));
			}
			break;
		case 'd':
		case 'i':
			if (lflag == 0) {
				if (ptr) {
					c->type = AG_CELL_PINT;
				} else {
					c->type = AG_CELL_INT;
					c->data.i = va_arg(ap, int);
				}
#ifdef HAVE_64BIT
			} else if (lflag == 2) {
				if (ptr) {
					c->type = AG_CELL_PINT64;
				} else {
					c->type = AG_CELL_INT64;
					c->data.l = va_arg(ap, Sint64);
				}
#endif
			} else {
				if (ptr) {
					c->type = AG_CELL_PLONG;
				} else {
					c->type = AG_CELL_LONG;
					c->data.l = va_arg(ap, long);
				}
			}
			break;
		case 'u':
			if (lflag == 0) {
				if (ptr) {
					c->type = AG_CELL_PUINT;
				} else {
					c->type = AG_CELL_UINT;
					c->data.i = va_arg(ap, int);
				}
#ifdef HAVE_64BIT
			} else if (lflag == 2) {
				if (ptr) {
					c->type = AG_CELL_PUINT64;
				} else {
					c->type = AG_CELL_UINT64;
					c->data.l = va_arg(ap, Uint64);
				}
#endif
			} else {
				if (ptr) {
					c->type = AG_CELL_PULONG;
				} else {
					c->type = AG_CELL_ULONG;
					c->data.l = va_arg(ap, Ulong);
				}
			}
			break;
		case 'f':
		case 'g':
			if (lflag) {
				if (ptr) {
					c->type = AG_CELL_PFLOAT;
				} else {
					c->type = AG_CELL_FLOAT;
					c->data.f = va_arg(ap, double);
				}
			} else {
				if (ptr) {
					c->type = AG_CELL_PDOUBLE;
				} else {
					c->type = AG_CELL_DOUBLE;
					c->data.f = va_arg(ap, double);
				}
			}
			break;
		case 'p':
			c->type = AG_CELL_POINTER;
			c->data.p = va_arg(ap, void *);
			break;
		default:
			break;
		}
	}
	va_end(ap);

	rv = t->m++;
	AG_ObjectUnlock(t);
	return (rv);
}

int
AG_TableSaveASCII(AG_Table *t, FILE *f, char sep)
{
	char txt[AG_TABLE_TXT_MAX];
	Uint m, n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		if (t->cols[n].name[0] == '\0') {
			continue;
		}
		fputs(t->cols[n].name, f);
		fputc(sep, f);
	}
	fputc('\n', f);
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++) {
			if (t->cols[n].name[0] == '\0') {
				continue;
			}
			PrintCell(t, &t->cells[m][n], txt, sizeof(txt));
			fputs(txt, f);
			fputc(sep, f);
		}
		fputc('\n', f);
	}
	AG_ObjectUnlock(t);
	return (0);
}

static Uint32
DecrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Table *t = obj;
	Uint8 *ks;
	int numkeys;

	ks = SDL_GetKeyState(&numkeys);
	DecrementSelection(t, ks[SDLK_PAGEUP] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static Uint32
IncrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Table *t = obj;
	Uint8 *ks;
	int numkeys;

	ks = SDL_GetKeyState(&numkeys);
	IncrementSelection(t, ks[SDLK_PAGEDOWN] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static void
Init(void *obj)
{
	AG_Table *t = obj;

	WIDGET(t)->flags |= AG_WIDGET_FOCUSABLE|
	                    AG_WIDGET_UNFOCUSED_MOTION|
	                    AG_WIDGET_UNFOCUSED_BUTTONUP;

	t->sep = ":";
	t->flags = 0;
	t->hRow = agTextFontHeight+2;
	t->hCol = agTextFontHeight+4;
	t->wColMin = 16;
	t->wColDefault = 80;
	t->wHint = -1;				/* Use column size specs */
	t->hHint = t->hCol + t->hRow*2;
	t->r = AG_RECT(0,0,0,0);
	t->selMode = AG_TABLE_SEL_ROWS;
	t->selColor[0] = 0;
	t->selColor[1] = 0;
	t->selColor[2] = 250;
	t->selColor[3] = 32;
	t->wTot = 0;

	t->vbar = AG_ScrollbarNew(t, AG_SCROLLBAR_VERT, 0);
	t->hbar = AG_ScrollbarNew(t, AG_SCROLLBAR_HORIZ, 0);
	AG_SetInt(t->hbar, "min", 0);
	AG_BindInt(t->hbar, "value", &t->xOffs);
	AG_BindInt(t->hbar, "visible", &t->r.w);
	AG_BindInt(t->hbar, "max", &t->wTot);

	AG_WidgetSetFocusable(t->vbar, 0);
	AG_WidgetSetFocusable(t->hbar, 0);

	t->poll_ev = NULL;
	t->nResizing = -1;
	t->cols = NULL;
	t->cells = NULL;
	t->n = 0;
	t->m = 0;
	t->mVis = 0;
	t->xOffs = 0;
	t->dblClickRowEv = NULL;
	t->dblClickColEv = NULL;
	t->dblClickCellEv = NULL;
	t->dblClickedRow = -1;
	t->dblClickedCol = -1;
	t->wheelTicks = 0;
	SLIST_INIT(&t->popups);
	
	AG_SetEvent(t, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(t, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(t, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(t, "window-keydown", KeyDown, NULL);
	AG_SetEvent(t, "window-keyup", KeyUp, NULL);
	AG_SetEvent(t, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(t, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(t, "detached", LostFocus, NULL);
	AG_SetEvent(t, "dblclick-row-expire", ExpireRowDblClick, NULL);
	AG_SetEvent(t, "dblclick-col-expire", ExpireColDblClick, NULL);
	AG_SetEvent(t, "dblclick-cell-expire", ExpireCellDblClick, NULL);

	AG_SetTimeout(&t->decTo, DecrementTimeout, NULL, 0);
	AG_SetTimeout(&t->incTo, IncrementTimeout, NULL, 0);
}

static void
Destroy(void *obj)
{
	AG_Table *t = obj;
	AG_TablePopup *tp, *tpn;
	Uint m, n;
	
	for (tp = SLIST_FIRST(&t->popups);
	     tp != SLIST_END(&t->popups);
	     tp = tpn) {
		tpn = SLIST_NEXT(tp, popups);
		AG_ObjectDestroy(tp->menu);
		Free(tp);
	}
	
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++)
			AG_TableFreeCell(t, &t->cells[m][n]);
	}
	for (n = 0; n < t->n; n++) {
		AG_TablePoolFree(t, n);
	}
	Free(t->cols);
}


AG_WidgetClass agTableClass = {
	{
		"Agar(Widget:Table)",
		sizeof(AG_Table),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
