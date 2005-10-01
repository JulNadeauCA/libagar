/*	$Csoft: table.c,v 1.1 2005/10/01 09:50:49 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>

#include "table.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/scrollbar.h>
#include <engine/widget/label.h>
#include <engine/widget/cursors.h>

#include <string.h>
#include <stdarg.h>

static AG_WidgetOps agTableOps = {
	{
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

static void mousebuttondown(int, union evarg *);
static void mousebuttonup(int, union evarg *);
static void mousemotion(int, union evarg *);
static void keydown(int, union evarg *);
static void keyup(int, union evarg *);
static void lostfocus(int, union evarg *);
static void kbdscroll(int, union evarg *);

#define COLUMN_RESIZE_RANGE	10	/* Range in pixels for resize ctrls */
#define COLUMN_MIN_WIDTH	20	/* Minimum column width in pixels */

AG_Table *
AG_TablePolled(void *parent, u_int flags, void (*fn)(int, union evarg *),
    const char *fmt, ...)
{
	AG_Table *t;
	va_list ap;

	t = Malloc(sizeof(AG_Table), M_OBJECT);
	AG_TableInit(t, flags);
	t->poll_ev = AG_SetEvent(t, "table-poll", fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, t->poll_ev);
		}
		va_end(ap);
	}
	
	AG_ObjectAttach(parent, t);
	return (t);
}

