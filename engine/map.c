/*	$Csoft: map.c,v 1.158 2003/03/10 02:13:39 vedge Exp $	*/

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
	4, 2
};

static const struct object_ops map_ops = {
	map_destroy,
	map_load,
	map_save
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_NODEREFS	0x02
#define DEBUG_RESIZE	0x04
#define DEBUG_SCAN	0x08

int	map_debug = DEBUG_STATE|DEBUG_RESIZE;
int	map_nodesigs = 1;
#define engine_debug map_debug

# ifdef THREADS
static void	*map_check(void *);
# endif
#endif /* DEBUG */

static void	 map_layer_init(struct map_layer *lay, char *);
static void	 map_layer_destroy(struct map_layer *);

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
node_destroy(struct node *node)
{
	struct noderef *nref, *nextnref;

#ifdef DEBUG
	if (strcmp(NODE_MAGIC, node->magic) != 0)
		fatal("bad node\n");
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
	nref->layer = 0;
	SLIST_INIT(&nref->transforms);
}

void
noderef_destroy(struct noderef *nref)
{
	struct transform *trans, *ntrans;

#ifdef DEBUG
	if (strcmp(NODEREF_MAGIC, nref->magic) != 0)
		fatal("bad nref\n");
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

int
map_alloc_nodes(struct map *m, unsigned int w, unsigned int h)
{
	int x, y;
	
	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set("%ux%u nodes > %ux%u", w, h,
		    MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
		return (-1);
	}

	pthread_mutex_lock(&m->lock);

	m->mapw = w;
	m->maph = h;

	m->map = emalloc(h * sizeof(struct node *));
	for (y = 0; y < h; y++) {
		m->map[y] = emalloc(w * sizeof(struct node));

		for (x = 0; x < w; x++) {
			node_init(&m->map[y][x], x, y);
		}
	}

	pthread_mutex_unlock(&m->lock);
	return (0);
}

void
map_free_nodes(struct map *m)
{
	int x, y;
	struct node *node;

	pthread_mutex_lock(&m->lock);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node = &m->map[y][x];
			MAP_CHECK_NODE(node, x, y);
			node_destroy(node);
		}
		free(m->map[y]);
	}
	free(m->map);
	m->map = NULL;
	
	pthread_mutex_unlock(&m->lock);
}

static void
map_free_layers(struct map *m)
{
	int i;
	
	m->nlayers = 0;
	for (i = 0; i < m->nlayers; i++) {
		map_layer_destroy(&m->layers[i]);
	}
	free(m->layers);
	m->layers = NULL;
}

/* Shrink a map, freeing the excess nodes. */
void
map_shrink(struct map *m, unsigned int w, unsigned int h)
{
	struct node *node;
	int x, y;
	
	debug(DEBUG_RESIZE, "%ux%u -> %ux%u\n", m->mapw, m->maph, w, h);
	
	pthread_mutex_lock(&m->lock);
	
	/* Free the excess nodes. */
	for (y = h; y < m->maph; y++) {
		for (x = w; x < m->mapw; x++) {
			node = &m->map[y][x];
			MAP_CHECK_NODE(node, x, y);
			node_destroy(node);
			free(node);
		}
	}

	/* Reallocate the node arrays. */
	m->map = erealloc(m->map, h * sizeof(struct node *));
	if (w < m->mapw) {
		for (y = 0; y < m->maph; y++) {
			m->map[y] = erealloc(m->map[y],
			    w * sizeof(struct node));
		}
	}

	m->mapw = w;
	m->maph = h;
	
	pthread_mutex_unlock(&m->lock);
}

