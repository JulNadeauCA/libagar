/*	$Csoft: vg.c,v 1.19 2004/05/10 05:17:05 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/widget/toolbar.h>
#include <engine/widget/button.h>
#include <engine/widget/combo.h>
#include <engine/widget/tlist.h>
#endif

#include "vg.h"
#include "vg_primitive.h"

#include <engine/loader/vertex.h>

const struct version vg_ver = {
	"agar vg",
	0, 0
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

static const struct vg_element_ops *vge_types[] = {
	&vg_points_ops,
	&vg_lines_ops,
	&vg_line_strip_ops,
	&vg_line_loop_ops,
	NULL,			/* triangles */
	NULL,			/* triangle strip */
	NULL,			/* triangle fan */
	NULL,			/* quads */
	NULL,			/* quad strip */
	NULL,			/* polygon */
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
	vg->pobj = ob;
	gfx_alloc_pvt(ob, "vg");
	vg->map = map_new(ob, "raster");
	OBJECT(vg->map)->flags |= OBJECT_NON_PERSISTENT|OBJECT_INDESTRUCTIBLE;
	return (vg);
}

void
vg_init(struct vg *vg, int flags)
{
	int i, x, y;

	strlcpy(vg->name, _("Untitled"), sizeof(vg->name));
	vg->flags = flags;
	vg->redraw = 1;
	vg->w = 0;
	vg->h = 0;
	vg->scale = 1;
	vg->su = NULL;
	vg->fill_color = SDL_MapRGB(vfmt, 0, 0, 0);
	vg->grid_color = SDL_MapRGB(vfmt, 128, 128, 128);
	vg->grid_gap = 0.5;
	vg->origin = Malloc(sizeof(struct vg_vertex)*VG_NORIGINS, M_VG);
	vg->origin_radius = Malloc(sizeof(float)*VG_NORIGINS, M_VG);
	vg->origin_color = Malloc(sizeof(Uint32)*VG_NORIGINS, M_VG);
	vg->pobj = NULL;
	vg->map = NULL;
	vg->layers = Malloc(sizeof(struct vg_layer), M_VG);
	vg->nlayers = 0;
	vg->cur_layer = 0;
	vg->cur_block = NULL;
	vg_push_layer(vg, _("Layer 0"));
	vg->snap_mode = VG_GRID;
	vg->ortho_mode = VG_NO_ORTHO;
	vg->mask = NULL;
	TAILQ_INIT(&vg->vges);
	TAILQ_INIT(&vg->blocks);
	TAILQ_INIT(&vg->txtstyles);
	pthread_mutex_init(&vg->lock, &recursive_mutexattr);

	for (i = 0; i < VG_NORIGINS; i++) {
		vg->origin[i].x = 0;
		vg->origin[i].y = 0;
		vg->origin[i].z = 0;
		vg->origin[i].w = 1.0;
		vg->origin_radius[i] = 0.0625;
		vg->origin_color[i] = SDL_MapRGB(vfmt, 0, 0, 180);
	}
	vg->origin_radius[0] = 0.25;
	vg->origin_radius[1] = 0.125;
	vg->origin_radius[2] = 0.0625;
	vg->origin_color[0] = SDL_MapRGB(vfmt, 0, 200, 0);
	vg->origin_color[1] = SDL_MapRGB(vfmt, 0, 150, 0);
	vg->origin_color[2] = SDL_MapRGB(vfmt, 0, 50, 150);
	vg->norigin = VG_NORIGINS;
}

