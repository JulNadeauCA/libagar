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
#include <agar/gui/surface.h>
#include <agar/gui/load_surface.h>

#include <agar/gui/checkbox.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/notebook.h>
#include <agar/gui/radio.h>
#include <agar/gui/separator.h>
#include <agar/gui/textbox.h>
#include <agar/gui/tlist.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_tileview.h>
#include <agar/map/rg_icons.h>

#include <string.h>
#include <stdlib.h>

int pixmap_source = 1;
const char *pixmap_state_names[] = {
	"",
	" (free-hand)",
	" (vertical)",
	" (horizontal)",
	" (diagonal)"
};

RG_Pixmap *
RG_PixmapNew(RG_Tileset *ts, const char *pName, int flags)
{
	char name[RG_TILE_NAME_MAX];
	int no = 0;
	RG_Pixmap *px;

tryname:
	Snprintf(name, sizeof(name), _("%s #%d"),
	    (pName != NULL) ? pName : _("Pixmap"), no++);
	if (RG_TilesetFindPixmap(ts, name) != NULL)
		goto tryname;

	px = Malloc(sizeof(RG_Pixmap));
	RG_PixmapInit(px, ts, flags);
	Strlcpy(px->name, name, sizeof(px->name));
	TAILQ_INSERT_TAIL(&ts->pixmaps, px, pixmaps);
	return (px);
}

void
RG_PixmapInit(RG_Pixmap *px, RG_Tileset *ts, int flags)
{
	px->name[0] = '\0';
	px->flags = flags;
	px->xorig = 0;
	px->yorig = 0;
	px->nRefs = 0;
	px->ts = ts;
	px->su = NULL;
	px->ublks = Malloc(sizeof(RG_PixmapUndoBlk));
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
	px->ublks[0].mods = Malloc(sizeof(RG_PixmapMod));
	px->ublks[0].nmods = 0;
}

RG_Brush *
RG_PixmapAddBrush(RG_Pixmap *px, enum rg_brush_type type,
    RG_Pixmap *bpx)
{
	RG_Brush *br;

	br = Malloc(sizeof(RG_Brush));
	br->type = type;
	br->name[0] = '\0';
	br->flags = 0;
	br->px = bpx;
	br->px->nRefs++;
	TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	return (br);
}

void
RG_PixmapDelBrush(RG_Pixmap *px, RG_Brush *br)
{
	TAILQ_REMOVE(&px->brushes, br, brushes);
	br->px->nRefs--;
	Free(br);
}

int
RG_PixmapLoad(RG_Pixmap *px, AG_DataSource *buf)
{
	Uint32 i, nbrushes;

	AG_CopyString(px->name, buf, sizeof(px->name));
	px->flags = (int)AG_ReadUint32(buf);
	px->xorig = (int)AG_ReadSint16(buf);
	px->yorig = (int)AG_ReadSint16(buf);

	if ((px->su = AG_ReadSurface(buf)) == NULL)
		return (-1);

	nbrushes = AG_ReadUint32(buf);
	for (i = 0; i < nbrushes; i++) {
		RG_Brush *br;

		br = Malloc(sizeof(RG_Brush));
		AG_CopyString(br->name, buf, sizeof(br->name));
		br->type = (enum rg_brush_type)AG_ReadUint8(buf);
		br->flags = (int)AG_ReadUint32(buf);
		(void)AG_ReadSint16(buf);		/* Pad: xorig */
		(void)AG_ReadSint16(buf);		/* Pad: yorig */
		AG_CopyString(br->px_name, buf, sizeof(br->px_name));
		br->px = NULL;
		TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	}
	return (0);
}

void
RG_PixmapSave(RG_Pixmap *px, AG_DataSource *buf)
{
	Uint32 nbrushes = 0;
	off_t nbrushes_offs;
	RG_Brush *br;

	AG_WriteString(buf, px->name);
	AG_WriteUint32(buf, (Uint32)px->flags);
	AG_WriteSint16(buf, (Sint16)px->xorig);
	AG_WriteSint16(buf, (Sint16)px->yorig);
	AG_WriteSurface(buf, px->su);

	nbrushes_offs = AG_Tell(buf);
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
	AG_WriteUint32At(buf, nbrushes, nbrushes_offs);
}

