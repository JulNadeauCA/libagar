/*	$Csoft: map.c,v 1.138 2003/01/27 02:20:52 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include "engine.h"

#include <libfobj/fobj.h>

#include "map.h"
#include "version.h"
#include "config.h"
#include "world.h"
#include "view.h"

static const struct version map_ver = {
	"agar map",
	3, 0
};

static const struct object_ops map_ops = {
	map_destroy,
	map_load,
	map_save
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_NODEREFS	0x02

int	map_debug = DEBUG_STATE;
int	map_nodesigs = 0;
#define engine_debug map_debug
#endif

void
node_init(struct node *node, int x, int y)
{
#ifdef DEBUG
	strncpy(node->magic, NODE_MAGIC, 5);
	node->x = x;
	node->y = y;
#endif
	TAILQ_INIT(&node->nrefs);
	node->flags = 0;
}

void
node_destroy(struct node *node, int x, int y)
{
	struct noderef *nref, *nextnref;

#ifdef DEBUG
	if (strcmp(NODE_MAGIC, node->magic) != 0 ||
	    node->x != x || node->y != y) {
		fatal("inconsistent node\n");
	}
	strncpy(node->magic, "free", 5);
#endif
	for (nref = TAILQ_FIRST(&node->nrefs);
	     nref != TAILQ_END(&node->nrefs);
	     nref = nextnref) {
		nextnref = TAILQ_NEXT(nref, nrefs);
		noderef_destroy(nref);
		free(nref);
	}
}

void
noderef_init(struct noderef *nref)
{
#ifdef DEBUG
	strncpy(nref->magic, NODEREF_MAGIC, 5);
#endif
	nref->type = 0;
	nref->flags = 0;
	nref->pobj = NULL;
	nref->offs = 0;
	nref->xcenter = 0;
	nref->ycenter = 0;
	nref->xmotion = 0;
	nref->ymotion = 0;
	SLIST_INIT(&nref->transforms);
}

void
noderef_destroy(struct noderef *nref)
{
	struct transform *trans, *ntrans;

#ifdef DEBUG
	if (strcmp(NODEREF_MAGIC, nref->magic) != 0) {
		fatal("inconsistent node reference\n");
	}
	strncpy(nref->magic, "free", 5);
#endif

	for (trans = SLIST_FIRST(&nref->transforms);
	     trans != SLIST_END(&nref->transforms);
	     trans = ntrans) {
		ntrans = SLIST_NEXT(trans, transforms);
		transform_destroy(trans);
		free(trans);
	}

	switch (nref->type) {
	case NODEREF_WARP:
		free(nref->data.warp.map);
		break;
	default:
		break;
	}
}

void
map_alloc_nodes(struct map *m, Uint32 w, Uint32 h)
{
	Uint32 i, x, y;

	pthread_mutex_lock(&m->lock);

	m->mapw = w;
	m->maph = h;

	/* Allocate the two-dimensional node array. */
	m->map = emalloc((w * h) * sizeof(struct node *));
	for (i = 0; i < h; i++) {
		*(m->map + i) = emalloc(w * sizeof(struct node));
	}

	/* Initialize the nodes. */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			node_init(&m->map[y][x], x, y);
		}
	}

	pthread_mutex_unlock(&m->lock);
}

void
map_free_nodes(struct map *m)
{
	Uint32 x, y;
	struct node *node;

	pthread_mutex_lock(&m->lock);

	/* Free the node array. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node = &m->map[y][x];
			node_destroy(node, x, y);
		}
		free(*(m->map + y));
	}
	free(m->map);
	m->map = NULL;
	
	pthread_mutex_unlock(&m->lock);
}

/* Shrink a map, freeing the excess nodes. */
void
map_shrink(struct map *m, Uint32 w, Uint32 h)
{
	Uint32 x, y;
	int i;
	struct node *node;
	
	pthread_mutex_lock(&m->lock);
	
	/* Free the excess nodes. */
	for (y = h; y < m->maph; y++) {
		for (x = w; x < m->mapw; x++) {
			node = &m->map[y][x];
			node_destroy(node, x, y);
			free(node);
		}
	}

	/* Reallocate the node array. */
	m->map = erealloc(m->map, (w * h) * sizeof(struct node *));
	if (w < m->mapw) {
		for (i = 0; i < m->maph; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w * sizeof(struct node));
		}
	}
	if (h < m->maph) {
		for (i = m->maph; i < h; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w * sizeof(struct node));
		}
	}

	/* Sync the geometry. */
	m->mapw = w;
	m->maph = h;
	
	pthread_mutex_unlock(&m->lock);
}

