/*	$Csoft: map.c,v 1.45 2002/03/01 14:45:04 vedge Exp $	*/

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/text/text.h>
#include <engine/mapedit/mapedit.h>

static struct obvec map_vec = {
	map_destroy,
	map_load,
	map_save,
	NULL,		/* link */
	NULL,		/* unlink */
	map_dump
};

struct draw {
	SDL_Surface *s;		/* Source surface */
	Uint32	x, y;		/* View coordinates */
	Uint32	flags;		/* Node flags (for map editor) */

	TAILQ_ENTRY(draw) pdraws; /* Deferred rendering */
};

static void	 node_init(struct node *, Uint32);
static void	 node_destroy(struct node *);
static void	 map_draw(struct map *);
static void	 map_animate(struct map *);
static void	*map_draw_th(void *);

/*
 * Allocate nodes for the given map geometry.
 * Must be called on a locked map.
 */
void
map_allocnodes(struct map *m, Uint32 w, Uint32 h, Uint32 tilew, Uint32 tileh)
{
	Uint32 i, x, y;

	m->mapw = w;
	m->maph = h;
	m->tilew = tilew;
	m->tileh = tileh;

	/* This is why sprite sizes must be a power of two. */
	for (i = 0; (1 << i) != tilew; i++)
	    ;;
	m->shtilex = i;
	for (i = 1; (1 << i) != tileh; i++)
	    ;;
	m->shtiley = i;

	/* Allocate the two-dimensional node array. */
	m->map = (struct node **)emalloc((w * h) * sizeof(struct node *));
	for (i = 0; i < h; i++) {
		*(m->map + i) = (struct node *)emalloc(w * sizeof(struct node));
	}
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			node_init(&m->map[x][y], 0);
		}
	}
}

/*
 * Free map nodes.
 * Must be called on a locked map.
 */
void
map_freenodes(struct map *m)
{
	Uint32 x, y;

	for (x = 0; x < m->mapw; x++) {
		for (y = 0; y < m->maph; y++) {
			node_destroy(&m->map[x][y]);
		}
		free(*(m->map + x));
	}
	free(m->map);
}

struct map *
map_create(char *name, char *desc, Uint32 flags)
{
	struct map *m;

	m = (struct map *)emalloc(sizeof(struct map));
	memset(m, 0, sizeof(struct map));
	object_init(&m->obj, name, OBJ_DEFERGC|OBJ_EDITABLE, &map_vec);
	m->obj.desc = (desc != NULL) ? strdup(desc) : NULL;
	sprintf(m->obj.saveext, "m");
	m->flags = flags;
	m->view = mainview;
	if (pthread_mutex_init(&m->lock, NULL) != 0) {
		return (NULL);
	}

	return (m);
}

static void *
map_draw_th(void *p)
{
	struct map *m = (struct map *)p;

	while (m->flags & MAP_FOCUSED) {
		pthread_mutex_lock(&m->lock);
		map_animate(m);
		if (m->redraw) {
			m->redraw = 0;
			map_draw(m);
		}
		pthread_mutex_unlock(&m->lock);
		text_drawall();	/* XXX some waste */
		SDL_Delay(2);
	}

	return (NULL);
}

int
map_focus(struct map *m)
{
	m->view->map = m;
	m->flags |= MAP_FOCUSED;
	world->curmap = m;

	if (pthread_create(&m->draw_th, NULL, map_draw_th, m) != 0) {
		dperror("draw_th");
		return (-1);
	}

	return (0);
}

int
map_unfocus(struct map *m)
{
	m->view->map = NULL;
	m->flags &= ~(MAP_FOCUSED);	/* Will stop the rendering thread */
	world->curmap = NULL;

	if (curmapedit != NULL) {
		mapedit_unlink(curmapedit);
	}

	return (0);
}

/*
 * Allocate and link a map node.
 * Must be called on a locked map.
 */
struct noderef *
node_addref(struct node *node, void *ob, Uint32 offs, Uint32 flags)
{
	struct noderef *nref;

	nref = (struct noderef *)emalloc(sizeof(struct noderef));
	nref->pobj = (struct object *)ob;
	nref->offs = offs;
	nref->flags = flags;
	if (flags & MAPREF_ANIM) {
		nref->frame = 0;
		nref->fwait = 0;
		node->nanims++;
	}
	nref->xoffs = 0;
	nref->yoffs = 0;

	TAILQ_INSERT_TAIL(&node->nrefsh, nref, nrefs);
	node->nnrefs++;

	return (nref);
}

/*
 * Pop a reference off the stack and return it.
 * Must be called on a locked map.
 */
