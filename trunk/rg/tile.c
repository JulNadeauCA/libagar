/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>
#include <core/load_surface.h>
#include <core/load_xcf.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/tlist.h>
#include <gui/button.h>
#include <gui/textbox.h>
#include <gui/menu.h>
#include <gui/checkbox.h>
#include <gui/spinbutton.h>
#include <gui/mspinbutton.h>
#include <gui/toolbar.h>
#include <gui/label.h>
#include <gui/separator.h>
#include <gui/radio.h>
#include <gui/separator.h>
#include <gui/file_dlg.h>
#include <gui/pane.h>
#include <gui/icons.h>

#include "tileset.h"
#include "tileview.h"
#include "fill.h"
#include "sketchproj.h"
#include "icons.h"

#include <string.h>

const char *rgTileSnapModes[] = {
	N_("No snapping"),
	N_("Snap to grid"),
	NULL
};

/*
 * Blend a pixmap with the tile; add the source alpha to the destination
 * alpha of each pixel.
 */
static void
BlendOverlayAlpha(RG_Tile *t, SDL_Surface *su, SDL_Rect *rd)
{
	Uint sx, sy, dx, dy;
	Uint8 *pSrc, *pDst;
	Uint8 sR, sG, sB, sA;
	Uint8 dR, dG, dB, dA;
	int alpha;

	SDL_LockSurface(su);
	SDL_LockSurface(t->su);
	for (sy = 0, dy = rd->y; sy < su->h; sy++, dy++) {
		for (sx = 0, dx = rd->x; sx < su->w; sx++, dx++) {
			if (AG_CLIPPED_PIXEL(t->su, dx, dy))
				continue;

			pSrc = (Uint8 *)su->pixels + sy*su->pitch + (sx << 2);

			if (*(Uint32 *)pSrc == su->format->colorkey)
				continue;

			pDst = (Uint8 *)t->su->pixels + dy*t->su->pitch +
					(dx << 2);

			if (*(Uint32 *)pDst != t->su->format->colorkey) {
				SDL_GetRGBA(*(Uint32 *)pDst, t->su->format,
				    &dR, &dG, &dB, &dA);
				SDL_GetRGBA(*(Uint32 *)pSrc, su->format,
				    &sR, &sG, &sB, &sA);

				alpha = dA + sA;
				if (alpha > 255) {
					alpha = 255;
				}
				*(Uint32 *)pDst = SDL_MapRGBA(t->su->format,
				    (((sR - dR) * sA) >> 8) + dR,
				    (((sG - dG) * sA) >> 8) + dG,
				    (((sB - dB) * sA) >> 8) + dB,
				    (Uint8)alpha);
			} else {
				*(Uint32 *)pDst = *(Uint32 *)pSrc;
			}
		}
	}
	SDL_UnlockSurface(t->su);
	SDL_UnlockSurface(su);
}

#if 0
static void
BlendSDL(RG_Tile *t, SDL_Surface *su, SDL_Rect *rd)
{
	SDL_BlitSurface(su, NULL, t->su, rd);
}
#endif

RG_Tile *
RG_TileNew(RG_Tileset *ts, const char *pName, Uint16 w, Uint16 h, Uint flags)
{
	char name[RG_TILE_NAME_MAX];
	int no = 0;
	RG_Tile *t;

tryname:
	snprintf(name, sizeof(name), _("%s #%d"),
	    (pName != NULL) ? pName : _("Tile"), no++);
	if (RG_TilesetFindTile(ts, name) != NULL)
		goto tryname;

	t = Malloc(sizeof(RG_Tile));
	RG_TileInit(t, ts, name);
	RG_TileScale(ts, t, w, h, flags);

	if ((ts->ntiletbl+1) >= RG_TILE_ID_MAX) {
		AG_FatalError("Out of tile mappings");
	}
	ts->tiletbl = Realloc(ts->tiletbl, (ts->ntiletbl+1)*sizeof(RG_Tile *));
	ts->tiletbl[ts->ntiletbl] = t;
	t->main_id = ts->ntiletbl++;
	TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	return (t);
}

void
RG_TileInit(RG_Tile *t, RG_Tileset *ts, const char *name)
{
	strlcpy(t->name, name, sizeof(t->name));
	t->clname[0] = '\0';
	t->main_id = 0;
	t->flags = 0;
	t->su = NULL;
	t->ts = ts;
	t->nrefs = 0;
	t->blend_fn = BlendOverlayAlpha;
/*	t->blend_fn = BlendSDL; */
	t->attrs = NULL;
	t->layers = NULL;
	t->nw = 0;
	t->nh = 0;
	TAILQ_INIT(&t->elements);
	SLIST_INIT(&t->vars);
}

void
RG_TileScale(RG_Tileset *ts, RG_Tile *t, Uint16 w, Uint16 h, Uint flags)
{
	Uint32 sflags = SDL_SWSURFACE;
	int wNew, hNew;
	int x, y;
	Uint *nAttrs;
	int *nLayers;

	/* Resize the attribute grids. */
	wNew = w/RG_TILESZ + 1;
	hNew = h/RG_TILESZ + 1;
	nAttrs = Malloc(wNew*hNew*sizeof(Uint));
	nLayers = Malloc(wNew*hNew*sizeof(int));
	for (y = 0; y < hNew; y++) {
		for (x = 0; x < wNew; x++) {
			if (x >= t->nw || y >= t->nh) {
				nAttrs[y*wNew + x] = 0;
				nLayers[y*wNew + x] = 0;
			} else {
				nAttrs[y*wNew + x] = t->attrs[y*t->nw + x];
				nLayers[y*wNew + x] = t->layers[y*t->nw + x];
			}
		}
	}
	Free(t->attrs);
	Free(t->layers);
	t->attrs = nAttrs;
	t->layers = nLayers;

	t->nw = wNew;
	t->nh = hNew;

	if (flags & RG_TILE_SRCCOLORKEY) {
		sflags |= SDL_SRCCOLORKEY;
	}
	if (flags & RG_TILE_SRCALPHA) {
		sflags |= SDL_SRCALPHA;
	}
	t->flags = flags|RG_TILE_DIRTY;
	t->su = SDL_CreateRGBSurface(sflags, w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask, ts->fmt->Gmask, ts->fmt->Bmask, ts->fmt->Amask);
	if (t->su == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
}

void
RG_TileGenerate(RG_Tile *t)
{
	RG_TileElement *tel;
	
	SDL_SetAlpha(t->su, SDL_SRCALPHA, t->ts->fmt->alpha);

	/* TODO check for opaque fill features/pixmaps first */
	SDL_FillRect(t->su, NULL, SDL_MapRGBA(t->su->format, 0, 0, 0, 0));

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (!tel->visible) {
			continue;
		}
		switch (tel->type) {
		case RG_TILE_FEATURE:
			{
				RG_Feature *ft = tel->tel_feature.ft;

				if (ft->ops->apply != NULL)
					ft->ops->apply(ft, t,
					    tel->tel_feature.x,
					    tel->tel_feature.y);
			}
			break;
		case RG_TILE_PIXMAP:
			{
				RG_Pixmap *px = tel->tel_pixmap.px;
				SDL_Rect rd;

				rd.x = tel->tel_pixmap.x;
				rd.y = tel->tel_pixmap.y;
				rd.w = px->su->w;
				rd.h = px->su->h;
				t->blend_fn(t, px->su, &rd);
			}
			break;
		case RG_TILE_SKETCH:
			RG_SketchRender(t, tel);
			break;
		}
	}

	if ((t->flags & RG_TILE_SRCALPHA) == 0 &&
	    (t->flags & RG_TILE_SRCCOLORKEY)) {
		SDL_Surface *su = t->su;
		Uint i, size = su->w*su->h;
		Uint8 *p = su->pixels;
		Uint8 r, g, b, a;

		SDL_LockSurface(su);
		for (i = 0; i < size; i++) {
			SDL_GetRGBA(AG_GET_PIXEL(su,p), su->format,
			    &r, &g, &b, &a);
			if (a == 0) {
				AG_PUT_PIXEL(su, p, su->format->colorkey);
			} else {
				AG_PUT_PIXEL(su, p,
				    SDL_MapRGBA(su->format, r, g, b, a));
			}
			p += su->format->BytesPerPixel;
		}
		SDL_UnlockSurface(su);
		
		SDL_SetAlpha(t->su, 0, 0);
		SDL_SetColorKey(t->su, SDL_SRCCOLORKEY, t->ts->fmt->colorkey);
	} else if ((t->flags & (RG_TILE_SRCCOLORKEY|RG_TILE_SRCALPHA)) == 0) {
		SDL_SetAlpha(t->su, 0, 0);
		SDL_SetColorKey(t->su, 0, 0);
	} else {
		SDL_SetColorKey(t->su, SDL_SRCCOLORKEY, t->ts->fmt->colorkey);
	}
}

