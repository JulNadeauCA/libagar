/*	$Csoft: tableview.c,v 1.9 2004/11/30 11:36:54 vedge Exp $	*/

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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/scrollbar.h>
/* #include <engine/widget/textbox.h> */

#include <string.h>
#include <stdarg.h>

#include "tableview.h"

#define ID_INVALID ((unsigned int)-1)

static struct widget_ops tableview_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		tableview_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	tableview_draw,
	tableview_scale
};

enum {
	TEXT_COLOR,
	BG_COLOR,
	LINE_COLOR,
	SELECTION_COLOR,
	HEAD_COLOR,
	HEADTEXT_COLOR
};

/*
 * Worker function for foreach_visible_column()
 * arguments: tableview, visible_start, visible_end,
 * visible_index, and the argument passed to foreach_.
 * Should return 0 if foreach_ should stop.
 */
typedef int (*visible_do)(struct tableview *, int vis_start, int vis_end,
		          Uint32 vis_index, void *, void *);

static void foreach_visible_column(struct tableview *, visible_do, void *,
                                   void *);
static void view_changed(struct tableview *);
static int view_changed_check(struct tableview *, struct tableview_rowq *,
			      int, int, int *);
static struct tableview_row *row_get(struct tableview_rowq *, rowID);
static void select_all(struct tableview_rowq *);
static void deselect_all(struct tableview_rowq *);
static int count_visible_descendants(struct tableview_rowq *, int);
static void switch_columns(struct tableview *, unsigned int, unsigned int);
static int visible(struct tableview_row *);
static void render_dyncolumn(struct tableview *, unsigned int);
static int clicked_header(struct tableview *, int, int, Uint32, void *, void *);
static int clicked_row(struct tableview *, int, int, Uint32, void *, void *);
static int draw_column(struct tableview *, int, int, Uint32, void *, void *);


struct tableview *
tableview_new(void *parent, int flags, datafunc data_callback,
    compfunc sort_callback)
{
	struct tableview *tv;

	tv = Malloc(sizeof(struct tableview), M_WIDGET);
	tableview_init(tv, flags, data_callback, sort_callback);
	object_attach(parent, tv);

	return (tv);
}

