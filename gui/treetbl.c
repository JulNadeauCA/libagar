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

/*
 * Tree-based table widget. This is a table display (comparable to the
 * AG_Table(3) widget), but its rows are instead stored in a tree structure
 * and rows and columns are identified numerically.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/treetbl.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

#include <string.h>
#include <stdarg.h>

/*
 * Worker function for FOREACH_VISIBLE_COLUMN().
 * Return 0 to break the loop.
 */
typedef int (*VisibleForeachFn)(AG_Treetbl *tt, int x1, int x2, Uint32 visidx,
    void *, void *);

#define VISROW(tt,i) ((tt)->visible.items[i].row)
#define VISDEPTH(tt,i) ((tt)->visible.items[i].depth)

AG_Treetbl *
AG_TreetblNew(void *parent, Uint flags, AG_TreetblDataFn cellDataFn,
    AG_TreetblSortFn sortFn)
{
	AG_Treetbl *tt;

	tt = Malloc(sizeof(AG_Treetbl));
	AG_ObjectInit(tt, &agTreetblClass);

	tt->flags |= flags;
	if (flags & AG_TREETBL_HFILL) { WIDGET(tt)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_TREETBL_VFILL) { WIDGET(tt)->flags |= AG_WIDGET_VFILL; }

	tt->cellDataFn = cellDataFn;
	tt->sortFn = sortFn;

	AG_ObjectAttach(parent, tt);
	return (tt);
}

/* Timer for detecting double clicks. */
static Uint32
DoubleClickTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();

	tt->dblClicked = 0;
	/* TODO: if the cursor remains in the cell, activate a click-to-edit */
	return (0);
}

static void
FocusLost(AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();

	AG_DelTimer(tt, &tt->toDblClick);
	tt->dblClicked = 0;
}

static void
FontChangedRow(AG_Treetbl *_Nonnull tt, AG_TreetblRow *_Nonnull row)
{
	AG_TreetblRow *chld;
	Uint i;

	for (i = 0; i < tt->n; i++) {
		AG_TreetblCell *cell = &row->cell[i];

		if (cell->textSu != -1) {
			AG_WidgetUnmapSurface(tt, cell->textSu);
			cell->textSu = -1;
		}
	}

	TAILQ_FOREACH(chld, &row->children, siblings)
		FontChangedRow(tt, chld);
}

static void
FontChanged(AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();
	const AG_Font *font = WFONT(tt);
	AG_TreetblRow *row;
	Uint i;

	for (i = 0; i < tt->n; i++) {
		AG_TreetblCol *col = &tt->column[i];

		if (col->labelSu != -1) {
			AG_WidgetUnmapSurface(tt, col->labelSu);
			col->labelSu = -1;
		}
	}
	tt->hCol = font->height;
	tt->hRow = font->height + 2;

	TAILQ_FOREACH(row, &tt->children, siblings)
		FontChangedRow(tt, row);
}

static void
ScrollbarChanged(AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_PTR(1);

	tt->visible.dirty = 1;
	AG_Redraw(tt);
}

/* Process mouse column resize */
static void
ResizeColumn(AG_Treetbl *_Nonnull tt, int cid, int left)
{
	AG_TreetblCol *col = &tt->column[cid];
	int x;

	AG_MouseGetState(WIDGET(tt)->drv->mouse, &x, NULL);
	x -= WIDGET(tt)->rView.x1;
	col->w = (x-left) > 16 ? (x-left) : 16;
	AG_Redraw(tt);
}

/*
 * Return the number of visible descendants a given row has. must pass 0 for
 * i to get a meaningful result.
 */
static int
CountVisibleChld(AG_TreetblRowQ *_Nonnull in, int i)
{
	AG_TreetblRow *row;
	int j = i;

	TAILQ_FOREACH(row, in, siblings) {
		j++;
		if (row->flags & AG_TREETBL_ROW_EXPANDED &&
		    !TAILQ_EMPTY(&row->children))
			j = CountVisibleChld(&row->children, j);
	}
	return (j);
}

/* Swap two columns. */
static void
SwapColumns(AG_Treetbl *_Nonnull tt, Uint a, Uint b)
{
	AG_TreetblCol colTmp;

	/* a little sanity checking never hurt */
	if (a >= tt->n || b >= tt->n)
		return;
	if (a == b)
		return;

	colTmp = tt->column[a];
	tt->column[a] = tt->column[b];
	tt->column[b] = colTmp;
	tt->visible.dirty = 1;
	AG_Redraw(tt);
}

/* Process mouse column move. */
static void
MoveColumn(AG_Treetbl *_Nonnull tt, Uint cid, int left)
{
	AG_TreetblCol *col = &tt->column[cid];
	const AG_TreetblCol *colLeft = &tt->column[cid-1];
	const AG_TreetblCol *colRight = &tt->column[cid+1];
	int x;

	col->flags |= AG_TREETBL_COL_MOVING;

	AG_MouseGetState(WIDGET(tt)->drv->mouse, &x, NULL);
	x -= WIDGET(tt)->rView.x1;

	if ((col->w < colLeft->w  && x < left - colLeft->w + col->w) ||
	    (col->w >= colLeft->w && x < left)) {
		/*
		 * Drag left
		 */
		if (cid > 0) {
			left -= colLeft->w;
			SwapColumns(tt, cid, cid-1);
			cid--;
		}
	} else if ((col->w <= colRight->w && x > left + colRight->w) ||
		   (col->w > colRight->w  && x > left + col->w)) {
		/*
		 * Drag right
		 */
		if (cid < tt->n-1) {
			left += colRight->w;
			SwapColumns(tt, cid, cid+1);
			cid++;
		}
	}
	AG_Redraw(tt);
}

/*
 * Loops through each column currently onscreen. calls foreachFn for each
 * one, passing along args returns the index of the first foreachFn() call to
 * return zero, or tt->n, whichever comes first.
 */
