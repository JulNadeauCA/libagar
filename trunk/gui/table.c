/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <core/view.h>

#include "table.h"

#include "primitive.h"
#include "cursors.h"

#include <string.h>
#include <stdarg.h>

static AG_WidgetOps agTableOps = {
	{
		"AG_Widget:AG_Table",
		sizeof(AG_Table),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_TableDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_TableDraw,
	AG_TableScale
};

static void mousebuttondown(AG_Event *);
static void mousebuttonup(AG_Event *);
static void mousemotion(AG_Event *);
static void keydown(AG_Event *);
static void keyup(AG_Event *);
static void lostfocus(AG_Event *);
static void kbdscroll(AG_Event *);
static void dblclick_row_expire(AG_Event *);
static void dblclick_col_expire(AG_Event *);

#define COLUMN_RESIZE_RANGE	10	/* Range in pixels for resize ctrls */
#define COLUMN_MIN_WIDTH	20	/* Minimum column width in pixels */
#define LAST_VISIBLE(t) ((t)->m - (t)->mVis + 2)

AG_Table *
AG_TableNew(void *parent, Uint flags)
{
	AG_Table *t;

	t = Malloc(sizeof(AG_Table), M_OBJECT);
	AG_TableInit(t, flags);
	AG_ObjectAttach(parent, t);
	if (flags & AG_TABLE_FOCUS) {
		AG_WidgetFocus(t);
	}
	return (t);
}

AG_Table *
AG_TableNewPolled(void *parent, Uint flags, void (*fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Table *t;

	t = Malloc(sizeof(AG_Table), M_OBJECT);
	AG_TableInit(t, flags);
	t->flags |= AG_TABLE_POLL;
	t->poll_ev = AG_SetEvent(t, "table-poll", fn, NULL);
	AG_EVENT_GET_ARGS(t->poll_ev, fmt);

	AG_ObjectAttach(parent, t);
	return (t);
}

void
AG_TableInit(AG_Table *t, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION|
	              AG_WIDGET_UNFOCUSED_BUTTONUP;

	if (flags & AG_TABLE_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_TABLE_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(t, "table", &agTableOps, wflags);
	AG_WidgetBind(t, "selected-row", AG_WIDGET_POINTER, &t->selected_row);
	AG_WidgetBind(t, "selected-col", AG_WIDGET_POINTER, &t->selected_col);
	AG_WidgetBind(t, "selected-cell", AG_WIDGET_POINTER, &t->selected_cell);

	AG_MutexInitRecursive(&t->lock);
	t->flags = flags;
	t->selected_row = NULL;
	t->selected_col = NULL;
	t->selected_cell = NULL;
	t->row_h = agTextFontHeight+2;
	t->col_h = agTextFontHeight+4;
	t->prew = -1;				/* Use column size specs */
	t->preh = 64;
	t->wTbl = -1;
	t->vbar = AG_ScrollbarNew(t, AG_SCROLLBAR_VERT, 0);
	t->hbar = AG_ScrollbarNew(t, AG_SCROLLBAR_HORIZ, 0);
	t->poll_ev = NULL;
	t->nResizing = -1;
	t->cols = NULL;
	t->cells = NULL;
	t->n = 0;
	t->m = 0;
	t->mVis = 0;
	t->xoffs = 0;
	t->poll_ev = NULL;
	t->dblClickRowEv = NULL;
	t->dblClickColEv = NULL;
	t->dblClickedRow = -1;
	t->dblClickedCol = -1;
	SLIST_INIT(&t->popups);
	
	AG_SetEvent(t, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(t, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(t, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(t, "window-keydown", keydown, NULL);
	AG_SetEvent(t, "window-keyup", keyup, NULL);
	AG_SetEvent(t, "widget-lostfocus", lostfocus, NULL);
	AG_SetEvent(t, "widget-hidden", lostfocus, NULL);
	AG_SetEvent(t, "detached", lostfocus, NULL);
	AG_SetEvent(t, "kbdscroll", kbdscroll, NULL);
	AG_SetEvent(t, "dblclick-row-expire", dblclick_row_expire, NULL);
	AG_SetEvent(t, "dblclick-col-expire", dblclick_col_expire, NULL);
}

void
AG_TablePrescale(AG_Table *t, int w, int nrows)
{
	if (w != -1) { t->prew = w; }
	if (nrows != -1) { t->preh = nrows*agTextFontHeight; }
}

void
AG_TableDestroy(void *p)
{
	AG_Table *t = p;
	AG_TablePopup *tp, *tpn;
	Uint m, n;
	
	for (tp = SLIST_FIRST(&t->popups);
	     tp != SLIST_END(&t->popups);
	     tp = tpn) {
		tpn = SLIST_NEXT(tp, popups);
		AG_ObjectDestroy(tp->menu);
		Free(tp->menu, M_OBJECT);
		Free(tp, M_WIDGET);
	}
	
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++)
			AG_TableFreeCell(t, &t->cells[m][n]);
	}
	for (n = 0; n < t->n; n++) {
		AG_TablePoolFree(t, n);
	}
	Free(t->cols, M_WIDGET);

	AG_MutexDestroy(&t->lock);
	AG_WidgetDestroy(t);
}

static void
AG_TableSizeFillCols(AG_Table *t)
{
	Uint n;

	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		
		if (tc->flags & AG_TABLE_COL_FILL) {
			tc->w = t->wTbl;
			for (n = 0; n < t->n; n++) {
				if ((t->cols[n].flags & AG_TABLE_COL_FILL) == 0)
					tc->w -= t->cols[n].w;
			}
			break;
		}
	}
}

void
AG_TableScale(void *p, int w, int h)
{
	AG_Table *t = p;
	int n;

	if (w == -1 && h == -1) {
		if (t->prew != -1) {
			AGWIDGET(t)->w = t->vbar->bw + 2;
			for (n = 0; n < t->n; n++) {
				AG_TableCol *tc = &t->cols[n];
			
				if (tc->flags & AG_TABLE_COL_FILL) {
					AGWIDGET(t)->w += COLUMN_MIN_WIDTH;
					continue;
				}
				AGWIDGET(t)->w += tc->w;
			}
		} else {
			AGWIDGET(t)->w = t->prew;
		}
		AGWIDGET(t)->h = t->preh;
	}

	AGWIDGET(t->vbar)->x = AGWIDGET(t)->w - t->vbar->bw;
	AGWIDGET(t->vbar)->y = 0;
	AG_WidgetScale(t->vbar, t->vbar->bw, AGWIDGET(t)->h);
	
	AGWIDGET(t->hbar)->x = t->vbar->bw + 1;
	AGWIDGET(t->hbar)->y = AGWIDGET(t)->h - t->hbar->bw;
	AG_WidgetScale(t->hbar, AGWIDGET(t)->w - t->hbar->bw, t->vbar->bw);

	t->wTbl = AGWIDGET(t)->w - AGWIDGET(t->vbar)->w + 1;

	if (w != -1 && h != -1) {
		for (n = 0; n < t->n; n++) {
			AG_TableCol *tc = &t->cols[n];
	
			if ((tc->x + tc->w) > (t->wTbl - COLUMN_MIN_WIDTH))
				tc->w = t->wTbl - tc->x - COLUMN_MIN_WIDTH;
		}
	}

	AG_TableSizeFillCols(t);
	AG_TableUpdateScrollbars(t);
}

static __inline__ void
AG_TablePrintCell(AG_Table *t, AG_TableCell *c, char *buf, size_t bufsz)
{
	switch (c->type) {
	case AG_CELL_INT:
	case AG_CELL_UINT:
		snprintf(buf, bufsz, c->fmt, c->data.i);
		break;
	case AG_CELL_LONG:
	case AG_CELL_ULONG:
		snprintf(buf, bufsz, c->fmt, c->data.l);
		break;
	case AG_CELL_PINT:
		snprintf(buf, bufsz, c->fmt, *(int *)c->data.p);
		break;
	case AG_CELL_PUINT:
		snprintf(buf, bufsz, c->fmt, *(Uint *)c->data.p);
		break;
	case AG_CELL_PLONG:
		snprintf(buf, bufsz, c->fmt, *(long *)c->data.p);
		break;
	case AG_CELL_PULONG:
		snprintf(buf, bufsz, c->fmt, *(Ulong *)c->data.p);
		break;
	case AG_CELL_FLOAT:
		snprintf(buf, bufsz, c->fmt, (float)c->data.f);
		break;
	case AG_CELL_PFLOAT:
		snprintf(buf, bufsz, c->fmt, *(float *)c->data.p);
		break;
	case AG_CELL_DOUBLE:
		snprintf(buf, bufsz, c->fmt, c->data.f);
		break;
	case AG_CELL_PDOUBLE:
		snprintf(buf, bufsz, c->fmt, *(double *)c->data.p);
		break;
#ifdef SDL_HAS_64BIT_TYPE
	case AG_CELL_INT64:
	case AG_CELL_UINT64:
		snprintf(buf, bufsz, c->fmt, c->data.f);
		break;
	case AG_CELL_PUINT64:
		snprintf(buf, bufsz, c->fmt, *(Uint64 *)c->data.p);
		break;
	case AG_CELL_PINT64:
		snprintf(buf, bufsz, c->fmt, *(Sint64 *)c->data.p);
		break;
#endif
	case AG_CELL_PUINT32:
		snprintf(buf, bufsz, c->fmt, *(Uint32 *)c->data.p);
		break;
	case AG_CELL_PSINT32:
		snprintf(buf, bufsz, c->fmt, *(Sint32 *)c->data.p);
		break;
	case AG_CELL_PUINT16:
		snprintf(buf, bufsz, c->fmt, *(Uint16 *)c->data.p);
		break;
	case AG_CELL_PSINT16:
		snprintf(buf, bufsz, c->fmt, *(Sint16 *)c->data.p);
		break;
	case AG_CELL_PUINT8:
		snprintf(buf, bufsz, c->fmt, *(Uint8 *)c->data.p);
		break;
	case AG_CELL_PSINT8:
		snprintf(buf, bufsz, c->fmt, *(Sint8 *)c->data.p);
		break;
	case AG_CELL_STRING:
		strlcpy(buf, c->data.s, bufsz);
		break;
	case AG_CELL_PSTRING:
		strlcpy(buf, (char *)c->data.p, bufsz);
		break;
	case AG_CELL_FN_TXT:
		c->fnTxt(c->data.p, buf, bufsz);
		break;
	case AG_CELL_FN_SU:
		strlcpy(buf, "<image>", bufsz);
		break;
	case AG_CELL_POINTER:
		snprintf(buf, bufsz, c->fmt, c->data.p);
		break;
	case AG_CELL_NULL:
		if (c->fmt[0] == '\0') {
			strlcpy(buf, "<null>", bufsz);
		} else {
			strlcpy(buf, c->fmt, bufsz);
		}
		break;
	}
}

static __inline__ void
AG_TableDrawCell(AG_Table *t, AG_TableCell *c, SDL_Rect *rd)
{
	char txt[AG_TABLE_TXT_MAX];

	if (c->selected) {		     		 /* TODO col sel */
		Uint8 c[4] = { 0, 0, 250, 64 };

		agPrim.rect_blended(t, rd->x, rd->y, rd->w-1, rd->h-1,
		    c, AG_ALPHA_SRC);
	}

	if (c->surface >= 0) {
		if (t->flags & AG_TABLE_REDRAW_CELLS) {
			AG_WidgetUnmapSurface(t, c->surface);
		} else {
			goto blit;
		}
	}

	switch (c->type) {
	case AG_CELL_STRING:					/* Avoid copy */
		c->surface = AG_WidgetMapSurface(t,
		    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), c->data.s));
		goto blit;
	case AG_CELL_PSTRING:					/* Avoid copy */
		c->surface = AG_WidgetMapSurface(t,
		    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR),
		    (char *)c->data.p));
		goto blit;
	case AG_CELL_FN_SU:
		c->surface = AG_WidgetMapSurface(t,
		    c->fnSu(c->data.p, rd->x, rd->y));
		goto blit;
	case AG_CELL_NULL:
		if (c->fmt[0] != '\0') {
			c->surface = AG_WidgetMapSurface(t,
			    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR),
			    c->fmt));
			goto blit;
		} else {
			return;
		}
		break;
	default:
		AG_TablePrintCell(t, c, txt, sizeof(txt));
		break;
	}
	c->surface = AG_WidgetMapSurface(t,
	    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), txt));
