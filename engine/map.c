/*	$Csoft: map.c,v 1.112 2002/08/23 05:19:09 vedge Exp $	*/

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
#include <libfobj/buf.h>

#include "engine.h"
#include "map.h"
#include "version.h"
#include "config.h"

#include "widget/text.h"

static const struct version map_ver = {
	"agar map",
	2, 0
};

static const struct object_ops map_ops = {
	map_destroy,
	map_load,
	map_save
};

static __inline__ int	nodecmp(struct node *, struct node *);
static __inline__ void	node_init(struct node *);
static __inline__ void	node_free(struct map *, int, int);

static void	map_load_flat_nodes(int, struct map *, struct object **,
		    Uint32);
static void	map_load_rle_nodes(int, struct map *, struct object **,
		    Uint32);
static void	map_save_flat_nodes(struct fobj_buf *, struct map *,
		    struct object **, Uint32);
static void	map_save_rle_nodes(struct fobj_buf *, struct map *,
		    struct object **, Uint32);

static __inline__ void
node_init(struct node *node)
{
	memset(node, 0, sizeof(struct node));
	TAILQ_INIT(&node->nrefsh);
}

static __inline__ void
node_free(struct map *m, int x, int y)
{
	struct node *_node = &m->map[(y)][(x)];
	struct noderef *_nref, *_nextnref;

	for (_nref = TAILQ_FIRST(&_node->nrefsh);
	     _nref != TAILQ_END(&_node->nrefsh);
	     _nref = _nextnref) {
		_nextnref = TAILQ_NEXT(_nref, nrefs);
		free(_nref);
	}
}

/*
 * Allocate nodes for the given map geometry.
 * Map must be locked.
 */
void
map_allocnodes(struct map *m, Uint32 w, Uint32 h)
{
	Uint32 i, x, y;
	struct node *node;

	
	m->mapw = w;
	m->maph = h;

	/* This is why sprite sizes must be a power of two. */
	for (i = 0; (1 << i) != TILEW; i++)
	    ;;
	m->shtilex = i;
	for (i = 1; (1 << i) != TILEH; i++)
	    ;;
	m->shtiley = i;

	/* Allocate the two-dimensional node array. */
	m->map = emalloc((w * h) * sizeof(struct node *));
	for (i = 0; i < h; i++) {
		*(m->map + i) = emalloc(w * sizeof(struct node));
	}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
 			node = &m->map[y][x];
			node_init(node);
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
			node_free(m, x, y);
		}
		free(*(m->map + y));
	}
	free(m->map);
}

/*
 * Shrink a map and destroy excess nodes.
 * Map must be locked.
 */
void
map_shrink(struct map *m, Uint32 w, Uint32 h)
{
	Uint32 x, y;
	int i;
	
	/* Free excess nodes. */
	for (y = h; y < m->maph; y++) {
		for (x = w; x < m->mapw; x++) {
			node_free(m, x, y);
		}
	}

	/* Reallocate the two-dimensional node array. */
	m->map = erealloc(m->map, (w*h) * sizeof(struct node *));
	if (w < m->mapw) {
		for (i = 0; i < m->maph; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w*sizeof(struct node));
		}
	}
	if (h < m->maph) {
		for (i = m->maph; i < h; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w*sizeof(struct node));
		}
	}

	m->mapw = w;
	m->maph = h;
}

/*
 * Grow a map and initialize new nodes.
 * Map must be locked.
 */
void
map_grow(struct map *m, Uint32 w, Uint32 h)
{
	struct node *node;
	Uint32 x, y;
	int i;

	/* Reallocate the two-dimensional node array. */
	m->map = erealloc(m->map, (w*h) * sizeof(struct node *));
	if (w > m->mapw) {
		for (i = 0; i < m->maph; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w*sizeof(struct node));
		}
	}
	if (h > m->maph) {
		for (i = m->maph; i < h; i++) {
			*(m->map+i) = emalloc(w*sizeof(struct node));
		}
	}

	/* Initialize the new nodes. */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (x >= m->mapw || y >= m->maph) {
				node = &m->map[y][x];
				node_init(node);
			}
		}
	}
	m->mapw = w;
	m->maph = h;
}