void
RG_PixmapDestroy(RG_Pixmap *px)
{
	RG_Brush *br, *nbr;
	int i;

	if (px->su != NULL)
		AG_SurfaceFree(px->su);

	for (i = 0; i < px->nublks; i++) {
		Free(px->ublks[i].mods);
	}
	Free(px->ublks);

	for (br = TAILQ_FIRST(&px->brushes);
	     br != TAILQ_END(&px->brushes);
	     br = nbr) {
		nbr = TAILQ_NEXT(br, brushes);
		Free(br);
	}
}

/* Resize a pixmap and copy the previous surface at the given offset. */
void
RG_PixmapScale(RG_Pixmap *px, int w, int h)
{
	RG_Tileset *ts = px->ts;
	AG_Surface *nsu;
	AG_Color c;

	/* Create the new surface. */
	if ((nsu = RG_SurfaceStd(ts, w, h, AG_SURFACE_ALPHA)) == NULL)
		AG_FatalError(NULL);
	
	/* Copy the old surface over. */
	AG_ColorNone(&c);
	if (px->su != NULL) {
		AG_FillRect(nsu, NULL, &c);
		AG_SurfaceBlit(px->su, NULL, nsu, 0, 0);
		AG_SurfaceFree(px->su);
	} else {
		AG_FillRect(nsu, NULL, &c);
	}
	px->su = nsu;
}

static void
PollBrushes(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Pixmap *px = AG_PTR(1);
	RG_Brush *br;
	AG_TlistItem *it;

	AG_TlistClear(tl);
	it = AG_TlistAdd(tl, NULL, _("(none)"));
	it->cat = "brush";
	it->p1 = NULL;
	TAILQ_FOREACH(br, &px->brushes, brushes) {
		it = AG_TlistAdd(tl, NULL, "%s%s %s%s",
		    (br == px->curbrush) ? "*" : "", br->name,
		    (br->flags & RG_PIXMAP_BRUSH_ONESHOT) ? _("one-shot ") : "",
		    (br->type == RG_PIXMAP_BRUSH_RGB) ? _("rgb") : _("mono"));
		it->cat = "brush";
		it->p1 = br;
		AG_TlistSetIcon(tl, it, br->px->su);
	}
	AG_TlistRestore(tl);
}

static void
SelectBrush(AG_Event *_Nonnull event)
{
	RG_Pixmap *px = AG_PTR(1);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);
	RG_Brush *brush = ti->p1;

	px->curbrush = (px->curbrush == brush) ? NULL : brush;
}

static void
PollPixmaps(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_TlistItem *it;
	RG_Pixmap *px;

	AG_TlistClear(tl);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = AG_TlistAdd(tl, NULL, "%s (%u refs)", px->name,
		    px->nRefs);
		it->cat = "pixmap";
		it->p1 = px;
		AG_TlistSetIcon(tl, it, px->su);
	}
	AG_TlistRestore(tl);
}

static void
CreateBrush(AG_Event *_Nonnull event)
{
	RG_Pixmap *px          = AG_PTR(1);
	AG_Window *win         = AG_WINDOW_PTR(2);
	AG_Tlist *tlSrc        = AGTLIST   ( AG_GetPointer(win, "tlSrc") );
	AG_Textbox *tbName     = AGTEXTBOX ( AG_GetPointer(win, "tbName") );
	AG_Radio *radMode      = AGRADIO   ( AG_GetPointer(win, "radMode") );
	AG_Checkbox *cbOneShot = AGCHECKBOX( AG_GetPointer(win, "cbOneShot") );
	RG_Brush *pbr;
	AG_TlistItem *itSrc;
	RG_Pixmap *pxSrc;
	enum rg_brush_type bMode;

	if ((itSrc = AG_TlistSelectedItem(tlSrc)) == NULL) {
		return;
	}
	pxSrc = (RG_Pixmap *)itSrc->p1;

	bMode = (enum rg_brush_type)AG_GetInt(radMode, "value");
	pbr = RG_PixmapAddBrush(px, bMode, pxSrc);

	AG_TextboxCopyString(tbName, pbr->name, sizeof(pbr->name));
	if (pbr->name[0] == '\0') {
		Strlcpy(pbr->name, pxSrc->name, sizeof(pbr->name));
	}
	if (AG_GetInt(cbOneShot, "state")) {
		pbr->flags |= RG_PIXMAP_BRUSH_ONESHOT;
	}
	AG_ObjectDetach(win);
}

