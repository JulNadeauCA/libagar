/*	$Csoft: tableview.c,v 1.35 2005/09/29 02:16:25 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/config.h>
#include <engine/input.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/scrollbar.h>
/* #include <engine/widget/textbox.h> */

#include <string.h>
#include <stdarg.h>

#include "tableview.h"

#define ID_INVALID ((u_int)-1)

static AG_WidgetOps tableview_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_TableviewDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_TableviewDraw,
	AG_TableviewScale
};

/*
 * Worker function for foreach_visible_column()
 * arguments: tableview, visible_start, visible_end,
 * visible_index, and the argument passed to foreach_.
 * Should return 0 if foreach_ should stop.
 */
typedef int (*visible_do)(AG_Tableview *, int, int, Uint32, void *, void *);

static void foreach_visible_column(AG_Tableview *, visible_do, void *,
                                   void *);
static void view_changed(AG_Tableview *);
static int view_changed_check(AG_Tableview *, struct ag_tableview_rowq *,
			      int, int, int *);
static AG_TableviewRow *row_get(struct ag_tableview_rowq *, AG_TableviewRowID);
static void select_all(struct ag_tableview_rowq *);
static void deselect_all(struct ag_tableview_rowq *);
static int count_visible_descendants(struct ag_tableview_rowq *, int);
static void switch_columns(AG_Tableview *, u_int, u_int);
static int visible(AG_TableviewRow *);
static void render_dyncolumn(AG_Tableview *, u_int);
static int clicked_header(AG_Tableview *, int, int, Uint32, void *, void *);
static int clicked_row(AG_Tableview *, int, int, Uint32, void *, void *);
static int draw_column(AG_Tableview *, int, int, Uint32, void *, void *);

static void mousebuttonup(int, union evarg *);
static void mousebuttondown(int, union evarg *);
static void scrolled(int, union evarg *);
static void dblclick_expire(int, union evarg *);
static void lost_focus(int, union evarg *);
static void columnresize(int, union evarg *);
static void columnmove(int argc, union evarg *);


AG_Tableview *
AG_TableviewNew(void *parent, int flags, AG_TableviewDataFn data_callback,
    AG_TableviewSortFn sort_callback)
{
	AG_Tableview *tv;

	tv = Malloc(sizeof(AG_Tableview), M_WIDGET);
	AG_TableviewInit(tv, flags, data_callback, sort_callback);
	AG_ObjectAttach(parent, tv);
	return (tv);
}

