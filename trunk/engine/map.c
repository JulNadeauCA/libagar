/*	$Csoft: map.c,v 1.26 2002/02/15 10:00:23 vedge Exp $	*/

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

#define TILESHIFT	5	/* 32x32 */ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

struct	map *curmap;

struct draw {
	SDL_Surface *s;		/* Source surface */
	int	x, y;		/* View coordinates */

	TAILQ_ENTRY(draw) pdraws; /* Deferred rendering */
};

TAILQ_HEAD(, draw) deferdraws;	 /* Deferred rendering */

static void	 node_destroy(struct node *);
static int	 node_init(struct node *, struct object *, int, int, int);
static void	 mapedit_drawflags(struct map *, int, int, int);
static void	 map_draw(struct map *);
static void	 map_animate(struct map *);
static void	*map_draw_th(void *);

struct map *
map_create(char *name, char *desc, int flags, int width, int height)
{
	struct map *m;
	int x = 0, y = 0;

	m = (struct map *)emalloc(sizeof(struct map));
	object_create(&m->obj, name, desc,
	    DESTROY_HOOK|LOAD_FUNC|SAVE_FUNC|OBJ_DEFERGC|OBJ_EDITABLE);
	m->obj.destroy_hook = map_destroy;

	m->obj.load = map_load;
	m->obj.save = map_save;

	m->flags = flags;
	m->maph = height;
	m->mapw = width;
	m->defx = m->mapw / 2;
	m->defy = m->maph - 1;
	m->view = mainview;
	m->view->map = m;
	if (pthread_mutex_init(&m->lock, NULL) != 0) {
		return (NULL);
	}

	/* Initialize an empty map. */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (node_init(&m->map[x][y], NULL, 0, 0, 0) < 0) {
				return (NULL);
			}
		}
	}
	
	dprintf("%s: geo %dx%d flags 0x%x\n", m->obj.name, m->mapw, m->mapw,
	    m->flags);
	dprintf("%s: tilegeo %dx%d origin %dx%d\n", m->obj.name,
	    m->view->tilew, m->view->tileh,
	    m->defx, m->defy);
	return (m);
}

static void *
map_draw_th(void *p)
{
	struct map *m = (struct map *)p;

	while (m->flags & MAP_FOCUSED) {
		map_animate(m);
		if (m->redraw) {
			m->redraw = 0;
			map_draw(m);
		}
		SDL_Delay(3);
	}

	return (NULL);
}

int
map_focus(struct map *m)
{
	curmap = m;
	m->flags |= MAP_FOCUSED;
	
	dprintf("focusing on %s\n", m->obj.name);

	if (curmapedit != NULL) {
		char s[128];
		
		sprintf(s, "%s (edition)", m->obj.name);
		SDL_WM_SetCaption(s, "mapedit");
	} else {
		SDL_WM_SetCaption(m->obj.name, "mapedit");
	}

	if (pthread_create(&m->draw_th, NULL, map_draw_th, m) != 0) {
		perror("draw_th");
		return (-1);
	}

	return (0);
}

int
map_unfocus(struct map *m)
{
	dprintf("unfocusing %s\n", m->obj.name);
	
	m->flags &= ~(MAP_FOCUSED);	/* Will stop the rendering thread */

	curmap = NULL;
	if (curmapedit != NULL) {
		curmapedit->flags = 0;
	}

	return (0);
}

/*
 * Initialize a map node.
 * Must be called on a locked map.
 */
static int
node_init(struct node *node, struct object *ob, int offs, int nodeflags,
     int rflags)
{
	memset(node, 0, sizeof(struct node));

	TAILQ_INIT(&node->nrefsh);

	/* Used by the map editor to fill new maps. */
	if (ob != NULL) {
		node_addref(node, ob, offs, rflags);
		node->flags |= nodeflags;
	}
	return (0);
}

/*
 * Allocate and link a map node.
 * Must be called on a locked map.
 */
