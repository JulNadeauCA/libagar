/*	$Csoft: pixmap.c,v 1.14 2005/02/27 03:13:57 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/view.h>

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

#include "tileset.h"
#include "tileview.h"

static SDL_Cursor *saved_cursor = NULL;

void
pixmap_init(struct pixmap *px, struct tileset *ts, int flags)
{
	px->name[0] = '\0';
	px->ts = ts;
	px->flags = flags;
	px->xorig = 0;
	px->yorig = 0;
	px->su = NULL;
	px->nrefs = 0;
	px->ublks = Malloc(sizeof(struct pixmap_undoblk), M_RG);
	px->nublks = 1;
	px->curblk = 0;

	px->h = 0.0;
	px->s = 1.0;
	px->v = 1.0;
	px->a = 1.0;
	px->curbrush = NULL;
	px->blend_mode = PIXMAP_SPECIFIC_ALPHA;
	TAILQ_INIT(&px->brushes);

	pixmap_begin_undoblk(px);
	px->ublks[0].umods = Malloc(sizeof(struct pixmap_umod), M_RG);
	px->ublks[0].numods = 0;
}

struct pixmap_brush *
pixmap_insert_brush(struct pixmap *px, enum pixmap_brush_type type,
    struct pixmap *bpx)
{
	struct pixmap_brush *br;

	br = Malloc(sizeof(struct pixmap_brush), M_RG);
	br->type = type;
	br->name[0] = '\0';
	br->flags = 0;
	br->xorig = bpx->su->w/2;
	br->yorig = bpx->su->h/2;
	br->px = bpx;
	br->px->nrefs++;
	TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	return (br);
}

void
pixmap_remove_brush(struct pixmap *px, struct pixmap_brush *br)
{
	TAILQ_REMOVE(&px->brushes, br, brushes);
	br->px->nrefs--;
	Free(br, M_RG);
}

int
pixmap_load(struct pixmap *px, struct netbuf *buf)
{
	Uint32 i, nbrushes;

	copy_string(px->name, buf, sizeof(px->name));
	px->flags = (int)read_uint32(buf);
	px->xorig = (int)read_sint16(buf);
	px->yorig = (int)read_sint16(buf);
	if ((px->su = read_surface(buf, vfmt)) == NULL)
		return (-1);

	nbrushes = read_uint32(buf);
	for (i = 0; i < nbrushes; i++) {
		struct pixmap_brush *br;

		br = Malloc(sizeof(struct pixmap_brush), M_RG);
		copy_string(br->name, buf, sizeof(br->name));
		br->type = (enum pixmap_brush_type)read_uint8(buf);
		br->flags = (int)read_uint32(buf);
		br->xorig = (int)read_sint16(buf);
		br->yorig = (int)read_sint16(buf);
		copy_string(br->px_name, buf, sizeof(br->px_name));
		br->px = NULL;
		TAILQ_INSERT_TAIL(&px->brushes, br, brushes);
	}
	return (0);
}

void
pixmap_save(struct pixmap *px, struct netbuf *buf)
{
	Uint32 nbrushes = 0;
	off_t nbrushes_offs;
	struct pixmap_brush *br;

	write_string(buf, px->name);
	write_uint32(buf, (Uint32)px->flags);
	write_sint16(buf, (Sint16)px->xorig);
	write_sint16(buf, (Sint16)px->yorig);
	write_surface(buf, px->su);

	nbrushes_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(br, &px->brushes, brushes) {
		write_string(buf, br->name);
		write_uint8(buf, (Uint8)br->type);
		write_uint32(buf, (Uint32)br->flags);
		write_sint16(buf, (Sint16)br->xorig);
		write_sint16(buf, (Sint16)br->yorig);
		write_string(buf, br->px->name);
		nbrushes++;
	}
	pwrite_uint32(buf, nbrushes, nbrushes_offs);
}

void
pixmap_destroy(struct pixmap *px)
{
	struct pixmap_brush *br, *nbr;
	int i;

#ifdef DEBUG
	if (px->nrefs > 0)
		dprintf("%s has %d references\n", px->name, px->nrefs);
#endif
	if (px->su != NULL)
		SDL_FreeSurface(px->su);

	for (i = 0; i < px->nublks; i++) {
		Free(px->ublks[i].umods, M_RG);
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
pixmap_scale(struct pixmap *px, int w, int h, int xoffs, int yoffs)
{
	struct tileset *ts = px->ts;
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
	struct tileview *tv = argv[1].p;
	
	tv->tile->flags |= TILE_DIRTY;
}

static void
update_brushes(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct pixmap *px = argv[1].p;
	struct pixmap_brush *br;
	struct tlist_item *it;

	tlist_clear_items(tl);
	TAILQ_FOREACH(br, &px->brushes, brushes) {
		it = tlist_insert(tl, br->px->su, "%s%s %s",
		    (br == px->curbrush) ? "(*) " : "",
		    br->name,
		    (br->type == PIXMAP_BRUSH_MONO) ? "(mono)" :
		    (br->type == PIXMAP_BRUSH_RGB) ? "(rgb)" : "");
		it->class = "brush";
		it->p1 = br;
	}
	tlist_restore_selections(tl);
}

static void
select_brush(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct pixmap *px = argv[1].p;
	struct pixmap_brush *br;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl)) != NULL)
		px->curbrush = (px->curbrush == it->p1) ? NULL : it->p1;
}

static void
poll_pixmaps(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tlist_item *it;
	struct pixmap *px;

	tlist_clear_items(tl);
	TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
		it = tlist_insert(tl, px->su, "%s (%u refs)",
		    px->name, px->nrefs);
		it->class = "pixmap";
		it->p1 = px;
	}
	tlist_restore_selections(tl);
}

static void
insert_brush(int argc, union evarg *argv)
{
	struct pixmap *px = argv[1].p;
	struct tlist *tl = argv[2].p;
	struct textbox *tb = argv[3].p;
	struct radio *rad_types = argv[4].p;
	struct mspinbutton *msb_origin = argv[5].p;
	struct checkbox *cb_oneshot = argv[6].p;
	struct window *dlg_win = argv[7].p;
	enum pixmap_brush_type btype;
	struct pixmap *spx;
	struct pixmap_brush *pbr;
	struct tlist_item *it;

	if ((it = tlist_item_selected(tl)) == NULL) {
		return;
	}
	spx = it->p1;

	btype = (enum pixmap_brush_type)widget_get_int(rad_types, "value");
	pbr = pixmap_insert_brush(px, btype, spx);
	textbox_copy_string(tb, pbr->name, sizeof(pbr->name));
	if (pbr->name[0] == '\0') {
		strlcpy(pbr->name, spx->name, sizeof(pbr->name));
	}
	pbr->xorig = widget_get_int(msb_origin, "xvalue");
	pbr->yorig = widget_get_int(msb_origin, "yvalue");
	if (widget_get_int(cb_oneshot, "state"))
		pbr->flags |= PIXMAP_BRUSH_ONESHOT;

	view_detach(dlg_win);
}

static void
update_bropts(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct textbox *tb_name = argv[1].p;
	struct mspinbutton *msb_origin = argv[2].p;
	struct tlist_item *it;
	struct pixmap *spx;

	if ((it = tlist_item_selected(tl)) == NULL) {
		return;
	}
	spx = it->p1;
	textbox_printf(tb_name, "%s", spx->name);
	mspinbutton_set_value(msb_origin, "xvalue", spx->su->w/2);
	mspinbutton_set_value(msb_origin, "yvalue", spx->su->h/2);
}

static void
insert_brush_dlg(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct pixmap *px = argv[2].p;
	struct window *win, *pwin = argv[3].p;
	struct tlist *tl;
	struct box *bo;
	struct textbox *tb_name;
	struct button *bu;
	struct radio *rad_types;
	struct mspinbutton *msb_origin;
	struct checkbox *cb_oneshot;
	static const char *types[] = {
		_("Monochromatic"),
		_("Source RGB"),
		NULL
	};

	win = window_new(WINDOW_NO_CLOSE|WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("New %s brush"), px->name);
	window_set_position(win, WINDOW_CENTER, 1);

	tb_name = Malloc(sizeof(struct textbox), M_OBJECT);
	textbox_init(tb_name, _("Name: "));
	
	msb_origin = Malloc(sizeof(struct mspinbutton), M_OBJECT);
	mspinbutton_init(msb_origin, ",", _("Origin: "));
	mspinbutton_set_range(msb_origin, 0, TILE_SIZE_MAX-1);

	cb_oneshot = Malloc(sizeof(struct checkbox), M_OBJECT);
	checkbox_init(cb_oneshot, _("One-shot"));

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	box_set_padding(bo, 0);
	box_set_spacing(bo, 0);
	{
		label_new(bo, LABEL_STATIC, _("Source pixmap:"));

		tl = tlist_new(bo, TLIST_POLL);
		tlist_set_item_height(tl, TILESZ);
		tlist_prescale(tl, "XXXXXXXXXXXXXXXXXXX", 5);
		event_new(tl, "tlist-poll", poll_pixmaps, "%p", tv->ts);
		event_new(tl, "tlist-selected", update_bropts, "%p,%p",
		    tb_name, msb_origin);
		widget_focus(tl);
	}
	
	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		rad_types = radio_new(bo, types);
		widget_set_int(rad_types, "value", 0);
		object_attach(bo, tb_name);
		object_attach(bo, msb_origin);
		object_attach(bo, cb_oneshot);
	}

	separator_new(win, SEPARATOR_HORIZ);
	
	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	{
		bu = button_new(bo, _("OK"));
		event_new(bu, "button-pushed", insert_brush,
		    "%p,%p,%p,%p,%p,%p,%p", px, tl, tb_name, rad_types,
		    msb_origin, cb_oneshot, win);
	
		bu = button_new(bo, _("Cancel"));
		event_new(bu, "button-pushed", window_generic_detach, "%p",
		    win);
	}

	window_attach(pwin, win);
	window_show(win);
}

struct window *
pixmap_edit(struct tileview *tv, struct tile_element *tel)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct window *win;
	struct mspinbutton *msb;
	struct spinbutton *sb;
	struct checkbox *cb;
	struct box *bo;

	win = window_new(0, NULL);
	window_set_caption(win, _("Pixmap %s"), px->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
#if 0
	msb = mspinbutton_new(win, ",", _("Coordinates: "));
	widget_bind(msb, "xvalue", WIDGET_INT, &tel->tel_pixmap.x);
	widget_bind(msb, "yvalue", WIDGET_INT, &tel->tel_pixmap.y);
	mspinbutton_set_range(msb, 0, TILE_SIZE_MAX-1);
	event_new(msb, "mspinbutton-changed", update_tv, "%p", tv);
#endif

#if 0
	sb = spinbutton_new(win, _("Transparency: "));
	widget_bind(sb, "value", WIDGET_INT, &tel->tel_pixmap.alpha);
	spinbutton_set_range(sb, 0, 255);
	event_new(sb, "spinbutton-changed", update_tv, "%p", tv);
#endif

#if 0
	label_new(win, LABEL_POLLED, "Undo level: %u/%u", &px->curblk,
	    &px->nublks);
#endif

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		struct hsvpal *pal;
		struct fspinbutton *fsb;
		struct box *hb;

		pal = hsvpal_new(bo, px->su->format);
		WIDGET(pal)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		widget_bind(pal, "hue", WIDGET_FLOAT, &px->h);
		widget_bind(pal, "saturation", WIDGET_FLOAT, &px->s);
		widget_bind(pal, "value", WIDGET_FLOAT, &px->v);
		widget_bind(pal, "alpha", WIDGET_FLOAT, &px->a);
	
		hb = box_new(bo, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
		box_set_padding(hb, 1);
		{
			fsb = fspinbutton_new(hb, NULL, _("H: "));
			fspinbutton_prescale(fsb, "000");
			widget_bind(fsb, "value", WIDGET_FLOAT, &px->h);
			fspinbutton_set_range(fsb, 0.0, 359.0);
			fspinbutton_set_increment(fsb, 1);
			fspinbutton_set_precision(fsb, "f", 0);
		
			fsb = fspinbutton_new(hb, NULL, _("S: "));
			fspinbutton_prescale(fsb, "00.00");
			widget_bind(fsb, "value", WIDGET_FLOAT, &px->s);
			fspinbutton_set_range(fsb, 0.0, 1.0);
			fspinbutton_set_increment(fsb, 0.01);
			fspinbutton_set_precision(fsb, "f", 2);
		}
		
		hb = box_new(bo, BOX_HORIZ, BOX_WFILL|BOX_HOMOGENOUS);
		box_set_padding(hb, 1);
		{
			fsb = fspinbutton_new(hb, NULL, _("V: "));
			fspinbutton_prescale(fsb, "00.00");
			widget_bind(fsb, "value", WIDGET_FLOAT, &px->v);
			fspinbutton_set_range(fsb, 0.0, 1.0);
			fspinbutton_set_increment(fsb, 0.01);
			fspinbutton_set_precision(fsb, "f", 2);
			
			fsb = fspinbutton_new(hb, NULL, _("A: "));
			fspinbutton_prescale(fsb, "0.000");
			widget_bind(fsb, "value", WIDGET_FLOAT, &px->a);
			fspinbutton_set_range(fsb, 0.0, 1.0);
			fspinbutton_set_increment(fsb, 0.005);
			fspinbutton_set_precision(fsb, "f", 3);
		}
	}
		
	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		static const char *blend_modes[] = {
			N_("Specific alpha only"),
			N_("Specific+dest alphas"),
			N_("Brush alpha only"),
			N_("Brush+dest alphas"),
			N_("Destination alpha only"),
			N_("Disable blending"),
			NULL
		};
		struct radio *rad;

		label_new(bo, LABEL_STATIC, _("Blending method:"));
		rad = radio_new(bo, blend_modes);
		widget_bind(rad, "value", WIDGET_INT, &px->blend_mode);
	}

	separator_new(win, SEPARATOR_HORIZ);

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	{
		struct tlist *tl;
		struct button *bu;

		label_new(bo, LABEL_STATIC, _("Brushes:"));
		tl = tlist_new(bo, TLIST_POLL);
		event_new(tl, "tlist-poll", update_brushes, "%p", px);
		event_new(tl, "tlist-dblclick", select_brush, "%p", px);

		bu = button_new(bo, _("Insert brush"));
		WIDGET(bu)->flags |= WIDGET_WFILL;
		event_new(bu, "button-pushed", insert_brush_dlg,
		    "%p,%p,%p", tv, px, win);
	}
	return (win);
}

/* Create a new undo block at the current level, destroying higher blocks. */
void
pixmap_begin_undoblk(struct pixmap *px)
{
	struct pixmap_undoblk *ublk;

	while (px->nublks > px->curblk+1) {
		ublk = &px->ublks[px->nublks-1];
		Free(ublk->umods, M_RG);
		px->nublks--;
	}

	px->ublks = Realloc(px->ublks, ++px->nublks *
	                    sizeof(struct pixmap_umod));
	px->curblk++;

	ublk = &px->ublks[px->curblk];
	ublk->umods = Malloc(sizeof(struct pixmap_umod), M_RG);
	ublk->numods = 0;
}

