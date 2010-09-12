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

#include "table.h"
#include "primitive.h"
#include "cursors.h"
#include "keyboard.h"

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
	AG_ScheduleTimeout(t, &t->pollTo, 250);
	AG_ObjectUnlock(t);
	return (t);
}

void
AG_TableSetPollInterval(AG_Table *t, Uint ival)
{
	AG_ObjectLock(t);
	if (ival == 0) {
		AG_DelTimeout(t, &t->pollTo);
	} else {
		AG_ScheduleTimeout(t, &t->pollTo, ival);
	}
	AG_ObjectUnlock(t);
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
	t->selColor = AG_ColorRGBA(r,g,b,a);
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableSetColumnAction(AG_Table *t, Uint flags)
{
	AG_ObjectLock(t);
	t->colAction = flags;
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
	AG_WidgetUpdate(t);
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

/* Set row height in pixels */
void
AG_TableSetRowHeight(AG_Table *t, int h)
{
	AG_ObjectLock(t);
	t->hRow = h;
	AG_WidgetUpdate(t);
	AG_ObjectUnlock(t);
	AG_Redraw(t);
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
	int n;

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

/* Set the effective position of all embedded widgets in the table. */
static void
UpdateEmbeddedWidgets(AG_Table *t)
{
	AG_SizeAlloc wa;
	AG_Widget *wt;
	AG_Rect rd;
	AG_TableCol *col;
	AG_TableCell *c;
	int m, n;
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
	int vBarSz = 0, hBarSz = 0;
	
	t->r.w = a->w;
	t->r.h = a->h - t->hCol;
	t->r.y = t->hCol;

	if (t->r.h <= 0)
		return (-1);
	
	if (t->vbar && AG_WidgetVisible(t->vbar)) {
		AG_WidgetSizeReq(t->vbar, &rBar);
		if (rBar.w > a->w/2) { rBar.w = a->w/2; }
		aBar.x = a->w - rBar.w;
		aBar.y = 0;
		aBar.w = rBar.w;
		aBar.h = a->h;
		AG_WidgetSizeAlloc(t->vbar, &aBar);
		vBarSz = WIDTH(t->vbar);
	}
	if (t->hbar && AG_WidgetVisible(t->hbar)) {
		if (vBarSz) {
			aBar.h -= rBar.w;
			AG_WidgetSizeAlloc(t->vbar, &aBar);
		}
		AG_WidgetSizeReq(t->hbar, &rBar);
		if (rBar.h > a->h/2) { rBar.h = a->h/2; }
		aBar.x = 0;
		aBar.y = a->h - rBar.h;
		aBar.w = a->w - vBarSz;
		aBar.h = rBar.h;
		AG_WidgetSizeAlloc(t->hbar, &aBar);
		hBarSz = HEIGHT(t->hbar);
	}

	t->r.w -= vBarSz;
	t->r.h -= hBarSz;

	SizeColumns(t);

	/* Limit horizontal scrollbar parameters */
	if (t->xOffs + t->r.w >= t->wTot)
		t->xOffs = MAX(0, t->wTot - t->r.w);

	/* Limit vertical scrollbar parameters */
	t->mVis = t->r.h/t->hRow;
	if (t->mOffs+t->mVis >= t->m)
		t->mOffs = MAX(0, t->m - t->mVis);

	return (0);
}

void
AG_TablePrintCell(const AG_TableCell *c, char *buf, size_t bufsz)
{
	switch (c->type) {
	case AG_CELL_INT:
		if (strcmp(c->fmt, "%d") == 0) {
			StrlcpyInt(buf, c->data.i, bufsz);
		} else {
			Snprintf(buf, bufsz, c->fmt, c->data.i);
		}
		break;
	case AG_CELL_UINT:
		if (strcmp(c->fmt, "%u") == 0) {
			StrlcpyUint(buf, (Uint)c->data.i, bufsz);
		} else {
			Snprintf(buf, bufsz, c->fmt, (Uint)c->data.i);
		}
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
		Snprintf(buf, bufsz, c->fmt, c->data.u64);
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
	case AG_CELL_POINTER:
		Snprintf(buf, bufsz, c->fmt, c->data.p);
		break;
	default:
		if (bufsz > 0) {
			buf[0] = '\0';
		}
		break;
	}
}

static __inline__ void
DrawCell(AG_Table *t, AG_TableCell *c, AG_Rect *rd)
{
	char txt[AG_TABLE_TXT_MAX];

	if (c->surface != -1) {
		if (t->flags & AG_TABLE_REDRAW_CELLS) {
			AG_WidgetUnmapSurface(t, c->surface);
			c->surface = -1;
		} else {
			goto blit;
		}
	}

	switch (c->type) {
	case AG_CELL_STRING:					/* Avoid copy */
		AG_TextColor(agColors[TEXT_COLOR]);
		c->surface = AG_WidgetMapSurface(t, AG_TextRender(c->data.s));
		goto blit;
	case AG_CELL_PSTRING:					/* Avoid copy */
		AG_TextColor(agColors[TEXT_COLOR]);
		c->surface = AG_WidgetMapSurface(t, AG_TextRender((char *)
		                                                  c->data.p));
		goto blit;
	case AG_CELL_FN_SU:
		c->surface = AG_WidgetMapSurface(t,
		    c->fnSu(c->data.p, rd->x, rd->y));
		goto blit;
	case AG_CELL_FN_SU_NODUP:
		c->surface = AG_WidgetMapSurfaceNODUP(t,
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
			AG_TextColor(agColors[TEXT_COLOR]);
			c->surface = AG_WidgetMapSurface(t, AG_TextRender(c->fmt));
			goto blit;
		} else {
			return;
		}
		break;
	default:
		AG_TablePrintCell(c, txt, sizeof(txt));
		break;
	}
	AG_TextColor(agColors[TEXT_COLOR]);
	c->surface = AG_WidgetMapSurface(t, AG_TextRender(txt));
blit:
	AG_WidgetBlitSurface(t, c->surface,
	    rd->x,
	    rd->y + (t->hRow>>1) - (WSURFACE(t,c->surface)->h>>1));
}

static void
ScrollToSelection(AG_Table *t)
{
	int m;

	if (t->n < 1) {
		return;
	}
	for (m = 0; m < t->m; m++) {
		if (!t->cells[m][0].selected) {
			continue;
		}
		t->mOffs = (t->mOffs > m) ? m :
		    MAX(0, m - t->mVis + 2);
		AG_Redraw(t);
		return;
	}
}

#if 0
static void
SelectionToScroll(AG_Table *t)
{
	int m, n;

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
	int n, m;
	int x, y;
	
	for (n = 0, x = -t->xOffs;
	     n < t->n && x < t->r.w;
	     n++) {
		AG_TableCol *col = &t->cols[n];
		int cw;
	
		if (col->w <= 0) {
			continue;
		}
		cw = ((x + col->w) < t->r.w) ? col->w: t->r.w - x;
		for (m = t->mOffs, y = t->hCol;
		     m < MIN(t->m, t->mOffs+t->mVis) && (y < t->hCol+t->r.h);
		     m++) {
			if (t->cells[m][n].selected &&
			    m < (t->mOffs + t->mVis - 1)) {
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
	int n, m;

	STYLE(t)->TableBackground(t, t->r);
	
	AG_WidgetDraw(t->vbar);
	AG_WidgetDraw(t->hbar);
	
	if (t->flags & AG_TABLE_WIDGETS)
		UpdateEmbeddedWidgets(t);
	
	if (!(t->flags & AG_TABLE_NOAUTOSORT) &&
	    t->flags & AG_TABLE_NEEDSORT)
		AG_TableSort(t);

	AG_PushTextState();

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
			    agColors[TABLE_LINE_COLOR]);
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

			if (col->flags & AG_TABLE_SORT_ASCENDING) {
				AG_DrawArrowUp(t,
				    rCell.x + col->w - 10,
				    t->hCol/2,
				    10,
				    agColors[SCROLLBAR_ARR1_COLOR],
				    agColors[SCROLLBAR_ARR2_COLOR]);
			} else if (col->flags & AG_TABLE_SORT_DESCENDING) {
				AG_DrawArrowDown(t,
				    rCell.x + col->w - 10,
				    t->hCol/2,
				    10,
				    agColors[SCROLLBAR_ARR1_COLOR],
				    agColors[SCROLLBAR_ARR2_COLOR]);
			}
		}

		/* Rows of this column */
		for (m = t->mOffs, rCell.y = t->hCol;
		     m < t->m && (rCell.y < rCol.h);
		     m++) {
			AG_TableCell *c = &t->cells[m][n];

			AG_DrawLineH(t, 0, t->r.w, rCell.y,
			    agColors[TABLE_LINE_COLOR]);

			DrawCell(t, c, &rCell);
			if (c->selected) {
				AG_DrawRectBlended(t, rCell, t->selColor,
				    AG_ALPHA_SRC);
			}
			rCell.y += t->hRow;
		}

		AG_DrawLineH(t, 0, t->r.w, rCell.y,
		    agColors[TABLE_LINE_COLOR]);

		/* Indicate column selection. */
		if ((t->flags & AG_TABLE_HIGHLIGHT_COLS) && col->selected) {
			STYLE(t)->TableSelectedColumnBackground(t, n, rCol);
		}
		
		AG_PopClipRect(t);
		rCell.x += col->w;
	}
	if (rCell.x > 0 &&
	    rCell.x < t->r.w) {
		AG_DrawLineV(t,
		    rCell.x - 1,
		    t->hCol - 1,
		    rCol.h,
		    agColors[TABLE_LINE_COLOR]);
	}
	t->flags &= ~(AG_TABLE_REDRAW_CELLS);

	AG_PopTextState();
}

/* Register a new popup menu. */
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

/* Register a row double-click callback function. */
void
AG_TableSetRowDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickRowEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickRowEv, fmt);
	AG_ObjectUnlock(t);
}

/* Register a column double-click callback function. */
void
AG_TableSetColDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickColEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickColEv, fmt);
	AG_ObjectUnlock(t);
}