void
AG_TableInit(AG_Table *t, u_int flags)
{
	AG_WidgetInit(t, "table", &agTableOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL|AG_WIDGET_HFILL|
	    AG_WIDGET_UNFOCUSED_MOTION);
	AG_WidgetBind(t, "selected-row", AG_WIDGET_POINTER, &t->selected_row);
	AG_WidgetBind(t, "selected-col", AG_WIDGET_POINTER, &t->selected_col);
	AG_WidgetBind(t, "selected-cell", AG_WIDGET_POINTER, &t->selected_cell);

	pthread_mutex_init(&t->lock, &agRecursiveMutexAttr);
	t->flags = flags;
	t->selected_row = NULL;
	t->selected_col = NULL;
	t->selected_cell = NULL;
	t->row_h = agTextFontHeight+2;
	t->col_h = agTextFontHeight+4;
	t->prew = 128;
	t->preh = 64;
	t->vbar = AG_ScrollbarNew(t, AG_SCROLLBAR_VERT);
	t->hbar = AG_ScrollbarNew(t, AG_SCROLLBAR_HORIZ);
	t->poll_ev = NULL;
	t->nResizing = -1;

	t->cols = NULL;
	t->cells = NULL;
	t->n = 0;
	t->m = 0;
	t->mVis = 0;
	t->poll_ev = NULL;
	t->xoffs = 0;
	
	AG_SetEvent(t, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(t, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(t, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(t, "window-keydown", keydown, NULL);
	AG_SetEvent(t, "window-keyup", keyup, NULL);
	AG_SetEvent(t, "widget-lostfocus", lostfocus, NULL);
	AG_SetEvent(t, "widget-hidden", lostfocus, NULL);
//	AG_SetEvent(t, "detached", lostfocus, NULL);
	AG_SetEvent(t, "kbdscroll", kbdscroll, NULL);
}

void
AG_TableDestroy(void *p)
{
	AG_Table *t = p;
	u_int m, n;

	for (m = 0; m < t->m; m++) {
		for (n = 0; n < t->n; n++)
			AG_TableFreeCell(t, &t->cells[m][n]);
	}
	for (n = 0; n < t->n; n++) {
		AG_TablePoolFree(t, n);
	}
	Free(t->cols, M_WIDGET);

	pthread_mutex_destroy(&t->lock);
	AG_WidgetDestroy(t);
}

void
AG_TableScale(void *p, int w, int h)
{
	AG_Table *t = p;

	if (w == -1 && h == -1) {
		AGWIDGET(t)->w = t->prew;
		AGWIDGET(t)->h = t->preh;
	}

	AGWIDGET(t->vbar)->x = AGWIDGET(t)->w - t->vbar->button_size;
	AGWIDGET(t->vbar)->y = 0;
	AG_WidgetScale(t->vbar,
	     t->vbar->button_size,
	     AGWIDGET(t)->h);
	
	AGWIDGET(t->hbar)->x = t->vbar->button_size + 1;
	AGWIDGET(t->hbar)->y = AGWIDGET(t)->h - t->hbar->button_size;
	AG_WidgetScale(t->hbar,
	    AGWIDGET(t)->w - t->hbar->button_size,
	    t->vbar->button_size);

	AG_TableUpdateScrollbars(t);
}

static __inline__ void
AG_TableDrawCell(AG_Table *t, AG_TableCell *c, SDL_Rect *rd)
{
	char txt[AG_LABEL_MAX];

	if (c->selected) {		     		 /* TODO col sel */
		Uint8 c[4] = { 0, 0, 250, 64 };

		agPrim.rect_blended(t, rd->x+1, rd->y+1, rd->w-1, rd->h-1,
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
	case AG_CELL_STRING:
		c->surface = AG_WidgetMapSurface(t,
		    AG_TextRender(NULL, -1, AG_COLOR(TLIST_TXT_COLOR),
		    c->data.s));
		goto blit;
	case AG_CELL_PSTRING:
		c->surface = AG_WidgetMapSurface(t,
		    AG_TextRender(NULL, -1, AG_COLOR(TLIST_TXT_COLOR),
		    (char *)c->data.p));
		goto blit;
	case AG_CELL_INT:
	case AG_CELL_UINT:
		snprintf(txt, sizeof(txt), c->fmt, c->data.i);
		break;
	case AG_CELL_PINT:
	case AG_CELL_PUINT:
		snprintf(txt, sizeof(txt), c->fmt, *(int *)c->data.p);
		break;
	case AG_CELL_FLOAT:
		snprintf(txt, sizeof(txt), c->fmt, (float)c->data.f);
		break;
	case AG_CELL_PFLOAT:
		snprintf(txt, sizeof(txt), c->fmt, *(float *)c->data.p);
		break;
	case AG_CELL_DOUBLE:
		snprintf(txt, sizeof(txt), c->fmt, c->data.f);
		break;
	case AG_CELL_PDOUBLE:
		snprintf(txt, sizeof(txt), c->fmt, *(double *)c->data.p);
		break;
	case AG_CELL_PUINT32:
		snprintf(txt, sizeof(txt), c->fmt, *(Uint32 *)c->data.p);
		break;
	case AG_CELL_PSINT32:
		snprintf(txt, sizeof(txt), c->fmt, *(Sint32 *)c->data.p);
		break;
	case AG_CELL_PUINT16:
		snprintf(txt, sizeof(txt), c->fmt, *(Uint16 *)c->data.p);
		break;
	case AG_CELL_PSINT16:
		snprintf(txt, sizeof(txt), c->fmt, *(Sint16 *)c->data.p);
		break;
	case AG_CELL_PUINT8:
		snprintf(txt, sizeof(txt), c->fmt, *(Uint8 *)c->data.p);
		break;
	case AG_CELL_PSINT8:
		snprintf(txt, sizeof(txt), c->fmt, *(Sint8 *)c->data.p);
		break;
	case AG_CELL_FN:
		c->surface = AG_WidgetMapSurface(t,
		    c->fn(c->data.p, rd->x, rd->y));
		goto blit;
	case AG_CELL_NULL:
		return;
	default:
		break;
	}
	c->surface = AG_WidgetMapSurface(t,
	    AG_TextRender(NULL, -1, AG_COLOR(TLIST_TXT_COLOR), txt));
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

	if (AGWIDGET(t)->w <= t->vbar->button_size ||
	    AGWIDGET(t)->h <= t->row_h)
		return;

	agPrim.box(t, 0, 0, AGWIDGET(t)->w, AGWIDGET(t)->h, -1,
	    AG_COLOR(TLIST_BG_COLOR));

	pthread_mutex_lock(&t->lock);
	t->moffs = AG_WidgetInt(t->vbar, "value");
	
	if (t->poll_ev != NULL)
		t->poll_ev->handler(t->poll_ev->argc, t->poll_ev->argv);

	for (n = 0, x = t->xoffs;
	     n < t->n && x < AGWIDGET(t)->w;
	     n++) {
		AG_TableCol *col = &t->cols[n];
		int cw;
		
		cw = (x+col->w) < AGWIDGET(t)->w ? col->w: AGWIDGET(t)->w - x;

		/* Draw the column header and separator. */
		if (x > 0 && x < AGWIDGET(t)->w) {
			agPrim.vline(t, x-1, t->col_h-1, AGWIDGET(t)->h,
			    AG_COLOR(TLIST_LINE_COLOR));
		}
		agPrim.box(t, x, 0, cw, t->col_h - 1, 1,
		    AG_COLOR(TLIST_BG_COLOR));
		
		AG_WidgetPushClipRect(t, x, 0, cw, AGWIDGET(t)->h);
		AG_WidgetBlitSurface(t, col->surface,
		    x + cw/2 - AGWIDGET_SURFACE(t,col->surface)->w/2,
		    0);

		/* Draw the rows of this column. */
		for (m = t->moffs, y = t->row_h;
		     m < t->m && y < AGWIDGET(t)->h;
		     m++) {
			SDL_Rect rCell;
			
			agPrim.hline(t, 0, AGWIDGET(t)->w, y,
			    AG_COLOR(TLIST_LINE_COLOR));

			rCell.x = x;
			rCell.y = y;
			rCell.w = col->w;
			rCell.h = t->row_h;
			AG_TableDrawCell(t, &t->cells[m][n], &rCell);
			y += t->row_h;
		}

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
	    AG_COLOR(TLIST_LINE_COLOR));

	t->flags &= ~(AG_TABLE_REDRAW_CELLS);
	pthread_mutex_unlock(&t->lock);
}

/* Adjust the scrollbar offset according to the number of visible items. */
void
AG_TableUpdateScrollbars(AG_Table *t)
{
	AG_WidgetBinding *maxb, *offsetb;
	int *max, *offset;
	int noffset;
	
	t->mVis = AGWIDGET(t)->h / t->row_h;

	maxb = AG_WidgetGetBinding(t->vbar, "max", &max);
	offsetb = AG_WidgetGetBinding(t->vbar, "value", &offset);
	noffset = *offset;
	*max = t->m - t->mVis;
	if (noffset > *max) { noffset = *max; }
	if (noffset < 0) { noffset = 0; }
	if (*offset != noffset) {
		*offset = noffset;
		AG_WidgetBindingChanged(offsetb);
	}
	if (t->m > 0 && t->mVis > 0 && t->mVis < t->m) {
		AG_ScrollbarSetBarSize(t->vbar,
		    t->mVis*(AGWIDGET(t->vbar)->h - t->vbar->button_size*2) /
		    t->m);
	} else {
		AG_ScrollbarSetBarSize(t->vbar, -1);		/* Full range */
	}
	AG_WidgetUnlockBinding(offsetb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_TableRedrawCells(AG_Table *t)
{
	t->flags |= AG_TABLE_REDRAW_CELLS;
}

void
AG_TableFreeCell(AG_Table *t, AG_TableCell *c)
{
	if (c->surface >= 0)
		AG_WidgetUnmapSurface(t, c->surface);
}

int
AG_TablePoolAdd(AG_Table *t, u_int m, u_int n)
{
	AG_TableCol *tc = &t->cols[n];
	AG_TableCell *c1 = &t->cells[m][n], *c2;
	
	tc->pool = Realloc(tc->pool, (tc->mpool+1)*sizeof(AG_TableCell));
	memcpy(&tc->pool[tc->mpool], &t->cells[m][n], sizeof(AG_TableCell));
	return (tc->mpool++);
}

void
AG_TablePoolFree(AG_Table *t, u_int n)
{
	AG_TableCol *tc = &t->cols[n];
	u_int m;

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
	u_int m, n;

	pthread_mutex_lock(&t->lock);

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
	pthread_mutex_unlock(&t->lock);
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
	case AG_CELL_FLOAT:
	case AG_CELL_DOUBLE:
		return (c1->data.f - c2->data.f);
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
	case AG_CELL_FN:
		return ((c1->data.p != c2->data.p) || (c1->fn != c2->fn));
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
	u_int n, m, i;

	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];

		for (m = 0; m < tc->mpool; m++) {
			AG_TableCell *cDst = &t->cells[m][n];
			AG_TableCell *cPool = &tc->pool[m];

			if (AG_TableCompareCells(cDst, cPool) == 0) {
				cDst->surface = cPool->surface;
				cDst->selected = cPool->selected;
				cPool->surface = -1;
			}
			AG_TableFreeCell(t, cPool);
		}
		Free(tc->pool, M_WIDGET);
		tc->pool = NULL;
		tc->mpool = 0;
	}
	AG_TableUpdateScrollbars(t);
}

static __inline__ int
multisel(AG_Table *t)
{
	return ((t->flags & AG_TABLE_MULTITOGGLE) ||
	        ((t->flags & AG_TABLE_MULTI) && SDL_GetModState() & KMOD_CTRL));
}

static void
column_popup(AG_Table *t, int x)
{
}

static void
column_sel(AG_Table *t, int px)
{
	AG_TableCell *c;
	u_int n;
	int cx;
	int x = px - (COLUMN_RESIZE_RANGE/2);

	for (n = 0, cx = t->xoffs; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		int x2 = cx+tc->w;

		if (x > cx && x < x2) {
			if ((x2 - x) < COLUMN_RESIZE_RANGE) {
				if (t->nResizing == -1)
					t->nResizing = n;
			} else {
				if (multisel(t)) {
					tc->selected = !tc->selected;
				} else {
					tc->selected = 1;
				}
				goto cont;
			}
		}
		if (!(t->flags & AG_TABLE_MULTIMODE))
			tc->selected = 0;
cont:
		cx += tc->w;
	}
}

static void
cell_sel(AG_Table *t, int mc, int x)
{
	AG_TableCell *c;
	u_int m, n;

	if (multisel(t)) {
		if (mc < t->m) {
			for (n = 0; n < t->n; n++) {
				c = &t->cells[mc][n];
				c->selected = !c->selected;
				t->cols[n].selected = 0;
			}
		}
	} else {
		for (m = 0; m < t->m; m++) {
			for (n = 0; n < t->n; n++) {
				c = &t->cells[m][n];
				c->selected = ((int)m == mc);
				t->cols[n].selected = 0;
			}
		}
	}
}

static void
cell_popup(AG_Table *t, int mc, int x)
{
}

static __inline__ int
column_over(AG_Table *t, int y)
{
	return (AG_WidgetInt(t->vbar, "value") + y/t->row_h - 1);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	int m;

	pthread_mutex_lock(&t->lock);
	if ((m = column_over(t, y)) >= (int)t->m)
		goto out;

	switch (button) {
	case SDL_BUTTON_LEFT:
		if (m < 0) {
			column_sel(t, x);
		} else {
			cell_sel(t, m, x);
		}
		break;
	case SDL_BUTTON_RIGHT:
		if (m < 0) {
			column_popup(t, x);
		} else {
			cell_popup(t, m, x);
		}
		break;
	default:
		break;
	}
out:
	AG_WidgetFocus(t);
	pthread_mutex_unlock(&t->lock);
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	u_int m, n;

	switch (button) {
	case SDL_BUTTON_LEFT:
		if (t->nResizing >= 0) {
			t->nResizing = -1;
		}
		break;
	}
}

static void
keydown(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&t->lock);
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
	pthread_mutex_unlock(&t->lock);
}

static int
column_resize_over(AG_Table *t, int px)
{
	int x = px - (COLUMN_RESIZE_RANGE/2);
	u_int n;
	int cx;

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
mousemotion(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	int n, cx;
	int m;

	pthread_mutex_lock(&t->lock);
	if (t->nResizing >= 0 && (u_int)t->nResizing < t->n) {
		if ((t->cols[t->nResizing].w += xrel) < COLUMN_MIN_WIDTH) {
			t->cols[t->nResizing].w = COLUMN_MIN_WIDTH;
		}
	} else {
		if ((m = column_over(t, y)) == -1 &&
		    column_resize_over(t, x)) {
			AG_WidgetSetCursor(t, AG_HRESIZE_CURSOR);
		} else {
			AG_WidgetUnsetCursor(t);
		}
	}
	pthread_mutex_unlock(&t->lock);
}

static void
keyup(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&t->lock);
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
	pthread_mutex_unlock(&t->lock);
}

static void
lostfocus(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;

	pthread_mutex_lock(&t->lock);
	AG_CancelEvent(t, "key-tick");
	AG_CancelEvent(t, "dblclick-expire");
	if (t->nResizing >= 0) {
		AG_WidgetUnsetCursor(t);
		t->nResizing = -1;
	}
	pthread_mutex_unlock(&t->lock);
}

int
AG_TableRowSelected(AG_Table *t, u_int m)
{
	u_int n;

	for (n = 0; n < t->n; n++) {
		if (t->cells[m][n].selected)
			return (1);
	}
	return (0);
}

void
AG_TableSelectRow(AG_Table *t, u_int m)
{
	u_int n;

	for (n = 0; n < t->n; n++)
		t->cells[m][n].selected = 1;
}

void
AG_TableDeselectRow(AG_Table *t, u_int m)
{
	u_int n;

	for (n = 0; n < t->n; n++)
		t->cells[m][n].selected = 0;
}

void
AG_TableDeselectAllRows(AG_Table *t)
{
	u_int m, n;

	for (n = 0; n < t->n; n++) {
		for (m = 0; m < t->m; m++)
			t->cells[m][n].selected = 0;
	}
}

void
AG_TableSelectAllCols(AG_Table *t)
{
	u_int n;

	for (n = 0; n < t->n; n++)
		t->cols[n].selected = 1;
}

void
AG_TableDeselectAllCols(AG_Table *t)
{
	u_int n;

	for (n = 0; n < t->n; n++)
		t->cols[n].selected = 0;
}

static void
kbdscroll(int argc, union evarg *argv)
{
	AG_Table *t = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	u_int m2, n;
	int m;
	
	pthread_mutex_lock(&t->lock);
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
	pthread_mutex_unlock(&t->lock);
}

int
AG_TableAddCol(AG_Table *t, const char *name, const char *size_spec,
    int (*sort_fn)(const void *, const void *))
{
	AG_TableCol *tc, *lc;
	AG_TableCell *c;
	u_int m, n;

	/* Initialize the column information structure. */
	t->cols = Realloc(t->cols, (t->n+1)*sizeof(AG_TableCol));
	tc = &t->cols[t->n];
	strlcpy(tc->name, name, sizeof(tc->name));
	tc->sort_fn = sort_fn;
	tc->sort_order = 0;
	tc->selected = 0;
	tc->surface = AG_WidgetMapSurface(t,
	    AG_TextRender(NULL, -1, AG_COLOR(TLIST_TXT_COLOR), name));
	if (t->n > 0) {
		lc = &t->cols[t->n - 1];
		tc->x = lc->x+lc->w;
	} else {
		tc->x = 0;
	}
	tc->pool = NULL;
	tc->mpool = 0;
	
	switch (AG_WidgetParseSizeSpec(size_spec, &tc->w)) {
	case AG_WIDGET_PERCENT:
		tc->w = tc->w*AGWIDGET(t)->w/100;
		break;
	default:
		break;
	}

	/* Resize the row arrays. */
	for (m = 0; m < t->m; m++) {
		t->cells[m] = Realloc(t->cells[m],
		    (t->n+1)*sizeof(AG_TableCell));
		AG_TableInitCell(t, &t->cells[m][t->n]);
	}
	return (t->n++);
}

void
AG_TableInitCell(AG_Table *t, AG_TableCell *c)
{
	c->type = AG_CELL_NULL;
	c->fmt[0] = '\0';
	c->fn = NULL;
	c->selected = 0;
	c->surface = -1;
}

int
AG_TableAddRow(AG_Table *t, const char *fmtp, ...)
{
	char fmt[64], *sp = &fmt[0];
	va_list ap;
	u_int n;

	strlcpy(fmt, fmtp, sizeof(fmt));

	va_start(ap, fmtp);
	t->cells = Realloc(t->cells, (t->m+1)*sizeof(AG_TableCell));
	t->cells[t->m] = Malloc(t->n*sizeof(AG_TableCell), M_WIDGET);
	for (n = 0; n < t->n; n++) {
		AG_TableCol *tc = &t->cols[n];
		AG_TableCell *c = &t->cells[t->m][n];
		char *s = strsep(&sp, ":;, "), *sc;
		int ptr = 0, lflag = 0;

		AG_TableInitCell(t, c);
		for (sc = &s[0]; *sc != '\0'; sc++) {
			if (*sc == '%') {
				continue;
			}
			if (*sc == '*' || *sc == '[') {
				ptr++;
			} else if (*sc == 'l') {
				lflag++;
			} else {
				break;
			}
		}
		if (ptr) {
			c->data.p = va_arg(ap, void *);
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
		case 'i':
			if (ptr) {
				c->type = AG_CELL_PINT;
			} else {
				c->type = AG_CELL_INT;
				c->data.i = va_arg(ap, int);
			}
			break;
		case 'u':
			if (ptr) {
				c->type = AG_CELL_PUINT;
			} else {
				c->type = AG_CELL_UINT;
				c->data.i = va_arg(ap, int);
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
		case '[':
			if (sc[1] == 's') {
				if (sc[2] == '3' && sc[3] == '2') {
					c->type = AG_CELL_PSINT32;
				} else if (s[2] == '1' && sc[3] == '6') {
					c->type = AG_CELL_PSINT16;
				} else if (s[2] == '8') {
					c->type = AG_CELL_PSINT8;
				}
			} else if (sc[1] == 'u') {
				if (sc[2] == '3' && sc[3] == '2') {
					c->type = AG_CELL_PUINT32;
				} else if (s[2] == '1' && sc[3] == '6') {
					c->type = AG_CELL_PUINT16;
				} else if (s[2] == '8') {
					c->type = AG_CELL_PUINT8;
				}
			}
			break;
		}
	}
	va_end(ap);
	return (t->m++);
}