void
map_init(struct map *m, char *name, char *media, Uint32 flags)
{
	/* XXX audio */
	object_init(&m->obj, "map", name, media,
	    (media != NULL) ? OBJECT_ART|OBJECT_MEDIA_CAN_FAIL : 0, &map_ops);
	sprintf(m->obj.saveext, "m");
	m->flags = (flags != 0) ? flags : MAP_2D;
	m->redraw = 0;
	m->fps = 100;		/* XXX pref */
	m->mapw = 0;
	m->maph = 0;
	m->shtilex = 0;
	m->shtiley = 0;
	m->defx = 0;
	m->defy = 0;
	m->map = NULL;
	pthread_mutex_init(&m->lock, NULL);
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

	if (m->map != NULL) {
		map_freenodes(m);
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

static void
map_load_flat_nodes(int fd, struct map *m, struct object **pobjs, Uint32 nobjs)
{
	Uint32 x, y;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *ref;
			struct object *pobj = NULL;
			Uint32 i, nrefs;

			node->flags = read_uint32(fd);
			node->v1 = read_uint32(fd);
			node->v2 = read_uint32(fd);
			nrefs = read_uint32(fd);
#ifdef DEBUG
			if (nrefs > 256)
				dprintnode(m, x, y, "funny node");
#endif
			for (i = 0; i < nrefs; i++) {
				Uint32 obji, offs, frame, flags;
				struct object *pobj;

				obji = read_uint32(fd);
				offs = read_uint32(fd);
				frame = read_uint32(fd);
				flags = read_uint32(fd);
			
				if (obji > nobjs) {
					fatal("bogus reference to 0x%x > 0x%x "
					      "at %d,%d[%d]\n",
					    obji, nobjs, x, y, i);
				}
				if ((pobj = pobjs[obji]) != NULL) {
					ref = node_addref(node, pobj, offs,
					    flags);
					ref->frame = frame;
				} else {
					warning("ignoring reference to 0x%x at "
					    "%d,%d[%d]\n", obji, x, y, i);
				}
			}
		}
	}
}

static void
map_load_rle_nodes(int fd, struct map *m, struct object **pobjs, Uint32 nobjs)
{
	Uint32 refs = 0, i, j, nnrefs, count, totnodes = 0;
	struct node node;
	struct noderef *nref;
	Uint32 x, y, ox, oy;

	/* Read and decompress the nodes. */
	for (x = 0, y = 0;;) {
		memset(&node, 0, sizeof(struct node));
		TAILQ_INIT(&node.nrefsh);
		node.flags = read_uint32(fd);
		node.v1 = read_uint32(fd);
		node.v2 = read_uint32(fd);
		nnrefs = read_uint32(fd);
#ifdef DEBUG
		if (nnrefs > 256)
			dprintnode(m, x, y, "funny node");
#endif
		for (i = 0; i < nnrefs; i++) {
			struct object *pobj;
			Uint32 obji, offs, frame, flags;

			obji = read_uint32(fd);
			pobj = pobjs[obji];
			offs = read_uint32(fd);
			frame = read_uint32(fd);
			flags = read_uint32(fd);
#ifdef DEBUG
			if (offs > 4096)
				dprintnode(m, x, y, "bad offset");
#endif
			if (pobj != NULL) {
				nref = node_addref(&node, pobj, offs, flags);
				nref->frame = frame;
				refs++;
			} else {
				dprintf("at %dx%d:[%d]\n", x, y, i);
				fatal("nothing at index %d\n", obji);
			}
		}
		totnodes++;
		
		count = read_uint32(fd);
		if (count > 1) {
			printf("[%d x %d,%d] ", count, x, y);
			fflush(stdout);
		}

		ox = x;
		oy = y;
		for (j = 0; j < count; j++) {
			struct node *dstnode;
			struct noderef *srcnref;

			dstnode = &m->map[y][x];

			/* Copy this node. XXX inefficient */
			memcpy(dstnode, &node, sizeof(struct node));
			TAILQ_INIT(&dstnode->nrefsh);
			dstnode->nnrefs = 0;

			TAILQ_FOREACH(srcnref, &node.nrefsh, nrefs) {
				nref = node_addref(dstnode, srcnref->pobj,
				    srcnref->offs, srcnref->flags);

				if (srcnref->flags & MAPREF_ANIM_INDEPENDENT) {
					nref->frame = srcnref->frame;
				}
			}
			totnodes++;
			
			if (++x == m->mapw) {
				if (++y == m->maph) {
					return;
				}
				x = 0;
			}
		}
	}
	if (totnodes != m->mapw * m->maph) {
		dprintf("inconsistent map: %d nodes, should be %d\n", totnodes,
		    m->mapw * m->maph);
	}
}