/* Register a cell double-click callback function. */
void
AG_TableSetCellDblClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->dblClickCellEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->dblClickCellEv, fmt);
	AG_ObjectUnlock(t);
}

/* Register a row single-click callback function. */
void
AG_TableSetRowClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->clickRowEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->clickRowEv, fmt);
	AG_ObjectUnlock(t);
}

/* Register a column single-click callback function. */
void
AG_TableSetColClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->clickColEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->clickColEv, fmt);
	AG_ObjectUnlock(t);
}

/* Register a cell single-click callback function. */
void
AG_TableSetCellClickFn(AG_Table *t, AG_EventFn ev, const char *fmt, ...)
{
	AG_ObjectLock(t);
	t->clickCellEv = AG_SetEvent(t, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(t->clickCellEv, fmt);
	AG_ObjectUnlock(t);
}

/* Redraw table cells. */
void
AG_TableRedrawCells(AG_Table *t)
{
	AG_ObjectLock(t);
	t->flags |= AG_TABLE_REDRAW_CELLS;
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

/*
 * Cleanup a table cell structure.
 * Table must be locked.
 */
void
AG_TableFreeCell(AG_Table *t, AG_TableCell *c)
{
	if (c->widget != NULL) {
		AG_ObjectDetach(c->widget);
		AG_ObjectDestroy(c->widget);
	}
	if (c->surface != -1) {
		AG_WidgetUnmapSurface(t, c->surface);
		c->surface = -1;
	}
}

static __inline__ Uint
HashPrevCell(AG_Table *t, const AG_TableCell *c)
{
	char buf[AG_TABLE_HASHBUF_MAX];
	Uint h;
	Uchar *p;

	AG_TablePrintCell(c, buf, sizeof(buf));
	for (h = 0, p = (Uchar *)buf; *p != '\0'; p++) {
		h = 31*h + *p;
	}
	return (h % t->nPrevBuckets);
}

/*
 * Clear the items on the table and save the selection state. The function
 * returns with the table locked.
 */
void
AG_TableBegin(AG_Table *t)
{
	int m, n;

	AG_ObjectLock(t);		/* Lock across TableBegin/End */

	/* Copy the existing cells to the backing store and free the table. */
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++) {
			AG_TableCell *c = &t->cells[m][n], *cPrev;
			AG_TableBucket *tbPrev = &t->cPrev[HashPrevCell(t,c)];

			cPrev = Malloc(sizeof(AG_TableCell));
			memcpy(cPrev, c, sizeof(AG_TableCell));
			TAILQ_INSERT_HEAD(&tbPrev->cells, cPrev, cells);
			TAILQ_INSERT_HEAD(&t->cPrevList, cPrev, cells_list);
		}
		Free(t->cells[m]);
	}
	Free(t->cells);
	t->cells = NULL;
	t->m = 0;
	t->flags &= ~(AG_TABLE_WIDGETS);
}