/* Grow a map and initialize the new nodes. */
int
map_grow(struct map *m, unsigned int w, unsigned int h)
{
	int i, x, y;

	debug(DEBUG_RESIZE, "%ux%u -> %ux%u\n", m->mapw, m->maph, w, h);

	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set("%ux%u nodes > %ux%u", w, h,
		    MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
		return (-1);
	}

	pthread_mutex_lock(&m->lock);

	/* Reallocate the node arrays. */
	m->map = erealloc(m->map, h * sizeof(struct node *));
	for (y = 0; y < h; y++) {
		if (y > m->maph-1) {			/* New row */
			m->map[y] = emalloc(w * sizeof(struct node));
			for (x = 0; x < w; x++) {
				node_init(&m->map[y][x], x, y);
			}
		} else {				/* Existing row */
			if (w > m->mapw) {
				m->map[y] = erealloc(m->map[y],
				    w * sizeof(struct node));
				for (x = m->mapw-1; x < w; x++) {
					node_init(&m->map[y][x], x, y);
				}
			}
		}
	}
		
	m->mapw = w;
	m->maph = h;

	pthread_mutex_unlock(&m->lock);

	return (0);
}

/* Grow the map to ensure that m:[mx,my] is a valid node. */
int
map_adjust(struct map *m, int mx, int my)
{
	int rv = 0;

	debug(DEBUG_RESIZE, "%d,%d\n", mx, my);

	if (mx > MAP_MAX_WIDTH || my > MAP_MAX_HEIGHT) {
		error_set("%ux%u nodes > %ux%u", mx, my,
		    MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
		return (-1);
	}

	pthread_mutex_lock(&m->lock);
	if (mx > m->mapw) {
		if (map_grow(m, mx, m->maph) == -1) {
			rv = -1;
		}
	}
	if (my > m->maph) {
		if (map_grow(m, m->mapw, my) == -1) {
			rv = -1;
		}
	}
	pthread_mutex_unlock(&m->lock);

	return (rv);
}

/* Modify the zoom factor. */
void
map_set_zoom(struct map *m, Uint16 zoom)
{
	pthread_mutex_lock(&m->lock);

	m->zoom = zoom;
	m->tilew = m->zoom * TILEW / 100;
	m->tileh = m->zoom * TILEH / 100;

	if (m->tilew > 16384)			/* For soft scrolling */
		m->tilew = 16384;
	if (m->tileh > 16384)
		m->tileh = 16384;

	pthread_mutex_unlock(&m->lock);
}

void
map_init(struct map *m, enum map_type type, char *name, char *media)
{
	int i;

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
	m->ssx = TILEW;
	m->ssy = TILEH;

	m->layers = emalloc(sizeof(struct map_layer));
	m->nlayers = 1;
	map_layer_init(&m->layers[0], "Layer 0");

	pthread_mutexattr_init(&m->lockattr);
	pthread_mutexattr_settype(&m->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m->lock, &m->lockattr);

	OBJECT(m)->flags |= OBJECT_CONSISTENT;

#if defined(DEBUG) && defined(THREADS)
	if (map_debug & DEBUG_SCAN) {
		Pthread_create(&m->check_th, NULL, map_check, m);
	}
#endif
}

static void
map_layer_init(struct map_layer *lay, char *name)
{
	lay->name = Strdup(name);
	lay->visible = 1;
	lay->xinc = 1;
	lay->yinc = 1;
	lay->alpha = SDL_ALPHA_OPAQUE;
}

static void
map_layer_destroy(struct map_layer *lay)
{
	free(lay->name);
}

/* Create a new layer. */
int
map_push_layer(struct map *m, char *name)
{
	if (m->nlayers+1 > 256) {
		error_set("too many layers");
		return (-1);
	}
	m->layers = erealloc(m->layers,
	    (m->nlayers+1) * sizeof(struct map_layer));
	if (name == NULL) {				/* Default name */
		char *s;

		Asprintf(&s, "Layer %d", m->nlayers);
		map_layer_init(&m->layers[m->nlayers], s);
		free(s);
	} else {
		map_layer_init(&m->layers[m->nlayers], name);
	}
	m->nlayers++;
	return (0);
}

/* Remove the last layer. */
void
map_pop_layer(struct map *m)
{
	if (--m->nlayers < 1) {
		m->nlayers = 1;
	}
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
		fatal("bad node\n");
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
node_add_anim(struct node *node, void *pobj, Uint32 offs, Uint8 flags)
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
		fatal("bad node\n");
	}
#endif
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	return (nref);
}

