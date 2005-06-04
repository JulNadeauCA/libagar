/*	$Csoft: vg.c,v 1.50 2005/06/01 09:06:55 vedge Exp $	*/

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

const struct version vg_ver = {
	"agar vg",
	1, 0
};

extern const struct vg_element_ops vg_points_ops;
extern const struct vg_element_ops vg_lines_ops;
extern const struct vg_element_ops vg_line_strip_ops;
extern const struct vg_element_ops vg_line_loop_ops;
extern const struct vg_element_ops vg_circle_ops;
extern const struct vg_element_ops vg_arc_ops;
extern const struct vg_element_ops vg_ellipse_ops;
extern const struct vg_element_ops vg_text_ops;
extern const struct vg_element_ops vg_mask_ops;
extern const struct vg_element_ops vg_polygon_ops;

const struct vg_element_ops *vg_element_types[] = {
	&vg_points_ops,
	&vg_lines_ops,
	&vg_line_strip_ops,
	&vg_line_loop_ops,
	NULL,			/* triangles */
	NULL,			/* triangle strip */
	NULL,			/* triangle fan */
	NULL,			/* quads */
	NULL,			/* quad strip */
	&vg_polygon_ops,
	&vg_circle_ops,
	&vg_arc_ops,
	&vg_ellipse_ops,
	NULL,			/* Bezier curve */
	NULL,			/* Bezigon */
	&vg_text_ops,
	&vg_mask_ops,
};

struct vg *
vg_new(void *p, int flags)
{
	char path[OBJECT_PATH_MAX];
	struct object *ob = p;
	struct vg *vg;
	
	vg = Malloc(sizeof(struct vg), M_VG);
	vg_init(vg, flags);
	if (ob != NULL) {
		vg->pobj = ob;
		gfx_alloc_pvt(ob, "vg");
		vg->map = map_new(ob, "raster");
		OBJECT(vg->map)->flags |= OBJECT_NON_PERSISTENT|
					  OBJECT_INDESTRUCTIBLE;
	}
	return (vg);
}

void
vg_init(struct vg *vg, int flags)
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
	vg->selection_color = SDL_MapRGB(vg->fmt, 150, 150, 0);
	vg->grid_gap = 0.25;
	vg->origin = Malloc(sizeof(struct vg_vertex)*VG_NORIGINS, M_VG);
	vg->origin_radius = Malloc(sizeof(float)*VG_NORIGINS, M_VG);
	vg->origin_color = Malloc(sizeof(Uint32)*VG_NORIGINS, M_VG);
	vg->pobj = NULL;
	vg->map = NULL;
	vg->layers = Malloc(sizeof(struct vg_layer), M_VG);
	vg->nlayers = 0;
	vg->cur_layer = 0;
	vg->cur_block = NULL;
	vg->cur_vge = NULL;
	vg_push_layer(vg, _("Layer 0"));
	vg->snap_mode = VG_GRID;
	vg->ortho_mode = VG_NO_ORTHO;
	TAILQ_INIT(&vg->vges);
	TAILQ_INIT(&vg->blocks);
	TAILQ_INIT(&vg->styles);
	pthread_mutex_init(&vg->lock, &recursive_mutexattr);

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
vg_free_element(struct vg *vg, struct vg_element *vge)
{
	if (vge->ops->destroy != NULL) {
		vge->ops->destroy(vg, vge);
	}
	Free(vge->vtx, M_VG);
	Free(vge, M_VG);
}

static void
vg_destroy_elements(struct vg *vg)
{
	struct vg_element *vge, *nvge;
	
	for (vge = TAILQ_FIRST(&vg->vges);
	     vge != TAILQ_END(&vg->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vges);
		vg_free_element(vg, vge);
	}
	TAILQ_INIT(&vg->vges);
}

static void
vg_destroy_blocks(struct vg *vg)
{
	struct vg_block *vgb, *nvgb;

	for (vgb = TAILQ_FIRST(&vg->blocks);
	     vgb != TAILQ_END(&vg->blocks);
	     vgb = nvgb) {
		nvgb = TAILQ_NEXT(vgb, vgbs);
		Free(vgb, M_VG);
	}
	TAILQ_INIT(&vg->blocks);
}

static void
vg_destroy_styles(struct vg *vg)
{
	struct vg_style *st, *nst;

	for (st = TAILQ_FIRST(&vg->styles);
	     st != TAILQ_END(&vg->styles);
	     st = nst) {
		nst = TAILQ_NEXT(st, styles);
		Free(st, M_VG);
	}
	TAILQ_INIT(&vg->styles);
}

void
vg_reinit(struct vg *vg)
{
	vg_destroy_blocks(vg);
	vg_destroy_elements(vg);
	vg_destroy_styles(vg);
}

void
vg_destroy(struct vg *vg)
{
	struct object *ob = vg->pobj;
	int y;

	Free(vg->origin, M_VG);
	Free(vg->origin_radius, M_VG);
	Free(vg->origin_color, M_VG);

	if (ob != NULL && ob->gfx != NULL) {
		gfx_destroy(ob->gfx);
		ob->gfx = NULL;
	}
	Free(vg->layers, M_VG);

	vg_destroy_blocks(vg);
	vg_destroy_elements(vg);
	vg_destroy_styles(vg);
	pthread_mutex_destroy(&vg->lock);

	Free(vg->ints, M_VG);
}