blit:
	AG_WidgetBlitSurface(t, c->surface,
	    rd->x,
	    rd->y + (t->row_h>>1) - (AGWIDGET_SURFACE(t,c->surface)->h>>1));
}

void
AG_TableDraw(void *p)
{
	AG_Table *t = p;
	int n, m;
	int x, y;

	if (AGWIDGET(t)->w <= t->vbar->bw || AGWIDGET(t)->h <= t->row_h)
		return;

	agPrim.box(t, 0, 0, t->wTbl, AGWIDGET(t)->h, -1, AG_COLOR(TABLE_COLOR));

	AG_MutexLock(&t->lock);
	t->moffs = AG_WidgetInt(t->vbar, "value");

	if (t->poll_ev != NULL) {
		t->poll_ev->handler(t->poll_ev);
	}

	for (n = 0, x = t->xoffs;
	     n < t->n && x < t->wTbl;
	     n++) {
		AG_TableCol *col = &t->cols[n];
		int cw;
	
		if (col->w <= 0) {
			continue;
		}
		cw = ((x+col->w) < t->wTbl) ? col->w: t->wTbl - x;

		/* Draw the column header and separator. */
		if (x > 0 && x < t->wTbl) {
			agPrim.vline(t, x-1, t->col_h-1, AGWIDGET(t)->h,
			    AG_COLOR(TABLE_LINE_COLOR));
		}
		agPrim.box(t, x, 0, cw, t->col_h - 1, 1, AG_COLOR(TABLE_COLOR));
		
		AG_WidgetPushClipRect(t, x, 0, cw, AGWIDGET(t)->h);
		if (col->surface != -1) {
			AG_WidgetBlitSurface(t, col->surface,
			    x + cw/2 - AGWIDGET_SURFACE(t,col->surface)->w/2,
			    0);
		}

		/* Draw the rows of this column. */
		for (m = t->moffs, y = t->row_h;
		     m < t->m && y < AGWIDGET(t)->h;
		     m++) {
			SDL_Rect rCell;
			
			agPrim.hline(t, 0, t->wTbl, y,
			    AG_COLOR(TABLE_LINE_COLOR));

			rCell.x = x;
			rCell.y = y;
			rCell.w = col->w;
			rCell.h = t->row_h;
			AG_TableDrawCell(t, &t->cells[m][n], &rCell);
			y += t->row_h;
		}
		agPrim.hline(t, 0, t->wTbl, y,
		    AG_COLOR(TABLE_LINE_COLOR));

		/* Indicate column selection. */
		if (col->selected) {
			Uint8 c[4] = { 0, 0, 250, 32 };

			agPrim.rect_blended(t, x, 0, col->w, AGWIDGET(t)->h,
			    c, AG_ALPHA_SRC);
		}
		AG_WidgetPopClipRect(t);
		x += col->w;
	}
	agPrim.vline(t, x-1, t->col_h-1, AGWIDGET(t)->h,
	    AG_COLOR(TABLE_LINE_COLOR));

	t->flags &= ~(AG_TABLE_REDRAW_CELLS);
	AG_MutexUnlock(&t->lock);
}

