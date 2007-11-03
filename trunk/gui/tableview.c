/*
 * Copyright (c) 2004 John Blitch
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

#include "tableview.h"

#include "window.h"
#include "primitive.h"

#include <string.h>
#include <stdarg.h>

#define ID_INVALID ((Uint)-1)
#define VISROW(tv,i) ((tv)->visible.items[i].row)
#define VISDEPTH(tv,i) ((tv)->visible.items[i].depth)

/*
 * Worker function for foreach_visible_column()
 * Should return 0 if foreach_ should stop.
 */
typedef int (*visible_do)(AG_Tableview *tv, int x1, int x2, Uint32 visidx,
    void *, void *);

static void foreach_visible_column(AG_Tableview *, visible_do, void *,
                                   void *);
static void view_changed(AG_Tableview *);
static int view_changed_check(AG_Tableview *, struct ag_tableview_rowq *,
			      int, int, int *);
static AG_TableviewRow *row_get(struct ag_tableview_rowq *, AG_TableviewRowID);
static void select_all(struct ag_tableview_rowq *);
static void deselect_all(struct ag_tableview_rowq *);
static int count_visible_descendants(struct ag_tableview_rowq *, int);
static void switch_columns(AG_Tableview *, Uint, Uint);
static int visible(AG_TableviewRow *);
static void render_dyncolumn(AG_Tableview *, Uint);
static int clicked_header(AG_Tableview *, int, int, Uint32, void *, void *);
static int clicked_row(AG_Tableview *, int, int, Uint32, void *, void *);
static int draw_column(AG_Tableview *, int, int, Uint32, void *, void *);

static void mousebuttonup(AG_Event *);
static void mousebuttondown(AG_Event *);
static void scrolled(AG_Event *);
static void dblclick_expire(AG_Event *);
static void lost_focus(AG_Event *);
static void columnresize(AG_Event *);
static void columnmove(AG_Event *);


AG_Tableview *
AG_TableviewNew(void *parent, Uint flags, AG_TableviewDataFn data_callback,
    AG_TableviewSortFn sort_callback)
{
	AG_Tableview *tv;

	tv = Malloc(sizeof(AG_Tableview), M_WIDGET);
	AG_TableviewInit(tv, flags, data_callback, sort_callback);
	AG_ObjectAttach(parent, tv);
	if (flags & AG_TABLEVIEW_FOCUS) {
		AG_WidgetFocus(tv);
	}
	return (tv);
}

