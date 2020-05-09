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

/*
 * Table widget. Displays a set of cells organized in one or more columns.
 * Cells can contain text, numerical values, dynamically-generated content
 * or embedded Agar widgets. It implements a polled mode with asset-recycling
 * for tables which must be cleared and reconstructed frequently. Individual
 * cells, entire rows, or columns can be selected.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/table.h>
#include <agar/gui/primitive.h>
#include <agar/gui/cursors.h>
#include <agar/gui/keyboard.h>

#include <string.h>
#include <stdarg.h>

static int     SortCellsAscending(const void *_Nonnull, const void *_Nonnull);
static int     SortCellsDescending(const void *_Nonnull, const void *_Nonnull);
static AG_Size PrintCellGeneral(const AG_TableCell *_Nonnull, char *_Nonnull, AG_Size);
static void    DrawCell(AG_Table *_Nonnull, AG_TableCell *_Nonnull, const AG_Rect *);
static void    UpdateEmbeddedWidgets(AG_Table *_Nonnull);
static int     CompareCellsGeneral(const AG_TableCell *, const AG_TableCell *);
static int     CompareCellsFnTxt(const AG_TableCell *_Nonnull, const AG_TableCell *_Nonnull);
static void    RestoreRowSelections(AG_Table *_Nonnull);
static void    RestoreCellSelections(AG_Table *_Nonnull);
static void    RestoreColSelections(AG_Table *_Nonnull);

#undef  COLUMN_RESIZE_RANGE
#define COLUMN_RESIZE_RANGE 10  /* TODO css */

AG_Table *
AG_TableNew(void *parent, Uint flags)
{
	AG_Table *t;

	t = Malloc(sizeof(AG_Table));
	AG_ObjectInit(t, &agTableClass);

	if (flags & AG_TABLE_HFILL) { WIDGET(t)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_TABLE_VFILL) { WIDGET(t)->flags |= AG_WIDGET_VFILL; }
	t->flags |= flags;

	AG_ObjectAttach(parent, t);
	return (t);
}

/* Timer callback for polling updates. */
static Uint32
PollTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	AG_Event *fnPoll = t->fn[AG_TABLE_FN_POLL];

	fnPoll->fn(fnPoll);

	if (t->mOffs+t->mVis >= t->m) {
		t->mOffs = MAX(0, t->m - t->mVis);
	}
	AG_Redraw(t);
	return (to->ival);
}

AG_Table *
AG_TableNewPolled(void *parent, Uint flags, void (*fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Table *t;
	AG_Event *ev;
	va_list ap;

	t = AG_TableNew(parent, flags);

	AG_ObjectLock(t);

	t->flags |= AG_TABLE_POLL;
	ev = t->fn[AG_TABLE_FN_POLL] = AG_SetEvent(t, "table-poll", fn, NULL);
	if (fmt) {
		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}

	AG_AddTimer(t, &t->pollTo, 250, PollTimeout, NULL);

	AG_ObjectUnlock(t);
	return (t);
}

/*
 * Return the cell at unchecked location m,n.
 * The table must be locked.
 */
AG_TableCell *
AG_TableGetCell(AG_Table *t, int m, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
#ifdef AG_DEBUG
	if (m < 0 || m >= t->m ||
	    n < 0 || n >= t->n)
		AG_FatalError("Illegal cell access");
#endif
	return (&t->cells[m][n]);
}

/*
 * Cell selection control
 * The table must be locked.
 */
int
AG_TableCellSelected(AG_Table *t, int m, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	return (t->cells[m][n].selected);
}
void
AG_TableSelectCell(AG_Table *t, int m, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->cells[m][n].selected = 1;
}
void
AG_TableDeselectCell(AG_Table *t, int m, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->cells[m][n].selected = 0;
}

/*
 * Column selection control
 * The table must be locked.
 */
int
AG_TableColSelected(AG_Table *t, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	return (t->cols[n].flags & AG_TABLE_COL_SELECTED);
}
void
AG_TableSelectCol(AG_Table *t, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->cols[n].flags |= AG_TABLE_COL_SELECTED;
}
void
AG_TableDeselectCol(AG_Table *t, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->cols[n].flags &= ~(AG_TABLE_COL_SELECTED);
}

void
AG_TableSetPollInterval(AG_Table *t, Uint ival)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	if (ival == 0) {
		AG_DelTimer(t, &t->pollTo);
	} else {
		AG_AddTimer(t, &t->pollTo, ival, PollTimeout, NULL);
	}

	AG_ObjectUnlock(t);
}

/* Set an initial size requisition in width (pixels) x n (columns). */
void
AG_TableSizeHint(AG_Table *t, int w, int n)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	if (w != -1) { t->wHint = w; }
	if (n != -1) { t->hHint = n*agTextFontHeight; }
}

/* Set the selection mode (by row, by column or by cell). */
void
AG_TableSetSelectionMode(AG_Table *t, enum ag_table_selmode mode)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->selMode = mode;
}

/* Set the column click action (SELECT, SORT). */
void
AG_TableSetColumnAction(AG_Table *t, Uint flags)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->colAction = flags;
}

/* Set the field delimiter character(s) (defaults to ":") */
void
AG_TableSetSeparator(AG_Table *t, const char *sep)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	t->sep = sep;

	AG_ObjectUnlock(t);
}

/* Set column height in pixels */
void
AG_TableSetColHeight(AG_Table *t, int h)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->hCol = h;

	AG_WidgetUpdate(t);
	AG_Redraw(t);
}

/* Set row height in pixels */
void
AG_TableSetRowHeight(AG_Table *t, int h)
{
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->hRow = h;

	AG_WidgetUpdate(t);
	AG_Redraw(t);
}

/* Specify the minimum allowed column width in pixels. */
void
AG_TableSetColMin(AG_Table *t, int w)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
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
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	t->wColDefault = w;
}