void
pixmap_undo(struct tileview *tv, struct tile_element *tel)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct pixmap_undoblk *ublk = &px->ublks[px->curblk];
	int i;

	if (px->curblk-1 <= 0)
		return;

	if (SDL_MUSTLOCK(tv->scaled)) { SDL_LockSurface(tv->scaled); }
	for (i = 0; i < ublk->numods; i++) {
		struct pixmap_umod *umod = &ublk->umods[i];

		prim_put_pixel(px->su, umod->x, umod->y, umod->val);
#if 0
		tileview_scaled_pixel(tv,
		    tel->tel_pixmap.x + umod->x,
		    tel->tel_pixmap.y + umod->y,
		    umod->val);
#endif
	}
	if (SDL_MUSTLOCK(tv->scaled)) { SDL_UnlockSurface(tv->scaled); }

	px->curblk--;
#if 1
	tv->tile->flags |= TILE_DIRTY;
#endif
}

void
pixmap_redo(struct tileview *tv, struct tile_element *tel)
{
	struct pixmap *px = tel->tel_pixmap.px;
	
	dprintf("redo (curblk=%d )\n", px->curblk);
}

int
pixmap_put_pixel(struct tileview *tv, struct tile_element *tel, int x, int y,
    Uint32 pixel, int once)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct pixmap_undoblk *ublk = &px->ublks[px->nublks-1];
	struct pixmap_umod *umod;
	Uint8 *src;
	Uint8 r, g, b;
	u_int v = (pixel & px->su->format->Amask) >>
	          px->su->format->Ashift;
	Uint8 a = (v << px->su->format->Aloss) +
	          (v >> (8 - (px->su->format->Aloss << 1)));
	int i;
			
	/* Look for an existing mod to this pixel in this block. */
	for (i = ublk->numods-1; i >= 0; i--) {
		umod = &ublk->umods[i];
		if (umod->x == x && umod->y == y)
			break;
	}
	if (i >= 0) {
		if (once)
			return (1);
	} else {
		ublk->umods = Realloc(ublk->umods, (ublk->numods+1) *
				                   sizeof(struct pixmap_umod));
		umod = &ublk->umods[ublk->numods++];
		umod->type = PIXMAP_PIXEL_REPLACE;
		umod->x = (Uint16)x;
		umod->y = (Uint16)y;
	
		if (SDL_MUSTLOCK(px->su)) { SDL_LockSurface(px->su); }
		src = (Uint8 *)px->su->pixels + y*px->su->pitch +
		                                x*px->su->format->BytesPerPixel;
		umod->val = *(Uint32 *)src;
		if (SDL_MUSTLOCK(px->su)) { SDL_UnlockSurface(px->su); }
	}

	/* Plot the pixel on the pixmap and update the scaled display. */
	/* XXX use background caching to avoid regen for alpha pixels */
	switch (px->blend_mode) {
	case PIXMAP_NO_BLENDING:
		prim_put_pixel(px->su, x, y, pixel);
		if (a == 255) {
			tileview_scaled_pixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    pixel);
		} else {
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
	case PIXMAP_SPECIFIC_ALPHA:
		if (a == 255) {
			prim_put_pixel(px->su, x, y, pixel);
			tileview_scaled_pixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    pixel);
		} else {
			SDL_GetRGB(pixel, px->su->format, &r, &g, &b);
			prim_blend_rgb(px->su, x, y, PRIM_BLEND_SRCALPHA,
			    r, g, b, a);
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
	case PIXMAP_SPEC_AND_DEST_ALPHA:
		SDL_GetRGB(pixel, px->su->format, &r, &g, &b);
		prim_blend_rgb(px->su, x, y, PRIM_BLEND_MIXALPHA, r, g, b, a);
		tv->tile->flags |= TILE_DIRTY;
		break;
	case PIXMAP_BRUSH_ALPHA:
		SDL_GetRGBA(pixel, px->su->format, &r, &g, &b, &a);
		if (a == 255) {
			prim_put_pixel(px->su, x, y, pixel);
			tileview_scaled_pixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    pixel);
		} else {
			prim_blend_rgb(px->su, x, y, PRIM_BLEND_SRCALPHA,
			    r, g, b, a);
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
	case PIXMAP_BRUSH_AND_DEST_ALPHA:
		SDL_GetRGBA(pixel, px->su->format, &r, &g, &b, &a);
		prim_blend_rgb(px->su, x, y, PRIM_BLEND_MIXALPHA, r, g, b, a);
		tv->tile->flags |= TILE_DIRTY;
		break;
	case PIXMAP_DEST_ALPHA:
		if (a == 255) {
			prim_put_pixel(px->su, x, y, pixel);
			tileview_scaled_pixel(tv,
			    tel->tel_pixmap.x + x,
			    tel->tel_pixmap.y + y,
			    pixel);
		} else {
			SDL_GetRGB(pixel, px->su->format, &r, &g, &b);
			prim_blend_rgb(px->su, x, y, PRIM_BLEND_DSTALPHA,
			    r, g, b, a);
			tv->tile->flags |= TILE_DIRTY;
		}
		break;
	}
	return (0);
}

