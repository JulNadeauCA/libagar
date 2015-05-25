/*
 * Copyright (c) 2005-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/cursors.h>

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
	AG_Cursor *ac;

	ac = AGDRIVER_CLASS(drv)->createCursor(drv, w, h, data, mask,
	    xHot, yHot);
	if (ac == NULL) {
		return (NULL);
	}
	TAILQ_INSERT_TAIL(&drv->cursors, ac, cursors);
	drv->nCursors++;
	return (ac);
}

/* Delete a registered cursor. */
void
AG_CursorFree(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;

	TAILQ_REMOVE(&drv->cursors, ac, cursors);
	drv->nCursors--;
	AGDRIVER_CLASS(drv)->freeCursor(drv, ac);
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
	AG_Cursor *ac, *acNext;

	for (ac = TAILQ_FIRST(&drv->cursors);
	     ac != TAILQ_END(&drv->cursors);
	     ac = acNext) {
		acNext = TAILQ_NEXT(ac, cursors);
		AGDRIVER_CLASS(drv)->freeCursor(drv, ac);
	}
	TAILQ_INIT(&drv->cursors);
	drv->nCursors = 0;
}