void
vg_destroy_element(struct vg *vg, struct vg_element *vge)
{
	if (vge->block != NULL)
		TAILQ_REMOVE(&vge->block->vges, vge, vgbmbs);

	if (vg->cur_vge == vge)
		vg->cur_vge = NULL;

	TAILQ_REMOVE(&vg->vges, vge, vges);
	vg_free_element(vg, vge);
	vg->redraw++;
}

/*
 * Generate tile-sized fragments of the raster surface.
 * The vg must be tied to an object.
 */
void
vg_update_fragments(struct vg *vg)
{
	struct object *pobj = vg->pobj;
	struct noderef *r;
	int x, y;
	int mx, my;
	SDL_Rect sd, rd;

	rd.x = 0;
	rd.y = 0;
	sd.w = TILESZ;
	sd.h = TILESZ;
	
	for (y = 0, my = 0;
	     y < vg->su->h && my < vg->map->maph;
	     y += TILESZ, my++) {
		for (x = 0, mx = 0;
		     x < vg->su->w && mx < vg->map->mapw;
		     x += TILESZ, mx++) {
			struct node *n = &vg->map->map[my][mx];
			SDL_Surface *fragsu = NULL;
			Uint32 saflags, scflags, scolorkey;
			Uint8 salpha;
			int fw, fh;

			saflags = vg->su->flags&(SDL_SRCALPHA|SDL_RLEACCEL);
			scflags = vg->su->flags&(SDL_SRCCOLORKEY|SDL_RLEACCEL);
			salpha = vg->su->format->alpha;
			scolorkey = vg->su->format->colorkey;

			fw = vg->su->w-x < TILESZ ? vg->su->w-x : TILESZ;
			fh = vg->su->h-y < TILESZ ? vg->su->h-y : TILESZ;

			if (fw <= 0 || fh <= 0)
				fatal("fragment too small");

			TAILQ_FOREACH(r, &n->nrefs, nrefs) {
				if (r->type == NODEREF_SPRITE &&
				    r->layer == vg->map->cur_layer &&
				    r->r_sprite.obj == pobj) {
					fragsu = SPRITE(pobj,
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

			SDL_SetAlpha(vg->su, 0, 0);
			SDL_SetColorKey(vg->su, 0, 0);
			SDL_BlitSurface(vg->su, &sd, fragsu, &rd);
			SDL_SetAlpha(vg->su, saflags, salpha);
			SDL_SetColorKey(vg->su, scflags, scolorkey);

			if (r == NULL) {
				Uint32 sp;

				sp = gfx_insert_sprite(pobj->gfx, fragsu);
				SPRITE(pobj,sp).snap_mode = GFX_SNAP_TO_GRID;
				node_add_sprite(vg->map, n, pobj, sp);
			}
		}
	}
}

/*
 * Release the raster fragments.
 * The vg must be tied to an object.
 */
void
vg_destroy_fragments(struct vg *vg)
{
	struct gfx *gfx = vg->pobj->gfx;
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		sprite_destroy(&gfx->sprites[i]);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		object_destroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}
	gfx->nsprites = 0;
	gfx->nsubmaps = 0;

	if (vg->map != NULL)
		map_free_nodes(vg->map);
}

/* Adjust the vg bounding box and scaling factor. */
void
vg_scale(struct vg *vg, double w, double h, double scale)
{
	int pw = (int)(w*scale*TILESZ);
	int ph = (int)(h*scale*TILESZ);
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
		vg_destroy_fragments(vg);
		mw = vg->su->w/TILESZ+1;
		mh = vg->su->h/TILESZ+1;
		if (map_alloc_nodes(vg->map, mw, mh) == -1)
			fatal("%s", error_get());
	}
	vg->redraw++;
}

/*
 * Allocate the given type of element and begin its parametrization.
 * If a block is selected, associate the element with it.
 */
struct vg_element *
vg_begin_element(struct vg *vg, enum vg_element_type eltype)
{
	struct vg_element *vge;
	int i;

	vge = Malloc(sizeof(struct vg_element), M_VG);
	vge->flags = 0;
	vge->type = eltype;
	vge->style = NULL;
	vge->layer = vg->cur_layer;
	vge->drawn = 0;
	vge->selected = 0;
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
	vge->fill_st.style = VG_NOFILL;

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

	vge->ops = vg_element_types[eltype];
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
vg_end_element(struct vg *vg)
{
	vg->cur_vge = NULL;
}

/*
 * Select the given element for edition.
 * The vg must be locked.
 */
void
vg_select_element(struct vg *vg, struct vg_element *vge)
{
	vg->cur_vge = vge;
}

#ifdef DEBUG
static void
vg_draw_bboxes(struct vg *vg)
{
	struct vg_rect bbox;
	struct vg_element *vge;
	struct vg_block *vgb;
	int x, y, w, h;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->ops->bbox != NULL) {
			vge->ops->bbox(vg, vge, &bbox);
		} else {
			continue;
		}
		vg_rcoords2(vg, bbox.x, bbox.y, &x, &y);
		vg_rlength(vg, bbox.w, &w);
		vg_rlength(vg, bbox.h, &h);
		vg_rect_primitive(vg, x, y, w, h, vg->grid_color);
	}
	
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		Uint32 ext_color = SDL_MapRGB(vg->fmt, 0, 250, 0);

		vg_block_extent(vg, vgb, &bbox);
		vg_rcoords2(vg, bbox.x, bbox.y, &x, &y);
		vg_rlength(vg, bbox.w, &w);
		vg_rlength(vg, bbox.h, &h);
		vg_rect_primitive(vg, x, y, w, h, ext_color);
	}
}
#endif /* DEBUG */