/* Register a new popup menu. */
AG_MenuItem *
AG_TableSetPopup(AG_Table *t, int m, int n)
{
	AG_TablePopup *tp;
	AG_MenuItem *rv;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	SLIST_FOREACH(tp, &t->popups, popups) {
		if (tp->m == m && tp->n == n) {
			rv = tp->item;
			goto out;
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
out:
	AG_ObjectUnlock(t);
	return (rv);
}

/* Set a callback function. */
void
AG_TableSetFn(AG_Table *t, enum ag_table_fn which, AG_EventFn fn,
    const char *fmt, ...)
{
	if (which >= AG_TABLE_FN_LAST) {
		return;
	}
	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	t->fn[which] = AG_SetEvent(t, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(t->fn[which], fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(t);
}

/* Compute the effective widths of the columns. */
static void
SizeColumns(AG_Table *_Nonnull t)
{
	AG_TableCol *tc, *tcFill = NULL;
	AG_Rect r;
	int n, x;

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

	/* Update the cursor change areas for column resize. */
	r.y = WIDGET(t)->paddingTop;
	r.h = t->hCol;
	for (n = 0, x = WIDGET(t)->paddingLeft; n < t->n; n++) {
		tc = &t->cols[n];
		
		r.x = x - COLUMN_RESIZE_RANGE/2 - t->xOffs;
		r.w = COLUMN_RESIZE_RANGE;
		AG_SetStockCursor(t, &tc->ca, &r, AG_HRESIZE_CURSOR);
		x += tc->w;
	}
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Table *t = obj;
	AG_SizeReq rBar;
	int n;

	r->h = WIDGET(t)->paddingTop + t->hHint + WIDGET(t)->paddingBottom;
	r->w = WIDGET(t)->paddingLeft + WIDGET(t)->paddingRight;

	if (t->wHint != -1) {
		r->w += t->wHint;
	} else {
		for (n = 0; n < t->n; n++) {
			AG_TableCol *tc = &t->cols[n];
		
			if (tc->flags & AG_TABLE_COL_FILL || tc->wPct != -1) {
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
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Table *t = obj;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	int vBarSz=0, hBarSz=0;

	t->r.x = WIDGET(t)->paddingLeft;
	t->r.y = WIDGET(t)->paddingTop + t->hCol;
	t->r.w = a->w - t->r.x - WIDGET(t)->paddingRight;
	t->r.h = a->h - t->r.y - WIDGET(t)->paddingBottom;

	if (t->r.h <= 0)
		return (-1);
	
	if (t->vbar) {
		AG_WidgetSizeReq(t->vbar, &rBar);
		if (rBar.w > (a->w >> 1)) { rBar.w = (a->w >> 1); }
		aBar.x = a->w - rBar.w;
		aBar.y = 0;
		aBar.w = rBar.w;
		aBar.h = a->h;
		AG_WidgetSizeAlloc(t->vbar, &aBar);
		vBarSz = WIDTH(t->vbar);
	}
	if (t->hbar) {
		if (vBarSz) {
			aBar.h -= rBar.w;
			AG_WidgetSizeAlloc(t->vbar, &aBar);
		}
		AG_WidgetSizeReq(t->hbar, &rBar);
		if (rBar.h > (a->h >> 1)) { rBar.h = (a->h >> 1); }
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

	if (t->xOffs + t->r.w >= t->wTot)                /* Horiz scrollbar */
		t->xOffs = MAX(0, t->wTot - t->r.w);

	t->mVis = t->r.h/t->hRow;                         /* Vert scrollbar */
	if (t->mOffs+t->mVis >= t->m)
		t->mOffs = MAX(0, t->m - t->mVis);

	return (0);
}

AG_Size
AG_TablePrintCell(const AG_TableCell *c, char *buf, AG_Size bufsz)
{
	switch (c->type) {
	case AG_CELL_STRING:
		return Strlcpy(buf, c->data.s, bufsz);
	case AG_CELL_PSTRING:
		return Strlcpy(buf, (char *)c->data.p, bufsz);
	case AG_CELL_INT:
		if (strcmp(c->fmt, "%d") == 0) {
			return StrlcpyInt(buf, c->data.i, bufsz);
		} else {
			return Snprintf(buf, bufsz, c->fmt, c->data.i);
		}
	case AG_CELL_PINT:
		return Snprintf(buf, bufsz, c->fmt, *(int *)c->data.p);
	case AG_CELL_UINT:
		if (strcmp(c->fmt, "%u") == 0) {
			return StrlcpyUint(buf, (Uint)c->data.i, bufsz);
		} else {
			return Snprintf(buf, bufsz, c->fmt, (Uint)c->data.i);
		}
	case AG_CELL_PUINT:
		return Snprintf(buf, bufsz, c->fmt, *(Uint *)c->data.p);
	case AG_CELL_FLOAT:
		return Snprintf(buf, bufsz, c->fmt, (float)c->data.f);
	case AG_CELL_PFLOAT:
		return Snprintf(buf, bufsz, c->fmt, *(float *)c->data.p);
	case AG_CELL_DOUBLE:
		return Snprintf(buf, bufsz, c->fmt, c->data.f);
	case AG_CELL_PDOUBLE:
		return Snprintf(buf, bufsz, c->fmt, *(double *)c->data.p);
	default:
		return PrintCellGeneral(c, buf, bufsz);
	}
}

static AG_Size
PrintCellGeneral(const AG_TableCell *_Nonnull c, char *_Nonnull buf, AG_Size bufsz)
{
	switch (c->type) {
	case AG_CELL_FN_TXT:
		return c->fnTxt(c->data.p, buf, bufsz);
	case AG_CELL_POINTER:
		return Snprintf(buf, bufsz, c->fmt, c->data.p);
	case AG_CELL_LONG:
	case AG_CELL_ULONG:
		return Snprintf(buf, bufsz, c->fmt, c->data.l);
	case AG_CELL_PLONG:
		return Snprintf(buf, bufsz, c->fmt, *(long *)c->data.p);
	case AG_CELL_PULONG:
		return Snprintf(buf, bufsz, c->fmt, *(Ulong *)c->data.p);
	case AG_CELL_PUINT8:
		return Snprintf(buf, bufsz, c->fmt, *(Uint8 *)c->data.p);
	case AG_CELL_PSINT8:
		return Snprintf(buf, bufsz, c->fmt, *(Sint8 *)c->data.p);
	case AG_CELL_PUINT16:
		return Snprintf(buf, bufsz, c->fmt, *(Uint16 *)c->data.p);
	case AG_CELL_PSINT16:
		return Snprintf(buf, bufsz, c->fmt, *(Sint16 *)c->data.p);
	case AG_CELL_PUINT32:
		return Snprintf(buf, bufsz, c->fmt, *(Uint32 *)c->data.p);
	case AG_CELL_PSINT32:
		return Snprintf(buf, bufsz, c->fmt, *(Sint32 *)c->data.p);
#ifdef HAVE_64BIT
	case AG_CELL_SINT64:
	case AG_CELL_UINT64:
		return Snprintf(buf, bufsz, c->fmt, c->data.u64);
	case AG_CELL_PUINT64:
		return Snprintf(buf, bufsz, c->fmt, *(Uint64 *)c->data.p);
	case AG_CELL_PINT64:
		return Snprintf(buf, bufsz, c->fmt, *(Sint64 *)c->data.p);
#endif
	default:
		if (bufsz > 0)
			buf[0] = '\0';
	}
	return (0);
}

static void
ScrollToSelection(AG_Table *_Nonnull t)
{
	const int mVis = t->mVis;
	int m;

	if (t->n < 1) {
		return;
	}
	for (m = 0; m < t->m; m++) {
		if (!t->cells[m][0].selected) {
			continue;
		}
		t->mOffs = (t->mOffs > m) ? m : MAX(0, m-mVis+2);
		AG_Redraw(t);
		return;
	}
}

static int _Pure_Attribute
SelectionIsVisible(const AG_Table *_Nonnull t)
{
	const int mOffsVis = t->mOffs + t->mVis;
	const int hRow = t->hRow;
	const int hCol = t->hCol;
	int n, m;
	int x, y;
	
	for (n = 0, x = -t->xOffs;
	     n < t->n && x < t->r.w;
	     n++) {
		const AG_TableCol *col = &t->cols[n];
	
		if (col->w <= 0) {
			continue;
		}
		for (m = t->mOffs, y = hCol;
		     m < MIN(t->m, mOffsVis) && (y < hCol + t->r.h);
		     m++) {
			if (t->cells[m][n].selected && m < (mOffsVis - 1)) {
				return (1);
			}
			y += hRow;
		}
		x += col->w;
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Table *t = obj;
	const AG_Color *cFg  = &WCOLOR(t, FG_COLOR);
	const AG_Color *cSel = &WCOLOR(t, SELECTION_COLOR);
	AG_Color cLine       =  WCOLOR(t, LINE_COLOR);
	AG_Rect rCol, rCell;
	const int paddingTop  = WIDGET(t)->paddingTop;
	const int paddingLeft = WIDGET(t)->paddingLeft;
	int n,m, hCol;

	AG_WidgetDraw(t->vbar);
	AG_WidgetDraw(t->hbar);
	
	if (t->flags & AG_TABLE_WIDGETS)
		UpdateEmbeddedWidgets(t);
	
	if (!(t->flags & AG_TABLE_NOAUTOSORT) &&
	    t->flags & AG_TABLE_NEEDSORT)
		AG_TableSort(t);

	rCell.h = t->hRow + 1;

	hCol = t->hCol;
	rCol.y = 0;
	rCol.h = hCol + t->r.h - 2;

	AG_PushClipRect(t, &WIDGET(t)->r);
	AG_PushBlendingMode(t, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	for (n = 0, rCell.x = paddingLeft - t->xOffs;
	     n < t->n && rCell.x < t->r.w;
	     n++) {
		AG_TableCol *col = &t->cols[n];
		const int w = col->w;
		AG_Rect r;
	
		if (w <= 0) {
			continue;
		}
		rCell.w = w;
		rCol.w = ((rCell.x + w) < t->r.w) ? w : (t->r.w - rCell.x);
		rCol.x = rCell.x;

		/* Column header and separator */
		if (rCol.x > 0 && rCol.x < t->r.w) {
			AG_DrawLineV(t, rCol.x, hCol-1, rCol.h,
			    &WCOLOR(t, LINE_COLOR));           /* (not cLine) */
		}
		AG_PushClipRect(t, &rCol);

		r.x = rCol.x;
		r.y = paddingTop;
		r.w = rCol.w+1;
		r.h = hCol;

		if (col->flags & AG_TABLE_COL_SELECTED) {
			AG_DrawBoxSunk(t, &r, cSel);
		} else {
			AG_DrawBoxRaised(t, &r, cFg);
		}

		/* Column header label */
		if (col->name[0] != '\0') {
			if (col->surface == -1) {
				col->surface = AG_WidgetMapSurface(t,
				    AG_TextRender(col->name));
			}
			AG_WidgetBlitSurface(t, col->surface,
			    rCell.x + (w >> 1) - (WSURFACE(t,col->surface)->w >> 1),
			    paddingTop + (hCol >> 1) - (WSURFACE(t,col->surface)->h >> 1));

			if (col->flags & AG_TABLE_COL_ASCENDING) {
				AG_DrawArrowUp(t,
				    rCell.x + w - COLUMN_RESIZE_RANGE,
				    paddingTop + (hCol >> 1), 10,
				    &cLine);
			} else if (col->flags & AG_TABLE_COL_DESCENDING) {
				AG_DrawArrowDown(t,
				    rCell.x + w - COLUMN_RESIZE_RANGE,
				    paddingTop + (hCol >> 1), 10,
				    &cLine);
			}
		}

		/* Rows of this column */
		for (m = t->mOffs, rCell.y = paddingTop + hCol;
		     m < t->m && (rCell.y < rCol.h);
		     m++) {
			AG_TableCell *c = &t->cells[m][n];

			if (c->selected) {
				AG_DrawRect(t, &rCell, cSel);
			}
			AG_DrawLineH(t, 0, t->r.w - 1, rCell.y,
			    c->selected ? cSel : &cLine);
			DrawCell(t, c, &rCell);
			rCell.y += t->hRow;
		}

		/* Indicate column selection. */
		if ((t->flags & AG_TABLE_HIGHLIGHT_COLS) &&
		    (col->flags & AG_TABLE_COL_SELECTED)) {
			AG_Color c;

			AG_ColorRGBA_8(&c, 0,0,250, 32);
			AG_DrawRectBlended(t, &rCol, &c,
			    AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
		}

		AG_PopClipRect(t);
		rCell.x += w;
	}

	AG_PopBlendingMode(t);
	AG_PopClipRect(t);

	if (rCell.x > 0 &&
	    rCell.x < t->r.w) {
		AG_DrawLineV(t, rCell.x, hCol-1, rCol.h, &cLine);
	}
	t->flags &= ~(AG_TABLE_REDRAW_CELLS);
}

static void
DrawCell(AG_Table *_Nonnull t, AG_TableCell *_Nonnull c, const AG_Rect *rd)
{
	char buf[AG_TABLE_BUF_MAX], *pTxt = buf;
	AG_Size cSize;
	const int cellPaddingLeft = 2;		/* TODO css */

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
		c->surface = AG_WidgetMapSurface(t, AG_TextRender(c->data.s));
		goto blit;
	case AG_CELL_PSTRING:					/* Avoid copy */
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
			c->surface = AG_WidgetMapSurface(t, AG_TextRender(c->fmt));
			goto blit;
		} else {
			return;
		}
		break;
	default:
		if ((cSize = AG_TablePrintCell(c, buf, sizeof(buf)))
		    >= sizeof(buf)) {
			if ((pTxt = TryMalloc(cSize)) == NULL) {
				return;
			}
			AG_TablePrintCell(c, pTxt, cSize);
		}
		break;
	}

	c->surface = AG_WidgetMapSurface(t, AG_TextRender(pTxt));

	if (cSize >= sizeof(buf))
		free(pTxt);
blit:
	AG_WidgetBlitSurface(t, c->surface,
	    rd->x + cellPaddingLeft,
	    rd->y + (t->hRow >> 1) - (WSURFACE(t,c->surface)->h >> 1));
}

/* Set the effective position of all embedded widgets in the table. */
static void
UpdateEmbeddedWidgets(AG_Table *_Nonnull t)
{
	AG_SizeAlloc wa;
	AG_Widget *wt;
	AG_Rect rd;
	AG_TableCol *col;
	AG_TableCell *c;
	const int hRow = t->hRow;
	int m, n;
	int updateCoords = 0;

	rd.h = hRow;

	for (n=0, rd.x=-t->xOffs; n < t->n; n++) {
		col = &t->cols[n];
		rd.w = col->w;
		rd.y = t->hCol - t->mOffs*hRow;
		for (m = 0; m < t->m; m++) {
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
				updateCoords++;
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

			rd.y += hRow;
		}
		rd.x += col->w;
	}

	/* Apply any changes to widget size allocations. */
	if (updateCoords) {
		AG_WidgetUpdateCoords(t,
		    WIDGET(t)->rView.x1,
		    WIDGET(t)->rView.y1);
	}
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

/* Sort the items in the table. */
void
AG_TableSort(AG_Table *t)
{
	int (*sortFn)(const void *, const void *) = NULL;
	int i;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (i = 0; i < t->n; i++) {
		const AG_TableCol *tc = &t->cols[i];

		if (tc->flags & AG_TABLE_COL_ASCENDING) {
			sortFn = SortCellsAscending;
			break;
		} else if (tc->flags & AG_TABLE_COL_DESCENDING) {
			sortFn = SortCellsDescending;
			break;
		}
	}
	if (i == t->n) {
		goto out;
	}
	t->nSorting = i;
	qsort(t->cells, t->m, sizeof(AG_TableCell *), sortFn);
out:
	t->flags &= ~(AG_TABLE_NEEDSORT);
	AG_ObjectUnlock(t);
}

static int
SortCellsDescending(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const AG_TableCell *row1 = *(const AG_TableCell **)p1;
	const AG_TableCell *row2 = *(const AG_TableCell **)p2;
	const AG_Table *t = row1[0].tbl;
	const AG_TableCol *tc = &t->cols[t->nSorting];

	if (tc->sortFn) {
		return tc->sortFn(&row1[t->nSorting], &row2[t->nSorting]);
	}
	return AG_TableCompareCells(&row1[t->nSorting], &row2[t->nSorting]);
}

static int
SortCellsAscending(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const AG_TableCell *row1 = *(const AG_TableCell **)p1;
	const AG_TableCell *row2 = *(const AG_TableCell **)p2;
	const AG_Table *t = row1[0].tbl;
	const AG_TableCol *tc = &t->cols[t->nSorting];

	if (tc->sortFn) {
		return tc->sortFn(&row2[t->nSorting], &row1[t->nSorting]);
	}
	return AG_TableCompareCells(&row2[t->nSorting], &row1[t->nSorting]);
}

static __inline__ Uint
HashPrevCell(AG_Table *_Nonnull t, const AG_TableCell *_Nonnull c)
{
	char buf[AG_TABLE_HASHBUF_MAX];
	Uchar *p;
	Uint h;

	AG_TablePrintCell(c, buf, sizeof(buf));
	for (h = 0, p = (Uchar *)buf; *p != '\0'; p++) {
		h = 31*h + *p;
	}
	return (h % t->nPrevBuckets);
}

void
AG_TableClear(AG_Table *t)
{
	AG_TableBegin(t);
}

/*
 * Clear the items on the table and save the selection state. The function
 * returns with the table locked.
 */
void
AG_TableBegin(AG_Table *t)
{
	int m, n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);		/* Lock across TableBegin/End */

	/* Copy the existing cells to the backing store and free the table. */
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++) {
			AG_TableCell *c = &t->cells[m][n], *cPrev;
			AG_TableBucket *tbPrev = &t->cPrev[HashPrevCell(t,c)];

			cPrev = Malloc(sizeof(AG_TableCell));
			memcpy(cPrev, c, sizeof(AG_TableCell));
			cPrev->nPrev = n;
			TAILQ_INSERT_HEAD(&tbPrev->cells, cPrev, cells);
			TAILQ_INSERT_HEAD(&t->cPrevList, cPrev, cells_list);
		}
		free(t->cells[m]);
	}
	free(t->cells);
	t->cells = NULL;
	t->m = 0;
	t->flags &= ~(AG_TABLE_WIDGETS);
}

/*
 * Restore the selection state and recover the surfaces of matching
 * items in the backing store. Unlock the table.
 */
void
AG_TableEnd(AG_Table *t)
{
	static void (*pf[])(AG_Table *_Nonnull) = {
		RestoreRowSelections,		/* SEL_ROWS */
		RestoreCellSelections,		/* SEL_CELLS */
		RestoreColSelections,		/* SEL_COLS */
	};
	AG_TableCell *tc, *tcNext;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");

	if (t->n == 0)
		goto out;

	/*
	 * Recover surfaces and selection state from the backing store.
	 */
#ifdef AG_DEBUG
	if (t->selMode >= AG_TABLE_SEL_LAST)
		AG_FatalError("selMode");
#endif
	pf[t->selMode](t);

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

	/* It is safe to use memset() in place of TAILQ_INIT(). */
	memset(t->cPrev, 0, t->nPrevBuckets*sizeof(AG_TableBucket));
out:
	AG_ObjectUnlock(t);		/* Lock across TableBegin/End */
}


/* Restore selection state on a per-row basis. */
static void
RestoreRowSelections(AG_Table *_Nonnull t)
{
	int m, n;
	int nMatched, nCompared;

	for (m = 0; m < t->m; m++) {
		nMatched = 0;
		nCompared = 0;
		for (n = 0; n < t->n; n++) {
			AG_TableCell *c = &t->cells[m][n];
			AG_TableCell *cPrev;
			AG_TableBucket *tb;

			if (c->type == AG_CELL_NULL) {
				continue;
			}
			tb = &t->cPrev[HashPrevCell(t,c)];
			TAILQ_FOREACH(cPrev, &tb->cells, cells) {
				if (cPrev->nPrev != n ||
				    AG_TableCompareCells(c, cPrev) != 0) {
					continue;
				}
				c->surface = cPrev->surface;
				cPrev->surface = -1;
				if (cPrev->selected)
					nMatched++;
			}
			nCompared++;
		}
		if (nMatched == nCompared)
			AG_TableSelectRow(t, m);
	}
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
	case AG_CELL_INT:
		return (c1->data.i - c2->data.i);
	case AG_CELL_UINT:
		return ((Uint)c1->data.i - (Uint)c2->data.i);
	/* TODO */
	case AG_CELL_FLOAT:
		return (int)((c1->data.f - c2->data.f)*1000000.0f);
	case AG_CELL_DOUBLE:
		return (int)((c1->data.f - c2->data.f)*1000000.0);
	case AG_CELL_WIDGET:
		/* TODO hooks */
		return (1);
	default:
		return CompareCellsGeneral(c1, c2);
	}
	return (1);
}

static int
CompareCellsGeneral(const AG_TableCell *c1, const AG_TableCell *c2)
{
	switch (c1->type) {
	case AG_CELL_PSTRING:
		return (strcoll((char *)c1->data.p, (char *)c2->data.p));
	case AG_CELL_PINT:
	case AG_CELL_PUINT:
		return (*(int *)c1->data.p - *(int *)c2->data.p);
	case AG_CELL_FN_SU:
	case AG_CELL_FN_SU_NODUP:
		return ((c1->data.p != c2->data.p) || (c1->fnSu != c2->fnSu));
	case AG_CELL_FN_TXT:
		return CompareCellsFnTxt(c1, c2);
	case AG_CELL_POINTER:
		return (c1->data.p != c2->data.p);
	case AG_CELL_LONG:
		return (c1->data.l - c2->data.l);
	case AG_CELL_ULONG:
		return ((Ulong)c1->data.l - (Ulong)c2->data.l);
	case AG_CELL_PULONG:
	case AG_CELL_PLONG:
		return (*(long *)c1->data.p - *(long *)c2->data.p);
	case AG_CELL_PUINT8:
	case AG_CELL_PSINT8:
		return (*(Uint8 *)c1->data.p - *(Uint8 *)c2->data.p);
	case AG_CELL_PUINT16:
	case AG_CELL_PSINT16:
		return (*(Uint16 *)c1->data.p - *(Uint16 *)c2->data.p);
	case AG_CELL_PUINT32:
	case AG_CELL_PSINT32:
		return (*(Uint32 *)c1->data.p - *(Uint32 *)c2->data.p);
#ifdef HAVE_64BIT
	case AG_CELL_SINT64:
	case AG_CELL_UINT64:
		return (c1->data.u64 - c2->data.u64);
	case AG_CELL_PUINT64:
	case AG_CELL_PINT64:
		return (*(Uint64 *)c1->data.p - *(Uint64 *)c2->data.p);
#endif
	case AG_CELL_NULL:
	default:
		return (0);
	}
}

/* Compare two "%[Ft]" cells. */
static int
CompareCellsFnTxt(const AG_TableCell *_Nonnull c1, const AG_TableCell *_Nonnull c2)
{
	char b1[AG_TABLE_BUF_MAX];
	char b2[AG_TABLE_BUF_MAX];

	/* XXX TODO handle oversize cells */
	c1->fnTxt(c1->data.p, b1, sizeof(b1));
	c2->fnTxt(c2->data.p, b2, sizeof(b2));
	return (strcoll(b1, b2));
}

/*
 * Restore selection state on a per-cell basis. In most applications, the
 * user will almost always need to specify per-cell unique IDs for selections
 * to restore properly.
 */
static void
RestoreCellSelections(AG_Table *_Nonnull t)
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
RestoreColSelections(AG_Table *_Nonnull t)
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

/* Return true if multiple item selection is enabled. */
static __inline__ int
SelectingMultiple(AG_Table *_Nonnull t)
{
	return ((t->flags & AG_TABLE_MULTITOGGLE) ||
	        ((t->flags & AG_TABLE_MULTI) &&
		  AG_GetModState(t) & AG_KEYMOD_CTRL));
}

/* Return true if a range of items are being selected. */
static __inline__ int
SelectingRange(AG_Table *_Nonnull t)
{
	return ((t->flags & AG_TABLE_MULTI) &&
	        (AG_GetModState(t) & AG_KEYMOD_SHIFT));
}

/* Display the popup menu. */
static void
ShowPopup(AG_Table *_Nonnull t, AG_TablePopup *_Nonnull tp, int x, int y)
{
	if (tp->panel != NULL) {
		AG_MenuCollapse(tp->item);
		tp->panel = NULL;
	}
	tp->menu->itemSel = tp->item;
	tp->panel = AG_MenuExpand(t, tp->item, x+4, y+4);
}

/* Right click on a column header; display the column's popup menu. */
static void
ColumnRightClick(AG_Table *_Nonnull t, int px, int py)
{
	const int x = px - (COLUMN_RESIZE_RANGE/2);
	int n, cx;

	for (n = 0, cx = -t->xOffs;
	     n < t->n;
	     n++) {
		const int w = t->cols[n].w;
		const int x2 = cx+w;

		if (x > cx && x < x2) {
			AG_TablePopup *tp;

			SLIST_FOREACH(tp, &t->popups, popups) {
				if (tp->m == -1 && tp->n == n) {
					ShowPopup(t, tp, px,py);
					return;
				}
			}
		}
		cx += w;
	}
}

/* Left click on a column header; column action is sort. */
static void
ColumnLeftClickSort(AG_Table *_Nonnull t, AG_TableCol *_Nonnull tc)
{
	int n;

	for (n = 0; n < t->n; n++) {
		AG_TableCol *tcOther = &t->cols[n];

		if (tcOther != tc) {
			tcOther->flags &= ~(AG_TABLE_COL_ASCENDING);
			tcOther->flags &= ~(AG_TABLE_COL_DESCENDING);
		}
	}
	if (tc->flags & AG_TABLE_COL_ASCENDING) {
		Debug(t, "Sorting column \"%s\" descending\n", tc->name);
		tc->flags &= ~(AG_TABLE_COL_ASCENDING);
		tc->flags |= AG_TABLE_COL_DESCENDING;
	} else {
		Debug(t, "Sorting column \"%s\" ascending\n", tc->name);
		tc->flags &= ~(AG_TABLE_COL_DESCENDING);
		tc->flags |= AG_TABLE_COL_ASCENDING;
	}
	t->flags |= AG_TABLE_NEEDSORT;
}

/* Timer callback for double click. */
static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	int *which = AG_PTR(1);

	*which = -1;
	return (0);
}

/* Left click on a column header; resize or execute column action. */
static void
ColumnLeftClick(AG_Table *_Nonnull t, int px)
{
	AG_Event *ev;
	const int x = px - (COLUMN_RESIZE_RANGE/2);
	int multi = SelectingMultiple(t);
	int x1, n;

	for (n = 0, x1 = -t->xOffs;
	     n < t->n;
	     n++) {
		AG_TableCol *tc = &t->cols[n];
		const int x2 = x1 + tc->w;

		if (x > x1 && x < x2) {
			if ((x2 - x) <= COLUMN_RESIZE_RANGE) {
				if (t->nResizing == -1) {
					Debug(t, "Resizing col#%d (\"%s\")\n",
					    n, tc->name);
					t->nResizing = n;
				}
			} else {
				if (t->colAction & AG_TABLE_COL_SELECT) {
					Debug(t, "Selecting col#%d (\"%s\")\n",
					    n, tc->name);
					if (multi) {
						AG_INVFLAGS(tc->flags,
						    AG_TABLE_COL_SELECTED);
					} else {
						tc->flags |= AG_TABLE_COL_SELECTED;
					}
				}
				if (t->colAction & AG_TABLE_COL_SORT) {
					ColumnLeftClickSort(t, tc);
				}
				if ((ev = t->fn[AG_TABLE_FN_COL_CLICK]))
					AG_PostEventByPtr(t, ev, "%i", n);

				if (t->dblClickedCol != -1 &&
				    t->dblClickedCol == n) {
					AG_DelTimer(t, &t->dblClickTo);
					if ((ev = t->fn[AG_TABLE_FN_COL_DBLCLICK])) {
						AG_PostEventByPtr(t, ev, "%i", n);
					}
					t->dblClickedCol = -1;
				} else {
					t->dblClickedCol = n;
					AG_AddTimer(t, &t->dblClickTo,
					    agMouseDblclickDelay,
					    DoubleClickTimeout, "%p", &t->dblClickedCol);
				}
				goto cont;
			}
			AG_Redraw(t);
		}
		if (!multi) {
			tc->flags &= ~(AG_TABLE_COL_SELECTED);
			AG_Redraw(t);
		}
cont:
		x1 += tc->w;
	}
}

/* Process left click on a cell. */
static void
CellLeftClick(AG_Table *_Nonnull t, int mc, int x)
{
	AG_TableCell *c;
	AG_TableCol *col;
	AG_Event *ev;
	int m, n, i, j, nc;
	int xCol = 0;
	
	if (t->n < 1)
		return;

	for (nc = 0; nc < t->n; nc++) {
		col = &t->cols[nc];

		if (((t->xOffs + x) >= xCol &&                    /* Inside */
		     (t->xOffs + x) <  xCol + col->w) ||
		    (col->flags & AG_TABLE_COL_FILL)) {
			break;
		}
		xCol += col->w;
	}
	if (nc == t->n)
		nc = (t->n - 1);

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
				Debug(t, "Selecting rows #%d-#%d\n", m, mc);
				for (i = m; i <= mc; i++)
					AG_TableSelectRow(t, i);
			} else if (m > mc) {
				Debug(t, "Selecting rows #%d-#%d\n", mc, m);
				for (i = mc; i <= m; i++)
					AG_TableSelectRow(t, i);
			} else {
				Debug(t, "Selecting row #%d\n", mc);
				AG_TableSelectRow(t, mc);
			}
		} else if (SelectingMultiple(t)) {
			for (n = 0; n < t->n; n++) {
				c = &t->cells[mc][n];
				Debug(t, "Cell[%d][%d]: Selection %d -> %d\n", mc, n,
				    c->selected, !c->selected);
				c->selected = !c->selected;
			}
		} else {
			Debug(t, "Selecting row #%d\n", mc);
			AG_TableDeselectAllRows(t);
			AG_TableSelectRow(t, mc);
			if ((ev = t->fn[AG_TABLE_FN_ROW_CLICK]) != NULL)
				AG_PostEventByPtr(t, ev, "%i", mc);

			if (t->dblClickedRow != -1 &&
			    t->dblClickedRow == mc) {
				AG_DelTimer(t, &t->dblClickTo);
				if ((ev = t->fn[AG_TABLE_FN_ROW_DBLCLICK])) {
					AG_PostEventByPtr(t, ev, "%i", mc);
				}
				t->dblClickedRow = -1;
			} else {
				t->dblClickedRow = mc;
				AG_AddTimer(t, &t->dblClickTo,
				    agMouseDblclickDelay,
				    DoubleClickTimeout, "%p", &t->dblClickedRow);
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
				Debug(t, "Selecting cells [%d,%d][%d-%d]\n",
				    n, nc, m, mc);
				for (i = n; i <= nc; i++)
					for (j = m; j <= mc; j++)
						AG_TableSelectCell(t, j,i);
			} else if (m > mc && n > nc) {
				Debug(t, "Selecting cells [%d,%d][%d,%d]\n",
				    nc, n, mc, m);
				for (i = nc; i <= n; i++)
					for (j = mc; j <= m; j++)
						AG_TableSelectCell(t, j,i);
			} else {
				Debug(t, "Selecting cell %d,%d\n", mc, nc);
				AG_TableSelectCell(t, mc, nc);
			}
		} else if (SelectingMultiple(t)) {
			c = &t->cells[mc][nc];
			Debug(t, "Cell[%d][%d]: Selection %d -> %d\n", mc, nc,
			    c->selected, !c->selected);
			c->selected = !c->selected;
		} else {
			Debug(t, "Selecting cell %d,%d\n", mc, nc);
			for (m = 0; m < t->m; m++) {
				for (n = 0; n < t->n; n++) {
					c = &t->cells[m][n];
					c->selected = ((int)m == mc) &&
					              ((int)n == nc);
				}
			}
			if ((ev = t->fn[AG_TABLE_FN_CELL_CLICK]))
				AG_PostEventByPtr(t, ev, "%i", mc);

			if (t->dblClickedCell != -1 &&
			    t->dblClickedCell == mc) {
				AG_DelTimer(t, &t->dblClickTo);
				if ((ev = t->fn[AG_TABLE_FN_CELL_DBLCLICK])) {
					AG_PostEventByPtr(t, ev, "%i", mc);
				}
				t->dblClickedCell = -1;
			} else {
				t->dblClickedCell = mc;
				AG_AddTimer(t, &t->dblClickTo,
				    agMouseDblclickDelay,
				    DoubleClickTimeout, "%p", &t->dblClickedCell);
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
				col = &t->cols[n];
				AG_INVFLAGS(col->flags, AG_TABLE_COL_SELECTED);
			}
		} else {
			for (n = 0; n < t->n; n++) {
				col = &t->cols[n];
				if (n == nc) {
					col->flags |= AG_TABLE_COL_SELECTED;
				} else {
					col->flags &= ~(AG_TABLE_COL_SELECTED);
				}
			}
			if ((ev = t->fn[AG_TABLE_FN_COL_CLICK]))
				AG_PostEventByPtr(t, ev, "%i", nc);

			if (t->dblClickedCol != -1 &&
			    t->dblClickedCol == nc) {
				AG_DelTimer(t, &t->dblClickTo);
				if ((ev = t->fn[AG_TABLE_FN_COL_DBLCLICK])) {
					AG_PostEventByPtr(t, ev, "%i", nc);
				}
				t->dblClickedCol = -1;
			} else {
				t->dblClickedCol = nc;
				AG_AddTimer(t, &t->dblClickTo,
				    agMouseDblclickDelay,
				    DoubleClickTimeout, "%p", &t->dblClickedCol);
			}
		}
		break;
	default:
		break;
	}
	AG_Redraw(t);
}

/* Right click on cell; show the cell's popup menu. */
static void
CellRightClick(AG_Table *_Nonnull t, int m, int px, int py)
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

/* Return the row at the given y-coordinate. */
static __inline__ int
RowAtY(AG_Table *_Nonnull t, int y)
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
DecrementSelection(AG_Table *_Nonnull t, int inc)
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
	if (!SelectionIsVisible(t))
		ScrollToSelection(t);
}

static void
IncrementSelection(AG_Table *_Nonnull t, int inc)
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
	if (!SelectionIsVisible(t))
		ScrollToSelection(t);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int button = AG_INT(1);
	const int x = AG_INT(2) - WIDGET(t)->paddingLeft;
	const int y = AG_INT(3) - WIDGET(t)->paddingTop;
	int m;
	
	if (!AG_WidgetIsFocused(t))
		AG_WidgetFocus(t);
	
	switch (button) {
	case AG_MOUSE_WHEELUP:
		t->mOffs -= t->lineScrollAmount;
		if (t->mOffs < 0) {
			t->mOffs = 0;
		}
		AG_Redraw(t);
		break;
	case AG_MOUSE_WHEELDOWN:
		t->mOffs += t->lineScrollAmount;
		if (t->mOffs > (t->m - t->mVis)) {
			t->mOffs = MAX(0, t->m - t->mVis);
		}
		AG_Redraw(t);
		break;
	case AG_MOUSE_LEFT:
		if (y <= t->hCol) {
			ColumnLeftClick(t, x);
			break;
		}
		if (t->m == 0 || t->n == 0) {
			Debug(t, "Click: empty table\n");
			return;
		}
		m = RowAtY(t, y);
		Debug(t, "Cell click row #%d at y=%d\n", m, y);
		CellLeftClick(t, m, x);
		break;
	case AG_MOUSE_RIGHT:
		if (y <= t->hCol) {
			ColumnRightClick(t, x,y);
			break;
		}
		if (t->m == 0 || t->n == 0) {
			return;
		}
		m = RowAtY(t, y);
		CellRightClick(t, m, x,y);
		break;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int button = AG_INT(1);

	switch (button) {
	case AG_MOUSE_LEFT:
		if (t->nResizing >= 0) {
			t->nResizing = -1;
			AG_Redraw(t);
		}
		break;
	}
}

/* Timer callback for keyboard selection moving. */
static Uint32
MoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int incr = AG_INT(1);

	if (incr < 0) {
		DecrementSelection(t, -incr);
	} else {
		IncrementSelection(t, incr);
	}
	return (agKbdRepeat);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
		DecrementSelection(t, 1);
		AG_AddTimer(t, &t->moveTo, agKbdDelay, MoveTimeout, "%i", -1);
		break;
	case AG_KEY_DOWN:
		IncrementSelection(t, 1);
		AG_AddTimer(t, &t->moveTo, agKbdDelay, MoveTimeout, "%i", +1);
		break;
	case AG_KEY_PAGEUP:
		DecrementSelection(t, agPageIncrement);
		AG_AddTimer(t, &t->moveTo, agKbdDelay, MoveTimeout, "%i", -agPageIncrement);
		break;
	case AG_KEY_PAGEDOWN:
		IncrementSelection(t, agPageIncrement);
		AG_AddTimer(t, &t->moveTo, agKbdDelay, MoveTimeout, "%i", +agPageIncrement);
		break;
	case AG_KEY_HOME:
		t->mOffs = 0;
		AG_Redraw(t);
		break;
	case AG_KEY_END:
		t->mOffs = MAX(0, t->m - t->mVis);
		AG_Redraw(t);
		break;
	}
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int x = AG_INT(1) - WIDGET(t)->paddingLeft;
	const int xrel = AG_INT(3);

	if (x < 0 || x >= WIDTH(t))
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
KeyUp(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_PAGEUP:
	case AG_KEY_DOWN:
	case AG_KEY_PAGEDOWN:
		AG_DelTimer(t, &t->moveTo);
		break;
	}
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();
	const int fontHeight = WFONT(t)->height;
	Uint m, n;
	
	t->hRow = fontHeight + 4;		/* TODO css cell-padding */
	t->hCol = fontHeight + 4;		/* TODO css head-padding */
	
	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];

		if (tc->surface != -1) {
			AG_WidgetUnmapSurface(t, tc->surface);
			tc->surface = -1;
		}
		for (m = 0; m < t->m; m++) {
			AG_TableCell *c = &t->cells[m][n];

			if (c->surface != -1) {
				AG_WidgetUnmapSurface(t, c->surface);
				c->surface = -1;
			}
		}
	}
}