void
AG_TableviewInit(AG_Tableview *tv, Uint flags, AG_TableviewDataFn data_callback,
    AG_TableviewSortFn sort_callback)
{
	Uint wflags = AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	if (flags & AG_TABLEVIEW_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_TABLEVIEW_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(tv, &agTableviewOps, wflags);
	AG_MutexInitRecursive(&tv->lock);

	tv->data_callback = data_callback;
	tv->sort_callback = sort_callback;

	/* initialize flags */
	tv->selmulti = 0;
	tv->selsingle = 0;
	tv->selnoclear = 0;
	tv->reordercols = 0;
	tv->header = 0;
	tv->locked = 0;
	tv->sort = 0;
	tv->polled = 0;

	/* set requested flags */
	if (flags & AG_TABLEVIEW_REORDERCOLS)	tv->reordercols = 1;
	if (!(flags & AG_TABLEVIEW_NOHEADER))	tv->header = 1;
	if (!(flags & AG_TABLEVIEW_NOSORT))	tv->sort = 1;
	if (flags & AG_TABLEVIEW_REORDERCOLS)	tv->reordercols = 1;
	if (flags & AG_TABLEVIEW_SELMULTI)	tv->selmulti = 1;
	if (flags & AG_TABLEVIEW_POLLED)	tv->polled = 1;

	tv->head_height = tv->header ? agTextFontHeight : 0;
	tv->row_height = agTextFontHeight+2;
	tv->dblclicked = 0;
	tv->sbar_v = AG_ScrollbarNew(tv, AG_SCROLLBAR_VERT, 0);
	tv->sbar_h = (flags & AG_TABLEVIEW_HORIZ) ?
	    AG_ScrollbarNew(tv, AG_SCROLLBAR_HORIZ, 0) : NULL;
	//tv->editbox = NULL;

	AG_WidgetSetInt(tv->sbar_v, "min", 0);
	AG_WidgetSetInt(tv->sbar_v, "max", 0);
	AG_WidgetSetInt(tv->sbar_v, "value", 0);

	if (tv->sbar_h != NULL) {
		AG_WidgetSetInt(tv->sbar_h, "min", 0);
		AG_WidgetSetInt(tv->sbar_h, "max", 0);
		AG_WidgetSetInt(tv->sbar_h, "value", 0);
	}
	tv->column = NULL;
	tv->columncount = 0;
	tv->sortMode = AG_TABLEVIEW_SORT_NOT;

	tv->sortColumn = ID_INVALID;
	tv->enterEdit = ID_INVALID;
	tv->expanderColumn = ID_INVALID;

	TAILQ_INIT(&tv->children);
	TAILQ_INIT(&tv->backstore);
	tv->expandedrows = 0;

	tv->visible.redraw_rate = 0;
	tv->visible.redraw_last = SDL_GetTicks();
	tv->visible.count = 0;
	tv->visible.items = NULL;

	tv->prew = 10;
	tv->preh = tv->head_height + (tv->row_height * 4);

	/*
	 * XXX - implement key selection / scroll changing AG_SetEvent(tv,
	 * "window-keydown", lvist_keydown, NULL); AG_SetEvent(tv,
	 * "window-keyup", lvist_keyup, NULL); AG_SetEvent(tv, "key-tick",
	 * key_tick, NULL);
	 */

	/* private, internal events */
	AG_SetEvent(tv, "window-mousebuttonup", mousebuttonup, NULL);
	AG_SetEvent(tv, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(tv->sbar_v, "scrollbar-changed", scrolled, "%p", tv);
	AG_SetEvent(tv, "dblclick-expire", dblclick_expire, NULL);
	AG_SetEvent(tv, "widget-lostfocus", lost_focus, NULL);
	AG_SetEvent(tv, "widget-hidden", lost_focus, NULL);

	AG_SetEvent(tv, "column-resize", columnresize, NULL);
	AG_SetEvent(tv, "column-move", columnmove, NULL);
}

void
AG_TableviewSizeHint(AG_Tableview *tv, const char *text, int nitems)
{
	AG_TextSize(text, &tv->prew, NULL);
	tv->preh = tv->head_height + (tv->row_height * nitems);
}


AG_TableviewCol *
AG_TableviewColAdd(AG_Tableview *tv, int flags, AG_TableviewColID cid,
    const char *label, const char *size)
{
	AG_TableviewCol *col = NULL;
	Uint i;

	if (tv->locked || cid == ID_INVALID)
		return (NULL);

	AG_MutexLock(&tv->lock);

	/* column identifier must be unique */
	for (i = 0; i < tv->columncount; i++) {
		if (tv->column[i].cid == cid)
			goto out;
	}

	tv->column = Realloc(tv->column, (tv->columncount+1) *
					 sizeof(AG_TableviewCol));
	col = &tv->column[tv->columncount];
	col->cid = cid;
	col->idx = tv->columncount;
	col->mousedown = 0;
	tv->columncount++;

	if (tv->columncount == 1)
		tv->expanderColumn = cid;

	/* initialize flags */
	col->mousedown = 0;
	col->moving = 0;
	col->editable = 0;
	col->resizable = 1;
	col->update = 0;
	col->fill = 0;
	col->dynamic = 0;

	/* set requested flags */
	if (flags & AG_TABLEVIEW_COL_EDITABLE)	col->editable = 1;
	if (flags & AG_TABLEVIEW_COL_KEYEDIT)	tv->enterEdit = cid;
	if (flags & AG_TABLEVIEW_COL_NORESIZE)	col->resizable = 0;
	if (flags & AG_TABLEVIEW_COL_DYNAMIC)	col->dynamic = 1;
	if (flags & AG_TABLEVIEW_COL_EXPANDER)	tv->expanderColumn = cid;
	if (flags & AG_TABLEVIEW_COL_FILL)		col->fill = 1;
	if ((flags & AG_TABLEVIEW_COL_UPDATE) &&
	    (flags & AG_TABLEVIEW_COL_DYNAMIC))	col->update = 1;

	/* column label */
	if (label == NULL) {
		col->label[0] = '\0';
	} else {
		strlcpy(col->label, label, sizeof(col->label));
	}
	AG_TextColor(TABLEVIEW_HTXT_COLOR);
	col->label_img = AG_TextRender(col->label);
	col->label_id = AG_WidgetMapSurface(tv, col->label_img);

	/* column width */
	if (size != NULL) {
		switch (AG_WidgetParseSizeSpec(size, &col->w)) {
		case AG_WIDGET_PERCENT:
			col->w = col->w*WIDGET(tv)->w/100;
			break;
		default:
			break;
		}
	} else {
		col->w = 6;
		col->fill = 1;
	}

	tv->visible.dirty = 1;
out:
	AG_MutexUnlock(&tv->lock);
	return (col);
}

void
AG_TableviewColSelect(AG_Tableview *tv, AG_TableviewColID cid)
{
	Uint i, ind = -1, valid = 0;

	AG_MutexLock(&tv->lock);

        /* check if cid is valid */
        for (i = 0; i < tv->columncount; i++) {
                if (tv->column[i].cid == cid) {
                        valid = 1;
			ind = i;
			break;
		}
        }

	if (valid) {
		tv->column[ind].mousedown = 1;
	}
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewSetUpdate(AG_Tableview *tv, Uint ms)
{
	AG_MutexLock(&tv->lock);
	tv->visible.redraw_rate = ms;
	AG_MutexUnlock(&tv->lock);
}

AG_TableviewRow *
AG_TableviewRowAddFn(AG_Tableview *tv, int flags,
    AG_TableviewRow *parent, void *userp, AG_TableviewRowID rid, ...)
{
	AG_TableviewRow *row;
	Uint i;
	va_list ap;
	AG_TableviewColID cid;

	AG_MutexLock(&tv->lock);

	tv->locked = 1;

	/* verify the row identifier is not in use */
	if (row_get(&tv->children, rid)) {
		AG_MutexUnlock(&tv->lock);
		return (NULL);
	}
	row = Malloc(sizeof(AG_TableviewRow), M_WIDGET);
	row->cell = Malloc(sizeof(struct ag_tableview_cell) * tv->columncount,
	    M_WIDGET);
	row->userp = userp;
	row->dynamic = !(flags & AG_TABLEVIEW_STATIC_ROW);

	/* initialize cells */
	for (i = 0; i < tv->columncount; i++) {
		row->cell[i].text = NULL;
		row->cell[i].image = NULL;
	}

	/* import static data */
	va_start(ap, rid);
	while ((cid = va_arg(ap, AG_TableviewColID)) != -1) {
		void *data = va_arg(ap, void *);

		for (i = 0; i < tv->columncount; i++) {
			if (cid == tv->column[i].cid) {
				/*
				 * If the user already passed for this col,
				 * don't leak.
				 */
				if (row->cell[i].text != NULL) {
					free(row->cell[i].text);
				}
				if (row->cell[i].image != NULL) {
					SDL_FreeSurface(row->cell[i].image);
				}
				row->cell[i].text = (data != NULL) ?
				                    Strdup((char *)data) :
						    Strdup("(null)");

				AG_TextColor(TABLEVIEW_CTXT_COLOR);
				row->cell[i].image = AG_TextRender(
				    row->cell[i].text);
				break;
			}
		}
	}
	va_end(ap);

	TAILQ_INIT(&row->children);

	/* initialize flags */
	row->selected = 0;
	row->expanded = 0;

	/* set requested flags */
	row->rid = rid;
	row->parent = parent;

	if (parent) {
		TAILQ_INSERT_TAIL(&parent->children, row, siblings);
	} else {
		TAILQ_INSERT_TAIL(&tv->children, row, siblings);
	}

	/* increment scroll only if visible: */
	if (visible(row))
		tv->expandedrows++;

	tv->visible.dirty = 1;
	AG_MutexUnlock(&tv->lock);
	return (row);
}

static void
tableview_row_destroy(AG_Tableview *tv, AG_TableviewRow *row)
{
	int i;

	for (i = 0; i < tv->columncount; i++) {
		SDL_FreeSurface(row->cell[i].image);
		free(row->cell[i].text);
	}

	Free(row->cell, M_WIDGET);
	Free(row, M_WIDGET);
}

void
AG_TableviewRowDel(AG_Tableview *tv, AG_TableviewRow *row)
{
	AG_TableviewRow *row1, *row2;

	if (row == NULL)
		return;

	AG_MutexLock(&tv->lock);

	/* first remove children */
	row1 = TAILQ_FIRST(&row->children);
	while (row1 != NULL) {
		row2 = TAILQ_NEXT(row1, siblings);
		AG_TableviewRowDel(tv, row1);
		row1 = row2;
	}

	/* now that children are gone, remove this row */
	if (visible(row))
		tv->expandedrows--;

	if (row->parent) {
		TAILQ_REMOVE(&row->parent->children, row, siblings);
	} else {
		TAILQ_REMOVE(&tv->children, row, siblings);
	}
	if (tv->polled) {
		TAILQ_INSERT_TAIL(&tv->backstore, row, backstore);
		goto out;
	}
	tableview_row_destroy(tv, row);
out:
	tv->visible.dirty = 1;
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowDelAll(AG_Tableview *tv)
{
	AG_TableviewRow *row1, *row2;

	AG_MutexLock(&tv->lock);
	row1 = TAILQ_FIRST(&tv->children);
	while (row1 != NULL) {
		row2 = TAILQ_NEXT(row1, siblings);
		AG_TableviewRowDel(tv, row1);
		row1 = row2;
	}
	TAILQ_INIT(&tv->children);
	
	tv->expandedrows = 0;
	tv->visible.dirty = 1;
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowRestoreAll(AG_Tableview *tv)
{
	AG_TableviewRow *row, *nrow, *srow;
	int i;

	AG_MutexLock(&tv->lock);
		
	for (row = TAILQ_FIRST(&tv->backstore);
	     row != TAILQ_END(&tv->backstore);
	     row = nrow) {
		nrow = TAILQ_NEXT(row, backstore);
		TAILQ_FOREACH(srow, &tv->children, siblings) {
			for (i = 0; i < tv->columncount; i++) {
				if (strcmp(row->cell[i].text,
				    srow->cell[i].text) != 0)
					break;
			}
			if (i == tv->columncount)
				srow->selected = row->selected;
		}
		tableview_row_destroy(tv, row);
	}
	TAILQ_INIT(&tv->backstore);

	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowSelect(AG_Tableview *tv, AG_TableviewRow *row)
{
	AG_MutexLock(&tv->lock);
	if (!tv->selmulti) {
		AG_TableviewRowDeselectAll(tv, NULL);
	}
	row->selected = 1;
	AG_MutexUnlock(&tv->lock);
}

AG_TableviewRow *
AG_TableviewRowGet(AG_Tableview *tv, AG_TableviewRowID rid)
{
	AG_TableviewRow *row;

	/* XXX pointless lock */
	AG_MutexLock(&tv->lock);
	row = row_get(&tv->children, rid);
	AG_MutexUnlock(&tv->lock);
	return (row);
}

void
AG_TableviewRowSelectAll(AG_Tableview *tv, AG_TableviewRow *root)
{
	if (!tv->selmulti)
		return;

	AG_MutexLock(&tv->lock);
	if (root == NULL) {
		select_all(&tv->children);
	} else {
		select_all(&root->children);
	}
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowDeselectAll(AG_Tableview *tv, AG_TableviewRow *root)
{
	AG_MutexLock(&tv->lock);
	if (root == NULL) {
		deselect_all(&tv->children);
	} else {
		deselect_all(&root->children);
	}
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowExpand(AG_Tableview *tv, AG_TableviewRow *in)
{
	AG_MutexLock(&tv->lock);
	
	if (!in->expanded) {
		in->expanded = 1;
		if (visible(in)) {
			tv->expandedrows +=
			    count_visible_descendants(&in->children, 0);
			tv->visible.dirty = 1;
		}
	}
	
	AG_MutexUnlock(&tv->lock);
}

void
AG_TableviewRowCollapse(AG_Tableview *tv, AG_TableviewRow *in)
{
	AG_MutexLock(&tv->lock);
	
	if (in->expanded) {
		in->expanded = 0;
		if (visible(in)) {
			tv->expandedrows -=
			    count_visible_descendants(&in->children, 0);
			tv->visible.dirty = 1;
		}
	}
	
	AG_MutexUnlock(&tv->lock);
}

static void
Destroy(void *p)
{
	AG_Tableview *tv = p;

	AG_TableviewRowDelAll(tv);
	Free(tv->column, M_WIDGET);
	Free(tv->visible.items, M_WIDGET);
	AG_MutexDestroy(&tv->lock);
	AG_WidgetDestroy(tv);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Tableview *tv = p;
	int i;

	AG_MutexLock(&tv->lock);

	r->w = tv->prew + tv->sbar_v->bw;
	r->h = tv->preh + (tv->sbar_h == NULL ? 0 : tv->sbar_h->bw);

	for (i = 0; i < tv->columncount; i++) {
		r->w += tv->column[i].w;
	}
	AG_MutexUnlock(&tv->lock);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Tableview *tv = p;
	Uint rows_per_view, i;
	AG_SizeAlloc aBar;

	AG_MutexLock(&tv->lock);
	
	/* Size vertical scroll bar. */
	aBar.x = a->w - tv->sbar_v->bw;
	aBar.y = 0;
	aBar.w = tv->sbar_v->bw;
	aBar.h = a->h - (tv->sbar_h ? tv->sbar_h->bw : 0);
	AG_WidgetSizeAlloc(tv->sbar_v, &aBar);

	/* Size horizontal scroll bar, if enabled. */
	if (tv->sbar_h != NULL) {
		int col_w = 0;

		aBar.x = 0;
		aBar.y = a->h - tv->sbar_h->bw;
		aBar.w = a->w - tv->sbar_h->bw;
		aBar.h = tv->sbar_h->bw;
		AG_WidgetSizeAlloc(tv->sbar_v, &aBar);

		for (i = 0; i < tv->columncount; i++) {
			col_w += tv->column[i].w;
		}
		if (col_w > WIDGET(tv->sbar_h)->w) {
			int scroll = col_w - WIDGET(tv->sbar_h)->w;

			AG_WidgetSetInt(tv->sbar_h, "max", scroll);
			if (AG_WidgetInt(tv->sbar_h, "value") > scroll) {
				AG_WidgetSetInt(tv->sbar_h, "value", scroll);
			}
			AG_ScrollbarSetBarSize(tv->sbar_h,
			    WIDGET(tv->sbar_h)->w * (a->w - tv->sbar_h->bw*3) / 
			    col_w);
		} else {
			AG_WidgetSetInt(tv->sbar_h, "value", 0);
			AG_ScrollbarSetBarSize(tv->sbar_h, -1);
		}
	}

	/* Calculate widths for TABLEVIEW_COL_FILL columns. */
	{
		Uint fill_cols, nonfill_width, fill_width;

		fill_cols = 0;
		nonfill_width = 0;
		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill) {
				fill_cols++;
			} else {
				nonfill_width += tv->column[i].w;
			}
		}

		fill_width = (a->w - WIDGET(tv->sbar_v)->w - nonfill_width) /
		             fill_cols;

		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill)
				tv->column[i].w = fill_width;
		}
	}

	/* Calculate how many rows the view holds. */
	{
		int view_height = a->h - (NULL == tv->sbar_h ? 0 :
				  WIDGET(tv->sbar_h)->h);

		if (view_height < tv->head_height) {
			view_height = tv->head_height;
		}
		rows_per_view = (view_height - tv->head_height) /
		    tv->row_height;

		if ((view_height - tv->head_height) % tv->row_height)
			rows_per_view++;
	}

	if (tv->visible.count < rows_per_view) {
		/* visible area increased */
		tv->visible.items = Realloc(tv->visible.items,
		    sizeof(struct ag_tableview_rowdocket_item) * rows_per_view);

		for (i = tv->visible.count; i < rows_per_view; i++)
			VISROW(tv,i) = NULL;

		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	} else if (tv->visible.count > rows_per_view) {
		/* visible area decreased */
		tv->visible.items = Realloc(tv->visible.items,
		    sizeof(struct ag_tableview_rowdocket_item) * rows_per_view);
		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	}

	AG_MutexUnlock(&tv->lock);
	return (0);
}

static void
Draw(void *p)
{
	AG_Tableview *tv = p;
	Uint i;
	int y, update = 0;
	const int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);

	AG_MutexLock(&tv->lock);
	
	/* before we draw, update if needed */
	if (tv->visible.dirty)
		view_changed(tv);

	if (tv->visible.redraw_rate &&
	    SDL_GetTicks() > tv->visible.redraw_last + tv->visible.redraw_rate)
		update = 1;
	
	/* Draw the background box */
	STYLE(tv)->TableBackground(tv, 0, 0, WIDTH(tv), HEIGHT(tv));
	
	/* draw row selection hilites */
	y = tv->head_height;
	for (i = 0; i < tv->visible.count; i++) {
		if (VISROW(tv,i) == NULL) {
			break;
		}
		STYLE(tv)->TableRowBackground(tv,
		    AG_RECT(1, y, view_width-2, tv->row_height),
		    VISROW(tv,i)->selected);
		y += tv->row_height;
	}

	/* draw columns */
	foreach_visible_column(tv, draw_column, &update, NULL);

	if (update) {
		tv->visible.redraw_last = SDL_GetTicks();
	}
	AG_MutexUnlock(&tv->lock);
}

/*
 * *********************** INTERNAL FUNCTIONS ***********************
 * 
 * we assume appropriate mutexes are locked.
 */

/*
 * loops through each column currently onscreen. calls visible_fn for each
 * one, passing along args returns the index of the first dothis() call to
 * return zero, or tv->columncount, whichever comes first.
 */
static void
foreach_visible_column(AG_Tableview *tv, visible_do dothis, void *arg1,
    void *arg2)
{
	int x, first_col, col_width;
	Uint i;
	int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);
	int view_edge = (tv->sbar_h ? AG_WidgetInt(tv->sbar_h, "value") : 0);

	x = 0;
	first_col = -1;
	for (i = 0; i < tv->columncount; i++) {
		/* skip until we find an onscreen column */
		if (first_col == -1 &&
		    x + tv->column[i].w < view_edge) {
			x += tv->column[i].w;	/* x represents the offset in
						 * the table */
			continue;
		}
		/* OK, we found the first onscreen column */
		else if (first_col == -1) {
			first_col = i;
			x = x - view_edge;	/* x now represents the
						 * offset on the screen */
		}
		/* stop after the last onscreen column */
		if (x >= view_width)
			break;

		/* determine how wide this column will be */
		col_width = (x + tv->column[i].w > view_width) ?
			view_width - x : tv->column[i].w;

		/* do what we should */
		if (dothis(tv, x, x + col_width, i, arg1, arg2) == 0)
			return;

		x += tv->column[i].w;
	}
}

/*
 * called after any addition or removal of visible rows. Rebuilds the array
 * of rows to be drawn (tv->visible)
 */
static void
view_changed(AG_Tableview *tv)
{
	int rows_per_view, max, filled, value;
	int view_height = WIDGET(tv)->h - (tv->sbar_h != NULL ?
			                   WIDGET(tv->sbar_h)->h : 0);
	int scrolling_area = WIDGET(tv->sbar_v)->h - tv->sbar_v->bw*2;
	Uint i;

	/* cancel double clicks if what's under it changes it */
	tv->dblclicked = 0;
	AG_CancelEvent(tv, "dblclick-expire");

	rows_per_view = (view_height - tv->head_height) / tv->row_height;
	if ((view_height - tv->head_height) % tv->row_height)
		rows_per_view++;

	max = tv->expandedrows - rows_per_view;
	if (max < 0) {
		max = 0;
	}
	if (max && ((view_height - tv->head_height) % tv->row_height) < 16) {
		max++;
	}
	AG_WidgetSetInt(tv->sbar_v, "max", max);
	if (AG_WidgetInt(tv->sbar_v, "value") > max)
		AG_WidgetSetInt(tv->sbar_v, "value", max);

	/* Calculate Scrollbar Size */
	if (rows_per_view && tv->expandedrows > rows_per_view) {
		AG_ScrollbarSetBarSize(tv->sbar_v,
		    rows_per_view * scrolling_area / tv->expandedrows);
	} else {
		AG_ScrollbarSetBarSize(tv->sbar_v, -1);
	}

	/* locate visible rows */
	value = AG_WidgetInt(tv->sbar_v, "value");
	filled = view_changed_check(tv, &tv->children, 0, 0, &value);

	/* blank empty rows */
	for (i = filled; i < tv->visible.count; i++)
		VISROW(tv,i) = NULL;

	/* render dynamic columns */
	for (i = 0; i < tv->columncount; i++)
		if (tv->column[i].dynamic)
			render_dyncolumn(tv, i);

	tv->visible.redraw_last = SDL_GetTicks();
	tv->visible.dirty = 0;
}

/*
 * recursive companion to view_changed. finds the visible rows and populates
 * tv->visible.items array with them
 */
static int
view_changed_check(AG_Tableview *tv, struct ag_tableview_rowq* in, int depth,
    int filled, int *seen)
{
	AG_TableviewRow *row;
	Uint x = 0;

	TAILQ_FOREACH(row, in, siblings) {
		if (*seen) {
			(*seen)--;
		} else {
			if ((filled+x) < tv->visible.count) {
				VISROW(tv,filled+x) = row;
				VISDEPTH(tv,filled+x) = depth;
				x++;
			}
		}

		if (row->expanded && filled+x < tv->visible.count)
			x += view_changed_check(tv, &row->children, depth + 1,
			    filled + x, seen);

		if (filled+x >= tv->visible.count)
			return (x);
	}
	return (x);
}

/*
 * return a pointer if the row with the given identifer exists in or in a
 * descendant of the rowq. If it does not exist, return NULL
 */
static AG_TableviewRow *
row_get(struct ag_tableview_rowq *searchIn, AG_TableviewRowID rid)
{
	AG_TableviewRow *row, *row2;

	TAILQ_FOREACH(row, searchIn, siblings) {
		if (row->rid == rid)
			return (row);

		if (!TAILQ_EMPTY(&row->children)) {
			row2 = row_get(&row->children, rid);
			if (row2 != NULL)
				return (row2);
		}
	}
	return (NULL);
}

/* set the selection bit for all rows in and descended from the rowq */
static void
select_all(struct ag_tableview_rowq * children)
{
	AG_TableviewRow *row;

	TAILQ_FOREACH(row, children, siblings) {
		row->selected = 1;
		if (row->expanded && !TAILQ_EMPTY(&row->children))
			select_all(&row->children);
	}
}

AG_TableviewRow *
AG_TableviewRowSelected(AG_Tableview *tv)
{
	AG_TableviewRow *row;

	AG_MutexLock(&tv->lock);
	TAILQ_FOREACH(row, &tv->children, siblings) {
		if (row->selected) {
			AG_MutexUnlock(&tv->lock);
			return (row);
		}
	}
	AG_MutexUnlock(&tv->lock);
	return (NULL);
}

/* clear the selection bit for all rows in and descended from the rowq */
static void
deselect_all(struct ag_tableview_rowq *children)
{
	AG_TableviewRow *row;

	TAILQ_FOREACH(row, children, siblings) {
		row->selected = 0;
		if (row->expanded && !TAILQ_EMPTY(&row->children))
			deselect_all(&row->children);
	}
}

/*
 * return the number of visible descendants a given row has. must pass 0 for
 * i to get a meaningful result
 */
static int
count_visible_descendants(struct ag_tableview_rowq *in, int i)
{
	AG_TableviewRow *row;
	int j = i;

	TAILQ_FOREACH(row, in, siblings) {
		j++;
		if (row->expanded && !TAILQ_EMPTY(&row->children))
			j = count_visible_descendants(&row->children, j);
	}
	return (j);
}

/* switch column a with column b */
static void
switch_columns(AG_Tableview * tv, Uint a, Uint b)
{
	AG_TableviewCol col_tmp;

	/* a little sanity checking never hurt */
	if (a >= tv->columncount || b >= tv->columncount)
		return;
	if (a == b)
		return;

	col_tmp = tv->column[a];
	tv->column[a] = tv->column[b];
	tv->column[b] = col_tmp;
	tv->visible.dirty = 1;
}

/*
 * return 1 if the row is visible return 0 if an ancestor row is collapsed,
 * hiding it.
 */
static int
visible(AG_TableviewRow *in)
{
	AG_TableviewRow *row = in->parent;

	while (row != NULL) {
		if (!row->expanded) {
			return (0);
		}
		row = row->parent;
	}
	return (1);
}

static void
render_dyncolumn(AG_Tableview *tv, Uint idx)
{
	char *celltext;
	Uint i, cidx = tv->column[idx].idx;

	for (i = 0; i < tv->visible.count; i++) {
		AG_TableviewRow *row = VISROW(tv,i);

		if (row == NULL)
			break;
		if (!row->dynamic)
			continue;

		if (row->cell[cidx].image != NULL)
			SDL_FreeSurface(row->cell[cidx].image);

		celltext = tv->data_callback(tv, tv->column[idx].cid, row->rid);
		AG_TextColor(TABLEVIEW_CTXT_COLOR);
		row->cell[cidx].image =
		    AG_TextRender((celltext != NULL) ? celltext : "");
	}
}

static int
clicked_header(AG_Tableview *tv, int x1, int x2, Uint32 idx,
    void *arg1, void *arg2)
{
	int x = *(int *)arg1;

	if ((x < x1 || x >= x2) && idx != tv->columncount-1)
		return (1);

	/* click on the CENTER */
	if (x >= x1+3 && x <= x2-3) {
		if (tv->reordercols) {
			AG_SchedEvent(NULL, tv, 400, "column-move",
				       "%i,%i", idx, x1);
		}
		if (tv->reordercols || tv->sort)
			tv->column[idx].mousedown = 1;
	}
	/* click on the LEFT resize line */
	else if (idx-1 >= 0 && idx-1 < tv->columncount &&
	         tv->column[idx-1].resizable &&
		 idx > 0 &&
		 x < x1+3) {
		AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-resize",
		    "%i,%i", idx-1, x1-tv->column[idx-1].w);
	}
	/* click on the RIGHT resize line */
	else if (idx >= 0 && idx < tv->columncount &&
	         tv->column[idx].resizable &&
		 (x > x2-3 ||
		  (idx == tv->columncount-1 && x < x2+3))) {
		AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-resize",
		    "%i,%i", idx, x1);
	}
	return (0);
}

