/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Timer inspector tool. This displays all running timers registered
 * through the Object system.
 */

#include <core/core.h>

#include <gui/window.h>
#include <gui/tableview.h>

#include "dev.h"

static AG_Tableview *tv = NULL;
static AG_Timeout refresher;

static Uint32 
UpdateTbl(void *obj, Uint32 ival, void *arg)
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

		AG_ObjectLock(ob);
		TAILQ_FOREACH(to, &ob->timeouts, timeouts) {
			Snprintf(text, sizeof(text), "%p: %u ticks", to,
			    (Uint)to->ticks);
			AG_TableviewRowAdd(tv, 0, row1, NULL, id++, 0, text);
		}
		AG_ObjectUnlock(ob);
	}
	AG_UnlockTiming();
	return (ival);
}

static void
CloseWindow(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	AG_Tableview *tv = AG_PTR(1);

	AG_DelTimeout(tv, &refresher);
	AG_ViewDetach(win);
}

AG_Window *
DEV_TimerInspector(void)
{
	AG_Window *win;

	if ((win = AG_WindowNewNamed(0, "DEV_TimerInspector")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Timer Inspector"));

	tv = AG_TableviewNew(win, AG_TABLEVIEW_NOHEADER|AG_TABLEVIEW_EXPAND,
	   NULL, NULL);
	AG_TableviewSizeHint(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZ", 6);
	AG_TableviewColAdd(tv, 0, 0, NULL, NULL);
	
	AG_SetTimeout(&refresher, UpdateTbl, tv, 0);
	AG_AddTimeout(tv, &refresher, 50);
	
	AG_SetEvent(win, "window-close", CloseWindow, "%p", tv);
	return (win);
}
