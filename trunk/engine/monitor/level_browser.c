/*	$Csoft: level_browser.c,v 1.1 2002/11/16 00:57:40 vedge Exp $	*/

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
#include <engine/mapedit/mapview.h>

#include "monitor.h"

struct window *
level_browser_window(void)
{
	struct window *win;
	struct region *reg;
	struct mapview *mv;

	win = window_generic_new(320, 200,
	    "monitor-level-browser-%s", OBJECT(view->rootmap->map)->name);
	if (win == NULL) {
		return (NULL);		/* Exists */
	}
	window_set_caption(win, "%s view", OBJECT(view->rootmap->map)->name);

	reg = region_new(win, 0, 0, 0, 100, 100);
	mv = mapview_new(reg, NULL, view->rootmap->map,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_SHOW_CURSOR,
	    100, 100);

	return (win);
}

#endif	/* DEBUG */