static int
clicked_row(AG_Tableview *tv, int x1, int x2, Uint32 idx, void *arg1,
    void *arg2)
{
	const int x = *(int *)arg1;
	const int y = *(int *)arg2;
	AG_TableviewRow *row = NULL;
	int depth = 0;
	int ts = tv->row_height/2 + 1;
	SDLMod modifiers = SDL_GetModState();
	Uint i, j, row_idx;
	int px;

	if (x < x1 || x >= x2)
		return (1);

	/* Find the visible row under the cursor. */
	for (i = 0, px = tv->head_height;
	     i < tv->visible.count;
	     i++, px += tv->row_height) {
		if (y >= px && y < px+tv->row_height) {
			if (VISROW(tv,i) == NULL) {
				i = tv->visible.count;
			}
			break;
		}
	}
	if (i != tv->visible.count) {
		row = VISROW(tv,i);
		depth = VISDEPTH(tv,i);
	}
	row_idx = i;

	/* Clicking on blank space clears the selection. */
	if (row == NULL) {
		deselect_all(&tv->children);
		//AG_PostEvent(NULL, tv, "tableview-selectclear", "");
		return (0);
	}

	/* Check for a click on the +/- button, if applicable. */
	if (tv->column[idx].cid == tv->expanderColumn &&
	    !TAILQ_EMPTY(&row->children) &&
	    x > (x1+4+(depth*(ts+4))) &&
	    x < (x1+4+(depth*(ts+4))+ts)) {
		if (row->expanded) {
			row->expanded = 0;
			/*
			 * XXX deselect_all could return count to save
			 * redundant call to count_visible_descendants
			 */
			deselect_all(&row->children);
			tv->expandedrows -= count_visible_descendants(
			    &row->children, 0);
		} else {
			row->expanded = 1;
			tv->expandedrows += count_visible_descendants(
			    &row->children, 0);
		}
		tv->visible.dirty = 1;
		return (0);
	}
	
	/* Handle command/control clicks and range selections. */
	if ((modifiers & KMOD_META || modifiers & KMOD_CTRL)) {
		if (row->selected) {
			row->selected = 0;
			AG_PostEvent(NULL, tv, "tableview-deselect", "%p", row);
		} else {
			if (!tv->selmulti) {
				deselect_all(&tv->children);
			}
			row->selected = 1;
			AG_PostEvent(NULL, tv, "tableview-select", "%p", row);
		}
	} else if (modifiers & KMOD_SHIFT) {
		for (j = 0; j < tv->visible.count; j++) {
			if (VISROW(tv,j) != NULL &&
			    VISROW(tv,j)->selected)
				break;
		}
		if (j < tv->visible.count) {
			/* XXX the selection may start on an invisible row */
			if (j < row_idx) {
				for (i = j; i <= row_idx; i++) {
					if (VISROW(tv,i) != NULL)
						VISROW(tv,i)->selected = 1;
				}
			} else if (j > row_idx) {
				for (i = row_idx; i <= j; i++) {
					if (VISROW(tv,i) != NULL)
						VISROW(tv,i)->selected = 1;
				}
			} else {
				if (VISROW(tv,row_idx) != NULL)
					VISROW(tv,row_idx)->selected = 1;
			}
		}
	} else {
		deselect_all(&tv->children);
		if (!row->selected) {
			row->selected = 1;
			AG_PostEvent(NULL, tv, "tableview-select", "%p", row);
		}
	}

	/* Handle double-clicks. */
	if (tv->dblclicked) {
		AG_CancelEvent(tv, "dblclick-expire");
		deselect_all(&tv->children);
		row->selected = 1;
		tv->dblclicked = 0;
		AG_PostEvent(NULL, tv, "tableview-dblclick", "%p", row);
	} else {
		tv->dblclicked++;
		AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "dblclick-expire",
		    NULL);
	}
	return (0);
}

