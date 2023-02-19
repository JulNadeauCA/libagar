/*
 * Copyright (c) 2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Display the cache of loaded fonts.
 */

#include <agar/core/core.h>
#if defined(AG_WIDGETS) && defined(AG_TIMERS)

#include <agar/gui/window.h>
#include <agar/gui/table.h>

static void
PollFonts(AG_Event *_Nonnull event)
{
	static const char *fontTypeNames[] = {
		N_("Vector"),
		N_("Bitmap"),
		N_("Dummy")
	};
	AG_Table *tbl = AG_TABLE_SELF();
	AG_Font *font;
	const Uint32 t = AG_GetTicks();

	AG_TableBegin(tbl);
	AG_MutexLock(&agTextLock);
	TAILQ_FOREACH(font, &agFontCache, fonts) {
		char metrics[64];
		char style[64];

		Snprintf(metrics, sizeof(metrics),
		    "h(" AGSI_BOLD "%d" AGSI_RST ") "
		    AGSI_UP_ARROW "(" AGSI_BOLD "%d" AGSI_RST ") "
		    AGSI_DN_ARROW "(" AGSI_BOLD "%d" AGSI_RST ")",
		    font->height, font->ascent, font->descent);

		AG_FontGetStyleName(style, sizeof(style), font->flags);

		AG_TableAddRow(tbl, "%s:%s:%.1f:%s:%.1f:%s",
		    OBJECT(font)->name,
		    fontTypeNames[font->spec.type],
		    font->spec.size,
		    style,
		    (float)((t - font->tAccess) / 1000.0f),
		    metrics);
	}
	AG_MutexUnlock(&agTextLock);
	AG_TableEnd(tbl);
}

AG_Window *
AG_DEV_FontInfo(void)
{
	AG_Window *win;
	AG_Table *tbl;

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Font Cache"));
	AG_WindowSetPosition(win, AG_WINDOW_BL, 0);

	tbl = AG_TableNewPolled(win, AG_TABLE_EXPAND, PollFonts, NULL);
	AG_TableAddCol(tbl, _("Name"),    "<XXXXXXXXXXXXXXX>",   NULL);
	AG_TableAddCol(tbl, _("Class"),   "<XXXXX>",             NULL);
	AG_TableAddCol(tbl, _("Size"),    "<XXXXXXXXXX>",        NULL);
	AG_TableAddCol(tbl, _("Style"),   "<XXXXXX>",            NULL);
	AG_TableAddCol(tbl, _("Time"),    "<XXXXXXX>",           NULL);
	AG_TableAddCol(tbl, _("Metrics"), NULL,                  NULL);
	AG_TableSizeHint(tbl, 700, 30);

	AG_WindowShow(win);
	return (win);
}

#endif /* AG_WIDGETS and AG_TIMERS */