/* Grow a map and initialize new nodes. */
void
map_grow(struct map *m, Uint32 w, Uint32 h)
{
	Uint32 x, y;
	int i;

	pthread_mutex_lock(&m->lock);

	/* Reallocate the node array. */
	m->map = erealloc(m->map, (w * h) * sizeof(struct node *));
	if (w > m->mapw) {
		for (i = 0; i < m->maph; i++) {
			*(m->map+i) = erealloc(*(m->map+i),
			    w * sizeof(struct node));
		}
	}
	if (h > m->maph) {
		for (i = m->maph; i < h; i++) {
			*(m->map+i) = emalloc(w * sizeof(struct node));
		}
	}

	/* Initialize the new nodes. */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (x >= m->mapw || y >= m->maph) {
				node_init(&m->map[y][x], x, y);
			}
		}
	}

	/* Sync the geometry. */
	m->mapw = w;
	m->maph = h;

	pthread_mutex_unlock(&m->lock);
}

/* Grow the map to ensure that m:[mx,my] is a valid node. */
void
map_adjust(struct map *m, Uint32 mx, Uint32 my)
{
	pthread_mutex_lock(&m->lock);
	if (mx >= m->mapw)
		map_grow(m, mx, m->maph);
	if (my >= m->maph)
		map_grow(m, m->mapw, my);
	pthread_mutex_unlock(&m->lock);
}

/* Modify the zoom factor. */
void
map_set_zoom(struct map *m, Uint16 zoom)
{
	pthread_mutex_lock(&m->lock);

	m->zoom = zoom;
	m->tilew = m->zoom * TILEW / 100;
	m->tileh = m->zoom * TILEH / 100;

	if (m->tilew > 32767)			/* For soft scrolling */
		m->tilew = 32767;
	if (m->tileh > 32767)
		m->tileh = 32767;

	pthread_mutex_unlock(&m->lock);
}

void
map_init(struct map *m, enum map_type type, char *name, char *media)
{
	object_init(&m->obj, "map", name, media,
	    (media != NULL) ? OBJECT_ART|OBJECT_ART_CAN_FAIL: 0, &map_ops);
	m->type = type;
	m->redraw = 0;
	m->mapw = 0;
	m->maph = 0;
	m->defx = 0;
	m->defy = 0;
	m->map = NULL;
	m->tilew = TILEW;
	m->tileh = TILEH;
	m->zoom = 100;
	pthread_mutexattr_init(&m->lockattr);
	pthread_mutexattr_settype(&m->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m->lock, &m->lockattr);
}

/*
 * Insert a reference to a sprite at pobj:offs.
 * The map containing the node must be locked.
 */
struct noderef *
node_add_sprite(struct node *node, void *pobj, Uint32 offs)
{
	struct noderef *nref;

	nref = emalloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_SPRITE;
	nref->pobj = pobj;
	nref->offs = offs;

#ifdef DEBUG
	if (strcmp(node->magic, NODE_MAGIC) != 0) {
		fatal("inconsistent node\n");
	}
#endif
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	return (nref);
}

/*
 * Insert a reference to an animation at pobj:offs.
 * The map containing the node must be locked.
 */
struct noderef *
node_add_anim(struct node *node, void *pobj, Uint32 offs, Uint32 flags)
{
	struct noderef *nref;

	nref = emalloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_ANIM;
	nref->pobj = pobj;
	nref->offs = offs;
	nref->data.anim.flags = flags;
	nref->data.anim.frame = 0;

#ifdef DEBUG
	if (strcmp(node->magic, NODE_MAGIC) != 0) {
		fatal("inconsistent node\n");
	}
#endif
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);

	node->flags |= NODE_HAS_ANIM;			/* Optimization */
	return (nref);
}

/*
 * Insert a warp point. The destination map name is resolved at runtime.
 * The map containing the node must be locked.
 */
struct noderef *
node_add_warp(struct node *node, char *mapname, Uint32 x, Uint32 y, Uint8 dir)
{
	struct noderef *nref;

	nref = emalloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_WARP;
	nref->data.warp.map = Strdup(mapname);
	nref->data.warp.x = x;
	nref->data.warp.y = y;
	nref->data.warp.dir = dir;

#ifdef DEBUG
	if (strcmp(node->magic, NODE_MAGIC) != 0) {
		fatal("inconsistent node\n");
	}
#endif
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	return (nref);
}