/*
 * Insert a warp point. The destination map name is resolved at runtime.
 * The map containing the node must be locked.
 */
struct noderef *
node_add_warp(struct node *node, char *mapname, int x, int y, Uint8 dir)
{
	struct noderef *nref;

	if (x > MAP_MAX_WIDTH || y > MAP_MAX_HEIGHT)
		fatal("bad warp coords\n");

	nref = emalloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_WARP;
	nref->data.warp.map = Strdup(mapname);
	nref->data.warp.x = x;
	nref->data.warp.y = y;
	nref->data.warp.dir = dir;

#ifdef DEBUG
	if (strcmp(node->magic, NODE_MAGIC) != 0) {
		fatal("bad node\n");
	}
#endif
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	return (nref);
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
}

/*
 * Copy a node reference from one node to another.
 * The map(s) containing the source/destination nodes must be locked.
 */
struct noderef *
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
	dst->layer = src->layer;

	/* Inherit the transformations. */
	SLIST_FOREACH(src_trans, &src->transforms, transforms) {
		struct transform *dst_trans;

		dst_trans = emalloc(sizeof(struct transform));
		transform_copy(src_trans, dst_trans);
		SLIST_INSERT_HEAD(&dst->transforms, dst_trans, transforms);
	}
	return (dst);
}

/*
 * Remove a noderef from a node.
 * The map containing the node must be locked.
 */
void
node_remove_ref(struct node *node, struct noderef *nref)
{
	TAILQ_REMOVE(&node->nrefs, nref, nrefs);
	noderef_destroy(nref);
	free(nref);
}

/* Remove all references on a specific layer. */
void
node_clear_layer(struct node *node, Uint8 layer)
{
	struct noderef *nref, *nnref;

	for (nref = TAILQ_FIRST(&node->nrefs);
	     nref != TAILQ_END(&node->nrefs);
	     nref = nnref) {
		nnref = TAILQ_NEXT(nref, nrefs);
		if (nref->layer == layer) {
			TAILQ_REMOVE(&node->nrefs, nref, nrefs);
			noderef_destroy(nref);
			free(nref);
		}
	}
}

/*
 * Move a noderef to the upper layer.
 * The map containing the node must be locked.
 */
void
node_moveup_ref(struct node *node, struct noderef *nref)
{
	struct noderef *next = TAILQ_NEXT(nref, nrefs);

	if (next != NULL) {
		TAILQ_REMOVE(&node->nrefs, nref, nrefs);
		TAILQ_INSERT_AFTER(&node->nrefs, next, nref, nrefs);
	}
	
}

/*
 * Move a noderef to the lower layer.
 * The map containing the node must be locked.
 */
void
node_movedown_ref(struct node *node, struct noderef *nref)
{
	struct noderef *prev = TAILQ_PREV(nref, noderefq, nrefs);

	if (prev != NULL) {
		TAILQ_REMOVE(&node->nrefs, nref, nrefs);
		TAILQ_INSERT_BEFORE(prev, nref, nrefs);
	}
}

/*
 * Move a noderef to the tail of the queue.
 * The map containing the node must be locked.
 */
void
node_movetail_ref(struct node *node, struct noderef *nref)
{
	if (nref != TAILQ_LAST(&node->nrefs, noderefq)) {
		TAILQ_REMOVE(&node->nrefs, nref, nrefs);
		TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	}
}

/*
 * Move a noderef to the head of the queue.
 * The map containing the node must be locked.
 */
void
node_movehead_ref(struct node *node, struct noderef *nref)
{
	if (nref != TAILQ_FIRST(&node->nrefs)) {
		TAILQ_REMOVE(&node->nrefs, nref, nrefs);
		TAILQ_INSERT_HEAD(&node->nrefs, nref, nrefs);
	}
}