struct noderef *
node_addref(struct node *node, void *ob, int offs, int rflags)
{
	struct noderef *nref;

	nref = (struct noderef *)emalloc(sizeof(struct noderef));
	nref->pobj = ob;
	nref->offs = offs;
	nref->flags = rflags;
	if (rflags & MAPREF_ANIM) {
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
map_clean(struct map *m, struct object *ob, int offs, int flags, int rflags)
{
	int x = 0, y;

	/* Initialize the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_destroy(&m->map[x][y]);
			if (node_init(&m->map[x][y], ob, offs, flags,
			    rflags) < 0) {
				return;
			}
		}
	}

	m->map[m->defx][m->defy].flags |= NODE_ORIGIN;
}

void
map_destroy(void *p)
{
	struct map *m = (struct map *)p;
	int x = 0, y;

	if (m->flags & MAP_FOCUSED) {
		map_unfocus(m);
	}

	if (pthread_mutex_lock(&m->lock) == 0) {
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				node_destroy(&m->map[x][y]);
			}
		}
		pthread_mutex_unlock(&m->lock);
		pthread_mutex_destroy(&m->lock);
	} else {
		perror(m->obj.name);
	}
}

/* Must be called on a locked map. */
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
static __inline__ void
map_plot_sprite(struct map *m, SDL_Surface *s, int x, int y)
{
	static SDL_Rect rs, rd;

#ifdef DEBUG
	if (s == NULL) {
		dprintf("null sprite on %s:%d,%d\n", m->obj.name, x, y);
		return;
	}
#endif

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;
	if (m->flags & MAP_VARTILEGEO) {
		/* The sprite size should be a multiple of the tile size. */
		if (rd.w > m->view->tilew) {
			x -= (rs.w / m->view->tilew) / 2;
		}
		if (rd.h > m->view->tileh) {
			y -= (rs.h / m->view->tileh) / 2;
		}
	}
	rd.x = x;
	rd.y = y;

	SDL_BlitSurface(s, &rs, m->view->v, &rd);
}

/*
 * Update all animations in the map view, and move references
 * with a nonzero x/y offset value.
 */
static void
map_animate(struct map *m)
{
	int x, y;
	int vx, vy;

	TAILQ_INIT(&deferdraws);

	pthread_mutex_lock(&m->lock);

	for (y = m->view->mapy, vy = 0;
	    (y < m->view->maph + m->view->mapy + 1);
	     y++, vy++) {

		/* XXX */
		if (curmapedit != NULL &&
		    vy == 0 && curmapedit->flags & MAPEDIT_OBJLIST) {
			vy++;
		}
	
		for (x = m->view->mapx, vx = 0;
		     x < m->view->mapw + m->view->mapx + 1;
		     x++, vx++) {
			static struct node *node;
			static struct noderef *nref;
			static SDL_Surface *src;
			static int rx, ry;

			/* XXX */
			if (curmapedit != NULL) {
				if (vx == m->view->mapw &&
				    curmapedit->flags & MAPEDIT_TILELIST) {
					continue;
				}
				if (vx == 0 &&
				    curmapedit->flags & MAPEDIT_TILESTACK) {
					vx++;
				}
			}

			node = &m->map[x][y];

			if (node->nanims < 1 &&
			   (node->flags & NODE_ANIM) == 0) {
				/* map_draw() shall handle this. */
				continue;
			}

			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				rx = (vx << TILESHIFT) + nref->xoffs;
				ry = (vy << TILESHIFT) + nref->yoffs;

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
			
				if (node->flags & NODE_ANIM &&
				    nref->flags & MAPREF_ANIM) {
					struct draw *ndraw;

					ndraw = (struct draw *)
					    emalloc(sizeof(struct draw));
					ndraw->s = src;
					ndraw->x = rx;
					ndraw->y = ry;

					TAILQ_INSERT_TAIL(&deferdraws, ndraw,
					    pdraws);
				} else {
					map_plot_sprite(m, src, rx, ry);
				}
			}

			if (curmapedit != NULL) {
				mapedit_drawflags(m, node->flags, vx, vy);
				if (curmapedit->flags & MAPEDIT_TILELIST)
					mapedit_tilelist(curmapedit);
			}
		}
	}

	if (!TAILQ_EMPTY(&deferdraws)) {
		struct draw *draw;
	
		TAILQ_FOREACH(draw, &deferdraws, pdraws) {
			map_plot_sprite(m, draw->s, draw->x, draw->y);
			free(draw);
		}
	}
	
	pthread_mutex_unlock(&m->lock);

	SDL_UpdateRect(m->view->v, 0, 0, 0, 0);
}

/* Draw all sprites in the map view. */
static void
map_draw(struct map *m)
{
	int x, y;
	int vx, vy;

	pthread_mutex_lock(&m->lock);

	for (y = m->view->mapy, vy = 0;
	     y < m->view->maph + m->view->mapy + 1;
	     y++, vy++) {

	     	/* XXX */
		if (curmapedit != NULL &&
		    vy == 0 && curmapedit->flags & MAPEDIT_OBJLIST) {
			vy++;
		}

		for (x = m->view->mapx, vx = 0;
		     x < m->view->mapw + m->view->mapx + 1;
		     x++, vx++) {
			static struct node *node;
			static struct noderef *nref;

			/* XXX */
			if (curmapedit != NULL) {
				if (vx == 0 &&
				    curmapedit->flags & MAPEDIT_TILESTACK) {
					vx++;
				}
				if (vx == m->view->mapw &&
				    curmapedit->flags & MAPEDIT_TILELIST) {
					continue;
				}
			}

			node = &m->map[x][y];

			if (node->nanims > 0 || (node->flags & NODE_ANIM)) {
				/* map_animate() shall handle this. */
				continue;
			}
	
			if (curmapedit != NULL) {
				static SDL_Rect erd;

				/* XXX draw a background on view area. */
				erd.w = m->view->tilew;
				erd.h = m->view->tileh;
				erd.x = vx * erd.w;
				erd.y = vy * erd.h;
				SDL_FillRect(m->view->v, &erd, 0);
			}

			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					map_plot_sprite(m,
					    nref->pobj->sprites[nref->offs],
					    (vx << TILESHIFT) + nref->xoffs,
					    (vy << TILESHIFT) + nref->yoffs);
				}
			}

			if (curmapedit != NULL) {
				mapedit_drawflags(m, node->flags, vx, vy);
			}
		}
	}

	if (curmapedit != NULL) {
		if (curmapedit->flags & MAPEDIT_OBJLIST)
			mapedit_objlist(curmapedit);
		if (curmapedit->flags & MAPEDIT_TILESTACK)
			mapedit_tilestack(curmapedit);
	}
	pthread_mutex_unlock(&m->lock);

	SDL_UpdateRect(m->view->v, 0, 0, 0, 0);
}

