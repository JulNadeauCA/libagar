/*	$Csoft: vg.c,v 1.1 2004/03/17 06:04:59 vedge Exp $	*/

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
#include <engine/vg/vg.h>

static const struct {
	enum vg_element_type type;
	void (*draw_func)(struct vg *, struct vg_element *vge);
} vge_types[] = {
	{ VG_POINTS,	vg_draw_points },
};
const int nvge_types = sizeof(vge_types) / sizeof(vge_types[0]);

/* Initialize a new vector graphics context for the given object. */
struct vg *
vg_new(void *p, int flags, float w, float h, float scale, const char *name)
{
	struct vg *vg;
	struct object *ob = p;
	SDL_Surface *su;
	Uint32 suflags = SDL_SWSURFACE;

	vg = Malloc(sizeof(struct vg), M_VG);
	vg->w = w;
	vg->h = h;
	suflags |= (flags & VG_ANTIALIAS) ? SDL_SRCALPHA : SDL_SRCCOLORKEY;
	pthread_mutex_init(&vg->lock, &recursive_mutexattr);

	/* Create the surface for rasterisation. */
	su = SDL_CreateRGBSurface(suflags, (int)(w*scale), (int)(h*scale), 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	);
	if (su == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		goto fail;
	}
	vg->fill_color = SDL_MapRGB(su->format, 0, 0, 0);

	/* Allocate a private gfx structure and map to hold the fragments. */
	ob->gfx = Malloc(sizeof(struct gfx), M_GFX);
	gfx_init(ob->gfx, name);
	vg->map = map_new(ob, name);
	vg_scale(vg, scale);
	return (vg);
fail:
	pthread_mutex_destroy(&vg->lock);
	free(vg);
	return (NULL);
}

void
vg_scale(struct vg *vg, float f)
{
	int mw = (int)(vg->w*f);
	int mh = (int)(vg->h*f);

	//map_resize(vg->map, )
	map_free_nodes(vg->map);
	map_alloc_nodes(vg->map, mw > 0 ? mw : 1, mh > 0 ? mh : 1);
	vg->scale = f;
}

void
vg_destroy(struct vg *vg)
{
	struct vg_element *vge, *nvge;

	for (vge = TAILQ_FIRST(&vg->rasq);
	     vge != TAILQ_END(&vg->rasq);
	     vge = nvge) {
		nvge = TAILQ_NEXT(vge, rasq);
		Free(vge->vertices, M_VG);
		Free(vge, M_VG);
	}

	pthread_mutex_destroy(&vg->lock);
	SDL_FreeSurface(vg->su);
	map_destroy(vg->map);
	Free(vg->map, M_OBJECT);
}

/* Begin drawing the specified type of element. */
struct vg_element *
vg_begin(struct vg *vg, enum vg_element_type eltype)
{
	struct vg_element *vge;

	vge = Malloc(sizeof(struct vg_element), M_VG);
	vge->type = eltype;
	vge->vertices = NULL;
	vge->nvertices = 0;
	TAILQ_INSERT_HEAD(&vg->rasq, vge, rasq);
	return (vge);
}

/* Specify a single vertex. */
void
vg_vertex(struct vg *vg, float x, float y)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->rasq);

	if (vge->vertices == NULL) {
		vge->vertices = Malloc(2*sizeof(float), M_VG);
	} else {
		vge->vertices = Realloc(vge->vertices, (vge->nvertices+2) *
		    sizeof(float), M_VG);
	}
	vge->vertices[vge->nvertices++] = x;
	vge->vertices[vge->nvertices++] = y;
}

/* Specify an array of vertices. */
void
vg_vertices(struct vg *vg, float *vertices, int nvertices)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->rasq);
	int i;
	
	for (i = 0; i < nvertices-1; i++)
		vg_vertex(vg, vertices[i], vertices[i+1]);
}

/* Move the current element to the tail of the queue. */
void
vg_end(struct vg *vg)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->rasq);

	TAILQ_REMOVE(&vg->rasq, vge, rasq);
	TAILQ_INSERT_TAIL(&vg->rasq, vge, rasq);
}

void
vg_clear(struct vg *vg)
{
	SDL_FillRect(vg->su, NULL, vg->fill_color);
}

/* Perform rasterisation. */
void
vg_render(struct vg *vg)
{
	struct vg_element *vge;
	int i;

	SDL_FillRect(vg->su, NULL, vg->fill_color);

	TAILQ_FOREACH(vge, &vg->rasq, rasq) {
		for (i = 0; i < nvge_types; i++) {
			if (vge_types[i].type == vge->type)
				vge_types[i].draw_func(vg, vge);
		}
	}
}