static void
vg_free_element(struct vg_element *vge)
{
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
		vg_free_element(vge);
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
vg_destroy_txtstyles(struct vg *vg)
{
	struct vg_text_style *ts, *nts;

	for (ts = TAILQ_FIRST(&vg->txtstyles);
	     ts != TAILQ_END(&vg->txtstyles);
	     ts = nts) {
		nts = TAILQ_NEXT(ts, txtstyles);
		Free(ts, M_VG);
	}
	TAILQ_INIT(&vg->txtstyles);
}


void
vg_destroy(struct vg *vg)
{
	struct object *ob = vg->pobj;
	int y;

	Free(vg->origin, M_VG);
	Free(vg->origin_radius, M_VG);
	Free(vg->origin_color, M_VG);

	if (ob->gfx != NULL) {
		gfx_destroy(ob->gfx);
		ob->gfx = NULL;
	}
	Free(vg->layers, M_VG);
#if 0
	if (vg->mask != NULL) {
		for (y = 0; y < vg->map->maph; y++) {
			Free(vg->mask[y], M_VG);
		}
		Free(vg->mask, M_VG);
	}
#endif
	vg_destroy_elements(vg);
	vg_destroy_blocks(vg);
	vg_destroy_txtstyles(vg);
	pthread_mutex_destroy(&vg->lock);
}

void
vg_destroy_element(struct vg *vg, struct vg_element *vge)
{
	if (vge->block != NULL)
		TAILQ_REMOVE(&vge->block->vges, vge, vgbmbs);

	TAILQ_REMOVE(&vg->vges, vge, vges);
	vg_free_element(vge);
	vg->redraw = 1;
}

/* Generate tile-sized fragments of the raster surface. */
void
vg_update_fragments(struct vg *vg)
{
	struct map *rmap = vg->map;
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
	     y < vg->su->h;
	     y += TILESZ, my++) {
		for (x = 0, mx = 0;
		     x < vg->su->w;
		     x += TILESZ, mx++) {
			struct node *n = &rmap->map[my][mx];
			SDL_Surface *fragsu = NULL;
			Uint32 saflags, scflags, scolorkey;
			Uint8 salpha;
			int fw, fh;

#if 0
			if (vg->mask[my][mx])
				continue;
#endif

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
				    r->layer == rmap->cur_layer &&
				    r->r_sprite.obj == pobj) {
					fragsu = SPRITE(pobj, r->r_sprite.offs);
					break;
				}
			}
			if (r == NULL) {
				fragsu = SDL_CreateRGBSurface(SDL_SWSURFACE|
				    saflags|scflags, fw, fh,
				    vg->su->format->BitsPerPixel,
				    vg->su->format->Rmask,
				    vg->su->format->Gmask,
				    vg->su->format->Bmask,
				    vg->su->format->Amask);
				if (fragsu == NULL)
					fatal("SDL_CreateRGBSurface: %s",
					    SDL_GetError());
			}
			
			sd.x = x;
			sd.y = y;

			SDL_SetAlpha(vg->su, 0, 0);
			SDL_SetColorKey(vg->su, 0, 0);
			SDL_BlitSurface(vg->su, &sd, fragsu, &rd);
			SDL_SetAlpha(vg->su, saflags, salpha);
			SDL_SetColorKey(vg->su, scflags, scolorkey);

			if (r == NULL) {
				node_add_sprite(rmap, n, vg->pobj,
				    gfx_insert_sprite(vg->pobj->gfx, fragsu));
			}
		}
	}
}

void
vg_destroy_fragments(struct vg *vg)
{
	struct gfx *gfx = vg->pobj->gfx;
	Uint32 i;

	for (i = 0; i < gfx->nsprites; i++) {
		SDL_FreeSurface(gfx->sprites[i]);
	}
	for (i = 0; i < gfx->nsubmaps; i++) {
		object_destroy(gfx->submaps[i]);
		Free(gfx->submaps[i], M_OBJECT);
	}
	gfx->nsprites = 0;
	gfx->nsubmaps = 0;
}

/* Mark every element as dirty. */
void
vg_redraw_elements(struct vg *vg)
{
	struct vg_element *vge;

	TAILQ_FOREACH(vge, &vg->vges, vges)
		vge->redraw = 1;
}