void
pixmap_apply_brush(struct tileview *tv, struct tile_element *tel,
    int x0, int y0, Uint32 specPx)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct pixmap_brush *br = px->curbrush;
	SDL_Surface *brsu = br->px->su;
	Uint8 *src = brsu->pixels;
	Uint8 r, g, b, specA;
	int x, y;
	
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
	for (y = 0; y < brsu->h; y++) {
		if (y0+y >= px->su->h) {
			break;
		}
		for (x = 0; x < brsu->w; x++) {
			Uint32 Px, brPx;
			Uint8 brA;

			if (x0+x >= px->su->w)
				break;

			brPx = *(Uint32 *)src;
			src += sizeof(Uint32);

			if (brsu->format->Amask != 0) {
				u_int v = (brPx & brsu->format->Amask) >>
				          brsu->format->Ashift;
				
				brA = (v << brsu->format->Aloss) +
				      (v >> (8 - (brsu->format->Aloss << 1)));
			} else {
				brA = 255;
			}

			switch (br->type) {
			case PIXMAP_BRUSH_MONO:
				SDL_GetRGB(specPx, brsu->format, &r, &g, &b);
				break;
			case PIXMAP_BRUSH_RGB:
				SDL_GetRGB(brPx, brsu->format, &r, &g, &b);
				break;
			}
			Px = SDL_MapRGBA(brsu->format, r, g, b,
			    (specA + brA)/2);

			if (brA != 0)
				pixmap_put_pixel(tv, tel, x0+x, y0+y, Px,
				br->flags & PIXMAP_BRUSH_ONESHOT);
		}
	}
	if (SDL_MUSTLOCK(brsu))
		SDL_UnlockSurface(brsu);
}