static __inline__ void
GenerateElementName(RG_TileElement *tel, RG_Tile *t, const char *fname)
{
	RG_TileElement *oel;
	Uint elno = 0;

tryname:
	snprintf(tel->name, sizeof(tel->name), "%s <#%u>", fname,
	    elno);
	TAILQ_FOREACH(oel, &t->elements, elements) {
		if (strcmp(oel->name, tel->name) == 0)
			break;
	}
	if (oel != NULL) {
		elno++;
		goto tryname;
	}
}

RG_TileElement *
RG_TileFindElement(RG_Tile *t, enum rg_tile_element_type type, const char *name)
{
	RG_TileElement *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type == type &&
		    strcmp(tel->name, name) == 0)
			break;
	}
	return (tel);
}

RG_TileElement *
RG_TileAddFeature(RG_Tile *t, const char *name, void *ft, int x, int y)
{
	RG_TileElement *tel;

	tel = Malloc(sizeof(RG_TileElement));
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		GenerateElementName(tel, t, RG_FEATURE(ft)->name);
	}
	tel->type = RG_TILE_FEATURE;
	tel->visible = 1;
	tel->tel_feature.ft = ft;
	tel->tel_feature.x = x;
	tel->tel_feature.y = y;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	t->flags |= RG_TILE_DIRTY;
	return (tel);
}

void
RG_TileDelFeature(RG_Tile *t, void *ftp, int destroy)
{
	RG_Feature *ft = ftp;
	RG_TileElement *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_feature.ft == ft)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel);

		if (--ft->nrefs == 0 && destroy) {
			AG_TextInfo(_("Destroying unreferenced feature: %s"),
			    ft->name);
			TAILQ_REMOVE(&t->ts->features, ft, features);
			AG_FeatureDestroy(ft);
			Free(ft);
		}
	}
}

RG_TileElement *
RG_TileAddPixmap(RG_Tile *t, const char *name, RG_Pixmap *px, int x, int y)
{
	RG_TileElement *tel;

	tel = Malloc(sizeof(RG_TileElement));
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		GenerateElementName(tel, t, px->name);
	}
	tel->type = RG_TILE_PIXMAP;
	tel->visible = 1;
	tel->tel_pixmap.px = px;
	tel->tel_pixmap.x = x;
	tel->tel_pixmap.y = y;
	tel->tel_pixmap.alpha = 255;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	px->nrefs++;

	t->flags |= RG_TILE_DIRTY;
	return (tel);
}

RG_TileElement *
RG_TileAddSketch(RG_Tile *t, const char *name, RG_Sketch *sk, int x, int y)
{
	RG_TileElement *tel;

	tel = Malloc(sizeof(RG_TileElement));
	if (name != NULL) {
		strlcpy(tel->name, name, sizeof(tel->name));
	} else {
		GenerateElementName(tel, t, sk->name);
	}
	tel->type = RG_TILE_SKETCH;
	tel->visible = 1;
	tel->tel_sketch.sk = sk;
	tel->tel_sketch.x = x;
	tel->tel_sketch.y = y;
	tel->tel_sketch.alpha = 255;
	tel->tel_sketch.scale = 1.0;
	TAILQ_INSERT_TAIL(&t->elements, tel, elements);
	sk->nrefs++;
	t->flags |= RG_TILE_DIRTY;
	return (tel);
}

void
RG_TileDelPixmap(RG_Tile *t, RG_Pixmap *px, int destroy)
{
	RG_TileElement *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_pixmap.px == px)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel);
		if (--px->nrefs == 0 && destroy) {
			AG_TextInfo(_("Destroying unreferenced pixmap: %s"),
			    px->name);
			TAILQ_REMOVE(&t->ts->pixmaps, px, pixmaps);
			RG_PixmapDestroy(px);
			Free(px);
		}
	}
}

void
RG_TileDelSketch(RG_Tile *t, RG_Sketch *sk, int destroy)
{
	RG_TileElement *tel;

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->tel_sketch.sk == sk)
			break;
	}
	if (tel != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		Free(tel);
		if (--sk->nrefs == 0 && destroy) {
			AG_TextInfo(_("Destroying unreferenced sketch: %s"),
			    sk->name);
			TAILQ_REMOVE(&t->ts->sketches, sk, sketches);
			RG_SketchDestroy(sk);
			Free(sk);
		}
	}
}

void
RG_TileSave(RG_Tile *t, AG_DataSource *buf)
{
	Uint32 nelements = 0;
	off_t nelements_offs;
	RG_TileElement *tel;
	int x, y;

	AG_WriteString(buf, t->name);
	AG_WriteUint8(buf, t->flags & ~RG_TILE_DIRTY);
	AG_WriteSurface(buf, t->su);
	
	AG_WriteSint16(buf, (Sint16)t->xOrig);
	AG_WriteSint16(buf, (Sint16)t->yOrig);
	AG_WriteUint8(buf, (Uint8)t->snap_mode);

	AG_WriteUint32(buf, (Uint32)t->nw);
	AG_WriteUint32(buf, (Uint32)t->nh);
	for (y = 0; y < t->nh; y++) {
		for (x = 0; x < t->nw; x++) {
			AG_WriteUint32(buf, (Uint32)t->attrs[y*t->nw + x]);
			AG_WriteSint32(buf, (Sint32)t->layers[y*t->nw + x]);
		}
	}

	nelements_offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(tel, &t->elements, elements) {
		AG_WriteString(buf, tel->name);
		AG_WriteUint32(buf, (Uint32)tel->type);
		AG_WriteUint8(buf, (Uint8)tel->visible);

		switch (tel->type) {
		case RG_TILE_FEATURE:
			{
				RG_Feature *ft = tel->tel_feature.ft;

				AG_WriteString(buf, ft->name);
				AG_WriteSint32(buf, (Sint32)tel->tel_feature.x);
				AG_WriteSint32(buf, (Sint32)tel->tel_feature.y);
			}
			break;
		case RG_TILE_PIXMAP:
			{
				RG_Pixmap *px = tel->tel_pixmap.px;

				AG_WriteString(buf, px->name);
				AG_WriteSint32(buf, (Sint32)tel->tel_pixmap.x);
				AG_WriteSint32(buf, (Sint32)tel->tel_pixmap.y);
				AG_WriteUint8(buf,
				    (Uint8)tel->tel_pixmap.alpha);
			}
			break;
		case RG_TILE_SKETCH:
			{
				RG_Sketch *sk = tel->tel_sketch.sk;

				AG_WriteString(buf, sk->name);
				AG_WriteSint32(buf, (Sint32)tel->tel_sketch.x);
				AG_WriteSint32(buf, (Sint32)tel->tel_sketch.y);
				AG_WriteUint8(buf,
				    (Uint8)tel->tel_sketch.alpha);
			}
			break;
		}
		nelements++;
	}
	AG_WriteUint32At(buf, nelements, nelements_offs);
}