static void
FOREACH_VISIBLE_COLUMN(AG_Treetbl *_Nonnull tt, VisibleForeachFn foreachFn,
    void *_Nullable arg1, void *_Nullable arg2)
{
	int x = 0, first_col = -1, wCol;
	Uint i;

	first_col = -1;
	for (i = 0; i < tt->n; i++) {
		AG_TreetblCol *col = &tt->column[i];

		if (first_col == -1 && x + col->w < tt->xOffs) {
			x += col->w;		/* x = offset in table */
			continue;
		} else if (first_col == -1) {
			first_col = i;
			x -= tt->xOffs;		/* x = offset on screen */
		}
		if (x >= tt->r.w)
			break;

		wCol = (x + col->w > tt->r.w) ? (tt->r.w - x) : col->w;
		if (foreachFn(tt, x, x+wCol, i, arg1, arg2) == 0)
			return;

		x += col->w;
	}
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();
	AG_TreetblCol *col = NULL;
	const int coord_x = AG_INT(2);
	const int coord_y = AG_INT(3);
	int left;
	Uint i;

	/*
	 * Our goal here is to set/toggle a column's sorting mode if the
	 * click fell on the header and the user did not drag it
	 */
	/* XXX - fix horiz sbar */
	left = 0;
	for (i = 0; i < tt->n; i++) {
		col = &tt->column[i];
		if (col->flags & AG_TREETBL_COL_SELECTED) {
			break;
		}
		left += col->w;
	}
	if (col == NULL)
		return;

	if (i < tt->n) {
		/*
		 * if the column wasn't moved and the mouse is still within
		 * the column's header, do sort work
		 */
		if (tt->flags & AG_TREETBL_SORT &&
		    !(col->flags & AG_TREETBL_COL_MOVING) &&
		    coord_y < tt->hCol &&
		    coord_x >= left &&
		    coord_x < (left+col->w)) {
			if (col->flags & AG_TREETBL_COL_SORTING) {
				if (tt->sortMode == AG_TREETBL_SORT_DSC) {
					tt->sortMode = AG_TREETBL_SORT_ASC;
				} else {
					tt->sortMode = AG_TREETBL_SORT_DSC;
				}
			}
			AG_TreetblSetSortCol(tt, col);
		}
		col->flags &= ~(AG_TREETBL_COL_MOVING|AG_TREETBL_COL_SELECTED);
	}
	AG_Redraw(tt);
}

/* Process click on a column header. */
static int
ClickedColumnHeader(AG_Treetbl *_Nonnull tt, int x1, int x2, Uint32 idx,
    void *_Nullable arg1, void *_Nullable arg2)
{
	int x = *(int *)arg1;
	AG_TreetblCol *col = &tt->column[idx];
	AG_TreetblCol *colLeft = &tt->column[idx-1];

	if ((x < x1 || x >= x2) && idx != tt->n-1)
		return (1);

	/* click on the CENTER */
	if (x >= x1+3 && x <= x2-3) {
		if (tt->flags & AG_TREETBL_REORDERCOLS) {
			MoveColumn(tt, idx, x1);
		}
		if (tt->flags & AG_TREETBL_REORDERCOLS ||
		    tt->flags & AG_TREETBL_SORT) {
			col->flags |= AG_TREETBL_COL_SELECTED;
			AG_Redraw(tt);
		}
	}
	/* click on the LEFT resize line */
	else if (idx-1 < tt->n &&
	         colLeft->flags & AG_TREETBL_COL_RESIZABLE &&
		 idx > 0 &&
		 x < x1+3) {
		ResizeColumn(tt, idx-1, (x1 - colLeft->w));
	}
	/* click on the RIGHT resize line */
	else if (idx < tt->n &&
	         (col->flags & AG_TREETBL_COL_RESIZABLE) &&
		 (x > x2-3 || (idx == tt->n-1 && x < x2+3))) {
		ResizeColumn(tt, idx, x1);
	}
	return (0);
}

/* Clear the selection bit for all rows in and descended from the rowq */
static void
DeselectAll(AG_TreetblRowQ *_Nonnull children)
{
	AG_TreetblRow *row;

	TAILQ_FOREACH(row, children, siblings) {
		row->flags &= ~(AG_TREETBL_ROW_SELECTED);
		if (row->flags & AG_TREETBL_ROW_EXPANDED &&
		    !TAILQ_EMPTY(&row->children))
			DeselectAll(&row->children);
	}
}