/* Compare two "%[Ft]" cells. */
static int
AG_TableCompareFnTxtCells(const AG_TableCell *c1, const AG_TableCell *c2)
{
	char b1[AG_TABLE_TXT_MAX];
	char b2[AG_TABLE_TXT_MAX];

	c1->fnTxt(c1->data.p, b1, sizeof(b1));
	c2->fnTxt(c2->data.p, b2, sizeof(b2));
	return (strcoll(b1, b2));
}

/* Compare two table cells. */
int
AG_TableCompareCells(const AG_TableCell *c1, const AG_TableCell *c2)
{
	if (c1->type != c2->type || strcmp(c1->fmt, c2->fmt) != 0) {
		if (c1->type == AG_CELL_NULL || c2->type == AG_CELL_NULL) {
			return (c1->type == AG_CELL_NULL ? 1 : -1);
		}
		return (1);
	}
	if (c1->id != 0 && c2->id != 0) {
		return (c1->id - c2->id);
	}
	switch (c1->type) {
	case AG_CELL_STRING:
		return (strcoll(c1->data.s, c2->data.s));
	case AG_CELL_PSTRING:
		return (strcoll((char *)c1->data.p, (char *)c2->data.p));
	case AG_CELL_INT:
		return (c1->data.i - c2->data.i);
	case AG_CELL_UINT:
		return ((Uint)c1->data.i - (Uint)c2->data.i);
	case AG_CELL_LONG:
		return (c1->data.l - c2->data.l);
	case AG_CELL_ULONG:
		return ((Ulong)c1->data.l - (Ulong)c2->data.l);
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
	case AG_CELL_FN_SU_NODUP:
		return ((c1->data.p != c2->data.p) || (c1->fnSu != c2->fnSu));
	case AG_CELL_FN_TXT:
		return AG_TableCompareFnTxtCells(c1, c2);
	case AG_CELL_WIDGET:
		/* XXX TODO hooks */
		return (1);
	case AG_CELL_NULL:
		return (0);
	}
	return (1);
}

