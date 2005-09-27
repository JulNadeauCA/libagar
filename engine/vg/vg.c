/*	$Csoft: vg.c,v 1.70 2005/09/27 00:25:20 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef EDITION
#include <engine/map/mapview.h>
#include <engine/map/tool.h>

#include <engine/widget/toolbar.h>
#include <engine/widget/button.h>
#include <engine/widget/combo.h>
#include <engine/widget/tlist.h>
#include <engine/widget/menu.h>
#endif

#include "vg.h"
#include "vg_primitive.h"

#include <engine/loader/vertex.h>

const AG_Version vgVer = {
	"agar vg",
	4, 0
};

extern const VG_ElementOps vgPointsOps;
extern const VG_ElementOps vgLinesOps;
extern const VG_ElementOps vgLineStripOps;
extern const VG_ElementOps vgLineLoopOps;
extern const VG_ElementOps vgCircleOps;
extern const VG_ElementOps vgArcOps;
extern const VG_ElementOps vgEllipseOps;
extern const VG_ElementOps vgTextOps;
extern const VG_ElementOps vgMaskOps;
extern const VG_ElementOps vgPolygonOps;

const VG_ElementOps *vgElementTypes[] = {
	&vgPointsOps,
	&vgLinesOps,
	&vgLineStripOps,
	&vgLineLoopOps,
	NULL,			/* triangles */
	NULL,			/* triangle strip */
	NULL,			/* triangle fan */
	NULL,			/* quads */
	NULL,			/* quad strip */
	&vgPolygonOps,
	&vgCircleOps,
	&vgArcOps,
	&vgEllipseOps,
	NULL,			/* Bezier curve */
	NULL,			/* Bezigon */
	&vgTextOps,
	&vgMaskOps,
};

VG *
VG_New(void *p, int flags)
{
	char path[AG_OBJECT_PATH_MAX];
	AG_Object *ob = p;
	VG *vg;
	
	vg = Malloc(sizeof(VG), M_VG);
	VG_Init(vg, flags);
	if (ob != NULL) {
		ob->gfx = AG_GfxNew(ob);
		ob->gfx->used = 1;

		vg->pobj = ob;
		vg->map = AG_MapNew(ob, "raster");
		AGOBJECT(vg->map)->flags |= AG_OBJECT_NON_PERSISTENT|
					     AG_OBJECT_INDESTRUCTIBLE;
	}
	return (vg);
}

