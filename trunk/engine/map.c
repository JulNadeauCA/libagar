/*	$Csoft: map.c,v 1.84 2002/05/11 05:55:00 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/version.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/text.h>

static const struct version map_ver = {
	"agar map",
	1, 11
};

static const struct obvec map_ops = {
	map_destroy,
	map_load,
	map_save,
	NULL,		/* link */
	NULL		/* unlink */
};

struct draw {
	SDL_Surface *s;		/* Source surface */
	Uint32	x, y;		/* View coordinates */
	Uint32	flags;		/* Node flags (for map editor) */

	TAILQ_ENTRY(draw) pdraws; /* Deferred rendering */
};

/*
 * Allocate nodes for the given map geometry.
 * Map must be locked.
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
	m->map = emalloc((w * h) * sizeof(struct node *));
	for (i = 0; i < h; i++) {
		*(m->map + i) = emalloc(w * sizeof(struct node));
	}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			struct node *node = &m->map[y][x];
	
			memset(node, 0, sizeof(struct node));
			TAILQ_INIT(&node->nrefsh);
		}
	}
}

/*
 * Free map nodes.
 * Map must be locked.
 */
void
map_freenodes(struct map *m)
{
	Uint32 x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *nref, *nextnref;

#ifdef DEBUG
			if (node->nnrefs > 256) {
				dprintnode(m, x, y, "funny node\n");
				continue;
			}
#endif

			for (nref = TAILQ_FIRST(&node->nrefsh);
			     nref != TAILQ_END(&node->nrefsh);
			     nref = nextnref) {
				nextnref = TAILQ_NEXT(nref, nrefs);
				free(nref);
			}
		}
		free(*(m->map + y));
	}
	free(m->map);
}

void
map_init(struct map *m, char *name, char *media, Uint32 flags)
{
	object_init(&m->obj, name, media, (media != NULL) ? OBJ_ART : 0,
	    &map_ops);
	sprintf(m->obj.saveext, "m");
	m->flags = flags;
	m->redraw = 0;
	m->fps = 100;		/* XXX pref */
	m->mapw = 0;
	m->maph = 0;
	m->tilew = 0;
	m->tileh = 0;
	m->shtilex = 0;
	m->shtiley = 0;
	m->defx = 0;
	m->defy = 0;
	m->map = NULL;
	m->view = mainview;
	pthread_mutex_init(&m->lock, NULL);
}

/*
 * Display a particular map.
 *
 * This map, the current map and the world must not be locked by the
 * caller thread.
 */
void
map_focus(struct map *m)
{
	pthread_mutex_lock(&world->lock);
	if (world->curmap != NULL && world->curmap != m) {
		dprintf("unfocusing %s\n", OBJECT(world->curmap)->name);
		pthread_mutex_lock(&world->curmap->lock);
		world->curmap->flags &= ~(MAP_FOCUSED);
		pthread_mutex_unlock(&world->curmap->lock);
	}
	dprintf("focusing %s\n", OBJECT(m)->name);
	world->curmap = m;
	world->curview = m->view;
	pthread_mutex_unlock(&world->lock);

	pthread_mutex_lock(&m->lock);
	m->view->map = m;
	m->flags |= MAP_FOCUSED;
	pthread_mutex_unlock(&m->lock);
}

/*
 * Display a particular map.
 * Map must be locked, world must not.
 */
void
map_unfocus(struct map *m)
{
	m->view->map = NULL;
	m->flags &= ~(MAP_FOCUSED);	/* Will stop the rendering thread */

	pthread_mutex_lock(&world->lock);
	world->curmap = NULL;
	pthread_mutex_unlock(&world->lock);

	if (curmapedit != NULL) {
		mapedit_unlink(curmapedit);
	}
}

/*
 * Allocate and link a map node.
 * Must be called on a locked map.
 */