static void
LostFocus(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_SELF();

	AG_DelTimer(t, &t->moveTo);
	AG_DelTimer(t, &t->dblClickTo);

	if (t->nResizing >= 0)
		t->nResizing = -1;
}

int
AG_TableRowSelected(AG_Table *t, int m)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
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

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	if (m < t->m) {
		for (n = 0; n < t->n; n++) {
			t->cells[m][n].selected = 1;
		}
		AG_PostEvent(t, "row-selected", "%i", m);
	}

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectRow(AG_Table *t, int m)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	if (m < t->m)
		for (n = 0; n < t->n; n++)
			t->cells[m][n].selected = 0;

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

void
AG_TableSelectAllRows(AG_Table *t)
{
	int m, n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 1;
	}

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectAllRows(AG_Table *t)
{
	int m, n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 0;
	}

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

void
AG_TableSelectAllCols(AG_Table *t)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (n = 0; n < t->n; n++)
		t->cols[n].flags |= AG_TABLE_COL_SELECTED;

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

void
AG_TableDeselectAllCols(AG_Table *t)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (n = 0; n < t->n; n++)
		t->cols[n].flags &= ~(AG_TABLE_COL_SELECTED);

	AG_Redraw(t);
	AG_ObjectUnlock(t);
}

int
AG_TableAddCol(AG_Table *t, const char *name, const char *size_spec,
    int (*sortFn)(const void *, const void *))
{
	AG_TableCol *tc, *colsNew;
	int m, n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	if ((colsNew = TryRealloc(t->cols, (t->n+1)*sizeof(AG_TableCol))) == NULL) {
		n = -1;
		goto out;
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
	tc->w = 0;
	tc->wPct = -1;
	tc->surface = -1;
	tc->ca = NULL;

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
		    (t->n + 1)*sizeof(AG_TableCell))) == NULL) {
			n = -1;
			goto out;
		}
		t->cells[m] = cNew;
		AG_TableInitCell(t, &t->cells[m][t->n]);
	}
	n = t->n++;
	t->flags |= AG_TABLE_NEEDSORT;
	AG_Redraw(t);