struct noderef *
node_popref(struct node *node)
{
	struct noderef *nref;

	if (TAILQ_EMPTY(&node->nrefsh)) {
		return (NULL);
	}
		
	nref = TAILQ_FIRST(&node->nrefsh);
	TAILQ_REMOVE(&node->nrefsh, nref, nrefs);
	node->nnrefs--;

	if (nref->flags & MAPREF_ANIM) {
		node->nanims--;
	}

	return (nref);
}

/*
 * Push a reference onto the stack.
 * Must be called on a locked map.
 */
int
node_pushref(struct node *node, struct noderef *nref)
{
	TAILQ_INSERT_HEAD(&node->nrefsh, nref, nrefs);
	node->nnrefs++;
	
	if (nref->flags & MAPREF_ANIM) {
		node->nanims++;
	}

	return (0);
}

/*
 * Delete a map entry reference.
 * Must be called on a locked map.
 */
int
node_delref(struct node *node, struct noderef *nref)
{
	if (nref->flags & MAPREF_ANIM) {
		node->nanims--;
	}
	
	TAILQ_REMOVE(&node->nrefsh, nref, nrefs);
	node->nnrefs--;
	free(nref);

	return (0);
}

/*
 * Reinitialize a map to zero.
 * Must be called on a locked map.
 */
void
map_clean(struct map *m, struct object *ob, Uint32 offs, Uint32 nflags,
    Uint32 rflags)
{
	Uint32 x = 0, y;

	/* Initialize the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[x][y];
		
			node_destroy(node);
			node_init(node, nflags);
	
			if (ob != NULL) {
				node_addref(node, ob, offs, rflags);
			}
		}
	}

	m->map[m->defx][m->defy].flags |= NODE_ORIGIN;
}

int
map_destroy(void *p)
{
	struct map *m = (struct map *)p;

	pthread_mutex_lock(&m->lock);

	if (m->flags & MAP_FOCUSED) {
		map_unfocus(m);
	}

	map_freenodes(m);

	pthread_mutex_unlock(&m->lock);

	return (0);
}

/*
 * Initialize a node.
 * Must be called on a locked map.
 */
static void
node_init(struct node *node, Uint32 flags)
{
	memset(node, 0, sizeof(struct node));
	node->flags = flags;
	TAILQ_INIT(&node->nrefsh);
}

/*
 * Free all the references of a node.
 * Must be called on a locked map.
 */
static void
node_destroy(struct node *node)
{
	struct noderef *nref1, *nref2;

	nref1 = TAILQ_FIRST(&node->nrefsh);
	while (nref1 != NULL) {
		nref2 = TAILQ_NEXT(nref1, nrefs);
		free(nref1);
		nref1 = nref2;
	}
	TAILQ_INIT(&node->nrefsh);
}

/* Draw a sprite at any given location. */
void
map_plot_sprite(struct map *m, SDL_Surface *s, Uint32 x, Uint32 y)
{
	static SDL_Rect rs, rd;

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;
	if (m->flags & MAP_VARTILEGEO) {
		/* The sprite size should be a multiple of the tile size. */
		/* XXX math */
		if (rd.w > m->tilew) {
			x -= (rs.w / m->tilew) / 2;
		}
		if (rd.h > m->tileh) {
			y -= (rs.h / m->tileh) / 2;
		}
	}
	rd.x = x;
	rd.y = y;

	SDL_BlitSurface(s, &rs, m->view->v, &rd);
}

/*
 * Render all animations in the map view.
 * Must be called on a locked map.
 */
static void
map_animate(struct map *m)
{
	static Uint32 x, y;
	static Uint32 vx, vy;
	static TAILQ_HEAD(, draw) deferdraws;

	TAILQ_INIT(&deferdraws);
	
	for (y = m->view->mapy, vy = m->view->mapyoffs;
	    (vy < m->view->maph + m->view->mapyoffs);
	     y++, vy++) {

		for (x = m->view->mapx, vx = m->view->mapxoffs;
		     vx < m->view->mapw + m->view->mapxoffs;
		     x++, vx++) {
			static struct node *node;
			static struct noderef *nref;
			static SDL_Surface *src;
			static Uint32 rx, ry;
			
			if (m->view->mapmask[vy - m->view->mapyoffs]
			    [vx - m->view->mapxoffs] & MAPMASK_NORENDER) {
				continue;
			}

			node = &m->map[x][y];

			if (node->nanims < 1 &&
			   (node->flags & NODE_ANIM) == 0) {
				/* map_draw() shall handle this. */
				continue;
			}

			if (curmapedit != NULL) {
				mapedit_predraw(m, node->flags, vx, vy);
			}

			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				rx = (vx << m->shtilex) + nref->xoffs;
				ry = (vy << m->shtiley) + nref->yoffs;

				if (nref->flags & MAPREF_SPRITE) {
					src = nref->pobj->sprites[nref->offs];
				} else if (nref->flags & MAPREF_ANIM) {
					static struct anim *anim;

					anim = nref->pobj->anims[nref->offs];
					src = anim->frames[nref->frame];
				
					if ((anim->delay < 1) ||
					    (++nref->fwait > anim->delay)) {
						nref->fwait = 0;
						if (nref->frame++ >=
						    anim->nframes - 1) {
						    	/* Loop */
							nref->frame = 0;
						}
					}
				}

				/*
				 * Blit only once other sprites/frames are
				 * drawn (ie. soft-scrolling).
				 *
				 * XXX pre-allocate
				 */
				if (nref->xoffs != 0 || nref->yoffs != 0) {
					struct draw *ndraw;

					ndraw = (struct draw *)
					    emalloc(sizeof(struct draw));
					ndraw->s = src;
					ndraw->x = rx;
					ndraw->y = ry;
					ndraw->flags = node->flags;

					TAILQ_INSERT_TAIL(&deferdraws, ndraw,
					    pdraws);
				} else {
					map_plot_sprite(m, src, rx, ry);
				}
#if 0
				if (nref->yoffs < 0)
					map_plot_sprite(m,
					    curmapedit->obj.sprites[
					    MAPEDIT_NVEL], rx, ry);
				if (nref->yoffs > 0)
					map_plot_sprite(m,
					    curmapedit->obj.sprites[
					    MAPEDIT_SVEL], rx, ry);
				if (nref->xoffs < 0)
					map_plot_sprite(m,
					    curmapedit->obj.sprites[
					    MAPEDIT_WVEL], rx, ry);
				if (nref->xoffs > 0)
					map_plot_sprite(m,
					    curmapedit->obj.sprites[
					    MAPEDIT_EVEL], rx, ry);
#endif
			}
			
			if (curmapedit != NULL) {
				mapedit_postdraw(m, node->flags, vx, vy);
			}
			SDL_UpdateRect(m->view->v, rx, ry,
			    m->tilew, m->tileh);
		}
	}

	if (!TAILQ_EMPTY(&deferdraws)) {
		struct draw *draw;
	
		TAILQ_FOREACH(draw, &deferdraws, pdraws) {
			map_plot_sprite(m, draw->s, draw->x, draw->y);
			SDL_UpdateRect(m->view->v,
			    draw->x, draw->y, m->tilew, m->tileh);
			free(draw);
		}
		TAILQ_INIT(&deferdraws);
	}
}