/* Process click over a row. */
static int
ClickedRow(AG_Treetbl *_Nonnull tt, int x1, int x2, Uint32 idx,
    void *_Nullable arg1, void *_Nullable arg2)
{
	AG_KeyMod kmod = AG_GetModState(tt);
	const int x = *(int *)arg1;
	const int y = *(int *)arg2;
	AG_TreetblRow *row = NULL;
	const int ts = (tt->hRow >> 1) + 1;
	Uint i, j, row_idx;
	int depth=0, px;

	if (x < x1 || x >= x2)
		return (1);

	/* Find the visible row under the cursor. */
	for (i = 0, px = tt->hCol;
	     i < tt->visible.count;
	     i++, px += tt->hRow) {
		if (y >= px && y < px+tt->hRow) {
			if (VISROW(tt,i) == NULL) {
				i = tt->visible.count;
			}
			break;
		}
	}
	if (i != tt->visible.count) {
		row = VISROW(tt,i);
		depth = VISDEPTH(tt,i);
	}
	row_idx = i;

	/* Clicking on blank space clears the selection. */
	if (row == NULL) {
		DeselectAll(&tt->children);
		/* AG_PostEvent(tt, "treetbl-selectclear", ""); */
		return (0);
	}

	/* Check for a click on the +/- button, if applicable. */
	if ((tt->column[idx].flags & AG_TREETBL_COL_EXPANDER) &&
	    !TAILQ_EMPTY(&row->children) &&
	    x > (x1+4+(depth*(ts+4))) &&
	    x < (x1+4+(depth*(ts+4))+ts)) {
		if (row->flags & AG_TREETBL_ROW_EXPANDED) {
			row->flags &= ~(AG_TREETBL_ROW_EXPANDED);
			/*
			 * XXX DeselectAll could return count to save
			 * redundant call to CountVisibleChld
			 */
			DeselectAll(&row->children);
			tt->nExpandedRows -= CountVisibleChld(
			    &row->children, 0);
		} else {
			row->flags |= AG_TREETBL_ROW_EXPANDED;
			tt->nExpandedRows += CountVisibleChld(&row->children, 0);
		}
		tt->visible.dirty = 1;
		AG_Redraw(tt);
		return (0);
	}
	
	/* Handle command/control clicks and range selections. */
	if (((tt->flags & AG_TREETBL_MULTITOGGLE) || kmod & AG_KEYMOD_META ||
	      kmod & AG_KEYMOD_CTRL)) {
		if (row->flags & AG_TREETBL_ROW_SELECTED) {
			row->flags &= ~(AG_TREETBL_ROW_SELECTED);
			AG_PostEvent(tt, "treetbl-deselect", "%p", row);
			AG_Redraw(tt);
		} else {
			if (!(tt->flags & AG_TREETBL_MULTI) && 
			    !(tt->flags & AG_TREETBL_MULTITOGGLE)) {
				DeselectAll(&tt->children);
			}
			row->flags |= AG_TREETBL_ROW_SELECTED;
			AG_PostEvent(tt, "treetbl-select", "%p", row);
			AG_Redraw(tt);
		}
	} else if (kmod & AG_KEYMOD_SHIFT) {
		for (j = 0; j < tt->visible.count; j++) {
			if (VISROW(tt,j) != NULL &&
			    (VISROW(tt,j)->flags & AG_TREETBL_ROW_SELECTED))
				break;
		}
		if (j < tt->visible.count) {
			/* XXX the selection may start on an invisible row */
			if (j < row_idx) {
				for (i = j; i <= row_idx; i++) {
					if (VISROW(tt,i) != NULL)
						VISROW(tt,i)->flags |=
						    AG_TREETBL_ROW_SELECTED;
				}
			} else if (j > row_idx) {
				for (i = row_idx; i <= j; i++) {
					if (VISROW(tt,i) != NULL)
						VISROW(tt,i)->flags |=
						    AG_TREETBL_ROW_SELECTED;
				}
			} else {
				if (VISROW(tt,row_idx) != NULL)
					VISROW(tt,row_idx)->flags |=
					    AG_TREETBL_ROW_SELECTED;
			}
			AG_Redraw(tt);
		}
	} else {
		DeselectAll(&tt->children);
		if (!(row->flags & AG_TREETBL_ROW_SELECTED)) {
			row->flags |= AG_TREETBL_ROW_SELECTED;
			AG_PostEvent(tt, "treetbl-select", "%p", row);
			AG_Redraw(tt);
		}
	}

	/* Handle double-clicks. */
	if (tt->dblClicked) {
		AG_DelTimer(tt, &tt->toDblClick);
		if(!TAILQ_EMPTY(&row->children)) {
			if (row->flags & AG_TREETBL_ROW_EXPANDED) {
				AG_TreetblCollapseRow(tt, row);
			} else {
				AG_TreetblExpandRow(tt, row);
			}
		}
		else {
			DeselectAll(&tt->children);
			row->flags |= AG_TREETBL_ROW_SELECTED;
			AG_Redraw(tt);
		}
		tt->dblClicked = 0;
		AG_PostEvent(tt, "treetbl-dblclick", "%p", row);
	} else {
		tt->dblClicked++;
		AG_AddTimer(tt, &tt->toDblClick, agMouseDblclickDelay,
		    DoubleClickTimeout, NULL);
	}
	return (0);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();
	const int button  = AG_INT(1);
	int coord_x = AG_INT(2);
	int coord_y = AG_INT(3);

	if (!AG_WidgetIsFocused(tt)) {
		AG_WidgetFocus(tt);
	}
	switch (button) {
	case AG_MOUSE_WHEELUP:
	case AG_MOUSE_WHEELDOWN:
		/* TODO */
		return;
	}

	if (tt->hCol > 0 && coord_y < tt->hCol) {
		/* a mouse down on the column header */
		FOREACH_VISIBLE_COLUMN(tt, ClickedColumnHeader, &coord_x, NULL);
	} else {
		/* a mouse down in the body */
		FOREACH_VISIBLE_COLUMN(tt, ClickedRow, &coord_x, &coord_y);
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Treetbl *tt = obj;
	
	WIDGET(tt)->flags |= AG_WIDGET_FOCUSABLE | AG_WIDGET_USE_TEXT;

	tt->cellDataFn = NULL;
	tt->sortFn = NULL;
	tt->flags = 0;
	tt->r.x = 0;
	tt->r.y = 0;
	tt->r.w = 0;
	tt->r.h = 0;
	tt->xOffs = 0;
	tt->xMax = 0;
	tt->yOffs = 0;
	tt->yMax = 0;

	tt->hCol = agTextFontHeight;
	tt->hRow = agTextFontHeight + 2;
	tt->dblClicked = 0;

	tt->vBar = AG_ScrollbarNew(tt, AG_SCROLLBAR_VERT, AG_SCROLLBAR_EXCL);
	AG_SetInt(tt->vBar,  "min",   0);
	AG_BindInt(tt->vBar, "max",   &tt->yMax);
	AG_BindInt(tt->vBar, "value", &tt->yOffs);
	AG_SetEvent(tt->vBar, "scrollbar-changed", ScrollbarChanged, "%p", tt);

	tt->hBar = AG_ScrollbarNew(tt, AG_SCROLLBAR_HORIZ, AG_SCROLLBAR_EXCL);
	AG_SetInt(tt->hBar,  "min",   0);
	AG_BindInt(tt->hBar, "max",   &tt->xMax);
	AG_BindInt(tt->hBar, "value", &tt->xOffs);

	tt->n = 0;
	tt->sortMode = AG_TREETBL_SORT_NOT;
	tt->column = NULL;
	TAILQ_INIT(&tt->children);
	TAILQ_INIT(&tt->backstore);
	tt->nExpandedRows = 0;
	tt->lineScrollAmount = 5;
	tt->visible.redraw_rate = 0;
	tt->visible.redraw_last = AG_GetTicks();
	tt->visible.count = 0;
	tt->visible.items = NULL;
	tt->wHint = 10;
	tt->hHint = tt->hCol + (tt->hRow << 2);
	
	AG_InitTimer(&tt->toDblClick, "dblClick", 0);

	AG_SetEvent(tt, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(tt, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tt, "widget-lostfocus", FocusLost, NULL);
	AG_AddEvent(tt, "widget-hidden", FocusLost, NULL);
	AG_AddEvent(tt, "font-changed", FontChanged, NULL);
}

void
AG_TreetblSizeHint(AG_Treetbl *tt, int w, int nrows)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	tt->wHint = w;
	tt->hHint = tt->hCol + tt->hRow*nrows;

	AG_ObjectUnlock(tt);
}

void
AG_TreetblSetColHeight(AG_Treetbl *tt, int h)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	tt->hCol = h;

	AG_WidgetUpdate(tt);
	AG_Redraw(tt);
}

