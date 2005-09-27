/*	$Csoft: pixmap.c,v 1.45 2005/09/27 00:25:19 vedge Exp $	*/

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
#include <engine/input.h>

#include <engine/map/map.h>

#include <engine/loader/surface.h>

#include <engine/widget/cursors.h>
#include <engine/widget/window.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/label.h>
#include <engine/widget/radio.h>
#include <engine/widget/tlist.h>
#include <engine/widget/separator.h>
#include <engine/widget/notebook.h>

#include "tileset.h"
#include "tileview.h"

static SDL_Cursor *saved_cursor = NULL;
int pixmap_source = 1;
const char *pixmap_state_names[] = {
	"",
	" (free-hand)",
	" (vertical)",
	" (horizontal)",
	" (diagonal)"
};

void
RG_PixmapInit(RG_Pixmap *px, RG_Tileset *ts, int flags)
{
	px->name[0] = '\0';
	px->ts = ts;
	px->flags = flags;
	px->xorig = 0;
	px->yorig = 0;
	px->su = NULL;
	px->nrefs = 0;
	px->ublks = Malloc(sizeof(struct rg_pixmap_undoblk), M_RG);
	px->nublks = 1;
	px->curblk = 0;

	px->h = 0.0;
	px->s = 1.0;
	px->v = 1.0;
	px->a = 1.0;
	px->curbrush = NULL;
	px->blend_mode = RG_PIXMAP_OVERLAY_ALPHA;
	TAILQ_INIT(&px->brushes);

	RG_PixmapBeginUndoBlk(px);
	px->ublks[0].mods = Malloc(sizeof(struct rg_pixmap_mod), M_RG);
	px->ublks[0].nmods = 0;
}

struct rg_pixmap_brush *
RG_PixmapAddBrush(RG_Pixmap *px, enum pixmap_brush_type type,
    RG_Pixmap *bpx)
{
	struct rg_pixmap_brush *br;

	br = Malloc(sizeof(struct rg_pixmap_brush), M_RG);
	br->type = type;
	br->name[0] = '\0';
	br->flags = 0;
	br->px = bpx;
	br->px->nrefs++;
	TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	return (br);
}

void
RG_PixmapDelBrush(RG_Pixmap *px, struct rg_pixmap_brush *br)
{
	TAILQ_REMOVE(&px->brushes, br, brushes);
	br->px->nrefs--;
	Free(br, M_RG);
}

int
RG_PixmapLoad(RG_Pixmap *px, AG_Netbuf *buf)
{
	Uint32 i, nbrushes;

	AG_CopyString(px->name, buf, sizeof(px->name));
	px->flags = (int)AG_ReadUint32(buf);
	px->xorig = (int)AG_ReadSint16(buf);
	px->yorig = (int)AG_ReadSint16(buf);
	if ((px->su = AG_ReadSurface(buf, agVideoFmt)) == NULL)
		return (-1);

	nbrushes = AG_ReadUint32(buf);
	for (i = 0; i < nbrushes; i++) {
		struct rg_pixmap_brush *br;

		br = Malloc(sizeof(struct rg_pixmap_brush), M_RG);
		AG_CopyString(br->name, buf, sizeof(br->name));
		br->type = (enum pixmap_brush_type)AG_ReadUint8(buf);
		br->flags = (int)AG_ReadUint32(buf);
		AG_ReadSint16(buf);			/* Pad: xorig */
		AG_ReadSint16(buf);			/* Pad: yorig */
		AG_CopyString(br->px_name, buf, sizeof(br->px_name));
		br->px = NULL;
		TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	}
	return (0);
}

void
RG_PixmapSave(RG_Pixmap *px, AG_Netbuf *buf)
{
	Uint32 nbrushes = 0;
	off_t nbrushes_offs;
	struct rg_pixmap_brush *br;

	AG_WriteString(buf, px->name);
	AG_WriteUint32(buf, (Uint32)px->flags);
	AG_WriteSint16(buf, (Sint16)px->xorig);
	AG_WriteSint16(buf, (Sint16)px->yorig);
	AG_WriteSurface(buf, px->su);

	nbrushes_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(br, &px->brushes, brushes) {
		AG_WriteString(buf, br->name);
		AG_WriteUint8(buf, (Uint8)br->type);
		AG_WriteUint32(buf, (Uint32)br->flags);
		AG_WriteSint16(buf, (Sint16)br->px->xorig);
		AG_WriteSint16(buf, (Sint16)br->px->yorig);
		AG_WriteString(buf, br->px->name);
		nbrushes++;
	}
	AG_PwriteUint32(buf, nbrushes, nbrushes_offs);
}

