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
#include <agar/gui/text.h>

#include <agar/map/map.h>

static void
Init(void *_Nonnull p)
{
	MAP_ToolPushStatus(p, _("Specify element and $(L) to invert."));
}

static int
MouseButtonDown(void *_Nonnull p, int x, int y, int btn)
{
	MAP_ModBegin(TOOL(p)->mv->map);
	return (0);
}

static int
MouseButtonUp(void *_Nonnull p, int x, int y, int btn)
{
	MAP_Tool *t = p;
	MAP_View *mv = t->mv;
	MAP *m = mv->map;

	if (m->nMods == 0) {
		MAP_ModCancel(m);
	}
	MAP_ModEnd(m);
	return (0);
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull n)
{
	MAP_View *mv = TOOL(p)->mv;
	MAP *m = mv->map;
	MAP_Item *nref;
	RG_Transform *xf;
	int nmods = 0;

	MAP_ModNodeChg(m, mv->cx, mv->cy);

	TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;
		
		nmods++;

		TAILQ_FOREACH(xf, &nref->transforms, transforms) {
			if (xf->type == RG_TRANSFORM_RGB_INVERT) {
				TAILQ_REMOVE(&nref->transforms, xf,
				    transforms);
				break;
			}
		}
		if (xf != NULL)
			continue;

		if ((xf = RG_TransformNew(RG_TRANSFORM_RGB_INVERT, 0, NULL))
		    == NULL) {
			AG_TextMsgFromError();
			continue;
		}
		TAILQ_INSERT_TAIL(&nref->transforms, xf, transforms);
		break;
	}
	return (nmods);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
	AG_Color c;

	AG_ColorRGBA_8(&c, 255,255,255,64);
	AG_DrawRectBlended(TOOL(p)->mv, rd, &c,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	return (1);
}

const MAP_ToolOps mapInvertOps = {
	"Invert", N_("Invert RGB values of tile"),
	&mapIconInvert,
	sizeof(MAP_Tool),
	0,
	1,
	Init,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	Cursor,
	Effect,
	NULL,			/* mousemotion */
	MouseButtonDown,
	MouseButtonUp,
	NULL,			/* keydown */
	NULL			/* keyup */
};