/* Evaluate the intersection between two rectangles. */
int
vg_rcollision(struct vg *vg, struct vg_rect *r1, struct vg_rect *r2,
    struct vg_rect *ixion)
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
vg_rasterize_element(struct vg *vg, struct vg_element *vge)
{
	struct vg_element *ovge;
	struct vg_rect r1, r2;
	Uint32 color_save = 0;

	if (!vge->drawn) {
		if (vge->selected) {
			color_save = vge->color;
			vge->color = vg->selection_color;
		}

		vge->ops->draw(vg, vge);
		vge->drawn = 1;
		
		if (vge->selected)
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
			if (vg_rcollision(vg, &r1, &r2, NULL))
				vg_rasterize_element(vg, ovge);
		}
	}
}

/* Rasterize elements marked dirty and update the affected tiles. */
void
vg_rasterize(struct vg *vg)
{
	struct vg_element *vge;
	struct vg_block *vgb;
	int i;

	pthread_mutex_lock(&vg->lock);

	SDL_FillRect(vg->su, NULL, vg->fill_color);

	if (vg->flags & VG_VISGRID)
		vg_draw_grid(vg);
#ifdef DEBUG
	if (vg->flags & VG_VISBBOXES)
		vg_draw_bboxes(vg);
#endif
	
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vge->drawn = 0;
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vg_rasterize_element(vg, vge);
	}
	if (vg->flags & VG_VISORIGIN)
		vg_draw_origin(vg);

	if (vg->pobj != NULL)
		vg_update_fragments(vg);

	vg->redraw = 0;
	pthread_mutex_unlock(&vg->lock);
}

/*
 * Translate a length in pixel to a vg vector magnitude.
 * The vg must be locked.
 */
void
vg_vlength(struct vg *vg, int len, double *vlen)
{
	*vlen = (double)(len/vg->scale/TILESZ);
}

/*
 * Translate tile coordinates to relative vg coordinates, applying
 * positional and orthogonal restrictions as needed.
 * The vg must be locked.
 */
void
vg_vcoords2(struct vg *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/TILESZ -
	    vg->origin[0].x;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/TILESZ -
	    vg->origin[0].y;
	
	if (vg->snap_mode != VG_FREE_POSITIONING)
		vg_snap_to(vg, vx, vy);
	if (vg->ortho_mode != VG_NO_ORTHO)
		vg_ortho_restrict(vg, vx, vy);
}

/*
 * Translate tile coordinates to absolute vg coordinates.
 * The vg must be locked.
 */
void
vg_avcoords2(struct vg *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/TILESZ;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/TILESZ;

	if (vg->snap_mode != VG_FREE_POSITIONING)
		vg_snap_to(vg, vx, vy);
}

/*
 * Translate relative vg coordinates to raster coordinates.
 * The vg must be locked.
 */
void
vg_rcoords2(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ) +
	      (int)(vg->origin[0].x*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ) +
	      (int)(vg->origin[0].y*vg->scale*TILESZ);
}

/*
 * Translate absolute vg coordinates to integer raster coordinates.
 * The vg must be locked.
 */
void
vg_arcoords2(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ);
}

/*
 * Translate relative vg coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
vg_rcoords2d(struct vg *vg, double vx, double vy, double *rx, double *ry)
{
	*rx = vx*vg->scale*TILESZ +
	      vg->origin[0].x*vg->scale*TILESZ;
	*ry = vy*vg->scale*TILESZ +
	      vg->origin[0].y*vg->scale*TILESZ;
}

/*
 * Translate vertex coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
vg_vtxcoords2d(struct vg *vg, struct vg_vertex *vtx, double *rx, double *ry)
{
	if (rx != NULL)	*rx = VG_RASXF(vg,vtx->x);
	if (ry != NULL)	*ry = VG_RASYF(vg,vtx->y);
}

/*
 * Translate vertex coordinates to integral raster coordinates.
 * The vg must be locked.
 */
void
vg_vtxcoords2i(struct vg *vg, struct vg_vertex *vtx, int *rx, int *ry)
{
	if (rx != NULL)	*rx = VG_RASX(vg,vtx->x);
	if (ry != NULL) *ry = VG_RASY(vg,vtx->y);
}

/*
 * Translate absolute vg coordinates to floating-point raster coordinates.
 * The vg must be locked.
 */
void
vg_arcoords2d(struct vg *vg, double vx, double vy, double *rx, double *ry)
{
	*rx = vx*vg->scale*TILESZ;
	*ry = vy*vg->scale*TILESZ;
}

