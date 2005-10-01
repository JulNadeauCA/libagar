/*	$Csoft: texture.c,v 1.9 2005/09/27 00:25:19 vedge Exp $	*/

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

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/combo.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/radio.h>

#include "tileset.h"
#include "tileview.h"

void
RG_TextureInit(RG_Texture *tex, RG_Tileset *ts, const char *name)
{
	strlcpy(tex->name, name, sizeof(tex->name));
	tex->tileset[0] = '\0';
	tex->tile[0] = '\0';
	tex->t = NULL;
	tex->wrap_s = RG_TEXTURE_REPEAT;
	tex->wrap_t = RG_TEXTURE_REPEAT;
	tex->flags = 0;
	tex->blend_func = AG_ALPHA_OVERLAY;
	tex->alpha = 255;
}

void
RG_TextureDestroy(RG_Texture *tex)
{
}

int
RG_TextureLoad(RG_Texture *tex, AG_Netbuf *buf)
{
	AG_CopyString(tex->tileset, buf, sizeof(tex->tileset));
	AG_CopyString(tex->tile, buf, sizeof(tex->tile));
	tex->flags = (int)AG_ReadUint32(buf);
	tex->wrap_s = (enum texture_wrap_mode)AG_ReadUint8(buf);
	tex->wrap_t = (enum texture_wrap_mode)AG_ReadUint8(buf);
	tex->blend_func = (enum ag_blend_func)AG_ReadUint8(buf);
	tex->alpha = AG_ReadUint8(buf);
	return (0);
}

void
RG_TextureSave(RG_Texture *tex, AG_Netbuf *buf)
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
	   (tex->t = RG_TilesetResvTile(tex->tileset, tex->tile)) == NULL) {
		return (NULL);
	}
	return (tex);
}

static void
find_tilesets(AG_Tlist *tl, AG_Object *pob, int depth)
{
	AG_Object *cob;
	AG_TlistItem *it;
	
	if (AGOBJECT_TYPE(pob, "tileset")) {
		it = AG_TlistAdd(tl, AG_ObjectIcon(pob), "%s%s", pob->name,
		    (pob->flags & AG_OBJECT_DATA_RESIDENT) ?
		    _(" (resident)") : "");
		it->p1 = pob;
	}
	TAILQ_FOREACH(cob, &pob->children, cobjs)
		find_tilesets(tl, cob, depth+1);
}

static void
poll_tilesets(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	AG_LockLinkage();
	find_tilesets(tl, agWorld, 0);
	AG_UnlockLinkage();
	AG_TlistRestore(tl);
}

static void
poll_src_tiles(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Texture *tex = argv[1].p;
	RG_Tileset *ts;
	RG_Tile *t;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	if (tex->tileset[0] != '\0' &&
	    (ts = AG_ObjectFind(tex->tileset)) != NULL &&
	    AGOBJECT_TYPE(ts, "tileset")) {
		TAILQ_FOREACH(t, &ts->tiles, tiles) {
			it = AG_TlistAdd(tl, NULL, t->name);
			it->p1 = t;
			AG_TlistSetIcon(tl, it, t->su);
		}
	}
	AG_TlistRestore(tl);
}

static void
select_tileset(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Texture *tex = argv[1].p;
	AG_TlistItem *it = argv[2].p;
	RG_Tileset *ts = it->p1;

	AG_ObjectCopyName(ts, tex->tileset, sizeof(tex->tileset));
	tex->tile[0] = '\0';
}

static void
select_src_tile(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Texture *tex = argv[1].p;
	AG_TlistItem *it = argv[2].p;
	RG_Tile *t = it->p1;

	strlcpy(tex->tile, t->name, sizeof(tex->tile));
}

static int
compare_pixmap_ents(const AG_TlistItem *it1, const AG_TlistItem *it2)
{
	return (0);
}

AG_Window *
RG_TextureEdit(RG_Texture *tex)
{
	const char *wrap_modes[] ={
		N_("Repeat"),
		N_("Clamp"),
		N_("Clamp to edge"),
		N_("Clamp to border"),
		NULL
	};
	AG_Window *win;
	AG_Box *bo;
	AG_Combo *com;
	AG_Tlist *tl;
	AG_Spinbutton *sb;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Radio *rad;
	AG_Textbox *tb;
	
	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, "%s", tex->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	tb = AG_TextboxNew(win, _("Name: "));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, tex->name,
	    sizeof(tex->name));

	com = AG_ComboNew(win, AG_COMBO_POLL, _("Tileset: "));
	AG_SetEvent(com->list, "tlist-poll", poll_tilesets, NULL);
	AG_SetEvent(com, "combo-selected", select_tileset, "%p", tex);
	AG_ComboSelectText(com, tex->tileset);
	AG_TextboxPrintf(com->tbox, "%s", tex->tileset);

	tl = AG_TlistNew(win, AG_TLIST_POLL);
	AG_SetEvent(tl, "tlist-poll", poll_src_tiles, "%p", tex);
	AG_SetEvent(tl, "tlist-selected", select_src_tile, "%p", tex);
	AG_TlistSelectText(tl, tex->tile);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL);
	ntab = AG_NotebookAddTab(nb, _("Wrapping"), AG_BOX_VERT);
	{
		AG_LabelStatic(ntab, _("S-coordinate: "));
		rad = AG_RadioNew(ntab, wrap_modes);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &tex->wrap_s);
		
		AG_LabelStatic(ntab, _("T-coordinate: "));
		rad = AG_RadioNew(ntab, wrap_modes);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &tex->wrap_t);
	}

	ntab = AG_NotebookAddTab(nb, _("Blending"), AG_BOX_VERT);
	{
		AG_LabelStatic(ntab, _("Blending function: "));
		rad = AG_RadioNew(ntab, agBlendFuncNames);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &tex->blend_func);
	}

	sb = AG_SpinbuttonNew(win, _("Overall alpha: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_UINT8, &tex->alpha);
	return (win);
}
