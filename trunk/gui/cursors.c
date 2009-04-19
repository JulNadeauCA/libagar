/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <gui/view.h>

#include "cursors.h"
#include "cursors/fill.xpm"
#include "cursors/erase.xpm"
#include "cursors/pick.xpm"
#include "cursors/hresize.xpm"
#include "cursors/vresize.xpm"
#include "cursors/lldiag.xpm"
#include "cursors/lrdiag.xpm"
#include "cursors/text.xpm"

#define CURSOR_MAX_W 32
#define CURSOR_MAX_H 32

SDL_Cursor *agCursors[AG_LAST_CURSOR];
SDL_Cursor *agDefaultCursor = NULL;

static SDL_Cursor *
GenCursor(char *xpm[], int xHot, int yHot)
{
	int i = -1, row, col;
	Uint8 data[4*CURSOR_MAX_W];
	Uint8 mask[4*CURSOR_MAX_H];
	int w, h;

	sscanf(xpm[0], "%d %d", &w, &h);

	for (row = 0; row < h; row++) {
		for (col = 0; col < w; col++) {
			if (col % 8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			} else {
				i++;
				data[i] = 0;
				mask[i] = 0;
			}
			switch (xpm[row+4][col]) {
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
	return SDL_CreateCursor(data, mask, w, h, xHot, yHot);
}

void
AG_CursorsInit(void)
{
	agDefaultCursor = SDL_GetCursor();
	agCursors[AG_FILL_CURSOR] = GenCursor(fill_xpm, 23, 25);
	agCursors[AG_ERASE_CURSOR] = GenCursor(erase_xpm, 10, 20);
	agCursors[AG_PICK_CURSOR] = GenCursor(pick_xpm, 8, 22);
	agCursors[AG_HRESIZE_CURSOR] = GenCursor(hresize_xpm, 16, 17);
	agCursors[AG_VRESIZE_CURSOR] = GenCursor(vresize_xpm, 15, 16);
	agCursors[AG_LLDIAG_CURSOR] = GenCursor(lldiag_xpm, 16, 15);
	agCursors[AG_LRDIAG_CURSOR] = GenCursor(lrdiag_xpm, 16, 15);
	agCursors[AG_TEXT_CURSOR] = GenCursor(text_xpm, 15, 15);
}

void
AG_CursorsDestroy(void)
{
	int i;

	for (i = 0; i < AG_LAST_CURSOR; i++)
		SDL_FreeCursor(agCursors[i]);
}