/*
 * Translate the magnitude of a vg vector to the raster equivalent in pixels.
 * The vg must be locked.
 */
void
vg_rlength(struct vg *vg, double len, int *rlen)
{
	*rlen = (int)(len*vg->scale*TILESZ);
}

static __inline__ struct vg_vertex *
vg_alloc_vertex(struct vg_element *vge)
{
	if (vge->vtx == NULL) {
		vge->vtx = Malloc(sizeof(struct vg_vertex), M_VG);
	} else {
		vge->vtx = Realloc(vge->vtx, (vge->nvtx+1) *
		                             sizeof(struct vg_vertex));
	}
	return (&vge->vtx[vge->nvtx++]);
}

/* Pop the highest vertex off the vertex array. */
struct vg_vertex *
vg_pop_vertex(struct vg *vg)
{
	struct vg_element *vge = vg->cur_vge;

	if (vge->vtx == NULL)
		return (NULL);
#ifdef DEBUG
	if (vge->nvtx-1 < 0)
		fatal("neg nvtx");
#endif
	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(struct vg_vertex));
	vg->redraw++;
	return (vge->nvtx > 0 ? &vge->vtx[vge->nvtx-1] : NULL);
}

/* Push a 2D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex2(struct vg *vg, double x, double y)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = 0;
	vtx->w = 1.0;
	vg_block_offset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a 3D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex3(struct vg *vg, double x, double y, double z)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = 1.0;
	vg_block_offset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a homogenized 3D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex4(struct vg *vg, double x, double y, double z, double w)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(vg->cur_vge);
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = w;
	vg_block_offset(vg, vtx);
	vg->redraw++;
	return (vtx);
}

/* Push a series of vertices onto the vertex array. */
void
vg_vertex_array(struct vg *vg, const struct vg_vertex *svtx, u_int nsvtx)
{
	struct vg_element *vge = vg->cur_vge;
	u_int i;
	
	for (i = 0; i < nsvtx; i++) {
		struct vg_vertex *vtx;

		vtx = vg_alloc_vertex(vge);
		memcpy(vtx, &svtx[i], sizeof(struct vg_vertex));
		vg_block_offset(vg, vtx);
	}
	vg->redraw++;
}

/* Create a new global style. */
struct vg_style *
vg_create_style(struct vg *vg, enum vg_style_type type, const char *name)
{
	struct vg_style *vgs;

	vgs = Malloc(sizeof(struct vg_style), M_VG);
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
		vgs->vg_fill_st.style = VG_NOFILL;
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
vg_style(struct vg *vg, const char *name)
{
	struct vg_element *vge = vg->cur_vge;
	struct vg_style *st;

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
		    sizeof(struct vg_line_style));
		break;
	case VG_TEXT_STYLE:
		memcpy(&vge->text_st, &st->vg_text_st,
		    sizeof(struct vg_text_style));
		break;
	case VG_FILL_STYLE:
		memcpy(&vge->fill_st, &st->vg_fill_st,
		    sizeof(struct vg_fill_style));
		break;
	}
	return (0);
}

/* Specify the layer# to associate with the current element. */
void
vg_layer(struct vg *vg, int layer)
{
	vg->cur_vge->layer = layer;
}

/* Specify the color of the current element (format-specific). */
void
vg_color(struct vg *vg, Uint32 color)
{
	vg->cur_vge->color = color;
	vg->redraw++;
}

/* Specify the color of the current element (RGB triplet). */
void
vg_color3(struct vg *vg, int r, int g, int b)
{
	vg->cur_vge->color = SDL_MapRGB(vg->fmt, r, g, b);
	vg->redraw++;
}

/* Specify the color of the current element (RGB triplet + alpha). */
void
vg_color4(struct vg *vg, int r, int g, int b, int a)
{
	vg->cur_vge->color = SDL_MapRGBA(vg->fmt, r, g, b, a);
	vg->redraw++;
}