struct noderef *
node_addref(struct node *node, void *ob, Uint32 offs, Uint32 flags)
{
	struct noderef *nref;

	nref = emalloc(sizeof(struct noderef));
	nref->pobj = (struct object *)ob;
	nref->offs = offs;
	nref->flags = flags;

	if (flags & MAPREF_ANIM) {
		if (flags & MAPREF_ANIM_INDEPENDENT) {
			nref->frame = 0;
			nref->fdelta = 0;
		}
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
void
node_pushref(struct node *node, struct noderef *nref)
{
	TAILQ_INSERT_HEAD(&node->nrefsh, nref, nrefs);
	node->nnrefs++;
	
	if (nref->flags & MAPREF_ANIM) {
		node->nanims++;
	}
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
		node->flags &= ~(MAPREF_ANIM);
	}

#ifdef DEBUG
	do {
		struct noderef *fnref;
		int found = 0;

		TAILQ_FOREACH(fnref, &node->nrefsh, nrefs) {
			if (fnref == nref) {
				found++;
			}
		}
		if (!found) {
			dprintf("noderef %p not in node %p\n", nref, node);
			return (-1);
		}
	} while (0);
#endif

	TAILQ_REMOVE(&node->nrefsh, nref, nrefs);
	node->nnrefs--;
	free(nref);

	return (0);
}

/*
 * Reinitialize all nodes.
 * Must be called on a locked map.
 */
void
map_clean(struct map *m, struct object *ob, Uint32 offs, Uint32 nflags,
    Uint32 rflags)
{
	int x = 0, y;

	/* Initialize the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			
			memset(node, 0, sizeof(struct node));
			TAILQ_INIT(&node->nrefsh);
			node->flags = nflags;
	
			if (ob != NULL) {
				node_addref(node, ob, offs, rflags);
			}
		}
	}

	m->map[m->defy][m->defx].flags |= NODE_ORIGIN;
}

void
map_destroy(void *p)
{
	struct map *m = p;

	pthread_mutex_lock(&m->lock);
	if (m->flags & MAP_FOCUSED) {
		map_unfocus(m);
	}
	if (m->map != NULL) {
		map_freenodes(m);
	}
	pthread_mutex_unlock(&m->lock);
}

/*
 * Render a map node.
 * Inline since this is called from draw functions with many variables.
 * Must be called on a locked map.
 */
static __inline__ void
map_rendernode(struct map *m, struct node *node, Uint32 rx, Uint32 ry)
{
	static struct noderef *nref;
	static SDL_Surface *src;
	static SDL_Rect rd;

	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if (nref->flags & MAPREF_SPRITE) {
			src = SPRITE(nref->pobj, nref->offs);
		} else if (nref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			anim = ANIM(nref->pobj, nref->offs);
		
			if (nref->flags & MAPREF_ANIM_DELTA &&
			   (nref->flags & MAPREF_ANIM_STATIC) == 0) {
				Uint32 t;
				src = anim->frames[anim->frame];

				t = SDL_GetTicks();
				if ((t - anim->delta) >= anim->delay) {
					anim->delta = t;
					if (++anim->frame >
					    anim->nframes - 1) {
						/* Loop */
						anim->frame = 0;
					}
				}
			} else if (nref->flags & MAPREF_ANIM_INDEPENDENT) {
				src = anim->frames[nref->frame];

				if ((nref->flags & MAPREF_ANIM_STATIC) == 0) {
					if ((anim->delay < 1) ||
					    (++nref->fdelta > anim->delay+1)) {
						nref->fdelta = 0;
						if (++nref->frame >
						    anim->nframes - 1) {
							/* Loop */
							nref->frame = 0;
						}
					}
				}
			}
		}
		rd.w = src->w;
		rd.h = src->h;
		rd.x = rx + nref->xoffs;
		rd.y = ry + nref->yoffs;
		SDL_BlitSurface(src, NULL, m->view->v, &rd);
	}
}

#ifdef XDEBUG
#define XDEBUG_RECTS(view, ri) do {					\
		int ei;							\
		for (ei = 0; ei < (ri); ei++) {				\
			if ((view)->rects[ei].x > (view)->width ||	\
			    (view)->rects[ei].y > (view)->height) {	\
				dprintrect("bogus", &(view)->rects[ei]);\
				dprintf("bogus rectangle %d/%d\n", ei,	\
				    ri);				\
			}						\
		}							\
	} while (/*CONSTCOND*/0)
#else
#define XDEBUG_RECTS(view, ri)
#endif

/*
 * Render all animations in the map view.
 *
 * Nodes with the NODE_ANIM flag set are rendered by the animator, even
 * if there is no animation on the node (used for soft-scrolling); nearby
 * nodes with the NODE_OVERLAP are redrawn first.
 *
 * Must be called on a locked map.
 */
