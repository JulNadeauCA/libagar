/*	$Csoft: monitor.c,v 1.12 2002/11/14 05:59:02 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/text.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"
#include "sprite_browser.h"
#include "object_browser.h"
#include "map_view.h"

const struct object_ops monitor_ops = {
	monitor_destroy,
	NULL,			/* load */
	NULL			/* save */
};

struct monitor monitor;		/* Debug monitor */

static void		 show_tool(int, union evarg *);
static struct window	*toolbar_window(struct monitor *);

static void
show_tool(int argc, union evarg *argv)
{
	struct widget *wid = argv[0].p;
	struct window *win = argv[1].p;

	OBJECT_ASSERT(wid, "widget");
	OBJECT_ASSERT(win, "window");

	window_show(win);
}

static struct window *
toolbar_window(struct monitor *mon)
{
	const int xdiv = 25, ydiv = 100;
	struct window *win;
	struct region *reg;
	struct button *button;

	win = window_new("monitor-toolbar", "Monitor", WINDOW_SOLID,
	    view->w - 86, 16,
	    130, 104,
	    130, 104);
	
	reg = region_new(win, REGION_HALIGN, 0,  0, 100, 50);
	reg->spacing = 1;
	
	/* Object browser */
	button = button_new(reg, NULL, SPRITE(mon, MONITOR_OBJECT_BROWSER), 0,
	    xdiv, ydiv);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed",
	    show_tool, "%p", mon->wins.object_browser);

	/* Sprite browser */
	button = button_new(reg, NULL, SPRITE(mon, MONITOR_SPRITE_BROWSER), 0,
	    xdiv, ydiv);
	event_new(button, "button-pushed",
	    show_tool, "%p", mon->wins.sprite_browser);
	
	reg = region_new(win, REGION_HALIGN, 0, 50, 100, 50);
	reg->spacing = 1;
	
	/* Map view */
	button = button_new(reg, NULL, SPRITE(mon, MONITOR_MAP_VIEW), 0,
	    xdiv, ydiv);
	event_new(button, "button-pushed", map_view_show, "%p", mon);

	return (win);
}

void
monitor_init(struct monitor *mon, char *name)
{
	object_init(&mon->obj, "debug-monitor", name, "monitor",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &monitor_ops);
	event_new(mon, "world-attached-object",
	    object_browser_attached_object, NULL);
	event_new(mon, "world-detached-object",
	    object_browser_detached_object, NULL);

	mon->wins.object_browser = object_browser_window(mon);
	mon->wins.sprite_browser = sprite_browser_window(mon);
	mon->wins.toolbar = toolbar_window(mon);
}

void
monitor_destroy(void *ob)
{
	struct monitor *mon = ob;

	object_destroy(mon->wins.toolbar);
	object_destroy(mon->wins.object_browser);
	object_destroy(mon->wins.sprite_browser);
}

void
monitor_tool_init(struct monitor_tool *tool, char *name, struct monitor *mon,
    const void *toolops)
{
	static pthread_mutex_t toolid_lock = PTHREAD_MUTEX_INITIALIZER;
	static int toolid = 0;
	char *toolname;

	pthread_mutex_lock(&toolid_lock);
	toolname = object_name(name, toolid++);
	pthread_mutex_unlock(&toolid_lock);
	object_init(&tool->obj, "monitor-tool", toolname, NULL, 0, toolops);
	free(toolname);

	tool->flags = 0;
	tool->mon = mon;
	tool->win = (MONITOR_TOOL_OPS(tool)->tool_window != NULL) ? 
	    MONITOR_TOOL_OPS(tool)->tool_window(tool) : NULL;
	tool->type = strdup(name);
}

struct mapview *
monitor_tool_mapview(void)
{
	struct window *win;
	struct region *reg;
	struct widget *wid;

	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		pthread_mutex_lock(&win->lock);
		if ((win->flags & WINDOW_SHOWN) == 0) {
			continue;
		}
		TAILQ_FOREACH(reg, &win->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
				if (!WIDGET_FOCUSED(wid)) {
					continue;
				}
				if (strcmp(wid->type, "mapview") == 0) {
					pthread_mutex_unlock(&win->lock);
					pthread_mutex_unlock(&view->lock);
					return ((struct mapview *)wid);
				}
			}
		}
		pthread_mutex_unlock(&win->lock);
	}
	pthread_mutex_unlock(&view->lock);
	return (NULL);
}

#endif	/* DEBUG */