void
map_destroy(void *p)
{
	struct map *m = p;
	int i = 0;

#if defined(DEBUG) && defined(THREADS)
	if (map_debug & DEBUG_SCAN) {
		pthread_kill(m->check_th, SIGKILL);
	}
#endif
	if (m->map != NULL)
		map_free_nodes(m);
	if (m->layers != NULL)
		map_free_layers(m);

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
	Uint8 layer;

	/* Read the type of reference, flags and the layer#. */
	type = read_uint32(fd);
	flags = read_uint32(fd);
	layer = read_uint8(fd);

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
				(*nref)->layer = layer;
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
				(*nref)->layer = layer;
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
			(*nref)->layer = layer;
			
			free(map_id);
		}
		break;
	default:
		fatal("unknown noderef type\n");
		break;
	}

	/* Read the transforms. */
	ntrans = read_uint32(fd);
	if (ntrans > 0xffff) {
		fatal("node has >64k transforms\n");
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
	struct noderef *nref;
	
	/* Load the node properties. */
	node->flags = read_uint32(fd);
	node->flags &= ~(NODE_EPHEMERAL);
	read_uint32(fd);			/* Pad: v1 */
	
	/* Load the node references. */
	nrefs = read_uint32(fd);
#ifdef DEBUG
	if (nrefs > 0xffff) {
		fatal("node has >64k references\n");
	}
#endif
	for (i = 0; i < nrefs; i++) {
		noderef_load(fd, deps, node, &nref);
	}
}

static void
map_layer_load(int fd, struct map *m, struct map_layer *lay)
{
	lay->name = read_string(fd, NULL);
	lay->visible = (int)read_uint8(fd);
	lay->xinc = read_sint16(fd);
	lay->yinc = read_sint16(fd);
	lay->alpha = read_uint8(fd);
}

int
map_load(void *ob, int fd)
{
	struct map *m = ob;
	struct object_table *deps;
	struct version ver;
	Uint32 w, h, defx, defy, tilew, tileh;
	int x, y;

	if (version_read(fd, &map_ver, &ver) != 0)
		return (-1);

	pthread_mutex_lock(&m->lock);

	if (m->map != NULL)
		map_free_nodes(m);
	if (ver.minor >= 2) {				/* Multilayering */
		if (m->layers != NULL)
			map_free_layers(m);
	}

	m->type = (enum map_type)read_uint32(fd);
	w = read_uint32(fd);
	h = read_uint32(fd);
	defx = read_uint32(fd);
	defy = read_uint32(fd);
	tilew = read_uint32(fd);
	tileh = read_uint32(fd);
	if (w > 65535 || h > 65535 ||
	    defx > 65535 || defy > 65535 ||
	    tilew > 65535 || tileh > 65535) {
		error_set("bad geo/coords");
	}
	m->mapw = (unsigned int)w;
	m->maph = (unsigned int)h;
	m->defx = (int)defx;
	m->defy = (int)defy;
	m->tilew = (int)tilew;
	m->tileh = (int)tileh;

	m->zoom = read_uint16(fd);
	if (ver.minor >= 1) {				/* Soft-scroll offs */
		m->ssx = read_sint16(fd);
		m->ssy = read_sint16(fd);
	}
	if (ver.minor >= 2) {				/* Multilayering */
		int i;

		m->nlayers = read_uint32(fd);
		debug(DEBUG_STATE, "%d layers\n", m->nlayers);
		m->layers = emalloc(m->nlayers * sizeof(struct map_layer));
		for (i = 0; i < m->nlayers; i++) {
			map_layer_load(fd, m, &m->layers[i]);
		}
	}

	debug(DEBUG_STATE,
	    "type %s, geo %ux%u, origin at %d,%d, %dx%d tiles, %d%% zoom\n",
	    m->type == MAP_2D ? "2d" : m->type == MAP_3D ? "3d" : "???",
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
	buf_write_uint8(buf, nref->layer);

	debug(DEBUG_NODEREFS, "type %d, flags 0x%x, layer %d\n",
	    nref->type, nref->flags, nref->layer);

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
		buf_write_uint32(buf, (Uint32)nref->data.anim.flags);
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
		MAP_CHECK_NODEREF(nref);
		if ((nref->flags & NODEREF_SAVEABLE) == 0)
			continue;
		noderef_save(buf, deps, nref);
		nrefs++;
	}
	buf_pwrite_uint32(buf, nrefs, nrefs_offs);
}