/* Restore selection state on a per-row basis. */
static void
TableRestoreRowSelections(AG_Table *t)
{
	int m, n;
	int nMatched, nCompared, sel;

	for (m = 0; m < t->m; m++) {
		nMatched = 0;
		nCompared = 0;
		sel = 0;
		for (n = 0; n < t->n; n++) {
			AG_TableCell *c = &t->cells[m][n];
			AG_TableCell *cPrev;
			AG_TableBucket *tb;

			if (c->type == AG_CELL_NULL) {
				continue;
			}
			tb = &t->cPrev[HashPrevCell(t,c)];
			TAILQ_FOREACH(cPrev, &tb->cells, cells) {
				if (AG_TableCompareCells(c, cPrev) != 0) {
					continue;
				}
				c->surface = cPrev->surface;
				cPrev->surface = -1;
				if (!(cPrev->flags & AG_TABLE_CELL_NOCOMPARE)) {
					if (cPrev->selected)
						nMatched++;
				}
			}
			if (!(c->flags & AG_TABLE_CELL_NOCOMPARE))
				nCompared++;
		}
		if (nMatched == nCompared)
			AG_TableSelectRow(t, m);
	}
}

/*
 * Restore selection state on a per-cell basis. In most applications, the
 * user will almost always need to specify per-cell unique IDs for selections
 * to restore properly.
 */
static void
TableRestoreCellSelections(AG_Table *t)
{
	int m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++) {
			AG_TableCell *c = &t->cells[m][n];
			AG_TableCell *cPrev;
			AG_TableBucket *tb;
			
			if (c->type == AG_CELL_NULL) {
				continue;
			}
			tb = &t->cPrev[HashPrevCell(t,c)];
			TAILQ_FOREACH(cPrev, &tb->cells, cells) {
				if (AG_TableCompareCells(c, cPrev) != 0) {
					continue;
				}
				c->selected = cPrev->selected;
				c->surface = cPrev->surface;
				cPrev->surface = -1;
				break;
			}
		}
	}
}

/*
 * Restore selection state on a per-column basis.
 */
static void
TableRestoreColSelections(AG_Table *t)
{
	int m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++) {
			AG_TableCell *c = &t->cells[m][n];
			AG_TableCell *cPrev;
			AG_TableBucket *tb;
			
			if (c->type == AG_CELL_NULL) {
				continue;
			}
			tb = &t->cPrev[HashPrevCell(t,c)];

			TAILQ_FOREACH(cPrev, &tb->cells, cells) {
				if (AG_TableCompareCells(c, cPrev) != 0) {
					continue;
				}
				c->surface = cPrev->surface;
				cPrev->surface = -1;
			}
		}
	}
}

/*
 * Restore the selection state and recover the surfaces of matching
 * items in the backing store. Unlock the table.
 */
void
AG_TableEnd(AG_Table *t)
{
	AG_TableCell *tc, *tcNext;

	if (t->n == 0)
		goto out;
	
	/* Recover surfaces and selection state from the backing store. */
	switch (t->selMode) {
	case AG_TABLE_SEL_ROWS:
		TableRestoreRowSelections(t);
		break;
	case AG_TABLE_SEL_CELLS:
		TableRestoreCellSelections(t);
		break;
	case AG_TABLE_SEL_COLS:
		TableRestoreColSelections(t);
		break;
	}

	/* Clear the backing store. */
	for (tc = TAILQ_FIRST(&t->cPrevList);
	     tc != TAILQ_END(&t->cPrevList);
	     tc = tcNext) {
		tcNext = TAILQ_NEXT(tc, cells_list);
		if (tc->surface != -1) {
			AG_WidgetUnmapSurface(t, tc->surface);
		}
		Free(tc);
	}
	TAILQ_INIT(&t->cPrevList);

	/* It is safe to use memset() in place of TAILQ_INIT() in a loop. */
	memset(t->cPrev, 0, t->nPrevBuckets*sizeof(AG_TableBucket));
out:
	AG_ObjectUnlock(t);		/* Lock across TableBegin/End */
}

static int
AG_TableSortCellsAsc(const void *p1, const void *p2)
{
	const AG_TableCell *row1 = *(const AG_TableCell **)p1;
	const AG_TableCell *row2 = *(const AG_TableCell **)p2;
	AG_Table *t = row1[0].tbl;
	AG_TableCol *tc = &t->cols[t->nSorting];

	if (tc->sortFn) {
		return tc->sortFn(&row1[t->nSorting], &row2[t->nSorting]);
	}
	return AG_TableCompareCells(&row1[t->nSorting], &row2[t->nSorting]);
}

static int
AG_TableSortCellsDsc(const void *p1, const void *p2)
{
	const AG_TableCell *row1 = *(const AG_TableCell **)p1;
	const AG_TableCell *row2 = *(const AG_TableCell **)p2;
	AG_Table *t = row1[0].tbl;
	AG_TableCol *tc = &t->cols[t->nSorting];

	if (tc->sortFn) {
		return tc->sortFn(&row2[t->nSorting], &row1[t->nSorting]);
	}
	return AG_TableCompareCells(&row2[t->nSorting], &row1[t->nSorting]);
}

/* Sort the items in the table. */
void
AG_TableSort(AG_Table *t)
{
	int i;
	int (*sortFn)(const void *, const void *) = NULL;

	for (i = 0; i < t->n; i++) {
		AG_TableCol *tc = &t->cols[i];

		if (tc->flags & AG_TABLE_SORT_ASCENDING) {
			sortFn = AG_TableSortCellsAsc;
			break;
		} else if (tc->flags & AG_TABLE_SORT_DESCENDING) {
			sortFn = AG_TableSortCellsDsc;
			break;
		}
	}
	if (i == t->n) {
		return;
	}
	t->nSorting = i;
	qsort(t->cells, t->m, sizeof(AG_TableCell *), sortFn);
	t->flags &= ~(AG_TABLE_NEEDSORT);
}