static void
UpdateBrushOptions(AG_Event *_Nonnull event)
{
	AG_Textbox *tbName = AG_TEXTBOX_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	RG_Pixmap *spx;

	if (it != NULL) {
		spx = it->p1;
		AG_TextboxSetString(tbName , spx->name);
	}
}

static void
CreateBrushDlg(AG_Event *_Nonnull event)
{
	RG_Tileview *tv = RG_TILEVIEW_PTR(1);
	RG_Pixmap *px = AG_PTR(2);
	AG_Window *win, *pwin = AG_WINDOW_PTR(3);
	AG_Tlist *tlSrc;
	AG_Box *bo;
	AG_Textbox *tbName;
	AG_Radio *rad;
	AG_Checkbox *cbOneShot;
	static const char *types[] = {
		N_("Monochromatic"),
		N_("Source RGB"),
		NULL
	};

	if ((win = AG_WindowNew(AG_WINDOW_NOCLOSE | AG_WINDOW_NOMINIMIZE))
	    == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("New %s brush"), px->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	tbName = AG_TextboxNew(NULL, AG_TEXTBOX_HFILL, _("Name: "));
	AG_SetPointer(win, "tbName", tbName);
	AG_WidgetFocus(tbName);

	cbOneShot = AG_CheckboxNew(NULL, 0, _("One-shot"));
	AG_SetPointer(win, "cbOneShot", cbOneShot);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_EXPAND);
	AG_SetStyle(bo, "padding", "0");
	AG_SetStyle(bo, "spacing", "0");
	{
		AG_LabelNewS(bo, 0, _("Source pixmap:"));

		tlSrc = AG_TlistNew(bo, AG_TLIST_POLL | AG_TLIST_EXPAND);
		AG_TlistSetItemHeight(tlSrc, RG_TILESZ);
		AG_TlistSizeHint(tlSrc, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_SetEvent(tlSrc, "tlist-poll", PollPixmaps, "%p", tv->ts);
		AG_SetEvent(tlSrc, "tlist-selected", UpdateBrushOptions, "%p", tbName);
		AG_SetPointer(win, "tlSrc", tlSrc);
	}
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNewS(bo, 0, _("Pixel mode:"));
		rad = AG_RadioNew(bo, AG_RADIO_EXPAND, types);
		AG_SetInt(rad, "value", 0);
		AG_SetPointer(win, "radMode", rad);
		AG_ObjectAttach(bo, tbName);
		AG_ObjectAttach(bo, cbOneShot);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	
	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"), CreateBrush, "%p,%p", px, win);
		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

static void
FlipPixmap(AG_Event *_Nonnull event)
{
	RG_Pixmap *px = AG_PTR(1);
	AG_Size totsize = px->su->h * px->su->pitch;
	Uint8 *row, *buf;
	Uint8 *fb = px->su->pixels;
	int y;

	buf = Malloc(totsize);
	memcpy(buf, fb, totsize);
	row = buf + totsize - px->su->pitch;
	for (y = 0; y < px->su->h; y++) {
		memcpy(fb, row, px->su->pitch);
		row -= px->su->pitch;
		fb += px->su->pitch;
	}
	Free(buf);
}

static void
MirrorPixmap(AG_Event *_Nonnull event)
{
	RG_Pixmap *px = AG_PTR(1);
	Uint8 *row, *rowp;
	AG_Surface *S = px->su;
	Uint8 *fb = S->pixels;
	int x, y, Bpp = S->format.BytesPerPixel;

	row = Malloc(S->pitch);
	for (y = 0; y < S->h; y++) {
		memcpy(row, fb, S->pitch);
		rowp = row + S->pitch - Bpp;
		for (x = 0; x < S->w; x++) {
			AG_SurfacePut32_At(S, fb,
			    AG_SurfaceGet32_At(S, rowp));
			fb += Bpp;
			rowp -= Bpp;
		}
	}
	free(row);
}

AG_Window *
RG_PixmapEdit(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_HSVPal *pal;
	AG_Tlist *tl;

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Pixmap %s"), px->name);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	ntab = AG_NotebookAdd(nb, _("Colors"), AG_BOX_VERT);
	{
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		AG_BindPointer(pal, "pixel-format", (void *)&tv->ts->fmt);
		AG_BindFloat(pal, "hue", &px->h);
		AG_BindFloat(pal, "saturation", &px->s);
		AG_BindFloat(pal, "value", &px->v);
		AG_BindFloat(pal, "alpha", &px->a);
	
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);

		AG_CheckboxNewInt(ntab, 0, _("Source pixmap only"),
		    &pixmap_source);
	}
	ntab = AG_NotebookAdd(nb, _("Brushes"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_TlistSetItemHeight(tl, RG_TILESZ);
		AG_SetEvent(tl, "tlist-poll", PollBrushes, "%p", px);
		AG_SetEvent(tl, "tlist-dblclick", SelectBrush, "%p", px);
		AG_TlistSelectPtr(tl, px->curbrush);

		AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Create new brush"),
		    CreateBrushDlg, "%p,%p,%p", tv, px, win);
	}
	ntab = AG_NotebookAdd(nb, _("Blending"), AG_BOX_VERT);
	{
		static const char *blend_modes[] = {
			N_("Overlay alpha"),
			N_("Average alpha"),
			N_("Destination alpha"),
			N_("Disable blending"),
			NULL
		};

		AG_LabelNewS(ntab, 0, _("Blending method:"));
		AG_RadioNewUint(ntab, 0, blend_modes, (void *)&px->blend_mode);
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
		AG_CheckboxNewInt(ntab, 0, _("Source pixmap only"),
		    &pixmap_source);
	}
	
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MIDDLE_LEFT, 360, 500);
	return (win);
}