void
tableview_init(struct tableview * tv, int flags, datafunc data_callback,
    compfunc sort_callback)
{
	static void mousebuttonup(int, union evarg *);
	static void mousebuttondown(int, union evarg *);
	static void scrolled(int, union evarg *);
	static void dblclick_expire(int, union evarg *);
	static void lost_focus(int, union evarg *);
	static void columnresize(int, union evarg *);
	static void columnmove(int argc, union evarg *);

	widget_init(tv, "tableview", &tableview_ops,
	  WIDGET_FOCUSABLE | WIDGET_CLIPPING | WIDGET_WFILL | WIDGET_HFILL);
	pthread_mutex_init(&tv->lock, &recursive_mutexattr);

	widget_map_color(tv, BG_COLOR, "background", 120, 120, 120, 255);
	widget_map_color(tv, HEAD_COLOR, "header", 180, 180, 180, 255);
	widget_map_color(tv, HEADTEXT_COLOR, "headertext", 0, 0, 0, 255);
	widget_map_color(tv, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(tv, LINE_COLOR, "line", 50, 50, 50, 255);
	widget_map_color(tv, SELECTION_COLOR, "selection", 50, 50, 120, 255);

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

	/* set requested flags */
	if (flags & TABLEVIEW_REORDERCOLS)
		tv->reordercols = 1;
	if (!(flags & TABLEVIEW_NOHEADER))
		tv->header = 1;
	if (!(flags & TABLEVIEW_NOSORT))
		tv->sort = 1;
	if (flags & TABLEVIEW_REORDERCOLS)
		tv->reordercols = 1;
	if (flags & TABLEVIEW_SELMULTI)
		tv->selmulti = 1;

	tv->head_height = tv->header ? text_font_height : 0;
	tv->row_height = text_font_height+2;
	tv->dblclicked = 0;
	tv->sbar_v = scrollbar_new(tv, SCROLLBAR_VERT);
	tv->sbar_h = (flags & TABLEVIEW_HORIZ) ?
		scrollbar_new(tv, SCROLLBAR_HORIZ) : NULL;
	//tv->editbox = NULL;

	widget_set_int(tv->sbar_v, "min", 0);
	widget_set_int(tv->sbar_v, "max", 0);
	widget_set_int(tv->sbar_v, "value", 0);

	if (tv->sbar_h) {
		widget_set_int(tv->sbar_h, "min", 0);
		widget_set_int(tv->sbar_h, "max", 0);
		widget_set_int(tv->sbar_h, "value", 0);
	}
	tv->column = NULL;
	tv->columncount = 0;
	tv->sortMode = tableview_sort_not;

	tv->sortColumn = ID_INVALID;
	tv->enterEdit = ID_INVALID;
	tv->expanderColumn = ID_INVALID;

	TAILQ_INIT(&tv->children);
	tv->expandedrows = 0;

	tv->visible.redraw_rate = 0;
	tv->visible.redraw_last = SDL_GetTicks();
	tv->visible.count = 0;
	tv->visible.items = NULL;

	tableview_prescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZ", 5);

	/*
	 * XXX - implement key selection / scroll changing event_new(tv,
	 * "window-keydown", lvist_keydown, NULL); event_new(tv,
	 * "window-keyup", lvist_keyup, NULL); event_new(tv, "key-tick",
	 * key_tick, NULL);
	 */

	/* private, internal events */
	event_new(tv, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(tv, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(tv->sbar_v, "scrollbar-changed", scrolled, "%p", tv);
	event_new(tv, "dblclick-expire", dblclick_expire, NULL);
	event_new(tv, "widget-lostfocus", lost_focus, NULL);
	event_new(tv, "widget-hidden", lost_focus, NULL);

	event_new(tv, "column-resize", columnresize, NULL);
	event_new(tv, "column-move", columnmove, NULL);
}

void
tableview_prescale(struct tableview * tv, const char *text, int nitems)
{
	text_prescale(text, &tv->prew, NULL);
	tv->preh = tv->head_height + (tv->row_height * nitems);
}

void
tableview_col_add(struct tableview * tv, int flags, colID cid,
    const char *label, char *width)
{
	struct tableview_column *col;
	unsigned int i;
	int data_w, label_w;

	if (tv->locked || cid == ID_INVALID)
		return;

	pthread_mutex_lock(&tv->lock);

	/* column identifier must be unique */
	for (i = 0; i < tv->columncount; i++) {
		if (tv->column[i].cid == cid)
			goto out;
	}

	tv->column = Realloc(tv->column,
		   (tv->columncount + 1) * sizeof(struct tableview_column));
	col = &tv->column[tv->columncount];
	col->cid = cid;
	col->idx = tv->columncount;
	col->mousedown = 0;
	tv->columncount++;

	if (1 == tv->columncount)
		tv->expanderColumn = cid;

	/* initialize flags */
	col->mousedown = 0;
	col->moving = 0;
	col->editable = 0;
	col->resizable = 0;
	col->update = 0;
	col->fill = 0;
	col->dynamic = 0;

	/* set requested flags */
	if (flags & TABLEVIEW_COL_EDITABLE)
		col->editable = 1;
	if (flags & TABLEVIEW_COL_ENTEREDIT)
		tv->enterEdit = cid;
	if (flags & TABLEVIEW_COL_RESIZABLE)
		col->resizable = 1;
	if ((flags & TABLEVIEW_COL_UPDATE) &&
	    (flags & TABLEVIEW_COL_DYNAMIC))
		col->update = 1;
	if (flags & TABLEVIEW_COL_FILL)
		col->fill = 1;
	if (flags & TABLEVIEW_COL_DYNAMIC)
		col->dynamic = 1;
	if (flags & TABLEVIEW_COL_EXPANDER)
		tv->expanderColumn = cid;

	/* column label */
	if (NULL == label) {
		label = "";
	}
	strncpy(col->label, label, TABLEVIEW_LABEL_MAX);
	col->label_img = text_render(NULL, -1, WIDGET_COLOR(tv, HEADTEXT_COLOR),
	    label);
	label_w = col->label_img->w;

	/* column's width is the wider of user width string and label string */
	if (NULL == width) {
		width = "";
	}
	text_prescale(width, &data_w, NULL);
	col->w = (label_w > data_w ? label_w : data_w) + 6;

	tv->visible.dirty = 1;

out:
	pthread_mutex_unlock(&tv->lock);
}

void
tableview_set_update(struct tableview * tv, unsigned int ms)
{
	pthread_mutex_lock(&tv->lock);
	tv->visible.redraw_rate = ms;
	pthread_mutex_unlock(&tv->lock);
}

struct tableview_row *
tableview_row_addfn(struct tableview * tv, int flags,
    struct tableview_row * parent, rowID rid,...)
{
	struct tableview_row *row;
	unsigned int i;
	va_list ap;
	colID cid;
	void *data;

	pthread_mutex_lock(&tv->lock);

	tv->locked = 1;

	/* verify the row identifier is not in use */
	if (row_get(&tv->children, rid)) {
		pthread_mutex_unlock(&tv->lock);
		return (NULL);
	}
	row = Malloc(sizeof(struct tableview_row), M_WIDGET);
	row->cell = Malloc(sizeof(struct cell) * tv->columncount, M_WIDGET);

	/* initialize cells */
	for (i = 0; i < tv->columncount; i++) {
		row->cell[i].text = NULL;
		row->cell[i].image = NULL;
	}

	/* import static data */
	va_start(ap, rid);
	while (-1 != (cid = va_arg(ap, colID))) {
		data = va_arg(ap, void *);

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
				row->cell[i].text = Strdup((char *) data);
				row->cell[i].image = text_render(NULL, -1,
				    WIDGET_COLOR(tv, TEXT_COLOR),
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
		++tv->expandedrows;

	tv->visible.dirty = 1;

	pthread_mutex_unlock(&tv->lock);
	return (row);
}

void
tableview_row_del(struct tableview * tv, struct tableview_row * row)
{
	unsigned int i;
	struct tableview_row *row1, *row2;

	if (NULL == row)
		return;

	pthread_mutex_lock(&tv->lock);

	/* first remove children */

	row1 = TAILQ_FIRST(&row->children);
	while (row1) {
		row2 = TAILQ_NEXT(row1, siblings);
		tableview_row_del(tv, row1);
		row1 = row2;
	}

	/* now that children are gone, remove this row */
	if (visible(row))
		--tv->expandedrows;

	if (row->parent) {
		TAILQ_REMOVE(&row->parent->children, row, siblings);
	} else {
		TAILQ_REMOVE(&tv->children, row, siblings);
	}

	for (i = 0; i < tv->columncount; i++) {
		SDL_FreeSurface(row->cell[i].image);
		free(row->cell[i].text);
	}

	Free(row->cell, M_WIDGET);
	Free(row, M_WIDGET);

	tv->visible.dirty = 1;

	pthread_mutex_unlock(&tv->lock);
}

void
tableview_row_del_all(struct tableview * tv)
{
	struct tableview_row *row1, *row2;

	pthread_mutex_lock(&tv->lock);

	row1 = TAILQ_FIRST(&tv->children);
	while (row1) {
		row2 = TAILQ_NEXT(row1, siblings);
		tableview_row_del(tv, row1);
		row1 = row2;
	}

	TAILQ_INIT(&tv->children);
	tv->expandedrows = 0;
	tv->visible.dirty = 1;

	pthread_mutex_unlock(&tv->lock);
}

void
tableview_row_select(struct tableview * tv, struct tableview_row * row)
{
	pthread_mutex_lock(&tv->lock);
	
	if (!tv->selmulti) {
		tableview_row_deselect_all(tv, NULL);
	}
	row->selected = 1;
	
	pthread_mutex_unlock(&tv->lock);
}

struct tableview_row *
tableview_row_get(struct tableview * tv, rowID rid)
{
	struct tableview_row *row;

	/* XXX pointless lock */
	pthread_mutex_lock(&tv->lock);
	row = row_get(&tv->children, rid);
	pthread_mutex_unlock(&tv->lock);

	return (row);
}

void
tableview_row_select_all(struct tableview * tv, struct tableview_row * root)
{
	if (!tv->selmulti)
		return;

	pthread_mutex_lock(&tv->lock);
	if (NULL == root) {
		select_all(&tv->children);
	} else {
		select_all(&root->children);
	}
	pthread_mutex_unlock(&tv->lock);
}

void
tableview_row_deselect_all(struct tableview * tv, struct tableview_row * root)
{
	pthread_mutex_lock(&tv->lock);
	
	if (NULL == root) {
		deselect_all(&tv->children);
	} else {
		deselect_all(&root->children);
	}
	
	pthread_mutex_unlock(&tv->lock);
}

void
tableview_row_expand(struct tableview * tv, struct tableview_row * in)
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
tableview_row_collapse(struct tableview * tv, struct tableview_row * in)
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

/*
 * *********************** EXTERNAL FUNCTIONS **********************
 */

void
tableview_destroy(void *p)
{
	struct tableview *tv = p;
	unsigned int i;

	pthread_mutex_lock(&tv->lock);

	tableview_row_del_all(tv);

	for (i = 0; i < tv->columncount; i++) {
		SDL_FreeSurface(tv->column[i].label_img);
	}
	Free(tv->column, M_WIDGET);
	Free(tv->visible.items, M_WIDGET);

	pthread_mutex_unlock(&tv->lock);
	pthread_mutex_destroy(&tv->lock);

	widget_destroy(tv);
}

void
tableview_scale(void *p, int w, int h)
{
	struct tableview *tv = p;
	unsigned int rows_per_view, i;

	pthread_mutex_lock(&tv->lock);

	/* estimate but don't change anything */
	if (w == -1 && h == -1) {
		WIDGET(tv)->w = tv->prew + tv->sbar_v->button_size;
		WIDGET(tv)->h = tv->preh + (NULL == tv->sbar_h ? 0 :
		                tv->sbar_h->button_size);
		pthread_mutex_unlock(&tv->lock);
		return;
	}
	/* vertical scroll bar */
	WIDGET(tv->sbar_v)->x = w - tv->sbar_v->button_size;
	WIDGET(tv->sbar_v)->y = 0;
	widget_scale(tv->sbar_v, -1, h -
	    (tv->sbar_h ? tv->sbar_v->button_size : 0));

	/* horizontal scroll bar, if enabled */
	if (NULL != tv->sbar_h) {
		int col_w = 0;

		WIDGET(tv->sbar_h)->x = 0;
		WIDGET(tv->sbar_h)->y = h - tv->sbar_v->button_size;
		widget_scale(tv->sbar_h, w - tv->sbar_v->button_size, -1);

		for (i = 0; i < tv->columncount; i++)
			col_w += tv->column[i].w;

		if (col_w > WIDGET(tv->sbar_h)->w) {
			int scroll = col_w - WIDGET(tv->sbar_h)->w;

			widget_set_int(tv->sbar_h, "max", scroll);
			if (widget_get_int(tv->sbar_h, "value") > scroll) {
				widget_set_int(tv->sbar_h, "value", scroll);
			}
			scrollbar_set_bar_size(tv->sbar_h,
			    WIDGET(tv->sbar_h)->w *
			    (w - tv->sbar_h->button_size * 3) / col_w);
		} else {
			widget_set_int(tv->sbar_h, "value", 0);
			scrollbar_set_bar_size(tv->sbar_h, -1);
		}
	}
	/* calculate widths for TABLEVIEW_COL_FILL cols */
	{
		unsigned int fill_cols, nonfill_width, fill_width;

		fill_cols = 0;
		nonfill_width = 0;
		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill) {
				++fill_cols;
			} else {
				nonfill_width += tv->column[i].w;
			}
		}

		fill_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w -
		    nonfill_width) / fill_cols;

		for (i = 0; i < tv->columncount; i++) {
			if (tv->column[i].fill)
				tv->column[i].w = fill_width;
		}
	}

	/* Calculate how many rows the view holds */
	{
		int view_height = h - (NULL == tv->sbar_h ? 0 :
				  WIDGET(tv->sbar_h)->h);

		if (view_height < tv->head_height) {
			view_height = tv->head_height;
		}
		rows_per_view = (view_height - tv->head_height) /
		    tv->row_height;

		if ((view_height - tv->head_height) % tv->row_height)
			++rows_per_view;
	}

	if (tv->visible.count < rows_per_view) {
		/* visible area increased */
		tv->visible.items = Realloc(tv->visible.items,
		    sizeof(struct rowdocket_item) * rows_per_view);

		for (i = tv->visible.count; i < rows_per_view; i++)
			tv->visible.items[i].row = NULL;

		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	} else if (tv->visible.count > rows_per_view) {
		/* visible area decreased */
		tv->visible.items = Realloc(tv->visible.items,
		    sizeof(struct rowdocket_item) * rows_per_view);
		tv->visible.count = rows_per_view;
		tv->visible.dirty = 1;
	}
	pthread_mutex_unlock(&tv->lock);
}

void
tableview_draw(void *p)
{
	struct tableview *tv = p;
	unsigned int i;
	int y, update = 0;
	const int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);

	if (WIDGET(tv)->w < 26 || WIDGET(tv)->h < 5)
		return;

	pthread_mutex_lock(&tv->lock);

	/* before we draw, update if needed */
	if (tv->visible.dirty)
		view_changed(tv);

	if (tv->visible.redraw_rate &&
	    SDL_GetTicks() > tv->visible.redraw_last + tv->visible.redraw_rate)
		update = 1;

	/* Draw the background box */
	primitives.box(tv, 0, 0, view_width, WIDGET(tv)->h, -1, BG_COLOR);

	/* draw row selection hilites */
	y = tv->head_height;
	for (i = 0; i < tv->visible.count; i++) {
		if (NULL == tv->visible.items[i].row) {
			break;
		}
		if (tv->visible.items[i].row->selected) {
			primitives.box(tv, 1, y, view_width - 2,
			    tv->row_height, 1, SELECTION_COLOR);
		}
		y += tv->row_height;
	}

	/* draw columns */
	foreach_visible_column(tv, draw_column, &update, NULL);

	if (update)
		tv->visible.redraw_last = SDL_GetTicks();

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
foreach_visible_column(struct tableview * tv, visible_do dothis,
    void *arg1, void *arg2)
{
	int x, first_col, col_width;
	unsigned int i;
	int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);
	int view_edge = (tv->sbar_h ? widget_get_int(tv->sbar_h, "value") : 0);

	x = 0;
	first_col = -1;
	for (i = 0; i < tv->columncount; i++) {
		/* skip until we find an onscreen column */
		if (-1 == first_col &&
		    x + tv->column[i].w < view_edge) {
			x += tv->column[i].w;	/* x represents the offset in
						 * the table */
			continue;
		}
		/* OK, we found the first onscreen column */
		else if (-1 == first_col) {
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
		if (0 == dothis(tv, x, x + col_width, i, arg1, arg2))
			return;

		x += tv->column[i].w;
	}
}

/*
 * called after any addition or removal of visible rows. Rebuilds the array
 * of rows to be drawn (tv->visible)
 */
static void
view_changed(struct tableview * tv)
{
	int rows_per_view, max, filled, value;
	int view_height =
	WIDGET(tv)->h - (NULL == tv->sbar_h ? 0 : WIDGET(tv->sbar_h)->h);
	int scrolling_area = WIDGET(tv->sbar_v)->h - tv->sbar_v->button_size*2;
	unsigned int i;

	/* cancel double clicks if what's under it changes it */
	tv->dblclicked = 0;
	event_cancel(tv, "dblclick-expire");

	rows_per_view = (view_height - tv->head_height) / tv->row_height;
	if ((view_height - tv->head_height) % tv->row_height)
		++rows_per_view;

	max = tv->expandedrows - rows_per_view;
	if (max < 0) {
		max = 0;
	}
	if (max && ((view_height - tv->head_height) % tv->row_height) < 16) {
		++max;
	}
	widget_set_int(tv->sbar_v, "max", max);
	if (widget_get_int(tv->sbar_v, "value") > max)
		widget_set_int(tv->sbar_v, "value", max);

	/* Calculate Scrollbar Size */
	if (rows_per_view && tv->expandedrows > rows_per_view)
		scrollbar_set_bar_size(tv->sbar_v,
		    rows_per_view * scrolling_area / tv->expandedrows);
	else
		scrollbar_set_bar_size(tv->sbar_v, -1);

	/* locate visible rows */
	value = widget_get_int(tv->sbar_v, "value");
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
view_changed_check(struct tableview * tv, struct tableview_rowq * in, int depth,
    int filled, int *seen)
{
	struct tableview_row *row;
	unsigned int x = 0;

	TAILQ_FOREACH(row, in, siblings) {
		if (*seen) {
			--(*seen);
		} else {
			tv->visible.items[filled + x].row = row;
			tv->visible.items[filled + x].depth = depth;
			++x;
		}

		if (row->expanded && filled + x < tv->visible.count)
			x += view_changed_check(tv, &row->children, depth + 1,
			    filled + x, seen);

		if (filled + x >= tv->visible.count)
			return (x);
	}

	return (x);
}

/*
 * return a pointer if the row with the given identifer exists in or in a
 * descendant of the rowq. If it does not exist, return NULL
 */
static struct tableview_row *
row_get(struct tableview_rowq * searchIn, rowID rid)
{
	struct tableview_row *row, *row2;

	TAILQ_FOREACH(row, searchIn, siblings) {
		if (row->rid == rid)
			return (row);

		if (!TAILQ_EMPTY(&row->children)) {
			row2 = row_get(&row->children, rid);
			if (row2)
				return (row2);
		}
	}
	return (NULL);
}

/* set the selection bit for all rows in and descended from the rowq */
static void
select_all(struct tableview_rowq * children)
{
	struct tableview_row *row;

	TAILQ_FOREACH(row, children, siblings) {
		row->selected = 1;
		if (row->expanded && !TAILQ_EMPTY(&row->children))
			select_all(&row->children);
	}
}

/* clear the selection bit for all rows in and descended from the rowq */
static void
deselect_all(struct tableview_rowq * children)
{
	struct tableview_row *row;

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
count_visible_descendants(struct tableview_rowq * in, int i)
{
	struct tableview_row *row;
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
switch_columns(struct tableview * tv, unsigned int a, unsigned int b)
{
	struct tableview_column col_tmp;

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
visible(struct tableview_row * in)
{
	struct tableview_row *row = in->parent;

	while (NULL != row) {
		if (!row->expanded) {
			return (0);
		}
		row = row->parent;
	}
	return (1);
}

static void
render_dyncolumn(struct tableview * tv, unsigned int idx)
{
	char *celltext;
	struct tableview_row *row;
	unsigned int i, cidx = tv->column[idx].idx;

	for (i = 0; i < tv->visible.count; i++) {
		if (NULL == (row = tv->visible.items[i].row))
			break;

		if (row->cell[cidx].image)
			SDL_FreeSurface(row->cell[cidx].image);

		celltext = tv->data_callback(tv->column[idx].cid,
		    tv->visible.items[i].row->rid);

		row->cell[cidx].image =
		    text_render(NULL, -1, WIDGET_COLOR(tv, TEXT_COLOR),
		    celltext);
	}
}

static int
clicked_header(struct tableview * tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	int x = *(int *) arg1;

	if ((x < visible_start || x >= visible_end) &&
	    idx != tv->columncount - 1)
		return (1);

	/* click on the CENTER */
	if (x >= visible_start + 3 &&
	    x <= visible_end - 3) {
		if (tv->reordercols) {
			event_schedule(NULL, tv, 400, "column-move",
				       "%i, %i", idx, visible_start);
		}
		if (tv->reordercols || tv->sort)
			tv->column[idx].mousedown = 1;
	}
	/* click on the LEFT resize line */
	else if (tv->column[idx - 1].resizable &&
		 idx > 0 &&
		 x < visible_start + 3) {
		 event_schedule(NULL, tv, mouse_dblclick_delay, "column-resize",
		     "%i, %i", idx - 1, visible_start - tv->column[idx - 1].w);
	}
	/* click on the RIGHT resize line */
	else if (tv->column[idx].resizable &&
		 (x > visible_end - 3 ||
		  (idx == tv->columncount - 1 && x < visible_end + 3))) {
		event_schedule(NULL, tv, mouse_dblclick_delay, "column-resize",
		    "%i, %i", idx, visible_start);
	}
	return (0);
}

static int
clicked_row(struct tableview * tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	const int x = *(int *) arg1;
	const int y = *(int *) arg2;
	struct tableview_row *row = NULL;
	int depth = 0;
	int ts = tv->row_height / 2 + 1;
	SDLMod modifiers = SDL_GetModState();

	if (x < visible_start || x >= visible_end)
		return (1);

	/* find the visible row clicked on */
	{
		unsigned int row_idx;
		int pix;

		pix = tv->head_height;
		for (row_idx = 0; row_idx < tv->visible.count; row_idx++) {
			if (y >= pix && y < pix + tv->row_height) {
				if (NULL == tv->visible.items[row_idx].row)
					row_idx = tv->visible.count;
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
	if (NULL == row) {
		deselect_all(&tv->children);
		//event_post(NULL, tv, "tableview-selectclear", "");
		return (0);
	}

	/* check for a click on the +/- button, if applicable */
	if (tv->column[idx].cid == tv->expanderColumn &&
	    !TAILQ_EMPTY(&row->children) &&
	    x > (visible_start + 4 + (depth * (ts + 4))) &&
	    x < (visible_start + 4 + (depth * (ts + 4)) + ts)) {
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
			event_post(NULL, tv, "tableview-deselect", "%p", row);
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
			event_post(NULL, tv, "tableview-select", "%p", row);
		}
	}
	/* handle a normal click */
	else {
		if (!row->selected) {
			deselect_all(&tv->children);
			row->selected = 1;
			event_post(NULL, tv, "tableview-select", "%p", row);
		}
	}

	/* handle double-clicks */
	if (tv->dblclicked) {
		event_cancel(tv, "dblclick-expire");
		deselect_all(&tv->children);
		row->selected = 1;
		tv->dblclicked = 0;
		event_post(NULL, tv, "tableview-dblclick", "%p", row);
	} else {
		tv->dblclicked++;
		event_schedule(NULL, tv, mouse_dblclick_delay,
		    "dblclick-expire", "");
	}

	return (0);
}

static int
draw_column(struct tableview * tv, int visible_start, int visible_end,
    Uint32 idx, void *arg1, void *arg2)
{
	const int view_width = (WIDGET(tv)->w - WIDGET(tv->sbar_v)->w);
	const int *update = (int *) arg1;
	unsigned int j, cidx = tv->column[idx].idx;
	int y;

	/* draw label for this column */
	if (tv->header) {
		int label_x;
		label_x = tv->column[idx].w/2 - tv->column[idx].label_img->w/2;
		primitives.box(tv, visible_start, 0, tv->column[idx].w,
		    tv->head_height,
		    (tv->column[idx].mousedown ||
		     tv->column[idx].cid == tv->sortColumn) ? -1 : 1,
		    HEAD_COLOR);
		widget_blit(tv, tv->column[idx].label_img,
		    visible_start + label_x,
		    0);
	}
	/* check for the need to update */
	if (*update && tv->column[idx].update)
		render_dyncolumn(tv, idx);

	/* draw cells in this column */
	y = tv->head_height;
	for (j = 0; j < tv->visible.count; j++) {
		int offset = visible_start + 4;

		if (NULL == tv->visible.items[j].row) {
			break;
		}
		if (tv->column[idx].cid == tv->expanderColumn) {
			int tw = tv->row_height / 2 + 1;
			int ty = y + tw / 2;

			offset += tv->visible.items[j].depth * (tw + 4);

			if (!TAILQ_EMPTY(&tv->visible.items[j].row->children)) {
				primitives.frame(tv, offset, ty, tw, tw,
				    LINE_COLOR);
				if (tv->visible.items[j].row->expanded) {
					primitives.minus(tv, offset, ty,
					    tw - 2,
					    tw - 2,
					    LINE_COLOR);
				} else {
					primitives.plus(tv, offset, ty,
					    tw - 2,
					    tw - 2,
					    LINE_COLOR);
				}
			}
			offset += tw + 4;
		}
		if (tv->visible.items[j].row->cell[cidx].image) {
			widget_blit(tv,
			    tv->visible.items[j].row->cell[cidx].image,
			    offset, y + 1);
		}
		y += tv->row_height;
	}

	/* Fill the Remaining Space in column heading */
	if (tv->header &&
	    idx == tv->columncount - 1 &&
	    visible_end < view_width) {
		primitives.box(tv,
		    visible_end,
		    0,
		    view_width - visible_end,
		    tv->head_height,
		    1,
		    HEAD_COLOR);
	}
	return (1);
}

/*
 * *********************** EVENT HANDLERS **********************
 */

static void
mousebuttonup(int argc, union evarg * argv)
{
	struct tableview *tv = argv[0].p;
	int coord_x = argv[2].i, coord_y = argv[3].i;
	int left;
	unsigned int i;

	event_cancel(tv, "column-resize");
	event_cancel(tv, "column-move");

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
		    coord_x < (left + tv->column[i].w)) {
			/* toggle the sort mode */
			if (tv->sortColumn == ID_INVALID ||
			    tv->sortMode == tableview_sort_dsc) {
				tv->sortMode = tableview_sort_asc;
			} else {
				tv->sortMode = tableview_sort_dsc;
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
	struct tableview *tv = argv[0].p;
	int coord_x = argv[2].i, coord_y = argv[3].i;

	widget_focus(tv);

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
scrolled(int argc, union evarg * argv)
{
	((struct tableview *) argv[1].p)->visible.dirty = 1;
}

static void
dblclick_expire(int argc, union evarg * argv)
{
	struct tableview *tv = argv[0].p;

	pthread_mutex_lock(&tv->lock);

	/* the user hasn't clicked again, so cancel the double click */
	tv->dblclicked = 0;

	/* XXX - if the cursor remains in the cell, activate a click-to-edit */

	pthread_mutex_unlock(&tv->lock);
}

/* XXX - this seems to be called after every click.. */
static void
lost_focus(int argc, union evarg * argv)
{
	struct tableview *tv = argv[0].p;

	//event_cancel(tv, "key-tick");
	//event_cancel(tv, "dblclick-expire");
	//tv->dblclicked = 0;
	//tv->keymoved = 0;
}

static void
columnresize(int argc, union evarg * argv)
{
	struct tableview *tv = argv[0].p;
	int col = argv[1].i, left = argv[2].i;
	int x;

	SDL_GetMouseState(&x, NULL);
	x -= WIDGET(tv)->cx;

	tv->column[col].w = (x - left > tv->column[col].label_img->w) ?
	    (x - left) : tv->column[col].label_img->w;

	event_resched(tv, "column-resize", mouse_dblclick_delay);
}

static void
columnmove(int argc, union evarg * argv)
{
	struct tableview *tv = argv[0].p;
	unsigned int col = (unsigned int) argv[1].i;
	int left = argv[2].i;
	int x;

	/*
	 * if we haven't mouseupped by the time this event is called, set
	 * moving to indicate we won't sort.
	 */
	tv->column[col].moving = 1;

	SDL_GetMouseState(&x, NULL);
	x -= WIDGET(tv)->cx;

	/* dragging to the left */
	if ((tv->column[col].w < tv->column[col - 1].w &&
	     x < left - tv->column[col - 1].w + tv->column[col].w) ||
	    (tv->column[col].w >= tv->column[col - 1].w &&
	     x < left)) {
		if (col > 0) {
			left -= tv->column[col - 1].w;
			switch_columns(tv, col, col - 1);
			--col;
		}
	}
	/* dragging to the right */
	else if ((tv->column[col].w <= tv->column[col + 1].w &&
		  x > left + tv->column[col + 1].w) ||
		 (tv->column[col].w > tv->column[col + 1].w &&
		  x > left + tv->column[col].w)) {
		if (col < tv->columncount - 1) {
			left += tv->column[col + 1].w;
			switch_columns(tv, col, col + 1);
			++col;
		}
	}
	event_schedule(NULL, tv, mouse_dblclick_delay, "column-move",
	    "%i, %i", col, left);
}