static void
mapedit_drawflags(struct map *m, int flags, int vx, int vy)
{
	vx <<= TILESHIFT;
	vy <<= TILESHIFT;

	if (curmapedit->flags & MAPEDIT_DRAWGRID)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_GRID],
		    vx, vy);
	if ((curmapedit->flags & MAPEDIT_DRAWPROPS) == 0)
		return;

	if (flags == 0) {
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_BLOCKED],
		    vx, vy);
		return;
	}
	if (flags & NODE_ORIGIN)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_ORIGIN],
		    vx, vy);
#if 0
	if (flags & NODE_WALK)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_WALK],
		    vx, vy);
#endif
	if (flags & NODE_CLIMB)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_CLIMB],
		    vx, vy);
	if (flags & NODE_SLIP)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_SLIP],
		    vx, vy);
	if (flags & NODE_BIO)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_BIO],
		    vx, vy);
	if (flags & NODE_REGEN)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_REGEN],
		    vx, vy);
	if (flags & NODE_SLOW)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_SLOW],
		    vx, vy);
	if (flags & NODE_HASTE)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_HASTE],
		    vx, vy);
#if 0
	if (flags & NODE_ANIM)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_ANIM],
		    vx, vy);
#endif
}

/*
 * Return the map entry reference for ob:offs, or the first match
 * for ob if offs is -1.
 */
struct noderef *
node_findref(struct node *node, void *ob, int offs)
{
	struct noderef *nref;

	/* XXX bsearch */
	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if (nref->pobj == ob && (nref->offs == offs || offs < 0)) {
			return (nref);
		}
	}

	return (NULL);
}

/*
 * Load a map from file. The map must be already locked.
 */