out:
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
	c->tbl = t;
	c->id = 0;
	c->nPrev = 0;
}

int
AG_TableAddRow(AG_Table *t, const char *fmtp, ...)
{
	char fmt[64], *sp = &fmt[0];
	va_list ap;
	int n, rv;
	AG_TableCell **cNew;

	Strlcpy(fmt, fmtp, sizeof(fmt));

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	if ((cNew = TryRealloc(t->cells, (t->m+1)*sizeof(AG_TableCell))) == NULL) {
		rv = -1;
		goto out;
	}
	if ((cNew[t->m] = TryMalloc(t->n*sizeof(AG_TableCell))) == NULL) {
		Free(cNew);
		rv = -1;
		goto out;
	}
	t->cells = cNew;

	va_start(ap, fmtp);
	for (n = 0; n < t->n; n++) {
		AG_TableCell *c = &t->cells[t->m][n];
		char *s = AG_Strsep(&sp, t->sep), *sc;
		int ptr = 0, lflag = 0, ptr_long = 0;
		int infmt = 0;

		AG_TableInitCell(t, c);
		if (s == NULL || s[0] == '\0') {
			continue;
		}
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
			} else if (infmt && strchr("sdixufgp]", *sc) != NULL) {
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
				{
					char *sArg = va_arg(ap, char *);

					if (sArg != NULL && sArg[0] != '\0') {
						Strlcpy(c->data.s, sArg,
						    sizeof(c->data.s));
					} else {
						c->data.s[0] = '\0';
					}
				}
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
					c->type = AG_CELL_SINT64;
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
		case 'x':
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
		}
	}
	va_end(ap);

	rv = t->m++;

	t->flags |= AG_TABLE_NEEDSORT;

	AG_Redraw(t);