/* Check whether a node points to at least one animation. */
static __inline__ int
node_has_anim(struct node *node)
{
	struct noderef *nref;

	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		if (nref->type == NODEREF_ANIM)
			break;
	}
	return (nref != NULL ? 1 : 0);
}

/*
 * Move a node reference from one node to another, at the tail of the queue.
 * The map(s) containing the source/destination nodes must be locked.
 */
void
node_move_ref(struct noderef *nref, struct node *src_node,
    struct node *dst_node)
{
	TAILQ_REMOVE(&src_node->nrefs, nref, nrefs);
	TAILQ_INSERT_TAIL(&dst_node->nrefs, nref, nrefs);

	if (nref->type == NODEREF_ANIM) {		/* Optimization */
		if (!node_has_anim(src_node)) {
			src_node->flags &= ~(NODE_HAS_ANIM);
		}
		dst_node->flags |= NODE_HAS_ANIM;
	}
}

/*
 * Copy a node reference from one node to another.
 * The map(s) containing the source/destination nodes must be locked.
 */
void
node_copy_ref(struct noderef *src, struct node *dst_node)
{
	struct transform *src_trans;
	struct noderef *dst = NULL;

	/* Allocate a new noderef with the same data. */
	switch (src->type) {
	case NODEREF_SPRITE:
		dst = node_add_sprite(dst_node, src->pobj, src->offs);
		dst->xcenter = src->xcenter;
		dst->ycenter = src->ycenter;
		dst->xmotion = src->xmotion;
		dst->ymotion = src->ymotion;
		break;
	case NODEREF_ANIM:
		dst = node_add_anim(dst_node, src->pobj, src->offs,
		    src->data.anim.flags);
		dst->xcenter = src->xcenter;
		dst->ycenter = src->ycenter;
		dst->xmotion = src->xmotion;
		dst->ymotion = src->ymotion;
		break;
	case NODEREF_WARP:
		dst = node_add_warp(dst_node, src->data.warp.map,
		    src->data.warp.x, src->data.warp.y, src->data.warp.dir);
		break;
	default:
		fatal("bad noderef type\n");
		break;
	}
	dst->flags = src->flags;

	/* Inherit the transformations. */
	SLIST_FOREACH(src_trans, &src->transforms, transforms) {
		struct transform *dst_trans;

		dst_trans = emalloc(sizeof(struct transform));
		transform_copy(src_trans, dst_trans);
		SLIST_INSERT_HEAD(&dst->transforms, dst_trans, transforms);
	}
}

/*
 * Delete a map entry reference.
 * The map containing the node must be locked.
 */
void
node_remove_ref(struct node *node, struct noderef *nref)
{
	TAILQ_REMOVE(&node->nrefs, nref, nrefs);

	noderef_destroy(nref);
	free(nref);
	
	if (!node_has_anim(node)) {			/* Optimization */
		node->flags &= ~(NODE_HAS_ANIM);
	}
}

void
map_destroy(void *p)
{
	struct map *m = p;

	if (m->map != NULL) {
		map_free_nodes(m);
	}
	pthread_mutex_destroy(&m->lock);
	pthread_mutexattr_destroy(&m->lockattr);
}

/*
 * Load a noderef structure, translating the object ids using dependencies.
 * The noderef's parent map must be locked.
 */
void
noderef_load(int fd, struct object_table *deps, struct node *node,
    struct noderef **nref)
{
	enum noderef_type type;
	Uint32 i, ntrans = 0;
	struct transform *trans;
	Uint32 flags;

	/* Read the type of reference and flags. */
	type = read_uint32(fd);
	flags = read_uint32(fd);