AG_Toolbar *
RG_PixmapToolbar(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Toolbar *tbar;

	tbar = AG_ToolbarNew(tv->tel_box, AG_TOOLBAR_VERT, 1, 0);
	AG_ToolbarButtonIcon(tbar, rgIconStamp.s, 0,
	    CreateBrushDlg, "%p,%p,%p", tv, px, AG_ParentWindow(tv));
	AG_ToolbarButtonIcon(tbar, rgIconFlip.s, 0, FlipPixmap, "%p", px);
	AG_ToolbarButtonIcon(tbar, rgIconMirror.s, 0, MirrorPixmap, "%p,%p", px);

	return (tbar);
}

/* Create a new undo block at the current level, destroying higher blocks. */
void
RG_PixmapBeginUndoBlk(RG_Pixmap *px)
{
	RG_PixmapUndoBlk *ublk;

	while (px->nublks > px->curblk+1) {
		ublk = &px->ublks[px->nublks-1];
		Free(ublk->mods);
		px->nublks--;
	}

	px->ublks = Realloc(px->ublks, ++px->nublks*sizeof(RG_PixmapUndoBlk));
	px->curblk++;

	ublk = &px->ublks[px->curblk];
	ublk->mods = Malloc(sizeof(RG_PixmapMod));
	ublk->nmods = 0;
}

void
RG_PixmapUndo(RG_Tileview *tv, RG_TileElement *tel)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	RG_PixmapUndoBlk *ublk = &px->ublks[px->curblk];
	int i;

	if (px->curblk-1 <= 0)
		return;

	for (i = 0; i < ublk->nmods; i++) {
		RG_PixmapMod *mod = &ublk->mods[i];
		RG_PutPixel(px->su, mod->x, mod->y, mod->val);
	}

	px->curblk--;
	tv->tile->flags |= RG_TILE_DIRTY;
}

void
RG_PixmapRedo(RG_Tileview *tv, RG_TileElement *tel)
{
#if 0
	RG_Pixmap *px = tel->tel_pixmap.px;
	printf("redo (curblk=%d )\n", px->curblk);
#endif
}