/*
 * Draw all sprites in the map view.
 * Must be called on a locked map.
 */
static void
map_draw(struct map *m)
{
	static Uint32 x, y;
	static Uint32 vx, vy;

	for (y = m->view->mapy, vy = m->view->mapyoffs;
	     vy < m->view->maph + m->view->mapyoffs;
	     y++, vy++) {

		for (x = m->view->mapx, vx = m->view->mapxoffs;
		     vx < m->view->mapw + m->view->mapxoffs;
		     x++, vx++) {
			static struct node *node;
			static struct noderef *nref;
			static int nsprites;

			if (m->view->mapmask[vy - m->view->mapyoffs]
			    [vx - m->view->mapxoffs] & MAPMASK_NORENDER) {
				continue;
			}

			node = &m->map[x][y];

			if (node->nanims > 0 || (node->flags & NODE_ANIM)) {
				/* map_animate() shall handle this. */
				continue;
			}
			
			if (curmapedit != NULL) {
				mapedit_predraw(m, node->flags, vx, vy);
			}

			nsprites = 0;
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					map_plot_sprite(m,
					    nref->pobj->sprites[nref->offs],
					    (vx << m->shtilex) + nref->xoffs,
					    (vy << m->shtiley) + nref->yoffs);
					nsprites++;
				}
			}

			if (nsprites > 0 && curmapedit != NULL) {
				mapedit_postdraw(m, node->flags, vx, vy);
			}
		}
	}

	if (curmapedit != NULL) {
		mapedit_tilestack(curmapedit);
	}

	SDL_UpdateRect(m->view->v, 0, 0, 0, 0);
}

/*
 * Return the map entry reference for ob:offs, or the first match
 * for ob if offs is -1.
 *
 * Must be called on a locked map.
 */
struct noderef *
node_findref(struct node *node, void *ob, Sint32 offs, Uint32 flags)
{
	struct noderef *nref;

	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if ((nref->pobj == ob && (nref->flags & flags)) &&
		    (nref->offs == offs || offs < 0)) {
			return (nref);
		}
	}

	return (NULL);
}

/*
 * Load a map from file.
 * Must be called on a locked map.
 */