void
VG_Init(VG *vg, int flags)
{
	Uint32 sflags = SDL_SWSURFACE|SDL_RLEACCEL;
	int i, x, y;

	if (flags & VG_ALPHA)		sflags |= SDL_SRCALPHA;
	if (flags & VG_COLORKEY)	sflags |= SDL_SRCCOLORKEY;
	if (flags & VG_RLEACCEL)	sflags |= SDL_RLEACCEL;

	strlcpy(vg->name, _("Untitled"), sizeof(vg->name));
	vg->flags = flags;
	vg->redraw = 1;
	vg->w = 0;
	vg->h = 0;
	vg->scale = 1;
	vg->su = SDL_CreateRGBSurface(sflags, 16, 16, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    (flags & VG_ALPHA) ? 0x000000ff : 0x0
#else
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    (flags & VG_ALPHA) ? 0xff000000 : 0x0
#endif
	);
	if (vg->su == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	vg->fmt = vg->su->format;
	vg->fill_color = SDL_MapRGBA(vg->fmt, 0, 0, 0, 0);
	vg->grid_color = SDL_MapRGB(vg->fmt, 128, 128, 128);
	vg->selection_color = SDL_MapRGB(vg->fmt, 255, 255, 0);
	vg->mouseover_color = SDL_MapRGB(vg->fmt, 200, 200, 0);
	vg->grid_gap = 0.25;
	vg->origin = Malloc(sizeof(VG_Vtx)*VG_NORIGINS, M_VG);
	vg->origin_radius = Malloc(sizeof(float)*VG_NORIGINS, M_VG);
	vg->origin_color = Malloc(sizeof(Uint32)*VG_NORIGINS, M_VG);
	vg->pobj = NULL;
	vg->map = NULL;
	vg->layers = Malloc(sizeof(VG_Layer), M_VG);
	vg->nlayers = 0;
	vg->cur_layer = 0;
	vg->cur_block = NULL;
	vg->cur_vge = NULL;
	VG_PushLayer(vg, _("Layer 0"));
	vg->snap_mode = VG_GRID;
	vg->ortho_mode = VG_NO_ORTHO;
	TAILQ_INIT(&vg->vges);
	TAILQ_INIT(&vg->blocks);
	TAILQ_INIT(&vg->styles);
	pthread_mutex_init(&vg->lock, &agRecursiveMutexAttr);

	for (i = 0; i < VG_NORIGINS; i++) {
		vg->origin[i].x = 0;
		vg->origin[i].y = 0;
		vg->origin[i].z = 0;
		vg->origin[i].w = 1.0;
		vg->origin_radius[i] = 0.0625;
		vg->origin_color[i] = SDL_MapRGB(vg->fmt, 0, 0, 180);
	}
	vg->origin_radius[0] = 0.25;
	vg->origin_radius[1] = 0.125;
	vg->origin_radius[2] = 0.075;
	vg->origin_color[0] = SDL_MapRGB(vg->fmt, 0, 200, 0);
	vg->origin_color[1] = SDL_MapRGB(vg->fmt, 0, 150, 0);
	vg->origin_color[2] = SDL_MapRGB(vg->fmt, 0, 80, 150);
	vg->norigin = VG_NORIGINS;

	vg->ints = NULL;
	vg->nints = 0;
}

void
VG_FreeElement(VG *vg, VG_Element *vge)
{
	if (vge->ops->destroy != NULL) {
		vge->ops->destroy(vg, vge);
	}
	Free(vge->vtx, M_VG);
	Free(vge, M_VG);
}

static void
VG_DestroyElements(VG *vg)
{
	VG_Element *vge, *nvge;
	
	for (vge = TAILQ_FIRST(&vg->vges);
	     vge != TAILQ_END(&vg->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vges);
		VG_FreeElement(vg, vge);
	}
	TAILQ_INIT(&vg->vges);
}

static void
VG_DestroyBlocks(VG *vg)
{
	VG_Block *vgb, *nvgb;

	for (vgb = TAILQ_FIRST(&vg->blocks);
	     vgb != TAILQ_END(&vg->blocks);
	     vgb = nvgb) {
		nvgb = TAILQ_NEXT(vgb, vgbs);
		Free(vgb, M_VG);
	}
	TAILQ_INIT(&vg->blocks);
}

static void
VG_DestroyStyles(VG *vg)
{
	VG_Style *st, *nst;

	for (st = TAILQ_FIRST(&vg->styles);
	     st != TAILQ_END(&vg->styles);
	     st = nst) {
		nst = TAILQ_NEXT(st, styles);
		Free(st, M_VG);
	}
	TAILQ_INIT(&vg->styles);
}

void
VG_Reinit(VG *vg)
{
	VG_DestroyBlocks(vg);
	VG_DestroyElements(vg);
	VG_DestroyStyles(vg);
}

void
VG_Destroy(VG *vg)
{
	AG_Object *ob = vg->pobj;
	int y;

	Free(vg->origin, M_VG);
	Free(vg->origin_radius, M_VG);
	Free(vg->origin_color, M_VG);

	if (ob != NULL && ob->gfx != NULL) {
		AG_GfxDestroy(ob->gfx);
		ob->gfx = NULL;
	}
	Free(vg->layers, M_VG);

	VG_DestroyBlocks(vg);
	VG_DestroyElements(vg);
	VG_DestroyStyles(vg);
	pthread_mutex_destroy(&vg->lock);

	Free(vg->ints, M_VG);
}

void
VG_DestroyElement(VG *vg, VG_Element *vge)
{
	if (vge->block != NULL)
		TAILQ_REMOVE(&vge->block->vges, vge, vgbmbs);

	if (vg->cur_vge == vge)
		vg->cur_vge = NULL;

	TAILQ_REMOVE(&vg->vges, vge, vges);
	VG_FreeElement(vg, vge);
	vg->redraw++;
}

/*
 * Generate tile-sized fragments of the raster surface.
 * The vg must be tied to an object.
 */
void
VG_UpdateFragments(VG *vg)
{
	AG_Object *pobj = vg->pobj;
	AG_Nitem *r;
	int x, y;
	int mx, my;
	SDL_Rect sd, rd;
	Uint32 saflags = vg->su->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
	Uint32 scflags = vg->su->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
	Uint8 salpha = vg->su->format->alpha;
	Uint32 scolorkey = vg->su->format->colorkey;

	rd.x = 0;
	rd.y = 0;
	sd.w = AGTILESZ;
	sd.h = AGTILESZ;
	
	SDL_SetAlpha(vg->su, 0, 0);
	SDL_SetColorKey(vg->su, 0, 0);
	
	for (y = 0, my = 0;
	     y < vg->su->h && my < vg->map->maph;
	     y += AGTILESZ, my++) {
		for (x = 0, mx = 0;
		     x < vg->su->w && mx < vg->map->mapw;
		     x += AGTILESZ, mx++) {
			AG_Node *n = &vg->map->map[my][mx];
			SDL_Surface *fragsu = NULL;
			int fw, fh;

			fw = vg->su->w-x < AGTILESZ ? vg->su->w-x : AGTILESZ;
			fh = vg->su->h-y < AGTILESZ ? vg->su->h-y : AGTILESZ;
#ifdef DEBUG
			if (fw <= 0 || fh <= 0)
				fatal("fragment too small");
#endif
			TAILQ_FOREACH(r, &n->nrefs, nrefs) {
				if (r->type == AG_NITEM_SPRITE &&
				    r->layer == vg->map->cur_layer &&
				    r->r_sprite.obj == pobj) {
					fragsu = AG_SPRITE(pobj,
					                r->r_sprite.offs).su;
					break;
				}
			}
			if (r == NULL) {
				fragsu = SDL_CreateRGBSurface(SDL_SWSURFACE|
				    saflags|scflags, fw, fh,
				    vg->fmt->BitsPerPixel,
				    vg->fmt->Rmask,
				    vg->fmt->Gmask,
				    vg->fmt->Bmask,
				    vg->fmt->Amask);
			}
			if (fragsu == NULL)
				continue;
			
			sd.x = x;
			sd.y = y;

			SDL_BlitSurface(vg->su, &sd, fragsu, &rd);

			if (r == NULL) {
				Uint32 sp;

				sp = AG_GfxAddSprite(pobj->gfx, fragsu);
				AG_SPRITE(pobj,sp).snap_mode =
				    AG_GFX_SNAP_TO_GRID;
				AG_NodeAddSprite(vg->map, n, pobj, sp);
			}
		}
	}
	vg->map->origin.x = vg->map->mapw>>1;
	vg->map->origin.y = vg->map->maph>>1;

	SDL_SetAlpha(vg->su, saflags, salpha);
	SDL_SetColorKey(vg->su, scflags, scolorkey);
}

/*
 * Release the raster fragments.
 * The vg must be tied to an object.
 */
void
VG_FreeFragments(VG *vg)
{
	AG_Gfx *gfx = vg->pobj->gfx;
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		AG_SpriteDestroy(gfx, i);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		AG_ObjectDestroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}
	gfx->nsprites = 0;
	gfx->nsubmaps = 0;

	if (vg->map != NULL)
		AG_MapFreeNodes(vg->map);
}

/* Set the default scale factor. */
void
VG_DefaultScale(VG *vg, double scale)
{
	vg->default_scale = scale;
}

/* Set the default scale factor. */
void
VG_SetGridGap(VG *vg, double gap)
{
	vg->grid_gap = gap;
}

