/*
 * Copyright (c) 2003-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/checkbox.h>

#include <agar/map/map.h>

static int mapFlipSelection = 0;	/* XXX instance */

static void
Init(void *_Nonnull p)
{
	MAP_ToolPushStatus(p,
	    _("Select element and use $(L)=Mirror, $(R)=Flip"));
}

static void
EditPane(void *_Nonnull p, void *_Nonnull con)
{
	AG_CheckboxNewInt(con, 0, _("Flip entire selection"),
	    &mapFlipSelection);
}

static void
ToggleXform(MAP_Item *_Nonnull mi, int type)
{
	RG_Transform *xf;

	TAILQ_FOREACH(xf, &mi->transforms, transforms) {
		if (xf->type == type) {
			TAILQ_REMOVE(&mi->transforms, xf, transforms);
			return;
		}
	}
	if ((xf = RG_TransformNew(type, 0, NULL)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	TAILQ_INSERT_TAIL(&mi->transforms, xf, transforms);
}

static int
MouseButtonDown(void *_Nonnull p, int xmap, int ymap, int b)
{
	MAP_Tool *t = p;
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	int selx = mv->mx + mv->mouse.x;
	int sely = mv->my + mv->mouse.y;
	int w = 1;
	int h = 1;
	int x, y;

	if (!mapFlipSelection ||
	    MAP_ViewGetSelection(mv, &selx, &sely, &w, &h) == -1) {
		if (selx < 0 || selx >= (int)m->mapw ||
		    sely < 0 || sely >= (int)m->maph)
			return (0);
	}

	MAP_ModBegin(m);

	for (y = sely; y < sely+h; y++) {
		for (x = selx; x < selx+w; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *nref;

			MAP_ModNodeChg(m, x, y);
			
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer != m->cur_layer)
					continue;

				if (b == AG_MOUSE_LEFT) {
					ToggleXform(nref, RG_TRANSFORM_MIRROR);
				} else if (b == AG_MOUSE_RIGHT) {
					ToggleXform(nref, RG_TRANSFORM_FLIP);
				}
			}
		}
	}

	MAP_ModEnd(m);
	return (1);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
	AG_Color c;

	AG_ColorRGBA_8(&c, 255,255,0,64);
	AG_DrawRectBlended(TOOL(p)->mv, rd, &c,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	return (1);
}

const MAP_ToolOps mapFlipOps = {
	"Flip", N_("Flip/mirror node element"),
	&mapIconFlip,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	EditPane,
	NULL,			/* edit */
	Cursor,
	NULL,			/* effect */
	NULL,			/* mousemotion */
	MouseButtonDown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
