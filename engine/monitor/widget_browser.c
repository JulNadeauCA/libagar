/*	$Csoft: widget_browser.c,v 1.1 2002/11/17 23:13:11 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <engine/mcconfig.h>

#ifdef DEBUG

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

/*
 * Window stack
 */

static void
tl_windows_poll(int argc, union evarg *argv)
{
	struct tlist *tl_windows = argv[0].p;
	struct window *win;

	tlist_clear_items(tl_windows);

	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		tlist_insert_item(tl_windows, NULL, OBJECT(win)->name, win);
	}
	pthread_mutex_unlock(&view->lock);

	tlist_restore_selections(tl_windows);
}

static void
tl_windows_show(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	window_show(win);
}

static void
tl_windows_hide(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	window_hide(win);
}

static void
tl_windows_detach(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct window *win = argv[1].p;

	view_detach(WIDGET(bu)->win);
	view_detach(win);
}

/*
 * Region list
 */

static void
tl_regions_poll(int argc, union evarg *argv)
{
	struct tlist *tl_regions = argv[0].p;
	struct window *pwin = argv[1].p;
	struct region *reg;

	tlist_clear_items(tl_regions);

	pthread_mutex_lock(&pwin->lock);
	TAILQ_FOREACH(reg, &pwin->regionsh, regions) {
		tlist_insert_item(tl_regions, NULL, OBJECT(reg)->name, reg);
	}
	pthread_mutex_unlock(&pwin->lock);

	tlist_restore_selections(tl_regions);
}

static void
tl_windows_selected(int argc, union evarg *argv)
{
	struct tlist *tl_windows = argv[0].p;
	struct tlist_item *it = argv[1].p;
	struct window *pwin = it->p1;
	struct window *win;
	struct region *reg;
	int i;

	win = window_generic_new(466, 261,
	    "monitor-widget-browser-%s-win", OBJECT(pwin)->name);
	if (win == NULL) {
		return;		/* Exists */
	}
	window_set_caption(win, "%s window", OBJECT(pwin)->name);
	
	pthread_mutex_lock(&pwin->lock);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 70);
	{
		label_new(reg, 100, 0, "Identifier: \"%s\"",
		    OBJECT(pwin)->name);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Flags: 0x%x", &pwin->flags);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Caption: \"%s\"", &pwin->caption);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Titlebar height: %d pixels", &pwin->titleh);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Geometry: %[wxh] (min %dx%d) at %[x,y]",
		    &pwin->rd,
		    &pwin->minw, &pwin->minh,
		    &pwin->rd);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Region spacing: %d pixels",
		    &pwin->spacing);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Body: %[rect]", &pwin->body);

		label_polled_new(reg, 100, 0, &pwin->lock,
		    "Focus: %p",
		    &pwin->focus);
	}
	
	reg = region_new(win, 0, 0, 70, 60, 30);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_regions_poll, "%p", pwin);
	}

	reg = region_new(win, REGION_VALIGN, 61, 70, 39, 30);
	{
		struct button *bu;

		bu = button_new(reg, "Show", NULL, 0, 100, 33);
		event_new(bu, "button-pushed", tl_windows_show, "%p", pwin);

		bu = button_new(reg, "Hide", NULL, 0, 100, 33);
		event_new(bu, "button-pushed", tl_windows_hide, "%p", pwin);

		bu = button_new(reg, "Detach", NULL, 0, 100, 33);
		event_new(bu, "button-pushed", tl_windows_detach, "%p", pwin);
	}

	pthread_mutex_unlock(&pwin->lock);
	window_show(win);
}

struct window *
widget_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct tlist *tl_windows;

	if ((win = window_generic_new(184, 100, "monitor-widget-browser"))
	    == NULL) {
		return (NULL);	/* Exists */
	}
	window_set_caption(win, "Window stack");

	reg = region_new(win, 0, 0, 0, 100, 100);
	tl_windows = tlist_new(reg, 100, 100, TLIST_POLL);
	event_new(tl_windows, "tlist-changed", tl_windows_selected, NULL);
	event_new(tl_windows, "tlist-poll", tl_windows_poll, NULL);

	return (win);
}

#endif	/* DEBUG */
