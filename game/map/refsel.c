/*	$Csoft: refsel.c,v 1.7 2005/08/27 04:34:06 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#ifdef MAP

#include "map.h"
#include "mapedit.h"
#include "refsel.h"

void
refsel_update(AG_Mapview *mv, int xRel, int yRel)
{
	AG_Map *m = mv->map;
	AG_Nitem *r;
	int nx, ny;
	int tilesz = AGMTILESZ(mv);

	for (ny = mv->my;
	     (ny - mv->my) <= mv->mh && ny < m->maph;
	     ny++) {
		for (nx = mv->mx;
		     (nx - mv->mx) <= mv->mw && nx < m->mapw;
		     nx++) {
			AG_Node *node = &m->map[ny][nx];

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if ((r->flags & AG_NITEM_SELECTED) == 0) {
					continue;
				}
				r->r_gfx.xcenter += xRel;
				r->r_gfx.ycenter += yRel;

				if (xRel > 0 && r->r_gfx.xcenter > AGTILESZ) {
					r->r_gfx.xcenter = AGTILESZ;
				} else if (xRel<0 && r->r_gfx.xcenter < 0) {
					r->r_gfx.xcenter = 0;
				}
				
				if (yRel > 0 && r->r_gfx.ycenter > AGTILESZ) {
					r->r_gfx.ycenter = AGTILESZ;
				} else if (yRel < 0 && r->r_gfx.ycenter < 0) {
					r->r_gfx.ycenter = 0;
				}
			}
		}
	}
}

static void
refsel_init(void *p)
{
	AG_MaptoolPushStatus(p,
	    _("Select an element with $(L). Hold $(C) to select "
	      "multiple elements."));
}

const AG_MaptoolOps agMapRefselOps = {
	"Refsel", N_("Select Node Elements"),
	SELECT_REF_ICON,
	sizeof(AG_Maptool),
	0,
	refsel_init,
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

#endif /* MAP */
