/*	$Csoft: tileview.c,v 1.20 2005/01/05 04:44:05 vedge Exp $	*/

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
#include <engine/view.h>

#include "tileview.h"

const struct widget_ops tileview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		tileview_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	tileview_draw,
	tileview_scale
};

struct tileview *
tileview_new(void *parent, struct tileset *ts, struct tile *t)
{
	struct tileview *tv;

	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, t);
	object_attach(parent, tv);
	return (tv);
}

void
tileview_init(struct tileview *tv, struct tileset *ts, struct tile *tile)
{
	widget_init(tv, "tileview", &tileview_ops, WIDGET_WFILL|WIDGET_HFILL);

	tv->ts = ts;
	tv->tile = tile;
}

void
tileview_scale(void *p, int rw, int rh)
{
	struct tileview *tv = p;

	if (rw == -1 && rh == -1) {
		WIDGET(tv)->w = tv->tile->su->w;
		WIDGET(tv)->h = tv->tile->su->h;
	}
}

void
tileview_draw(void *p)
{
	struct tileview *tv = p;
	
	widget_blit(tv, tv->tile->su, 0, 0);
}

void
tileview_destroy(void *p)
{
	struct tileview *tv = p;

	widget_destroy(tv);
}

