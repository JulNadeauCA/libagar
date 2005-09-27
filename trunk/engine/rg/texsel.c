/*	$Csoft: texsel.c,v 1.4 2005/08/29 03:29:05 vedge Exp $	*/

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

#include "texsel.h"

RG_TextureSelector *
RG_TextureSelectorNew(void *parent, RG_Tileset *tset, int flags)
{
	RG_TextureSelector *ts;

	ts = Malloc(sizeof(RG_TextureSelector), M_OBJECT);
	RG_TextureSelectorInit(ts, tset, flags);
	AG_ObjectAttach(parent, ts);
	return (ts);
}

static void
poll_textures(int argc, union evarg *argv)
{
	RG_TextureSelector *ts = argv[0].p;
	AG_Tlist *tl = (AG_Tlist *)ts;
	RG_Texture *tex;
	AG_TlistItem *it;

	AG_TlistClear(tl);

	TAILQ_FOREACH(tex, &ts->tset->textures, textures) {
		RG_Tile *t;

		if (tex->tileset[0] != '\0' && tex->tile[0] != '\0' &&
		    (t = RG_TilesetResvTile(tex->tileset, tex->tile))
		     != NULL) {
			it = AG_TlistAdd(tl, NULL, "%s (<%s> %ux%u)",
			    tex->name, t->name, t->su->w, t->su->h);
			it->class = "texture";
			it->p1 = tex;
			AG_TlistSetIcon(tl, it, t->su);
		}
	}
	AG_TlistRestore(tl);
}

static void
select_texture(int argc, union evarg *argv)
{
	RG_TextureSelector *ts = argv[0].p;
	AG_Tlist *tl = (AG_Tlist *)ts;
	AG_TlistItem *it;
	RG_Texture *tex;
	AG_WidgetBinding *bTexname;
	char *texname;

	bTexname = AG_WidgetGetBinding(ts, "texture-name", &texname);
	if ((it = AG_TlistSelectedItem(tl)) != NULL) {
		tex = it->p1;
		strlcpy(texname, tex->name, bTexname->size);
	}
	AG_WidgetUnlockBinding(bTexname);
}

void
RG_TextureSelectorInit(RG_TextureSelector *ts, RG_Tileset *tset, int flags)
{
	AG_TlistInit(&ts->tl, AG_TLIST_POLL);
	AG_TlistSetItemHeight(&ts->tl, AGTILESZ);
	AG_SetEvent(&ts->tl, "tlist-poll", poll_textures, NULL);
	AG_SetEvent(&ts->tl, "tlist-selected", select_texture, NULL);
	
	ts->tset = tset;
	ts->flags = flags;
	ts->texname[0] = '\0';

	AG_WidgetBind(ts, "texture-name", AG_WIDGET_STRING, ts->texname,
	    sizeof(ts->texname));
}

void
RG_TextureSelectorDestroy(void *p)
{
	RG_TextureSelector *ts = p;

	AG_TlistDestroy(ts);
}

