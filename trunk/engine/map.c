/*	$Csoft: map.c,v 1.70 2002/04/14 04:52:46 vedge Exp $	*/

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

#include <engine/text/text.h>
#include <engine/mapedit/mapedit.h>

static struct obvec map_vec = {
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

static void	 node_init(struct node *, Uint32);
static void	 node_destroy(struct node *);

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
			node_init(&m->map[y][x], 0);
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

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_destroy(&m->map[y][x]);
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
	m->fps = 100;
	if (pthread_mutex_init(&m->lock, NULL) != 0) {
		return (NULL);
	}

	return (m);
}

int
map_focus(struct map *m)
{
	m->view->map = m;
	m->flags |= MAP_FOCUSED;
	world->curmap = m;

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
		if (flags & MAPREF_ANIM_INDEPENDENT) {
			nref->frame = 0;
			nref->fdelta = 0;
		}
		node->flags |= NODE_ANIM;
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
		node->flags &= ~(MAPREF_ANIM);
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
			struct node *node = &m->map[y][x];
		
			node_destroy(node);
			node_init(node, nflags);
	
			if (ob != NULL) {
				node_addref(node, ob, offs, rflags);
			}
		}
	}

	m->map[m->defy][m->defx].flags |= NODE_ORIGIN;
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

static __inline__ void
map_rendernode(struct map *m, struct node *node, Uint32 rx, Uint32 ry)
{
	static struct noderef *nref;
	static SDL_Surface *src;
	static SDL_Rect rd;

	TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
		if (nref->flags & MAPREF_SPRITE) {
			src = nref->pobj->sprites[nref->offs];
		} else if (nref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			anim = nref->pobj->anims[nref->offs];
		
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
	static Uint32 x, y, vx, vy, rx, ry;
	Uint32 ri = 0;

	for (y = view->mapy, vy = view->mapyoffs;
	     vy < (view->vmaph + view->mapyoffs) && y < m->maph;
	     y++, vy++) {

		ry = vy << m->shtiley;
	
		for (x = view->mapx, vx = view->mapxoffs;
		     vx < (view->vmapw + view->mapxoffs) && x < m->mapw;
		     x++, vx++) {
			static struct node *node;
		
			if (view->mapmask[vy - view->mapyoffs]
			    [vx - view->mapxoffs] > 0) {
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

				/* Draw nearby nodes. */
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
	Uint32 x, y, vx, vy, rx, ry;
	Uint32 ri = 0;
	struct viewport *view = m->view;

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

			if (view->mapmask[vy - view->mapyoffs]
			    [vx - view->mapxoffs] > 0) {
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
					    nref->pobj->sprites[nref->offs],
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
		SDL_UpdateRects(view->v, ri, view->rects);
	}

	/* XXX does not belong here */
	if (curmapedit != NULL) {
		mapedit_tilestack(curmapedit);
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
	
	if (version_read(fd, "agar map", 1, 11) != 0) {
		return (-1);
	}

	m->flags = fobj_read_uint32(fd);
	m->mapw  = fobj_read_uint32(fd);
	m->maph  = fobj_read_uint32(fd);
	m->defx  = fobj_read_uint32(fd);
	m->defy  = fobj_read_uint32(fd);
	m->tilew = fobj_read_uint32(fd);
	m->tileh = fobj_read_uint32(fd);
	
	nobjs = fobj_read_uint32(fd);
	pobjs = (struct object **)emalloc(nobjs * sizeof(struct object *));
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
	
	pthread_mutex_lock(&m->lock);
	
	map_allocnodes(m, m->mapw, m->maph, m->tilew, m->tileh);

	/* Adapt the viewport to this tile geometry. */
	view_setmode(m->view, m, -1, NULL);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			Uint32 i, nnrefs;
			
			node->flags = fobj_read_uint32(fd);
			node->v1 = fobj_read_uint32(fd);
			node->v2 = fobj_read_uint32(fd);
			nnrefs = fobj_read_uint32(fd);

			for (i = 0; i < nnrefs; i++) {
				struct object *pobj;
				struct noderef *nref;
				Uint32 obji, offs, frame, flags;

				obji = fobj_read_uint32(fd);
				pobj = pobjs[obji];
				offs = fobj_read_uint32(fd);
				frame = fobj_read_uint32(fd);
				flags = fobj_read_uint32(fd);

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

	free(pobjs);

	/* XXX not sure about this */
	m->redraw++;

	return (0);
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
	struct object *pob, **pobjs;
	Uint32 x = 0, y, totrefs = 0, nobjs = 0;
	size_t solen = 0;
	off_t soffs;

	buf = fobj_create_buf(65536, 32767);	/* XXX tune */

	version_write(fd, "agar map", 1, 11);

	fobj_bwrite_uint32(buf, m->flags);
	fobj_bwrite_uint32(buf, m->mapw);
	fobj_bwrite_uint32(buf, m->maph);
	fobj_bwrite_uint32(buf, m->defx);
	fobj_bwrite_uint32(buf, m->defy);
	fobj_bwrite_uint32(buf, m->tilew);
	fobj_bwrite_uint32(buf, m->tileh);
	
	soffs = buf->offs;
	fobj_bwrite_uint32(buf, 0);

	pthread_mutex_lock(&world->lock);
	/* XXX ugly */
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		solen += sizeof(struct object *);
	}
	pobjs = (struct object **)emalloc(solen);
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		if ((pob->flags & OBJ_EDITABLE) == 0)
			continue;
		fobj_bwrite_string(buf, pob->name);
		fobj_bwrite_uint32(buf, 0);
		pobjs[nobjs++] = pob;
	}
	pthread_mutex_unlock(&world->lock);

	fobj_bpwrite_uint32(buf, nobjs, soffs);
	
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
	text_msg(5, TEXT_SLEEP, "%s: %dx%d, %d refs\n", m->obj.name,
	    m->mapw, m->maph, totrefs);

	free(pobjs);

	return (0);
}