/* Push a new layer onto the layer stack. */
struct vg_layer *
vg_push_layer(struct vg *vg, const char *name)
{
	struct vg_layer *vgl;

	vg->layers = Realloc(vg->layers, (vg->nlayers+1) *
	                                 sizeof(struct vg_layer));
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
vg_pop_layer(struct vg *vg)
{
	if (--vg->nlayers < 1)
		vg->nlayers = 1;
	
	vg->redraw++;
}

void
vg_save(struct vg *vg, struct netbuf *buf)
{
	off_t nblocks_offs, nvges_offs, nstyles_offs;
	Uint32 nblocks = 0, nvges = 0, nstyles = 0;
	struct vg_block *vgb;
	struct vg_style *vgs;
	struct vg_element *vge;
	int i;

	version_write(buf, &vg_ver);

	pthread_mutex_lock(&vg->lock);

	write_string(buf, vg->name);
	write_uint32(buf, (Uint32)vg->flags);
	write_double(buf, vg->w);
	write_double(buf, vg->h);
	write_double(buf, vg->scale);
	write_color(buf, vg->fmt, vg->fill_color);
	write_color(buf, vg->fmt, vg->grid_color);
	write_color(buf, vg->fmt, vg->selection_color);
	write_double(buf, vg->grid_gap);
	write_uint32(buf, (Uint32)vg->cur_layer);

	/* Save the origin points. */
	write_uint32(buf, vg->norigin);
	for (i = 0; i < vg->norigin; i++) {
		write_vertex(buf, &vg->origin[i]);
		write_float(buf, vg->origin_radius[i]);
		write_color(buf, vg->fmt, vg->origin_color[i]);
	}

	/* Save the layer information. */
	write_uint32(buf, vg->nlayers);
	for (i = 0; i < vg->nlayers; i++) {
		struct vg_layer *layer = &vg->layers[i];

		write_string(buf, layer->name);
		write_uint8(buf, (Uint8)layer->visible);
		write_color(buf, vg->fmt, layer->color);
		write_uint8(buf, layer->alpha);
	}

	/* Save the block information. */
	nblocks_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(vgb, &vg->blocks, vgbs) {
		if (vgb->flags & VG_BLOCK_NOSAVE)
			continue;

		write_string(buf, vgb->name);
		write_uint32(buf, (Uint32)vgb->flags);
		write_vertex(buf, &vgb->pos);
		write_vertex(buf, &vgb->origin);
		write_double(buf, vgb->theta);
		nblocks++;
	}
	pwrite_uint32(buf, nblocks, nblocks_offs);

	/* Save the global style information. */
	nstyles_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		write_string(buf, vgs->name);
		write_uint8(buf, (Uint8)vgs->type);
		write_color(buf, vg->fmt, vgs->color);

		switch (vgs->type) {
		case VG_LINE_STYLE:
			write_uint8(buf, (Uint8)vgs->vg_line_st.style);
			write_uint8(buf, (Uint8)vgs->vg_line_st.endpoint_style);
			write_uint16(buf, vgs->vg_line_st.stipple);
			write_uint8(buf, vgs->vg_line_st.thickness);
			write_uint8(buf, vgs->vg_line_st.miter_len);
			break;
		case VG_FILL_STYLE:
			write_uint8(buf, (Uint8)vgs->vg_fill_st.style);
			break;
		case VG_TEXT_STYLE:
			write_string(buf, vgs->vg_text_st.face);
			write_uint8(buf, (Uint8)vgs->vg_text_st.size);
			write_uint32(buf, (Uint32)vgs->vg_text_st.flags);
			break;
		}
		nstyles++;
	}
	pwrite_uint32(buf, nstyles, nstyles_offs);

	/* Save the vg elements. */
	nvges_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->flags & VG_ELEMENT_NOSAVE)
			continue;

		write_uint32(buf, (Uint32)vge->type);
		write_string(buf, vge->block != NULL ? vge->block->name : NULL);
		write_uint32(buf, (Uint32)vge->layer);
		write_color(buf, vg->fmt, vge->color);

		/* Save the line style information. */
		write_uint8(buf, (Uint8)vge->line_st.style);
		write_uint8(buf, (Uint8)vge->line_st.endpoint_style);
		write_uint16(buf, vge->line_st.stipple);
		write_uint8(buf, vge->line_st.thickness);
		write_uint8(buf, vge->line_st.miter_len);

		/* Save the filling style information. */
		write_uint8(buf, (Uint8)vge->fill_st.style);
		
		/* Save the text style information. */
		write_string(buf, vge->text_st.face);
		write_uint8(buf, (Uint8)vge->text_st.size);
		write_uint32(buf, (Uint32)vge->text_st.flags);

		/* Save the vertices. */
		write_uint32(buf, vge->nvtx);
		for (i = 0; i < vge->nvtx; i++)
			write_vertex(buf, &vge->vtx[i]);

		/* Save element specific data. */
		switch (vge->type) {
		case VG_CIRCLE:
			write_double(buf, vge->vg_circle.radius);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			write_double(buf, vge->vg_arc.w);
			write_double(buf, vge->vg_arc.h);
			write_double(buf, vge->vg_arc.s);
			write_double(buf, vge->vg_arc.e);
			break;
		case VG_TEXT:
			if (vge->vg_text.su != NULL) {
				SDL_FreeSurface(vge->vg_text.su);
				vge->vg_text.su = NULL;
			}
			write_string(buf, vge->vg_text.text);
			write_double(buf, vge->vg_text.angle);
			write_uint8(buf, (Uint8)vge->vg_text.align);
			break;
		case VG_MASK:
			write_float(buf, vge->vg_mask.scale);
			write_uint8(buf, (Uint8)vge->vg_mask.visible);
			write_string(buf, NULL);		       /* Pad */
			break;
		default:
			break;
		}
		nvges++;
	}
	pwrite_uint32(buf, nvges, nvges_offs);

	pthread_mutex_unlock(&vg->lock);
}