/* Adjust the vg bounding box and scaling factor. */
void
vg_scale(struct vg *vg, double w, double h, double scale)
{
	int pw = (int)(w*scale*TILESZ);
	int ph = (int)(h*scale*TILESZ);
	int mw, mh;
	Uint32 suflags;
	int y;

	if (scale < 0)
		fatal("neg scale");

	vg->scale = scale;
	vg->w = w;
	vg->h = h;
	dprintf("%.02fx%.02f*%.02f, %dx%d pixels\n", w, h, scale, pw, ph);

	if (vg->su != NULL) {
		SDL_FreeSurface(vg->su);
	}
	suflags = SDL_SWSURFACE;
	suflags |= (vg->flags & VG_ANTIALIAS) ? SDL_SRCALPHA : SDL_SRCCOLORKEY;
	if ((vg->su = SDL_CreateRGBSurface(suflags, pw, ph, vfmt->BitsPerPixel,
	    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, vfmt->Amask)) == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

	vg_destroy_fragments(vg);
	map_free_nodes(vg->map);
	
	mw = vg->su->w/TILESZ+1;
	mh = vg->su->h/TILESZ+1;

	if (map_alloc_nodes(vg->map, mw, mh) == -1)
		fatal("%s", error_get());

#if 0
	if (vg->mask != NULL) {
		for (y = 0; y < mh; y++) {
			Free(vg->mask[y], M_VG);
		}
		Free(vg->mask, M_VG);
	}
	vg->mask = Malloc(mh*sizeof(int), M_VG);
	for (y = 0; y < mh; y++) {
		vg->mask[y] = Malloc(mw*sizeof(int), M_VG);
		memset(vg->mask[y], 0, mw*sizeof(int));
	}
#endif
	vg_redraw_elements(vg);
}

/* Allocate the given type of element and begin its parametrization. */
struct vg_element *
vg_begin_element(struct vg *vg, enum vg_element_type eltype)
{
	struct vg_element *vge;
	struct vg_element_ops *vgops;
	int i;

	vge = Malloc(sizeof(struct vg_element), M_VG);
	vge->flags = 0;
	vge->type = eltype;
	vge->layer = vg->cur_layer;
	vge->redraw = 1;
	vge->drawn = 0;
	vge->vtx = NULL;
	vge->nvtx = 0;
	vge->line.style = VG_CONTINUOUS;
	vge->line.stipple = 0x0;
	vge->line.thickness = 1;
	vge->fill.style = VG_NOFILL;
	vge->fill.tex.gfx_obj = NULL;
	vge->fill.tex.gfx_index = 0;
	vge->fill.color = SDL_MapRGB(vfmt, 255, 255, 255);
	vge->color = SDL_MapRGB(vfmt, 255, 255, 255);

	pthread_mutex_lock(&vg->lock);
	TAILQ_INSERT_HEAD(&vg->vges, vge, vges);

	/* Insert into the current block if there is any. */
	if (vg->cur_block != NULL) {
		TAILQ_INSERT_TAIL(&vg->cur_block->vges, vge, vgbmbs);
		vge->block = vg->cur_block;
		if (vge->block->flags & VG_BLOCK_NOSAVE)
			vge->flags |= VG_ELEMENT_NOSAVE;
	} else {
		vge->block = NULL;
	}

	/* Set up the generic operation vector. */
	vge->ops = vge_types[eltype];
	if (vge->ops->init != NULL)
		vge->ops->init(vg, vge);

	vg->redraw = 1;
	pthread_mutex_unlock(&vg->lock);

	return (vge);
}

/*
 * Clear the surface with the filling color.
 * The vg must be locked.
 */
void
vg_clear(struct vg *vg)
{
	SDL_FillRect(vg->su, NULL, vg->fill_color);
	vg->redraw = 1;
}

#ifdef DEBUG
static void
vg_draw_bboxes(struct vg *vg)
{
	struct vg_rect bbox;
	struct vg_element *vge;
	const struct vg_element_ops *vgops;
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
		vg_line_primitive(vg, x, y, x+w, y, vg->grid_color);
		vg_line_primitive(vg, x, y, x, y+h, vg->grid_color);
		vg_line_primitive(vg, x, y+h, x+w, y+h, vg->grid_color);
		vg_line_primitive(vg, x+w, y, x+w, y+h, vg->grid_color);
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
static void
vg_rasterize_element(struct vg *vg, struct vg_element *vge)
{
	struct vg_element *ovge;
	struct vg_rect r1, r2;

	if (!vge->drawn) {
		vge->drawn = 1;
		vge->ops->draw(vg, vge);
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
	int i;

	pthread_mutex_lock(&vg->lock);

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
		if (vge->redraw)
			vg_rasterize_element(vg, vge);
	}
	if (vg->flags & VG_VISORIGIN)
		vg_draw_origin(vg);

	vg_update_fragments(vg);

	pthread_mutex_unlock(&vg->lock);
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
 * Translate absolute vg coordinates to raster coordinates.
 * The vg must be locked.
 */
void
vg_arcoords2(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ);
}

/*
 * Translate vg length to pixel length.
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
		vge->vtx = Realloc(vge->vtx,
		    (vge->nvtx+1)*sizeof(struct vg_vertex), M_VG);
	}
	return (&vge->vtx[vge->nvtx++]);
}

/* Pop the highest vertex off the vertex array. */
void
vg_pop_vertex(struct vg *vg)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	if (vge->vtx == NULL)
		return;

	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(struct vg_vertex),
	    M_VG);
}