int
RG_TileLoad(RG_Tile *t, AG_DataSource *buf)
{
	RG_Tileset *ts = t->ts;
	Uint32 i, nelements;
	int x, y;
	
	t->flags = AG_ReadUint8(buf);
	t->su = AG_ReadSurface(buf, ts->fmt);

	t->xOrig = (int)AG_ReadSint16(buf);
	t->yOrig = (int)AG_ReadSint16(buf);
	t->snap_mode = (int)AG_ReadUint8(buf);

	t->nw = (Uint)AG_ReadUint32(buf);
	t->nh = (Uint)AG_ReadUint32(buf);
	t->attrs = Realloc(t->attrs, t->nw*t->nh*sizeof(Uint));
	t->layers = Realloc(t->layers, t->nw*t->nh*sizeof(int));
	for (y = 0; y < t->nh; y++) {
		for (x = 0; x < t->nw; x++) {
			t->attrs[y*t->nw + x] = (Uint)AG_ReadUint32(buf);
			t->layers[y*t->nw + x] = (int)AG_ReadSint32(buf);
		}
	}

	nelements = AG_ReadUint32(buf);
	for (i = 0; i < nelements; i++) {
		char name[RG_TILE_ELEMENT_NAME_MAX];
		enum rg_tile_element_type type;
		RG_TileElement *tel;
		int visible;

		AG_CopyString(name, buf, sizeof(name));
		type = (enum rg_tile_element_type)AG_ReadUint32(buf);
		visible = (int)AG_ReadUint8(buf);

		switch (type) {
		case RG_TILE_FEATURE:
			{
				char feat_name[RG_FEATURE_NAME_MAX];
				RG_Feature *ft;
				Sint32 x, y;

				AG_CopyString(feat_name, buf,
				    sizeof(feat_name));
				x = AG_ReadSint32(buf);
				y = AG_ReadSint32(buf);
				TAILQ_FOREACH(ft, &ts->features, features) {
					if (strcmp(ft->name, feat_name) == 0)
						break;
				}
				if (ft != NULL) {
					tel = RG_TileAddFeature(t, name, ft,
					    x, y);
					tel->visible = visible;
				} else {
					AG_TextMsg(AG_MSG_ERROR,
					    _("%s: no such feature: %s "
					      "(ignored)"), t->name, feat_name);
				}
			}
			break;
		case RG_TILE_PIXMAP:
			{
				char pix_name[RG_PIXMAP_NAME_MAX];
				RG_Pixmap *px;
				Sint32 x, y;
				int alpha;

				AG_CopyString(pix_name, buf, sizeof(pix_name));
				x = AG_ReadSint32(buf);
				y = AG_ReadSint32(buf);
				alpha = (int)AG_ReadUint8(buf);

				TAILQ_FOREACH(px, &ts->pixmaps, pixmaps) {
					if (strcmp(px->name, pix_name) == 0)
						break;
				}
				if (px == NULL) {
					AG_SetError("bad pixmap: %s", pix_name);
					return (-1);
				}
				tel = RG_TileAddPixmap(t, name, px, x, y);
				tel->tel_pixmap.alpha = alpha;
				tel->visible = visible;
			}
			break;
		case RG_TILE_SKETCH:
			{
				char sk_name[RG_SKETCH_NAME_MAX];
				RG_Sketch *sk;
				Sint32 x, y;
				int alpha;

				AG_CopyString(sk_name, buf, sizeof(sk_name));
				x = AG_ReadSint32(buf);
				y = AG_ReadSint32(buf);
				alpha = (int)AG_ReadUint8(buf);

				TAILQ_FOREACH(sk, &ts->sketches, sketches) {
					if (strcmp(sk->name, sk_name) == 0)
						break;
				}
				if (sk == NULL) {
					AG_SetError("bad sketch: %s", sk_name);
					return (-1);
				}
				tel = RG_TileAddSketch(t, name, sk, x, y);
				tel->tel_sketch.alpha = alpha;
				tel->visible = visible;
			}
			break;
		default:
			break;
		}
	}
	t->flags &= ~RG_TILE_DIRTY;
	return (0);
}

void
RG_TileDestroy(RG_Tile *t)
{
	RG_TileElement *tel, *tel_next;
	RG_TileVariant *var, *var_next;

	Free(t->attrs);
	Free(t->layers);
	SDL_FreeSurface(t->su);
	
	for (tel = TAILQ_FIRST(&t->elements);
	     tel != TAILQ_END(&t->elements);
	     tel = tel_next) {
		tel_next = TAILQ_NEXT(tel, elements);
		Free(tel);
	}
	for (var = SLIST_FIRST(&t->vars);
	     var != SLIST_END(&t->vars);
	     var = var_next) {
		var_next = SLIST_NEXT(var, vars);
		Free(var);
	}
}

#ifdef EDITION

static void
GeoCtrlButtonUp(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	RG_TileviewCtrl *ctrl = AG_PTR(1);
	RG_Tileset *ts = tv->ts;
	RG_Tile *t = tv->tile;
	int w = RG_TileviewInt(ctrl, 2);
	int h = RG_TileviewInt(ctrl, 3);
	
	if (w != t->su->w || h != t->su->h)  {
		RG_TileScale(ts, t, w, h, t->flags);
		RG_TileviewSetZoom(tv, tv->zoom, 0);
	}
}

static void
CloseElement(RG_Tileview *tv)
{
	RG_TileviewCtrl *ctrl, *nctrl;
	RG_Tile *t = tv->tile;

	for (ctrl = TAILQ_FIRST(&tv->ctrls);
	     ctrl != TAILQ_END(&tv->ctrls);
	     ctrl = nctrl) {
		nctrl = TAILQ_NEXT(ctrl, ctrls);
		RG_TileviewDelCtrl(tv, ctrl);
	}

	switch (tv->state) {
	case RG_TILEVIEW_TILE_EDIT:
		tv->tv_tile.geo_ctrl = NULL;
		tv->tv_tile.orig_ctrl = NULL;
		break;
	case RG_TILEVIEW_FEATURE_EDIT:
		if (tv->tv_feature.ft->ops->flags & FEATURE_AUTOREDRAW) {
			RG_TileviewSetAutoRefresh(tv, 0, 0);
		}
		if (tv->tv_feature.menu != NULL) {
			RG_FeatureCloseMenu(tv);
		}
		if (tv->tv_feature.win != NULL) {
			AG_ViewDetach(tv->tv_feature.win);
			tv->tv_feature.win = NULL;
		}
		break;
	case RG_TILEVIEW_PIXMAP_EDIT:
		if (tv->tv_pixmap.win != NULL) {
			tv->tv_pixmap.ctrl = NULL;
			AG_ViewDetach(tv->tv_pixmap.win);
			tv->tv_pixmap.win = NULL;
		}
		break;
	case RG_TILEVIEW_SKETCH_EDIT:
		if (tv->tv_sketch.win != NULL) {
			tv->tv_sketch.ctrl = NULL;
			AG_ViewDetach(tv->tv_sketch.win);
			tv->tv_sketch.win = NULL;
		}
		break;
	default:
		if (tv->menu != NULL) {
			RG_TileCloseMenu(tv);
		}
		break;
	}
	
	tv->state = RG_TILEVIEW_TILE_EDIT;
	tv->edit_mode = 0;

	tv->tv_tile.geo_ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_RDIMENSIONS,
	    "%i,%i,%u,%u", 0, 0,
	    (Uint)t->su->w,
	    (Uint)t->su->h);
	tv->tv_tile.geo_ctrl->buttonup =
	    AG_SetEvent(tv, NULL, GeoCtrlButtonUp, "%p",
	    tv->tv_tile.geo_ctrl);

	tv->tv_tile.orig_ctrl = RG_TileviewAddCtrl(tv, RG_TILEVIEW_POINT,
	    "%*i,%*i", &t->xOrig, &t->yOrig);

	tv->tv_tile.orig_ctrl->cIna.r = 0;
	tv->tv_tile.orig_ctrl->cIna.g = 255;
	tv->tv_tile.orig_ctrl->cIna.b = 0;
	tv->tv_tile.orig_ctrl->cHigh.r = 73;
	tv->tv_tile.orig_ctrl->cHigh.g = 186;
	tv->tv_tile.orig_ctrl->cHigh.b = 51;
	tv->tv_tile.orig_ctrl->cLow.r = 86;
	tv->tv_tile.orig_ctrl->cLow.g = 161;
	tv->tv_tile.orig_ctrl->cLow.b = 71;
	tv->tv_tile.orig_ctrl->cOver.r = 191;
	tv->tv_tile.orig_ctrl->cOver.g = 170;
	tv->tv_tile.orig_ctrl->cOver.b = 47;
	tv->tv_tile.orig_ctrl->aEna = 70;
	tv->tv_tile.orig_ctrl->aIna = 30;
	tv->tv_tile.orig_ctrl->aOver= 100;

	if (tv->tel_tbar != NULL) {
		AG_ObjectDetach(tv->tel_tbar);
		AG_WindowUpdate(AG_WidgetParentWindow(tv->tel_tbar));
		AG_ObjectDestroy(tv->tel_tbar);
		Free(tv->tel_tbar);
		tv->tel_tbar = NULL;
	}
}

static void
ElementClosedEv(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);

	if (tv->edit_mode)
		CloseElement(tv);
}

static void
PixmapCtrlButtonUp(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	RG_TileviewCtrl *ctrl = AG_PTR(1);
	RG_Pixmap *px = AG_PTR(2);
	RG_Tile *t = tv->tile;
	int w = RG_TileviewInt(ctrl, 2);
	int h = RG_TileviewInt(ctrl, 3);
	
	if (w != px->su->w || h != px->su->h) {
		RG_PixmapScale(px, w, h, ctrl->xoffs, ctrl->yoffs);
	}
	t->flags |= RG_TILE_DIRTY;
}

