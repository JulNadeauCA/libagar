/*	$Csoft: widget_browser.c,v 1.27 2003/07/28 15:29:59 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#ifdef DEBUG

#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/palette.h>

#include <engine/mapedit/mapview.h>

#include "monitor.h"

/* Update the window list display. */
static void
poll_windows(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct window *win;

	tlist_clear_items(tl);

	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		tlist_insert_item(tl, NULL, OBJECT(win)->name, win);
	}
	pthread_mutex_unlock(&view->lock);

	tlist_restore_selections(tl);
}

/* Set the visibility bit on a window. */
static void
show_window(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct window *win;

	it = tlist_item_selected(tl);
	if (it == NULL) {
		text_msg(MSG_ERROR, _("No window is selected."));
		return;
	}

	win = it->p1;
	window_show(win);
}

/* Clear the visibility bit on a window. */
static void
hide_window(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct window *win;

	it = tlist_item_selected(tl);
	if (it == NULL) {
		text_msg(MSG_ERROR, _("No window is selected."));
		return;
	}

	win = it->p1;
	window_hide(win);
}

/* Detach a window (assuming it is not referenced). */
static void
detach_window(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct window *win;

	it = tlist_item_selected(tl);
	if (it == NULL) {
		text_msg(MSG_ERROR, _("No window is selected."));
		return;
	}

	win = it->p1;
	view_detach(win);
}

/* Update the list of colors for a widget. */
static void
poll_colors(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct widget *wid = argv[1].p;
	int i;

	tlist_clear_items(tl);
	for (i = 0; i < wid->ncolors; i++) {
		tlist_insert_item(tl, NULL, wid->color_names[i],
		    &wid->colors[i]);
	}
	tlist_restore_selections(tl);
}

/* Select a color to edit in a widget's color scheme. */
static void
select_color(int argc, union evarg *argv)
{
	struct palette *pal = argv[2].p;
	struct tlist_item *it = argv[3].p;
	Uint32 *col = it->p1;

	widget_bind(pal, "color", WIDGET_UINT32, NULL, col);
}

/* Display widget information. */
static void
examine_widget(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist_item *it;
	struct widget *wid;
	struct window *win;
	struct vbox *vb;

	if ((it = tlist_item_selected(tl)) == NULL) {
		text_msg(MSG_ERROR, _("No widget is selected."));
		return;
	}
	wid = it->p1;

	win = window_new(NULL);
	window_set_caption(win, _("%s widget"), OBJECT(wid)->name);
	
	vb = vbox_new(win, VBOX_WFILL);
	{
		struct label *lab;

		label_new(vb, _("Name: \"%s\""), OBJECT(wid)->name);
		label_new(vb, _("Type: %s"), wid->type);
		label_polled_new(vb, &pwin->lock, _("Flags: 0x%x"),
		    &wid->flags);

		lab = label_polled_new(vb, &pwin->lock,
		    _("Geo: %dx%d at %d,%d (view %d,%d)"),
		    &wid->w, &wid->h, &wid->x, &wid->y,
		    &wid->cx, &wid->cy);
		label_prescale(lab,
		    _("Geo: 0000x0000 at 0000x0000 (view 0000,0000)"));
	}

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		struct tlist *tl_colors;
		struct palette *pal;

		pal = palette_new(vb, PALETTE_RGB);
		tl_colors = tlist_new(vb, TLIST_POLL);
		event_new(tl_colors, "tlist-poll", poll_colors, "%p", wid);
		event_new(tl_colors, "tlist-changed", select_color, "%p, %p",
		    wid, pal);
	}
	window_show(win);
}

static void
find_widgets(struct widget *wid, struct tlist *widtl, int depth)
{
	struct tlist_item *it;

	it = tlist_insert_item(widtl, NULL, OBJECT(wid)->name, wid);
	it->depth = depth;
	
	if (!TAILQ_EMPTY(&OBJECT(wid)->childs)) {
		it->flags |= TLIST_HAS_CHILDREN;
	}

	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_childs(widtl, it)) {
		struct widget *cwid;

		OBJECT_FOREACH_CHILD(cwid, wid, widget)
			find_widgets(cwid, widtl, depth+1);
	}
}

/* Update the widget tree display. */
static void
poll_widgets(int argc, union evarg *argv)
{
	struct tlist *widtl = argv[0].p;
	struct window *win = argv[1].p;
	
	tlist_clear_items(widtl);
	lock_linkage();
	pthread_mutex_lock(&win->lock);

	find_widgets(WIDGET(win), widtl, 0);

	pthread_mutex_unlock(&win->lock);
	unlock_linkage();
	tlist_restore_selections(widtl);
}

/* Display window information. */
static void
examine_window(int argc, union evarg *argv)
{
	struct tlist *wintl = argv[1].p;
	struct tlist_item *it;
	struct window *pwin, *win;
	struct tlist *tl;
	struct hbox *hb;

	it = tlist_item_selected(wintl);
	if (it == NULL) {
		text_msg(MSG_ERROR, _("No window is selected."));
		return;
	}
	pwin = it->p1;

	if ((win = window_new("monitor-win-%s", OBJECT(pwin)->name)) == NULL) {
		return;
	}
	window_set_caption(win, _("%s window"), OBJECT(pwin)->name);
	window_set_closure(win, WINDOW_DETACH);

	label_new(win, _("Name: \"%s\""), OBJECT(pwin)->name);
	label_polled_new(win, &pwin->lock, _("Flags: 0x%x"), &pwin->flags);

	tl = tlist_new(win, TLIST_TREE|TLIST_POLL);
	event_new(tl, "tlist-poll", poll_widgets, "%p", pwin);

	hb = hbox_new(win, HBOX_WFILL|HBOX_HOMOGENOUS);
	{
		struct button *bu;

		bu = button_new(hb, _("Examine"));
		event_new(bu, "button-pushed", examine_widget, "%p, %p",
		    tl, pwin);
	}

	window_show(win);
}

struct window *
widget_debug_window(void)
{
	struct window *win;
	struct hbox *hb;
	struct tlist *tl;

	if ((win = window_new("monitor-window-stack")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Window stack"));
	window_set_closure(win, WINDOW_DETACH);

	tl = tlist_new(win, TLIST_POLL);
	event_new(tl, "tlist-poll", poll_windows, NULL);

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	{
		struct button *bu;
		
		bu = button_new(hb, _("Examine"));
		event_new(bu, "button-pushed", examine_window, "%p", tl);
		bu = button_new(hb, _("Show"));
		event_new(bu, "button-pushed", show_window, "%p", tl);
		bu = button_new(hb, _("Hide"));
		event_new(bu, "button-pushed", hide_window, "%p", tl);
		bu = button_new(hb, _("Detach"));
		event_new(bu, "button-pushed", detach_window, "%p", tl);
	}
	return (win);
}

#endif	/* DEBUG */
