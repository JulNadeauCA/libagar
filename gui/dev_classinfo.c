/*
 * Copyright (c) 2008-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Display registered AG_Object classes.
 */

#include <agar/core/core.h>
#if defined(AG_WIDGETS) && defined(AG_TIMERS)

#include <agar/gui/window.h>
#include <agar/gui/table.h>

static void
GenClassTable(AG_Table *_Nonnull tbl, AG_ObjectClass *_Nonnull C)
{
	AG_ObjectClass *Csub;
	
	AG_TableAddRow(tbl, "%s:%d:%s:%s",
	    C->name, C->size,
	    C->pvt.libs[0] != '\0' ? C->pvt.libs : "",
	    C->hier);

	TAILQ_FOREACH(Csub, &C->pvt.sub, pvt.subclasses)
		GenClassTable(tbl, Csub);
}

static void
PollClasses(AG_Event *_Nonnull event)
{
	AG_Table *tbl = AG_TABLE_SELF();

	/* XXX tree */
	AG_TableBegin(tbl);
	GenClassTable(tbl, &agObjectClass);
	AG_TableEnd(tbl);
}

AG_Window *
AG_DEV_ClassInfo(void)
{
	AG_Window *win;
	AG_Table *tbl;

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Registered classes"));
	AG_WindowSetPosition(win, AG_WINDOW_TL, 0);
	AG_SetStyle(win, "font-size", "90%");

	tbl = AG_TableNewPolled(win, AG_TABLE_EXPAND, PollClasses, NULL);
	AG_TableAddCol(tbl, _("Name"), "<XXXXXXXXXXXXXXX>", NULL);
	AG_TableAddCol(tbl, _("Size"), "<XXXX>", NULL);
	AG_TableAddCol(tbl, _("Modules"), "<XXXXXXX>", NULL);
	AG_TableAddCol(tbl, _("Hierarchy"), NULL, NULL);
	AG_TableSizeHint(tbl, 600, 20);

	AG_WindowShow(win);
	return (win);
}

#endif /* AG_WIDGETS and AG_TIMERS */
