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
#include <agar/gui/combo.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/radio.h>
#include <agar/gui/textbox.h>
#include <agar/gui/tlist.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>

#include <string.h>

void
RG_TextureInit(RG_Texture *tex, const char *name)
{
	Strlcpy(tex->name, name, sizeof(tex->name));
	tex->tileset[0] = '\0';
	tex->tile[0] = '\0';
	tex->t = NULL;
	tex->wrap_s = RG_TEXTURE_REPEAT;
	tex->wrap_t = RG_TEXTURE_REPEAT;
	tex->flags = 0;
	tex->blend_func = (int)AG_ALPHA_OVERLAY;
	tex->alpha = 255;
}

void
RG_TextureDestroy(RG_Texture *tex)
{
}

int
RG_TextureLoad(RG_Texture *tex, AG_DataSource *buf)
{
	AG_CopyString(tex->tileset, buf, sizeof(tex->tileset));
	AG_CopyString(tex->tile, buf, sizeof(tex->tile));
	tex->flags = (int)AG_ReadUint32(buf);
	tex->wrap_s = (Uint)AG_ReadUint8(buf);
	tex->wrap_t = (Uint)AG_ReadUint8(buf);
	tex->blend_func = (int)AG_ReadUint8(buf);
	tex->alpha = AG_ReadUint8(buf);
	return (0);
}

void
RG_TextureSave(RG_Texture *tex, AG_DataSource *buf)
{
	AG_WriteString(buf, tex->tileset);
	AG_WriteString(buf, tex->tile);
	AG_WriteUint32(buf, (Uint32)tex->flags);
	AG_WriteUint8(buf, (Uint8)tex->wrap_s);
	AG_WriteUint8(buf, (Uint8)tex->wrap_t);
	AG_WriteUint8(buf, (Uint8)tex->blend_func);
	AG_WriteUint8(buf, tex->alpha);
}

RG_Texture *
RG_TextureFind(RG_Tileset *ts, const char *texname)
{
	RG_Texture *tex;

	TAILQ_FOREACH(tex, &ts->textures, textures) {
		if (strcmp(tex->name, texname) == 0)
			break;
	}
	if (tex == NULL ||
	   (tex->t = RG_TilesetResvTile(OBJECT(ts)->root, tex->tileset, 
	    tex->tile)) == NULL) {
		return (NULL);
	}
	return (tex);
}

#if 0
static void
find_tilesets(AG_Tlist *_Nonnull tl, AG_Object *_Nonnull pob, int depth)
{
	AG_Object *cob;
	AG_TlistItem *it;
	
	if (AG_OfClass(pob, "RG_Tileset:*")) {
		it = AG_TlistAdd(tl, AG_ObjectIcon(pob), "%s%s", pob->name,
		    OBJECT_RESIDENT(pob) ? _(" (resident)") : "");
		it->p1 = pob;
	}
	TAILQ_FOREACH(cob, &pob->children, cobjs)
		find_tilesets(tl, cob, depth+1);
}

static void
PollTilesets(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();

	AG_TlistClear(tl);
	AG_LockLinkage();
	find_tilesets(tl, agWorld, 0);
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}
#endif

static void
PollSourceTiles(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *vfsRoot = AG_OBJECT_PTR(1);
	RG_Texture *tex = AG_PTR(2);
	RG_Tileset *ts;
	RG_Tile *t;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	if (tex->tileset[0] != '\0' &&
	    (ts = AG_ObjectFindS(vfsRoot, tex->tileset)) != NULL &&
	    AG_OfClass(ts, "RG_Tileset:*")) {
		TAILQ_FOREACH(t, &ts->tiles, tiles) {
			it = AG_TlistAddS(tl, NULL, t->name);
			it->p1 = t;
			AG_TlistSetIcon(tl, it, t->su);
		}
	}
	AG_TlistRestore(tl);
}

static void
SelectTileset(AG_Event *_Nonnull event)
{
	RG_Texture *tex = AG_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	RG_Tileset *ts = it->p1;

	AG_ObjectCopyName(ts, tex->tileset, sizeof(tex->tileset));
	tex->tile[0] = '\0';
}

static void
SelectSourceTile(AG_Event *_Nonnull event)
{
	RG_Texture *tex = AG_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	RG_Tile *t = it->p1;

	Strlcpy(tex->tile, t->name, sizeof(tex->tile));
}

AG_Window *
RG_TextureEdit(void *vfsRoot, RG_Texture *tex)
{
	const char *wrapModes[] ={
		N_("Repeat"),
		N_("Clamp"),
		N_("Clamp to edge"),
		N_("Clamp to border"),
		NULL
	};
	AG_Window *win;
	AG_Combo *comTS;
	AG_Tlist *tl;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Textbox *tb;
	AG_Numerical *num;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, tex->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	tb = AG_TextboxNew(win, 0, _("Name: "));
	AG_TextboxBindUTF8(tb, tex->name, sizeof(tex->name));
	AG_WidgetFocus(tb);

	comTS = AG_ComboNew(win, AG_COMBO_POLL|AG_COMBO_HFILL, _("Tileset: "));
#if 0
	AG_SetEvent(comTS->list, "tlist-poll", PollTilesets, NULL);
#endif
	AG_SetEvent(comTS, "combo-selected", SelectTileset, "%p", tex);
	AG_ComboSelectText(comTS, tex->tileset);
	AG_TextboxSetString(comTS->tbox, tex->tileset);

	tl = AG_TlistNew(win, AG_TLIST_POLL|AG_TLIST_EXPAND);
	AG_SetEvent(tl, "tlist-poll", PollSourceTiles, "%p,%p", vfsRoot, tex);
	AG_SetEvent(tl, "tlist-selected", SelectSourceTile, "%p", tex);
	AG_TlistSelectText(tl, tex->tile);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAdd(nb, _("S-Wrapping"), AG_BOX_VERT);
	{
		AG_LabelNewS(ntab, 0, _("S-coordinate wrapping: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL, wrapModes, &tex->wrap_s);
	}
	ntab = AG_NotebookAdd(nb, _("T-Wrapping"), AG_BOX_VERT);
	{
		AG_LabelNewS(ntab, 0, _("T-coordinate wrapping: "));
		AG_RadioNewUint(ntab, AG_RADIO_HFILL, wrapModes, &tex->wrap_t);
	}
	ntab = AG_NotebookAdd(nb, _("Blending"), AG_BOX_VERT);
	{
		AG_LabelNewS(ntab, 0, _("Blending function: "));
		AG_RadioNewInt(ntab, AG_RADIO_HFILL, agAlphaFuncNames,
		    &tex->blend_func);
	}

	num = AG_NumericalNew(win, 0, NULL, _("Overall alpha: "));
	AG_BindUint8(num, "value", &tex->alpha);

	AG_WidgetFocus(comTS);
	return (win);
}