void
RG_PixmapDestroy(RG_Pixmap *px)
{
	struct rg_pixmap_brush *br, *nbr;
	int i;

	if (px->su != NULL)
		SDL_FreeSurface(px->su);

	for (i = 0; i < px->nublks; i++) {
		Free(px->ublks[i].mods, M_RG);
	}
	Free(px->ublks, M_RG);

	for (br = TAILQ_FIRST(&px->brushes);
	     br != TAILQ_END(&px->brushes);
	     br = nbr) {
		nbr = TAILQ_NEXT(br, brushes);
		Free(br, M_RG);
	}
}

/* Resize a pixmap and copy the previous surface at the given offset. */
void
RG_PixmapScale(RG_Pixmap *px, int w, int h, int xoffs, int yoffs)
{
	RG_Tileset *ts = px->ts;
	SDL_Surface *nsu;

	/* Create the new surface. */
	nsu = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
	    w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    ts->fmt->Amask);
	if (nsu == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

	/* Copy the old surface over. */
	if (px->su != NULL) {
		SDL_Rect rd;

		SDL_SetAlpha(nsu, px->su->flags &
		    (SDL_SRCALPHA|SDL_RLEACCEL),
		    px->su->format->alpha);
		SDL_SetColorKey(nsu, px->su->flags &
		    (SDL_SRCCOLORKEY|SDL_RLEACCEL),
		    px->su->format->colorkey);

		SDL_SetAlpha(px->su, 0, 0);
		SDL_SetColorKey(px->su, 0, 0);
		rd.x = xoffs;
		rd.y = yoffs;
		SDL_BlitSurface(px->su, NULL, nsu, &rd);
		SDL_FreeSurface(px->su);
	}
	px->su = nsu;
}

static void
update_tv(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	
	tv->tile->flags |= RG_TILE_DIRTY;
}

static void
poll_brushes(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Pixmap *px = argv[1].p;
	struct rg_pixmap_brush *br;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	it = AG_TlistAdd(tl, NULL, _("(none)"));
	it->class = "brush";
	it->p1 = NULL;
	TAILQ_FOREACH(br, &px->brushes, brushes) {
		it = AG_TlistAdd(tl, NULL, "%s%s %s%s",
		    (br == px->curbrush) ? "*" : "", br->name,
		    (br->flags & RG_PIXMAP_BRUSH_ONESHOT) ? _("one-shot ") : "",
		    (br->type == RG_PIXMAP_BRUSH_RGB) ? _("rgb") : _("mono"));
		it->class = "brush";
		it->p1 = br;
		AG_TlistSetIcon(tl, it, br->px->su);
	}
	AG_TlistRestore(tl);
}

static void
select_brush(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Pixmap *px = argv[1].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) != NULL) {
		struct rg_pixmap_brush *br = it->p1;

		px->curbrush = (px->curbrush == br) ? NULL : br;
	} else {
		px->curbrush = NULL;
	}
}

static void
poll_pixmaps(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	AG_TlistItem *it;
	RG_Pixmap *px;

	AG_TlistClear(tl);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = AG_TlistAdd(tl, NULL, "%s (%u refs)", px->name,
		    px->nrefs);
		it->class = "pixmap";
		it->p1 = px;
		AG_TlistSetIcon(tl, it, px->su);
	}
	AG_TlistRestore(tl);
}

static void
insert_brush(int argc, union evarg *argv)
{
	RG_Pixmap *px = argv[1].p;
	AG_Tlist *tl = argv[2].p;
	AG_Textbox *tb = argv[3].p;
	AG_Radio *rad_types = argv[4].p;
	AG_Checkbox *cb_oneshot = argv[5].p;
	AG_Window *dlg_win = argv[6].p;
	enum pixmap_brush_type btype;
	RG_Pixmap *spx;
	struct rg_pixmap_brush *pbr;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		return;
	}
	spx = it->p1;

	btype = (enum pixmap_brush_type)AG_WidgetInt(rad_types, "value");
	pbr = RG_PixmapAddBrush(px, btype, spx);
	AG_TextboxCopyString(tb, pbr->name, sizeof(pbr->name));
	if (pbr->name[0] == '\0') {
		strlcpy(pbr->name, spx->name, sizeof(pbr->name));
	}
	if (AG_WidgetInt(cb_oneshot, "state")) {
		pbr->flags |= RG_PIXMAP_BRUSH_ONESHOT;
	}
	AG_ViewDetach(dlg_win);
}