/* Push a 2D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex2(struct vg *vg, double x, double y)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(TAILQ_FIRST(&vg->vges));
	vtx->x = x;
	vtx->y = y;
	vtx->z = 0;
	vtx->w = 1.0;
	vg_block_offset(vg, vtx);
	return (vtx);
}

/* Push a 3D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex3(struct vg *vg, double x, double y, double z)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(TAILQ_FIRST(&vg->vges));
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = 1.0;
	vg_block_offset(vg, vtx);
	return (vtx);
}

/* Push a homogenized 3D vertex onto the vertex array. */
struct vg_vertex *
vg_vertex4(struct vg *vg, double x, double y, double z, double w)
{
	struct vg_vertex *vtx;

	vtx = vg_alloc_vertex(TAILQ_FIRST(&vg->vges));
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = w;
	vg_block_offset(vg, vtx);
	return (vtx);
}

/* Push a series of vertices onto the vertex array. */
void
vg_vertex_array(struct vg *vg, const struct vg_vertex *svtx, unsigned int nsvtx)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);
	unsigned int i;
	
	for (i = 0; i < nsvtx; i++) {
		struct vg_vertex *vtx;

		vtx = vg_alloc_vertex(vge);
		memcpy(vtx, &svtx[i], sizeof(struct vg_vertex));
		vg_block_offset(vg, vtx);
	}
}

/* Specify the layer# to associate with the current element. */
void
vg_layer(struct vg *vg, int layer)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->layer = layer;
}

/* Specify the color of the current element (format-specific). */
void
vg_color(struct vg *vg, Uint32 color)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = color;
}

/* Specify the color of the current element (RGB triplet). */
void
vg_color3(struct vg *vg, int r, int g, int b)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = SDL_MapRGB(vfmt, r, g, b);
}

/* Specify the color of the current element (RGB triplet + alpha). */
void
vg_color4(struct vg *vg, int r, int g, int b, int a)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = SDL_MapRGBA(vfmt, r, g, b, a);
}