	/* Read the reference data. */
	switch (type) {
	case NODEREF_SPRITE:
		{
			Uint32 obji, offs;
			Sint16 xcenter, ycenter;

			obji = read_uint32(fd);		/* Object# */
			if (obji > deps->nobjs) {
				fatal("bad object table index\n");
			}

			offs = read_uint32(fd);		/* Sprite# */
			xcenter = read_sint16(fd);
			ycenter = read_sint16(fd);

			debug(DEBUG_NODEREFS,
			    "sprite: obj[%d]:%d, center %d,%d\n",
			    obji, offs, xcenter, ycenter);

			if (deps->objs[obji] != NULL) {
				*nref = node_add_sprite(node, deps->objs[obji],
				    offs);
				(*nref)->flags = flags;
				(*nref)->xcenter = xcenter;
				(*nref)->ycenter = ycenter;
			} else {
				fatal("missing sprite dep 0x%x\n", obji);
			}
		}
		break;
	case NODEREF_ANIM:
		{
			Uint32 obji, offs, animflags;
			Sint16 xcenter, ycenter;

			obji = read_uint32(fd);
			if (obji > deps->nobjs) {
				fatal("bad object table index\n");
			}
			offs = read_uint32(fd);
			xcenter = read_sint16(fd);
			ycenter = read_sint16(fd);
			animflags = read_uint32(fd);

			if (deps->objs[obji] != NULL) {
				debug(DEBUG_NODEREFS,
				    "anim: %s:%d, center %d,%d, aflags 0x%x\n",
				    deps->objs[obji]->name, offs,
				    xcenter, ycenter, animflags);
				*nref = node_add_anim(node, deps->objs[obji],
				    offs, animflags);
				(*nref)->flags = flags;
				(*nref)->xcenter = xcenter;
				(*nref)->ycenter = ycenter;
			} else {
				fatal("missing anim dep 0x%x\n", obji);
			}
		}
		break;
	case NODEREF_WARP:
		{
			char *map_id;
			Uint32 ox, oy;
			Uint8 dir;

			map_id = read_string(fd, NULL);
			ox = read_uint32(fd);
			oy = read_uint32(fd);
			dir = read_uint8(fd);
			debug(DEBUG_NODEREFS, "warp: to %s:%d,%d, dir 0x%x\n",
			    map_id, ox, oy, dir);
			*nref = node_add_warp(node, map_id, ox, oy, dir);
			(*nref)->flags = flags;
			
			free(map_id);
		}
		break;
	default:
		fatal("unknown noderef type\n");
		break;
	}

	/* Read the transforms. */
	ntrans = read_uint32(fd);
	if (ntrans > 32768) {
		fatal("node has >32k transforms\n");
	}
	for (i = 0; i < ntrans; i++) {
		struct transform *trans;

		trans = emalloc(sizeof(struct transform));
		transform_load(fd, trans);
	}
}

void
node_load(int fd, struct object_table *deps, struct node *node)
{
	Uint32 i, nrefs;
	
	/* Load the node properties. */
	node->flags = read_uint32(fd);
	read_uint32(fd);			/* Pad: v1 */
	
	/* Load the node references. */
	nrefs = read_uint32(fd);
#ifdef DEBUG
	if (nrefs > 32768) {
		fatal("node has >32k references\n");
	}
#endif
	
	for (i = 0; i < nrefs; i++) {
		struct noderef *nref;

		noderef_load(fd, deps, node, &nref);
	}
}

int
map_load(void *ob, int fd)
{
	struct map *m = ob;
	struct object_table *deps;
	Uint32 x, y;

	if (version_read(fd, &map_ver) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&m->lock);

	if (m->map != NULL) {
		/* Reinitialize the map. */
		map_free_nodes(m);
	}

	m->type = (enum map_type)read_uint32(fd);
	m->mapw = read_uint32(fd);
	m->maph = read_uint32(fd);
	m->defx = read_uint32(fd);
	m->defy = read_uint32(fd);
	m->tilew = read_uint32(fd);
	m->tileh = read_uint32(fd);
	m->zoom = read_uint16(fd);

	debug(DEBUG_STATE,
	    "type %s, geo %dx%d, origin at %d,%d, %dx%d tiles, %d%% zoom\n",
	    m->type==MAP_2D ? "2d" : m->type==MAP_3D ? "3d" : "???",
	    m->mapw, m->maph, m->defx, m->defy, m->tilew, m->tileh,
	    m->zoom);

	/* Read the possible dependencies. */
	deps = object_table_load(fd, OBJECT(m)->name);

	/* Allocate and load the nodes. */
	map_alloc_nodes(m, m->mapw, m->maph);
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_load(fd, deps, &m->map[y][x]);
		}
	}
	pthread_mutex_unlock(&m->lock);

	object_table_destroy(deps);
	return (0);
}