int
pixmap_mousewheel(struct tileview *tv, struct tile_element *tel,
    int nwheel)
{
	Uint8 *keystate = SDL_GetKeyState(NULL);
	struct pixmap *px = tel->tel_pixmap.px;

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
pixmap_apply(struct tileview *tv, struct tile_element *tel, int x, int y)
{
	struct pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b;
	Uint8 a = (Uint8)(px->a*255);
	Uint8 *keystate;
	int erase_mode;
	enum pixmap_blend_mode bmode_save = 0;
	enum pixmap_brush_type btype_save = 0;

	keystate = SDL_GetKeyState(NULL);
	erase_mode = keystate[SDLK_e];

	if (erase_mode) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		prim_hsv2rgb(px->h/360.0, px->s, px->v, &r, &g, &b);
	}

	if (erase_mode) {
		bmode_save = px->blend_mode;
		px->blend_mode = PIXMAP_NO_BLENDING;
	}

	if (px->curbrush != NULL) {
		if (erase_mode) {
			btype_save = px->curbrush->type;
			px->curbrush->type = PIXMAP_BRUSH_MONO;
		}

		pixmap_apply_brush(tv, tel, x, y,
		    SDL_MapRGBA(px->su->format, r, g, b, a));

		if (erase_mode) {
			px->curbrush->type = btype_save;
		}
	} else {
		pixmap_put_pixel(tv, tel, x, y,
		    SDL_MapRGBA(px->su->format, r, g, b, a), 1);
	}
	
	if (erase_mode)
		px->blend_mode = bmode_save;
}

