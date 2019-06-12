/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
	char *_Nullable *_Nonnull data;	/* Start of XPM */
	int x, y;			/* Hotspot */
} builtins[] = {
	{ fill_xpm,	23,25 },	/* FILL */
	{ erase_xpm,	10,20 },	/* ERASE */
	{ pick_xpm,	8,22 },		/* PICK */
	{ hresize_xpm,	16,17 },	/* HRESIZE */
	{ vresize_xpm,	15,16 },	/* VRESIZE */
	{ lldiag_xpm,	16,15 },	/* LLDIAG */
	{ lrdiag_xpm,	16,15 },	/* LRDIAG */
	{ text_xpm,	15,15 }		/* TEXT */
};

/* Create a new cursor from raw bitmap data and a transparency mask. */
AG_Cursor *
AG_CursorNew(void *obj, Uint w, Uint h, const Uint8 *data, const Uint8 *mask,
    int xHot, int yHot)
{
	AG_Driver *drv = obj;
	AG_Cursor *ac;

	ac = AGDRIVER_CLASS(drv)->createCursor(drv, w,h, data,mask, xHot,yHot);
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

/* Create a cursor from XPM data. */
AG_Cursor *
AG_CursorFromXPM(void *drv, char *xpm[], int xHot, int yHot)
{
	int i = -1, x,y, size;
	Uint8 *data, *mask;
	AG_Cursor *curs;
	int w,h;

	size = (AG_CURSOR_MAX_W * AG_CURSOR_MAX_H * 4);
	data = Malloc(size);
	mask = Malloc(size);

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
	curs = AG_CursorNew(drv, w,h, data, mask, xHot,yHot);
	free(data);
	free(mask);
	return (curs);
}

/* Initialize Agar's set of built-in cursors. */
void
AG_InitStockCursors(AG_Driver *drv)
{
	int i;

	for (i = 0; i < AG_LAST_CURSOR-1; i++) {
		(void)AG_CursorFromXPM(drv,
		    builtins[i].data,
		    builtins[i].x,
		    builtins[i].y);
	}
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

/* Initialize an AG_Cursor structure. */
void
AG_CursorInit(AG_Cursor *ac)
{
	ac->data = NULL;
	ac->mask = NULL;
	ac->w = 0;
	ac->h = 0;
	ac->xHot = 0;
	ac->yHot = 0;
	ac->p = NULL;
}

/* Return a pointer to a built-in cursor. */
AG_Cursor *
AG_GetStockCursor(void *obj, int name)
{
	AG_Driver *drv = AGDRIVER(obj);
	AG_Cursor *ac;
	int i = 0;

	AG_TAILQ_FOREACH(ac, &drv->cursors, cursors) {
		if (i++ == name)
			break;
	}
	if (ac == NULL) {
		AG_FatalError("AG_GetStockCursor");
	}
	return (ac);
}

/* Return a pointer to the active cursor. */
AG_Cursor *
AG_GetActiveCursor(void *drv)
{
	return (AGDRIVER(drv)->activeCursor);
}

/* Test if cursor is visible */
int
AG_CursorIsVisible(void *drv)
{
	return AGDRIVER_CLASS(drv)->getCursorVisibility(drv);
}

/* Display the cursor */
void
AG_ShowCursor(void *drv)
{
	AGDRIVER_CLASS(drv)->setCursorVisibility(drv, 1);
}

/* Hide the cursor */
void
AG_HideCursor(void *drv)
{
	AGDRIVER_CLASS(drv)->setCursorVisibility(drv, 0);
}