/* Insert a column in the table. */
AG_TreetblCol *
AG_TreetblAddCol(AG_Treetbl *tt, int colID, const char *width, const char *text,
    ...)
{
	AG_TreetblCol *col, *colsNew;
	va_list args;
	Uint i;

	if (colID == -1)
		return (NULL);

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	/* Check for existing column ID */
	if ((tt->flags & AG_TREETBL_NODUPCHECKS) == 0) {
		for (i = 0; i < tt->n; i++) {
			if (tt->column[i].cid == colID)
				break;
		}
		if (i < tt->n) {
			AG_SetError("Existing column ID: %d", colID);
			goto fail;
		}
	}

	/* Allocate new column */
	if ((colsNew = TryRealloc(tt->column, (tt->n+1)*sizeof(AG_TreetblCol)))
	    == NULL) {
		goto fail;
	}
	tt->column = colsNew;
	col = &tt->column[tt->n];
	col->tbl = tt;
	col->flags = 0;
	col->cid = colID;
	col->idx = tt->n;
	col->labelSu = -1;
	tt->n++;

	/* Use first column for expander controls by default. */
	if (tt->n == 1)
		col->flags |= AG_TREETBL_COL_EXPANDER;

	/* Format the column header text. */
	if (text == NULL) {
		col->label = Strdup("");
	} else {
		va_start(args, text);
		Vasprintf(&col->label, text, args);
		va_end(args);
	}

	/* Set the default column width. */
	if (width != NULL) {
		if (AG_WidgetParseSizeSpec(width, &col->w) == AG_WIDGET_PERCENT)
			col->w = col->w*WIDTH(tt)/100;
	} else {
		col->w = 6;
		col->flags |= AG_TREETBL_COL_FILL;
	}

	tt->visible.dirty = 1;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
	return (col);
fail:
	AG_ObjectUnlock(tt);
	return (NULL);
}

/* Select the column controlling the sorting. */
void
AG_TreetblSetSortCol(AG_Treetbl *tt, AG_TreetblCol *col)
{
	int i;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	for (i = 0; i < tt->n; i++) {
		tt->column[i].flags &= ~(AG_TREETBL_COL_SORTING);
	}
	col->flags |= AG_TREETBL_COL_SORTING;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

/* Set the sorting order */
void
AG_TreetblSetSortMode(AG_Treetbl *tt, enum ag_treetbl_sort_mode mode)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	tt->sortMode = mode;
	AG_Redraw(tt);
}

/* Select an alternate column for the expand/collapse controls. */
void
AG_TreetblSetExpanderCol(AG_Treetbl *tt, AG_TreetblCol *col)
{
	int i;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	for (i = 0; i < tt->n; i++) {
		tt->column[i].flags &= ~(AG_TREETBL_COL_EXPANDER);
	}
	col->flags |= AG_TREETBL_COL_EXPANDER;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

/* Select a column. */
void
AG_TreetblSelectCol(AG_Treetbl *tt, AG_TreetblCol *col)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	col->flags |= AG_TREETBL_COL_SELECTED;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

/* Deselect a column. */
void
AG_TreetblDeselectCol(AG_Treetbl *tt, AG_TreetblCol *col)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	col->flags &= ~(AG_TREETBL_COL_SELECTED);

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

/* Select a column by ID */
int
AG_TreetblSelectColID(AG_Treetbl *tt, int colID)
{
	int i;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

        for (i = 0; i < tt->n; i++) {
                if (tt->column[i].cid == colID)
			break;
        }
	if (i == tt->n) {
		goto no_match;
	}
	tt->column[i].flags |= AG_TREETBL_COL_SELECTED;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
	return (0);
no_match:
	AG_ObjectUnlock(tt);
	return (-1);
}

/* Deselect a column by ID */
int
AG_TreetblDeselectColID(AG_Treetbl *tt, int colID)
{
	int i;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

        for (i = 0; i < tt->n; i++) {
                if (tt->column[i].cid == colID)
			break;
        }
	if (i == tt->n) {
		goto no_match;
	}
	tt->column[i].flags &= ~(AG_TREETBL_COL_SELECTED);

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
	return (0);
no_match:
	AG_ObjectUnlock(tt);
	return (-1);
}

void
AG_TreetblSetRefreshRate(AG_Treetbl *tt, Uint ms)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	tt->visible.redraw_rate = ms;
}

/*
 * Return 1 if the row is visible return 0 if an ancestor row is collapsed,
 * hiding it.
 */
static int
RowIsVisible(AG_TreetblRow *_Nonnull in)
{
	AG_TreetblRow *row = in->parent;

	while (row != NULL) {
		if (!(row->flags & AG_TREETBL_ROW_EXPANDED)) {
			return (0);
		}
		row = row->parent;
	}
	return (1);
}