void
map_animate(struct map *m)
{
	struct viewport *view = m->view;
	static int x, y, vx, vy, rx, ry;
	int ri = 0;

	for (y = view->mapy, vy = view->mapyoffs;
	     vy < (view->vmaph + view->mapyoffs) && y < m->maph;
	     y++, vy++) {

		ry = vy << m->shtiley;
	
		for (x = view->mapx, vx = view->mapxoffs;
		     vx < (view->vmapw + view->mapxoffs) && x < m->mapw;
		     x++, vx++) {
			static struct node *node;
#ifdef DEBUG
			int i;

			i = VIEW_MAPMASK(view, vx, vy);
			if (i < 0 || i > 32) {
				dprintf("funny mask: %d at %d,%d\n", i, vx, vy);
				return;
			}
#endif
			if (VIEW_MAPMASK(view, vx, vy) > 0) {
				continue;
			}

			node = &m->map[y][x];
			if (node->flags & NODE_OVERLAP) {
				/* Will later draw in a synchronized fashion. */
				continue;
			}

			rx = vx << m->shtilex;

			if (node->flags & NODE_ANIM) {
				static struct node *nnode;

				/* Draw nearby nodes. XXX */
				if (x > 1) {
					nnode = &m->map[y][x - 1]; /* Left */
					if (nnode->flags & NODE_OVERLAP &&
					    vx > 1) {
						MAPEDIT_PREDRAW(m, nnode,
						    vx - 1, vy);
						map_rendernode(m, nnode,
						    rx - m->tilew, ry);
						MAPEDIT_POSTDRAW(m, nnode,
						    vx - 1, vy);
						view->rects[ri++] =
						    view->maprects[vy][vx - 1];
					}
				}
				if (x < m->mapw - 1) {
					nnode = &m->map[y][x + 1]; /* Right */
					if (nnode->flags & NODE_OVERLAP &&
					    vx < view->mapw + view->mapxoffs) {
						MAPEDIT_PREDRAW(m, nnode,
						    vx + 1, vy);
						map_rendernode(m, nnode,
						    rx + m->tilew, ry);
						MAPEDIT_POSTDRAW(m, nnode,
						    vx + 1, vy);
						view->rects[ri++] =
						    view->maprects[vy][vx + 1];
					}
				}
				if (y > 1) {
					nnode = &m->map[y - 1][x]; /* Up */
					if (nnode->flags & NODE_OVERLAP &&
					    vy > 1) {
						MAPEDIT_PREDRAW(m, nnode,
						    vx, vy - 1);
						map_rendernode(m, nnode,
						    rx, ry - m->tileh);
						MAPEDIT_POSTDRAW(m, nnode,
						    vx, vy - 1);
						view->rects[ri++] =
						    view->maprects[vy - 1][vx];
					}
				}
				if (y < m->maph - 1) {
					nnode = &m->map[y + 1][x]; /* Down */
					if (nnode->flags & NODE_OVERLAP &&
					    vy < view->maph + view->mapyoffs) {
						MAPEDIT_PREDRAW(m, nnode,
						    vx, vy + 1);
						map_rendernode(m, nnode,
						    rx, ry + m->tileh);
						MAPEDIT_POSTDRAW(m, nnode,
						    vx, vy + 1);
						view->rects[ri++] =
						    view->maprects[vy + 1][vx];
					}
				}

				/* Draw the node itself. */
				MAPEDIT_PREDRAW(m, node, vx, vy);
				map_rendernode(m, node, rx, ry);
				MAPEDIT_POSTDRAW(m, node, vx, vy);
				view->rects[ri++] =
				    view->maprects[vy][vx];
			} else if (node->nanims > 0) {
				MAPEDIT_PREDRAW(m, node, vx, vy);
				map_rendernode(m, node, rx, ry);
				MAPEDIT_POSTDRAW(m, node, vx, vy);
				view->rects[ri++] =
				    view->maprects[vy][vx];
			}
		}
	}
	if (ri > 0) {
		XDEBUG_RECTS(view, ri);
		SDL_UpdateRects(view->v, ri, view->rects);
	}
}

/*
 * Draw all sprites in the map view.
 * Must be called on a locked map.
 */