int
RG_PixmapPutPixel(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint32 pixel, int once)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	RG_PixmapUndoBlk *ublk = &px->ublks[px->nublks-1];
	RG_PixmapMod *mod;
	AG_Surface *S = px->su;
	Uint8 *pSrc;
	Uint v = (pixel & S->format.Amask) >> S->format.Ashift;
	Uint8 a = (v << S->format.Aloss) +
	          (v >> (8 - (S->format.Aloss << 1)));
	int i;
	AG_Color c;

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
				                  sizeof(RG_PixmapMod));
		mod = &ublk->mods[ublk->nmods++];
		mod->type = RG_PIXMAP_PIXEL_REPLACE;
		mod->x = (Uint16)x;
		mod->y = (Uint16)y;
	
		pSrc = (Uint8 *)S->pixels + y*S->pitch +
		                            x*S->format.BytesPerPixel;
		mod->val = *(Uint32 *)pSrc;
	}

	/* Plot the pixel on the pixmap and update the scaled display. */
	/* XXX use background caching to avoid regen for alpha pixels */
	
	AG_GetColor32(&c, pixel, &S->format);

	switch (px->blend_mode) {
	case RG_PIXMAP_NO_BLENDING:
		RG_PutPixel(S, x, y, pixel);
		if (a == 255) {
			RG_TileviewPixelCached(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    c.r, c.g, c.b);
		} else {
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case RG_PIXMAP_OVERLAY_ALPHA:
		if (a == 255) {
			RG_PutPixel(S, x, y, pixel);
			RG_TileviewPixelCached(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    c.r, c.g, c.b);
		} else {
			RG_BlendRGB(S, x, y, RG_PRIM_OVERLAY_ALPHA, &c);
			tv->tile->flags |= RG_TILE_DIRTY;
		}
		break;
	case RG_PIXMAP_AVERAGE_ALPHA:
		RG_BlendRGB(S, x, y, RG_PRIM_AVERAGE_ALPHA, &c);
		tv->tile->flags |= RG_TILE_DIRTY;
		break;
	case RG_PIXMAP_DEST_ALPHA:
		RG_BlendRGB(S, x, y, RG_PRIM_DST_ALPHA, &c);
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
	RG_Brush *br = px->curbrush;
	AG_Surface *brsu = br->px->su;
	AG_PixelFormat *fmt = &brsu->format;
	Uint8 *pBrush = brsu->pixels;
	Uint8 specA;
	int x, y, dx, dy;
	AG_Color c;
	
	if (fmt->Amask != 0) {
		Uint v = (specPx & fmt->Amask) >> fmt->Ashift;
		specA = (v << fmt->Aloss) + (v >> (8 - (fmt->Aloss << 1)));
	} else {
		specA = 255;
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

			if (fmt->Amask != 0) {
				Uint v = (brPx & fmt->Amask) >> fmt->Ashift;
				brA = (v << fmt->Aloss) + (v >> (8 - (fmt->Aloss << 1)));
			} else {
				brA = 255;
			}
			switch (br->type) {
			case RG_PIXMAP_BRUSH_MONO:
				AG_GetColor32(&c, specPx, fmt);
				break;
			case RG_PIXMAP_BRUSH_RGB:
			default:
				AG_GetColor32(&c, brPx, fmt);
				break;
			}
			c.a = (specA + brA) >> 1;
			Px = AG_MapPixel32(fmt, &c);

			/* TODO use a specific mod type */
			if (brA != 0)
				RG_PixmapPutPixel(tv, tel, dx, dy, Px,
				    br->flags & RG_PIXMAP_BRUSH_ONESHOT);
		}
	}
}

int
RG_PixmapWheel(RG_Tileview *tv, RG_TileElement *tel, int nwheel)
{
	int *kbd = AG_GetKeyState(tv);
	RG_Pixmap *px = tel->tel_pixmap.px;

	if (kbd[AG_KEY_H]) {
		px->h += (nwheel == 0) ? -3 : 3;
		if (px->h < 0.0) { px->h = 359.0; }
		else if (px->h > 359.0) { px->h = 0.0; }
		return (1);
	}
	if (kbd[AG_KEY_S]) {
		px->s += (nwheel == 0) ? 0.05 : -0.05;
		if (px->s < 0.0) {  px->s = 0.0; }
		else if (px->s > 1.0) { px->s = 1.0; }
		return (1);
	}
	if (kbd[AG_KEY_V]) {
		px->v += (nwheel == 0) ? -0.05 : 0.05;
		if (px->v < 0.0) { px->v = 0.0; }
		else if (px->v > 1.0) { px->v = 1.0; }
		return (1);
	}
	if (kbd[AG_KEY_A]) {
		px->a += (nwheel == 0) ? 0.1 : -0.1;
		if (px->a < 0.0) { px->a = 0.0; }
		else if (px->a > 1.0) { px->a = 1.0; }
		return (1);
	}
	return (0);
}