/* Insert a row in the table. */
AG_TreetblRow *
AG_TreetblAddRow(AG_Treetbl *tt, AG_TreetblRow *pRow, int rowID,
    const char *argSpec, ...)
{
	char argBuf[128], *pArg = argBuf;
	AG_TreetblRow *row;
	Uint i, arg, nArgs = 0;
	va_list ap;

	Strlcpy(argBuf, argSpec, sizeof(argBuf));
	while (AG_Strsep(&pArg, ", ") != NULL)
		nArgs++;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	/* Check if row ID is already use */
	if (!(tt->flags & AG_TREETBL_NODUPCHECKS) &&
	    AG_TreetblLookupRowRecurse(&tt->children, rowID)) {
		AG_SetError("Existing row ID: %d", rowID);
		goto fail;
	}
	
	if ((row = TryMalloc(sizeof(AG_TreetblRow))) == NULL) {
		goto fail;
	}
	row->tbl = tt;
	if ((row->cell = TryMalloc(sizeof(AG_TreetblCell)*tt->n)) == NULL) {
		Free(row);
		goto fail;
	}
	row->flags = 0;
	row->parent = pRow;
	TAILQ_INIT(&row->children);

	for (i = 0; i < tt->n; i++) {
		row->cell[i].text = NULL;
		row->cell[i].textSu = -1;
	}

	/* import static data */
	va_start(ap, argSpec);
	for (arg = 0; arg < nArgs; arg++) {
		int colID = va_arg(ap, int);
		void *data = va_arg(ap, void *);
		AG_TreetblCell *cell;

		for (i = 0; i < tt->n; i++) {
			if (colID == tt->column[i].cid)
				break;
		}
		if (i == tt->n) {
			continue;
		}
		cell = &row->cell[i];
		Free(cell->text);
		cell->text = (data != NULL) ?
		             Strdup((char *)data) :
		             Strdup("(null)");
		if (cell->textSu != -1) {
			AG_WidgetUnmapSurface(tt, cell->textSu);
			cell->textSu = -1;
		}
	}
	va_end(ap);

	row->rid = rowID;

	if (pRow != NULL) {
		TAILQ_INSERT_TAIL(&pRow->children, row, siblings);
	} else {
		TAILQ_INSERT_TAIL(&tt->children, row, siblings);
	}

	/* increment scroll only if visible: */
	if (RowIsVisible(row))
		tt->nExpandedRows++;

	tt->visible.dirty = 1;

	AG_ObjectUnlock(tt);
	AG_Redraw(tt);
	return (row);
fail:
	AG_ObjectUnlock(tt);
	return (NULL);
}

/*
 * Return a pointer if the row with the given identifer exists in or in a
 * descendant of the rowq. If it does not exist, return NULL.
 */
AG_TreetblRow *
AG_TreetblLookupRowRecurse(AG_TreetblRowQ *searchIn, int rid)
{
	AG_TreetblRow *row, *row2;

	AG_TAILQ_FOREACH(row, searchIn, siblings) {
		if (row->rid == rid)
			return (row);

		if (!AG_TAILQ_EMPTY(&row->children)) {
			row2 = AG_TreetblLookupRowRecurse(&row->children, rid);
			if (row2 != NULL)
				return (row2);
		}
	}
	return (NULL);
}

/*
 * Lookup a row by ID.
 * Return value is valid as long as Treetbl is locked.
 */
AG_TreetblRow *_Nullable
AG_TreetblLookupRow(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	row = AG_TreetblLookupRowRecurse(&tt->children, rowID);

	AG_ObjectUnlock(tt);
	return (row);
}

/* Delete a row by ID */
int
AG_TreetblDelRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	if ((row = AG_TreetblLookupRow(tt,rowID)) == NULL) {
		return (-1);
	}
	AG_TreetblDelRow(tt, row);
	return (0);
}

/* Select a row by ID */
int
AG_TreetblSelectRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	if ((row = AG_TreetblLookupRow(tt, rowID)) == NULL) {
		return (-1);
	}
	AG_TreetblSelectRow(tt, row);
	return (0);
}

/* Deselect a row by ID */
int
AG_TreetblDeselectRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	if ((row = AG_TreetblLookupRow(tt, rowID)) == NULL) {
		return (-1);
	}
	AG_TreetblDeselectRow(tt, row);
	return (0);
}

/* Expand a row by ID */
int
AG_TreetblExpandRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	if ((row = AG_TreetblLookupRow(tt, rowID)) == NULL) {
		return (-1);
	}
	AG_TreetblExpandRow(tt, row);
	return (0);
}

/* Collapse a row by ID */
int
AG_TreetblCollapseRowID(AG_Treetbl *tt, int rowID)
{
	AG_TreetblRow *row;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");

	if ((row = AG_TreetblLookupRow(tt, rowID)) == NULL) {
		return (-1);
	}
	AG_TreetblCollapseRow(tt, row);
	return (0);
}

static void
DestroyRow(AG_Treetbl *_Nonnull tt, AG_TreetblRow *_Nonnull row)
{
	int i;

	for (i = 0; i < tt->n; i++) {
		AG_TreetblCell *cell = &row->cell[i];

		if (cell->textSu != -1) {
			AG_WidgetUnmapSurface(tt, cell->textSu);
		}
		Free(cell->text);
	}
	Free(row->cell);
	free(row);
}