static void
update_bropts(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Textbox *tb_name = argv[1].p;
	AG_TlistItem *it = argv[2].p;
	RG_Pixmap *spx;

	if (it != NULL) {
		spx = it->p1;
		AG_TextboxPrintf(tb_name, "%s", spx->name);
	}
}

static void
insert_brush_dlg(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	RG_Pixmap *px = argv[2].p;
	AG_Window *win, *pwin = argv[3].p;
	AG_Tlist *tl;
	AG_Box *bo;
	AG_Textbox *tb_name;
	AG_Radio *rad_types;
	AG_Checkbox *cb_oneshot;
	static const char *types[] = {
		N_("Monochromatic"),
		N_("Source RGB"),
		NULL
	};

	win = AG_WindowNew(AG_WINDOW_NO_CLOSE|AG_WINDOW_NO_MINIMIZE, NULL);
	AG_WindowSetCaption(win, _("New %s brush"), px->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	tb_name = Malloc(sizeof(AG_Textbox), M_OBJECT);
	AG_TextboxInit(tb_name, _("Name: "));
	
	cb_oneshot = Malloc(sizeof(AG_Checkbox), M_OBJECT);
	AG_CheckboxInit(cb_oneshot, _("One-shot"));

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL|AG_BOX_HFILL);
	AG_BoxSetPadding(bo, 0);
	AG_BoxSetSpacing(bo, 0);
	{
		AG_LabelNew(bo, AG_LABEL_STATIC, _("Source pixmap:"));

		tl = AG_TlistNew(bo, AG_TLIST_POLL);
		AG_TlistSetItemHeight(tl, AGTILESZ);
		AG_TlistPrescale(tl, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_SetEvent(tl, "tlist-poll", poll_pixmaps, "%p", tv->ts);
		AG_SetEvent(tl, "tlist-selected", update_bropts, "%p", tb_name);
		AG_WidgetFocus(tl);
	}
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		rad_types = AG_RadioNew(bo, types);
		AG_WidgetSetInt(rad_types, "value", 0);
		AG_ObjectAttach(bo, tb_name);
		AG_ObjectAttach(bo, cb_oneshot);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	
	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_WFILL);
	{
		AG_ButtonAct(bo, _("OK"), 0, insert_brush,
		    "%p,%p,%p,%p,%p,%p", px, tl, tb_name, rad_types,
		    cb_oneshot, win);
		AG_ButtonAct(bo, _("Cancel"), 0, AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
flip_pixmap(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	RG_Pixmap *px = argv[2].p;
	size_t totsize = px->su->h*px->su->pitch;
	Uint8 *row, *buf;
	Uint8 *fb = px->su->pixels;
	int y;

	buf = Malloc(totsize, M_RG);
	memcpy(buf, fb, totsize);
	row = buf + totsize - px->su->pitch;
	for (y = 0; y < px->su->h; y++) {
		memcpy(fb, row, px->su->pitch);
		row -= px->su->pitch;
		fb += px->su->pitch;
	}
	Free(buf, M_RG);
}

static void
mirror_pixmap(int argc, union evarg *argv)
{
	RG_Tileview *tv = argv[1].p;
	RG_Pixmap *px = argv[2].p;
	Uint8 *row, *rowp;
	Uint8 *fb = px->su->pixels;
	int x, y;

	row = Malloc(px->su->pitch, M_RG);
	for (y = 0; y < px->su->h; y++) {
		memcpy(row, fb, px->su->pitch);
		rowp = row + px->su->pitch - px->su->format->BytesPerPixel;
		for (x = 0; x < px->su->w; x++) {
			AG_PUT_PIXEL(px->su, fb, AG_GET_PIXEL(px->su, rowp));
			fb += px->su->format->BytesPerPixel;
			rowp -= px->su->format->BytesPerPixel;
		}
	}
	Free(row, M_RG);
}

AG_Window *
RG_PixmapEdit(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Window *win;
	AG_MSpinbutton *msb;
	AG_Spinbutton *sb;
	AG_Checkbox *cb;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Pixmap %s"), px->name);
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);

	ntab = AG_NotebookAddTab(nb, _("Colors"), AG_BOX_VERT);
	{
		AG_HSVPal *pal;
		AG_FSpinbutton *fsb;
		AG_Box *hb;

		pal = AG_HSVPalNew(ntab);
		AGWIDGET(pal)->flags |= AG_WIDGET_WFILL|AG_WIDGET_HFILL;
		AG_WidgetBind(pal, "pixel-format", AG_WIDGET_POINTER,
		    &tv->ts->fmt);
		AG_WidgetBind(pal, "hue", AG_WIDGET_FLOAT, &px->h);
		AG_WidgetBind(pal, "saturation", AG_WIDGET_FLOAT, &px->s);
		AG_WidgetBind(pal, "value", AG_WIDGET_FLOAT, &px->v);
		AG_WidgetBind(pal, "alpha", AG_WIDGET_FLOAT, &px->a);
	
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		cb = AG_CheckboxNew(ntab, _("Source pixmap only"));
		AG_WidgetBind(cb, "state", AG_WIDGET_BOOL, &pixmap_source);
	}

	ntab = AG_NotebookAddTab(nb, _("Brushes"), AG_BOX_VERT);
	{
		AG_Tlist *tl;

		tl = AG_TlistNew(ntab, AG_TLIST_POLL);
		AG_TlistSetItemHeight(tl, AGTILESZ);
		AG_SetEvent(tl, "tlist-poll", poll_brushes, "%p", px);
		AG_SetEvent(tl, "tlist-dblclick", select_brush, "%p", px);
		AG_TlistSelectPtr(tl, px->curbrush);

		AG_ButtonAct(ntab, _("Create new brush"), AG_BUTTON_WFILL,
		    insert_brush_dlg, "%p,%p,%p", tv, px, win);
	}
	
	ntab = AG_NotebookAddTab(nb, _("Blending"), AG_BOX_VERT);
	{
		static const char *blend_modes[] = {
			N_("Overlay alpha"),
			N_("Average alpha"),
			N_("Destination alpha"),
			N_("Disable blending"),
			NULL
		};
		AG_Radio *rad;
		AG_Checkbox *cb;

		AG_LabelNew(ntab, AG_LABEL_STATIC, _("Blending method:"));
		rad = AG_RadioNew(ntab, blend_modes);
		AG_WidgetBind(rad, "value", AG_WIDGET_INT, &px->blend_mode);

		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		cb = AG_CheckboxNew(ntab, _("Source pixmap only"));
		AG_WidgetBind(cb, "state", AG_WIDGET_BOOL, &pixmap_source);
	}

	return (win);
}

AG_Toolbar *
RG_PixmapToolbar(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Toolbar *tbar;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1, 0);
	AG_ToolbarAddButton(tbar, 0, AGICON(STAMP_TOOL_ICON), 0, 0,
	    insert_brush_dlg, "%p,%p,%p", tv, px, AG_WidgetParentWindow(tv));
	AG_ToolbarAddButton(tbar, 0, AGICON(FLIP_TOOL_ICON), 0, 0,
	    flip_pixmap, "%p,%p", tv, px);
	AG_ToolbarAddButton(tbar, 0, AGICON(MIRROR_TOOL_ICON), 0, 0,
	    mirror_pixmap, "%p,%p", tv, px);

	return (tbar);
}

/* Create a new undo block at the current level, destroying higher blocks. */
void
RG_PixmapBeginUndoBlk(RG_Pixmap *px)
{
	struct rg_pixmap_undoblk *ublk;

	while (px->nublks > px->curblk+1) {
		ublk = &px->ublks[px->nublks-1];
		Free(ublk->mods, M_RG);
		px->nublks--;
	}

	px->ublks = Realloc(px->ublks, ++px->nublks *
	                    sizeof(struct rg_pixmap_mod));
	px->curblk++;

	ublk = &px->ublks[px->curblk];
	ublk->mods = Malloc(sizeof(struct rg_pixmap_mod), M_RG);
	ublk->nmods = 0;
}

void
RG_PixmapUndo(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	struct rg_pixmap_undoblk *ublk = &px->ublks[px->curblk];
	int i;

	if (px->curblk-1 <= 0)
		return;

	if (SDL_MUSTLOCK(tv->scaled)) { SDL_LockSurface(tv->scaled); }
	for (i = 0; i < ublk->nmods; i++) {
		struct rg_pixmap_mod *mod = &ublk->mods[i];

		RG_PutPixel(px->su, mod->x, mod->y, mod->val);
	}
	if (SDL_MUSTLOCK(tv->scaled)) { SDL_UnlockSurface(tv->scaled); }

	px->curblk--;
	tv->tile->flags |= RG_TILE_DIRTY;
}

void
RG_PixmapRedo(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	
	dprintf("redo (curblk=%d )\n", px->curblk);
}

int
RG_PixmapPutPixel(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint32 pixel, int once)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	struct rg_pixmap_undoblk *ublk = &px->ublks[px->nublks-1];
	struct rg_pixmap_mod *mod;
	Uint8 *pSrc;
	Uint8 r, g, b;
	u_int v = (pixel & px->su->format->Amask) >>
	          px->su->format->Ashift;
	Uint8 a = (v << px->su->format->Aloss) +
	          (v >> (8 - (px->su->format->Aloss << 1)));
	int i;
			
	/* Look for an existing mod to this pixel in this block. */
	for (i = ublk->nmods-1; i >= 0; i--) {
		mod = &ublk->mods[i];
		if (mod->x == x && mod->y == y)
			break;
	}
	if (i >= 0) {
		if (once)
			return (1);
	} else {
		ublk->mods = Realloc(ublk->mods, (ublk->nmods+1) *
				                  sizeof(struct rg_pixmap_mod));
		mod = &ublk->mods[ublk->nmods++];
		mod->type = RG_PIXMAP_PIXEL_REPLACE;
		mod->x = (Uint16)x;
		mod->y = (Uint16)y;
	
		SDL_LockSurface(px->su);
		pSrc = (Uint8 *)px->su->pixels + y*px->su->pitch +
		                                x*px->su->format->BytesPerPixel;
		mod->val = *(Uint32 *)pSrc;
		SDL_UnlockSurface(px->su);
	}

	/* Plot the pixel on the pixmap and update the scaled display. */
	/* XXX use background caching to avoid regen for alpha pixels */
	
	SDL_GetRGBA(pixel, px->su->format, &r, &g, &b, &a);

	switch (px->blend_mode) {
	case RG_PIXMAP_NO_BLENDING:
		RG_PutPixel(px->su, x, y, pixel);
		if (a == 255) {
			RG_TileviewScaledPixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    r, g, b);
		} else {
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case RG_PIXMAP_OVERLAY_ALPHA:
		if (a == 255) {
			RG_PutPixel(px->su, x, y, pixel);
			RG_TileviewScaledPixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    r, g, b);
		} else {
			RG_BlendRGB(px->su, x, y, RG_PRIM_OVERLAY_ALPHA,
			    r, g, b, a);
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case RG_PIXMAP_AVERAGE_ALPHA:
		RG_BlendRGB(px->su, x, y, RG_PRIM_AVERAGE_ALPHA, r, g, b, a);
		tv->tile->flags |= RG_TILE_DIRTY;
		break;
	case RG_PIXMAP_DEST_ALPHA:
		RG_BlendRGB(px->su, x, y, RG_PRIM_DST_ALPHA, r, g, b, a);
		tv->tile->flags |= RG_TILE_DIRTY;
		break;
	}
	return (0);
}