/* Adjust the vg bounding box and scaling factor. */
void
VG_Scale(VG *vg, double w, double h, double scale)
{
	int pw = (int)(w*scale*AGTILESZ);
	int ph = (int)(h*scale*AGTILESZ);
	int mw, mh, y;
	Uint32 Rmask = vg->fmt->Rmask;
	Uint32 Gmask = vg->fmt->Gmask;
	Uint32 Bmask = vg->fmt->Bmask;
	Uint32 Amask = vg->fmt->Amask;
	int depth = vg->fmt->BitsPerPixel;
	Uint32 colorkey = vg->fmt->colorkey;
	Uint8 alpha = vg->fmt->alpha;
	Uint32 sFlags = vg->su->flags & (SDL_SWSURFACE|SDL_SRCALPHA|
	                                 SDL_SRCCOLORKEY|SDL_RLEACCEL);

#ifdef DEBUG
	if (scale < 0.0)
		fatal("neg scale");
#endif

	SDL_FreeSurface(vg->su);

	vg->scale = scale;
	vg->w = w;
	vg->h = h;

	if ((vg->su = SDL_CreateRGBSurface(sFlags, pw, ph, depth,
	    Rmask, Gmask, Bmask, Amask)) == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	vg->fmt = vg->su->format;

	/* Resize the fragment map. */
	if (vg->pobj != NULL) {
		VG_FreeFragments(vg);
		mw = vg->su->w/AGTILESZ+1;
		mh = vg->su->h/AGTILESZ+1;
		if (AG_MapAllocNodes(vg->map, mw, mh) == -1)
			fatal("%s", AG_GetError());
	}
	vg->redraw++;
}

/*
 * Allocate the given type of element and begin its parametrization.
 * If a block is selected, associate the element with it.
 */
VG_Element *
VG_Begin(VG *vg, enum vg_element_type eltype)
{
	VG_Element *vge;
	int i;

	vge = Malloc(sizeof(VG_Element), M_VG);
	vge->flags = 0;
	vge->type = eltype;
	vge->style = NULL;
	vge->layer = vg->cur_layer;
	vge->drawn = 0;
	vge->selected = 0;
	vge->mouseover = 0;
	vge->vtx = NULL;
	vge->nvtx = 0;
	vge->color = SDL_MapRGB(vg->fmt, 250, 250, 250);

	vge->line_st.style = VG_CONTINUOUS;
	vge->line_st.endpoint_style = VG_SQUARE;
	vge->line_st.stipple = 0x0;
	vge->line_st.thickness = 1;
	vge->line_st.miter_len = 0;
	vge->text_st.face[0] = '\0';
	vge->text_st.size = 12;
	vge->text_st.flags = 0;
	vge->fill_st.style = VG_SOLID;
	vge->fill_st.texture[0] = '\0';
	vge->fill_st.texture_alpha = 255;

	pthread_mutex_lock(&vg->lock);
	TAILQ_INSERT_TAIL(&vg->vges, vge, vges);

	if (vg->cur_block != NULL) {
		TAILQ_INSERT_TAIL(&vg->cur_block->vges, vge, vgbmbs);
		vge->block = vg->cur_block;
		if (vge->block->flags & VG_BLOCK_NOSAVE)
			vge->flags |= VG_ELEMENT_NOSAVE;
	} else {
		vge->block = NULL;
	}

	vge->ops = vgElementTypes[eltype];
	if (vge->ops->init != NULL)
		vge->ops->init(vg, vge);

	vg->cur_vge = vge;
	vg->redraw++;
	pthread_mutex_unlock(&vg->lock);
	return (vge);
}

/*
 * End the parametrization of the current element.
 * The vg must be locked.
 */
void
VG_End(VG *vg)
{
	vg->cur_vge = NULL;
}

/*
 * Select the given element for edition.
 * The vg must be locked.
 */
void
VG_Select(VG *vg, VG_Element *vge)
{
	vg->cur_vge = vge;
}

#ifdef DEBUG
void
VG_DrawExtents(VG *vg)
{
	VG_Rect bbox;
	VG_Element *vge;
	VG_Block *vgb;
	int x, y, w, h;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->ops->bbox != NULL) {
			vge->ops->bbox(vg, vge, &bbox);
		} else {
			continue;
		}
		VG_Rcoords2(vg, bbox.x, bbox.y, &x, &y);
		VG_RLength(vg, bbox.w, &w);
		VG_RLength(vg, bbox.h, &h);
		VG_RectPrimitive(vg, x-1, y-1, w+2, h+2, vg->grid_color);
	}
	
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		Uint32 ext_color = SDL_MapRGB(vg->fmt, 0, 250, 0);

		VG_BlockExtent(vg, vgb, &bbox);
		VG_Rcoords2(vg, bbox.x, bbox.y, &x, &y);
		VG_RLength(vg, bbox.w, &w);
		VG_RLength(vg, bbox.h, &h);
		VG_RectPrimitive(vg, x-1, y-1, w+2, h+2, ext_color);
	}
}
#endif /* DEBUG */

/* Evaluate the intersection between two rectangles. */
int
VG_Rintersect(VG *vg, VG_Rect *r1, VG_Rect *r2,
    VG_Rect *ixion)
{
	double r1xmin, r1xmax, r1ymin, r1ymax;
	double r2xmin, r2xmax, r2ymin, r2ymax;
	double ixw, ixh;

	r1xmin = r1->x;
	r1ymin = r1->y;
	r1xmax = r1->x+r1->w;
	r1ymax = r1->y+r1->h;

	r2xmin = r2->x;
	r2ymin = r2->y;
	r2xmax = r2->x+r2->w;
	r2ymax = r2->y+r2->h;

	if (r2xmin > r1xmin)
		r1xmin = r2xmin;
	if (r2ymin > r1ymin)
		r1ymin = r2ymin;

	if (r2xmax < r1xmax)
		r1xmax = r2xmax;
	if (r2ymax < r1ymax)
		r1ymax = r2ymax;
	
	ixw = r1xmax - (r1xmin > 0 ? r1xmax-r1xmin : 0);
	ixh = r1ymax - (r1ymin > 0 ? r1ymax-r1ymin : 0);
	if (ixion != NULL) {
		ixion->x = r1xmin;
		ixion->y = r1ymin;
		ixion->w = ixw;
		ixion->h = ixh;
	}
	return (ixw > 0 && ixh > 0);
}

/*
 * Rasterize an element as well as other overlapping elements.
 * The vg must be locked.
 */
void
VG_RasterizeElement(VG *vg, VG_Element *vge)
{
	VG_Element *ovge;
	VG_Rect r1, r2;
	Uint32 color_save = 0;			/* XXX -Wuninitialized */

	if (!vge->drawn) {
		if (vge->mouseover) {
			Uint8 r, g, b;

			color_save = vge->color;
			SDL_GetRGB(vge->color, vg->fmt, &r, &g, &b);
			if (r > 200 && g > 200 && b > 200) {
				r = 0;
				g = 255;
				b = 0;
			} else {
				r = MIN(r+50,255);
				g = MIN(g+50,255);
				b = MIN(b+50,255);
			}
			vge->color = SDL_MapRGB(vg->fmt, r, g, b);
		}

		vge->ops->draw(vg, vge);
		vge->drawn = 1;
		
		if (vge->mouseover)
			vge->color = color_save;
	}
	if (vge->ops->bbox != NULL) {
		vge->ops->bbox(vg, vge, &r1);
		TAILQ_FOREACH(ovge, &vg->vges, vges) {
			if (ovge->drawn || ovge == vge ||
			    ovge->ops->bbox == NULL) {
				continue;
			}
			ovge->ops->bbox(vg, ovge, &r2);
			if (VG_Rintersect(vg, &r1, &r2, NULL))
				VG_RasterizeElement(vg, ovge);
		}
	}
}

/* Rasterize elements marked dirty and update the affected tiles. */
void
VG_Rasterize(VG *vg)
{
	VG_Element *vge;
	VG_Block *vgb;
	int i;

	pthread_mutex_lock(&vg->lock);

	SDL_FillRect(vg->su, NULL, vg->fill_color);

	if (vg->flags & VG_VISGRID)
		VG_DrawGrid(vg);
#ifdef DEBUG
	if (vg->flags & VG_VISBBOXES)
		VG_DrawExtents(vg);
#endif
	
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vge->drawn = 0;
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		VG_RasterizeElement(vg, vge);
	}
	if (vg->flags & VG_VISORIGIN)
		VG_DrawOrigin(vg);

	if (vg->pobj != NULL)
		VG_UpdateFragments(vg);

	vg->redraw = 0;
	pthread_mutex_unlock(&vg->lock);
}

