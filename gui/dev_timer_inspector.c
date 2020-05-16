/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#if defined(AG_WIDGETS) && defined(AG_TIMERS) && defined(AG_ENABLE_STRING)

#include <agar/gui/window.h>
#include <agar/gui/treetbl.h>
#include <agar/gui/button.h>

static Uint32 
RefreshTableTimeout(AG_Timer *_Nonnull refreshTo, AG_Event *_Nonnull event)
{
	AG_Treetbl *tt = AG_TREETBL_SELF();
	AG_Label *lbl = AG_LABEL_PTR(1);
	const int *pauseFlag = (const int *)AG_PTR(2);
	extern struct ag_objectq agTimerObjQ;
	AG_Object *ob;
	int id;

	if (*pauseFlag) {
		goto out;
	}
	AG_LabelText(lbl, _("Ticks: %u"), (Uint)AG_GetTicks());

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
		TAILQ_FOREACH(to, &ob->timers, pvt.timers) {
			AG_TreetblAddRow(tt, objRow, id++,
			    "%s,%s,%s,%s",
			    0, to->name,
			    1, AG_PrintfN(0,"%d",to->id),
			    2, AG_PrintfN(1,"%u",(Uint)to->ival),
			    3, AG_PrintfN(2,"%u",(Uint)to->tSched));
		}
		AG_ObjectUnlock(ob);
	}
out:
	return (refreshTo->ival);
}

static void
CloseWindow(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();

	AG_ObjectDetach(win);
}

AG_Window *
AG_DEV_TimerInspector(void)
{
	static int pauseFlag = 0;
	AG_Window *win;
	AG_Treetbl *tt;
	AG_Label *lbl;
	AG_Timer *to;

	if ((win = AG_WindowNewNamedS(0, "DEV_TimerInspector")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Timer Inspector"));

	lbl = AG_LabelNew(win, AG_LABEL_HFILL, _("Ticks: ..."));
	AG_ButtonNewInt(win, AG_BUTTON_EXCL | AG_BUTTON_STICKY, _("Pause"), &pauseFlag);

	tt = AG_TreetblNew(win, AG_TREETBL_EXPAND, NULL, NULL);
	AG_TreetblSizeHint(tt, 100, 30);
	AG_TreetblAddCol(tt, 0, "<XXXXXXXXXXXXXXX>", _("Name"));
	AG_TreetblAddCol(tt, 1, "<XXXXX>", _("ID"));
	AG_TreetblAddCol(tt, 2, "<XXXXXXXX>", _("Ticks"));
	AG_TreetblAddCol(tt, 3, "<XXXXXXXX>", "tSched");

	to = AG_AddTimerAuto(tt, 100, RefreshTableTimeout, "%p,%p", lbl, &pauseFlag);
	if (to != NULL)
		Strlcpy(to->name, "timerInspector", sizeof(to->name));
	
	AG_SetEvent(win, "window-close", CloseWindow, "%p", tt);
	return (win);
}

#endif /* AG_WIDGETS and AG_TIMERS and AG_ENABLE_STRING */