int
map_load(void *ob, int fd)
{
	char magic[9];
	struct map *m = (struct map *)ob;
	Uint32 vermin, vermaj;
	Uint32 x, y, refs = 0;

	/* Verify the signature and version major. */
	if (read(fd, magic, 10) != 10)
		goto badmagic;
	if (strncmp(magic, MAP_MAGIC, 10) != 0)
		goto badmagic;
	vermaj = (int) fobj_read_uint32(fd);
	vermin = (int) fobj_read_uint32(fd);
	if (vermaj > MAP_VERMAJ ||
	    (vermaj == MAP_VERMAJ && vermin > MAP_VERMIN))
		goto badver;

	/* Read map information. */
	m->flags = (int) fobj_read_uint32(fd);
	m->mapw  = (int) fobj_read_uint32(fd);
	m->maph  = (int) fobj_read_uint32(fd);
	m->defx  = (int) fobj_read_uint32(fd);
	m->defy  = (int) fobj_read_uint32(fd);
	m->tilew = (int) fobj_read_uint32(fd);
	m->tileh = (int) fobj_read_uint32(fd);

	map_allocnodes(m, m->mapw, m->maph, m->tilew, m->tileh);

	/* Adapt the viewport to this tile geometry. */
	view_setmode(m->view, m, -1, NULL);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[x][y];
			Uint32 i, nnrefs;
			
			/* Read the map entry flags. */
			node->flags = fobj_read_uint32(fd);

			/* Read the optional integer values. */
			node->v1 = fobj_read_uint32(fd);
			node->v2 = fobj_read_uint32(fd);

			/* Read the reference count. */
			nnrefs = fobj_read_uint32(fd);

			for (i = 0; i < nnrefs; i++) {
				struct object *pobj;
				struct noderef *nref;
				char *pobjstr;
				Uint32 offs, frame, flags;

				/* Read object:offset reference. */
				pobjstr = fobj_read_string(fd);
				offs = fobj_read_uint32(fd);
				frame = fobj_read_uint32(fd);
				flags = fobj_read_uint32(fd);
				pobj = object_strfind(pobjstr);

				if (pobj == NULL) {
					fatal("no match for \"%s\"\n", pobjstr);
					return (-1);
				}
				free(pobjstr);

				nref = node_addref(node, pobj, offs, flags);
				nref->frame = frame;

				refs++;
			}
		}
	}
	
	return (0);
badver:
	fatal("map version %d.%d > %d.%d\n", vermaj, vermin,
	    MAP_VERMAJ, MAP_VERMIN);
	return (-1);
badmagic:
	fatal("bad magic\n");
	return (-1);
}

/*
 * Save a map to a file.
 * Must be called on a locked map.
 */
int
map_save(void *ob, int fd)
{
	struct map *m = (struct map *)ob;
	struct fobj_buf *buf;
	Uint32 x = 0, y, totrefs = 0;

	buf = fobj_create_buf(65536, 32767);	/* XXX tune */

	fobj_bwrite(buf, MAP_MAGIC, 10);
	fobj_bwrite_uint32(buf, MAP_VERMAJ);
	fobj_bwrite_uint32(buf, MAP_VERMIN);

	fobj_bwrite_uint32(buf, m->flags);
	fobj_bwrite_uint32(buf, m->mapw);
	fobj_bwrite_uint32(buf, m->maph);
	fobj_bwrite_uint32(buf, m->defx);
	fobj_bwrite_uint32(buf, m->defy);
	fobj_bwrite_uint32(buf, m->tilew);
	fobj_bwrite_uint32(buf, m->tileh);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			off_t soffs;
			struct node *node = &m->map[x][y];
			struct noderef *nref;
			Uint32 nrefs = 0;
			
			/* Write the node flags. */
			fobj_bwrite_uint32(buf, node->flags & ~(NODE_DONTSAVE));
			
			/* Write the optional integer values. */
			fobj_bwrite_uint32(buf, node->v1);
			fobj_bwrite_uint32(buf, node->v2);

			/* We do not know the reference count yet. */
			soffs = buf->offs;
			fobj_bwrite_uint32(buf, 0);

			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref != NULL && nref->flags & MAPREF_SAVE) {
					fobj_bwrite_string(buf,
					    nref->pobj->name);
					fobj_bwrite_uint32(buf, nref->offs);
					fobj_bwrite_uint32(buf, nref->frame);
					fobj_bwrite_uint32(buf, nref->flags);

					nrefs++;
				}
			}

			/* Write the reference count. */
			fobj_bpwrite_uint32(buf, nrefs, soffs);
			totrefs += nrefs;
		}
	}

	fobj_flush_buf(buf, fd);
	dprintf("%s: %dx%d, %d refs\n", m->obj.name, m->mapw, m->maph, totrefs);

	return (0);
}

void
map_dump(void *p)
{
	struct map *m = (struct map *)p;

	printf("flags 0x%x geo %dx%d tilegeo %dx%d origin at %dx%d\n",
	    m->flags, m->mapw, m->maph, m->tilew, m->tileh, m->defx, m->defy,
	    m->defx, m->defy);
}