static void
fill_ortho(struct tileview *tv, struct tile_element *tel, int x, int y,
    Uint32 cOrig, Uint32 cFill)
{
	struct pixmap *px = tel->tel_pixmap.px;
	Uint8 *pDst = (Uint8 *)px->su->pixels +
		     y*px->su->pitch +
		     x*px->su->format->BytesPerPixel;
	Uint32 cDst = *(Uint32 *)pDst;

	if (cDst != cOrig)
		return;
	
	if (pixmap_put_pixel(tv, tel, x, y, cFill, 1) == 1)
		return;

	if (x-1 >= 0)		fill_ortho(tv, tel, x-1, y, cOrig, cFill);
	if (y-1 >= 0)		fill_ortho(tv, tel, x, y-1, cOrig, cFill);
	if (x+1 < px->su->w)	fill_ortho(tv, tel, x+1, y, cOrig, cFill);
	if (y+1 < px->su->h)	fill_ortho(tv, tel, x, y+1, cOrig, cFill);
}

static void
pixmap_fill(struct tileview *tv, struct tile_element *tel, int x, int y)
{
	struct pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b, a = (Uint8)(px->a*255);
	Uint8 *keystate;
	Uint8 *pOrig = (Uint8 *)px->su->pixels +
		     y*px->su->pitch +
		     x*px->su->format->BytesPerPixel;
	Uint32 cOrig = *(Uint32 *)pOrig, cFill;

	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_e]) {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	} else {
		prim_hsv2rgb(px->h/360.0, px->s, px->v, &r, &g, &b);
	}
	cFill = SDL_MapRGBA(px->su->format, r, g, b, a);
	fill_ortho(tv, tel, x, y, cOrig, cFill);
	tv->tile->flags |= TILE_DIRTY;
}