static void
map_layer_save(struct fobj_buf *buf, struct map_layer *lay)
{
	buf_write_string(buf, lay->name);
	buf_write_uint8(buf, (Uint8)lay->visible);
	buf_write_sint16(buf, lay->xinc);
	buf_write_sint16(buf, lay->yinc);
	buf_write_uint8(buf, lay->alpha);
}

int
map_save(void *p, int fd)
{
	struct map *m = p;
	struct fobj_buf *buf;
	struct object *pob;
	struct object_table *deps;
	int x, y;
	
	version_write(fd, &map_ver);

	buf = fobj_create_buf(65536, 32767);

	pthread_mutex_lock(&m->lock);

	debug(DEBUG_STATE,
	    "type %s, geo %dx%d, origin at %d,%d, %dx%d tiles\n",
	    m->type==MAP_2D ? "2d" : m->type==MAP_3D ? "3d" : "???",
	    m->mapw, m->maph, m->defx, m->defy, TILEW, TILEH);

	buf_write_uint32(buf, (Uint32)m->type);
	buf_write_uint32(buf, (Uint32)m->mapw);
	buf_write_uint32(buf, (Uint32)m->maph);
	buf_write_uint32(buf, (Uint32)m->defx);
	buf_write_uint32(buf, (Uint32)m->defy);
	buf_write_uint32(buf, (Uint32)m->tilew);
	buf_write_uint32(buf, (Uint32)m->tileh);
	buf_write_uint16(buf, m->zoom);
	if (map_ver.minor >= 1) {			/* Soft-scroll offs */
		buf_write_sint16(buf, m->ssx);
		buf_write_sint16(buf, m->ssy);
	}
	if (map_ver.minor >= 2) {			/* Multilayering */
		int i;

		buf_write_uint32(buf, m->nlayers);
		for (i = 0; i < m->nlayers; i++) {
			map_layer_save(buf, &m->layers[i]);
		}
	}

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
			MAP_CHECK_NODE(&m->map[y][x], x, y);
			node_save(buf, deps, &m->map[y][x]);
		}
	}
	
	pthread_mutex_unlock(&m->lock);

	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	object_table_destroy(deps);
	return (0);
}

#if defined(DEBUG) && defined(THREADS)

/* Verify the integrity of a map. This may also help finding races. */
void *
map_check(void *arg)
{
	struct map *m = arg;
	struct node *node;
	struct noderef *nref;
	int x, y;

	for (;;) {
		pthread_mutex_lock(&m->lock);
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				node = &m->map[y][x];

				MAP_CHECK_NODE(node, x, y);
				TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
					MAP_CHECK_NODEREF(nref);
				}
			}
		}
		pthread_mutex_unlock(&m->lock);
		SDL_Delay(1000);

		if ((map_debug & DEBUG_SCAN) == 0)
			return (NULL);
	}
	return (NULL);
}

#endif	/* DEBUG && THREADS */

static void
noderef_draw_scaled(struct map *m, SDL_Surface *s, Sint16 rx, Sint16 ry)
{
	SDL_Rect clip;
	int x, y, dh, dw, sx, sy;
	Uint8 *src, r1, g1, b1, a1;

	dw = s->w * m->zoom / 100;
	dh = s->h * m->zoom / 100;

	/* XXX cache the scaled surfaces. */
	/* XXX inefficient */
	/* XXX no support for SDL_RLEACCEL! */

	if (SDL_MUSTLOCK(view->v))
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
	if (SDL_MUSTLOCK(view->v))
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