static void
SketchCtrlButtonUp(AG_Event *event)
{
	RG_Tileview *tv = AG_SELF();
	RG_TileviewCtrl *ctrl = AG_PTR(1);
	RG_TileElement *tel = AG_PTR(2);
	RG_Sketch *sk = tel->tel_sketch.sk;
	RG_Tile *t = tv->tile;
	int w = RG_TileviewInt(ctrl, 2);
	int h = RG_TileviewInt(ctrl, 3);
	
	if (w != sk->vg->su->w || h != sk->vg->su->h)  {
		RG_SketchScale(sk, w, h, tel->tel_sketch.scale,
		    ctrl->xoffs, ctrl->yoffs);
	}
	t->flags |= RG_TILE_DIRTY;
}

static void
OpenElement(RG_Tileview *tv, RG_TileElement *tel)
{
	AG_Window *pWin;
	
	if ((pWin = AG_WidgetParentWindow(tv)) == NULL)
		return;

	CloseElement(tv);

	if (tv->state == RG_TILEVIEW_TILE_EDIT) {
		RG_TileviewDelCtrl(tv, tv->tv_tile.geo_ctrl);
		RG_TileviewDelCtrl(tv, tv->tv_tile.orig_ctrl);
		tv->tv_tile.geo_ctrl = NULL;
		tv->tv_tile.orig_ctrl = NULL;
	}

	switch (tel->type) {
	case RG_TILE_FEATURE:
		{
			AG_Window *win;
			RG_Feature *ft = tel->tel_feature.ft;

			tv->state = RG_TILEVIEW_FEATURE_EDIT;
			tv->tv_feature.ft = ft;
			tv->tv_feature.menu = NULL;
			tv->tv_feature.menu_item = NULL;
			tv->tv_feature.menu_win = NULL;

			if (ft->ops->flags & FEATURE_AUTOREDRAW)
				RG_TileviewSetAutoRefresh(tv, 1, 125);
			
			if (ft->ops->edit != NULL) {
				win = ft->ops->edit(ft, tv);
				AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT,
				    0);
				AG_WindowAttach(pWin, win);
				AG_WindowShow(win);

				tv->tv_feature.win = win;
				AG_SetEvent(win, "window-close",
				    ElementClosedEv, "%p", tv);
				
				agView->winToFocus = pWin;
				AG_WidgetFocus(tv);
			} else {
				tv->tv_feature.win = NULL;
			}

			if (ft->ops->toolbar != NULL) {
				tv->tel_tbar = ft->ops->toolbar(ft, tv);
				AG_WindowUpdate(pWin);
			}
		}
		break;
	case RG_TILE_PIXMAP:
		{
			AG_Window *win;
			
			tv->state = RG_TILEVIEW_PIXMAP_EDIT;
			tv->tv_pixmap.px = tel->tel_pixmap.px;
			tv->tv_pixmap.tel = tel;
			tv->tv_pixmap.ctrl = RG_TileviewAddCtrl(tv,
			    RG_TILEVIEW_RECTANGLE, "%*i,%*i,%u,%u",
			    &tel->tel_pixmap.x,
			    &tel->tel_pixmap.y,
			    (Uint)tel->tel_pixmap.px->su->w,
			    (Uint)tel->tel_pixmap.px->su->h);
			tv->tv_pixmap.ctrl->buttonup =
			    AG_SetEvent(tv, NULL, PixmapCtrlButtonUp, "%p,%p",
			    tv->tv_pixmap.ctrl, tel->tel_pixmap.px);
			tv->tv_pixmap.state = RG_TVPIXMAP_IDLE;
			tv->tv_pixmap.win = win = RG_PixmapEdit(tv, tel);
			tv->tv_pixmap.menu = NULL;
			tv->tv_pixmap.menu_item = NULL;
			tv->tv_pixmap.menu_win = NULL;

			AG_WindowAttach(pWin, win);
			AG_WindowShow(win);
			AG_SetEvent(win, "window-close", ElementClosedEv,
			    "%p",tv);
			agView->winToFocus = pWin;
			AG_WidgetFocus(tv);
			
			tv->tel_tbar = RG_PixmapToolbar(tv, tel);
			AG_WindowUpdate(pWin);
		}
		break;
	case RG_TILE_SKETCH:
		{
			AG_Window *win;
			
			tv->state = RG_TILEVIEW_SKETCH_EDIT;
			tv->tv_sketch.sk = tel->tel_sketch.sk;
			tv->tv_sketch.tel = tel;
			tv->tv_sketch.ctrl = RG_TileviewAddCtrl(tv,
			    RG_TILEVIEW_RECTANGLE, "%*i,%*i,%u,%u",
			    &tel->tel_sketch.x,
			    &tel->tel_sketch.y,
			    tel->tel_sketch.sk->vg->su->w,
			    tel->tel_sketch.sk->vg->su->h);
			tv->tv_sketch.ctrl->buttonup =
			    AG_SetEvent(tv, NULL, SketchCtrlButtonUp, "%p,%p",
			    tv->tv_sketch.ctrl, tel);
			tv->tv_sketch.win = win = RG_SketchEdit(tv, tel);
			tv->tv_sketch.menu = NULL;
			tv->tv_sketch.menu_item = NULL;
			tv->tv_sketch.menu_win = NULL;

			AG_WindowAttach(pWin, win);
			AG_WindowShow(win);
			AG_SetEvent(win, "window-close", ElementClosedEv,
			    "%p",tv);
			agView->winToFocus = pWin;
			AG_WidgetFocus(tv);

			tv->tel_tbar = RG_SketchToolbar(tv, tel);
			AG_WindowUpdate(pWin);
		}
		break;
	}
	tv->edit_mode = 1;
}

static void
CreatePixmap(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	AG_TlistItem *eit;
	RG_Pixmap *px;
	RG_TileElement *tel;
	Uint pixno = 0;
	RG_Pixmap *opx;

	px = Malloc(sizeof(RG_Pixmap));
	RG_PixmapInit(px, tv->ts, 0);
tryname:
	snprintf(px->name, sizeof(px->name), "pixmap #%u", pixno);
	TAILQ_FOREACH(opx, &tv->ts->pixmaps, pixmaps) {
		if (strcmp(opx->name, px->name) == 0)
			break;
	}
	if (opx != NULL) {
		pixno++;
		goto tryname;
	}

	RG_PixmapScale(px, tv->tile->su->w, tv->tile->su->h, 0, 0);
	TAILQ_INSERT_TAIL(&tv->ts->pixmaps, px, pixmaps);

	tel = RG_TileAddPixmap(tv->tile, NULL, px, 0, 0);
	OpenElement(tv, tel);

	/* Select the newly inserted feature. */
	AG_PostEvent(NULL, tlFeatures, "tlist-poll", NULL);
	AG_TlistDeselectAll(tlFeatures);
	TAILQ_FOREACH(eit, &tlFeatures->items, items) {
		RG_TileElement *tel;

		if (strcmp(eit->cat, "pixmap") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_pixmap.px == px) {
			AG_TlistSelect(tlFeatures, eit);
			break;
		}
	}
}

static void
CreateSketch(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	AG_TlistItem *eit;
	RG_Sketch *sk, *osk;
	RG_TileElement *tel;
	Uint skno = 0;

	sk = Malloc(sizeof(RG_Sketch));
	RG_SketchInit(sk, tv->ts, 0);
tryname:
	snprintf(sk->name, sizeof(sk->name), "sketch #%u", skno);
	TAILQ_FOREACH(osk, &tv->ts->sketches, sketches) {
		if (strcmp(osk->name, sk->name) == 0)
			break;
	}
	if (osk != NULL) {
		skno++;
		goto tryname;
	}

	RG_SketchScale(sk, tv->tile->su->w, tv->tile->su->h, 1.0, 0, 0);
	TAILQ_INSERT_TAIL(&tv->ts->sketches, sk, sketches);
	tel = RG_TileAddSketch(tv->tile, NULL, sk, 0, 0);
	tv->tile->flags |= RG_TILE_DIRTY;
	OpenElement(tv, tel);

	/* Select the newly inserted feature. */
	AG_PostEvent(NULL, tlFeatures, "tlist-poll", NULL);
	AG_TlistDeselectAll(tlFeatures);
	TAILQ_FOREACH(eit, &tlFeatures->items, items) {
		RG_TileElement *tel;

		if (strcmp(eit->cat, "sketch") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_sketch.sk == sk) {
			AG_TlistSelect(tlFeatures, eit);
			break;
		}
	}
}