/*
 * Translate a length in pixel to a vg vector magnitude.
 * The vg must be locked.
 */
void
VG_VLength(VG *vg, int len, double *vlen)
{
	*vlen = (double)(len/vg->scale/AGTILESZ);
}

/*
 * Translate tile coordinates to relative vg coordinates, applying
 * positional and orthogonal restrictions as needed.
 * The vg must be locked.
 */
void
VG_Vcoords2(VG *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/AGTILESZ -
	    vg->origin[0].x;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/AGTILESZ -
	    vg->origin[0].y;
	
	if (vg->snap_mode != VG_FREE_POSITIONING)
		VG_SnapPoint(vg, vx, vy);
	if (vg->ortho_mode != VG_NO_ORTHO)
		VG_RestrictOrtho(vg, vx, vy);
}

int
VG_Map2Vec(VG *vg, int rx, int ry, double *vx, double *vy)
{
	*vx = (double)rx/vg->scale/AGTILESZ - vg->origin[0].x;
	*vy = (double)ry/vg->scale/AGTILESZ - vg->origin[0].y;
	
	if (vg->snap_mode != VG_FREE_POSITIONING)
		VG_SnapPoint(vg, vx, vy);
	if (vg->ortho_mode != VG_NO_ORTHO)
		VG_RestrictOrtho(vg, vx, vy);
	
	return ((rx < 0 || ry < 0) ? -1 : 0);
}

/*
 * Translate map coordinates to absolute vg coordinates.
 * The vg must be locked.
 */
int
VG_Map2VecAbs(VG *vg, int rx, int ry, double *vx, double *vy)
{
	*vx = (double)rx/vg->scale/AGTILESZ;
	*vy = (double)ry/vg->scale/AGTILESZ;

	if (vg->snap_mode != VG_FREE_POSITIONING)
		VG_SnapPoint(vg, vx, vy);

	return ((rx < 0 || ry < 0) ? -1 : 0);
}

/*
 * Translate tile coordinates to absolute vg coordinates.
 * The vg must be locked.
 */
void
VG_AbsVcoords2(VG *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/AGTILESZ;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/AGTILESZ;

	if (vg->snap_mode != VG_FREE_POSITIONING)
		VG_SnapPoint(vg, vx, vy);
}

/*
 * Translate relative vg coordinates to raster coordinates.
 * The vg must be locked.
 */
void
VG_Rcoords2(VG *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*AGTILESZ) +
	      (int)(vg->origin[0].x*vg->scale*AGTILESZ);
	*ry = (int)(vy*vg->scale*AGTILESZ) +
	      (int)(vg->origin[0].y*vg->scale*AGTILESZ);
}

/*
 * Translate absolute vg coordinates to integer raster coordinates.
 * The vg must be locked.
 */
void
VG_AbsRcoords2(VG *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*AGTILESZ);
	*ry = (int)(vy*vg->scale*AGTILESZ);
}

/*
 * Translate relative vg coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
VG_Rcoords2d(VG *vg, double vx, double vy, double *rx, double *ry)
{
	*rx = vx*vg->scale*AGTILESZ +
	      vg->origin[0].x*vg->scale*AGTILESZ;
	*ry = vy*vg->scale*AGTILESZ +
	      vg->origin[0].y*vg->scale*AGTILESZ;
}

/*
 * Translate vertex coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
VG_VtxCoords2d(VG *vg, VG_Vtx *vtx, double *rx, double *ry)
{
	if (rx != NULL)	*rx = VG_RASXF(vg,vtx->x);
	if (ry != NULL)	*ry = VG_RASYF(vg,vtx->y);
}

/*
 * Translate vertex coordinates to integral raster coordinates.
 * The vg must be locked.
 */
void
VG_VtxCoords2i(VG *vg, VG_Vtx *vtx, int *rx, int *ry)
{
	if (rx != NULL)	*rx = VG_RASX(vg,vtx->x);
	if (ry != NULL) *ry = VG_RASY(vg,vtx->y);
}

/*
 * Translate absolute vg coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
VG_AbsRcoords2d(VG *vg, double vx, double vy, double *rx, double *ry)
{
	*rx = vx*vg->scale*AGTILESZ;
	*ry = vy*vg->scale*AGTILESZ;
}

/*
 * Translate the magnitude of a vg vector to the raster equivalent in pixels.
 * The vg must be locked.
 */
void
VG_RLength(VG *vg, double len, int *rlen)
{
	*rlen = (int)(len*vg->scale*AGTILESZ);
}

VG_Vtx *
VG_AllocVertex(VG_Element *vge)
{
	if (vge->vtx == NULL) {
		vge->vtx = Malloc(sizeof(VG_Vtx), M_VG);
	} else {
		vge->vtx = Realloc(vge->vtx, (vge->nvtx+1)*sizeof(VG_Vtx));
	}
	return (&vge->vtx[vge->nvtx++]);
}

/* Pop the highest vertex off the vertex array. */
VG_Vtx *
VG_PopVertex(VG *vg)
{
	VG_Element *vge = vg->cur_vge;

	if (vge->vtx == NULL)
		return (NULL);
#ifdef DEBUG
	if (vge->nvtx-1 < 0)
		fatal("neg nvtx");
#endif
	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(VG_Vtx));
	vg->redraw++;
	return (vge->nvtx > 0 ? &vge->vtx[vge->nvtx-1] : NULL);
}

/* Push a 2D vertex onto the vertex array. */
VG_Vtx *
VG_Vertex2(VG *vg, double x, double y)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = 0;
	vtx->w = 1.0;
	VG_BlockOffset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a 3D vertex onto the vertex array. */
VG_Vtx *
VG_Vertex3(VG *vg, double x, double y, double z)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = 1.0;
	VG_BlockOffset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a homogenized 3D vertex onto the vertex array. */