/*
 * Save a noderef structure, encoding the object ids using dependencies.
 * The noderef's parent map must be locked.
 */
void
noderef_save(struct fobj_buf *buf, struct object_table *deps,
    struct noderef *nref)
{
	off_t ntrans_offs;
	Uint32 i, ntrans = 0;
	struct transform *trans;
	
	/* Save the type of reference and flags. */
	buf_write_uint32(buf, nref->type);
	buf_write_uint32(buf, nref->flags);

	debug(DEBUG_NODEREFS, "type %d, flags 0x%x\n", nref->type, nref->flags);

	/* Save the reference data. */
	switch (nref->type) {
	case NODEREF_SPRITE:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == nref->pobj)
				break;
		}
		buf_write_uint32(buf, i);		/* Object# */
		buf_write_uint32(buf, nref->offs);	/* Sprite# */
		buf_write_sint16(buf, nref->xcenter);
		buf_write_sint16(buf, nref->ycenter);
		debug(DEBUG_NODEREFS, "sprite: obj[%d]:%d, center %d,%d\n",
		    i, nref->offs, nref->xcenter, nref->ycenter);
		break;
	case NODEREF_ANIM:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == nref->pobj)
				break;
		}
		buf_write_uint32(buf, i);		/* Object# */
		buf_write_uint32(buf, nref->offs);	/* Anim# */
		buf_write_sint16(buf, nref->xcenter);
		buf_write_sint16(buf, nref->ycenter);
		buf_write_uint32(buf, nref->data.anim.flags);
		debug(DEBUG_NODEREFS,
		    "anim: obj[%d]:%d, center %d,%d, aflags 0x%x\n",
		    i, nref->offs, nref->xcenter, nref->ycenter,
		    nref->data.anim.flags);
		break;
	case NODEREF_WARP:
		buf_write_string(buf, nref->data.warp.map);
		buf_write_uint32(buf, nref->data.warp.x);
		buf_write_uint32(buf, nref->data.warp.y);
		buf_write_uint8(buf, nref->data.warp.dir);
		debug(DEBUG_NODEREFS, "warp: to %s:[%d,%d], dir 0x%x",
		    nref->data.warp.map, nref->data.warp.x, nref->data.warp.y,
		    nref->data.warp.dir);
	default:
		debug(DEBUG_NODEREFS, "not saving %d node\n", nref->type);
		break;
	}

	/* Save the transforms. */
	ntrans_offs = buf->offs;
	buf_write_uint32(buf, 0);				/* Skip count */
	SLIST_FOREACH(trans, &nref->transforms, transforms) {
		transform_save(buf, trans);
		ntrans++;
	}
	buf_pwrite_uint32(buf, ntrans, ntrans_offs);		/* Save count */
}

void
node_save(struct fobj_buf *buf, struct object_table *deps, struct node *node)
{
	struct noderef *nref;
	off_t nrefs_offs;
	Uint32 nrefs = 0;
	
	/* Save the node properties. */
	buf_write_uint32(buf, node->flags & ~(NODE_EPHEMERAL));
	buf_write_uint32(buf, 0);			/* Pad: v1 */

	/* Save the node references. */
	nrefs_offs = buf->offs;
	buf_write_uint32(buf, 0);			/* Skip */
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		if ((nref->flags & NODEREF_SAVEABLE) == 0)
			continue;
		noderef_save(buf, deps, nref);
		nrefs++;
	}
	buf_pwrite_uint32(buf, nrefs, nrefs_offs);
}