int
vg_load(struct vg *vg, struct netbuf *buf)
{
	Uint32 norigin, nlayers, nstyles, nelements, nblocks;
	Uint32 i;

	if (version_read(buf, &vg_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&vg->lock);
	copy_string(vg->name, buf, sizeof(vg->name));
	vg->flags = read_uint32(buf);
	vg->w = read_double(buf);
	vg->h = read_double(buf);
	vg->scale = read_double(buf);
	vg->fill_color = read_color(buf, vg->fmt);
	vg->grid_color = read_color(buf, vg->fmt);
	vg->selection_color = read_color(buf, vg->fmt);
	vg->grid_gap = read_double(buf);
	vg->cur_layer = (int)read_uint32(buf);
	vg->cur_block = NULL;
	vg->cur_vge = NULL;
	dprintf("name `%s' bbox %.2fx%.2f scale %.2f\n", vg->name, vg->w,
	    vg->h, vg->scale);
	vg_scale(vg, vg->w, vg->h, vg->scale);

	/* Read the origin points. */
	if ((norigin = read_uint32(buf)) < 1) {
		error_set("norigin < 1");
		goto fail;
	}
	vg->origin = Realloc(vg->origin, norigin*sizeof(struct vg_vertex));
	vg->origin_radius = Realloc(vg->origin_radius, norigin*sizeof(float));
	vg->origin_color = Realloc(vg->origin_color, norigin*sizeof(Uint32));
	vg->norigin = norigin;
	for (i = 0; i < vg->norigin; i++) {
		read_vertex(buf, &vg->origin[i]);
		vg->origin_radius[i] = read_float(buf);
		vg->origin_color[i] = read_color(buf, vg->fmt);
	}
	dprintf("%d origin vertices\n", vg->norigin);

	/* Read the layer information. */
	if ((nlayers = read_uint32(buf)) < 1) {
		error_set("nlayers < 1");
		goto fail;
	}
	vg->layers = Realloc(vg->layers, nlayers*sizeof(struct vg_layer));
	for (i = 0; i < nlayers; i++) {
		struct vg_layer *layer = &vg->layers[i];

		dprintf("layer %d: name `%s' vis %d\n", i, layer->name,
		    layer->visible);
		copy_string(layer->name, buf, sizeof(layer->name));
		layer->visible = (int)read_uint8(buf);
		layer->color = read_color(buf, vg->fmt);
		layer->alpha = read_uint8(buf);
	}
	vg->nlayers = nlayers;

	/* Read the block information. */
	vg_destroy_blocks(vg);
	nblocks = read_uint32(buf);
	dprintf("%u blocks\n", nblocks);
	for (i = 0; i < nblocks; i++) {
		struct vg_block *vgb;

		vgb = Malloc(sizeof(struct vg_block), M_VG);
		copy_string(vgb->name, buf, sizeof(vgb->name));
		vgb->flags = (int)read_uint32(buf);
		read_vertex(buf, &vgb->pos);
		read_vertex(buf, &vgb->origin);
		vgb->theta = read_double(buf);
		TAILQ_INIT(&vgb->vges);
		TAILQ_INSERT_TAIL(&vg->blocks, vgb, vgbs);
	}

	/* Read the global style information. */
	vg_destroy_styles(vg);
	nstyles = read_uint32(buf);
	dprintf("%u styles\n", nstyles);
	for (i = 0; i < nstyles; i++) {
		char sname[VG_STYLE_NAME_MAX];
		enum vg_style_type type;
		struct vg_style *vgs;

		copy_string(sname, buf, sizeof(sname));
		type = (enum vg_style_type)read_uint8(buf);
		vgs = vg_create_style(vg, type, sname);
		vgs->color = read_color(buf, vg->fmt);

		switch (type) {
		case VG_LINE_STYLE:
			vgs->vg_line_st.style = read_uint8(buf);
			vgs->vg_line_st.endpoint_style = read_uint8(buf);
			vgs->vg_line_st.stipple = read_uint16(buf);
			vgs->vg_line_st.thickness = read_uint8(buf);
			vgs->vg_line_st.miter_len = read_uint8(buf);
			break;
		case VG_FILL_STYLE:
			vgs->vg_fill_st.style = read_uint8(buf);
			break;
		case VG_TEXT_STYLE:
			copy_string(vgs->vg_text_st.face, buf,
			    sizeof(vgs->vg_text_st.face));
			vgs->vg_text_st.size = (int)read_uint8(buf);
			vgs->vg_text_st.flags = (int)read_uint32(buf);
			break;
		}
	}

	/* Read the vg elements. */
	vg_destroy_elements(vg);
	nelements = read_uint32(buf);
	dprintf("%u elements\n", nelements);
	for (i = 0; i < nelements; i++) {
		enum vg_element_type type;
		struct vg_element *vge;
		struct vg_block *block;
		char *block_id;
		Uint32 nlayer;
		int j;
	
		type = (enum vg_element_type)read_uint32(buf);
		block_id = read_string_len(buf, VG_BLOCK_NAME_MAX);
		nlayer = (int)read_uint32(buf);

		vge = vg_begin_element(vg, type);
		vge->color = read_color(buf, vg->fmt);

		/* Load the line style information. */
		vge->line_st.style = read_uint8(buf);
		vge->line_st.endpoint_style = read_uint8(buf);
		vge->line_st.stipple = read_uint16(buf);
		vge->line_st.thickness = read_uint8(buf);
		vge->line_st.miter_len = read_uint8(buf);

		/* Load the filling style information. */
		vge->fill_st.style = read_uint8(buf);

		/* Load the text style information. */
		copy_string(vge->text_st.face, buf, sizeof(vge->text_st.face));
		vge->text_st.size = (int)read_uint8(buf);
		vge->text_st.flags = (int)read_uint32(buf);

		/* Load the vertices. */
		vge->nvtx = read_uint32(buf);
		vge->vtx = Malloc(vge->nvtx*sizeof(struct vg_vertex), M_VG);
		for (j = 0; j < vge->nvtx; j++)
			read_vertex(buf, &vge->vtx[j]);

		/* Associate the element with a block if necessary. */
		if (block_id != NULL) {
			TAILQ_FOREACH(block, &vg->blocks, vgbs) {
				if (strcmp(block->name, block_id) == 0)
					break;
			}
			Free(block_id, 0);
			if (block == NULL) {
				error_set("unexisting block");
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
				error_set("circle nvtx < 1");
				vg_destroy_element(vg, vge);
				goto fail;
			}
			vge->vg_circle.radius = read_double(buf);
			break;
		case VG_ARC:
		case VG_ELLIPSE:
			if (vge->nvtx < 1) {
				error_set("arc nvtx < 1");
				vg_destroy_element(vg, vge);
				goto fail;
			}
			vge->vg_arc.w = read_double(buf);
			vge->vg_arc.h = read_double(buf);
			vge->vg_arc.s = read_double(buf);
			vge->vg_arc.e = read_double(buf);
			break;
		case VG_TEXT:
			if (vge->nvtx < 1) {
				error_set("text nvtx < 1");
				vg_destroy_element(vg, vge);
				goto fail;
			}
			copy_string(vge->vg_text.text, buf,
			    sizeof(vge->vg_text.text));
			vge->vg_text.angle = read_double(buf);
			vge->vg_text.align = read_uint8(buf);
			break;
		case VG_MASK:
			vge->vg_mask.scale = read_float(buf);
			vge->vg_mask.visible = (int)read_uint8(buf);
			read_string(buf);			       /* Pad */
			break;
		default:
			break;
		}
		vg_end_element(vg);
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

static struct timeout zoom_in_to, zoom_out_to;

void
vg_geo_changed(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;

	vg_scale(vg, vg->w, vg->h, vg->scale);
	vg->redraw = 1;
}

void
vg_changed(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;

	vg->redraw = 1;
}

static void
poll_layers(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct vg *vg = argv[1].p;
	int i;
	
	tlist_clear_items(tl);
	for (i = 0; i < vg->nlayers; i++) {
		struct vg_layer *layer = &vg->layers[i];
		char label[TLIST_LABEL_MAX];

		if (layer->visible) {
			snprintf(label, sizeof(label), _("%s (visible %s)\n"),
			    layer->name,
			    (i == vg->cur_layer) ? _(", editing") : "");
		} else {
			snprintf(label, sizeof(label), _("%s (invisible %s)\n"),
			    layer->name,
			    (i == vg->cur_layer) ? _(", editing") : "");
		}
		tlist_insert_item(tl, NULL, label, layer);
	}
	tlist_restore_selections(tl);

	/* XXX load/save hack */
	if (vg->cur_layer >= vg->nlayers)
		vg->cur_layer--;
}

static void
select_layer(int argc, union evarg *argv)
{
	struct combo *com = argv[0].p;
	struct vg *vg = argv[1].p;
	struct tlist_item *it = argv[2].p;
	int i = 0;

	TAILQ_FOREACH(it, &com->list->items, items) {
		if (it->selected) {
			struct vg_layer *lay = it->p1;

			vg->cur_layer = i;
			textbox_printf(com->tbox, "%d. %s", i, lay->name);
			return;
		}
		i++;
	}
	text_msg(MSG_ERROR, _("No layer is selected."));
}

struct combo *
vg_layer_selector(void *parent, struct vg *vg)
{
	struct combo *com;

	com = combo_new(parent, COMBO_POLL, _("Layer:"));
	textbox_printf(com->tbox, "%d. %s", vg->cur_layer,
	    vg->layers[vg->cur_layer].name);
	event_new(com->list, "tlist-poll", poll_layers, "%p", vg);
	event_new(com, "combo-selected", select_layer, "%p", vg);
	return (com);
}

static void
zoom_status(struct tool *t, struct vg *vg)
{
	mapview_status(t->mv, _("Scaling factor: %.2f, grid: %.2f"),
	    vg->scale, vg->grid_gap);
}

static Uint32
zoom_in_tick(void *p, Uint32 ival, void *arg)
{
	struct tool *t = arg;
	struct vg *vg = t->p;

	vg->scale += 0.125;
	vg_scale(vg, vg->w, vg->h, vg->scale);
	zoom_status(t, vg);
	return (ival);
}

static Uint32
zoom_out_tick(void *p, Uint32 ival, void *arg)
{
	struct tool *t = arg;
	struct vg *vg = t->p;

	vg->scale -= 0.125;
	if (vg->scale < 0.125) {
		vg->scale = 0.125;
	}
	vg_scale(vg, vg->w, vg->h, vg->scale);
	zoom_status(t, vg);
	return (ival);
}

static void
zoom_in(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		if (SDL_GetModState() & KMOD_CTRL) {
			return;
		}
		zoom_in_tick(NULL, 0, t);
#if 0
		timeout_add(NULL, &zoom_in_to, 80);
#endif
	} else {
#if 0
		lock_timeout(NULL);
		if (timeout_scheduled(NULL, &zoom_in_to)) {
			timeout_del(NULL, &zoom_in_to);
		}
		unlock_timeout(NULL);
#endif
	}
}

static void
zoom_out(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		if (SDL_GetModState() & KMOD_CTRL) {
			return;
		}
		zoom_out_tick(NULL, 0, t);
#if 0
		timeout_add(NULL, &zoom_out_to, 80);
#endif
	} else {
#if 0
		lock_timeout(NULL);
		if (timeout_scheduled(NULL, &zoom_out_to)) {
			timeout_del(NULL, &zoom_out_to);
		}
		unlock_timeout(NULL);
#endif
	}
}

static void
zoom_ident(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		vg_scale(vg, vg->w, vg->h, 1.0);
		zoom_status(t, vg);
	}
}