void
pixmap_mousebuttondown(struct tileview *tv, struct tile_element *tel,
    int x, int y, int button)
{
	struct pixmap *px = tel->tel_pixmap.px;
	Uint8 *keystate;

	pixmap_begin_undoblk(px);
	
	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_f]) {
		pixmap_fill(tv, tel, x, y);
	} else {
		tv->tv_pixmap.state = TILEVIEW_PIXMAP_FREEHAND;
		pixmap_apply(tv, tel, x, y);
	}
}

void
pixmap_mousebuttonup(struct tileview *tv, struct tile_element *tel, int x,
    int y, int button)
{
	tv->tv_pixmap.state = TILEVIEW_PIXMAP_IDLE;
}

void
pixmap_keydown(struct tileview *tv, struct tile_element *tel,
    int keysym, int keymod)
{
	switch (keysym) {
	case SDLK_f:
		if (saved_cursor == NULL) {
			saved_cursor = SDL_GetCursor();
			SDL_SetCursor(fill_cursor);
		}
		break;
	case SDLK_e:
		if (saved_cursor == NULL) {
			saved_cursor = SDL_GetCursor();
			SDL_SetCursor(erase_cursor);
		}
		break;
	}
}

void
pixmap_keyup(struct tileview *tv, struct tile_element *tel, int keysym,
    int keymod)
{
	if (saved_cursor != NULL) {
		SDL_SetCursor(saved_cursor);
		saved_cursor = NULL;
	}
}

void
pixmap_mousemotion(struct tileview *tv, struct tile_element *tel, int x, int y,
    int xrel, int yrel, int state)
{
	struct pixmap *px = tel->tel_pixmap.px;

	if (tv->tv_pixmap.state == TILEVIEW_PIXMAP_FREEHAND)
		pixmap_apply(tv, tel, x, y);
}