static void
pixmap_apply(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel,
    int x, int y)
{
	int *kbd = AG_GetKeyState(tv);
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b;
	Uint8 a = (Uint8)(px->a*255);
	int erase_mode;
	enum rg_pixmap_blend_mode bmode_save = 0;
	enum rg_brush_type btype_save = 0;

	erase_mode = kbd[AG_KEY_E];
	if (erase_mode) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		AG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
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
		    AG_MapPixel32_RGBA8(&px->su->format, r,g,b,a));

		if (erase_mode) {
			px->curbrush->type = btype_save;
		}
	} else {
		RG_PixmapPutPixel(tv, tel, x,y,
		    AG_MapPixel32_RGBA8(&px->su->format, r,g,b,a), 1);
	}
	
	if (erase_mode)
		px->blend_mode = bmode_save;
}

Uint32
RG_PixmapSourcePixel(RG_Tileview *tv, RG_TileElement *tel, int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Surface *S;
	Uint8 *pSrc;
	Uint32 cSrc;

	if (pixmap_source) {
		S = px->su;
		pSrc = S->pixels + y*S->pitch + x*S->format.BytesPerPixel;
		cSrc = *(Uint32 *)pSrc;
	} else {
		S = tv->tile->su;
		pSrc = S->pixels +
		     (tel->tel_pixmap.y + y)*S->pitch +
		     (tel->tel_pixmap.x + x)*S->format.BytesPerPixel;
		cSrc = *(Uint32 *)pSrc;
	}
	return (cSrc);
}

void
RG_PixmapSourceRGBA(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	AG_Surface *S;
	Uint8 *pSrc;

	if (pixmap_source) {
		S = px->su;
		pSrc = S->pixels + y*S->pitch + x*S->format.BytesPerPixel;
		AG_GetColor32_RGBA8(*(Uint32 *)pSrc, &S->format, r,g,b,a);
	} else {
		S = tv->tile->su;
		pSrc = S->pixels +
		     (tel->tel_pixmap.y+y)*S->pitch +
		     (tel->tel_pixmap.x+x)*S->format.BytesPerPixel;
		AG_GetColor32_RGBA8(*(Uint32 *)pSrc, &S->format, r,g,b,a);
	}
}

static void
fill_ortho(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel, int x, int y,
    Uint32 cOrig, Uint32 cFill)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
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
randfill_ortho(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel,
    int x, int y, Uint32 cOrig, Uint32 cFill, Uint32 *_Nonnull bit,
    Uint32 *_Nonnull r)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint32 cDst;
	int flag;

	cDst = RG_PixmapSourcePixel(tv, tel, x, y);
	if (cDst != cOrig)
		return;

	if (*bit == 0) {
#if 0
		*r = arc4random(); /* XXX portability issues */
#else
		*r = 0x11223344;
#endif
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
pixmap_fill(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel,
    int x, int y)
{
	int *kbd = AG_GetKeyState(tv);
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a = (Uint8)(px->a*255);
	Uint32 cOrig, cFill;

	cOrig = RG_PixmapSourcePixel(tv, tel, x, y);
	if (kbd[AG_KEY_E]) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		AG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
	}
	cFill = AG_MapPixel32_RGBA8(&px->su->format, r,g,b,a);
	fill_ortho(tv, tel, x, y, cOrig, cFill);
	tv->tile->flags |= RG_TILE_DIRTY;
}

static void
pixmap_randfill(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel,
    int x, int y)
{
	int *kbd = AG_GetKeyState(tv);
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a = (Uint8)(px->a*255);
	Uint32 cOrig, cFill;
	Uint32 rand;
	Uint32 bit = 0;

	cOrig = RG_PixmapSourcePixel(tv, tel, x, y);
	if (kbd[AG_KEY_E]) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		AG_HSV2RGB(px->h, px->s, px->v, &r, &g, &b);
	}
	cFill = AG_MapPixel32_RGBA8(&px->su->format, r,g,b,a);
	randfill_ortho(tv, tel, x, y, cOrig, cFill, &bit, &rand);
	tv->tile->flags |= RG_TILE_DIRTY;
}