static void
toggle_grid(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		if (vg->flags & VG_VISGRID) {
			vg->flags &= ~VG_VISGRID;
		} else {
			vg->flags |= VG_VISGRID;
		}
	}
}

static void
toggle_bboxes(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		if (vg->flags & VG_VISBBOXES) {
			vg->flags &= ~VG_VISBBOXES;
		} else {
			vg->flags |= VG_VISBBOXES;
		}
	}
}

static void
expand_grid(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		vg->grid_gap += 0.25;
		zoom_status(t, vg);
	}
}

static void
contract_grid(struct tool *t, int state)
{
	struct vg *vg = t->p;

	if (state) {
		vg->grid_gap -= 0.25;
		if (vg->grid_gap < 0.25) {
			vg->grid_gap = 0.25;
		}
		zoom_status(t, vg);
	}
}

static void
init_scale_tool(struct tool *t)
{
	tool_bind_key(t, KMOD_NONE, SDLK_EQUALS, zoom_in, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_MINUS, zoom_out, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_0, zoom_ident, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_1, zoom_ident, 0);
	tool_bind_mousebutton(t, SDL_BUTTON_WHEELDOWN, 1, zoom_out, 0);
	tool_bind_mousebutton(t, SDL_BUTTON_WHEELUP, 1, zoom_in, 0);

	timeout_set(&zoom_in_to, zoom_in_tick, t, 0);
	timeout_set(&zoom_out_to, zoom_out_tick, t, 0);
}

