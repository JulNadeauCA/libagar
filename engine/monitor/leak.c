/*	$Csoft: leak.c,v 1.4 2004/05/12 05:34:25 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/widget/tlist.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

extern struct error_mement error_mements[];
static const char *mement_names[] = {
	"generic",
	"object",
	"position",
	"dep",
	"prop",
	"event",
	"gfx",
	"audio",
	"map",
	"map_noderef",
	"mapedit",
	"nodexform",
	"nodemask",
	"widget",
	"vg",
	"view",
	"netbuf",
	"ttf",
	"xcf",
	"den",
	"text",
	"typesw",
	"input",
	"cad",
	"eda",
	"game",
	"math"
};

static void
poll_mements(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it;
	struct error_mement *ment;
	int i;

	tlist_clear_items(tl);
	for (ment = &error_mements[0], i = 0;
	     ment < &error_mements[M_LAST];
	     ment++, i++) {
		char text[TLIST_LABEL_MAX];

		snprintf(text, sizeof(text), "[%s] - %u buffers (%lu)",
		    mement_names[i], ment->nallocs - ment->nfrees,
		    (unsigned long)ment->msize);
		tlist_insert_item(tl, ICON(OBJ_ICON), text, ment);
	}
	tlist_restore_selections(tl);
}

struct window *
leak_window(void)
{
	struct window *win;
	struct tlist *tl;

	if ((win = window_new("monitor-leak")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Leak detection"));
	window_set_closure(win, WINDOW_DETACH);

	tl = tlist_new(win, TLIST_POLL|TLIST_MULTI|TLIST_STATIC_ICONS);
	event_new(tl, "tlist-poll", poll_mements, NULL);
	return (win);
}

#endif	/* DEBUG */