void
AG_TableviewInit(AG_Tableview *tv, int flags, AG_TableviewDataFn data_callback,
    AG_TableviewSortFn sort_callback)
{
	AG_WidgetInit(tv, "tableview", &tableview_ops,
	  AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING|AG_WIDGET_WFILL|
	  AG_WIDGET_HFILL);
	pthread_mutex_init(&tv->lock, &agRecursiveMutexAttr);

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
	if (flags & AG_TABLEVIEW_SELMULTI)		tv->selmulti = 1;
	if (flags & AG_TABLEVIEW_POLLED)		tv->polled = 1;

	tv->head_height = tv->header ? agTextFontHeight : 0;
	tv->row_height = agTextFontHeight+2;
	tv->dblclicked = 0;
	tv->sbar_v = AG_ScrollbarNew(tv, AG_SCROLLBAR_VERT);
	tv->sbar_h = (flags & AG_TABLEVIEW_HORIZ) ?
		AG_ScrollbarNew(tv, AG_SCROLLBAR_HORIZ) : NULL;
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
AG_TableviewPrescale(AG_Tableview *tv, const char *text, int nitems)
{
	AG_TextPrescale(text, &tv->prew, NULL);
	tv->preh = tv->head_height + (tv->row_height * nitems);
}


AG_TableviewCol *
AG_TableviewColAdd(AG_Tableview *tv, int flags, AG_TableviewColID cid,
    const char *label, const char *size)
{
	AG_TableviewCol *col = NULL;
	u_int i;

	if (tv->locked || cid == ID_INVALID)
		return (NULL);

	pthread_mutex_lock(&tv->lock);

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
	col->label_img = AG_TextRender(NULL, -1, AG_COLOR(TABLEVIEW_HTXT_COLOR),
	    col->label);
	col->label_id = AG_WidgetMapSurface(tv, col->label_img);

	/* column width */
	if (size != NULL) {
		switch (AG_WidgetParseSizeSpec(size, &col->w)) {
		case AG_WIDGET_PERCENT:
			col->w = col->w*AGWIDGET(tv)->w/100;
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
	pthread_mutex_unlock(&tv->lock);
	return (col);
}

void
AG_TableviewColSelect(AG_Tableview *tv, AG_TableviewColID cid)
{
	u_int i, ind = -1, valid = 0;

	pthread_mutex_lock(&tv->lock);

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
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewSetUpdate(AG_Tableview *tv, u_int ms)
{
	pthread_mutex_lock(&tv->lock);
	tv->visible.redraw_rate = ms;
	pthread_mutex_unlock(&tv->lock);
}

AG_TableviewRow *
AG_TableviewRowAddFn(AG_Tableview *tv, int flags,
    AG_TableviewRow *parent, void *userp, AG_TableviewRowID rid, ...)
{
	AG_TableviewRow *row;
	u_int i;
	va_list ap;
	AG_TableviewColID cid;

	pthread_mutex_lock(&tv->lock);

	tv->locked = 1;

	/* verify the row identifier is not in use */
	if (row_get(&tv->children, rid)) {
		pthread_mutex_unlock(&tv->lock);
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
				row->cell[i].image = AG_TextRender(NULL, -1,
				    AG_COLOR(TABLEVIEW_CTXT_COLOR),
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
	pthread_mutex_unlock(&tv->lock);
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
	u_int i;
	AG_TableviewRow *row1, *row2;

	if (row == NULL)
		return;

	pthread_mutex_lock(&tv->lock);

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
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowDelAll(AG_Tableview *tv)
{
	AG_TableviewRow *row1, *row2;

	pthread_mutex_lock(&tv->lock);
	row1 = TAILQ_FIRST(&tv->children);
	while (row1 != NULL) {
		row2 = TAILQ_NEXT(row1, siblings);
		AG_TableviewRowDel(tv, row1);
		row1 = row2;
	}
	TAILQ_INIT(&tv->children);
	
	tv->expandedrows = 0;
	tv->visible.dirty = 1;
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowRestoreAll(AG_Tableview *tv)
{
	AG_TableviewRow *row, *nrow, *srow;
	int i;

	pthread_mutex_lock(&tv->lock);
		
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

	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowSelect(AG_Tableview *tv, AG_TableviewRow *row)
{
	pthread_mutex_lock(&tv->lock);
	if (!tv->selmulti) {
		AG_TableviewRowDeselectAll(tv, NULL);
	}
	row->selected = 1;
	pthread_mutex_unlock(&tv->lock);
}

AG_TableviewRow *
AG_TableviewRowGet(AG_Tableview *tv, AG_TableviewRowID rid)
{
	AG_TableviewRow *row;

	/* XXX pointless lock */
	pthread_mutex_lock(&tv->lock);
	row = row_get(&tv->children, rid);
	pthread_mutex_unlock(&tv->lock);
	return (row);
}

void
AG_TableviewRowSelectAll(AG_Tableview *tv, AG_TableviewRow *root)
{
	if (!tv->selmulti)
		return;

	pthread_mutex_lock(&tv->lock);
	if (root == NULL) {
		select_all(&tv->children);
	} else {
		select_all(&root->children);
	}
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowDeselectAll(AG_Tableview *tv, AG_TableviewRow *root)
{
	pthread_mutex_lock(&tv->lock);
	if (root == NULL) {
		deselect_all(&tv->children);
	} else {
		deselect_all(&root->children);
	}
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowExpand(AG_Tableview *tv, AG_TableviewRow *in)
{
	pthread_mutex_lock(&tv->lock);
	
	if (!in->expanded) {
		in->expanded = 1;
		if (visible(in)) {
			tv->expandedrows +=
			    count_visible_descendants(&in->children, 0);
			tv->visible.dirty = 1;
		}
	}
	
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewRowCollapse(AG_Tableview *tv, AG_TableviewRow *in)
{
	pthread_mutex_lock(&tv->lock);
	
	if (in->expanded) {
		in->expanded = 0;
		if (visible(in)) {
			tv->expandedrows -=
			    count_visible_descendants(&in->children, 0);
			tv->visible.dirty = 1;
		}
	}
	
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewDestroy(void *p)
{
	AG_Tableview *tv = p;
	u_int i;

	pthread_mutex_lock(&tv->lock);

	AG_TableviewRowDelAll(tv);

	Free(tv->column, M_WIDGET);
	Free(tv->visible.items, M_WIDGET);

	pthread_mutex_unlock(&tv->lock);
	pthread_mutex_destroy(&tv->lock);

	AG_WidgetDestroy(tv);
}

void
AG_TableviewScale(void *p, int w, int h)
{
	AG_Tableview *tv = p;
	u_int rows_per_view, i;

	pthread_mutex_lock(&tv->lock);

	/* estimate but don't change anything */
	if (w == -1 && h == -1) {
		AGWIDGET(tv)->w = tv->prew + tv->sbar_v->button_size;
		AGWIDGET(tv)->h = tv->preh + (NULL == tv->sbar_h ? 0 :
		                tv->sbar_h->button_size);

		for (i = 0; i < tv->columncount; i++) {
			AGWIDGET(tv)->w += tv->column[i].w;
		}
		goto out;
	}
	/* vertical scroll bar */
	AGWIDGET(tv->sbar_v)->x = w - tv->sbar_v->button_size;
	AGWIDGET(tv->sbar_v)->y = 0;
	AG_WidgetScale(tv->sbar_v, -1, h -
	    (tv->sbar_h ? tv->sbar_v->button_size : 0));

	/* horizontal scroll bar, if enabled */
	if (tv->sbar_h != NULL) {
		int col_w = 0;

		AGWIDGET(tv->sbar_h)->x = 0;
		AGWIDGET(tv->sbar_h)->y = h - tv->sbar_v->button_size;
		AG_WidgetScale(tv->sbar_h, w - tv->sbar_v->button_size, -1);

		for (i = 0; i < tv->columncount; i++)
			col_w += tv->column[i].w;

		if (col_w > AGWIDGET(tv->sbar_h)->w) {
			int scroll = col_w - AGWIDGET(tv->sbar_h)->w;

			AG_WidgetSetInt(tv->sbar_h, "max", scroll);
			if (AG_WidgetInt(tv->sbar_h, "value") > scroll) {
				AG_WidgetSetInt(tv->sbar_h, "value", scroll);
			}
			AG_ScrollbarSetBarSize(tv->sbar_h,
			    AGWIDGET(tv->sbar_h)->w *
			    (w - tv->sbar_h->button_size * 3) / col_w);
		} else {
			AG_WidgetSetInt(tv->sbar_h, "value", 0);
			AG_ScrollbarSetBarSize(tv->sbar_h, -1);
		}
	}
	/* calculate widths for AG_TABLEVIEW_COL_FILL cols */
	{
		u_int fill_cols, nonfill_width, fill_width;

		fill_cols = 0;
		nonfill_width = 0;
		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill) {
				fill_cols++;
			} else {
				nonfill_width += tv->column[i].w;
			}
		}

		fill_width = (AGWIDGET(tv)->w - AGWIDGET(tv->sbar_v)->w -
		    nonfill_width) / fill_cols;

		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill)
				tv->column[i].w = fill_width;
		}
	}

	/* Calculate how many rows the view holds */
	{
		int view_height = h - (NULL == tv->sbar_h ? 0 :
				  AGWIDGET(tv->sbar_h)->h);

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
			tv->visible.items[i].row = NULL;

		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	} else if (tv->visible.count > rows_per_view) {
		/* visible area decreased */
		tv->visible.items = Realloc(tv->visible.items,
		    sizeof(struct ag_tableview_rowdocket_item) * rows_per_view);
		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	}
out:
	pthread_mutex_unlock(&tv->lock);
}

void
AG_TableviewDraw(void *p)
{
	AG_Tableview *tv = p;
	u_int i;
	int y, update = 0;
	const int view_width = (AGWIDGET(tv)->w - AGWIDGET(tv->sbar_v)->w);

	pthread_mutex_lock(&tv->lock);
	
	if (tv->visible.count == 0)
		goto out;

	/* before we draw, update if needed */
	if (tv->visible.dirty)
		view_changed(tv);

	if (tv->visible.redraw_rate &&
	    SDL_GetTicks() > tv->visible.redraw_last + tv->visible.redraw_rate)
		update = 1;

	/* Draw the background box */
	agPrim.box(tv, 0, 0, AGWIDGET(tv)->w, AGWIDGET(tv)->h, -1,
	    AG_COLOR(TABLEVIEW_COLOR));
	
	/* draw row selection hilites */
	y = tv->head_height;
	for (i = 0; i < tv->visible.count; i++) {
		if (tv->visible.items[i].row == NULL) {
			break;
		}
		if (tv->visible.items[i].row->selected) {
			agPrim.box(tv,
			    1, y,
			    view_width - 2,
			    tv->row_height,
			    1, AG_COLOR(TABLEVIEW_SEL_COLOR));
		}
		y += tv->row_height;
	}

	/* draw columns */
	foreach_visible_column(tv, draw_column, &update, NULL);

	if (update) {
		tv->visible.redraw_last = SDL_GetTicks();
	}
out:
	pthread_mutex_unlock(&tv->lock);
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
	u_int i;
	int view_width = (AGWIDGET(tv)->w - AGWIDGET(tv->sbar_v)->w);
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
	int view_height = AGWIDGET(tv)->h - (tv->sbar_h != NULL ?
			                   AGWIDGET(tv->sbar_h)->h : 0);
	int scrolling_area = AGWIDGET(tv->sbar_v)->h -
	    tv->sbar_v->button_size*2;
	u_int i;

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
		tv->visible.items[i].row = NULL;

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
	u_int x = 0;

	TAILQ_FOREACH(row, in, siblings) {
		if (*seen) {
			(*seen)--;
		} else {
			if ((filled+x) < tv->visible.count) {
				tv->visible.items[filled+x].row = row;
				tv->visible.items[filled+x].depth = depth;
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

	pthread_mutex_lock(&tv->lock);
	TAILQ_FOREACH(row, &tv->children, siblings) {
		if (row->selected) {
			pthread_mutex_unlock(&tv->lock);
			return (row);
		}
	}
	pthread_mutex_unlock(&tv->lock);
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
switch_columns(AG_Tableview * tv, u_int a, u_int b)
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
render_dyncolumn(AG_Tableview *tv, u_int idx)
{
	char *celltext;
	AG_TableviewRow *row;
	u_int i, cidx = tv->column[idx].idx;

	for (i = 0; i < tv->visible.count; i++) {
		if ((row = tv->visible.items[i].row) == NULL)
			break;
		if (!row->dynamic)
			continue;

		if (row->cell[cidx].image != NULL)
			SDL_FreeSurface(row->cell[cidx].image);

		celltext = tv->data_callback(tv,
		    tv->column[idx].cid,
		    tv->visible.items[i].row->rid);

		row->cell[cidx].image =
		    AG_TextRender(NULL, -1, AG_COLOR(TABLEVIEW_CTXT_COLOR),
		    celltext != NULL ? celltext : "");
	}
}

static int
clicked_header(AG_Tableview *tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	int x = *(int *)arg1;

	if ((x < visible_start || x >= visible_end) &&
	    idx != tv->columncount-1)
		return (1);

	/* click on the CENTER */
	if (x >= visible_start+3 &&
	    x <= visible_end-3) {
		if (tv->reordercols) {
			AG_SchedEvent(NULL, tv, 400, "column-move",
				       "%i, %i", idx, visible_start);
		}
		if (tv->reordercols || tv->sort)
			tv->column[idx].mousedown = 1;
	}
	/* click on the LEFT resize line */
	else if (tv->column[idx-1].resizable &&
		 idx > 0 &&
		 x < visible_start+3) {
		 AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-resize",
		     "%i, %i", idx-1, visible_start - tv->column[idx - 1].w);
	}
	/* click on the RIGHT resize line */
	else if (tv->column[idx].resizable &&
		 (x > visible_end-3 ||
		  (idx == tv->columncount-1 && x < visible_end+3))) {
		AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-resize",
		    "%i, %i", idx, visible_start);
	}
	return (0);
}

static int
clicked_row(AG_Tableview *tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	const int x = *(int *)arg1;
	const int y = *(int *)arg2;
	AG_TableviewRow *row = NULL;
	int depth = 0;
	int ts = tv->row_height/2 + 1;
	SDLMod modifiers = SDL_GetModState();

	if (x < visible_start || x >= visible_end)
		return (1);

	/* find the visible row clicked on */
	{
		u_int row_idx;
		int pix;

		pix = tv->head_height;
		for (row_idx = 0; row_idx < tv->visible.count; row_idx++) {
			if (y >= pix && y < pix+tv->row_height) {
				if (tv->visible.items[row_idx].row == NULL) {
					row_idx = tv->visible.count;
				}
				break;
			}
			pix += tv->row_height;
		}

		if (row_idx != tv->visible.count) {
			row = tv->visible.items[row_idx].row;
			depth = tv->visible.items[row_idx].depth;
		}
	}

	/* clicking on blank space clears the selection */
	if (row == NULL) {
		deselect_all(&tv->children);
		//AG_PostEvent(NULL, tv, "tableview-selectclear", "");
		return (0);
	}

	/* check for a click on the +/- button, if applicable */
	if (tv->column[idx].cid == tv->expanderColumn &&
	    !TAILQ_EMPTY(&row->children) &&
	    x > (visible_start+4+(depth * (ts+4))) &&
	    x < (visible_start+4+(depth * (ts+4))+ts)) {
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
	
	/* handle a command/control click... */
	if ((modifiers & KMOD_META || modifiers & KMOD_CTRL)) {
		/* ...which always disselects a selected row */
		if (row->selected) {
			row->selected = 0;
			AG_PostEvent(NULL, tv, "tableview-deselect", "%p", row);
		}
		/*
		 * ...or selects a row (clearing other selections if not
		 * selmulti)
		 */
		else {
			if (!tv->selmulti) {
				deselect_all(&tv->children);
			}
			row->selected = 1;
			AG_PostEvent(NULL, tv, "tableview-select", "%p", row);
		}
	}
	/* handle a normal click */
	else {
		if (!row->selected) {
			deselect_all(&tv->children);
			row->selected = 1;
			AG_PostEvent(NULL, tv, "tableview-select", "%p", row);
		}
	}

	/* handle double-clicks */
	if (tv->dblclicked) {
		AG_CancelEvent(tv, "dblclick-expire");
		deselect_all(&tv->children);
		row->selected = 1;
		tv->dblclicked = 0;
		AG_PostEvent(NULL, tv, "tableview-dblclick", "%p", row);
	} else {
		tv->dblclicked++;
		AG_SchedEvent(NULL, tv, agMouseDblclickDelay,
		    "dblclick-expire", "");
	}
	return (0);
}

static int
draw_column(AG_Tableview *tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	const int view_width = (AGWIDGET(tv)->w - AGWIDGET(tv->sbar_v)->w);
	const int *update = (int *)arg1;
	u_int j, cidx = tv->column[idx].idx;
	int y;

	/* draw label for this column */
	if (tv->header) {
		int label_x;

		label_x = tv->column[idx].w/2 - tv->column[idx].label_img->w/2;
		agPrim.box(tv, visible_start, 0, tv->column[idx].w,
		    tv->head_height,
		    (tv->column[idx].mousedown ||
		     tv->column[idx].cid == tv->sortColumn) ? -1 : 1,
		    AG_COLOR(TABLEVIEW_HEAD_COLOR));
		AG_WidgetBlitSurface(tv, tv->column[idx].label_id,
		    visible_start + label_x,
		    0);
	}
	/* check for the need to update */
	if (*update && tv->column[idx].update)
		render_dyncolumn(tv, idx);

	/* draw cells in this column */
	y = tv->head_height;
	for (j = 0; j < tv->visible.count; j++) {
		int offset = visible_start+4;

		if (tv->visible.items[j].row == NULL) {
			break;
		}
		if (tv->column[idx].cid == tv->expanderColumn) {
			int tw = tv->row_height/2 + 1;
			int ty = y + tw/2;

			offset += tv->visible.items[j].depth * (tw+4);

			if (!TAILQ_EMPTY(&tv->visible.items[j].row->children)) {
				Uint8 c[4] = { 255, 255, 255, 128 };

				agPrim.frame(tv, offset, ty, tw, tw,
				    AG_COLOR(TABLEVIEW_LINE_COLOR));
				if (tv->visible.items[j].row->expanded) {
					agPrim.minus(tv, offset, ty,
					    tw - 2,
					    tw - 2,
					    c, AG_ALPHA_SRC);
				} else {
					agPrim.plus(tv, offset, ty,
					    tw - 2,
					    tw - 2,
					    c, AG_ALPHA_SRC);
				}
			}
			offset += tw+4;
		}
		if (tv->visible.items[j].row->cell[cidx].image) {
			/* XXX inefficient in opengl mode */
			AG_WidgetBlit(tv,
			    tv->visible.items[j].row->cell[cidx].image,
			    offset, y+1);
		}
		y += tv->row_height;
	}

	/* Fill the Remaining Space in column heading */
	if (tv->header &&
	    idx == tv->columncount-1 &&
	    visible_end < view_width) {
		agPrim.box(tv,
		    visible_end,
		    0,
		    view_width - visible_end,
		    tv->head_height,
		    1,
		    AG_COLOR(TABLEVIEW_HEAD_COLOR));
	}
	return (1);
}

/*
 * *********************** EVENT HANDLERS **********************
 */

static void
mousebuttonup(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[0].p;
	int coord_x = argv[2].i, coord_y = argv[3].i;
	int left;
	u_int i;

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
mousebuttondown(int argc, union evarg * argv)
{
	AG_Tableview *tv = argv[0].p;
	int coord_x = argv[2].i;
	int coord_y = argv[3].i;

	AG_WidgetFocus(tv);

	pthread_mutex_lock(&tv->lock);
	if (tv->header && coord_y < tv->head_height) {
		/* a mouse down on the column header */
		foreach_visible_column(tv, clicked_header, &coord_x, NULL);
	} else {
		/* a mouse down in the body */
		foreach_visible_column(tv, clicked_row, &coord_x, &coord_y);
	}
	pthread_mutex_unlock(&tv->lock);
}

static void
scrolled(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[1].p;

	tv->visible.dirty = 1;
}

static void
dblclick_expire(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[0].p;

	pthread_mutex_lock(&tv->lock);

	/* the user hasn't clicked again, so cancel the double click */
	tv->dblclicked = 0;

	/* XXX - if the cursor remains in the cell, activate a click-to-edit */

	pthread_mutex_unlock(&tv->lock);
}

/* XXX - this seems to be called after every click.. */
static void
lost_focus(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[0].p;

	//AG_CancelEvent(tv, "key-tick");
	//AG_CancelEvent(tv, "dblclick-expire");
	//tv->dblclicked = 0;
	//tv->keymoved = 0;
}

static void
columnresize(int argc, union evarg *argv)
{
	AG_Tableview *tv = argv[0].p;
	int col = argv[1].i, left = argv[2].i;
	int x;

	AG_MouseGetState(&x, NULL);
	x -= AGWIDGET(tv)->cx;
	tv->column[col].w = (x-left > tv->column[col].label_img->w) ?
	    (x-left) : tv->column[col].label_img->w;

	AG_ReschedEvent(tv, "column-resize", agMouseDblclickDelay);
}

static void
columnmove(int argc, union evarg * argv)
{
	AG_Tableview *tv = argv[0].p;
	u_int col = (u_int)argv[1].i;
	int left = argv[2].i;
	int x;

	/*
	 * if we haven't mouseupped by the time this event is called, set
	 * moving to indicate we won't sort.
	 */
	tv->column[col].moving = 1;

	AG_MouseGetState(&x, NULL);
	x -= AGWIDGET(tv)->cx;

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
	AG_SchedEvent(NULL, tv, agMouseDblclickDelay, "column-move",
	    "%i, %i", col, left);
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
	vasprintf(&row->cell[cell].text, fmt, args);
	va_end(args);

	row->cell[cell].image = AG_TextRender(NULL, -1,
	    AG_COLOR(TABLEVIEW_CTXT_COLOR),
	    row->cell[cell].text);
}
