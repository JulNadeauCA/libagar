/*	$Csoft: vg.c,v 1.4 2004/04/10 03:01:17 vedge Exp $	*/

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

#include "vg.h"
#include "vg_primitive.h"

static const struct {
	enum vg_element_type type;
	void (*draw_func)(struct vg *, struct vg_element *vge);
} vge_types[] = {
	{ VG_LINES,		vg_draw_lines },
	{ VG_LINE_STRIP,	vg_draw_line_strip },
	{ VG_LINE_LOOP,		vg_draw_line_loop },
	{ VG_POINTS,		vg_draw_points },
	{ VG_CIRCLE,		vg_draw_circle }
};
const int nvge_types = sizeof(vge_types) / sizeof(vge_types[0]);

/* Create a new vector drawing. */
struct vg *
vg_new(void *p, int flags)
{
	char path[OBJECT_PATH_MAX];
	struct object *ob = p;
	struct vg *vg;
	
	vg = Malloc(sizeof(struct vg), M_VG);
	vg->flags = flags;
	vg->w = 0;
	vg->h = 0;
	vg->scale = 1;
	vg->fill_color = SDL_MapRGB(vfmt, 0, 0, 0);
	vg->origin_color = SDL_MapRGB(vfmt, 200, 200, 255);
	vg->su = NULL;
	vg->pobj = ob;
	vg->ox = 0.5;
	vg->oy = 0.5;
	TAILQ_INIT(&vg->vges);
	pthread_mutex_init(&vg->lock, &recursive_mutexattr);

	gfx_new_pvt(ob);
	vg->map = map_new(ob, "raster");
	OBJECT(vg->map)->flags |= OBJECT_NON_PERSISTENT|OBJECT_INDESTRUCTIBLE;
	return (vg);
}

void
vg_destroy(struct vg *vg)
{
	struct vg_element *vge, *nvge;
	struct object *ob = vg->pobj;

	if (ob->gfx != NULL) {
		vg_destroy_fragments(vg);
		gfx_destroy(ob->gfx);
		ob->gfx = NULL;
	}

	for (vge = TAILQ_FIRST(&vg->vges);
	     vge != TAILQ_END(&vg->vges);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, vges);
		Free(vge->vtx, M_VG);
		Free(vge, M_VG);
	}
	pthread_mutex_destroy(&vg->lock);
}

void
vg_undo_element(struct vg *vg, struct vg_element *vge)
{
	TAILQ_REMOVE(&vg->vges, vge, vges);
	Free(vge->vtx, M_VG);
	Free(vge, M_VG);
}

