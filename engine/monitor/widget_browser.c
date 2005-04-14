/*	$Csoft: widget_browser.c,v 1.41 2005/03/10 09:43:55 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/debug.h>

#ifdef DEBUG

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/palette.h>
#include <engine/widget/spinbutton.h>

#include "monitor.h"

static void
poll_windows_do(struct tlist *tl, struct window *win, int depth)
{
	char text[TLIST_LABEL_MAX];
	struct window *subwin;
	struct tlist_item *it;

	if (strcmp(win->caption, "win-popup") == 0)
		return;

	strlcpy(text, win->caption, sizeof(text));
	if (strcmp(OBJECT(win)->name, "win-generic") != 0) {
		strlcat(text, " (", sizeof(text));
		strlcat(text, OBJECT(win)->name, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}

	it = tlist_insert_item(tl, NULL, text, win);
	it->depth = depth;
	it->class = "window";

	TAILQ_FOREACH(subwin, &win->subwins, swins)
		poll_windows_do(tl, subwin, depth+1);
}

static void
poll_windows(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct window *win;

	tlist_clear_items(tl);
	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		poll_windows_do(tl, win, 0);
	}
	pthread_mutex_unlock(&view->lock);
	tlist_restore_selections(tl);
}

static void
show_window(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct window *win;

	if ((it = tlist_item_selected(tl)) == NULL)
		return;

	win = it->p1;
	window_show(win);
}

static void
hide_window(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	struct window *win;

	if ((it = tlist_item_selected(tl)) == NULL)
		return;

	win = it->p1;
	window_hide(win);
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
		return;
	}
	wid = it->p1;

	win = window_new(0, NULL);
	window_set_caption(win, _("Widget info: %s"), OBJECT(wid)->name);
	
	vb = vbox_new(win, VBOX_WFILL);
	{
		struct label *lab;

		label_new(vb, LABEL_STATIC, _("Name: \"%s\""),
		    OBJECT(wid)->name);
		label_new(vb, LABEL_STATIC, _("Type: %s"), wid->type);
		label_new(vb, LABEL_POLLED_MT, _("Flags: 0x%x"), &pwin->lock,
		    &wid->flags);

		lab = label_new(vb, LABEL_POLLED_MT,
		    _("Geometry: %dx%d at %d,%d (view %d,%d)"), &pwin->lock,
		    &wid->w, &wid->h, &wid->x, &wid->y,
		    &wid->cx, &wid->cy);
		label_prescale(lab,
		    _("Geometry: 0000x0000 at 0000x0000 (view 0000,0000)"));
	}
	window_show(win);
}

static void
poll_widgets_do(struct widget *wid, struct tlist *widtl, int depth)
{
	char text[TLIST_LABEL_MAX];
	struct tlist_item *it;

	strlcpy(text, OBJECT(wid)->name, sizeof(text));
	if (strcmp(wid->type, "window") == 0) {
		struct window *win = (struct window *)wid;

		strlcat(text, " (", sizeof(text));
		strlcat(text, win->caption, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}
	it = tlist_insert_item(widtl, NULL, text, wid);
	it->depth = depth;
	it->class = "widget";
	
	if (!TAILQ_EMPTY(&OBJECT(wid)->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
	}

	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_children(widtl, it)) {
		struct widget *cwid;

		OBJECT_FOREACH_CHILD(cwid, wid, widget)
			poll_widgets_do(cwid, widtl, depth+1);
	}
}

static void
poll_widgets(int argc, union evarg *argv)
{
	struct tlist *widtl = argv[0].p;
	struct window *win = argv[1].p;
	
	tlist_clear_items(widtl);
	lock_linkage();
	pthread_mutex_lock(&win->lock);

	poll_widgets_do(WIDGET(win), widtl, 0);

	pthread_mutex_unlock(&win->lock);
	unlock_linkage();
	tlist_restore_selections(widtl);
}

static void
scale_window(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	window_scale(win, -1, -1);
	window_scale(win, WIDGET(win)->w, WIDGET(win)->h);
	WINDOW_UPDATE(win);
}

static void
examine_window(int argc, union evarg *argv)
{
	struct tlist *wintl = argv[1].p;
	struct tlist_item *it;
	struct window *pwin, *win;
	struct tlist *tl;
	struct AGMenuItem *mi;
	struct spinbutton *sb;

	if ((it = tlist_item_selected(wintl)) == NULL) {
		text_msg(MSG_ERROR, _("No window is selected."));
		return;
	}
	pwin = it->p1;

	if ((win = window_new(WINDOW_DETACH, "monitor-win-%s",
	    OBJECT(pwin)->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s", OBJECT(pwin)->name);

	label_new(win, LABEL_STATIC, "Name: \"%s\"", OBJECT(pwin)->name);
	label_new(win, LABEL_POLLED_MT, "Flags: 0x%x", &pwin->lock,
	    &pwin->flags);

	sb = spinbutton_new(win, _("Widget spacing: "));
	widget_bind(sb, "value", WIDGET_INT, &pwin->spacing);
	spinbutton_set_min(sb, 0);
	event_new(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = spinbutton_new(win, _("Top padding: "));
	widget_bind(sb, "value", WIDGET_INT, &pwin->ypadding_top);
	spinbutton_set_min(sb, 0);
	event_new(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = spinbutton_new(win, _("Bottom padding: "));
	widget_bind(sb, "value", WIDGET_INT, &pwin->ypadding_bot);
	spinbutton_set_min(sb, 0);
	event_new(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = spinbutton_new(win, _("Horizontal padding: "));
	widget_bind(sb, "value", WIDGET_INT, &pwin->xpadding);
	spinbutton_set_min(sb, 0);
	event_new(sb, "spinbutton-changed", scale_window, "%p", pwin);

	tl = tlist_new(win, TLIST_TREE|TLIST_POLL);
	event_new(tl, "tlist-poll", poll_widgets, "%p", pwin);
	event_new(tl, "tlist-dblclick", examine_widget, "%p,%p", tl, pwin);

	mi = tlist_set_popup(tl, "widget");
	{
		menu_action(mi, _("Display informations..."), -1,
		    examine_widget, "%p,%p", tl, pwin);
	}
	window_show(win);
}

struct window *
widget_debug_window(void)
{
	struct window *win;
	struct tlist *tl;
	struct AGMenuItem *it;

	if ((win = window_new(WINDOW_DETACH, "monitor-window-stack"))
	    == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Window stack"));

	tl = tlist_new(win, TLIST_POLL);
	event_new(tl, "tlist-poll", poll_windows, NULL);
	event_new(tl, "tlist-dblclick", examine_window, "%p", tl);

	it = tlist_set_popup(tl, "window");
	{
		menu_action(it, _("Display informations..."), -1,
		    examine_window, "%p", tl);
		menu_separator(it);
		menu_action(it, _("Show this window"), -1,
		    show_window, "%p", tl);
		menu_action(it, _("Hide this window"), -1,
		    hide_window, "%p", tl);
	}
	window_set_geometry(win, view->w/4, view->h/3, view->w/2, view->h/4);
	return (win);
}

#endif	/* DEBUG */