int
map_save(void *p, int fd)
{
	struct map *m = p;
	struct fobj_buf *buf;
	struct object *pob;
	struct object_table *deps;
	Uint32 x, y;
	
	version_write(fd, &map_ver);

	buf = fobj_create_buf(65536, 32767);

	pthread_mutex_lock(&m->lock);

	debug(DEBUG_STATE,
	    "type %s, geo %dx%d, origin at %d,%d, %dx%d tiles\n",
	    m->type==MAP_2D ? "2d" : m->type==MAP_3D ? "3d" : "???",
	    m->mapw, m->maph, m->defx, m->defy, TILEW, TILEH);

	buf_write_uint32(buf, (Uint32)m->type);
	buf_write_uint32(buf, m->mapw);
	buf_write_uint32(buf, m->maph);
	buf_write_uint32(buf, m->defx);
	buf_write_uint32(buf, m->defy);
	buf_write_uint32(buf, m->tilew);
	buf_write_uint32(buf, m->tileh);
	buf_write_uint16(buf, m->zoom);

	/* Write the dependencies. */
	deps = object_table_new();
	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(pob, &world->wobjs, wobjs) {
		if ((pob->flags & OBJECT_ART) == 0 ||		/* XXX */
		     pob->flags & OBJECT_CANNOT_MAP) {
			continue;
		} 
		object_table_insert(deps, pob);
	}
	pthread_mutex_unlock(&world->lock);
	object_table_save(buf, deps);

	/* Write the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_save(buf, deps, &m->map[y][x]);
		}
	}
	
	pthread_mutex_unlock(&m->lock);

	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	object_table_destroy(deps);
	return (0);
}

#ifdef DEBUG

static void	*map_verify_loop(void *);

/* Verify the integrity of a map. This may also help finding races. */
static void *
map_verify_loop(void *arg)
{
	struct map *m = arg;
	struct node *n;
	int x = 0, y;

	for (;;) {
		pthread_mutex_lock(&m->lock);
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				struct noderef *nref;
				
				n = &m->map[y][x];
				if (strcmp(n->magic, NODE_MAGIC) != 0) {
					fatal("bad node magic\n");
				}
				TAILQ_FOREACH(nref, &n->nrefs, nrefs) {
					if (strcmp(nref->magic, NODEREF_MAGIC)
					    != 0) {
						fatal("bad noderef magic\n");
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

	Pthread_create(&verify_th, NULL, map_verify_loop, m);
}

#endif	/* DEBUG */

static __inline__ void
noderef_draw_scaled(struct map *m, SDL_Surface *s, Sint16 rx, Sint16 ry)
{
	int x, y, dh, dw, sx, sy;
	Uint8 *src, r1, g1, b1, a1;
	SDL_Rect clip;

	dw = s->w * m->zoom / 100;
	dh = s->h * m->zoom / 100;

	/* XXX cache the scaled surfaces. */
	/* XXX inefficient */
	SDL_LockSurface(view->v);
	for (y = 0; y < dh; y++) {
		if ((sy = y * TILEH / m->tileh) >= s->h)
			break;
		for (x = 0; x < dw; x++) {
			if ((sx = x * TILEW / m->tilew) >= s->w)
				break;
			src = (Uint8 *)s->pixels +
			    sy*s->pitch +
			    sx*s->format->BytesPerPixel;
			if (s->flags & SDL_SRCALPHA) {
				SDL_GetRGBA(*(Uint32 *)src, s->format,
				    &r1, &g1, &b1, &a1);
				view_alpha_blend(view->v, rx+x, ry+y,
				    r1, g1, b1, a1);
			} else {
				SDL_GetRGB(*(Uint32 *)src, s->format,
				    &r1, &g1, &b1);
				VIEW_PUT_PIXEL(view->v, rx+x, ry+y,
				    SDL_MapRGB(view->v->format, r1, g1, b1));
			}
		}
	}
	SDL_UnlockSurface(view->v);
}

/*
 * Render a map node.
 * Map must be locked.
 */
__inline__ void
noderef_draw(struct map *m, struct noderef *nref, Sint16 rx, Sint16 ry)
{
	SDL_Rect rd;
	SDL_Surface *su;
	
	switch (nref->type) {
	case NODEREF_SPRITE:
		su = SPRITE(nref->pobj, nref->offs);
		break;
	case NODEREF_ANIM:
		{
			struct art_anim *anim;
			
			anim = ANIM(nref->pobj, nref->offs);
			if (anim == NULL) {
				fatal("bad anim\n");
			}
			su = anim->frames[anim->frame];
			
			/* XXX do this somewhere else! */
			art_anim_tick(anim, nref);
		}
		break;
	default:				/* Not a drawable */
		return;
	}
		
	if (m->zoom != 100) {
		rd.x = rx + (nref->xcenter * m->zoom / 100) +
		    (nref->xmotion * m->zoom / 100);
		rd.y = ry + (nref->ycenter * m->zoom / 100) +
		    (nref->ymotion * m->zoom / 100);

		noderef_draw_scaled(m, su, rd.x, rd.y);
	} else {
		rd.x = rx + nref->xcenter + nref->xmotion;
		rd.y = ry + nref->ycenter + nref->ymotion;

		SDL_BlitSurface(su, NULL, view->v, &rd);
	}
}