/*
 * Adjust the scrollbar offset according to the number of visible items,
 * and scroll if requested.
 */
void
AG_TableUpdateScrollbars(AG_Table *t)
{
	AG_WidgetBinding *maxb, *offsetb;
	int *max, *offset;

	t->mVis = AGWIDGET(t)->h / t->row_h;

	maxb = AG_WidgetGetBinding(t->vbar, "max", &max);
	offsetb = AG_WidgetGetBinding(t->vbar, "value", &offset);
	if ((*max = LAST_VISIBLE(t)) < 0) {
		*max = 0;
	}
	if (*offset > *max) {
		*offset = *max;
		AG_WidgetBindingChanged(offsetb);
	} else if (*offset < 0) {
		*offset = 0;
		AG_WidgetBindingChanged(offsetb);
	}
	if (t->m > 0 && t->mVis > 0 && (t->mVis-1) < t->m) {
		AG_ScrollbarSetBarSize(t->vbar,
		    (t->mVis-1)*(AGWIDGET(t->vbar)->h - t->vbar->bw*2) / t->m);
	} else {
		AG_ScrollbarSetBarSize(t->vbar, -1);		/* Full range */
	}
	AG_WidgetUnlockBinding(offsetb);
	AG_WidgetUnlockBinding(maxb);
}

