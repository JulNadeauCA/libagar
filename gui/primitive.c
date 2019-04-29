/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/primitive.h>

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_primitive.h"

/*
 * Render a gimp-style background tiling.
 */
void
AG_DrawTiling(void *obj, const AG_Rect *r, int tsz, int offs,
    const AG_Color *c1, const AG_Color *c2)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Driver *drv = wid->drv;
	AG_Rect rd;
	int alt1 = 0, alt2 = 0;
	int x = wid->rView.x1 + r->x;
	int y = wid->rView.y1 + r->y;
	int x2 = x + r->w;
	int y2 = y + r->h;
	int tsz_offs = tsz+offs;

	rd.w = tsz;
	rd.h = tsz;

	/* XXX inelegant */
	for (rd.y = y-tsz_offs; rd.y < y2; rd.y += tsz) {
		for (rd.x = x-tsz_offs; rd.x < x2; rd.x += tsz) {
			if (alt1++ == 1) {
				wid->drvOps->drawRectFilled(drv, &rd, c1);
				alt1 = 0;
			} else {
				wid->drvOps->drawRectFilled(drv, &rd, c2);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}