static void
AttachPixmap(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Window *dlgWin = AG_PTR(2);
	AG_Tlist *tlFeatures = AG_PTR(3);
	AG_Tlist *tl_pixmaps = AG_PTR(4);
	AG_TlistItem *it;
	RG_TileElement *tel;
	RG_Pixmap *px;

	if ((it = AG_TlistSelectedItem(tl_pixmaps)) == NULL) {
		return;
	}
	px = it->p1;

	tel = RG_TileAddPixmap(tv->tile, NULL, px, 0, 0);
	OpenElement(tv, tel);

	/* Select the newly inserted feature. */
	AG_PostEvent(NULL, tlFeatures, "tlist-poll", NULL);
	AG_TlistDeselectAll(tlFeatures);
	TAILQ_FOREACH(it, &tlFeatures->items, items) {
		RG_TileElement *tel;

		if (strcmp(it->cat, "pixmap") != 0) {
			continue;
		}
		tel = it->p1;
		if (tel->tel_pixmap.px == px) {
			AG_TlistSelect(tlFeatures, it);
			break;
		}
	}
	AG_ViewDetach(dlgWin);
}

static void
AttachPixmapDlg(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	AG_Tlist *tl;
	RG_Pixmap *px;
	AG_Window *win;
	AG_Box *bo;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOMINIMIZE);
	AG_WindowSetCaption(win, _("Attach existing pixmap"));

	tl = AG_TlistNew(win, AG_TLIST_EXPAND|AG_TLIST_FOCUS);
	AG_TlistSetItemHeight(tl, RG_TILESZ);
	AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXXXXX", 5);

	TAILQ_FOREACH(px, &tv->ts->pixmaps, pixmaps) {
		AG_TlistItem *it;

		it = AG_TlistAdd(tl, NULL, "%s (%ux%u)", px->name,
		    px->su->w, px->su->h);
		it->p1 = px;
		AG_TlistSetIcon(tl, it, px->su);
	}
	
	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"), AttachPixmap,
		    "%p,%p,%p,%p", tv, win, tlFeatures, tl);
		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}
	AG_WindowAttach(AG_WidgetParentWindow(tv), win);
	AG_WindowShow(win);
}

static void
AttachSketch(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Window *dlgWin = AG_PTR(2);
	AG_Tlist *tlFeatures = AG_PTR(3);
	AG_Tlist *tl_sketches = AG_PTR(4);
	AG_TlistItem *it;
	RG_TileElement *tel;
	RG_Sketch *sk;

	if ((it = AG_TlistSelectedItem(tl_sketches)) == NULL) {
		return;
	}
	sk = it->p1;

	tel = RG_TileAddSketch(tv->tile, NULL, sk, 0, 0);
	OpenElement(tv, tel);

	/* Select the newly inserted feature. */
	AG_PostEvent(NULL, tlFeatures, "tlist-poll", NULL);
	AG_TlistDeselectAll(tlFeatures);
	TAILQ_FOREACH(it, &tlFeatures->items, items) {
		RG_TileElement *tel;

		if (strcmp(it->cat, "sketch") != 0) {
			continue;
		}
		tel = it->p1;
		if (tel->tel_sketch.sk == sk) {
			AG_TlistSelect(tlFeatures, it);
			break;
		}
	}
	AG_ViewDetach(dlgWin);
}

static void
AttachSketchDlg(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	AG_Tlist *tl;
	RG_Sketch *sk;
	AG_Window *win;
	AG_Box *bo;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOMINIMIZE);
	AG_WindowSetCaption(win, _("Attach existing sketch"));

	tl = AG_TlistNew(win, AG_TLIST_EXPAND|AG_TLIST_FOCUS);
	AG_TlistSetItemHeight(tl, RG_TILESZ);
	AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXXXXXXXXXXX", 5);

	TAILQ_FOREACH(sk, &tv->ts->sketches, sketches) {
		AG_TlistItem *it;

		it = AG_TlistAdd(tl, NULL, "%s (%ux%u, %.0f%%)",
		    sk->name, sk->vg->su->w, sk->vg->su->h,
		    sk->vg->scale*100.0);
		it->p1 = sk;
		AG_TlistSetIcon(tl, it, sk->vg->su);
	}
	
	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"), AttachSketch, "%p,%p,%p,%p",
		    tv, win, tlFeatures, tl);
		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(AG_WidgetParentWindow(tv), win);
	AG_WindowShow(win);
}

static void
SelectFeature(AG_Tlist *tlFeatures, void *fp)
{
	RG_Feature *feat = fp;
	AG_TlistItem *eit;

	/* Select the newly inserted feature. */
	AG_PostEvent(NULL, tlFeatures, "tlist-poll", NULL);
	AG_TlistDeselectAll(tlFeatures);
	TAILQ_FOREACH(eit, &tlFeatures->items, items) {
		RG_TileElement *tel;

		if (strcmp(eit->cat, "feature") != 0) {
			continue;
		}
		tel = eit->p1;
		if (tel->tel_feature.ft == feat) {
			AG_TlistSelect(tlFeatures, eit);
			break;
		}
	}
}

static void
AddFillFeature(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	struct rg_fill_feature *fill;
	RG_TileElement *tel;

	fill = Malloc(sizeof(struct rg_fill_feature));
	RG_FillInit(fill, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, RG_FEATURE(fill), features);
	tel = RG_TileAddFeature(tv->tile, NULL, fill, 0, 0);
	OpenElement(tv, tel);
	SelectFeature(tlFeatures, fill);
}

static void
AddSketchProjFeature(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tlFeatures = AG_PTR(2);
	struct rg_sketchproj *sproj;
	RG_TileElement *tel;

	sproj = Malloc(sizeof(struct rg_sketchproj));
	RG_SketchProjInit(sproj, tv->ts, 0);
	TAILQ_INSERT_TAIL(&tv->ts->features, RG_FEATURE(sproj), features);
	tel = RG_TileAddFeature(tv->tile, NULL, sproj, 0, 0);
	OpenElement(tv, tel);
	SelectFeature(tlFeatures, sproj);
}