out:
	AG_ObjectUnlock(t);
	return (rv);
}

void
AG_TableDelRow(AG_Table *t, int m)
{
	int n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
	AG_ObjectLock(t);

	for (n = 0; n < t->n; n++) {
		AG_TableCell *c = &t->cells[m][n];

		AG_Debug(t, "cell: %d, sel=%d\n", n, c->selected);
	}

	AG_ObjectUnlock(t);
}

int
AG_TableSaveASCII(AG_Table *t, void *pf, char sep)
{
	char buf[AG_TABLE_BUF_MAX];
	FILE *f = (FILE *)pf;
	AG_Size cSize;
	int m, n;

	AG_OBJECT_ISA(t, "AG_Widget:AG_Table:*");
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
			if ((cSize = AG_TablePrintCell(&t->cells[m][n], buf,
			    sizeof(buf))) >= sizeof(buf)) {
				char *text = TryMalloc(cSize);

				AG_TablePrintCell(&t->cells[m][n], text, cSize);
				fputs(text, f);
				free(text);
			} else {
				fputs(buf, f);
			}
			fputc(sep, f);
		}
		fputc('\n', f);
	}

	AG_ObjectUnlock(t);
	return (0);
}

static void
ScrollbarChanged(AG_Event *_Nonnull event)
{
	AG_Table *t = AG_TABLE_PTR(1);

	SizeColumns(t);
}