void
RG_PixmapApplyBrush(RG_Tileview *tv, RG_TileElement *tel,
    int x0, int y0, Uint32 specPx)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	struct rg_pixmap_brush *br = px->curbrush;
	SDL_Surface *brsu = br->px->su;
	Uint8 *pBrush = brsu->pixels;
	Uint8 r, g, b, specA;
	int x, y, dx, dy;
	
	if (brsu->format->Amask != 0) {
		u_int v = (specPx & brsu->format->Amask) >>
		          brsu->format->Ashift;
				
		specA = (v << brsu->format->Aloss) +
		        (v >> (8 - (brsu->format->Aloss << 1)));
	} else {
		specA = 255;
	}

	if (SDL_MUSTLOCK(brsu)) {
		SDL_LockSurface(brsu);
	}
	for (y = 0, dy = y0; y < brsu->h; y++, dy++) {
		for (x = 0, dx = x0; x < brsu->w; x++, dx++) {
			Uint32 Px, brPx;
			Uint8 brA;

			brPx = *(Uint32 *)pBrush;
			pBrush += sizeof(Uint32);
			
			if (dy < 0 || dy >= px->su->h ||
			    dx < 0 || dx >= px->su->w)
				continue;

			if (brsu->format->Amask != 0) {
				u_int v = (brPx & brsu->format->Amask) >>
				          brsu->format->Ashift;
				
				brA = (v << brsu->format->Aloss) +
				      (v >> (8 - (brsu->format->Aloss << 1)));
			} else {
				brA = 255;
			}

			switch (br->type) {
			case RG_PIXMAP_BRUSH_MONO:
				SDL_GetRGB(specPx, brsu->format, &r, &g, &b);
				break;
			case RG_PIXMAP_BRUSH_RGB:
				SDL_GetRGB(brPx, brsu->format, &r, &g, &b);
				break;
			}
			Px = SDL_MapRGBA(brsu->format, r, g, b,
			    (specA + brA)/2);

			/* TODO use a specific mod type */
			if (brA != 0)
				RG_PixmapPutPixel(tv, tel, dx, dy, Px,
				    br->flags & RG_PIXMAP_BRUSH_ONESHOT);
		}
	}
	if (SDL_MUSTLOCK(brsu))
		SDL_UnlockSurface(brsu);
}