VG_Vtx *
VG_Vertex4(VG *vg, double x, double y, double z, double w)
{
	VG_Vtx *vtx;

	vtx = VG_AllocVertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = w;
	VG_BlockOffset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a series of vertices onto the vertex array. */
void
VG_VertexV(VG *vg, const VG_Vtx *svtx, u_int nsvtx)
{
	VG_Element *vge = vg->cur_vge;
	u_int i;
	
	for (i = 0; i < nsvtx; i++) {
		VG_Vtx *vtx;

		vtx = VG_AllocVertex(vge);
		memcpy(vtx, &svtx[i], sizeof(VG_Vtx));
		VG_BlockOffset(vg, vtx);
	}
	vg->redraw++;
}

/* Create a new global style. */
VG_Style *
VG_CreateStyle(VG *vg, enum vg_style_type type, const char *name)
{
	VG_Style *vgs;

	vgs = Malloc(sizeof(VG_Style), M_VG);
	strlcpy(vgs->name, name, sizeof(vgs->name));
	vgs->type = type;
	vgs->color = SDL_MapRGB(vg->fmt, 250, 250, 250);
	switch (vgs->type) {
	case VG_LINE_STYLE:
		vgs->vg_line_st.style = VG_CONTINUOUS;
		vgs->vg_line_st.endpoint_style = VG_SQUARE;
		vgs->vg_line_st.stipple = 0x0;
		vgs->vg_line_st.thickness = 1;
		vgs->vg_line_st.miter_len = 0;
		break;
	case VG_FILL_STYLE:
		vgs->vg_fill_st.style = VG_SOLID;
		vgs->vg_fill_st.texture[0] = '\0';
		vgs->vg_fill_st.texture_alpha = 255;
		break;
	case VG_TEXT_STYLE:
		vgs->vg_text_st.face[0] = '\0';
		vgs->vg_text_st.size = 12;
		vgs->vg_text_st.flags = 0;
		break;
	}
	TAILQ_INSERT_TAIL(&vg->styles, vgs, styles);
	return (vgs);
}

/* Associate the given style with the current element. */
int
VG_SetStyle(VG *vg, const char *name)
{
	VG_Element *vge = vg->cur_vge;
	VG_Style *st;

	TAILQ_FOREACH(st, &vg->styles, styles) {
		if (strcmp(st->name, name) == 0)
			break;
	}
	if (st == NULL) {
		return (-1);
	}
	switch (st->type) {
	case VG_LINE_STYLE:
		memcpy(&vge->line_st, &st->vg_line_st,
		    sizeof(VG_LineStyle));
		break;
	case VG_TEXT_STYLE:
		memcpy(&vge->text_st, &st->vg_text_st,
		    sizeof(VG_TextStyle));
		break;
	case VG_FILL_STYLE:
		memcpy(&vge->fill_st, &st->vg_fill_st,
		    sizeof(VG_FillingStyle));
		break;
	}
	return (0);
}

/* Specify the layer# to associate with the current element. */
void
VG_SetLayer(VG *vg, int layer)
{
	vg->cur_vge->layer = layer;
}

/* Specify the color of the current element (format-specific). */
void
VG_Color(VG *vg, Uint32 color)
{
	vg->cur_vge->color = color;
	vg->redraw++;
}

/* Specify the color of the current element (RGB triplet). */
void
VG_Color3(VG *vg, int r, int g, int b)
{
	vg->cur_vge->color = SDL_MapRGB(vg->fmt, r, g, b);
	vg->redraw++;
}

/* Specify the color of the current element (RGB triplet + alpha). */
void
VG_Color4(VG *vg, int r, int g, int b, int a)
{
	vg->cur_vge->color = SDL_MapRGBA(vg->fmt, r, g, b, a);
	vg->redraw++;
}

/* Push a new layer onto the layer stack. */
VG_Layer *
VG_PushLayer(VG *vg, const char *name)
{
	VG_Layer *vgl;

	vg->layers = Realloc(vg->layers, (vg->nlayers+1) *
	                                 sizeof(VG_Layer));
	vgl = &vg->layers[vg->nlayers];
	vg->nlayers++;

	strlcpy(vgl->name, name, sizeof(vgl->name));
	vgl->visible = 1;
	vgl->alpha = 255;
	vgl->color = SDL_MapRGB(vg->fmt, 255, 255, 255);
	
	vg->redraw++;
	return (vgl);
}

/* Pop the highest layer off the layer stack. */
void
VG_PopLayer(VG *vg)
{
	if (--vg->nlayers < 1)
		vg->nlayers = 1;
	
	vg->redraw++;
}

void
VG_Save(VG *vg, AG_Netbuf *buf)
{
	off_t nblocks_offs, nvges_offs, nstyles_offs;
	Uint32 nblocks = 0, nvges = 0, nstyles = 0;
	VG_Block *vgb;
	VG_Style *vgs;
	VG_Element *vge;
	int i;

	AG_WriteVersion(buf, &vgVer);

	pthread_mutex_lock(&vg->lock);

	AG_WriteString(buf, vg->name);
	AG_WriteUint32(buf, (Uint32)vg->flags);
	AG_WriteDouble(buf, vg->w);
	AG_WriteDouble(buf, vg->h);
	AG_WriteDouble(buf, vg->scale);
	AG_WriteDouble(buf, vg->default_scale);
	AG_WriteColor(buf, vg->fmt, vg->fill_color);
	AG_WriteColor(buf, vg->fmt, vg->grid_color);
	AG_WriteColor(buf, vg->fmt, vg->selection_color);
	AG_WriteColor(buf, vg->fmt, vg->mouseover_color);
	AG_WriteDouble(buf, vg->grid_gap);
	AG_WriteUint32(buf, (Uint32)vg->cur_layer);

	/* Save the origin points. */
	AG_WriteUint32(buf, vg->norigin);
	for (i = 0; i < vg->norigin; i++) {
		AG_WriteVertex(buf, &vg->origin[i]);
		AG_WriteFloat(buf, vg->origin_radius[i]);
		AG_WriteColor(buf, vg->fmt, vg->origin_color[i]);
	}

	/* Save the layer information. */
	AG_WriteUint32(buf, vg->nlayers);
	for (i = 0; i < vg->nlayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_WriteString(buf, layer->name);
		AG_WriteUint8(buf, (Uint8)layer->visible);
		AG_WriteColor(buf, vg->fmt, layer->color);
		AG_WriteUint8(buf, layer->alpha);
	}

	/* Save the block information. */
	nblocks_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		if (vgb->flags & VG_BLOCK_NOSAVE)
			continue;

		AG_WriteString(buf, vgb->name);
		AG_WriteUint32(buf, (Uint32)vgb->flags);
		AG_WriteVertex(buf, &vgb->pos);
		AG_WriteVertex(buf, &vgb->origin);
		AG_WriteDouble(buf, vgb->theta);
		nblocks++;
	}
	AG_PwriteUint32(buf, nblocks, nblocks_offs);

	/* Save the global style information. */
	nstyles_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		AG_WriteString(buf, vgs->name);
		AG_WriteUint8(buf, (Uint8)vgs->type);
		AG_WriteColor(buf, vg->fmt, vgs->color);

		switch (vgs->type) {
		case VG_LINE_STYLE:
			AG_WriteUint8(buf, (Uint8)vgs->vg_line_st.style);
			AG_WriteUint8(buf, (Uint8)vgs->vg_line_st.endpoint_style);
			AG_WriteUint16(buf, vgs->vg_line_st.stipple);
			AG_WriteUint8(buf, vgs->vg_line_st.thickness);
			AG_WriteUint8(buf, vgs->vg_line_st.miter_len);
			break;
		case VG_FILL_STYLE:
			AG_WriteUint8(buf, (Uint8)vgs->vg_fill_st.style);
			AG_WriteString(buf, vgs->vg_fill_st.texture);
			AG_WriteUint8(buf, vgs->vg_fill_st.texture_alpha);
			break;
		case VG_TEXT_STYLE:
			AG_WriteString(buf, vgs->vg_text_st.face);
			AG_WriteUint8(buf, (Uint8)vgs->vg_text_st.size);
			AG_WriteUint32(buf, (Uint32)vgs->vg_text_st.flags);
			break;
		}
		nstyles++;
	}
	AG_PwriteUint32(buf, nstyles, nstyles_offs);

	/* Save the vg elements. */
	nvges_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->flags & VG_ELEMENT_NOSAVE)
			continue;

		AG_WriteUint32(buf, (Uint32)vge->type);
		AG_WriteString(buf, vge->block != NULL ? vge->block->name : NULL);
		AG_WriteUint32(buf, (Uint32)vge->layer);
		AG_WriteColor(buf, vg->fmt, vge->color);

		/* Save the line style information. */
		AG_WriteUint8(buf, (Uint8)vge->line_st.style);
		AG_WriteUint8(buf, (Uint8)vge->line_st.endpoint_style);
		AG_WriteUint16(buf, vge->line_st.stipple);
		AG_WriteUint8(buf, vge->line_st.thickness);
		AG_WriteUint8(buf, vge->line_st.miter_len);

		/* Save the filling style information. */
		AG_WriteUint8(buf, (Uint8)vge->fill_st.style);
		AG_WriteString(buf, vge->fill_st.texture);
		AG_WriteUint8(buf, vge->fill_st.texture_alpha);
		
		/* Save the text style information. */
		AG_WriteString(buf, vge->text_st.face);
		AG_WriteUint8(buf, (Uint8)vge->text_st.size);
		AG_WriteUint32(buf, (Uint32)vge->text_st.flags);

		/* Save the vertices. */
		AG_WriteUint32(buf, (Uint32)vge->nvtx);
		for (i = 0; i < vge->nvtx; i++)
			AG_WriteVertex(buf, &vge->vtx[i]);

		/* Save element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			AG_WriteDouble(buf, vge->vg_circle.radius);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			AG_WriteDouble(buf, vge->vg_arc.w);
			AG_WriteDouble(buf, vge->vg_arc.h);
			AG_WriteDouble(buf, vge->vg_arc.s);
			AG_WriteDouble(buf, vge->vg_arc.e);
			break;
		case VG_TEXT:
			if (vge->vg_text.su != NULL) {
				SDL_FreeSurface(vge->vg_text.su);
				vge->vg_text.su = NULL;
			}
			AG_WriteString(buf, vge->vg_text.text);
			AG_WriteDouble(buf, vge->vg_text.angle);
			AG_WriteUint8(buf, (Uint8)vge->vg_text.align);
			break;
		case VG_MASK:
			AG_WriteFloat(buf, vge->vg_mask.scale);
			AG_WriteUint8(buf, (Uint8)vge->vg_mask.visible);
			AG_WriteString(buf, NULL);		       /* Pad */
			break;
		default:
			break;
		}
		nvges++;
	}
	AG_PwriteUint32(buf, nvges, nvges_offs);

	pthread_mutex_unlock(&vg->lock);
}

