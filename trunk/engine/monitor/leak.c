/*	$Csoft: leak.c,v 1.11 2005/01/05 04:44:04 vedge Exp $	*/

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

#include <engine/widget/window.h>
#include <engine/widget/tableview.h>
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
	"rg",
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

static char *
leak_callback(colID cid, rowID rid)
{
	static char text[32];
	struct error_mement *ment = &error_mements[rid];
  
  	switch (cid) {
	case 0:
		return ((char *)mement_names[rid]);
	case 1:
		snprintf(text, sizeof(text), "%lu", (unsigned long)
		    (ment->nallocs-ment->nfrees));
		return (text);
	case 2:
		snprintf(text, sizeof(text), "%lu", (unsigned long)ment->msize);
		return (text);
	default:
		break;
	}
	return (NULL);
}

struct window *
leak_window(void)
{
	int i, nleak_ents = sizeof(mement_names) / sizeof(mement_names[0]);
	struct window *win;
	struct tableview *tv;

	if ((win = window_new(WINDOW_DETACH, "monitor-leak")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Leak detection"));
	
	tv = tableview_new(win, 0, leak_callback, NULL);
	tableview_prescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ", 8);
	tableview_set_update(tv, 250);
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_FILL|
			      TABLEVIEW_COL_UPDATE, 0,
			      _("Subsystem"), "map_noderef");
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_FILL|
			      TABLEVIEW_COL_UPDATE, 1,
			      _("Buffers"), "XXXXXXXXXX");
	tableview_col_add(tv, TABLEVIEW_COL_DYNAMIC|TABLEVIEW_COL_FILL|
			      TABLEVIEW_COL_UPDATE, 2,
			      _("Requests"), "XXXXXXXXX");
	
	for (i = 0; i < nleak_ents; i++)
		tableview_row_add(tv, 0, NULL, i);

	return (win);
}

#endif	/* DEBUG */