int
RG_PixmapMouseWheel(RG_Tileview *tv, RG_TileElement *tel,
    int nwheel)
{
	Uint8 *keystate = SDL_GetKeyState(NULL);
	RG_Pixmap *px = tel->tel_pixmap.px;

	if (keystate[SDLK_h]) {
		px->h += (nwheel == 0) ? -3 : 3;
		if (px->h < 0.0) { px->h = 359.0; }
		else if (px->h > 359.0) { px->h = 0.0; }
		return (1);
	}
	if (keystate[SDLK_s]) {
		px->s += (nwheel == 0) ? 0.05 : -0.05;
		if (px->s < 0.0) {  px->s = 0.0; }
		else if (px->s > 1.0) { px->s = 1.0; }
		return (1);
	}
	if (keystate[SDLK_v]) {
		px->v += (nwheel == 0) ? -0.05 : 0.05;
		if (px->v < 0.0) { px->v = 0.0; }
		else if (px->v > 1.0) { px->v = 1.0; }
		return (1);
	}
	if (keystate[SDLK_a]) {
		px->a += (nwheel == 0) ? 0.1 : -0.1;
		if (px->a < 0.0) { px->a = 0.0; }
		else if (px->a > 1.0) { px->a = 1.0; }
		return (1);
	}
	return (0);
}