static void
pixmap_pick(RG_Tileview *_Nonnull tv, RG_TileElement *_Nonnull tel,
    int x, int y)
{
	RG_Pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a;

	RG_PixmapSourceRGBA(tv, tel, x, y, &r, &g, &b, &a);
	AG_RGB2HSV(r, g, b, &px->h, &px->s, &px->v);
	px->a = ((float)a)/255.0;
}

void
RG_PixmapKeydown(RG_Tileview *tv, int ksym)
{
}

void
RG_PixmapKeyup(RG_Tileview *tv)
{
}

void
RG_PixmapButtondown(RG_Tileview *tv, RG_TileElement *tel,
    int x, int y, int xMouse, int yMouse, int button)
{
	int *kbd = AG_GetKeyState(tv);
	RG_Pixmap *px = tel->tel_pixmap.px;

	if (button == AG_MOUSE_MIDDLE) {
		RG_PixmapOpenMenu(tv, xMouse+5, yMouse+5);
		return;
	} else if (button == AG_MOUSE_RIGHT) {
		tv->scrolling++;
		return;
	} else if (button != AG_MOUSE_LEFT) {
		return;
	}

	if (kbd[AG_KEY_F]) {
		enum rg_pixmap_blend_mode bmode_save = px->blend_mode;

		if (kbd[AG_KEY_R] || kbd[AG_KEY_E]) {
			px->blend_mode = RG_PIXMAP_NO_BLENDING;
		}
		RG_PixmapBeginUndoBlk(px);
		if (kbd[AG_KEY_R]) {
			pixmap_randfill(tv, tel, x, y);
		} else {
			pixmap_fill(tv, tel, x, y);
		}
		px->blend_mode = bmode_save;
	} else if (kbd[AG_KEY_C]) {
		pixmap_pick(tv, tel, x, y);
	} else {
		if (kbd[AG_KEY_H]) {
			tv->tv_pixmap.state = RG_TVPIXMAP_HORIZONTAL;
		} else if (kbd[AG_KEY_V]) {
			tv->tv_pixmap.state = RG_TVPIXMAP_VERTICAL;
		} else {
			tv->tv_pixmap.state = RG_TVPIXMAP_FREEHAND;
		}
		RG_PixmapBeginUndoBlk(px);
		pixmap_apply(tv, tel, x, y);
	}
}

void
RG_PixmapButtonup(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    int xMouse, int yMouse, int button)
{
	if (button == AG_MOUSE_LEFT) {
		tv->tv_pixmap.state = RG_TVPIXMAP_IDLE;
		tv->tile->flags |= RG_TILE_DIRTY;
	}
}

void
RG_PixmapMotion(RG_Tileview *tv, RG_TileElement *tel, int x, int y,
    int xrel, int yrel, int state)
{
	int *kbd = AG_GetKeyState(tv);

#if 0
	TODO: Set AG_FILL_CURSOR, AG_ERASE_CURSOR, AG_PICK_CURSOR
#endif
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
	default:
		break;
	}

	if (state == AG_MOUSE_LEFT) {
		if (kbd[AG_KEY_C])
			pixmap_pick(tv, tel, x, y);
	}
}

void
RG_PixmapOpenMenu(RG_Tileview *tv, int x, int y)
{
	if (tv->tv_pixmap.menu != NULL) {
		AG_PopupShowAt(tv->tv_pixmap.menu, x,y);
		return;
	}
	tv->tv_pixmap.menu = AG_PopupNew(tv);
	RG_TileviewGenericMenu(tv, tv->tv_pixmap.menu->root);
	AG_PopupShowAt(tv->tv_pixmap.menu, x,y);
}

void
RG_PixmapCloseMenu(RG_Tileview *tv)
{
	if (tv->tv_pixmap.menu != NULL) {
		AG_PopupDestroy(tv->tv_pixmap.menu);
		tv->tv_pixmap.menu = NULL;
	}
}

