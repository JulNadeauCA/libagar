/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Simple memory leak detection tool.
 */

#include <config/debug.h>
#ifdef DEBUG

#include <core/core.h>
#include <core/view.h>

#include <gui/window.h>
#include <gui/tableview.h>

#include "dev.h"

extern struct ag_malloc_type agMallocTypes[];
static const char *mallocTypeNames[] = {
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
	"loader",
	"text",
	"typesw",
	"input",
	"cad",
	"eda",
	"game",
	"math",
	"sg"
};

static char *
UpdateTblCell(AG_Tableview *tv, AG_TableviewColID cid, AG_TableviewRowID rid)
{
	static char text[32];
	struct ag_malloc_type *ment = &agMallocTypes[rid];
  
  	switch (cid) {
	case 0:
		return ((char *)mallocTypeNames[rid]);
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

AG_Window *
DEV_LeakDetector(void)
{
	int i;
	int nents = sizeof(mallocTypeNames) / sizeof(mallocTypeNames[0]);
	AG_Window *win;
	AG_Tableview *tv;

	if ((win = AG_WindowNewNamed(0, "DEV_LeakDetector")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Leak Detector"));
	
	tv = AG_TableviewNew(win, AG_TABLEVIEW_EXPAND, UpdateTblCell, NULL);
	AG_TableviewPrescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ", 8);
	AG_TableviewSetUpdate(tv, 250);
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
	    0, _("Subsystem"), NULL);
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
	    1, _("Buffers"), "<XXXXXXXXXX>");
	AG_TableviewColAdd(tv, AG_TABLEVIEW_COL_DYNAMIC|AG_TABLEVIEW_COL_UPDATE,
	    2, _("Requests"), "<XXXXXXXXX>");
	
	for (i = 0; i < nents; i++) {
		AG_TableviewRowAdd(tv, 0, NULL, NULL, i);
	}
	return (win);
}

#endif	/* DEBUG */