AG_MenuItem *
AG_TableSetPopup(AG_Table *t, int m, int n)
{
	AG_TablePopup *tp;

	SLIST_FOREACH(tp, &t->popups, popups) {
		if (tp->m == m && tp->n == n) {
			AG_MenuFreeSubItems(tp->item);
			return (tp->item);
		}
	}
	tp = Malloc(sizeof(AG_TablePopup), M_WIDGET);
	tp->m = m;
	tp->n = n;
	tp->panel = NULL;
	tp->menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(tp->menu, 0);
	tp->item = AG_MenuAddItem(tp->menu, "");
	SLIST_INSERT_HEAD(&t->popups, tp, popups);
	return (tp->item);
}

void
AG_TableSetRowDblClickFn(AG_Table *tbl, AG_EventFn ev, const char *fmt, ...)
{
	AG_MutexLock(&tbl->lock);
	tbl->dblClickRowEv = AG_SetEvent(tbl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tbl->dblClickRowEv, fmt);
	AG_MutexUnlock(&tbl->lock);
}

void
AG_TableSetColDblClickFn(AG_Table *tbl, AG_EventFn ev, const char *fmt, ...)
{
	AG_MutexLock(&tbl->lock);
	tbl->dblClickColEv = AG_SetEvent(tbl, NULL, ev, NULL);
	AG_EVENT_GET_ARGS(tbl->dblClickColEv, fmt);
	AG_MutexUnlock(&tbl->lock);
}