static int
draw_column(AG_Tableview *tv, int x1, int x2, Uint32 idx, void *arg1,
    void *arg2)
{
	const int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);
	const int *update = (int *)arg1;
	Uint j, cidx = tv->column[idx].idx;
	int y;

	/* draw label for this column */
	if (tv->header) {
		int xLbl;

		STYLE(tv)->TableColumnHeaderBackground(tv, idx,
		    AG_RECT(x1, 0,
		            tv->column[idx].w,
			    tv->head_height),
		    (tv->column[idx].mousedown ||
		     tv->column[idx].cid == tv->sortColumn));

		xLbl = tv->column[idx].w/2 - tv->column[idx].label_img->w/2;
		AG_WidgetBlitSurface(tv, tv->column[idx].label_id,
		    x1 + xLbl,
		    0);
	}
	/* check for the need to update */
	if (*update && tv->column[idx].update)
		render_dyncolumn(tv, idx);

	/* draw cells in this column */
	y = tv->head_height;
	for (j = 0; j < tv->visible.count; j++) {
		int x = x1+4;

		if (VISROW(tv,j) == NULL) {
			break;
		}
		if (tv->column[idx].cid == tv->expanderColumn) {
			int tw = tv->row_height/2 + 1;

			x += VISDEPTH(tv,j)*(tw+4);
			if (!TAILQ_EMPTY(&VISROW(tv,j)->children)) {
				STYLE(tv)->TreeSubnodeIndicator(tv,
				    AG_RECT(x, y+tw/2, tw, tw),
				    VISROW(tv,j)->expanded);
			}
			x += tw+4;
		}
		if (VISROW(tv,j)->cell[cidx].image) {
			/* XXX inefficient in opengl mode */
			AG_WidgetBlit(tv,
			    VISROW(tv,j)->cell[cidx].image,
			    x, y+1);
		}
		y += tv->row_height;
	}

	/* Fill the Remaining Space in column heading */
	if (tv->header && idx == tv->columncount-1 && x2 < view_width) {
		STYLE(tv)->TableColumnHeaderBackground(tv, -1,
		    AG_RECT(x2, 0,
		            view_width-x2,
			    tv->head_height),
		    0);
	}
	return (1);
}