static void
init_grid_tool(struct tool *t)
{
	tool_bind_key(t, KMOD_NONE, SDLK_g, toggle_grid, 0);
	tool_bind_key(t, KMOD_NONE, SDLK_b, toggle_bboxes, 0);
	tool_bind_key(t, KMOD_CTRL, SDLK_EQUALS, expand_grid, 0);
	tool_bind_key(t, KMOD_CTRL, SDLK_MINUS, contract_grid, 0);
}

struct tool vg_scale_tool = {
	N_("Scale drawing"),
	N_("Zoom in and out on the drawing."),
	MAGNIFIER_TOOL_ICON,
	-1,
	init_scale_tool,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};

struct tool vg_grid_tool = {
	N_("Grid"),
	N_("Toggle the grid."),
	SNAP_GRID_ICON,
	-1,
	init_grid_tool,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
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
	struct vg *vg = argv[1].p;
	struct tool *tool = argv[2].p;
	struct mapview *mv = argv[3].p;

	mapview_select_tool(mv, tool, vg);
	widget_focus(mv);
}

static void
show_blocks(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;
	struct window *win;

	win = vg_block_editor(vg);
	window_show(win);
}

void
vg_reg_menu(struct AGMenu *m, struct AGMenuItem *pitem, struct vg *vg,
    struct mapview *mv)
{
	extern struct tool vg_origin_tool;
	extern struct tool vg_line_tool;
	extern struct tool vg_circle_tool;
	extern struct tool vg_text_tool;
	struct AGMenuItem *mi_snap;
	
	mi_snap = menu_action(pitem, _("Snap to"), SNAP_FREE_ICON, NULL, NULL);
	vg_reg_snap_menu(m, mi_snap, vg);

	menu_action(pitem, _("Show blocks"), VGBLOCK_ICON,
	    show_blocks, "%p", vg);
	menu_action(pitem, _("Move origin"), vg_origin_tool.icon,
	    select_tool, "%p,%p,%p", vg, &vg_origin_tool, mv);
	menu_action(pitem, _("Line strip"), vg_line_tool.icon,
	    select_tool, "%p,%p,%p", vg, &vg_line_tool, mv);
	menu_action(pitem, _("Circle"), vg_circle_tool.icon,
	    select_tool, "%p,%p,%p", vg, &vg_circle_tool, mv);
	menu_action(pitem, _("Text"), vg_text_tool.icon,
	    select_tool, "%p,%p,%p", vg, &vg_text_tool, mv);
}
#endif /* EDITION */