void
AG_TableRedrawCells(AG_Table *t)
{
	AG_MutexLock(&t->lock);
	t->flags |= AG_TABLE_REDRAW_CELLS;
	AG_MutexUnlock(&t->lock);
}

void
AG_TableFreeCell(AG_Table *t, AG_TableCell *c)
{
	if (c->surface >= 0)
		AG_WidgetUnmapSurface(t, c->surface);
}

int
AG_TablePoolAdd(AG_Table *t, Uint m, Uint n)
{
	AG_TableCol *tc = &t->cols[n];
	AG_TableCell *c1 = &t->cells[m][n], *c2;
	
	tc->pool = Realloc(tc->pool, (tc->mpool+1)*sizeof(AG_TableCell));
	memcpy(&tc->pool[tc->mpool], &t->cells[m][n], sizeof(AG_TableCell));
	return (tc->mpool++);
}

void
AG_TablePoolFree(AG_Table *t, Uint n)
{
	AG_TableCol *tc = &t->cols[n];
	Uint m;

	for (m = 0; m < tc->mpool; m++) {
		AG_TableFreeCell(t, &tc->pool[m]);
	}
	Free(tc->pool, M_WIDGET);
	tc->pool = NULL;
	tc->mpool = 0;
}

/* Clear the items on the table and save the selection state. */
void
AG_TableBegin(AG_Table *t)
{
	Uint m, n;

	AG_MutexLock(&t->lock);
	/* Copy the existing cells to the column pools and free the table. */
	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++) {
			AG_TablePoolAdd(t, m, n);
		}
		Free(t->cells[m], M_WIDGET);
	}
	Free(t->cells, M_WIDGET);
	t->cells = NULL;
	t->m = 0;
	AG_WidgetSetInt(t->vbar, "max", 0);
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
#ifdef SDL_HAS_64BIT_TYPE
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
	case AG_CELL_NULL:
		return (0);
	}
	return (1);
}

/*
 * Restore the selection state and recuperate the surfaces of matching
 * items in the column pool.
 */
void
AG_TableEnd(AG_Table *t)
{
	Uint n, m, i;

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
		Free(tc->pool, M_WIDGET);
		tc->pool = NULL;
		tc->mpool = 0;
	}
	AG_TableUpdateScrollbars(t);
	AG_MutexUnlock(&t->lock);
}

static __inline__ int
SelectingMultiple(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTITOGGLE) ||
	        ((t->flags & AG_TABLE_MULTI) && SDL_GetModState() & KMOD_CTRL));
}

static __inline__ int
SelectingRange(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTI) &&
	        (SDL_GetModState() & KMOD_SHIFT));
}

static void
ShowPopup(AG_Table *tbl, AG_TablePopup *tp)
{
	int x, y;

	AG_MouseGetState(&x, &y);
	if (tp->panel != NULL) {
		AG_MenuCollapse(tp->menu, tp->item);
		tp->panel = NULL;
	}
	tp->menu->sel_item = tp->item;
	tp->panel = AG_MenuExpand(tp->menu, tp->item, x+4, y+4);
}

static void
ColumnRightClick(AG_Table *t, int px)
{
	Uint n;
	int cx;
	int x = px - (COLUMN_RESIZE_RANGE/2);

	for (n = 0, cx = t->xoffs; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		int x2 = cx+tc->w;
		AG_TablePopup *tp;

		if (x > cx && x < x2) {
			SLIST_FOREACH(tp, &t->popups, popups) {
				if (tp->m == -1 && tp->n == n) {
					ShowPopup(t, tp);
					return;
				}
			}
		}
		cx += tc->w;
	}
}