void
map_draw(struct map *m)
{
	static int x, y, vx, vy, rx, ry;
	struct viewport *view = m->view;
	int ri = 0;
	
	for (y = view->mapy, vy = view->mapyoffs;
	     vy < (view->vmaph + view->mapyoffs) && y < m->maph;
	     y++, vy++) {

		ry = vy << m->shtilex;

		for (x = view->mapx, vx = view->mapxoffs;
		     vx < (view->vmapw + view->mapxoffs) && x < m->mapw;
		     x++, vx++) {
			static struct node *node;
			static struct noderef *nref;
			static Uint32 nsprites;
#ifdef DEBUG
			int i;

			i = VIEW_MAPMASK(view, vx, vy);
			if (i < 0 || i > 32) {
				dprintf("funny mask: %d at %d,%d\n", i, vx, vy);
				return;
			}
#endif
			if (VIEW_MAPMASK(view, vx, vy) > 0) {
				continue;
			}

			rx = vx << m->shtilex;

			node = &m->map[y][x];

			if (node->nanims > 0 || (node->flags &
			   (NODE_ANIM|NODE_OVERLAP))) {
				/* map_animate() shall handle this. */
				continue;
			}
		
			MAPEDIT_PREDRAW(m, node, vx, vy);

			nsprites = 0;
			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref->flags & MAPREF_SPRITE) {
					static SDL_Rect rd;

					rd.x = rx + nref->xoffs;
					rd.y = ry + nref->yoffs;
					rd.w = m->tilew;
					rd.h = m->tileh;
					SDL_BlitSurface(
					    SPRITE(nref->pobj, nref->offs),
					    NULL, view->v, &rd);
					view->rects[ri++] =
					    view->maprects[vy][vx];
					nsprites++;
				}
			}
			if (nsprites > 0) {
				MAPEDIT_POSTDRAW(m, node, vx, vy);
			} else {
				view->rects[ri++] =
				    view->maprects[vy][vx];
			}
		}
	}

	if (ri > 0) {
		XDEBUG_RECTS(view, ri);
		SDL_UpdateRects(view->v, ri, view->rects);
	}

	/* XXX thread unsafe */
	if (curmapedit != NULL) {
		curmapedit->redraw++;
	}
}