static __inline__ void
pixmap_apply(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b;
	Uint8 a = (Uint8)(px->a*255);
	Uint8 *keystate;
	int erase_mode;
	enum rg_pixmap_blend_mode bmode_save = 0;
	enum pixmap_brush_type btype_save = 0;

	keystate = SDL_GetKeyState(NULL);
	erase_mode = keystate[SDLK_e];

	if (erase_mode) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		RG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
	}

	if (erase_mode) {
		bmode_save = px->blend_mode;
		px->blend_mode = RG_PIXMAP_NO_BLENDING;
	}

	if (px->curbrush != NULL) {
		if (erase_mode) {
			btype_save = px->curbrush->type;
			px->curbrush->type = RG_PIXMAP_BRUSH_MONO;
		}

		RG_PixmapApplyBrush(tv, tel,
		    x - px->curbrush->px->xorig,
		    y - px->curbrush->px->yorig,
		    SDL_MapRGBA(px->su->format, r, g, b, a));

		if (erase_mode) {
			px->curbrush->type = btype_save;
		}
	} else {
		RG_PixmapPutPixel(tv, tel, x, y,
		    SDL_MapRGBA(px->su->format, r, g, b, a), 1);
	}
	
	if (erase_mode)
		px->blend_mode = bmode_save;
}

Uint32
RG_PixmapSourcePixel(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 *pSrc;
	Uint32 cSrc;

	if (pixmap_source) {
		SDL_LockSurface(px->su);
		pSrc = (Uint8 *)px->su->pixels +
		     y*px->su->pitch +
		     x*px->su->format->BytesPerPixel;
		cSrc = *(Uint32 *)pSrc;
		SDL_UnlockSurface(px->su);
	} else {
		SDL_LockSurface(tv->tile->su);
		pSrc = (Uint8 *)tv->tile->su->pixels +
		     (tel->tel_pixmap.y+y)*tv->tile->su->pitch +
		     (tel->tel_pixmap.x+x)*tv->tile->su->format->BytesPerPixel;
		cSrc = *(Uint32 *)pSrc;
		SDL_UnlockSurface(tv->tile->su);
	}
	return (cSrc);
}

void
RG_PixmapSourceRGBA(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 *pSrc;

	if (pixmap_source) {
		SDL_LockSurface(px->su);
		pSrc = (Uint8 *)px->su->pixels +
		     y*px->su->pitch +
		     x*px->su->format->BytesPerPixel;
		SDL_GetRGBA(*(Uint32 *)pSrc, px->su->format, r, g, b, a);
		SDL_UnlockSurface(px->su);
	} else {
		SDL_LockSurface(tv->tile->su);
		pSrc = (Uint8 *)tv->tile->su->pixels +
		     (tel->tel_pixmap.y+y)*tv->tile->su->pitch +
		     (tel->tel_pixmap.x+x)*tv->tile->su->format->BytesPerPixel;
		SDL_GetRGBA(*(Uint32 *)pSrc, tv->tile->su->format, r, g, b, a);
		SDL_UnlockSurface(tv->tile->su);
	}
}

