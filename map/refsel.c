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
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>

#include <agar/map/map.h>
#include <agar/map/refsel.h>

void
MAP_UpdateRefSel(MAP_View *mv, int xRel, int yRel)
{
	MAP *m = mv->map;
	MAP_Item *r;
	int nx, ny;

	for (ny = mv->my;
	     (ny - mv->my) <= (int)mv->mh && ny < (int)m->maph;
	     ny++) {
		for (nx = mv->mx;
		     (nx - mv->mx) <= (int)mv->mw && nx < (int)m->mapw;
		     nx++) {
			MAP_Node *node = &m->map[ny][nx];

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if ((r->flags & MAP_ITEM_SELECTED) == 0) {
					continue;
				}
				r->r_gfx.xcenter += xRel;
				r->r_gfx.ycenter += yRel;

				if (xRel > 0 && r->r_gfx.xcenter > MAPTILESZ) {
					r->r_gfx.xcenter = MAPTILESZ;
				} else if (xRel<0 && r->r_gfx.xcenter < 0) {
					r->r_gfx.xcenter = 0;
				}
				
				if (yRel > 0 && r->r_gfx.ycenter > MAPTILESZ) {
					r->r_gfx.ycenter = MAPTILESZ;
				} else if (yRel < 0 && r->r_gfx.ycenter < 0) {
					r->r_gfx.ycenter = 0;
				}
			}
		}
	}
}

static void
Init(void *_Nonnull p)
{
	MAP_ToolPushStatus(p,
	    _("Select an element with $(L). Hold $(C) to select "
	      "multiple elements."));
}

const MAP_ToolOps mapRefselOps = {
	"Refsel", N_("Select Node Elements"),
	&mapIconSelectItem,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