int
VG_Load(VG *vg, AG_Netbuf *buf)
{
	Uint32 norigin, nlayers, nstyles, nelements, nblocks;
	Uint32 i;

	if (AG_ReadVersion(buf, &vgVer, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&vg->lock);
	AG_CopyString(vg->name, buf, sizeof(vg->name));
	vg->flags = AG_ReadUint32(buf);
	vg->w = AG_ReadDouble(buf);
	vg->h = AG_ReadDouble(buf);
	vg->scale = AG_ReadDouble(buf);
	vg->default_scale = AG_ReadDouble(buf);
	vg->fill_color = AG_ReadColor(buf, vg->fmt);
	vg->grid_color = AG_ReadColor(buf, vg->fmt);
	vg->selection_color = AG_ReadColor(buf, vg->fmt);
	vg->mouseover_color = AG_ReadColor(buf, vg->fmt);
	vg->grid_gap = AG_ReadDouble(buf);
	vg->cur_layer = (int)AG_ReadUint32(buf);
	vg->cur_block = NULL;
	vg->cur_vge = NULL;
	dprintf("%s: bbox %.2fx%.2f scale %.2f\n", vg->name, vg->w, vg->h,
	    vg->scale);
	VG_Scale(vg, vg->w, vg->h, vg->scale);

	/* Read the origin points. */
	if ((norigin = AG_ReadUint32(buf)) < 1) {
		AG_SetError("norigin < 1");
		goto fail;
	}
	vg->origin = Realloc(vg->origin, norigin*sizeof(VG_Vtx));
	vg->origin_radius = Realloc(vg->origin_radius, norigin*sizeof(float));
	vg->origin_color = Realloc(vg->origin_color, norigin*sizeof(Uint32));
	vg->norigin = norigin;
	for (i = 0; i < vg->norigin; i++) {
		AG_ReadVertex(buf, &vg->origin[i]);
		vg->origin_radius[i] = AG_ReadFloat(buf);
		vg->origin_color[i] = AG_ReadColor(buf, vg->fmt);
	}

	/* Read the layer information. */
	if ((nlayers = AG_ReadUint32(buf)) < 1) {
		AG_SetError("missing vg layer 0");
		goto fail;
	}
	vg->layers = Realloc(vg->layers, nlayers*sizeof(VG_Layer));
	for (i = 0; i < nlayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_CopyString(layer->name, buf, sizeof(layer->name));
		layer->visible = (int)AG_ReadUint8(buf);
		layer->color = AG_ReadColor(buf, vg->fmt);
		layer->alpha = AG_ReadUint8(buf);
	}
	vg->nlayers = nlayers;

	/* Read the block information. */
	VG_DestroyBlocks(vg);
	nblocks = AG_ReadUint32(buf);
	for (i = 0; i < nblocks; i++) {
		VG_Block *vgb;

		vgb = Malloc(sizeof(VG_Block), M_VG);
		AG_CopyString(vgb->name, buf, sizeof(vgb->name));
		vgb->flags = (int)AG_ReadUint32(buf);
		AG_ReadVertex(buf, &vgb->pos);
		AG_ReadVertex(buf, &vgb->origin);
		vgb->theta = AG_ReadDouble(buf);
		TAILQ_INIT(&vgb->vges);
		TAILQ_INSERT_TAIL(&vg->blocks, vgb, vgbs);
	}

	/* Read the global style information. */
	VG_DestroyStyles(vg);
	nstyles = AG_ReadUint32(buf);
	for (i = 0; i < nstyles; i++) {
		char sname[VG_STYLE_NAME_MAX];
		enum vg_style_type type;
		VG_Style *vgs;

		AG_CopyString(sname, buf, sizeof(sname));
		type = (enum vg_style_type)AG_ReadUint8(buf);
		vgs = VG_CreateStyle(vg, type, sname);
		vgs->color = AG_ReadColor(buf, vg->fmt);

		switch (type) {
		case VG_LINE_STYLE:
			vgs->vg_line_st.style = AG_ReadUint8(buf);
			vgs->vg_line_st.endpoint_style = AG_ReadUint8(buf);
			vgs->vg_line_st.stipple = AG_ReadUint16(buf);
			vgs->vg_line_st.thickness = AG_ReadUint8(buf);
			vgs->vg_line_st.miter_len = AG_ReadUint8(buf);
			break;
		case VG_FILL_STYLE:
			vgs->vg_fill_st.style = AG_ReadUint8(buf);
			AG_CopyString(vgs->vg_fill_st.texture, buf,
			    sizeof(vgs->vg_fill_st.texture));
			vgs->vg_fill_st.texture_alpha = AG_ReadUint8(buf);
			break;
		case VG_TEXT_STYLE:
			AG_CopyString(vgs->vg_text_st.face, buf,
			    sizeof(vgs->vg_text_st.face));
			vgs->vg_text_st.size = (int)AG_ReadUint8(buf);
			vgs->vg_text_st.flags = (int)AG_ReadUint32(buf);
			break;
		}
	}

	/* Read the vg elements. */
	VG_DestroyElements(vg);
	nelements = AG_ReadUint32(buf);
	for (i = 0; i < nelements; i++) {
		char block_id[VG_BLOCK_NAME_MAX];
		enum vg_element_type type;
		VG_Element *vge;
		VG_Block *block;
		Uint32 nlayer;
		int j;
	
		type = (enum vg_element_type)AG_ReadUint32(buf);
		AG_CopyString(block_id, buf, sizeof(block_id));
		nlayer = (int)AG_ReadUint32(buf);

		vge = VG_Begin(vg, type);
		vge->color = AG_ReadColor(buf, vg->fmt);

		/* Load the line style information. */
		vge->line_st.style = AG_ReadUint8(buf);
		vge->line_st.endpoint_style = AG_ReadUint8(buf);
		vge->line_st.stipple = AG_ReadUint16(buf);
		vge->line_st.thickness = AG_ReadUint8(buf);
		vge->line_st.miter_len = AG_ReadUint8(buf);

		/* Load the filling style information. */
		vge->fill_st.style = AG_ReadUint8(buf);
		AG_CopyString(vge->fill_st.texture, buf,
		    sizeof(vge->fill_st.texture));
		vge->fill_st.texture_alpha = AG_ReadUint8(buf);

		/* Load the text style information. */
		AG_CopyString(vge->text_st.face, buf,
		    sizeof(vge->text_st.face));
		vge->text_st.size = (int)AG_ReadUint8(buf);
		vge->text_st.flags = (int)AG_ReadUint32(buf);

		/* Load the vertices. */
		vge->nvtx = (u_int)AG_ReadUint32(buf);
		vge->vtx = Malloc(vge->nvtx*sizeof(VG_Vtx), M_VG);
		for (j = 0; j < vge->nvtx; j++)
			AG_ReadVertex(buf, &vge->vtx[j]);

		/* Associate the element with a block if necessary. */
		if (block_id[0] != '\0') {
			TAILQ_FOREACH(block, &vg->blocks, vgbs) {
				if (strcmp(block->name, block_id) == 0)
					break;
			}
			if (block == NULL) {
				AG_SetError("unexisting vg block: %s", block_id);
				goto fail;
			}
		} else {
			block = NULL;
		}
		if (block != NULL) {
			TAILQ_INSERT_TAIL(&block->vges, vge, vgbmbs);
			vge->block = block;
		}

		/* Load element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			if (vge->nvtx < 1) {
				AG_SetError("circle nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			vge->vg_circle.radius = AG_ReadDouble(buf);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			if (vge->nvtx < 1) {
				AG_SetError("arc nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			vge->vg_arc.w = AG_ReadDouble(buf);
			vge->vg_arc.h = AG_ReadDouble(buf);
			vge->vg_arc.s = AG_ReadDouble(buf);
			vge->vg_arc.e = AG_ReadDouble(buf);
			break;
		case VG_TEXT:
			if (vge->nvtx < 1) {
				AG_SetError("text nvtx < 1");
				VG_DestroyElement(vg, vge);
				goto fail;
			}
			AG_CopyString(vge->vg_text.text, buf,
			    sizeof(vge->vg_text.text));
			vge->vg_text.angle = AG_ReadDouble(buf);
			vge->vg_text.align = AG_ReadUint8(buf);
			break;
		case VG_MASK:
			vge->vg_mask.scale = AG_ReadFloat(buf);
			vge->vg_mask.visible = (int)AG_ReadUint8(buf);
			AG_ReadString(buf);			       /* Pad */
			break;
		default:
			break;
		}
		VG_End(vg);
	}

	vg->redraw = 1;
	pthread_mutex_unlock(&vg->lock);
	return (0);
fail:
	vg->redraw = 1;
	pthread_mutex_unlock(&vg->lock);
	return (-1);
}

