/*	$Csoft: timeouts.c,v 1.2 2004/09/12 05:57:24 vedge Exp $	*/

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
#include <engine/widget/tableview.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static struct tableview *tv = NULL;
static struct timeout refresher;

static Uint32 
timeouts_refresh(void *obj, Uint32 ival, void *arg)
{
	extern struct objectq timeout_objq;
	extern pthread_mutex_t timeout_lock;
	struct tableview_row *row1;
    char text[128];
	
	struct object *ob;
	struct timeout *to;
	int id;
	
	tableview_row_del_all(tv);
	pthread_mutex_lock(&timeout_lock);

	id = 0;
	TAILQ_FOREACH(ob, &timeout_objq, tobjs) {
	    row1 = tableview_row_add(tv, 0, NULL, id++, 0, ob->name);
        tableview_row_expand(tv, row1);
	    CIRCLEQ_FOREACH(to, &ob->timeouts, timeouts) {
            snprintf(text, sizeof(text), "%u ticks", to->ticks);
	        tableview_row_add(tv, 0, row1, id++, 0, text);
	    }
	}
	
	pthread_mutex_unlock(&timeout_lock);

    return 50;
}


struct window *
timeouts_window(void)
{
	struct window *win;

	if ((win = window_new(WINDOW_DETACH, "monitor-timeouts")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Running timers"));

	tv = tableview_new(win, TABLEVIEW_NOHEADER, NULL, NULL);
	tableview_prescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZ", 6);
	tableview_col_add(tv, TABLEVIEW_COL_FILL, 0, NULL, NULL);
	
	timeout_set(&refresher, timeouts_refresh, tv, 0);
	timeout_add(tv, &refresher, 50);
	
    return (win);
}

#endif	/* DEBUG */
