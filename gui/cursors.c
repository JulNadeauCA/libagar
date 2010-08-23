/*
 * Copyright (c) 2005-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "gui.h"
#include "cursors.h"

/* Built-in cursors */
#include "cursors/fill.xpm"
#include "cursors/erase.xpm"
#include "cursors/pick.xpm"
#include "cursors/hresize.xpm"
#include "cursors/vresize.xpm"
#include "cursors/lldiag.xpm"
#include "cursors/lrdiag.xpm"
#include "cursors/text.xpm"

static struct {
	char **data;
	int x, y;
} builtins[] = {
	{ NULL,		0,0 },
	{ fill_xpm,	23,25 },
	{ erase_xpm,	10,20 },
	{ pick_xpm,	8,22 },
	{ hresize_xpm,	16,17 },
	{ vresize_xpm,	15,16 },
	{ lldiag_xpm,	16,15 },
	{ lrdiag_xpm,	16,15 },
	{ text_xpm,	15,15 }
};

/* Create a new cursor from raw bitmap data and a transparency mask. */
AG_Cursor *
AG_CursorNew(void *obj, Uint w, Uint h, const Uint8 *data, const Uint8 *mask,
    int xHot, int yHot)
{
	AG_Driver *drv = obj;
	AG_Cursor *curs, *cursNew;
	Uint size = w*h;

	if ((cursNew = TryRealloc(drv->cursors,
	    (drv->nCursors+1)*sizeof(AG_Cursor))) == NULL) {
		return (NULL);
	}
	drv->cursors = cursNew;
	curs = &drv->cursors[drv->nCursors++];

	if ((curs->data = TryMalloc(size)) == NULL ||
	    (curs->mask = TryMalloc(size)) == NULL) {
		goto fail;
	}
	memcpy(curs->data, data, size);
	memcpy(curs->mask, mask, size);
	curs->w = w;
	curs->h = h;
	curs->xHot = xHot;
	curs->yHot = yHot;

	if (AGDRIVER_CLASS(drv)->createCursor(drv, curs) == -1) {
		goto fail;
	}
	return (curs);
fail:
	Free(curs->data);
	Free(curs->mask);
	drv->nCursors--;
	return (NULL);
}

static __inline__ void
FreeCursor(AG_Driver *drv, AG_Cursor *curs)
{
	AGDRIVER_CLASS(drv)->freeCursor(drv, curs);
	Free(curs->data);
	Free(curs->mask);
}

/* Delete a registered cursor. */
void
AG_CursorFree(void *obj, AG_Cursor *curs)
{
	AG_Driver *drv = obj;
	int i;

	for (i = 0; i < drv->nCursors; i++) {
		if (&drv->cursors[i] == curs)
			break;
	}
	if (i == drv->nCursors)
		AG_FatalError("No such cursor");

	FreeCursor(drv, curs);

	if (i < drv->nCursors-1) {
		memmove(&drv->cursors[i], &drv->cursors[i+1],
		    (drv->nCursors-1)*sizeof(AG_Cursor));
	}
	drv->nCursors--;
}

/* Create a cursor from the contents of an XPM file. */
AG_Cursor *
AG_CursorFromXPM(void *drv, char *xpm[], int xHot, int yHot)
{
	int i = -1, x, y;
	Uint8 data[4*AG_CURSOR_MAX_W*AG_CURSOR_MAX_H];
	Uint8 mask[4*AG_CURSOR_MAX_W*AG_CURSOR_MAX_H];
	int w, h;

	sscanf(xpm[0], "%d %d", &w, &h);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (x%8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				i++;
				data[i] = 0;
				mask[i] = 0;
			}
			switch (xpm[y+4][x]) {
			case '.':
				mask[i] |= 0x01;
				break;
			case '+':
				data[i] |= 0x01;
				mask[i] |= 0x01;
				break;
			case ' ':
				break;
			default:
				break;
			}
		}
	}
	return AG_CursorNew(drv, w,h, data, mask, xHot,yHot);
}

/* Initialize Agar's set of built-in cursors. */
int
AG_InitStockCursors(AG_Driver *drv)
{
	AG_Cursor *ac;
	int i;

	for (i = 1; i < AG_LAST_CURSOR; i++) {
		ac = AG_CursorFromXPM(drv, builtins[i].data,
		    builtins[i].x, builtins[i].y);
		if (ac == NULL)
			goto fail;
	}
	return (0);
fail:
	AG_FreeCursors(drv);
	return (-1);
}

/* Free all cursors allocated by a driver. */
void
AG_FreeCursors(AG_Driver *drv)
{
	int i;

	for (i = 0; i < drv->nCursors; i++) {
		FreeCursor(drv, &drv->cursors[i]);
	}
	Free(drv->cursors);
	drv->cursors = NULL;
}

#ifdef AG_LEGACY
void AG_SetCursor(int builtin) { }
void AG_UnsetCursor(void) { }
#endif /* AG_LEGACY */