static void
PollFeatures(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	RG_Tile *t = AG_PTR(2);
	RG_Tileview *tv = AG_PTR(4);
	RG_TileElement *tel;
	AG_TlistItem *it;
	static char attr_names[6];			/* XXX tlist hack */

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	
	it = AG_TlistAdd(tl, NULL, _("Grid Attributes"));
	it->cat = "attributes";
	it->depth = 0;
	it->flags |= AG_TLIST_HAS_CHILDREN;
	it->p1 = &attr_names[0];

	if (AG_TlistVisibleChildren(tl, it)) {
		it = AG_TlistAdd(tl, rgIconLayerEditor.s, _("%sLayers"),
		    (tv->state==RG_TILEVIEW_LAYERS_EDIT) ? "* " : "");
		it->cat = "layers";
		it->depth = 1;
		it->p1 = &attr_names[1];
		
		it = AG_TlistAdd(tl, rgIconWalkable.s, _("%sWalkable"),
		    (tv->state==RG_TILEVIEW_ATTRIB_EDIT &&
		     tv->edit_attr == RG_TILE_BLOCK) ? "* " : "");
		it->cat = "walkable-attrs";
		it->depth = 1;
		it->p1 = &attr_names[2];
		
		it = AG_TlistAdd(tl, rgIconClimbable.s, _("%sClimbable"),
		    (tv->state==RG_TILEVIEW_ATTRIB_EDIT &&
		     tv->edit_attr == RG_TILE_CLIMBABLE) ? "* " : "");
		it->cat = "climbable-attrs";
		it->depth = 1;
		it->p1 = &attr_names[3];
		
		it = AG_TlistAdd(tl, rgIconJumpable.s, _("%sJumpable"),
		    (tv->state==RG_TILEVIEW_ATTRIB_EDIT &&
		     tv->edit_attr == RG_TILE_JUMPABLE) ? "* " : "");
		it->cat = "jumpable-attrs";
		it->depth = 1;
		it->p1 = &attr_names[4];
		
		it = AG_TlistAdd(tl, rgIconSlippery.s, _("%sSlippery"),
		    (tv->state==RG_TILEVIEW_ATTRIB_EDIT &&
		     tv->edit_attr == RG_TILE_SLIPPERY) ? "* " : "");
		it->cat = "slippery-attrs";
		it->depth = 1;
		it->p1 = &attr_names[5];
	}

	TAILQ_FOREACH(tel, &t->elements, elements) {
		if (tel->type == RG_TILE_FEATURE) {
			RG_Feature *ft = tel->tel_feature.ft;
			RG_FeatureSketch *fsk;
			RG_FeaturePixmap *fpx;
	
			it = AG_TlistAdd(tl, rgIconObject.s, "%s%s%s",
			    (tv->state==RG_TILEVIEW_FEATURE_EDIT &&
			     tv->tv_feature.ft == ft) ? "* " : "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->cat = "feature";
			it->p1 = tel;
			it->depth = 0;

			if (!TAILQ_EMPTY(&ft->sketches) ||
			    !TAILQ_EMPTY(&ft->pixmaps)) {
				it->flags |= AG_TLIST_HAS_CHILDREN;
			}
			if (!AG_TlistVisibleChildren(tl, it))
				continue;

			TAILQ_FOREACH(fsk, &ft->sketches, sketches) {
				it = AG_TlistAdd(tl, rgIconDrawing.s,
				    "%s%s%s",
				    (tv->state==RG_TILEVIEW_SKETCH_EDIT &&
				     tv->tv_sketch.sk == fsk->sk) ? "* ": "",
				    tel->name,
				    fsk->visible ? "" : _(" (invisible)"));
				it->cat = "feature-sketch";
				it->p1 = fsk;
			}

			TAILQ_FOREACH(fpx, &ft->pixmaps, pixmaps) {
				it = AG_TlistAdd(tl, rgIconPixmap.s,
				    "%s%s (%d,%d)%s",
				    (tv->state==RG_TILEVIEW_PIXMAP_EDIT &&
				     tv->tv_pixmap.px == fpx->px) ? "* ": "",
				    tel->name, fpx->x, fpx->y,
				    fpx->visible ? "" : _(" (invisible)"));
				it->cat = "feature-pixmap";
				it->p1 = fpx;
			}
		} else if (tel->type == RG_TILE_PIXMAP) {
			RG_Pixmap *px = tel->tel_pixmap.px;

			it = AG_TlistAdd(tl, NULL, "%s%s%s",
			    (tv->state==RG_TILEVIEW_PIXMAP_EDIT &&
			     tv->tv_pixmap.px == px) ? "* ": "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->cat = "pixmap";
			it->p1 = tel;
			it->depth = 0;
			AG_TlistSetIcon(tl, it, px->su);
		} else if (tel->type == RG_TILE_SKETCH) {
			RG_Sketch *sk = tel->tel_sketch.sk;
			VG *vg = sk->vg;
			VG_Element *vge;

			it = AG_TlistAdd(tl, NULL, "%s%s%s",
			    (tv->state==RG_TILEVIEW_SKETCH_EDIT &&
			     tv->tv_sketch.sk == sk) ? "* ": "",
			    tel->name,
			    tel->visible ? "" : _(" (invisible)"));
			it->cat = "sketch";
			it->p1 = tel;
			it->depth = 0;
/*			AG_TlistSetIcon(tl, it, sk->vg->su); */

			if (!TAILQ_EMPTY(&vg->vges)) {
				it->flags |= AG_TLIST_HAS_CHILDREN;
			}
			if (!AG_TlistVisibleChildren(tl, it))
				continue;

			TAILQ_FOREACH(vge, &vg->vges, vges) {
				it = AG_TlistAdd(tl,
				    vgElementTypes[vge->type]->icon->s,
				    "%s%s", (vge == vg->cur_vge) ? "* " : "",
				    vgElementTypes[vge->type]->name);
				it->cat = "sketch-element";
				it->p1 = vge;
				it->depth = 1;
			}
		}
	}

	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
EditElement(AG_Event *event)
{
	AG_Widget *sndr = AG_SELF();
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	AG_TlistItem *it;
	
	if (AG_ObjectIsClass(sndr, "AG_Widget:AG_Button:*") && !tv->edit_mode) {
		CloseElement(tv);
		return;
	}
	CloseElement(tv);
	
	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	if (strcmp(it->cat, "feature") == 0 ||
	    strcmp(it->cat, "pixmap") == 0 ||
	    strcmp(it->cat, "sketch") == 0) {
		if (it->p1 != NULL) {
			OpenElement(tv, (RG_TileElement *)it->p1);
		}
	} else if (strcmp(it->cat, "walkable-attrs") == 0) {
		tv->state = RG_TILEVIEW_ATTRIB_EDIT;
		tv->edit_mode = 1;
		tv->edit_attr = RG_TILE_BLOCK;
	} else if (strcmp(it->cat, "climbable-attrs") == 0) {
		tv->state = RG_TILEVIEW_ATTRIB_EDIT;
		tv->edit_mode = 1;
		tv->edit_attr = RG_TILE_CLIMBABLE;
	} else if (strcmp(it->cat, "jumpable-attrs") == 0) {
		tv->state = RG_TILEVIEW_ATTRIB_EDIT;
		tv->edit_mode = 1;
		tv->edit_attr = RG_TILE_JUMPABLE;
	} else if (strcmp(it->cat, "slippery-attrs") == 0) {
		tv->state = RG_TILEVIEW_ATTRIB_EDIT;
		tv->edit_mode = 1;
		tv->edit_attr = RG_TILE_SLIPPERY;
	} else if (strcmp(it->cat, "layers") == 0) {
		tv->state = RG_TILEVIEW_LAYERS_EDIT;
		tv->edit_mode = 1;
	}
}

static void
DeleteElement(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_Tile *t = tv->tile;
	AG_Tlist *tlFeatures = AG_PTR(2);
	int detach_only = AG_INT(3);
	AG_TlistItem *it;
	RG_TileElement *tel;

	if ((it = AG_TlistSelectedItem(tlFeatures)) == NULL)
		return;

	if (tv->state == RG_TILEVIEW_SKETCH_EDIT &&
	    strcmp(it->cat, "sketch-element") == 0) {
	    	VG *vg = tv->tv_sketch.sk->vg;
		VG_Element *vge = it->p1;

		RG_SketchUnselect(tv, tv->tv_sketch.tel, vge);
		VG_DestroyElement(vg, vge);
		return;
	}

	tel = it->p1;

	/* XXX check that it's the element being deleted */
	if (tv->edit_mode)
		CloseElement(tv);
	
	if (strcmp(it->cat, "feature") == 0) {
		RG_TileDelFeature(t, tel->tel_feature.ft, !detach_only);
	} else if (strcmp(it->cat, "pixmap") == 0) {
		RG_TileDelPixmap(t, tel->tel_pixmap.px, !detach_only);
	} else if (strcmp(it->cat, "sketch") == 0) {
		RG_TileDelSketch(t, tel->tel_sketch.sk, !detach_only);
	}

	t->flags |= RG_TILE_DIRTY;
}

static void
UpdateTileSettings(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_MSpinbutton *msb = AG_PTR(2);
	AG_Window *dlg_w = AG_PTR(3);
	AG_Checkbox *ckey_cb = AG_PTR(4);
	AG_Checkbox *alpha_cb = AG_PTR(5);
	AG_Spinbutton *alpha_sb = AG_PTR(6);
	RG_Tileset *ts = tv->ts;
	RG_Tile *t = tv->tile;
	int w = AG_WidgetInt(msb, "xvalue");
	int h = AG_WidgetInt(msb, "yvalue");
	Uint flags = 0;

	if (AG_WidgetBool(ckey_cb, "state"))
		flags |= RG_TILE_SRCCOLORKEY;
	if (AG_WidgetBool(alpha_cb, "state"))
		flags |= RG_TILE_SRCALPHA;

	RG_TileScale(ts, t, w, h, flags);
	t->su->format->alpha = AG_WidgetInt(alpha_sb, "value");
	RG_TileviewSetZoom(tv, 100, 0);
	AG_ViewDetach(dlg_w);

	if (tv->state == RG_TILEVIEW_TILE_EDIT) {
		RG_TileviewSetInt(tv->tv_tile.geo_ctrl, 2, w);
		RG_TileviewSetInt(tv->tv_tile.geo_ctrl, 3, h);
	}
}

static void
TileSettingsDlg(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_Tile *t = tv->tile;
	AG_Window *win;
	AG_MSpinbutton *msb;
	AG_Box *box;
	AG_Checkbox *ckey_cb, *alpha_cb;
	AG_Spinbutton *alpha_sb;
	AG_Radio *rad;
	AG_Textbox *tb;

	if ((win = AG_WindowNewNamed(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOMINIMIZE, "rg-tileinfo-%s", t->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Tile information: %s"), t->name);

	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS, _("Name: "));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, t->name, sizeof(t->name));

	tb = AG_TextboxNew(win, AG_TEXTBOX_HFILL, _("Class: "));
	AG_WidgetBind(tb, "string", AG_WIDGET_STRING, t->clname,
	    sizeof(t->clname));

	msb = AG_MSpinbuttonNew(win, 0, "x", _("Size: "));
	AG_MSpinbuttonSetRange(msb, RG_TILE_SIZE_MIN, RG_TILE_SIZE_MAX);
	AG_WidgetSetInt(msb, "xvalue", t->su->w);
	AG_WidgetSetInt(msb, "yvalue", t->su->h);
	
	alpha_sb = AG_SpinbuttonNew(win, 0, _("Overall alpha: "));
	AG_SpinbuttonSetRange(alpha_sb, 0, 255);
	AG_WidgetSetInt(alpha_sb, "value", t->su->format->alpha);
	
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	
	ckey_cb = AG_CheckboxNew(win, 0, _("Colorkeying"));
	AG_WidgetSetInt(ckey_cb, "state", t->flags & RG_TILE_SRCCOLORKEY);

	alpha_cb = AG_CheckboxNew(win, 0, _("Source alpha"));
	AG_WidgetSetInt(alpha_cb, "state", t->flags & RG_TILE_SRCALPHA);
	
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	AG_LabelNewStaticString(win, 0, _("Snapping mode: "));
	rad = AG_RadioNew(win, AG_RADIO_HFILL, rgTileSnapModes);
	AG_WidgetBind(rad, "value", AG_WIDGET_INT, &t->snap_mode);

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	box = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	{
		AG_ButtonNewFn(box, 0, _("OK"),
		    UpdateTileSettings, "%p,%p,%p,%p,%p,%p",
		    tv, msb, win, ckey_cb, alpha_cb, alpha_sb);
		AG_ButtonNewFn(box, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(AG_WidgetParentWindow(tv), win);
	AG_WindowShow(win);
}

static void
MoveElementUp(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	RG_Tile *t = tv->tile;
	AG_TlistItem *it;
	RG_TileElement *tel, *ptel;

	if ((it = AG_TlistSelectedItem(tl)) == NULL ||
	    (strcmp(it->cat, "feature") != 0 &&
	     strcmp(it->cat, "pixmap") != 0 &&
	     strcmp(it->cat, "sketch") != 0)) {
		return;
	}
	tel = it->p1;
	if (tel != TAILQ_FIRST(&t->elements)) {
		ptel = TAILQ_PREV(tel, rg_tile_elementq, elements);
		TAILQ_REMOVE(&t->elements, tel, elements);
		TAILQ_INSERT_BEFORE(ptel, tel, elements);
	}
	t->flags |= RG_TILE_DIRTY;
}

static void
MoveElementDown(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	RG_Tile *t = tv->tile;
	AG_TlistItem *it;
	RG_TileElement *tel, *ntel;

	if ((it = AG_TlistSelectedItem(tl)) == NULL ||
	    (strcmp(it->cat, "feature") != 0 &&
	     strcmp(it->cat, "pixmap") != 0 &&
	     strcmp(it->cat, "sketch") != 0)) {
		return;
	}
	tel = it->p1;
	if ((ntel = TAILQ_NEXT(tel, elements)) != NULL) {
		TAILQ_REMOVE(&t->elements, tel, elements);
		TAILQ_INSERT_AFTER(&t->elements, ntel, tel, elements);
	}
	t->flags |= RG_TILE_DIRTY;
}

static void
ToggleElementVisibility(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	RG_Tile *t = tv->tile;
	AG_TlistItem *it;
	RG_TileElement *tel;

	if ((it = AG_TlistSelectedItem(tl)) == NULL ||
	    (strcmp(it->cat, "pixmap") != 0 &&
	     strcmp(it->cat, "sketch") != 0 &&
	     strcmp(it->cat, "feature") != 0)) {
		return;
	}
	tel = it->p1;
	tel->visible = !tel->visible;
	t->flags |= RG_TILE_DIRTY;
}

static void
Undo(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapUndo(tv, tv->tv_pixmap.tel);
		break;
	default:
		break;
	}
}

static void
Redo(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);

	switch (tv->state) {
	case RG_TILEVIEW_PIXMAP_EDIT:
		RG_PixmapRedo(tv, tv->tv_pixmap.tel);
		break;
	default:
		break;
	}
}

static void
ImportBMP(AG_Event *event)
{
	RG_Tile *t = AG_PTR(1);
	RG_Tileview *tv = AG_PTR(2);
	char *path = AG_STRING(3);
	SDL_Surface *bmp;
	RG_Pixmap *px;

	if ((bmp = SDL_LoadBMP(path)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, SDL_GetError());
		return;
	}
	px = RG_PixmapNew(t->ts, "Bitmap", 0);
	px->su = SDL_ConvertSurface(bmp, t->ts->fmt, 0);
	OpenElement(tv, RG_TileAddPixmap(t, NULL, px, 0, 0));
	RG_TileGenerate(t);
	SDL_FreeSurface(bmp);

	AG_TextInfo(_("Bitmap imported successfully (%ux%u)"),
	    t->su->w, t->su->h);
}

static void
ImportImageDlg(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_Tile *t = tv->tile;
	AG_FileDlg *dlg;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Import %s image from..."), t->name);
	dlg = AG_FileDlgNewMRU(win, "rg.mru.image-export",
	    AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(dlg, _("PC bitmap"), "*.bmp",
	    ImportBMP, "%p,%p", t, tv);
	AG_WindowAttach(AG_WidgetParentWindow(tv), win);
	AG_WindowShow(win);
}

static void
ExportBMP(AG_Event *event)
{
	RG_Tile *t = AG_PTR(1);
	char *path = AG_STRING(2);

	if (SDL_SaveBMP(t->su, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, SDL_GetError());
	} else {
		AG_TextInfo(_("%s successfully exported to %s"), t->name, path);
	}
}

static void
ExportImageDlg(AG_Event *event)
{
	RG_Tileview *tv = AG_PTR(1);
	RG_Tile *t = tv->tile;
	AG_FileDlg *dlg;
	AG_Window *win;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Export %s image to..."), t->name);
	dlg = AG_FileDlgNewMRU(win, "rg.mru.image-export",
	    AG_FILEDLG_SAVE|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgSetFilename(dlg, "%s.bmp", t->name);
	AG_FileDlgAddType(dlg, _("PC bitmap"), "*.bmp", ExportBMP, "%p", t);
	AG_WindowAttach(AG_WidgetParentWindow(tv), win);
	AG_WindowShow(win);
}

static void
InitTileFeatureMenu(RG_Tileview *tv, AG_Tlist *tl, AG_Window *win)
{
	AG_MenuItem *mi;

	mi = AG_TlistSetPopup(tl, "feature");
	{
		AG_MenuAction(mi, _("Toggle visibility"), NULL,
		    ToggleElementVisibility, "%p,%p", tv, tl);
#if 0
		AG_MenuAction(mi, _("Edit feature"), NULL,
		    EditElement, "%p,%p", tv, tl);
#endif
		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Detach feature"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 1);
		AG_MenuAction(mi, _("Destroy feature"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 0);
		
		AG_MenuSeparator(mi);

		AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
		    SDLK_u, KMOD_SHIFT,
		    MoveElementUp, "%p,%p", tv, tl);
		AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
		    SDLK_d, KMOD_SHIFT,
		    MoveElementDown, "%p,%p", tv, tl);
	}

	mi = AG_TlistSetPopup(tl, "pixmap");
	{
		AG_MenuAction(mi, _("Toggle visibility"), NULL,
		    ToggleElementVisibility, "%p,%p", tv, tl);

		AG_MenuSeparator(mi);
#if 0
		AG_MenuAction(mi, _("Edit pixmap"), NULL,
		    EditElement, "%p,%p", tv, tl);
#endif
		AG_MenuAction(mi, _("Detach pixmap"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 1);
		AG_MenuAction(mi, _("Destroy pixmap"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 0);
		
		AG_MenuSeparator(mi);
		
		AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
		    SDLK_u, KMOD_SHIFT,
		    MoveElementUp, "%p,%p", tv, tl);
		AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
		    SDLK_d, KMOD_SHIFT,
		    MoveElementDown, "%p,%p", tv, tl);
	}
	
	mi = AG_TlistSetPopup(tl, "sketch");
	{
		AG_MenuAction(mi, _("Toggle visibility"), NULL,
		    ToggleElementVisibility, "%p,%p", tv, tl);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Edit sketch"), rgIconSketch.s,
		    EditElement, "%p,%p", tv, tl);
		AG_MenuAction(mi, _("Detach sketch"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 1);
		AG_MenuAction(mi, _("Destroy sketch"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 0);
		
		AG_MenuSeparator(mi);
		
		AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
		    SDLK_u, KMOD_SHIFT,
		    MoveElementUp, "%p,%p", tv, tl);
		AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
		    SDLK_d, KMOD_SHIFT,
		    MoveElementDown, "%p,%p", tv, tl);
	}
	
	mi = AG_TlistSetPopup(tl, "sketch-element");
	{
		AG_MenuAction(mi, _("Edit sketch element"), rgIconSketch.s,
		    EditElement, "%p,%p", tv, tl);
		AG_MenuAction(mi, _("Delete sketch element"), agIconTrash.s,
		    DeleteElement, "%p,%p,%i", tv, tl, 1);
#if 0
		AG_MenuSeparator(mi);
		AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
		    SDLK_u, KMOD_SHIFT,
		    move_vg_element_up, "%p,%p", tv, tl);

		AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
		    SDLK_d, KMOD_SHIFT,
		    move_vg_element_down, "%p,%p", tv, tl);
#endif
	}
}

static void
CreateView(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	RG_Tile *t = AG_PTR(2);
	AG_Window *pWin = AG_PTR(3);
	AG_Window *win;
	RG_Tileview *tv;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_CENTER, 0);
	tv = RG_TileviewNew(win, ts, RG_TILEVIEW_READONLY);
	RG_TileviewSetTile(tv, t);
	AG_WindowAttach(pWin, win);
	AG_WindowShow(win);
}

AG_Window *
RG_TileEdit(RG_Tileset *ts, RG_Tile *t)
{
	AG_Window *win;
	AG_Menu *me;
	AG_MenuItem *mi;
	RG_Tileview *tv;
	AG_Tlist *tlFeatures;
	AG_Toolbar *tbar;
	AG_Button *btn;
	AG_Pane *pane;

	if ((win = AG_WindowNewNamed(0, "tile-%s:%s", OBJECT(ts)->name,
	    t->name)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_WindowSetSpacing(win, 0);
	AG_WindowSetPaddingTop(win, 0);
	
	tv = Malloc(sizeof(RG_Tileview));
	RG_TileviewInit(tv, ts, 0);
	RG_TileviewSetTile(tv, t);
	RG_TileScale(ts, t, t->su->w, t->su->h, t->flags);
	{
		extern RG_TileviewSketchToolOps sketch_line_ops;
		extern RG_TileviewSketchToolOps sketch_polygon_ops;
		extern RG_TileviewSketchToolOps sketch_circle_ops;

		RG_TileviewRegTool(tv, &sketch_line_ops);
		RG_TileviewRegTool(tv, &sketch_polygon_ops);
		RG_TileviewRegTool(tv, &sketch_circle_ops);
	}
	
	tlFeatures = Malloc(sizeof(AG_Tlist));
	AG_TlistInit(tlFeatures, AG_TLIST_POLL|AG_TLIST_TREE|AG_TLIST_EXPAND);
	WIDGET(tlFeatures)->flags &= ~(AG_WIDGET_HFILL);
	AG_TlistSizeHint(tlFeatures, _("FEATURE #00 <#00>"), 5);
	AG_SetEvent(tlFeatures, "tlist-poll", PollFeatures, "%p,%p,%p,%p",
	    ts, t, win, tv);
	InitTileFeatureMenu(tv, tlFeatures, win);
	
	me = AG_MenuNew(win, AG_MENU_HFILL);

	tbar = Malloc(sizeof(AG_Toolbar));
	AG_ToolbarInit(tbar, AG_TOOLBAR_HORIZ, 1, 0);

	mi = AG_MenuAddItem(me, ("File"));
	{
		AG_MenuAction(mi, _("Import image file..."), agIconLoad.s,
		    ImportImageDlg, "%p", tv);
		AG_MenuAction(mi, _("Export image file..."), agIconSave.s,
		    ExportImageDlg, "%p", tv);
		
		AG_MenuSeparator(mi);
		
		AG_MenuActionKb(mi, _("Close tileset"), agIconClose.s,
		    SDLK_w, KMOD_CTRL,
		    AG_WindowCloseGenEv, "%p", win);
	}
	
	mi = AG_MenuAddItem(me, _("Edit"));
	{
		AG_MenuActionKb(mi, _("Undo"), NULL, SDLK_z, KMOD_CTRL,
		    Undo, "%p", tv);
		AG_MenuActionKb(mi, _("Redo"), NULL, SDLK_r, KMOD_CTRL,
		    Redo, "%p", tv);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Tile settings..."), rgIconPixmapResize.s,
		    TileSettingsDlg, "%p", tv);
	}

	mi = AG_MenuAddItem(me, _("Features"));
	{
		AG_MenuTool(mi, tbar, _("Fill"), rgIconFill.s,
		    SDLK_f, KMOD_CTRL|KMOD_SHIFT,
		    AddFillFeature, "%p,%p", tv, tlFeatures);
		
		AG_MenuActionKb(mi, _("Sketch projection"), rgIconSketchProj.s,
		    SDLK_s, KMOD_CTRL|KMOD_SHIFT,
		    AddSketchProjFeature, "%p,%p", tv, tlFeatures);
	}

	mi = AG_MenuAddItem(me, _("View"));
	{
		AG_MenuAction(mi, _("Create view..."), rgIconMagnifier.s,
		    CreateView, "%p,%p,%p", tv->ts, tv->tile, win);
	}

	mi = AG_MenuAddItem(me, _("Pixmaps"));
	{
		AG_MenuTool(mi, tbar, _("Create pixmap"), rgIconPixmap.s, 0, 0,
		    CreatePixmap, "%p,%p", tv, tlFeatures);
		
		AG_MenuAction(mi, _("Attach existing pixmap..."),
		    rgIconPixmapAttach.s,
		    AttachPixmapDlg, "%p,%p", tv, tlFeatures);
	}
	
	mi = AG_MenuAddItem(me, _("Sketches"));
	{
		AG_MenuTool(mi, tbar, _("Create sketch..."), rgIconSketch.s,
		    0, 0,
		    CreateSketch, "%p,%p", tv, tlFeatures);
		
		AG_MenuAction(mi, _("Attach sketch..."), rgIconSketchAttach.s,
		    AttachSketchDlg, "%p,%p", tv, tlFeatures);

		/* TODO import */
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	{
		AG_ObjectAttach(pane->div[0], tlFeatures);
		WIDGET(tlFeatures)->flags |= AG_WIDGET_HFILL;
	
		btn = AG_ButtonNew(pane->div[0],
		    AG_BUTTON_STICKY|AG_BUTTON_HFILL,
		    _("Edit"));
		WIDGET(btn)->flags |= AG_WIDGET_HFILL;
		AG_WidgetBind(btn, "state", AG_WIDGET_INT, &tv->edit_mode);
		AG_SetEvent(btn, "button-pushed",
		    EditElement, "%p,%p", tv, tlFeatures);
		AG_SetEvent(tlFeatures, "tlist-dblclick",
		    EditElement, "%p,%p", tv, tlFeatures);

		AG_ObjectAttach(pane->div[1], tbar);

		tv->tel_box = AG_BoxNew(pane->div[1], AG_BOX_HORIZ,
		    AG_BOX_EXPAND);
		AG_ObjectAttach(tv->tel_box, tv);
		AG_WidgetFocus(tv);
	}

	/* Set the tile edition mode. */
	CloseElement(tv);

	AG_WindowSetGeometry(win,
	    agView->w/4, agView->h/4,
	    agView->w/2, agView->h/2);

	/* Center the tile. */
	tv->xoffs = (WIDGET(tv)->w - t->su->w)/2;
	tv->yoffs = (WIDGET(tv)->h - t->su->h)/2;

	return (win);
}

void
RG_TileOpenMenu(RG_Tileview *tv, int x, int y)
{
	if (tv->menu != NULL)
		RG_TileCloseMenu(tv);

	tv->menu = Malloc(sizeof(AG_Menu));
	AG_MenuInit(tv->menu, 0);

	tv->menu_item = AG_MenuAddItem(tv->menu, NULL);
	{
		RG_TileviewGenericMenu(tv, tv->menu_item);
	}
	tv->menu->itemSel = tv->menu_item;
	tv->menu_win = AG_MenuExpand(tv->menu, tv->menu_item, x, y);
}

void
RG_TileCloseMenu(RG_Tileview *tv)
{
	AG_MenuCollapse(tv->menu, tv->menu_item);
	AG_ObjectDestroy(tv->menu);
	Free(tv->menu);

	tv->menu = NULL;
	tv->menu_item = NULL;
	tv->menu_win = NULL;
}

#endif /* EDITION */