int
map_load(void *ob, int fd)
{
	struct map *m = (struct map *)ob;
	struct object **pobjs;
	Uint32 i, nobjs, tilew, tileh;

	dprintf("loading %s\n", OBJECT(m)->name);

	if (version_read(fd, &map_ver) != 0) {
		return (-1);
	}

	m->flags = read_uint32(fd);
	m->mapw  = read_uint32(fd);
	m->maph  = read_uint32(fd);
	m->defx  = read_uint32(fd);
	m->defy  = read_uint32(fd);
	tilew    = read_uint32(fd);
	tileh    = read_uint32(fd);
	if (tilew != TILEW || tileh != TILEH) {
		warning("%s: %dx%d map tiles\n", OBJECT(m)->name, tilew, tileh);
	}
	dprintf("flags 0x%x, geo %dx%d, origin at %d,%d, %dx%d tiles\n",
	    m->flags, m->mapw, m->maph, m->defx, m->defy, TILEW, TILEH);

	/* Load the object map. */
	nobjs = read_uint32(fd);
	pobjs = emalloc(nobjs * sizeof(struct object *));
	for (i = 0; i < nobjs; i++) {
		struct object *pob;
		char *s;

		s = read_string(fd);
		read_uint32(fd);		/* Unused */
		pob = object_strfind(s);

		pobjs[i] = pob;
		if (pob != NULL) {
#if 0
			dprintf("%s: uses %s\n", OBJECT(m)->name, pob->name);
#endif
		} else {
			warning("%s: cannot translate \"%s\"\n",
			    OBJECT(m)->name, s);
		}
		free(s);
	}

	pthread_mutex_lock(&m->lock);

	/* Initialize the nodes. */
	if (m->map != NULL) {
		map_freenodes(m);		/* XXX resize? */
		m->map = NULL;
	}
	map_allocnodes(m, m->mapw, m->maph);

	/* Read/decompress the nodes. */
	if (m->flags & MAP_RLE_COMPRESSION) {
		map_load_rle_nodes(fd, m, pobjs, nobjs);
	} else {
		map_load_flat_nodes(fd, m, pobjs, nobjs);
	}

	pthread_mutex_unlock(&m->lock);

	free(pobjs);
	return (0);
}

/* Compare the persistent properties of two nodes for compression purposes. */
static __inline__ int
nodecmp(struct node *n1, struct node *n2)
{
	struct noderef *nref1, *nref2;

	if (n1->nnrefs != n2->nnrefs ||
	    ((n1->flags & ~NODE_DONTSAVE) != (n2->flags & ~NODE_DONTSAVE)) ||
	    n1->v1 != n2->v1 ||
	    n1->v2 != n2->v2 ||
	    n1->nanims != n2->nanims) {
		return (-1);
	}

	for (nref1 = TAILQ_FIRST(&n1->nrefsh),
	     nref2 = TAILQ_FIRST(&n2->nrefsh);
	     nref1 != TAILQ_END(&n1->nrefsh) &&
	     nref2 != TAILQ_END(&n2->nrefsh);
	     nref1 = TAILQ_NEXT(nref1, nrefs),
	     nref2 = TAILQ_NEXT(nref2, nrefs)) {
		if ((nref1->flags & MAPREF_SAVE) == 0 ||
		    (nref2->flags & MAPREF_SAVE) == 0) {
			continue;
		}
		if (nref1->pobj != nref2->pobj ||
		    nref1->offs != nref2->offs ||
		    ((nref1->flags & ~MAPREF_DONTSAVE) !=
		     (nref2->flags & ~MAPREF_DONTSAVE)) ||
		    (nref1->flags & MAPREF_ANIM_INDEPENDENT &&
		     nref1->frame != nref2->frame)) {
			return (-1);
		}
	}
	return (0);
}

static void
map_save_flat_nodes(struct fobj_buf *buf, struct map *m,
    struct object **pobjs, Uint32 nobjs)
{
	Uint32 x, y;
	int totnodes = 0, totrefs = 0, savedrefs = 0;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct noderef *ref;
			struct node *n = &m->map[y][x];
			off_t soffs;
			Uint32 nrefs = 0;

			totnodes++;

			buf_write_uint32(buf, n->flags & ~(NODE_DONTSAVE));
			buf_write_uint32(buf, n->v1);
			buf_write_uint32(buf, n->v2);

			soffs = buf->offs;		/* Skip */
			buf_write_uint32(buf, 0);

			TAILQ_FOREACH(ref, &n->nrefsh, nrefs) {
				Uint32 i;

				totrefs++;
				if ((ref->flags & MAPREF_SAVE) == 0) {
					continue;
				}
				savedrefs++;
				for (i = 0; i < nobjs; i++) {
					if (pobjs[i] == ref->pobj) {
						break;
					}
				}
				buf_write_uint32(buf, i);
				buf_write_uint32(buf, ref->offs);
				buf_write_uint32(buf, ref->frame);
				buf_write_uint32(buf, ref->flags);
				nrefs++;
			}
			buf_pwrite_uint32(buf, nrefs, soffs);
		}
	}
	dprintf("%d nodes, %d saved refs (of %d total refs)\n", totnodes,
	    savedrefs, totrefs);
}