int
map_load(void *ob, char *path)
{
	char magic[9];
	struct map *m = (struct map *)ob;
	int vermin, vermaj;
	int f, x, y, refs = 0;

	f = open(path, O_RDONLY);
	if (f < 0) {
		perror(path);
		return (-1);
	}

	/* Verify the signature and version major. */
	if (read(f, magic, 10) != 10)
		goto badmagic;
	if (strncmp(magic, MAP_MAGIC, 10) != 0)
		goto badmagic;
	vermaj = (int) fobj_read_uint32(f);
	vermin = (int) fobj_read_uint32(f);
	if (vermaj > MAP_VERMAJ ||
	    (vermaj == MAP_VERMAJ && vermin > MAP_VERMIN))
		goto badver;

	/* Read map information. */
	m->flags = (int) fobj_read_uint32(f);
	m->mapw  = (int) fobj_read_uint32(f);
	m->maph  = (int) fobj_read_uint32(f);
	m->defx  = (int) fobj_read_uint32(f);
	m->defy  = (int) fobj_read_uint32(f);
	m->view->tilew = (int) fobj_read_uint32(f);
	m->view->tileh = (int) fobj_read_uint32(f);

	dprintf("%s: v%d.%d flags 0x%x geo %dx%d tilegeo %dx%d\n",
	    path, vermaj, vermin, m->flags, m->mapw, m->maph,
	    m->view->tilew, m->view->tileh);

	/* Adapt the viewport to this tile geometry. */
	view_setmode(m->view);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[x][y];
			int i, nnrefs;
			
			/* Read the map entry flags. */
			node->flags = fobj_read_uint32(f);

			/* Read the optional integer values. */
			node->v1 = fobj_read_uint32(f);
			node->v2 = fobj_read_uint32(f);

			/* Read the reference count. */
			nnrefs = fobj_read_uint32(f);

			for (i = 0; i < nnrefs; i++) {
				struct object *pobj;
				struct noderef *nref;
				char *pobjstr;
				int offs, frame, rflags;

				/* Read object:offset reference. */
				pobjstr = fobj_read_string(f);
				offs = fobj_read_uint32(f);
				frame = fobj_read_uint32(f);
				rflags = fobj_read_uint32(f);
				pobj = object_strfind(pobjstr);

				if (pobj == NULL) {
					fatal("no match for \"%s\"\n", pobjstr);
					return (-1);
				}
				free(pobjstr);

				nref = node_addref(node, pobj, offs, rflags);
				nref->frame = frame;

				refs++;
			}
		}
	}

	dprintf("%s: %d refs, origin: %dx%d\n", path, refs, m->defx, m->defy);
	return (0);

badver:
	fatal("map version %d.%d > %d.%d\n", vermaj, vermin,
	    MAP_VERMAJ, MAP_VERMIN);
	close(f);
	return (-1);
badmagic:
	fatal("bad magic\n");
	close(f);
	return (-1);
}

/*
 * Save a map to a file.
 * Must be called on a locked map.
 */
int
map_save(void *ob, char *path)
{
	struct map *m = (struct map *)ob;
	struct fobj_buf *buf;
	int x = 0, y, f, totrefs = 0;

	f = open(path, O_WRONLY|O_CREAT|O_TRUNC, 00600);
	if (f < 0) {
		perror(path);
		return (-1);
	}

	buf = fobj_create_buf(65536, 32767);	/* XXX tune */

	fobj_bwrite(buf, MAP_MAGIC, 10);
	fobj_bwrite_uint32(buf, MAP_VERMAJ);
	fobj_bwrite_uint32(buf, MAP_VERMIN);

	fobj_bwrite_uint32(buf, m->flags);
	fobj_bwrite_uint32(buf, m->mapw);
	fobj_bwrite_uint32(buf, m->maph);
	fobj_bwrite_uint32(buf, m->defx);
	fobj_bwrite_uint32(buf, m->defy);
	fobj_bwrite_uint32(buf, m->view->tilew);
	fobj_bwrite_uint32(buf, m->view->tileh);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			off_t soffs;
			struct node *node = &m->map[x][y];
			struct noderef *nref;
			int nrefs = 0;
			
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

	fobj_flush_buf(buf, f);
	close(f);
	dprintf("%s: %dx%d, %d refs\n", path, m->mapw, m->maph, totrefs);

	return (0);
}

#ifdef DEBUG
void
map_dump(struct map *m)
{
	printf("%3d. %10s geo %dx%d flags 0x%x\n", m->obj.id, m->obj.name,
	    m->mapw, m->maph, m->flags);
}
#endif /* DEBUG */

