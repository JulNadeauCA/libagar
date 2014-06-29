/*
 * Copyright (c) 2004-2013 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/window.h>
#include <agar/gui/treetbl.h>
#include <agar/dev/dev.h>

static Uint32 
RefreshTableTimeout(AG_Timer *refreshTo, AG_Event *event)
{
	AG_Treetbl *tt = AG_SELF();
	extern struct ag_objectq agTimerObjQ;
	AG_Object *ob;
	int id;
	
	AG_TreetblClearRows(tt);

	id = 0;
	TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
		AG_TreetblRow *objRow;
		AG_Timer *to;
		
		objRow = AG_TreetblAddRow(tt, NULL, id++,
		    "%s",
		    0, ob->name);
		AG_TreetblExpandRow(tt, objRow);

		AG_ObjectLock(ob);
		TAILQ_FOREACH(to, &ob->timers, timers) {
			AG_TreetblAddRow(tt, objRow, id++,
			    "%s,%s,%s,%s",
			    0, to->name,
			    1, AG_PrintfN(0,"%d",to->id),
			    2, AG_PrintfN(1,"%u",(Uint)to->ival),
			    3, AG_PrintfN(2,"%u",(Uint)to->tSched));
		}
		AG_ObjectUnlock(ob);
	}
	return (refreshTo->ival);
}

static void
CloseWindow(AG_Event *event)
{
	AG_Window *win = AG_SELF();

	AG_ObjectDetach(win);
}

AG_Window *
DEV_TimerInspector(void)
{
	AG_Window *win;
	AG_Treetbl *tt;
	AG_Timer *to;

	if ((win = AG_WindowNewNamedS(0, "DEV_TimerInspector")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Timer Inspector"));

	tt = AG_TreetblNew(win, AG_TREETBL_EXPAND, NULL, NULL);
	AG_TreetblSizeHint(tt, 200, 6);
	AG_TreetblAddCol(tt, 0, "<XXXXXXXXXXXXXXX>", _("Name"));
	AG_TreetblAddCol(tt, 1, "<XXXXX>", _("ID"));
	AG_TreetblAddCol(tt, 2, "<XXXXXXXX>", _("Ticks"));
	AG_TreetblAddCol(tt, 3, "<XXXXXXXX>", "tSched");

	to = AG_AddTimerAuto(tt, 10, RefreshTableTimeout, NULL);
	if (to != NULL)
		Strlcpy(to->name, "timerInspector", sizeof(to->name));
	
	AG_SetEvent(win, "window-close", CloseWindow, "%p", tt);
	return (win);
}