/*
 * Return the map entry reference for ob:offs, or the first match
 * for ob if offs is -1.
 *
 * Must be called on a locked map.
 *
 * XXX should be a macro
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

int
map_load(void *ob, int fd)
{
	struct map *m = (struct map *)ob;
	struct object **pobjs;
	Uint32 x, y, refs = 0, i, nobjs;

	/* The viewport (and the map mask), might change sizes. */
	text_destroyall();

	if (version_read(fd, &map_ver) != 0) {
		return (-1);
	}

	m->flags = fobj_read_uint32(fd);
	m->mapw  = fobj_read_uint32(fd);
	m->maph  = fobj_read_uint32(fd);
	m->defx  = fobj_read_uint32(fd);
	m->defy  = fobj_read_uint32(fd);
	m->tilew = fobj_read_uint32(fd);
	m->tileh = fobj_read_uint32(fd);

	/* Load the object map. */
	nobjs = fobj_read_uint32(fd);
	pobjs = emalloc(nobjs * sizeof(struct object *));
	for (i = 0; i < nobjs; i++) {
		char *s;
		struct object *pob;

		s = fobj_read_string(fd);
		fobj_read_uint32(fd);		/* Unused */
		pob = object_strfind(s);

		pobjs[i] = pob;
		if (pob != NULL) {
			dprintf("%s uses: %s\n", OBJECT(m)->name, pob->name);
		} else {
			warning("cannot translate \"%s\"\n", s);
		}

		free(s);
	}

	/* Allocate space for the nodes. */
	pthread_mutex_lock(&m->lock);
	if (m->map != NULL) {
		map_freenodes(m);
		m->map = NULL;
	}
	map_allocnodes(m, m->mapw, m->maph, m->tilew, m->tileh);

	/* Adjust the view. */
	view_setmode(m->view, m, -1, NULL);

	/* Read the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			Uint32 i, nnrefs;
			
			node->flags = fobj_read_uint32(fd);
			node->v1 = fobj_read_uint32(fd);
			node->v2 = fobj_read_uint32(fd);
			nnrefs = fobj_read_uint32(fd);

#ifdef DEBUG
			if (nnrefs > 128) {
				dprintnode(m, x, y, "funny node");
			}
#endif

			for (i = 0; i < nnrefs; i++) {
				struct object *pobj;
				struct noderef *nref;
				Uint32 obji, offs, frame, flags;

				obji = fobj_read_uint32(fd);
				pobj = pobjs[obji];
				offs = fobj_read_uint32(fd);
				frame = fobj_read_uint32(fd);
				flags = fobj_read_uint32(fd);

				if (offs > 256) {
					dprintf("bad offs\n");
				}

				if (pobj != NULL) {
					nref = node_addref(node, pobj, offs,
					    flags);
					nref->frame = frame;

					refs++;
				} else {
					dprintf("at %dx%d:[%d]\n", x, y, i);
					fatal("nothing at index %d\n", obji);
				}
			}
		}
	}
	pthread_mutex_unlock(&m->lock);

	m->redraw++;
	free(pobjs);
	return (0);
}

int
map_save(void *ob, int fd)
{
	struct map *m = (struct map *)ob;
	struct fobj_buf *buf;
	struct object *pob, **pobjs;
	Uint32 x = 0, y, totrefs = 0, nobjs = 0;
	size_t solen = 0;
	off_t soffs;

	buf = fobj_create_buf(65536, 32767);

	version_write(fd, &map_ver);

	fobj_bwrite_uint32(buf, m->flags);
	fobj_bwrite_uint32(buf, m->mapw);
	fobj_bwrite_uint32(buf, m->maph);
	fobj_bwrite_uint32(buf, m->defx);
	fobj_bwrite_uint32(buf, m->defy);
	fobj_bwrite_uint32(buf, m->tilew);
	fobj_bwrite_uint32(buf, m->tileh);
	
	soffs = buf->offs;
	fobj_bwrite_uint32(buf, 0);

	/* Write the object map. */
	pthread_mutex_assert(&world->lock);
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		solen += sizeof(struct object *);
	}
	pobjs = emalloc(solen);
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		if ((pob->flags & OBJ_ART) == 0) {
			continue;
		}
		fobj_bwrite_string(buf, pob->name);
		fobj_bwrite_uint32(buf, 0);
		pobjs[nobjs++] = pob;
	}
	fobj_bpwrite_uint32(buf, nobjs, soffs);

	/* Write the nodes. */
	pthread_mutex_lock(&m->lock);
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *nref;
			Uint32 nrefs = 0;
			
			fobj_bwrite_uint32(buf, node->flags & ~(NODE_DONTSAVE));
			fobj_bwrite_uint32(buf, node->v1);
			fobj_bwrite_uint32(buf, node->v2);
			soffs = buf->offs;
			fobj_bwrite_uint32(buf, 0);

			TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
				if (nref != NULL && nref->flags & MAPREF_SAVE) {
					Uint32 i, fi = 0;

					for (i = 0; i < nobjs; i++) {
						if (pobjs[i] == nref->pobj) {
							fi = i;
						}
					}
					fobj_bwrite_uint32(buf, fi);
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
	pthread_mutex_unlock(&m->lock);

	fobj_flush_buf(buf, fd);

	dprintf("%s: %dx%d, %d refs\n", OBJECT(m)->name, m->mapw, m->maph,
	    totrefs);
	free(pobjs);
	return (0);
}

#ifdef DEBUG

static void	*map_verify_loop(void *);

/* Verify the integrity of a map, also helps finding races. */
static void *
map_verify_loop(void *arg)
{
	struct map *m = arg;
	int x = 0, y;
	
	dprintf("%p: testing %s\n", pthread_self(),
	    OBJECT(m)->name);

	for (;;) {
		pthread_mutex_lock(&m->lock);
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				struct node *n = &m->map[y][x];
				struct noderef *nref;

				if (n->nnrefs > 0xff ||
				    ((n->flags & NODE_BLOCK) &&
				     (n->flags & NODE_WALK))) {
					dprintnode(m, x, y, "funny node");
					continue;
				}
				TAILQ_FOREACH(nref, &n->nrefsh, nrefs) {
					if (nref->pobj == NULL ||
					    nref->offs > 0xffff ||
					    nref->xoffs > 0xff ||
					    nref->yoffs > 0xff) {
						fatal("%s:%d,%d: funny ref\n",
						    OBJECT(m)->name, x, y);
					}
				}
			}
		}
		pthread_mutex_unlock(&m->lock);
		SDL_Delay(1);
	}
	return (NULL);
}

void
map_verify(struct map *m)
{
	pthread_t verify_th;

	pthread_create(&verify_th, NULL, map_verify_loop, m);
}

#endif