static void
Init(void *_Nonnull obj)
{
	AG_Table *t = obj;
	AG_Scrollbar *sb;
	Uint i;

	WIDGET(t)->flags |= AG_WIDGET_FOCUSABLE |
	                    AG_WIDGET_UNFOCUSED_MOTION |
	                    AG_WIDGET_UNFOCUSED_BUTTONUP |
			    AG_WIDGET_USE_TEXT;

	t->sep = ":";
	t->flags = 0;
	t->hRow = agTextFontHeight+4;
	t->hCol = agTextFontHeight+4;
	t->wColMin = 16;
	t->wColDefault = 80;
	t->wHint = -1;				/* Use column size specs */
	t->hHint = t->hCol + (t->hRow << 1);
	t->r.x = 0;
	t->r.y = 0;
	t->r.w = 0;
	t->r.h = 0;
	t->selMode = AG_TABLE_SEL_ROWS;
	t->wTot = 0;
	t->xOffs = 0;
	t->mOffs = 0;
	t->mVis = 0;
	t->colAction = AG_TABLE_COL_SORT;
	t->nSorting = 0;
	t->nResizing = -1;
	t->cols = NULL;
	t->cells = NULL;
	t->n = 0;
	t->m = 0;
	for (i = 0; i < AG_TABLE_FN_LAST; i++) {
		t->fn[i] = NULL;
	}
	t->lineScrollAmount = 5;
	SLIST_INIT(&t->popups);
	t->dblClickedRow = -1;
	t->dblClickedCol = -1;
	t->dblClickedCell = -1;

	AG_InitTimer(&t->moveTo, "move", 0);
	AG_InitTimer(&t->pollTo, "poll", 0);
	AG_InitTimer(&t->dblClickTo, "dblClick", 0);

	/* Horizontal scrollbar */
	sb = t->hbar = AG_ScrollbarNew(t, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_EXCL);
	AG_SetInt(sb,  "min",     0);
	AG_BindInt(sb, "max",     &t->wTot);
	AG_BindInt(sb, "value",   &t->xOffs);
	AG_BindInt(sb, "visible", &t->r.w);
	AG_WidgetSetFocusable(sb, 0);
	AG_AddEvent(sb, "scrollbar-changed", ScrollbarChanged, "%p", t);

	/* Vertical scrollbar */
	sb = t->vbar = AG_ScrollbarNew(t, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_SetInt(sb,  "min",     0);
	AG_BindInt(sb, "max",     &t->m);
	AG_BindInt(sb, "value",   &t->mOffs);
	AG_BindInt(sb, "visible", &t->mVis);
	AG_WidgetSetFocusable(sb, 0);

	t->nPrevBuckets = 256;
	t->cPrev = Malloc(t->nPrevBuckets*sizeof(AG_TableBucket));
	for (i = 0; i < t->nPrevBuckets; i++) {
		AG_TableBucket *tb = &t->cPrev[i];
		TAILQ_INIT(&tb->cells);
	}
	TAILQ_INIT(&t->cPrevList);
	
	AG_AddEvent(t, "font-changed", OnFontChange, NULL);
	AG_AddEvent(t, "widget-hidden", LostFocus, NULL);
	AG_AddEvent(t, "detached", LostFocus, NULL);
	AG_SetEvent(t, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(t, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(t, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(t, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(t, "key-down", KeyDown, NULL);
	AG_SetEvent(t, "key-up", KeyUp, NULL);
}

static void
Destroy(void *_Nonnull obj)
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
