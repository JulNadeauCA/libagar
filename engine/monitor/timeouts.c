/*	$Csoft: timeouts.c,v 1.1 2004/05/12 05:34:13 vedge Exp $	*/

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

static void
poll_timeouts(int argc, union evarg *argv)
{
	extern struct objectq timeout_objq;
	extern pthread_mutex_t timeout_lock;
	struct tlist *tl = argv[0].p;
	struct tlist_item *it;
	struct object *ob;
	struct timeout *to;
	int i;

	tlist_clear_items(tl);
	pthread_mutex_lock(&timeout_lock);
	TAILQ_FOREACH(ob, &timeout_objq, tobjs) {
		it = tlist_insert_item(tl, ICON(OBJ_ICON), ob->name, ob);
		it->depth = 0;
		CIRCLEQ_FOREACH(to, &ob->timeouts, timeouts) {
			char label[TLIST_LABEL_MAX];

			snprintf(label, sizeof(label), "%u ticks", to->ticks);
			it = tlist_insert_item(tl, ICON(EDA_START_SIM_ICON),
			    label, to);
			it ->depth = 1;
		}
	}
	pthread_mutex_unlock(&timeout_lock);
	tlist_restore_selections(tl);
}

struct window *
timeouts_window(void)
{
	struct window *win;
	struct tlist *tl;

	if ((win = window_new(WINDOW_DETACH, "monitor-timeouts")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Running timers"));

	tl = tlist_new(win, TLIST_POLL|TLIST_TREE);
	event_new(tl, "tlist-poll", poll_timeouts, NULL);
	return (win);
}

#endif	/* DEBUG */
