/*	$Csoft: monitor.c,v 1.8 2002/09/13 11:08:31 vedge Exp $	*/

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
	struct monitor *mon = argv[1].p;
	
	switch (argv[2].i) {
	case MONITOR_OBJECT_BROWSER:
		window_show(mon->wins.object_browser);
		break;
	case MONITOR_SPRITE_BROWSER:
		window_show(mon->wins.sprite_browser);
		break;
	}
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
	
	reg = region_new(win, REGION_VALIGN, 0,  0, 100, 50);
	reg->spacing = 1;
	
	/* Object browser */
	button = button_new(reg, NULL,
	    SPRITE(mon, MONITOR_OBJECT_BROWSER), 0, xdiv, ydiv);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed", 0,
	    show_tool, "%p, %i", mon, MONITOR_OBJECT_BROWSER);

	/* Sprite browser */
	button = button_new(reg, NULL,
	    SPRITE(mon, MONITOR_SPRITE_BROWSER), 0, xdiv, ydiv);
	win->focus = WIDGET(button);
	event_new(button, "button-pushed", 0,
	    show_tool, "%p, %i", mon, MONITOR_SPRITE_BROWSER);
	
	reg = region_new(win, REGION_VALIGN, -1, 50, 100, 50);
	reg->spacing = 1;
	
	return (win);
}

void
monitor_init(struct monitor *mon, char *name)
{
	object_init(&mon->obj, "debug-monitor", name, "monitor",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &monitor_ops);
	mon->wins.toolbar = toolbar_window(mon);
	mon->wins.object_browser = object_browser_window(mon);
	mon->wins.sprite_browser = sprite_browser_window(mon);
}

void
monitor_destroy(void *ob)
{
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

	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		if ((win->flags & WINDOW_SHOWN) == 0) {
			continue;
		}
		TAILQ_FOREACH(reg, &win->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
				if (!WIDGET_FOCUSED(wid)) {
					continue;
				}
				if (strcmp(wid->type, "mapview") == 0) {
					return ((struct mapview *)wid);
				}
			}
		}
	}
	return (NULL);
}


#endif