/* Push a new layer onto the layer stack. */
struct vg_layer *
vg_push_layer(struct vg *vg, const char *name)
{
	struct vg_layer *vgl;

	vg->layers = Realloc(vg->layers, (vg->nlayers+1) *
	                                 sizeof(struct vg_layer), M_VG);
	vgl = &vg->layers[vg->nlayers];
	vg->nlayers++;

	strlcpy(vgl->name, name, sizeof(vgl->name));
	vgl->visible = 1;
	vgl->alpha = 255;
	vgl->color = SDL_MapRGB(vfmt, 255, 255, 255);
	return (vgl);
}

/* Pop the highest layer off the layer stack. */
void
vg_pop_layer(struct vg *vg)
{
	if (--vg->nlayers < 1)
		vg->nlayers = 1;
}

void
vg_save(struct vg *vg, struct netbuf *buf)
{
	off_t nblocks_offs, nvges_offs;
	Uint32 nblocks = 0, nvges = 0;
	struct vg_block *vgb;
	struct vg_element *vge;
	int i;

	version_write(buf, &vg_ver);

	pthread_mutex_lock(&vg->lock);

	write_string(buf, vg->name);
	write_uint32(buf, (Uint32)vg->flags);
	write_double(buf, vg->w);
	write_double(buf, vg->h);
	write_double(buf, vg->scale);
	write_color(buf, vfmt, vg->fill_color);
	write_color(buf, vfmt, vg->grid_color);
	write_double(buf, vg->grid_gap);
	write_uint32(buf, (Uint32)vg->cur_layer);

	/* Save the origin points. */
	write_uint32(buf, vg->norigin);
	for (i = 0; i < vg->norigin; i++) {
		write_vertex(buf, &vg->origin[i]);
		write_float(buf, vg->origin_radius[i]);
		write_color(buf, vfmt, vg->origin_color[i]);
	}

	/* Save the layer information. */
	write_uint32(buf, vg->nlayers);
	for (i = 0; i < vg->nlayers; i++) {
		struct vg_layer *layer = &vg->layers[i];

		write_string(buf, layer->name);
		write_uint8(buf, (Uint8)layer->visible);
		write_color(buf, vfmt, layer->color);
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
		nblocks++;
	}
	pwrite_uint32(buf, nblocks, nblocks_offs);

	/* Save the vg elements. */
	nvges_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->flags & VG_ELEMENT_NOSAVE)
			continue;

		write_uint32(buf, (Uint32)vge->type);
		write_string(buf, vge->block != NULL ? vge->block->name : NULL);
		write_uint32(buf, (Uint32)vge->layer);
		write_color(buf, vfmt, vge->color);

		write_uint32(buf, vge->nvtx);
		for (i = 0; i < vge->nvtx; i++) {
			write_vertex(buf, &vge->vtx[i]);
		}
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
			write_uint32(buf, (Uint32)vge->vg_text.align);
			write_string(buf, vge->vg_text.style);
			write_string(buf, vge->vg_text.face);
			write_uint32(buf, (Uint32)vge->vg_text.size);
			write_uint32(buf, (Uint32)vge->vg_text.flags);
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
	Uint32 norigin, nlayers, nelements, nblocks;
	Uint32 i;

	if (version_read(buf, &vg_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&vg->lock);
	copy_string(vg->name, buf, sizeof(vg->name));
	vg->flags = read_uint32(buf);
	vg->w = read_double(buf);
	vg->h = read_double(buf);
	vg->scale = read_double(buf);
	vg->fill_color = read_color(buf, vfmt);
	vg->grid_color = read_color(buf, vfmt);
	vg->grid_gap = read_double(buf);
	vg->cur_layer = (int)read_uint32(buf);
	vg->cur_block = NULL;
	dprintf("name `%s' bbox %.2fx%.2f scale %.2f\n", vg->name, vg->w, vg->h,
	    vg->scale);

	/* Read the origin points. */
	if ((norigin = read_uint32(buf)) < 1) {
		error_set("norigin < 1");
		goto fail;
	}
	vg->origin = Realloc(vg->origin, norigin*sizeof(struct vg_vertex),
	    M_VG);
	vg->origin_radius = Realloc(vg->origin_radius, norigin*sizeof(float),
	    M_VG);
	vg->origin_color = Realloc(vg->origin_color, norigin*sizeof(Uint32),
	    M_VG);
	vg->norigin = norigin;
	for (i = 0; i < vg->norigin; i++) {
		read_vertex(buf, &vg->origin[i]);
		vg->origin_radius[i] = read_float(buf);
		vg->origin_color[i] = read_color(buf, vfmt);
	}
	dprintf("%d origin vertices\n", vg->norigin);

	/* Read the layer information. */
	if ((nlayers = read_uint32(buf)) < 1) {
		error_set("nlayers < 1");
		goto fail;
	}
	vg->layers = Realloc(vg->layers, nlayers*sizeof(struct vg_layer),
	    M_VG);
	for (i = 0; i < nlayers; i++) {
		struct vg_layer *layer = &vg->layers[i];

		dprintf("layer %d: name `%s' vis %d\n", i, layer->name,
		    layer->visible);
		copy_string(layer->name, buf, sizeof(layer->name));
		layer->visible = (int)read_uint8(buf);
		layer->color = read_color(buf, vfmt);
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
		TAILQ_INIT(&vgb->vges);
		TAILQ_INSERT_TAIL(&vg->blocks, vgb, vgbs);
	}

	/* Read the vg elements */
	vg_destroy_elements(vg);
	nelements = read_uint32(buf);
	dprintf("%u elements\n", nelements);
	for (i = 0; i < nelements; i++) {
		enum vg_element_type type;
		struct vg_element *vge;
		struct vg_block *block;
		char *block_id;
		Uint32 nlayer, color;
		int j;
	
		type = (enum vg_element_type)read_uint32(buf);
		block_id = read_string_len(buf, VG_BLOCK_NAME_MAX);
		nlayer = (int)read_uint32(buf);
		color = read_color(buf, vfmt);

		if (block_id != NULL) {
			TAILQ_FOREACH(block, &vg->blocks, vgbs) {
				if (strcmp(block->name, block_id) == 0)
					break;
			}
			if (block == NULL) {
				error_set("bad block id");
				Free(block_id, 0);
				goto fail;
			}
			Free(block_id, 0);
		} else {
			block = NULL;
		}

		vge = vg_begin_element(vg, type);
		vge->nvtx = read_uint32(buf);
		vge->vtx = Malloc(vge->nvtx*sizeof(struct vg_vertex), M_VG);
		for (j = 0; j < vge->nvtx; j++) {
			read_vertex(buf, &vge->vtx[j]);
		}
		if (block != NULL) {
			TAILQ_INSERT_TAIL(&block->vges, vge, vgbmbs);
			vge->block = block;
		}

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
			vge->vg_text.align = (enum vg_alignment)
			    read_uint32(buf);
			copy_string(vge->vg_text.style, buf,
			    sizeof(vge->vg_text.style));
			copy_string(vge->vg_text.face, buf,
			    sizeof(vge->vg_text.face));
			vge->vg_text.size = (int)read_uint32(buf);
			vge->vg_text.flags = (int)read_uint32(buf);
			break;
		case VG_MASK:
			vge->vg_mask.scale = read_float(buf);
			vge->vg_mask.visible = (int)read_uint8(buf);
			read_string(buf);			       /* Pad */
			break;
		default:
			break;
		}
	}

	vg->redraw++;
	pthread_mutex_unlock(&vg->lock);
	return (0);
fail:
	pthread_mutex_unlock(&vg->lock);
	return (-1);
}

#ifdef EDITION

void
vg_geo_changed(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;

	vg_scale(vg, vg->w, vg->h, vg->scale);
	vg->redraw++;
}

void
vg_changed(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;

	vg->redraw++;
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

#endif /* EDITION */