#ifdef EDITION
void
VG_GeoChangedEv(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;

	VG_Scale(vg, vg->w, vg->h, vg->scale);
	vg->redraw = 1;
}

void
VG_ChangedEv(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;

	vg->redraw = 1;
}

static void
poll_layers(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	VG *vg = argv[1].p;
	int i;
	
	AG_TlistClear(tl);
	for (i = 0; i < vg->nlayers; i++) {
		VG_Layer *layer = &vg->layers[i];
		char label[AG_TLIST_LABEL_MAX];

		if (layer->visible) {
			snprintf(label, sizeof(label), _("%s (visible %s)\n"),
			    layer->name,
			    (i == vg->cur_layer) ? _(", editing") : "");
		} else {
			snprintf(label, sizeof(label), _("%s (invisible %s)\n"),
			    layer->name,
			    (i == vg->cur_layer) ? _(", editing") : "");
		}
		AG_TlistAddPtr(tl, NULL, label, layer);
	}
	AG_TlistRestore(tl);

	/* XXX load/save hack */
	if (vg->cur_layer >= vg->nlayers)
		vg->cur_layer--;
}

static void
select_layer(int argc, union evarg *argv)
{
	AG_Combo *com = argv[0].p;
	VG *vg = argv[1].p;
	AG_TlistItem *it = argv[2].p;
	int i = 0;

	TAILQ_FOREACH(it, &com->list->items, items) {
		if (it->selected) {
			VG_Layer *lay = it->p1;

			vg->cur_layer = i;
			AG_TextboxPrintf(com->tbox, "%d. %s", i, lay->name);
			return;
		}
		i++;
	}
	AG_TextMsg(AG_MSG_ERROR, _("No layer is selected."));
}

