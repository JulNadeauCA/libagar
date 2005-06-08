/*	$Csoft: timeouts.c,v 1.9 2005/05/12 02:39:21 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/tableview.h>

#include "monitor.h"

static AG_Tableview *tv = NULL;
static AG_Timeout refresher;

static Uint32 
timeouts_refresh(void *obj, Uint32 ival, void *arg)
{
	extern struct ag_objectq agTimeoutObjQ;
	AG_Object *ob;
	AG_Timeout *to;
	int id;
	
	AG_TableviewRowDelAll(tv);
	AG_LockTiming();

	id = 0;
	TAILQ_FOREACH(ob, &agTimeoutObjQ, tobjs) {
		char text[128];
		AG_TableviewRow *row1;
		
		row1 = AG_TableviewRowAdd(tv, 0, NULL, NULL, id++, 0, ob->name);
		AG_TableviewRowExpand(tv, row1);

		pthread_mutex_lock(&ob->lock);
		CIRCLEQ_FOREACH(to, &ob->timeouts, timeouts) {
			snprintf(text, sizeof(text), "%p: %u ticks", to,
			    to->ticks);
			AG_TableviewRowAdd(tv, 0, row1, NULL, id++, 0, text);
		}
		pthread_mutex_unlock(&ob->lock);
	}
	AG_UnlockTiming();
	return (ival);
}

static void
close_timeouts(int argc, union evarg *argv)
{
	AG_Window *win = argv[0].p;
	AG_Tableview *tv = argv[1].p;

	AG_DelTimeout(tv, &refresher);
	AG_ViewDetach(win);
}

AG_Window *
AG_DebugTimeoutList(void)
{
	AG_Window *win;

	if ((win = AG_WindowNew(0, "monitor-timeouts")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Running timers"));

	tv = AG_TableviewNew(win, AG_TABLEVIEW_NOHEADER, NULL, NULL);
	AG_TableviewPrescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZ", 6);
	AG_TableviewColAdd(tv, 0, 0, NULL, NULL);
	
	AG_SetTimeout(&refresher, timeouts_refresh, tv, 0);
	AG_AddTimeout(tv, &refresher, 50);
	
	AG_SetEvent(win, "window-close", close_timeouts, "%p", tv);
	return (win);
}

#endif	/* DEBUG */