void
AG_TreetblDelRow(AG_Treetbl *tt, AG_TreetblRow *row)
{
	AG_TreetblRow *row1, *row2;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	/* first remove children */
	row1 = TAILQ_FIRST(&row->children);
	while (row1 != NULL) {
		row2 = TAILQ_NEXT(row1, siblings);
		AG_TreetblDelRow(tt, row1);
		row1 = row2;
	}

	/* now that children are gone, remove this row */
	if (RowIsVisible(row))
		tt->nExpandedRows--;

	if (row->parent) {
		TAILQ_REMOVE(&row->parent->children, row, siblings);
	} else {
		TAILQ_REMOVE(&tt->children, row, siblings);
	}
	if (tt->flags & AG_TREETBL_POLLED) {
		TAILQ_INSERT_TAIL(&tt->backstore, row, backstore);
		goto out;
	}
	DestroyRow(tt, row);
out:
	tt->visible.dirty = 1;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

/*
 * Clear the rows. If AG_TREETBL_POLLED is in effect, the selection state
 * is remembered.
 */
void
AG_TreetblClearRows(AG_Treetbl *tt)
{
	AG_TreetblRow *row1, *row2;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	row1 = TAILQ_FIRST(&tt->children);
	while (row1 != NULL) {
		row2 = TAILQ_NEXT(row1, siblings);
		AG_TreetblDelRow(tt, row1);
		row1 = row2;
	}
	TAILQ_INIT(&tt->children);
	
	tt->nExpandedRows = 0;
	tt->visible.dirty = 1;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

void
AG_TreetblRestoreRows(AG_Treetbl *tt)
{
	AG_TreetblRow *row, *nrow, *srow;
	int i;

	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);
		
	for (row = TAILQ_FIRST(&tt->backstore);
	     row != TAILQ_END(&tt->backstore);
	     row = nrow) {
		nrow = TAILQ_NEXT(row, backstore);
		TAILQ_FOREACH(srow, &tt->children, siblings) {
			for (i = 0; i < tt->n; i++) {
				if (strcmp(row->cell[i].text,
				    srow->cell[i].text) != 0)
					break;
			}
			if (i != tt->n) {
				continue;
			}
			if (row->flags & AG_TREETBL_ROW_SELECTED) {
				srow->flags |= AG_TREETBL_ROW_SELECTED;
			} else {
				srow->flags &= ~(AG_TREETBL_ROW_SELECTED);
			}
		}
		DestroyRow(tt, row);
	}
	TAILQ_INIT(&tt->backstore);

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

void
AG_TreetblBegin(AG_Treetbl *tt)
{
	AG_TreetblClearRows(tt);
}

void
AG_TreetblEnd(AG_Treetbl *tt)
{
	AG_TreetblRestoreRows(tt);
}

/* Select the given row. */
void
AG_TreetblSelectRow(AG_Treetbl *tt, AG_TreetblRow *row)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	if (!(tt->flags & AG_TREETBL_MULTI) &&
	    !(tt->flags & AG_TREETBL_MULTITOGGLE)) {
		AG_TreetblDeselectRow(tt, NULL);
	}
	row->flags |= AG_TREETBL_ROW_SELECTED;

	AG_Redraw(tt);
	AG_ObjectUnlock(tt);
}

static void
SelectAll(AG_TreetblRowQ *_Nonnull children)
{
	AG_TreetblRow *row;

	TAILQ_FOREACH(row, children, siblings) {
		row->flags |= AG_TREETBL_ROW_SELECTED;

		if (!TAILQ_EMPTY(&row->children))
			SelectAll(&row->children);
	}
}

/* Set the selection bit for all rows recursively. */
void
AG_TreetblSelectAll(AG_Treetbl *tt, AG_TreetblRow *root)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	if (!(tt->flags & AG_TREETBL_MULTI) &&
	    !(tt->flags & AG_TREETBL_MULTITOGGLE))
		goto out;

	if (root == NULL) {
		SelectAll(&tt->children);
	} else {
		SelectAll(&root->children);
	}
out:
	AG_ObjectUnlock(tt);
}

/* Deselect the given row. */
void
AG_TreetblDeselectRow(AG_Treetbl *tt, AG_TreetblRow *row)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	if (row == NULL) {
		DeselectAll(&tt->children);
	} else {
		DeselectAll(&row->children);
	}

	AG_ObjectUnlock(tt);
}

/* Expand a specified row for display. */
void
AG_TreetblExpandRow(AG_Treetbl *tt, AG_TreetblRow *in)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	if (!(in->flags & AG_TREETBL_ROW_EXPANDED)) {
		in->flags |= AG_TREETBL_ROW_EXPANDED;
		if (RowIsVisible(in)) {
			tt->nExpandedRows += CountVisibleChld(&in->children, 0);
			tt->visible.dirty = 1;
			AG_Redraw(tt);
		}
	}

	AG_ObjectUnlock(tt);
}

/* Collapse a specified row from display. */
void
AG_TreetblCollapseRow(AG_Treetbl *tt, AG_TreetblRow *in)
{
	AG_OBJECT_ISA(tt, "AG_Widget:AG_Treetbl:*");
	AG_ObjectLock(tt);

	if (in->flags & AG_TREETBL_ROW_EXPANDED) {
		in->flags &= ~(AG_TREETBL_ROW_EXPANDED);
		if (RowIsVisible(in)) {
			tt->nExpandedRows -= CountVisibleChld(&in->children, 0);
			tt->visible.dirty = 1;
			AG_Redraw(tt);
		}
	}

	AG_ObjectUnlock(tt);
}

