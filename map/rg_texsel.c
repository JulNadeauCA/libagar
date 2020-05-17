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

/*
 * Simple texture selection widget.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/tlist.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_texsel.h>

RG_TextureSelector *
RG_TextureSelectorNew(void *parent, RG_Tileset *tset, Uint flags)
{
	RG_TextureSelector *ts;

	ts = Malloc(sizeof(RG_TextureSelector));
	AG_ObjectInit(ts, &rgTextureSelectorClass);
	ts->flags |= flags;
	ts->tset = tset;
	AG_ObjectAttach(parent, ts);
	return (ts);
}

static void
PollTextures(AG_Event *_Nonnull event)
{
	RG_TextureSelector *ts = AG_SELF();
	AG_Tlist *tl = (AG_Tlist *)ts;
	RG_Texture *tex;
	AG_TlistItem *it;

	AG_TlistClear(tl);

	TAILQ_FOREACH(tex, &ts->tset->textures, textures) {
		RG_Tile *t;

		if (tex->tileset[0] != '\0' && tex->tile[0] != '\0' &&
		    (t = RG_TilesetResvTile(OBJECT(ts->tset)->root,
		     tex->tileset, tex->tile)) != NULL) {
			it = AG_TlistAdd(tl, NULL, "%s (<%s> %ux%u)",
			    tex->name, t->name, t->su->w, t->su->h);
			it->cat = "texture";
			it->p1 = tex;
			AG_TlistSetIcon(tl, it, t->su);
		}
	}
	AG_TlistRestore(tl);
}

static void
SelectTexture(AG_Event *_Nonnull event)
{
	RG_TextureSelector *ts = AG_SELF();
	AG_Tlist *tl = (AG_Tlist *)ts;
	AG_TlistItem *it = AG_TlistSelectedItem(tl);
	RG_Texture *tex = it->p1;

	AG_SetString(ts, "texture-name", tex->name);
}

static void
Init(void *_Nonnull obj)
{
	RG_TextureSelector *ts = obj;
	AG_Tlist *tl = obj;

	WIDGET(ts)->flags |= AG_WIDGET_EXPAND;

	tl->flags |= AG_TLIST_POLL;
	AG_TlistSetItemHeight(tl, RG_TILESZ);
	AG_SetEvent(tl, "tlist-poll", PollTextures, NULL);
	AG_SetEvent(tl, "tlist-selected", SelectTexture, NULL);
	
	ts->tset = NULL;
	ts->flags = 0;
	ts->texname[0] = '\0';

	AG_BindString(ts, "texture-name", ts->texname, sizeof(ts->texname));
}

AG_WidgetClass rgTextureSelectorClass = {
	{
		"Agar(Widget:Tlist):RG(TextureSelector)",
		sizeof(RG_TextureSelector),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	NULL,			/* size_request */
	NULL			/* size_allocate */
};