/* Return true if multiple item selection is enabled. */
static __inline__ int
SelectingMultiple(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTITOGGLE) ||
	        ((t->flags & AG_TABLE_MULTI) &&
		  AG_GetModState(WIDGET(t)->drv->kbd) & AG_KEYMOD_CTRL));
}

/* Return true if a range of items are being selected. */
static __inline__ int
SelectingRange(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTI) &&
	        (AG_GetModState(WIDGET(t)->drv->kbd) & AG_KEYMOD_SHIFT));
}

/* Display the popup menu. */
static void
ShowPopup(AG_Table *t, AG_TablePopup *tp, int x, int y)
{
	if (tp->panel != NULL) {
		AG_MenuCollapse(t, tp->item);
		tp->panel = NULL;
	}
	tp->menu->itemSel = tp->item;
	tp->panel = AG_MenuExpand(t, tp->item, x+4, y+4);
}

/* Right click on a column header; display the column's popup menu. */
static void
ColumnRightClick(AG_Table *t, int px, int py)
{
	int n;
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
					ShowPopup(t, tp, px,py);
					return;
				}
			}
		}
		cx += tc->w;
	}
}

/* Left click on a column header; column action is sort. */
static void
ColumnLeftClickSort(AG_Table *t, AG_TableCol *tc)
{
	int n;

	for (n = 0; n < t->n; n++) {
		AG_TableCol *tcOther = &t->cols[n];

		if (tcOther != tc) {
			tcOther->flags &= ~(AG_TABLE_SORT_ASCENDING);
			tcOther->flags &= ~(AG_TABLE_SORT_DESCENDING);
		}
	}
	if (tc->flags & AG_TABLE_SORT_ASCENDING) {
		tc->flags &= ~(AG_TABLE_SORT_ASCENDING);
		tc->flags |= AG_TABLE_SORT_DESCENDING;
	} else {
		tc->flags &= ~(AG_TABLE_SORT_DESCENDING);
		tc->flags |= AG_TABLE_SORT_ASCENDING;
	}
	t->flags |= AG_TABLE_NEEDSORT;
}

/* Left click on a column header; resize or execute column action. */
static void
ColumnLeftClick(AG_Table *t, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2), x1;
	int multi = SelectingMultiple(t);
	int n;

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
				if (t->colAction & AG_TABLE_COL_SELECT) {
					if (multi) {
						tc->selected = !tc->selected;
					} else {
						tc->selected = 1;
					}
				}
				if (t->colAction & AG_TABLE_COL_SORT) {
					ColumnLeftClickSort(t, tc);
				}
				if (t->clickColEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->clickColEv->name,
					    "%i", n);
				}
				if (t->dblClickedCol != -1 &&
				    t->dblClickedCol == n) {
					AG_CancelEvent(t, "dblclick-col-expire");
					if (t->dblClickColEv != NULL) {
						AG_PostEvent(NULL, t,
						    t->dblClickColEv->name,
						    "%i", n);
					}
					t->dblClickedCol = -1;
				} else {
					t->dblClickedCol = n;
					AG_SchedEvent(NULL, t,
					    agMouseDblclickDelay, "dblclick-col-expire", NULL);
				}
				goto cont;
			}
			AG_Redraw(t);
		}
		if (!multi) {
			tc->selected = 0;
			AG_Redraw(t);
		}
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
	int m, n, i, j, nc;
	
	for (nc = 0; nc < t->n; nc++) {
		tc = &t->cols[nc];
		if ((x + t->xOffs) > tc->x &&
		    (x + t->xOffs) < tc->x+tc->w)
			break;
	}
	if (nc == t->n) { nc = t->n-1; }

	switch (t->selMode) {
	case AG_TABLE_SEL_ROWS:
		if (SelectingRange(t)) {
			for (m = 0; m < t->m; m++) {
				if (AG_TableRowSelected(t,m))
					break;
			}
			if (m == t->m) {
				break;
			}
			if (m < mc) {
				for (i = m; i <= mc; i++)
					AG_TableSelectRow(t, i);
			} else if (m > mc) {
				for (i = mc; i <= m; i++)
					AG_TableSelectRow(t, i);
			} else {
				AG_TableSelectRow(t, mc);
			}
		} else if (SelectingMultiple(t)) {
			for (n = 0; n < t->n; n++) {
				c = &t->cells[mc][n];
				c->selected = !c->selected;
			}
		} else {
			AG_TableDeselectAllRows(t);
			AG_TableSelectRow(t, mc);
			if (t->clickRowEv != NULL) {
				AG_PostEvent(NULL, t,
				    t->clickRowEv->name,
				    "%i", mc);
			}
			if (t->dblClickedRow != -1 &&
			    t->dblClickedRow == mc) {
				AG_CancelEvent(t, "dblclick-row-expire");
				if (t->dblClickRowEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickRowEv->name, "%i", mc);
				}
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
			for (m = 0, n=0; m < t->m; m++) {
				for (n = 0; n < t->n; n++) {
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
				for (i = n; i <= nc; i++)
					for (j = m; j <= mc; j++)
						AG_TableSelectCell(t, j,i);
			} else if (m > mc && n > nc) {
				for (i = nc; i <= n; i++)
					for (j = mc; j <= m; j++)
						AG_TableSelectCell(t, j,i);
			} else {
				AG_TableSelectCell(t, mc, nc);
			}
		} else if (SelectingMultiple(t)) {
			c = &t->cells[mc][nc];
			c->selected = !c->selected;
		} else {
			for (m = 0; m < t->m; m++) {
				for (n = 0; n < t->n; n++) {
					c = &t->cells[m][n];
					c->selected = ((int)m == mc) &&
					              ((int)n == nc);
				}
			}
			if (t->clickCellEv != NULL) {
				AG_PostEvent(NULL, t,
				    t->clickCellEv->name,
				    "%i", mc);
			}
			if (t->dblClickedCell != -1 &&
			    t->dblClickedCell == mc) {
				AG_CancelEvent(t, "dblclick-cell-expire");
				if (t->dblClickCellEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickCellEv->name, "%i", mc);
				}
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
			for (n = 0; n < t->n; n++) {
				if (AG_TableColSelected(t,n))
					break;
			}
			if (n == t->n) {
				break;
			}
			if (n < nc) {
				for (i = n; i <= nc; i++)
					AG_TableSelectCol(t, i);
			} else if (n > nc) {
				for (i = nc; i <= n; i++)
					AG_TableSelectCol(t, i);
			} else {
				AG_TableSelectCol(t, nc);
			}
		} else if (SelectingMultiple(t)) {
			for (n = 0; n < t->n; n++) {
				tc = &t->cols[n];
				tc->selected = !tc->selected;
			}
		} else {
			for (n = 0; n < t->n; n++) {
				tc = &t->cols[n];
				tc->selected = ((int)n == nc);
			}
			if (t->clickColEv != NULL) {
				AG_PostEvent(NULL, t,
				    t->clickColEv->name,
				    "%i", nc);
			}
			if (t->dblClickedCol != -1 &&
			    t->dblClickedCol == nc) {
				AG_CancelEvent(t, "dblclick-col-expire");
				if (t->dblClickColEv != NULL) {
					AG_PostEvent(NULL, t,
					    t->dblClickColEv->name, "%i", nc);
				}
				t->dblClickedCol = -1;
			} else {
				t->dblClickedCol = nc;
				AG_SchedEvent(NULL, t, agMouseDblclickDelay,
				    "dblclick-col-expire", NULL);
			}
		}
		break;
	}
	AG_Redraw(t);
}