/* Regenerate fragments of the raster surface. */
void
vg_regen_fragments(struct vg *vg)
{
	struct map *m = vg->map;
	struct map *fmap;
	struct noderef *r;
	int x, y;

	vg_destroy_fragments(vg);
	fmap = gfx_insert_fragments(vg->pobj->gfx, vg->su);

	map_free_nodes(m);
	if (map_alloc_nodes(m, fmap->mapw, fmap->maph) == -1)
		fatal("%s", error_get());

	for (y = 0; y < fmap->maph; y++) {
		for (x = 0; x < fmap->mapw; x++) {
			struct node *n = &m->map[y][x];
			struct noderef *r;
			
			node_clear(m, n, m->cur_layer);
			node_copy(fmap, &fmap->map[y][x], -1, m, n,
			    m->cur_layer);
			
			TAILQ_FOREACH(r, &n->nrefs, nrefs) {
				if (r->layer != m->cur_layer) {
					continue;
				}
				switch (r->type) {
				case NODEREF_SPRITE:
					if (r->r_sprite.obj != NULL) {
						break;
					}
					r->r_sprite.obj = vg->pobj;
					object_add_dep(m, vg->pobj);
					object_page_in(vg->pobj, OBJECT_GFX);
					break;
				default:
					break;
				}
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

/* Scale the vg to the given geometry (where 1.0 == 1 tile). */
void
vg_scale(struct vg *vg, double w, double h, double scale)
{
	int pw = (int)(w*scale*TILESZ);
	int ph = (int)(h*scale*TILESZ);
	Uint32 suflags;

#ifdef DEBUG
	if (scale < 0)
		fatal("neg scale");
#endif
	vg->w = w;
	vg->h = h;
	vg->scale = scale;

	dprintf("%.02fx%.02f*%.02f, %dx%d pixels\n", w, h, scale, pw, ph);

	if (vg->su != NULL) {
		SDL_FreeSurface(vg->su);
	}
	suflags = SDL_SWSURFACE;
	suflags |= (vg->flags & VG_ANTIALIAS) ? SDL_SRCALPHA : SDL_SRCCOLORKEY;
	if ((vg->su = SDL_CreateRGBSurface(suflags, pw, ph, 32,
	     vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, vfmt->Amask)) == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
}

/* Allocate the given type of element and begin its parametrization. */
struct vg_element *
vg_begin(struct vg *vg, enum vg_element_type eltype)
{
	struct vg_element *vge;

	vge = Malloc(sizeof(struct vg_element), M_VG);
	vge->type = eltype;
	vge->vtx = NULL;
	vge->nvtx = 0;
	vge->line.style = VG_CONTINUOUS;
	vge->line.stipple = 0x0;
	vge->line.thickness = 1;
	vge->fill.style = VG_NOFILL;
	vge->fill.pat.gfx_obj = NULL;
	vge->fill.pat.gfx_offs = 0;
	vge->fill.color = SDL_MapRGB(vfmt, 255, 255, 255);
	vge->color = SDL_MapRGB(vfmt, 255, 255, 255);
	TAILQ_INSERT_HEAD(&vg->vges, vge, vges);

	switch (eltype) {
	case VG_POINTS:
		vge->vg_point.radius = 0.05;
		break;
	case VG_CIRCLE:
		vge->vg_circle.radius = 0.025;
		break;
	case VG_TEXT:
		vge->vg_text.text[0] = '\0';
		vge->vg_text.angle = 0;
		vge->vg_text.align = VG_ALIGN_MC;
		break;
	default:
		break;
	}
	return (vge);
}



/* Clear the surface with the filling color. */
void
vg_clear(struct vg *vg)
{
	SDL_FillRect(vg->su, NULL, vg->fill_color);
}

static __inline__ void
vg_draw_origin(struct vg *vg)
{
	int rx, ry, radius;

	vg_rcoords(vg, vg->ox, vg->oy, &rx, &ry);
	vg_rlength(vg, 0.325, &radius);
	vg_circle_primitive(vg, rx, ry, radius, vg->origin_color);
	vg_line_primitive(vg, rx, ry, rx+radius, ry, vg->origin_color);
	vg_line_primitive(vg, rx, ry, rx, ry+radius, vg->origin_color);
}

void
vg_rasterize(struct vg *vg)
{
	struct vg_element *vge;
	int i;

	vg_clear(vg);

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < nvge_types; i++) {
			if (vge_types[i].type == vge->type)
				vge_types[i].draw_func(vg, vge);
		}
	}
	if (vg->flags & VG_VISORIGIN)
		vg_draw_origin(vg);

	vg_regen_fragments(vg);
}

void
vg_origin(struct vg *vg, double x, double y)
{
	vg->ox = x;
	vg->oy = y;
}

/* Translate tile coordinates to vg coordinates. */
void
vg_vcoords(struct vg *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/TILESZ -
	    vg->ox/vg->scale;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/TILESZ -
	    vg->oy/vg->scale;
}

/* Translate vg coordinates to raster coordinates. */
void
vg_rcoords(struct vg *vg, double vx, double vy, int *rx, int *ry)
{
	*rx = (int)(vx*vg->scale*TILESZ) + (int)(vg->ox*vg->scale*TILESZ);
	*ry = (int)(vy*vg->scale*TILESZ) + (int)(vg->oy*vg->scale*TILESZ);
#ifdef DEBUG
	if (*rx > vg->su->w || *ry > vg->su->h)
		dprintf("%d,%d > %ux%u\n", *rx, *ry, vg->su->w, vg->su->h);
#endif
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
vg_undo_vertex(struct vg *vg)
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
	}
}