static void
Destroy(void *_Nonnull p)
{
	AG_Treetbl *tt = p;

	AG_TreetblClearRows(tt);
	Free(tt->column);
	Free(tt->visible.items);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Treetbl *tt = obj;
	AG_SizeReq rBar;
	int i;

	AG_WidgetSizeReq(tt->vBar, &rBar);
	r->w = tt->wHint + rBar.w;
	r->h = tt->hHint;

	if (tt->hBar != NULL) {
		AG_WidgetSizeReq(tt->hBar, &rBar);
		r->h += rBar.h;
	}
	
	for (i = 0; i < tt->n; i++)
		r->w += tt->column[i].w;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Treetbl *tt = obj;
	AG_Scrollbar *hBar, *vBar;
	Uint rows_per_view, i;
	AG_SizeAlloc aBar;
	AG_SizeReq rBar;

	if (a->h < tt->hCol || a->w < 8)
		return (-1);

	tt->r.x = 0;
	tt->r.y = tt->hCol;
	tt->r.w = WIDTH(tt);
	tt->r.h = HEIGHT(tt) - tt->hCol;

	/* Size vertical scroll bar. */
	vBar = tt->vBar;
	AG_WidgetSizeReq(vBar, &rBar);
	aBar.x = a->w - rBar.w;
	aBar.y = 0;
	aBar.w = rBar.w;
	aBar.h = a->h - aBar.w;
	AG_WidgetSizeAlloc(vBar, &aBar);
	tt->r.w -= WIDTH(vBar);

	/* Size horizontal scroll bar, if enabled. */
	if ((hBar = tt->hBar) != NULL) {
		int col_w = 0;

		AG_WidgetSizeReq(hBar, &rBar);
		aBar.x = 0;
		aBar.y = a->h - rBar.h;
		aBar.w = a->w - WIDTH(vBar);
		aBar.h = rBar.h;
		AG_WidgetSizeAlloc(hBar, &aBar);
		tt->r.h -= HEIGHT(hBar);

		for (i = 0; i < tt->n; i++) {
			col_w += tt->column[i].w;
		}
		if (col_w > WIDTH(hBar)) {
			tt->xMax = col_w - WIDTH(hBar);
			if (tt->xOffs > tt->xMax)
				tt->xOffs = tt->xMax;
		} else {
			tt->xOffs = 0;
		}
	}

	/* Calculate widths for TREETBL_COL_FILL columns. */
	{
		Uint fill_cols, nonfill_width, fill_width;

		fill_cols = 0;
		nonfill_width = 0;
		for (i = 0; i < tt->n; i++) {
			AG_TreetblCol *col = &tt->column[i];
			if (col->flags & AG_TREETBL_COL_FILL) {
				fill_cols++;
			} else {
				nonfill_width += col->w;
			}
		}
		if (fill_cols == 0)
			fill_cols = 1;

		fill_width = (a->w - WIDTH(vBar) - nonfill_width) / fill_cols;

		for (i = 0; i < tt->n; i++) {
			AG_TreetblCol *col = &tt->column[i];

			if (col->flags & AG_TREETBL_COL_FILL)
				col->w = fill_width;
		}
	}

	/* Calculate how many rows the view holds. */
	{
		if (tt->r.h < tt->hCol) {
			tt->r.h = tt->hCol;
		}
		rows_per_view = tt->r.h/ tt->hRow;
		if (tt->r.h % tt->hRow)
			rows_per_view++;
	}

	if (tt->visible.count < rows_per_view) {         /* visible increased */
		tt->visible.items = Realloc(tt->visible.items,
		    sizeof(struct ag_treetbl_rowdocket_item) * rows_per_view);

		for (i = tt->visible.count; i < rows_per_view; i++)
			VISROW(tt,i) = NULL;

		tt->visible.count = rows_per_view;
		tt->visible.dirty = 1;
	} else if (tt->visible.count > rows_per_view) {  /* visible decreased */
		tt->visible.items = Realloc(tt->visible.items,
		    sizeof(struct ag_treetbl_rowdocket_item) * rows_per_view);
		tt->visible.count = rows_per_view;
		tt->visible.dirty = 1;
	}
	return (0);
}