static void
fill_ortho(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint32 cOrig, Uint32 cFill)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 *pDst;
	Uint32 cDst;

	cDst = RG_PixmapSourcePixel(tv, tel, x, y);
	if (cDst != cOrig)
		return;
	
	if (RG_PixmapPutPixel(tv, tel, x, y, cFill, 1) == 1)
		return;

	if (x-1 >= 0)		fill_ortho(tv, tel, x-1, y, cOrig, cFill);
	if (y-1 >= 0)		fill_ortho(tv, tel, x, y-1, cOrig, cFill);
	if (x+1 < px->su->w)	fill_ortho(tv, tel, x+1, y, cOrig, cFill);
	if (y+1 < px->su->h)	fill_ortho(tv, tel, x, y+1, cOrig, cFill);
}

static void
randfill_ortho(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint32 cOrig, Uint32 cFill, Uint32 *bit, Uint32 *r)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 *pDst;
	Uint32 cDst;
	int flag;

	cDst = RG_PixmapSourcePixel(tv, tel, x, y);
	if (cDst != cOrig)
		return;

	if (*bit == 0) {
		*r = arc4random();
	}
	flag = ((*r) & (2<<(*bit))) >> ((*bit)+1);
	if (++(*bit) > 30) { (*bit) = 0; }

	if (RG_PixmapPutPixel(tv, tel, x, y, flag ? cFill : cOrig, 1) == 1)
		return;

	if (x-1 >= 0)
		randfill_ortho(tv, tel, x-1, y, cOrig, cFill, bit, r);
	if (y-1 >= 0)
		randfill_ortho(tv, tel, x, y-1, cOrig, cFill, bit, r);
	if (x+1 < px->su->w)
		randfill_ortho(tv, tel, x+1, y, cOrig, cFill, bit, r);
	if (y+1 < px->su->h)
		randfill_ortho(tv, tel, x, y+1, cOrig, cFill, bit, r);
}

static void
pixmap_fill(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a = (Uint8)(px->a*255);
	Uint8 *keystate;
	Uint32 cOrig, cFill;

	cOrig = RG_PixmapSourcePixel(tv, tel, x, y);
	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_e]) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		RG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
	}
	cFill = SDL_MapRGBA(px->su->format, r, g, b, a);
	fill_ortho(tv, tel, x, y, cOrig, cFill);
	tv->tile->flags |= RG_TILE_DIRTY;
}

static void
pixmap_randfill(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a = (Uint8)(px->a*255);
	Uint8 *keystate;
	Uint32 cOrig, cFill;
	Uint32 rand;
	Uint32 bit = 0;

	cOrig = RG_PixmapSourcePixel(tv, tel, x, y);
	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_e]) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		RG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
	}
	cFill = SDL_MapRGBA(px->su->format, r, g, b, a);
	randfill_ortho(tv, tel, x, y, cOrig, cFill, &bit, &rand);
	tv->tile->flags |= RG_TILE_DIRTY;
}

static void
pixmap_pick(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a;

	RG_PixmapSourceRGBA(tv, tel, x, y, &r, &g, &b, &a);
	RG_RGB2HSV(r, g, b, &px->h, &px->s, &px->v);
	px->a = ((float)a)/255.0;
}

void
RG_PixmapMousebuttonDown(RG_Tileview *tv, RG_TileElement *tel,
    int x, int y, int button)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 *keystate;

	if (button == SDL_BUTTON_MIDDLE) {
		int x, y;

		AG_MouseGetState(&x, &y);
		RG_PixmapOpenMenu(tv, x, y);
		return;
	} else if (button == SDL_BUTTON_RIGHT) {
		tv->scrolling++;
		return;
	} else if (button != SDL_BUTTON_LEFT) {
		return;
	}

	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_f]) {
		enum rg_pixmap_blend_mode bmode_save = px->blend_mode;

		if (keystate[SDLK_r] || keystate[SDLK_e]) {
			px->blend_mode = RG_PIXMAP_NO_BLENDING;
		}
		RG_PixmapBeginUndoBlk(px);
		if (keystate[SDLK_r]) {
			pixmap_randfill(tv, tel, x, y);
		} else {
			pixmap_fill(tv, tel, x, y);
		}
		px->blend_mode = bmode_save;
	} else if (keystate[SDLK_c]) {
		pixmap_pick(tv, tel, x, y);
	} else {
		if (keystate[SDLK_h]) {
			tv->tv_pixmap.state = RG_TVPIXMAP_HORIZONTAL;
		} else if (keystate[SDLK_v]) {
			tv->tv_pixmap.state = RG_TVPIXMAP_VERTICAL;
		} else if (keystate[SDLK_d]) {
			tv->tv_pixmap.state = RG_TVPIXMAP_DIAGONAL;
		} else {
			tv->tv_pixmap.state = RG_TVPIXMAP_FREEHAND;
		}
		RG_PixmapBeginUndoBlk(px);
		pixmap_apply(tv, tel, x, y);
	}
}