static void
ColumnLeftClick(AG_Table *tbl, int px)
{
	AG_TableCell *c;
	int x = px - (COLUMN_RESIZE_RANGE/2), x1;
	int multi = SelectingMultiple(tbl);
	Uint n;

	for (n = 0, x1 = tbl->xoffs; n < tbl->n; n++) {
		AG_TableCol *tc = &tbl->cols[n];
		int x2 = x1+tc->w;

		if (x > x1 && x < x2) {
			if ((x2 - x) < COLUMN_RESIZE_RANGE) {
				if (tbl->nResizing == -1) {
					dprintf("now resizing col%d\n", n);
					tbl->nResizing = n;
				}
			} else {
				if (multi) {
					tc->selected = !tc->selected;
				} else {
					tc->selected = 1;
				}
				if (tbl->dblClickedCol != -1 &&
				    tbl->dblClickedCol == n) {
					AG_CancelEvent(tbl,
					    "dblclick-col-expire");
					if (tbl->dblClickColEv != NULL) {
						AG_PostEvent(NULL, tbl,
						    tbl->dblClickColEv->name,
						    "%i", n);
					}
					AG_PostEvent(NULL, tbl,
					    "table-dblclick-col", "%i", n);
					tbl->dblClickedCol = -1;
				} else {
					tbl->dblClickedCol = n;
					AG_SchedEvent(NULL, tbl,
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

static void
CellLeftClick(AG_Table *tbl, int mc, int x)
{
	AG_TableCell *c;
	Uint m, n, i;

	if (SelectingRange(tbl)) {
		for (m = 0; m < tbl->m; m++) {
			if (AG_TableRowSelected(tbl, m))
				break;
		}
		if (m < tbl->m) {
			if (m < mc) {
				for (i = m; i <= mc; i++)
					AG_TableSelectRow(tbl, i);
			} else if (m > mc) {
				for (i = mc; i <= m; i++)
					AG_TableSelectRow(tbl, i);
			} else {
				AG_TableSelectRow(tbl, mc);
			}
		}
	} else if (SelectingMultiple(tbl)) {
		for (n = 0; n < tbl->n; n++) {
			c = &tbl->cells[mc][n];
			c->selected = !c->selected;
			tbl->cols[n].selected = 0;
		}
	} else {
		for (m = 0; m < tbl->m; m++) {
			for (n = 0; n < tbl->n; n++) {
				c = &tbl->cells[m][n];
				c->selected = ((int)m == mc);
				tbl->cols[n].selected = 0;
			}
		}
		if (tbl->dblClickedRow != -1 && tbl->dblClickedRow == mc) {
			AG_CancelEvent(tbl, "dblclick-row-expire");
			if (tbl->dblClickRowEv != NULL) {
				AG_PostEvent(NULL, tbl,
				    tbl->dblClickRowEv->name,
				    "%i", mc);
			}
			AG_PostEvent(NULL, tbl, "table-dblclick-row", "%i", mc);
			tbl->dblClickedRow = -1;
		} else {
			tbl->dblClickedRow = mc;
			AG_SchedEvent(NULL, tbl, agMouseDblclickDelay,
			    "dblclick-row-expire", NULL);
		}
	}
}

static void
CellRightClick(AG_Table *t, int m, int px)
{
	AG_TablePopup *tp;
	int x = px - (COLUMN_RESIZE_RANGE/2), cx;
	Uint n;

	for (n = 0, cx = t->xoffs; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		int x2 = cx+tc->w;
		AG_TablePopup *tp;

		if (x > cx && x < x2) {
			SLIST_FOREACH(tp, &t->popups, popups) {
				if ((tp->m == m || tp->m == -1) &&
				    (tp->n == n || tp->n == -1)) {
					ShowPopup(t, tp);
					return;
				}
			}
		}
		cx += tc->w;
	}
}

static __inline__ int
OverColumn(AG_Table *t, int y)
{
	if (y <= t->row_h) {
		return (-1);
	}
	return (AG_WidgetInt(t->vbar, "value") + y/t->row_h - 1);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	int m;

	AG_MutexLock(&t->lock);
	switch (button) {
	case SDL_BUTTON_WHEELUP:
		{
			static Uint32 t1 = 0;
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(t->vbar, "value", &offs);
			if (((*offs) -= AG_WidgetScrollDelta(&t1)) < 0) {
				*offs = 0;
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		{
			static Uint32 t1 = 0;
			AG_WidgetBinding *offsb;
			int *offs;

			offsb = AG_WidgetGetBinding(t->vbar, "value", &offs);
			if (((*offs) += AG_WidgetScrollDelta(&t1)) >
			    LAST_VISIBLE(t)) {
				*offs = LAST_VISIBLE(t);
			}
			AG_WidgetBindingChanged(offsb);
			AG_WidgetUnlockBinding(offsb);
		}
		break;
	case SDL_BUTTON_LEFT:
		if ((m = OverColumn(t, y)) >= (int)t->m) {
			AG_TableDeselectAllRows(t);
			goto out;
		}
		if (m < 0) {
			ColumnLeftClick(t, x);
		} else {
			CellLeftClick(t, m, x);
		}
		break;
	case SDL_BUTTON_RIGHT:
		if ((m = OverColumn(t, y)) >= (int)t->m) {
			goto out;
		}
		if (m < 0) {
			ColumnRightClick(t, x);
		} else {
			CellRightClick(t, m, x);
		}
		break;
	default:
		break;
	}
out:
	AG_WidgetFocus(t);
	AG_MutexUnlock(&t->lock);
}

static void
mousebuttonup(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);
	Uint m, n;

	switch (button) {
	case SDL_BUTTON_LEFT:
		if (t->nResizing >= 0) {
			dprintf("no longer resizing col%d\n", t->nResizing);
			t->nResizing = -1;
		}
		break;
	}
}

static void
keydown(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&t->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		AG_SchedEvent(NULL, t, agKbdDelay, "kbdscroll", "%i", keysym);
		AG_ExecEvent(t, "kbdscroll");
		break;
	default:
		break;
	}
	AG_MutexUnlock(&t->lock);
}

static int
OverColumnResizeControl(AG_Table *t, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2);
	Uint n;
	int cx;

	if (px > t->wTbl) {
		return (0);
	}
	for (n = 0, cx = t->xoffs; n < t->n; n++) {
		int x2 = cx + t->cols[n].w;

		if (x > cx && x < x2) {
			if ((x2 - x) < COLUMN_RESIZE_RANGE)
				return (1);
		}
		cx += t->cols[n].w;
	}
	return (0);
}

static void
mousemotion(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int xrel = AG_INT(3);
	int yrel = AG_INT(4);
	int n, cx;
	int m;

	AG_MutexLock(&t->lock);
	if (t->nResizing >= 0 && (Uint)t->nResizing < t->n) {
		AG_TableCol *tc = &t->cols[t->nResizing];

		if ((tc->w += xrel) < COLUMN_MIN_WIDTH) {
			tc->w = COLUMN_MIN_WIDTH;
		}
		if ((tc->x + tc->w) > (t->wTbl - COLUMN_MIN_WIDTH)) {
			tc->w = t->wTbl - tc->x - COLUMN_MIN_WIDTH;
		}
		AG_TableSizeFillCols(t);
		AG_SetCursor(AG_HRESIZE_CURSOR);
	} else {
		if ((m = OverColumn(t, y)) == -1 &&
		    OverColumnResizeControl(t, x))
			AG_SetCursor(AG_HRESIZE_CURSOR);
	}
	AG_MutexUnlock(&t->lock);
}

static void
keyup(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&t->lock);
	switch (keysym) {
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
		AG_CancelEvent(t, "kbdscroll");
		break;
	default:
		break;
	}
	AG_MutexUnlock(&t->lock);
}

static void
lostfocus(AG_Event *event)
{
	AG_Table *t = AG_SELF();

	AG_MutexLock(&t->lock);
	AG_CancelEvent(t, "key-tick");
	AG_CancelEvent(t, "dblclick-row-expire");
	AG_CancelEvent(t, "dblclick-col-expire");
	if (t->nResizing >= 0) {
		dprintf("lost focus: no longer resizing col%d\n", t->nResizing);
		t->nResizing = -1;
	}
	AG_MutexUnlock(&t->lock);
}

int
AG_TableRowSelected(AG_Table *t, Uint m)
{
	Uint n;

	for (n = 0; n < t->n; n++) {
		if (t->cells[m][n].selected)
			return (1);
	}
	return (0);
}

void
AG_TableSelectRow(AG_Table *t, Uint m)
{
	Uint n;

	for (n = 0; n < t->n; n++)
		t->cells[m][n].selected = 1;
}

void
AG_TableDeselectRow(AG_Table *t, Uint m)
{
	Uint n;

	for (n = 0; n < t->n; n++)
		t->cells[m][n].selected = 0;
}

void
AG_TableSelectAllRows(AG_Table *t)
{
	Uint m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 1;
	}
}

void
AG_TableDeselectAllRows(AG_Table *t)
{
	Uint m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 0;
	}
}

void
AG_TableSelectAllCols(AG_Table *t)
{
	Uint n;

	for (n = 0; n < t->n; n++)
		t->cols[n].selected = 1;
}

void
AG_TableDeselectAllCols(AG_Table *t)
{
	Uint n;

	for (n = 0; n < t->n; n++)
		t->cols[n].selected = 0;
}

static void
kbdscroll(AG_Event *event)
{
	AG_Table *t = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	Uint m2, n;
	int m;
	
	AG_MutexLock(&t->lock);
	switch (keysym) {
	case SDLK_UP:
		for (m = 0; m < t->m; m++) {
			if (AG_TableRowSelected(t, m)) {
				m--;
				break;
			}
		}
		AG_TableDeselectAllRows(t);
		AG_TableSelectRow(t, (m >= 0) ? m : 0);
		break;
	case SDLK_DOWN:
		for (m = t->m-1; m >= 0; m--) {
			if (AG_TableRowSelected(t, m)) {
				m++;
				break;
			}
		}
		AG_TableDeselectAllRows(t);
		AG_TableSelectRow(t, (m < t->m) ? m : t->m - 1);
		break;
	default:
		break;
	}
	AG_ReschedEvent(t, "kbdscroll", agKbdRepeat);
	AG_MutexUnlock(&t->lock);
}

static void
dblclick_row_expire(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();

	AG_MutexLock(&tbl->lock);
	tbl->dblClickedRow = -1;
	AG_MutexUnlock(&tbl->lock);
}

static void
dblclick_col_expire(AG_Event *event)
{
	AG_Table *tbl = AG_SELF();

	AG_MutexLock(&tbl->lock);
	tbl->dblClickedCol = -1;
	AG_MutexUnlock(&tbl->lock);
}

int
AG_TableAddCol(AG_Table *t, const char *name, const char *size_spec,
    int (*sort_fn)(const void *, const void *))
{
	AG_TableCol *tc, *lc;
	AG_TableCell *c;
	Uint m, n;

	AG_MutexLock(&t->lock);

	/* Initialize the column information structure. */
	t->cols = Realloc(t->cols, (t->n+1)*sizeof(AG_TableCol));
	tc = &t->cols[t->n];
	if (name != NULL) {
		strlcpy(tc->name, name, sizeof(tc->name));
	} else {
		tc->name[0] = '\0';
	}
	tc->flags = AG_TABLE_SORT_ASCENDING;
	tc->sort_fn = sort_fn;
	tc->selected = 0;
	tc->surface = (name != NULL) ? AG_WidgetMapSurface(t,
	    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), name)) : -1;
	if (t->n > 0) {
		lc = &t->cols[t->n - 1];
		tc->x = lc->x+lc->w;
	} else {
		tc->x = 0;
	}
	tc->pool = NULL;
	tc->mpool = 0;

	if (size_spec != NULL) {
		switch (AG_WidgetParseSizeSpec(size_spec, &tc->w)) {
		case AG_WIDGET_PERCENT:
			tc->w = tc->w*(AGWIDGET(t)->w - t->vbar->bw)/100;
			break;
		default:
			break;
		}
	} else {
		if (name != NULL) {
			tc->flags |= AG_TABLE_COL_FILL;
		} else {
			tc->w = 0;
		}
	}

	/* Resize the row arrays. */
	for (m = 0; m < t->m; m++) {
		t->cells[m] = Realloc(t->cells[m],
		    (t->n+1)*sizeof(AG_TableCell));
		AG_TableInitCell(t, &t->cells[m][t->n]);
	}
	n = t->n++;
	AG_MutexUnlock(&t->lock);
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
}

