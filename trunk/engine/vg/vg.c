/*	$Csoft: vg.c,v 1.7 2004/04/20 01:05:43 vedge Exp $	*/

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

static const struct vg_element_ops vge_types[] = {
	{ VG_LINES,		vg_draw_line_segments,	vg_line_bbox },
	{ VG_LINE_STRIP,	vg_draw_line_strip,	vg_line_bbox },
	{ VG_LINE_LOOP,		vg_draw_line_loop,	vg_line_bbox },
	{ VG_POINTS,		vg_draw_points,		vg_points_bbox },
	{ VG_CIRCLE,		vg_draw_circle,		vg_circle_bbox },
	{ VG_ELLIPSE,		vg_draw_ellipse,	vg_ellipse_bbox }
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
	int i;

	vg->flags = flags;
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
	vg_push_layer(vg, _("Layer 0"));
	vg->snap_mode = VG_GRID;
	TAILQ_INIT(&vg->vges);
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

void
vg_destroy(struct vg *vg)
{
	struct vg_element *vge, *nvge;
	struct object *ob = vg->pobj;

	Free(vg->origin, M_VG);
	Free(vg->origin_radius, M_VG);
	Free(vg->origin_color, M_VG);

	if (ob->gfx != NULL) {
		vg_destroy_fragments(vg);
		gfx_destroy(ob->gfx);
		ob->gfx = NULL;
	}
	Free(vg->layers, M_VG);

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

/* Regenerate fragments of the raster surface that must be redrawn. */
void
vg_regen_fragments(struct vg *vg)
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

			/* TODO if masked, skip */
			
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

/* Adjust the vg bounding box and scaling factor. */
void
vg_scale(struct vg *vg, double w, double h, double scale)
{
	int pw = (int)(w*scale*TILESZ);
	int ph = (int)(h*scale*TILESZ);
	Uint32 suflags;
	struct vg_element *vge;

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
	if ((vg->su = SDL_CreateRGBSurface(suflags, pw, ph, vfmt->BitsPerPixel,
	    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, vfmt->Amask)) == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

	vg_destroy_fragments(vg);
	map_free_nodes(vg->map);

	if (map_alloc_nodes(vg->map, vg->su->w/TILESZ+1, vg->su->h/TILESZ+1)
	    == -1) {
		fatal("%s", error_get());
	}
	TAILQ_FOREACH(vge, &vg->vges, vges)
		vge->redraw++;
}

/* Allocate the given type of element and begin its parametrization. */
struct vg_element *
vg_begin(struct vg *vg, enum vg_element_type eltype)
{
	struct vg_element *vge;

	vge = Malloc(sizeof(struct vg_element), M_VG);
	vge->type = eltype;
	vge->layer = 0;
	vge->redraw = 1;
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
	vge->bbox.x = 0;
	vge->bbox.y = 0;
	vge->bbox.w = 0;
	vge->bbox.h = 0;
	TAILQ_INSERT_HEAD(&vg->vges, vge, vges);

	switch (eltype) {
	case VG_POINTS:
		vge->vg_point.radius = 0.05;
		break;
	case VG_CIRCLE:
		vge->vg_circle.radius = 0.025;
		break;
	case VG_ARC:
	case VG_ELLIPSE:
		vge->vg_arc.w = 1;
		vge->vg_arc.h = 1;
		vge->vg_arc.s = 0;
		vge->vg_arc.e = 360;
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

static void
vg_draw_bboxes(struct vg *vg)
{
	struct vg_element *vge;
	int x, y, w, h;

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		vg_rcoords2(vg, vge->bbox.x, vge->bbox.y, &x, &y);
		vg_rlength(vg, vge->bbox.w, &w);
		vg_rlength(vg, vge->bbox.h, &h);
		vg_line_primitive(vg, x, y, x+w, y, vg->grid_color);
		vg_line_primitive(vg, x, y, x, y+h, vg->grid_color);
		vg_line_primitive(vg, x, y+h, x+w, y+h, vg->grid_color);
		vg_line_primitive(vg, x+w, y, x+w, y+h, vg->grid_color);
	}
}

void
vg_rasterize(struct vg *vg)
{
	struct vg_element *vge;
	int i;

	vg_clear(vg);

	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < nvge_types; i++) {
			if (vge_types[i].type == vge->type) {
#if 0
				vge_types[i].bbox(vg, vge, &vge->bbox);
#endif
				vge_types[i].draw(vg, vge);
			}
		}
	}
	if (vg->flags & VG_VISORIGIN)
		vg_draw_origin(vg);
	if (vg->flags & VG_VISGRID)
		vg_draw_grid(vg);
	if (vg->flags & VG_VISBBOXES)
		vg_draw_bboxes(vg);

	vg_regen_fragments(vg);
}

/* Translate tile coordinates to relative vg coordinates. */
void
vg_vcoords2(struct vg *vg, int rx, int ry, int xoff, int yoff, double *vx,
    double *vy)
{
	*vx = (double)rx/vg->scale + (double)xoff/vg->scale/TILESZ -
	    vg->origin[0].x/vg->scale;
	*vy = (double)ry/vg->scale + (double)yoff/vg->scale/TILESZ -
	    vg->origin[0].y/vg->scale;

	if (vg->snap_mode != VG_FREE_POSITIONING)
		vg_snap_to(vg, vx, vy);
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
