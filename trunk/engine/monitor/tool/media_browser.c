/*	$Csoft: media_browser.c,v 1.1 2002/09/04 03:21:01 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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
#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/tlist.h>

#include <engine/monitor/monitor.h>
#include <engine/mapedit/mapview.h>

#include "monitor_tool.h"
#include "media_browser.h"

static const struct monitor_tool_ops media_browser_ops = {
	{
		NULL,		/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	media_browser_window
};

static void	update_objlist(struct tlist *);

struct media_browser *
media_browser_new(struct monitor *mon, int flags)
{
	struct media_browser *media_browser;

	media_browser = emalloc(sizeof(struct media_browser));
	media_browser_init(media_browser, mon, flags);

	return (media_browser);
}

void
media_browser_init(struct media_browser *media_browser, struct monitor *mon,
    int flags)
{
	monitor_tool_init(&media_browser->tool, "media_browser", mon,
	    &media_browser_ops);

	media_browser->flags = flags;
}

static void
update_objlist(struct tlist *tl)
{
	dprintf("update objlist\n");
}

struct window *
media_browser_window(void *p)
{
	struct media_browser *obr = p;
	struct window *win;
	struct region *reg;
	struct textbox *obj_tbox, *offs_tbox;
	struct button *button;
	struct bitmap *bmp;
	struct tlist *objlist;

	win = window_new("monitor-media-browser", "Media browser",
	    WINDOW_SOLID|WINDOW_CENTER,
	    0, 0, 184, 228, 184, 228);

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	objlist = tlist_new(reg, 100, 100, 0);
	objlist->ops.update = update_objlist;

	return (win);
}

#endif	/* DEBUG */