/*
 * *********************** EVENT HANDLERS **********************
 */

static void
mousebuttonup(AG_Event *event)
{
	AG_Tableview *tv = AG_SELF();
	int coord_x = AG_INT(2), coord_y = AG_INT(3);
	int left;
	Uint i;

	AG_CancelEvent(tv, "column-resize");
	AG_CancelEvent(tv, "column-move");

	/*
	 * Our goal here is to set/toggle a column's sorting mode if the
	 * click fell on the header and the user did not drag it
	 */
	/* XXX - fix horiz sbar */
	left = 0;
	for (i = 0; i < tv->columncount; i++) {
		if (tv->column[i].mousedown) {
			break;
		}
		left += tv->column[i].w;
	}

	if (i < tv->columncount) {
		/*
		 * if the column wasn't moved and the mouse is still within
		 * the column's header, do sort work
		 */
		if (tv->sort &&
		    tv->column[i].moving == 0 &&
		    coord_y < tv->head_height &&
		    coord_x >= left &&
		    coord_x < (left+tv->column[i].w)) {
			/* toggle the sort mode */
			if (tv->sortColumn == ID_INVALID ||
			    tv->sortMode == AG_TABLEVIEW_SORT_DSC) {
				tv->sortMode = AG_TABLEVIEW_SORT_ASC;
			} else {
				tv->sortMode = AG_TABLEVIEW_SORT_DSC;
			}

			tv->sortColumn = tv->column[i].cid;

			//XXX - run sort here
		}
		tv->column[i].mousedown = 0;
		tv->column[i].moving = 0;
	}
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Tableview *tv = AG_SELF();
	int coord_x = AG_INT(2);
	int coord_y = AG_INT(3);

	AG_WidgetFocus(tv);

	AG_MutexLock(&tv->lock);
	if (tv->header && coord_y < tv->head_height) {
		/* a mouse down on the column header */
		foreach_visible_column(tv, clicked_header, &coord_x, NULL);
	} else {
		/* a mouse down in the body */
		foreach_visible_column(tv, clicked_row, &coord_x, &coord_y);
	}
	AG_MutexUnlock(&tv->lock);
}