/* Right click on cell; show the cell's popup menu. */
static void
CellRightClick(AG_Table *t, int m, int px, int py)
{
	int x = px - (COLUMN_RESIZE_RANGE/2), cx;
	int n;

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
				ShowPopup(t, tp, px,py);
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
	int n;
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
	int m = t->mOffs;
	
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
	case AG_MOUSE_WHEELUP:
		t->mOffs -= AG_WidgetScrollDelta(&t->wheelTicks);
		if (t->mOffs < 0) {
			t->mOffs = 0;
		}
		AG_Redraw(t);
		break;
	case AG_MOUSE_WHEELDOWN:
		t->mOffs += AG_WidgetScrollDelta(&t->wheelTicks);
		if (t->mOffs > (t->m - t->mVis)) {
			t->mOffs = MAX(0, t->m - t->mVis);
		}
		AG_Redraw(t);
		break;
	case AG_MOUSE_LEFT:
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
	case AG_MOUSE_RIGHT:
		if (OverColumnHeader(t, y)) {
			ColumnRightClick(t, x,y);
			break;
		}
		if (t->m == 0 || t->n == 0) {
			goto out;
		}
		m = RowAtY(t, y);
		CellRightClick(t, m, x,y);
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
	case AG_MOUSE_LEFT:
		if (t->nResizing >= 0) {
			t->nResizing = -1;
			AG_Redraw(t);
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
	case AG_KEY_UP:
		DecrementSelection(t, 1);
		AG_DelTimeout(t, &t->incTo);
		AG_ScheduleTimeout(t, &t->decTo, agKbdDelay);
		break;
	case AG_KEY_DOWN:
		IncrementSelection(t, 1);
		AG_DelTimeout(t, &t->decTo);
		AG_ScheduleTimeout(t, &t->incTo, agKbdDelay);
		break;
	case AG_KEY_PAGEUP:
		DecrementSelection(t, agPageIncrement);
		AG_DelTimeout(t, &t->incTo);
		AG_ScheduleTimeout(t, &t->decTo, agKbdDelay);
		break;
	case AG_KEY_PAGEDOWN:
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

	if (t->nResizing >= 0 && t->nResizing < t->n) {
		AG_TableCol *tc = &t->cols[t->nResizing];

		if ((tc->w += xrel) < t->wColMin) {
			tc->w = t->wColMin;
		}
		SizeColumns(t);
		AG_Redraw(t);
	}
}

static void
KeyUp(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_PAGEUP:
		AG_DelTimeout(t, &t->decTo);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_PAGEDOWN:
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
AG_TableRowSelected(AG_Table *t, int m)
{
	int n;

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
AG_TableSelectRow(AG_Table *t, int m)
{
	int n;

	AG_ObjectLock(t);
	if (m < t->m) {
		for (n = 0; n < t->n; n++)
			t->cells[m][n].selected = 1;
		AG_PostEvent(NULL, t, "row-selected", "%i", m);
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableDeselectRow(AG_Table *t, int m)
{
	int n;

	AG_ObjectLock(t);
	if (m < t->m) {
		for (n = 0; n < t->n; n++)
			t->cells[m][n].selected = 0;
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableSelectAllRows(AG_Table *t)
{
	int m, n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 1;
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableDeselectAllRows(AG_Table *t)
{
	int m, n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 0;
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableSelectAllCols(AG_Table *t)
{
	int n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		t->cols[n].selected = 1;
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
}

void
AG_TableDeselectAllCols(AG_Table *t)
{
	int n;

	AG_ObjectLock(t);
	for (n = 0; n < t->n; n++) {
		t->cols[n].selected = 0;
	}
	AG_ObjectUnlock(t);
	AG_Redraw(t);
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
    int (*sortFn)(const void *, const void *))
{
	AG_TableCol *tc, *lc;
	AG_TableCol *colsNew;
	int m, n;

	AG_ObjectLock(t);

	/* Initialize the column information structure. */
	if ((colsNew = TryRealloc(t->cols, (t->n+1)*sizeof(AG_TableCol)))
	    == NULL) {
		goto fail;
	}
	t->cols = colsNew;

	tc = &t->cols[t->n];
	if (name != NULL) {
		Strlcpy(tc->name, name, sizeof(tc->name));
	} else {
		tc->name[0] = '\0';
	}
	tc->flags = 0;
	tc->sortFn = sortFn;
	tc->selected = 0;
	tc->w = 0;
	tc->wPct = -1;

	AG_PushTextState();
	AG_TextColor(agColors[TEXT_COLOR]);
	tc->surface = (name == NULL) ? -1 :
	    AG_WidgetMapSurface(t, AG_TextRender(name));
	AG_PopTextState();

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
		AG_TableCell *cNew;

		if ((cNew = TryRealloc(t->cells[m],
		    (t->n+1)*sizeof(AG_TableCell))) == NULL) {
			goto fail;
		}
		t->cells[m] = cNew;
		AG_TableInitCell(t, &t->cells[m][t->n]);
	}
	n = t->n++;
	t->flags |= AG_TABLE_NEEDSORT;
	AG_ObjectUnlock(t);
	AG_Redraw(t);
	return (n);
fail:
	AG_ObjectUnlock(t);
	return (-1);
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
	c->tbl = t;
	c->id = 0;
	c->flags = 0;
}

int
AG_TableAddRow(AG_Table *t, const char *fmtp, ...)
{
	char fmt[64], *sp = &fmt[0];
	va_list ap;
	int n, rv;
	AG_TableCell **cNew;

	Strlcpy(fmt, fmtp, sizeof(fmt));

	AG_ObjectLock(t);

	if ((cNew = TryRealloc(t->cells, (t->m+1)*sizeof(AG_TableCell))) == NULL) {
		goto fail;
	}
	if ((cNew[t->m] = TryMalloc(t->n*sizeof(AG_TableCell))) == NULL) {
		Free(cNew);
		goto fail;
	}
	t->cells = cNew;

	va_start(ap, fmtp);
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
			} else if (sc[0] == 'F' && sc[1] == 'S') {
				c->type = AG_CELL_FN_SU_NODUP;
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
				if (!(c->widget->flags &
				      AG_WIDGET_TABLE_EMBEDDABLE)) {
					AG_SetError("%s widgets are not "
					            "%%[W]-embeddable",
					    AGOBJECT_CLASS(c->widget)->name);
					goto fail;
				}
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
	t->flags |= AG_TABLE_NEEDSORT;
	AG_ObjectUnlock(t);

	AG_Redraw(t);
	return (rv);
fail:
	AG_ObjectUnlock(t);
	return (-1);
}

int
AG_TableSaveASCII(AG_Table *t, FILE *f, char sep)
{
	char txt[AG_TABLE_TXT_MAX];
	int m, n;

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
			AG_TablePrintCell(&t->cells[m][n], txt, sizeof(txt));
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
	Uint8 *ks = AG_GetKeyState(WIDGET(t)->drv->kbd, NULL);

	DecrementSelection(t, ks[AG_KEY_PAGEUP] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static Uint32
IncrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Table *t = obj;
	Uint8 *ks = AG_GetKeyState(WIDGET(t)->drv->kbd, NULL);

	IncrementSelection(t, ks[AG_KEY_PAGEDOWN] ? agPageIncrement : 1);
	return (agKbdRepeat);
}

static Uint32
PollTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Table *t = obj;

	t->poll_ev->handler(t->poll_ev);
	if (t->mOffs+t->mVis >= t->m) {
		t->mOffs = MAX(0, t->m - t->mVis);
	}
	AG_Redraw(t);
	return (ival);
}

static void
Init(void *obj)
{
	AG_Table *t = obj;
	Uint i;

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
	t->selColor = AG_ColorRGBA(0,0,250,32);
	t->wTot = 0;
	t->colAction = AG_TABLE_COL_SORT;
	t->nSorting = 0;

	/* Horizontal scrollbar */
	t->hbar = AG_ScrollbarNew(t, AG_SCROLLBAR_HORIZ,
	    AG_SCROLLBAR_AUTOSIZE|AG_SCROLLBAR_AUTOHIDE);
	AG_BindInt(t->hbar, "value", &t->xOffs);
	AG_BindInt(t->hbar, "visible", &t->r.w);
	AG_BindInt(t->hbar, "max", &t->wTot);
	AG_WidgetSetFocusable(t->hbar, 0);

	/* Vertical scrollbar */
	t->vbar = AG_ScrollbarNew(t, AG_SCROLLBAR_VERT,
	    AG_SCROLLBAR_AUTOSIZE|AG_SCROLLBAR_AUTOHIDE);
	AG_BindInt(t->vbar, "value", &t->mOffs);
	AG_BindInt(t->vbar, "visible", &t->mVis);
	AG_BindInt(t->vbar, "max", &t->m);
	AG_WidgetSetFocusable(t->vbar, 0);

	t->poll_ev = NULL;
	t->nResizing = -1;
	t->cols = NULL;
	t->cells = NULL;
	t->n = 0;
	t->m = 0;
	t->mVis = 0;
	t->mOffs = 0;
	t->xOffs = 0;
	t->clickRowEv = NULL;
	t->clickColEv = NULL;
	t->clickCellEv = NULL;
	t->dblClickRowEv = NULL;
	t->dblClickColEv = NULL;
	t->dblClickCellEv = NULL;
	t->dblClickedRow = -1;
	t->dblClickedCol = -1;
	t->wheelTicks = 0;
	SLIST_INIT(&t->popups);

	t->nPrevBuckets = 256;
	t->cPrev = Malloc(t->nPrevBuckets*sizeof(AG_TableBucket));
	for (i = 0; i < t->nPrevBuckets; i++) {
		AG_TableBucket *tb = &t->cPrev[i];
		TAILQ_INIT(&tb->cells);
	}
	TAILQ_INIT(&t->cPrevList);

	AG_SetEvent(t, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(t, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(t, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(t, "key-down", KeyDown, NULL);
	AG_SetEvent(t, "key-up", KeyUp, NULL);
	AG_SetEvent(t, "widget-lostfocus", LostFocus, NULL);
	AG_AddEvent(t, "widget-hidden", LostFocus, NULL);
	AG_AddEvent(t, "detached", LostFocus, NULL);
	AG_SetEvent(t, "dblclick-row-expire", ExpireRowDblClick, NULL);
	AG_SetEvent(t, "dblclick-col-expire", ExpireColDblClick, NULL);
	AG_SetEvent(t, "dblclick-cell-expire", ExpireCellDblClick, NULL);

	AG_SetTimeout(&t->decTo, DecrementTimeout, NULL, 0);
	AG_SetTimeout(&t->incTo, IncrementTimeout, NULL, 0);
	AG_SetTimeout(&t->pollTo, PollTimeout, NULL, 0);
	
#ifdef AG_DEBUG
	AG_BindUint(t, "flags", &t->flags);
	AG_BindUint(t, "selMode", &t->selMode);
	AG_BindInt(t, "wHint", &t->wHint);
	AG_BindInt(t, "hHint", &t->hHint);
	AG_BindInt(t, "hRow", &t->hRow);
	AG_BindInt(t, "hCol", &t->hCol);
	AG_BindInt(t, "wColMin", &t->wColMin);
	AG_BindInt(t, "wColDefault", &t->wColDefault);
	AG_BindInt(t, "xOffs", &t->xOffs);
	AG_BindInt(t, "mOffs", &t->mOffs);
	AG_BindInt(t, "n", &t->n);
	AG_BindInt(t, "m", &t->m);
	AG_BindInt(t, "mVis", &t->mVis);
	AG_BindInt(t, "nResizing", &t->nResizing);
	AG_BindInt(t, "wTot", &t->wTot);
#endif /* AG_DEBUG */
}

static void
Destroy(void *obj)
{
	AG_Table *t = obj;
	AG_TablePopup *pop, *nPop;
	AG_TableCell *c, *cNext;
	int i;

	/* Free the attached popup menus. */
	for (pop = SLIST_FIRST(&t->popups);
	     pop != SLIST_END(&t->popups);
	     pop = nPop) {
		nPop = SLIST_NEXT(pop, popups);
		AG_ObjectDestroy(pop->menu);
		Free(pop);
	}

	/* Free the active cells. */
	for (i = 0; i < t->m; i++) {
		Free(t->cells[i]);
	}
	Free(t->cells);

	/* Free the backing store. */
	for (c = TAILQ_FIRST(&t->cPrevList);
	     c != TAILQ_END(&t->cPrevList);
	     c = cNext) {
		cNext = TAILQ_NEXT(c, cells_list);
		Free(c);
	}
	Free(t->cPrev);

	/* Free the columns. */
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
