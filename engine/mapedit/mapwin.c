/*	$Csoft: mapwin.c,v 1.4 2002/07/07 10:17:21 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "command.h"
#include "mapwin.h"
#include "mapview.h"
#include "fileops.h"
#include "tilestack.h"

struct window *
mapwin_new(struct mapedit *med, struct map *m)
{
	struct window *win;
	struct region *reg;
	struct mapview *mv, *mv_reg;
	struct button *bu;
	struct tilestack *ts;
	char caption[2048];

	sprintf(caption, "%s (%dx%d)", OBJECT(m)->name, m->mapw, m->maph);

	win = emalloc(sizeof(struct window));
	window_init(win, caption, WINDOW_SOLID, 10, 16, 75, 70);

	/* Map view */
	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, m, MAPVIEW_CENTER, 100, 100);

	/*
	 * Tools
	 */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -25);
	reg->spacing = 1;
	/* Load map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_LOAD_MAP),
	    0, 0, 0);
	event_new(bu, "button-pushed", 0, fileops_revert_map, "%p", mv);
	/* Save map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_SAVE_MAP),
	    0, 0, 0);
	event_new(bu, "button-pushed", 0, fileops_save_map, "%p", mv);
	/* Clear map */
	bu = button_new(reg, NULL, SPRITE(med, MAPEDIT_TOOL_CLEAR_MAP),
	    0, 0, 0);
	event_new(bu, "button-pushed", 0, fileops_clear_map, "%p", mv);

	/* Tile stack */
	reg = region_new(win, REGION_VALIGN, 0, 10, -TILEW, 90);
	reg->spacing = 1;
	ts = tilestack_new(reg, TILESTACK_VERT, 100, 100, mv);

	/*
	 * Map view
	 */
	reg = region_new(win, REGION_HALIGN, 10, 10, 90, 90);
	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, mv);
	pthread_mutex_unlock(&reg->win->lock);

	win->focus = WIDGET(mv);

	return (win);
}

