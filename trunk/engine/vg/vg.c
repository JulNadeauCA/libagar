/*	$Csoft: vg.c,v 1.13 2004/04/30 05:24:02 vedge Exp $	*/

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

static const struct vg_element_ops vge_types[] = {
	{
		VG_LINES,
		N_("Line segments"),
		NULL,
		vg_draw_line_segments,
		vg_line_bbox
	},
	{
		VG_LINE_STRIP,
		N_("Line strip"),
		NULL,
		vg_draw_line_strip,
		vg_line_bbox
	},
	{
		VG_LINE_LOOP,
		N_("Line loop"),
		NULL,
		vg_draw_line_loop,
		vg_line_bbox
	},
	{
		VG_POINTS,
		N_("Points"),
		vg_point_init,
		vg_draw_points,
		vg_points_bbox
	},
	{
		VG_CIRCLE,
		N_("Circle"),
		vg_circle_init,
		vg_draw_circle,
		vg_circle_bbox
	},
	{
		VG_ELLIPSE,
		N_("Ellipse"),
		vg_ellipse_init,
		vg_draw_ellipse,
		vg_ellipse_bbox
	}
};
const int nvge_types = sizeof(vge_types) / sizeof(vge_types[0]);

struct vg *
vg_new(void *p, int flags)
{
	char path[OBJECT_PATH_MAX];
	struct object *ob = p;
	struct vg *vg;
	
	vg = Malloc(sizeof(struct vg), M_VG);
	vg_init(vg, flags);
	vg->pobj = ob;
	gfx_new_pvt(ob, "vg");
	vg->map = map_new(ob, "raster");
	OBJECT(vg->map)->flags |= OBJECT_NON_PERSISTENT|OBJECT_INDESTRUCTIBLE;
	return (vg);
}

void
vg_init(struct vg *vg, int flags)
{
	int i, x, y;

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

void
vg_destroy(struct vg *vg)
{
	struct vg_element *vge, *nvge;
	struct vg_block *vgb, *nvgb;
	struct object *ob = vg->pobj;
	int y;

	Free(vg->origin, M_VG);
	Free(vg->origin_radius, M_VG);
	Free(vg->origin_color, M_VG);

	if (ob->gfx != NULL) {
		vg_destroy_fragments(vg);
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
	for (vge = TAILQ_FIRST(&vg->vges);
	     vge != TAILQ_END(&vg->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vges);
		vg_free_element(vge);
	}
	for (vgb = TAILQ_FIRST(&vg->blocks);
	     vgb != TAILQ_END(&vg->blocks);
	     vgb = nvgb) {
		nvgb = TAILQ_NEXT(vgb, vgbs);
		Free(vgb, M_VG);
	}
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
	int i;

	vge = Malloc(sizeof(struct vg_element), M_VG);
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

	TAILQ_INSERT_HEAD(&vg->vges, vge, vges);

	if (vg->cur_block != NULL) {
		TAILQ_INSERT_HEAD(&vg->cur_block->vges, vge, vgbmbs);
		vge->block = vg->cur_block;
	} else {
		vge->block = NULL;
	}

	for (i = 0; i < nvge_types; i++) {
		if (vge_types[i].type == eltype) {
			vge->ops.name = vge_types[i].name;
			vge->ops.init = vge_types[i].init;
			vge->ops.draw = vge_types[i].draw;
			vge->ops.bbox = vge_types[i].bbox;
			break;
		}
	}
	if (i == nvge_types) {
		fatal("no such element type");
	}
	if (vge->ops.init != NULL)
		vge->ops.init(vg, vge);

	vg->redraw = 1;
	return (vge);
}

/* Clear the surface with the filling color. */
void
vg_clear(struct vg *vg)
{
	SDL_FillRect(vg->su, NULL, vg->fill_color);
	vg->redraw = 1;
}

static void
vg_draw_bboxes(struct vg *vg)
{
	struct vg_rect bbox;
	struct vg_element *vge;
	int x, y, w, h;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < nvge_types; i++) {
			if (vge_types[i].type == vge->type)
				vge_types[i].bbox(vg, vge, &bbox);
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

/* Evaluate collision between two rectangles. */
int
vg_collision(struct vg *vg, struct vg_rect *r1, struct vg_rect *r2)
{
	return (1);
}

/*
 * Rasterize an element and the other elements coming in contact with it,
 * in a recursive fashion.
 */
static void
vg_rasterize_element(struct vg *vg, struct vg_element *vge)
{
	struct vg_element *ovge;
	struct vg_rect r1, r2;

	if (!vge->drawn) {
		vge->drawn = 1;
		vge->ops.draw(vg, vge);
	}

	/* Evaluate collisions with other elements. */
	vge->ops.bbox(vg, vge, &r1);
	TAILQ_FOREACH(ovge, &vg->vges, vges) {
		if (ovge->drawn || ovge == vge) {
			continue;
		}
		ovge->ops.bbox(vg, ovge, &r2);
		if (vg_collision(vg, &r1, &r2))
			vg_rasterize_element(vg, ovge);
	}
}

/* Rasterize elements marked dirty and update the fragments. */
void
vg_rasterize(struct vg *vg)
{
	struct vg_element *vge;
	int i;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vge->drawn = 0;
	}
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		if (vge->redraw)
			vg_rasterize_element(vg, vge);
	}
	if (vg->flags & VG_VISORIGIN)
		vg_draw_origin(vg);
	if (vg->flags & VG_VISGRID)
		vg_draw_grid(vg);
	if (vg->flags & VG_VISBBOXES)
		vg_draw_bboxes(vg);

	vg_update_fragments(vg);
}

/*
 * Translate tile coordinates to relative vg coordinates, applying
 * positional and orthogonal restrictions as needed.
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

/* Translate tile coordinates to absolute vg coordinates. */
void
vg_avcoords2(struct vg *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/TILESZ;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/TILESZ;

	if (vg->snap_mode != VG_FREE_POSITIONING)
		vg_snap_to(vg, vx, vy);
}

/* Translate relative vg coordinates to raster coordinates. */
void
vg_rcoords2(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ) +
	      (int)(vg->origin[0].x*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ) +
	      (int)(vg->origin[0].y*vg->scale*TILESZ);
}

/* Translate absolute vg coordinates to raster coordinates. */
void
vg_arcoords2(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ);
}

/* Translate vg length to pixel length. */
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

void
vg_pop_vertex(struct vg *vg)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	if (vge->vtx == NULL)
		return;

	vge->vtx = Realloc(vge->vtx, (--vge->nvtx)*sizeof(struct vg_vertex),
	    M_VG);
}

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

int
vg_near_vertex2(struct vg *vg, const struct vg_vertex *vtx, double x, double y,
    double fudge)
{
	return (x > vtx->x-fudge && x < vtx->x+fudge &&
	        y > vtx->y-fudge && y < vtx->y+fudge);
}

void
vg_layer(struct vg *vg, int layer)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->layer = layer;
}

void
vg_color(struct vg *vg, Uint32 color)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = color;
}

void
vg_color3(struct vg *vg, int r, int g, int b)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = SDL_MapRGB(vfmt, r, g, b);
}

void
vg_color4(struct vg *vg, int r, int g, int b, int a)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->color = SDL_MapRGBA(vfmt, r, g, b, a);
}

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

void
vg_pop_layer(struct vg *vg)
{
	if (--vg->nlayers < 1)
		vg->nlayers = 1;
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
