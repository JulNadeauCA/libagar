/*
 * Copyright (c) 2005-2021 Julien Nadeau Carriere <vedge@csoft.net>
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
	MAP *map = mv->map;
	int nx, ny;

	for (ny = mv->my;
	     (ny - mv->my) <= (int)mv->mh && ny < (int)map->h;
	     ny++) {
		for (nx = mv->mx;
		     (nx - mv->mx) <= (int)mv->mw && nx < (int)map->w;
		     nx++) {
			MAP_Node *node = &map->map[ny][nx];
			MAP_Item *mi;

			TAILQ_FOREACH(mi, &node->items, items) {
				MAP_Tile *mt;

				if (mi->type != MAP_ITEM_TILE ||
				    (mi->flags & MAP_ITEM_SELECTED) == 0) {
					continue;
				}
				mt = MAPTILE(mi);
				mt->xCenter += xRel;
				mt->yCenter += yRel;

				if (xRel > 0 && mt->xCenter > MAPTILESZ) {
					mt->xCenter = MAPTILESZ;
				} else if (xRel<0 && mt->xCenter < 0) {
					mt->xCenter = 0;
				}
				
				if (yRel > 0 && mt->yCenter > MAPTILESZ) {
					mt->yCenter = MAPTILESZ;
				} else if (yRel < 0 && mt->yCenter < 0) {
					mt->yCenter = 0;
				}
			}
		}
	}
}

static void
Init(void *_Nonnull tool)
{
	MAP_ToolPushStatus(tool,
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