static void
DrawSubnodeIndicator(AG_Treetbl *_Nonnull tt, const AG_Rect *_Nonnull r,
    int isExpanded)
{
	static AG_VectorElement expdSign[] = {
		{ AG_VE_LINE,    3,5,  1,0, 0, NULL },            /* - */
		{ AG_VE_LINE,    1,7,  1,0, 0, NULL },            /* | */
	};
	const AG_Color *cLine = &WCOLOR(tt, LINE_COLOR);
	AG_Color c;

	AG_ColorRGBA_8(&c, 0,0,0, 64);
	AG_DrawRectBlended(tt, r, &c, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	if (isExpanded) {
		AG_DrawVector(tt, 3,3, r, cLine, expdSign, 0,1);    /* - */
	} else {
		AG_DrawVector(tt, 3,3, r, cLine, expdSign, 0,2);    /* + */
	}
}
static void
DrawDynamicColumn(AG_Treetbl *_Nonnull tt, Uint idx)
{
	AG_TreetblCol *col = &tt->column[idx];
	Uint i;

	for (i = 0; i < tt->visible.count; i++) {
		AG_TreetblRow *row = VISROW(tt,i);
		AG_TreetblCell *cell;
		char *sNew;

		if (row == NULL)
			break;
		if (!(row->flags & AG_TREETBL_ROW_DYNAMIC))
			continue;

		cell = &row->cell[col->idx];
		sNew = tt->cellDataFn(tt, col->cid, row->rid);
		if (sNew == NULL) {
			Free(cell->text);
			cell->text = NULL;
			if (cell->textSu != -1) {
				AG_WidgetUnmapSurface(tt, cell->textSu);
				cell->textSu = -1;
			}
			continue;			/* Nothing to draw */
		}
		if (cell->text != NULL && strcmp(cell->text, sNew) == 0) {
			free(sNew);
			continue;			/* No change in text */
		}
		Free(cell->text);
		cell->text = sNew;
		if (cell->textSu != -1) {
			AG_WidgetUnmapSurface(tt, cell->textSu);
			cell->textSu = -1;
		}
	}
}

/* Render a column header and cells. */
static int
DrawColumn(AG_Treetbl *_Nonnull tt, int x1, int x2, Uint32 idx,
    void *_Nullable arg1, void *_Nullable arg2)
{
	const int *update = (int *)arg1;
	AG_TreetblCol *col = &tt->column[idx];
	AG_Rect rd;
	const int spacing = 4;
	Uint j;
	int y;

	/* Render the column header. */
	if (tt->hCol > 0) {
		rd.x = x1;
		rd.y = 0;
		rd.w = col->w;
		rd.h = tt->hCol;
		if ((col->flags & (AG_TREETBL_COL_SELECTED |
		                   AG_TREETBL_COL_SORTING))) {
			AG_DrawBoxSunk(tt, &rd, &WCOLOR(tt, SELECTION_COLOR));
		} else {
			AG_DrawBoxRaised(tt, &rd, &WCOLOR(tt, FG_COLOR));
		}

		if (col->label[0] != '\0') {
			int xLbl;

			if (col->labelSu == -1) {
				col->labelSu = AG_WidgetMapSurface(tt,
				    AG_TextRender(col->label));
			}
			xLbl = (col->w >> 1) -
			       (WSURFACE(tt,col->labelSu)->w >> 1);

			AG_WidgetBlitSurface(tt, col->labelSu, x1+xLbl, 0);
		}
	}

	/* Check for the need to update */
	if (*update && (col->flags & AG_TREETBL_COL_DYNAMIC))
		DrawDynamicColumn(tt, idx);

	/* Draw the cells under this column */
	AG_PushClipRect(tt, &tt->r);
	y = tt->hCol;
	for (j = 0; j < tt->visible.count; j++) {
		int x = x1+spacing;
		AG_TreetblCell *cell;

		if (VISROW(tt,j) == NULL) {
			break;
		}
		if (col->flags & AG_TREETBL_COL_EXPANDER) {
			int tw = (tt->hRow >> 1);

			if ((tw & 1) == 0) { tw++; }
	
			x += VISDEPTH(tt,j) * (tw + spacing);
			if (!TAILQ_EMPTY(&VISROW(tt,j)->children)) {
				rd.x = x;
				rd.y = y + (tw >> 1);
				rd.w = tw;
				rd.h = tw;
				DrawSubnodeIndicator(tt, &rd,
				    (VISROW(tt,j)->flags & AG_TREETBL_ROW_EXPANDED));
			}
			x += tw + spacing;
		}
		cell = &VISROW(tt,j)->cell[col->idx];

		if (cell->text != NULL) {
			if (cell->textSu == -1) {
				cell->textSu = AG_WidgetMapSurface(tt,
				    AG_TextRender(cell->text));
			}
			AG_WidgetBlitSurface(tt, cell->textSu, x, y+1);
		}
		y += tt->hRow;
	}
	AG_PopClipRect(tt);

	/* Fill the remaining space in column heading */
	if (tt->hCol > 0 &&
	    idx == tt->n-1 &&
	    x2 < tt->r.w) {
		rd.x = x2;
		rd.y = 0;
		rd.w = tt->r.w - x2;
		rd.h = tt->hCol;
		AG_DrawBoxRaised(tt, &rd, &WCOLOR(tt, FG_COLOR));
	}
	return (1);
}

/*
 * Recursive companion to ViewChanged. Finds the visible rows and populates
 * tt->visible.items array with them.
 */
static int
ViewChangedRecurse(AG_Treetbl *_Nonnull tt, AG_TreetblRowQ *_Nonnull in,
    int depth, int filled, int *_Nonnull seen)
{
	AG_TreetblRow *row;
	Uint x = 0;

	TAILQ_FOREACH(row, in, siblings) {
		if (*seen) {
			(*seen)--;
		} else {
			if ((filled+x) < tt->visible.count) {
				VISROW(tt,filled+x) = row;
				VISDEPTH(tt,filled+x) = depth;
				x++;
			}
		}

		if (row->flags & AG_TREETBL_ROW_EXPANDED &&
		    filled+x < tt->visible.count)
			x += ViewChangedRecurse(tt, &row->children, depth + 1,
			    filled + x, seen);

		if (filled+x >= tt->visible.count)
			return (x);
	}
	return (x);
}

/*
 * Called after any addition or removal of visible rows. Rebuilds the array
 * of rows to be drawn (tt->visible).
 */
static void
ViewChanged(AG_Treetbl *_Nonnull tt)
{
	int rows_per_view, max, filled, value;
	Uint i;

	/* cancel double clicks if what's under it changes it */
	AG_DelTimer(tt, &tt->toDblClick);
	tt->dblClicked = 0;

	rows_per_view = tt->r.h/tt->hRow;
	if (tt->r.h % tt->hRow)
		rows_per_view++;

	max = tt->nExpandedRows - rows_per_view;
	if (max < 0) {
		max = 0;
	}
	if (max && (tt->r.h % tt->hRow) < 16) {
		max++;
	}
	tt->yMax = max;
	if (tt->yOffs > max)
		tt->yOffs = max;

	/* locate visible rows */
	value = tt->yOffs;
	filled = ViewChangedRecurse(tt, &tt->children, 0, 0, &value);

	/* blank empty rows */
	for (i = filled; i < tt->visible.count; i++)
		VISROW(tt,i) = NULL;

	/* render dynamic columns */
	for (i = 0; i < tt->n; i++)
		if (tt->column[i].flags & AG_TREETBL_COL_DYNAMIC)
			DrawDynamicColumn(tt, i);

	tt->visible.redraw_last = AG_GetTicks();
	tt->visible.dirty = 0;
}

static void
Draw(void *_Nonnull obj)
{
	AG_Treetbl *tt = obj;
	Uint i;
	int w, hRow, y, update = 0;

	/* Before we draw, update if needed */
	if (tt->visible.dirty) {
		ViewChanged(tt);
	}
	if (tt->visible.redraw_rate &&
	    AG_GetTicks() > tt->visible.redraw_last + tt->visible.redraw_rate)
		update = 1;
	
	AG_DrawBoxSunk(tt, &tt->r, &WCOLOR(tt, FG_COLOR));

	w = tt->r.w;
	hRow = tt->hRow;
	
	AG_WidgetDraw(tt->vBar);
	if (tt->hBar != NULL)
		AG_WidgetDraw(tt->hBar);
	
	/* draw row selection hilites */
	y = tt->hCol;
	for (i = 0; i < tt->visible.count; i++) {
		if (VISROW(tt,i) == NULL) {
			break;
		}
		if (VISROW(tt,i)->flags & AG_TREETBL_ROW_SELECTED) {
			AG_Rect rd;

			rd.x = 1;
			rd.y = y;
			rd.w = w-2;
			rd.h = hRow;
			AG_DrawBoxRaised(tt, &rd, &WCOLOR(tt, SELECTION_COLOR));
		}
		y += hRow;
	}

	/* draw columns */
	AG_PushBlendingMode(tt, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	FOREACH_VISIBLE_COLUMN(tt, DrawColumn, &update, NULL);
	AG_PopBlendingMode(tt);

	if (update)
		tt->visible.redraw_last = AG_GetTicks();
}

/*
 * Return a pointer to the currently selected row or NULL.
 * The Treetbl object must be locked.
 */
AG_TreetblRow *
AG_TreetblSelectedRow(AG_Treetbl *tt)
{
	AG_TreetblRow *row;

	TAILQ_FOREACH(row, &tt->children, siblings) {
		if (row->flags & AG_TREETBL_ROW_SELECTED)
			return (row);
	}
	return (NULL);
}

AG_WidgetClass agTreetblClass = {
	{
		"Agar(Widget:Treetbl)",
		sizeof(AG_Treetbl),
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