void
RG_PixmapMousebuttonUp(RG_Tileview *tv, RG_TileElement *tel, int x,
    int y, int button)
{
	if (button == SDL_BUTTON_LEFT) {
		tv->tv_pixmap.state = RG_TVPIXMAP_IDLE;
		tv->tile->flags |= RG_TILE_DIRTY;
	}
}

void
RG_PixmapKeyDown(RG_Tileview *tv, RG_TileElement *tel,
    int keysym, int keymod)
{
	switch (keysym) {
	case SDLK_f:
		if (saved_cursor == NULL) {
			saved_cursor = SDL_GetCursor();
			SDL_SetCursor(agCursors[AG_FILL_CURSOR]);
		}
		break;
	case SDLK_e:
		if (saved_cursor == NULL) {
			saved_cursor = SDL_GetCursor();
			SDL_SetCursor(agCursors[AG_ERASE_CURSOR]);
		}
		break;
	case SDLK_c:
		if (saved_cursor == NULL) {
			saved_cursor = SDL_GetCursor();
			SDL_SetCursor(agCursors[AG_PICK_CURSOR]);
		}
		break;
	}
}

void
RG_PixmapKeyUp(RG_Tileview *tv, RG_TileElement *tel, int keysym,
    int keymod)
{
	if (saved_cursor != NULL) {
		SDL_SetCursor(saved_cursor);
		saved_cursor = NULL;
	}
}

void
RG_PixmapMouseMotion(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    int xrel, int yrel, int state)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	int d, x2, y2;

	switch (tv->tv_pixmap.state) {
	case RG_TVPIXMAP_FREEHAND:
		pixmap_apply(tv, tel, x, y);
		return;
	case RG_TVPIXMAP_VERTICAL:
		pixmap_apply(tv, tel, tv->tv_pixmap.xorig, y);
		return;
	case RG_TVPIXMAP_HORIZONTAL:
		pixmap_apply(tv, tel, x, tv->tv_pixmap.yorig);
		return;
	case RG_TVPIXMAP_DIAGONAL:
		if (y < tv->tv_pixmap.yorig) {
			d = tv->tv_pixmap.xorig - x;
			if (x < tv->tv_pixmap.xorig) {
				x2 = tv->tv_pixmap.xorig - d;
				y2 = tv->tv_pixmap.yorig - d;
			} else {
				x2 = tv->tv_pixmap.xorig - d;
				y2 = tv->tv_pixmap.yorig + d;
			}
		} else {
			d = tv->tv_pixmap.xorig - x;
			if (x < tv->tv_pixmap.xorig) {
				x2 = tv->tv_pixmap.xorig - d;
				y2 = tv->tv_pixmap.yorig + d;
			} else {
				x2 = tv->tv_pixmap.xorig - d;
				y2 = tv->tv_pixmap.yorig - d;
			}
		}
		pixmap_apply(tv, tel, x2, y2);
		return;
	default:
		break;
	}

	if (state == SDL_BUTTON_LEFT) {
		Uint8 *keystate = SDL_GetKeyState(NULL);

		if (keystate[SDLK_c])
			pixmap_pick(tv, tel, x, y);
	}
}

void
RG_PixmapOpenMenu(RG_Tileview *tv, int x, int y)
{
	RG_Pixmap *px = tv->tv_pixmap.px;
	AG_Menu *me;
	AG_MenuItem *mi;
	
	if (tv->tv_pixmap.menu != NULL)
		RG_PixmapCloseMenu(tv);

	me = tv->tv_pixmap.menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(me);

	mi = tv->tv_pixmap.menu_item = AG_MenuAddItem(me, NULL);
	{
		RG_TileviewGenericMenu(tv, mi);
	}
	tv->tv_pixmap.menu->sel_item = mi;
	tv->tv_pixmap.menu_win = AG_MenuExpand(me, mi, x, y);
}

void
RG_PixmapCloseMenu(RG_Tileview *tv)
{
	AG_Menu *me = tv->tv_pixmap.menu;
	AG_MenuItem *mi = tv->tv_pixmap.menu_item;

	AG_MenuCollapse(me, mi);
	AG_ObjectDestroy(me);
	Free(me, M_OBJECT);

	tv->tv_pixmap.menu = NULL;
	tv->tv_pixmap.menu_item = NULL;
	tv->tv_pixmap.menu_win = NULL;
}