int
AG_TableAddRow(AG_Table *t, const char *fmtp, ...)
{
	char fmt[64], *sp = &fmt[0];
	va_list ap;
	Uint n;

	strlcpy(fmt, fmtp, sizeof(fmt));

	va_start(ap, fmtp);
	t->cells = Realloc(t->cells, (t->m+1)*sizeof(AG_TableCell));
	t->cells[t->m] = Malloc(t->n*sizeof(AG_TableCell), M_WIDGET);
	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		AG_TableCell *c = &t->cells[t->m][n];
		char *s = AG_Strsep(&sp, ":"), *sc;
		int ptr = 0, lflag = 0, ptr_long = 0;
		int infmt = 0;

		AG_TableInitCell(t, c);
		strlcpy(c->fmt, s, sizeof(c->fmt));
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
			}
		}
		switch (sc[0]) {
		case 's':
			if (ptr) {
				c->type = AG_CELL_PSTRING;
			} else {
				c->type = AG_CELL_STRING;
				strlcpy(c->data.s, va_arg(ap, char *),
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
#ifdef SDL_HAS_64BIT_TYPE
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
#ifdef SDL_HAS_64BIT_TYPE
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
	return (t->m++);
}

int
AG_TableSaveASCII(AG_Table *t, FILE *f, char sep)
{
	char txt[AG_TABLE_TXT_MAX];
	Uint m, n;

	AG_MutexLock(&t->lock);
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
			AG_TablePrintCell(t, &t->cells[m][n], txt, sizeof(txt));
			fputs(txt, f);
			fputc(sep, f);
		}
		fputc('\n', f);
	}
	AG_MutexUnlock(&t->lock);
	return (0);
}