static void
scrolled(AG_Event *event)
{
	AG_Tableview *tv = AG_PTR(1);

	tv->visible.dirty = 1;
}

static void
dblclick_expire(AG_Event *event)
{
	AG_Tableview *tv = AG_SELF();

	AG_MutexLock(&tv->lock);

	/* the user hasn't clicked again, so cancel the double click */
	tv->dblclicked = 0;

	/* XXX - if the cursor remains in the cell, activate a click-to-edit */

	AG_MutexUnlock(&tv->lock);
}

/* XXX - this seems to be called after every click.. */
static void
lost_focus(AG_Event *event)
{
	//AG_CancelEvent(tv, "key-tick");
	//AG_CancelEvent(tv, "dblclick-expire");
	//tv->dblclicked = 0;
	//tv->keymoved = 0;
}

static void
columnresize(AG_Event *event)
{
	AG_Tableview *tv = AG_SELF();
	int col = AG_INT(1), left = AG_INT(2);
	int x;

	AG_MouseGetState(&x, NULL);
	x -= WIDGET(tv)->cx;
	tv->column[col].w = (x-left > tv->column[col].label_img->w) ?
	    (x-left) : tv->column[col].label_img->w;

	AG_ReschedEvent(tv, "column-resize", agMouseDblclickDelay);
}