AG_Combo *
VG_NewLayerSelector(void *parent, VG *vg)
{
	AG_Combo *com;

	com = AG_ComboNew(parent, AG_COMBO_POLL, _("Layer:"));
	AG_TextboxPrintf(com->tbox, "%d. %s", vg->cur_layer,
	    vg->layers[vg->cur_layer].name);
	AG_SetEvent(com->list, "tlist-poll", poll_layers, "%p", vg);
	AG_SetEvent(com, "combo-selected", select_layer, "%p", vg);
	return (com);
}

static void
zoom_status(AG_Maptool *t, VG *vg)
{
	AG_MapviewStatus(t->mv, _("Scale %.0f%%"), vg->scale*100.0);
}

static int
zoom_in(AG_Maptool *t, int button, int state, int x, int y, void *arg)
{
	VG *vg = t->p;

	vg->scale += 0.125;
	VG_Scale(vg, vg->w, vg->h, vg->scale);
	zoom_status(t, vg);
	return (1);
}

static int
zoom_out(AG_Maptool *t, int button, int state, int x, int y, void *arg)
{
	VG *vg = t->p;

	vg->scale -= 0.125;
	if (vg->scale < 0.125) {
		vg->scale = 0.125;
	}
	VG_Scale(vg, vg->w, vg->h, vg->scale);
	zoom_status(t, vg);
	return (1);
}

static int
zoom_ident(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	VG *vg = t->p;

	if (state) {
		VG_Scale(vg, vg->w, vg->h, vg->default_scale);
		zoom_status(t, vg);
		return (1);
	}
	return (0);
}

static int
toggle_grid(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	VG *vg = t->p;

	if (state) {
		if (vg->flags & VG_VISGRID) {
			vg->flags &= ~VG_VISGRID;
		} else {
			vg->flags |= VG_VISGRID;
		}
		return (1);
	}
	return (0);
}

static int
toggle_bboxes(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	VG *vg = t->p;

	if (state) {
		if (vg->flags & VG_VISBBOXES) {
			vg->flags &= ~VG_VISBBOXES;
		} else {
			vg->flags |= VG_VISBBOXES;
		}
		return (1);
	}
	return (0);
}

static int
expand_grid(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	VG *vg = t->p;

	if (state) {
		vg->grid_gap += 0.25;
		zoom_status(t, vg);
		return (1);
	}
	return (0);
}

static int
contract_grid(AG_Maptool *t, SDLKey key, int state, void *arg)
{
	VG *vg = t->p;

	if (state) {
		vg->grid_gap -= 0.25;
		if (vg->grid_gap < 0.25) {
			vg->grid_gap = 0.25;
		}
		zoom_status(t, vg);
		return (1);
	}
	return (0);
}

static void
init_scale_tool(void *t)
{
	AG_MaptoolBindKey(t, KMOD_NONE, SDLK_0, zoom_ident, NULL);
	AG_MaptoolBindKey(t, KMOD_NONE, SDLK_1, zoom_ident, NULL);
	AG_MaptoolBindKey(t, KMOD_NONE, SDLK_g, toggle_grid, NULL);
	AG_MaptoolBindKey(t, KMOD_NONE, SDLK_b, toggle_bboxes, NULL);
	AG_MaptoolBindKey(t, KMOD_CTRL, SDLK_EQUALS, expand_grid, NULL);
	AG_MaptoolBindKey(t, KMOD_CTRL, SDLK_MINUS, contract_grid, NULL);

	AG_MaptoolBindMouseButton(t, SDL_BUTTON_WHEELDOWN, zoom_out, NULL);
	AG_MaptoolBindMouseButton(t, SDL_BUTTON_WHEELUP, zoom_in, NULL);
}

const AG_MaptoolOps vgScaleTool = {
	N_("Scale drawing"), N_("Zoom in and out on the drawing."),
	MAGNIFIER_TOOL_ICON,
	sizeof(AG_Maptool),
	TOOL_HIDDEN,
	init_scale_tool,
	NULL,			/* destroy */
	NULL,			/* pane */
	NULL,			/* edit */
	NULL,			/* cursor */
	NULL,			/* effect */

	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */

#ifdef EDITION
static void
select_tool(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	char *name = argv[2].s;
	AG_Mapview *mv = argv[3].p;
	AG_Maptool *t;
	
	if ((t = AG_MapviewFindTool(mv, name)) != NULL) {
		AG_MapviewSelectTool(mv, t, vg);
		AG_WidgetFocus(mv);
	}
}

static void
show_blocks(int argc, union evarg *argv)
{
	VG *vg = argv[1].p;
	AG_Window *win;

	win = VG_BlockEditor(vg);
	AG_WindowShow(win);
}

void
VG_GenericMenu(AG_Menu *m, AG_MenuItem *pitem, VG *vg,
    struct ag_mapview *mv)
{
	extern const AG_MaptoolOps vgOriginTool;
	extern const AG_MaptoolOps vg_line_tool;
	extern const AG_MaptoolOps vg_circle_tool;
	extern const AG_MaptoolOps vgTextTool;
	AG_MenuItem *mi_snap;
	
	mi_snap = AG_MenuAction(pitem, _("Snap to"), SNAP_FREE_ICON, NULL, NULL);
	VG_SnapMenu(m, mi_snap, vg);

	AG_MenuAction(pitem, _("Show blocks"), VGBLOCK_ICON,
	    show_blocks, "%p", vg);
	AG_MenuAction(pitem, _("Move origin"), vgOriginTool.icon,
	    select_tool, "%p,%s,%p", vg, "Origin", mv);
	AG_MenuAction(pitem, _("Line strip"), vg_line_tool.icon,
	    select_tool, "%p,%s,%p", vg, "Lines", mv);
	AG_MenuAction(pitem, _("Circle"), vg_circle_tool.icon,
	    select_tool, "%p,%s,%p", vg, "Circles", mv);
	AG_MenuAction(pitem, _("Text"), vgTextTool.icon,
	    select_tool, "%p,%s,%p", vg, "Text", mv);
}
#endif /* EDITION */