static void
map_save_rle_nodes(struct fobj_buf *buf, struct map *m, struct object **pobjs,
    Uint32 nobjs)
{
	Uint32 x, y;
	off_t soffs;
	struct node *node;
	int totnodes, totcomp;

	for (x = 0, y = 0;;) {
		struct noderef *nref;
		struct node *cmpnode;
		Uint32 nrefs = 0;
		int count;

		node = &m->map[y][x];

		/* Write the node data. */
		buf_write_uint32(buf, node->flags & ~(NODE_DONTSAVE));
		buf_write_uint32(buf, node->v1);
		buf_write_uint32(buf, node->v2);
		soffs = buf->offs;
		buf_write_uint32(buf, 0);	/* Skip count. */

		/* Write the node references. */
		TAILQ_FOREACH(nref, &node->nrefsh, nrefs) {
			if (nref->flags & MAPREF_SAVE) {
				Uint32 i;

				for (i = 0; i < nobjs; i++) {
					if (pobjs[i] == nref->pobj) {
						break;
					}
				}
				buf_write_uint32(buf, i);
				buf_write_uint32(buf, nref->offs);
				buf_write_uint32(buf, nref->frame);
				buf_write_uint32(buf, nref->flags);
				nrefs++;
			}
		}
		buf_pwrite_uint32(buf, nrefs, soffs);

		/* Write the repeat count. */
		count = 1;
rle_scan:
		if (++x == m->mapw) {
			if (++y == m->maph) {
				return;
			}
			x = 0;
		}
		for (cmpnode = &m->map[y][x]; nodecmp(node, cmpnode) == 0;) {
			count++;
			totcomp++;
			goto rle_scan;
		}
		buf_write_uint32(buf, count);
		totnodes += count;
	}

#ifdef DEBUG
	if (totnodes != m->mapw*m->maph) {
		warning("wrote %d nodes, should be %d\n", totnodes,
		    m->mapw * m->maph);
	}
#endif

	dprintf("%s: %dx%d, %d%% compression (%d nodes of %d)\n",
	    OBJECT(m)->name, m->mapw, m->maph,
	    totcomp * 100 / (m->mapw * m->maph), totcomp, (m->mapw * m->maph));
}

/* World must be locked, map must not. */
int
map_save(void *p, int fd)
{
	struct map *m = p;
	struct fobj_buf *buf;
	struct object *pob, **pobjs;
	size_t solen = 0;
	off_t soffs;
	int totcomp = 0, totnodes = 0;
	Uint32 nobjs = 0;

	dprintf("saving %s\n", OBJECT(m)->name);

	buf = fobj_create_buf(65536, 32767);

	version_write(fd, &map_ver);

	buf_write_uint32(buf, m->flags);
	buf_write_uint32(buf, m->mapw);
	buf_write_uint32(buf, m->maph);
	buf_write_uint32(buf, m->defx);
	buf_write_uint32(buf, m->defy);
	buf_write_uint32(buf, TILEW);
	buf_write_uint32(buf, TILEH);
	dprintf("flags 0x%x, geo %dx%d, origin at %d,%d, %dx%d tiles\n",
	    m->flags, m->mapw, m->maph, m->defx, m->defy, TILEW, TILEH);

	soffs = buf->offs;			/* Skip */
	buf_write_uint32(buf, 0);

	/* Write the object map. */
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		solen += sizeof(struct object *);
	}
	pobjs = emalloc(solen);
	SLIST_FOREACH(pob, &world->wobjsh, wobjs) {
		if ((pob->flags & OBJECT_ART) == 0 ||
		     pob->flags & OBJECT_CANNOT_MAP) {
			continue;
		}
		buf_write_string(buf, pob->name);
		buf_write_uint32(buf, 0);
		pobjs[nobjs++] = pob;
		dprintf("reference %s\n", pob->name);
	}
	buf_pwrite_uint32(buf, nobjs, soffs);

	pthread_mutex_lock(&m->lock);

	/* Write the nodes. */
	if (m->flags & MAP_RLE_COMPRESSION) {
		map_save_rle_nodes(buf, m, pobjs, nobjs);
	} else {
		map_save_flat_nodes(buf, m, pobjs, nobjs);
	}
	
	pthread_mutex_unlock(&m->lock);

	fobj_flush_buf(buf, fd);

	free(pobjs);
	return (0);
}

/*
 * Grow the map to ensure that m:[mx,my] is a valid node.
 * Map must be locked.
 */
void
map_adjust(struct map *m, Uint32 mx, Uint32 my)
{
	if (mx >= m->mapw) {
		map_grow(m, m->mapw + mx, m->maph);
	}
	if (my >= m->maph) {
		map_grow(m, m->mapw, m->maph + my);
	}
}

#ifdef DEBUG

static void	*map_verify_loop(void *);

/* Verify the integrity of a map, also helps finding races. */
static void *
map_verify_loop(void *arg)
{
	struct map *m = arg;
	int x = 0, y;

	dprintf("checking %s\n", OBJECT(m)->name);

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

#endif	/* DEBUG */