static void
columnmove(AG_Event *event)
{
	AG_Tableview *tv = AG_SELF();
	Uint col = AG_UINT(1);
	int left = AG_INT(2);
	int x;

	/*
	 * if we haven't mouseupped by the time this event is called, set
	 * moving to indicate we won't sort.
	 */
	tv->column[col].moving = 1;

	AG_MouseGetState(&x, NULL);
	x -= WIDGET(tv)->cx;

	/* dragging to the left */
	if ((tv->column[col].w < tv->column[col-1].w &&
	     x < left - tv->column[col-1].w + tv->column[col].w) ||
	    (tv->column[col].w >= tv->column[col-1].w &&
	     x < left)) {
		if (col > 0) {
			left -= tv->column[col-1].w;
			switch_columns(tv, col, col-1);
			col--;
		}
	}
	/* dragging to the right */
	else if ((tv->column[col].w <= tv->column[col+1].w &&
		  x > left + tv->column[col+1].w) ||
		 (tv->column[col].w > tv->column[col+1].w &&
		  x > left + tv->column[col].w)) {
		if (col < tv->columncount-1) {
			left += tv->column[col+1].w;
			switch_columns(tv, col, col+1);
			col++;
		}
	}
	AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-move", "%u,%i",
	    col, left);
}

void
AG_TableviewCellPrintf(AG_Tableview *tv, AG_TableviewRow *row, int cell,
    const char *fmt, ...)
{
	va_list args;

	if (row->cell[cell].image != NULL) {
		SDL_FreeSurface(row->cell[cell].image);
	}
	Free(row->cell[cell].text, 0);

	va_start(args, fmt);
	Vasprintf(&row->cell[cell].text, fmt, args);
	va_end(args);

	AG_TextColor(TABLEVIEW_CTXT_COLOR);
	row->cell[cell].image = AG_TextRender(row->cell[cell].text);
}

const AG_WidgetOps agTableviewOps = {
	{
		"AG_Widget:AG_Tableview",
		sizeof(AG_Tableview),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
