/*
 * Copyright (c) 2004-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <gui/treetbl.h>

#include "dev.h"

static AG_Treetbl *tt = NULL;
static AG_Timeout refresher;

static Uint32 
UpdateTbl(void *obj, Uint32 ival, void *arg)
{
	extern struct ag_objectq agTimeoutObjQ;
	AG_Object *ob;
	AG_Timeout *to;
	int id;
	
	AG_TreetblClearRows(tt);
	AG_LockTiming();

	id = 0;
	TAILQ_FOREACH(ob, &agTimeoutObjQ, tobjs) {
		char text[128];
		AG_TreetblRow *objRow;
		
		objRow = AG_TreetblAddRow(tt, NULL, id++, "%s", ob->name);
		AG_TreetblExpandRow(tt, objRow);

		AG_ObjectLock(ob);
		TAILQ_FOREACH(to, &ob->timeouts, timeouts) {
			Snprintf(text, sizeof(text),
			    "%p: %u ticks", to, (Uint)to->ticks);
			AG_TreetblAddRow(tt, objRow, id++, "%s", text);
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
	AG_Treetbl *tt = AG_PTR(1);

	AG_DelTimeout(tt, &refresher);
	AG_ObjectDetach(win);
}

AG_Window *
DEV_TimerInspector(void)
{
	AG_Window *win;

	if ((win = AG_WindowNewNamedS(0, "DEV_TimerInspector")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Timer Inspector"));

	tt = AG_TreetblNew(win, AG_TREETBL_EXPAND, NULL, NULL);
	AG_TreetblSizeHint(tt, 200, 6);
	AG_TreetblAddCol(tt, 0, NULL, NULL);
	
	AG_SetTimeout(&refresher, UpdateTbl, tt, 0);
	AG_ScheduleTimeout(tt, &refresher, 50);
	
	AG_SetEvent(win, "window-close", CloseWindow, "%p", tt);
	return (win);
}
