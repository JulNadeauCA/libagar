/*	$Csoft: map.c,v 1.171 2003/04/24 01:03:06 vedge Exp $	*/

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

#include <engine/compat/snprintf.h>
#include <engine/engine.h>

#include <libfobj/fobj.h>

#include <engine/map.h>
#include <engine/version.h>
#include <engine/config.h>
#include <engine/world.h>
#include <engine/view.h>

static const struct version map_ver = {
	"agar map",
	4, 4
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
node_init(struct node *node)
{
#ifdef DEBUG
	node->magic = NODE_MAGIC;
#endif
	TAILQ_INIT(&node->nrefs);
	node->flags = 0;
}

void
node_destroy(struct node *node)
{
	struct noderef *nref, *nextnref;

#ifdef DEBUG
	if (node->magic != NODE_MAGIC)
		fatal("bad node");
	node->magic = 0;
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
	nref->magic = NODEREF_MAGIC;
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

/*
 * Adjust noderef centering offsets.
 * The parent map, if any, must be locked.
 */
int
noderef_set_center(struct noderef *nref, int xcenter, int ycenter)
{
	if (xcenter > -NODEREF_MAX_CENTER &&
	    xcenter < NODEREF_MAX_CENTER &&
	    ycenter > -NODEREF_MAX_CENTER &&
	    ycenter < NODEREF_MAX_CENTER) {
		nref->xcenter = (Sint16)xcenter;
		nref->ycenter = (Sint16)ycenter;
		return (0);
	} else {
		return (-1);
	}
}

/*
 * Adjust noderef motion offsets.
 * The parent map, if any, must be locked.
 */
int
noderef_set_motion(struct noderef *nref, int xmotion, int ymotion)
{
	if (xmotion > -NODEREF_MAX_CENTER &&
	    xmotion < NODEREF_MAX_CENTER &&
	    ymotion > -NODEREF_MAX_CENTER &&
	    ymotion < NODEREF_MAX_CENTER) {
		nref->xmotion = (Sint16)xmotion;
		nref->ymotion = (Sint16)ymotion;
		return (0);
	} else {
		return (-1);
	}
}

void
noderef_destroy(struct noderef *nref)
{
	struct transform *trans, *ntrans;

#ifdef DEBUG
	if (nref->magic != NODEREF_MAGIC)
		fatal("bad nref");
	nref->magic = 0;
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

	m->map = Malloc(h * sizeof(struct node *));
	for (y = 0; y < h; y++) {
		m->map[y] = Malloc(w * sizeof(struct node));

		for (x = 0; x < w; x++) {
			node_init(&m->map[y][x]);
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
			MAP_CHECK_NODE(node);
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

int
map_resize(struct map *m, unsigned int w, unsigned int h)
{
	int sx, sy, dx, dy;
	struct noderef *nref, *nnref;
	struct node **nmap;

	debug(DEBUG_RESIZE, "%ux%u -> %ux%u\n", m->mapw, m->maph, w, h);

	if (w < MAP_MIN_WIDTH || h < MAP_MIN_HEIGHT) {
		error_set("too small");
		return (-1);
	} else if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set("too big");
		return (-1);
	}

	pthread_mutex_lock(&m->lock);

	/* Allocate and initialize new node arrays. */
	nmap = Malloc(h * sizeof(struct node *));
	for (dy = 0; dy < h; dy++) {
		nmap[dy] = Malloc(w * sizeof(struct node));
		for (dx = 0; dx < w; dx++) {
			node_init(&nmap[dy][dx]);
		}
	}

	/* Copy the nodes over. */
	for (sy = 0, dy = 0;
	     sy < m->maph && dy < h;
	     sy++, dy++) {
		for (sx = 0, dx = 0;
		     sx < m->mapw && dx < w;
		     sx++, dx++) {
			struct node *srcnode = &m->map[sy][sx];
			struct node *dstnode = &nmap[dy][dx];
		
			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
				nnref = node_copy_ref(nref, dstnode);
				nnref->layer = nref->layer;
			}
			dstnode->flags = srcnode->flags;
		}
	}

	map_free_nodes(m);
	m->map = nmap;
	m->mapw = w;
	m->maph = h;
	if (m->origin.x >= w)
		m->origin.x = w-1;
	if (m->origin.y >= h)
		m->origin.y = h-1;

	pthread_mutex_unlock(&m->lock);
	return (0);
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
map_init(struct map *m, char *name, char *media)
{
	object_init(&m->obj, "map", name, media,
	    (media != NULL) ? OBJECT_ART|OBJECT_ART_CAN_FAIL: 0, &map_ops);
	m->redraw = 0;
	m->mapw = 0;
	m->maph = 0;
	m->origin.x = 0;
	m->origin.y = 0;
	m->origin.layer = 0;
	m->map = NULL;
	m->tilew = TILEW;
	m->tileh = TILEH;
	m->zoom = 100;
	m->ssx = TILEW;
	m->ssy = TILEH;
	m->cur_layer = 0;

	m->layers = Malloc(sizeof(struct map_layer));
	m->nlayers = 1;
	map_layer_init(&m->layers[0], "Layer 0");
	pthread_mutex_init(&m->lock, &recursive_mutexattr);

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
	char layname[MAP_LAYER_NAME_MAX];

	if (name != NULL) {
		if (snprintf(layname, sizeof(layname), "%s", name) >=
		    sizeof(layname)) {
			error_set("layer name too big");
		}
	} else {
		snprintf(layname, sizeof(layname), "Layer %u", m->nlayers);
	}

	if (m->nlayers+1 > MAP_MAX_LAYERS) {
		error_set("too many layers");
		return (-1);
	}
	m->layers = Realloc(m->layers,
	    (m->nlayers+1) * sizeof(struct map_layer));
	map_layer_init(&m->layers[m->nlayers], layname);
	m->nlayers++;
	return (0);
}

/* Remove the last layer. */
void
map_pop_layer(struct map *m)
{
	if (--m->nlayers < 1)
		m->nlayers = 1;
}

/*
 * Insert a reference to a sprite at pobj:offs.
 * The map containing the node must be locked.
 */
struct noderef *
node_add_sprite(struct node *node, void *pobj, Uint32 offs)
{
	struct noderef *nref;

	nref = Malloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_SPRITE;
	nref->pobj = pobj;
	nref->offs = offs;
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

	nref = Malloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_ANIM;
	nref->pobj = pobj;
	nref->offs = offs;
	nref->data.anim.flags = flags;
	nref->data.anim.frame = 0;
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
		fatal("bad warp coords");

	nref = Malloc(sizeof(struct noderef));
	noderef_init(nref);
	nref->type = NODEREF_WARP;
	nref->data.warp.map = Strdup(mapname);
	nref->data.warp.x = x;
	nref->data.warp.y = y;
	nref->data.warp.dir = dir;
	TAILQ_INSERT_TAIL(&node->nrefs, nref, nrefs);
	return (nref);
}

/*
 * Move a node reference from one node to another, at the tail of the queue.
 * The map(s) containing the source/destination nodes must be locked.
 */
void
node_move_ref_direct(struct noderef *nref, struct node *src_node,
    struct node *dst_node)
{
	TAILQ_REMOVE(&src_node->nrefs, nref, nrefs);
	TAILQ_INSERT_TAIL(&dst_node->nrefs, nref, nrefs);
}

/*
 * Move a node reference from one node to dmap:dx,dy, at the tail of the queue.
 * The map(s) containing the source/destination nodes must be locked.
 */
int
node_move_ref(struct noderef *nref, struct node *src_node,
    struct map *dmap, int dx, int dy)
{
	struct node *dst_node;

	if (dx < 0 || dy < 0 ||
	    dx >= dmap->mapw || dy >= dmap->maph) {
		error_set("exceeds boundaries");
		return (-1);
	}

	dst_node = &dmap->map[dy][dx];
	TAILQ_REMOVE(&src_node->nrefs, nref, nrefs);
	TAILQ_INSERT_TAIL(&dst_node->nrefs, nref, nrefs);
	return (0);
}

/*
 * Copy a node reference from one node to another.
 * The map(s) containing the source/destination nodes must be locked.
 */
struct noderef *
node_copy_ref(struct noderef *src, struct node *dst_node)
{
	struct transform *trans;
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
		fatal("bad nref type");
		break;
	}
	dst->flags = src->flags;
	dst->layer = src->layer;

	/* Inherit the transformations. */
	SLIST_FOREACH(trans, &src->transforms, transforms) {
		struct transform *ntrans;

		ntrans = Malloc(sizeof(struct transform));
		transform_init(ntrans, trans->type, trans->nargs, trans->args);
		SLIST_INSERT_HEAD(&dst->transforms, ntrans, transforms);
	}
	return (dst);
}

/*
 * Remove a noderef from a node and free it.
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
}

/*
 * Load a noderef structure.
 * The noderef's parent map must be locked.
 */
int
noderef_load(struct netbuf *buf, struct object_table *deps, struct node *node,
    struct noderef **nref)
{
	enum noderef_type type;
	Uint32 ntrans = 0;
	Uint8 flags;
	Uint8 layer;
	int i;

	/* Read the type of reference, flags and the layer#. */
	type = read_uint32(buf);
	flags = (Uint8)read_uint32(buf);
	layer = read_uint8(buf);

	/* Read the reference data. */
	switch (type) {
	case NODEREF_SPRITE:
		{
			Uint32 obji, offs;
			Sint16 xcenter, ycenter;

			obji = read_uint32(buf);		/* Object# */
			if (obji > deps->nobjs) {
				error_set("bad sprite obj#");
				return (-1);
			}
			offs = read_uint32(buf);		/* Sprite# */
			xcenter = read_sint16(buf);
			ycenter = read_sint16(buf);

			debug(DEBUG_NODEREFS,
			    "sprite: obj[%u]:%u, center %d,%d\n",
			    obji, offs, xcenter, ycenter);

			if (deps->objs[obji] != NULL) {
				*nref = node_add_sprite(node, deps->objs[obji],
				    offs);
				if (*nref == NULL)
					return (-1);
				(*nref)->flags = flags;
				(*nref)->xcenter = xcenter;
				(*nref)->ycenter = ycenter;
				(*nref)->layer = layer;
			} else {
				error_set("missing sprite dep");
				return (-1);
			}
		}
		break;
	case NODEREF_ANIM:
		{
			Uint32 obji, offs, animflags;
			Sint16 xcenter, ycenter;

			obji = read_uint32(buf);
			if (obji > deps->nobjs) {
				error_set("bad anim obj#");
				return (-1);
			}
			offs = read_uint32(buf);
			xcenter = read_sint16(buf);
			ycenter = read_sint16(buf);
			animflags = read_uint32(buf);

			if (deps->objs[obji] != NULL) {
				debug(DEBUG_NODEREFS,
				    "anim: %s:%u, center %d,%d, aflags 0x%X\n",
				    deps->objs[obji]->name, offs,
				    xcenter, ycenter, animflags);

				*nref = node_add_anim(node, deps->objs[obji],
				    offs, animflags);
				if (*nref == NULL)
					return (-1);
				(*nref)->flags = flags;
				(*nref)->xcenter = xcenter;
				(*nref)->ycenter = ycenter;
				(*nref)->layer = layer;
			} else {
				error_set("missing anim dep");
				return (-1);
			}
		}
		break;
	case NODEREF_WARP:
		{
			char map_id[OBJECT_NAME_MAX];
			Uint32 ox, oy;
			Uint8 dir;

			if (copy_string(map_id, buf, sizeof(map_id)) >=
			    sizeof(map_id)) {
				error_set("map_id too big");
				return (-1);
			}
			ox = (int)read_uint32(buf);
			oy = (int)read_uint32(buf);
			if (ox < 0 || ox > MAP_MAX_WIDTH || 
			    ox < 0 || oy > MAP_MAX_HEIGHT) {
				error_set("bad warp coords");
				return (-1);
			}
			dir = read_uint8(buf);
			debug(DEBUG_NODEREFS, "warp: to %s:%d,%d, dir 0x%x\n",
			    map_id, ox, oy, dir);

			if ((*nref = node_add_warp(node, map_id, ox, oy, dir))
			    == NULL) {
				return (-1);
			}
			(*nref)->flags = flags;
			(*nref)->layer = layer;
		}
		break;
	default:
		error_set("unknown noderef type");
		return (-1);
	}

	/* Read the transforms. */
	if ((ntrans = read_uint32(buf)) > NODEREF_MAX_TRANSFORMS) {
		error_set("too many transforms");
		goto fail;
	}
	for (i = 0; i < ntrans; i++) {
		struct transform *trans;

		trans = Malloc(sizeof(struct transform));
		transform_init(trans, 0, 0, NULL);
		if (transform_load(buf, trans) == -1) {
			free(trans);
			goto fail;
		}
		SLIST_INSERT_HEAD(&(*nref)->transforms, trans, transforms);
	}
	return (0);
fail:
	if (*nref != NULL) {
		noderef_destroy(*nref);
		*nref = NULL;
	}
	return (-1);
}

int
node_load(struct netbuf *buf, struct object_table *deps, struct node *node)
{
	Uint32 nrefs;
	struct noderef *nref;
	int i;
	
	/* Load the node properties. */
	node->flags = read_uint32(buf);
	read_uint32(buf);			/* Pad: v1 */
	
	/* Load the node references. */
	if ((nrefs = read_uint32(buf)) > NODE_MAX_NODEREFS) {
		error_set("too many nrefs");
		return (-1);
	}
	for (i = 0; i < nrefs; i++) {
		if (noderef_load(buf, deps, node, &nref) == -1) {
			node_destroy(node);
			node_init(node);
			return (-1);
		}
	}
	return (0);
}

static void
map_layer_load(struct netbuf *buf, struct map *m, struct map_layer *lay)
{
	lay->name = read_string(buf, NULL);
	lay->visible = (int)read_uint8(buf);
	lay->xinc = read_sint16(buf);
	lay->yinc = read_sint16(buf);
	lay->alpha = read_uint8(buf);
}

int
map_load(void *ob, struct netbuf *buf)
{
	struct map *m = ob;
	struct object_table deps;
	struct version ver;
	Uint32 w, h, origin_x, origin_y, tilew, tileh;
	int x, y;

	if (version_read(buf, &map_ver, &ver) != 0)
		return (-1);

	object_table_init(&deps);

	pthread_mutex_lock(&m->lock);

	if (m->map != NULL)
		map_free_nodes(m);
	if (ver.minor >= 2) {				/* Multilayering */
		if (m->layers != NULL)
			map_free_layers(m);
	}

	read_uint32(buf);				/* Always 0 */
	w = read_uint32(buf);
	h = read_uint32(buf);
	origin_x = read_uint32(buf);
	origin_y = read_uint32(buf);
	tilew = read_uint32(buf);
	tileh = read_uint32(buf);
	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT ||
	    origin_x > MAP_MAX_WIDTH || origin_y > MAP_MAX_HEIGHT ||
	    tilew > MAP_MAX_WIDTH || tileh > MAP_MAX_HEIGHT) {
		error_set("bad geo/coords");
		goto fail;
	}
	m->mapw = (unsigned int)w;
	m->maph = (unsigned int)h;
	m->tilew = (unsigned int)tilew;
	m->tileh = (unsigned int)tileh;
	m->origin.x = (int)origin_x;
	m->origin.y = (int)origin_y;

	m->zoom = read_uint16(buf);
	if (ver.minor >= 1) {				/* Soft-scroll offs */
		m->ssx = read_sint16(buf);
		m->ssy = read_sint16(buf);
	}
	if (ver.minor >= 2) {				/* Multilayering */
		int i;

		if ((m->nlayers = read_uint32(buf)) > MAP_MAX_LAYERS) {
			error_set("too many layers");
			goto fail;
		}
		debug(DEBUG_STATE, "%d layers\n", m->nlayers);
		m->layers = Malloc(m->nlayers * sizeof(struct map_layer));
		for (i = 0; i < m->nlayers; i++) {
			map_layer_load(buf, m, &m->layers[i]);
		}
	}
	if (ver.minor >= 3) 				/* Edited layer */
		m->cur_layer = (int)read_uint8(buf);
	if (ver.minor >= 4) 				/* Origin layer */
		m->origin.layer = (int)read_uint8(buf);

	debug(DEBUG_STATE,
	    "geo %ux%u, origin at [%d,%d,%d], %u%% zoom, %ux%u tiles\n",
	    m->mapw, m->maph, m->origin.x, m->origin.y, m->origin.layer,
	    m->zoom, m->tilew, m->tileh);

	/* Read the possible dependencies. */
	if (object_table_load(&deps, buf, OBJECT(m)->name) == -1)
		goto fail;

	/* Allocate and load the nodes. */
	if (map_alloc_nodes(m, m->mapw, m->maph) == -1)
		goto fail;

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			if (node_load(buf, &deps, &m->map[y][x]) == -1)
				goto fail;
		}
	}
	pthread_mutex_unlock(&m->lock);
	object_table_destroy(&deps);
	return (0);
fail:
	pthread_mutex_unlock(&m->lock);
	object_table_destroy(&deps);
	return (-1);
}

/*
 * Save a noderef structure, encoding the object ids using dependencies.
 * The noderef's parent map must be locked.
 */
void
noderef_save(struct netbuf *buf, struct object_table *deps,
    struct noderef *nref)
{
	off_t ntrans_offs;
	Uint32 i, ntrans = 0;
	struct transform *trans;

	/* Save the type of reference and flags. */
	write_uint32(buf, nref->type);
	write_uint32(buf, (Uint32)nref->flags);
	write_uint8(buf, nref->layer);

	debug(DEBUG_NODEREFS, "type %d, flags 0x%x, layer %d\n",
	    nref->type, nref->flags, nref->layer);

	/* Save the reference data. */
	switch (nref->type) {
	case NODEREF_SPRITE:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == nref->pobj)
				break;
		}
		write_uint32(buf, i);			/* Object# */
		write_uint32(buf, nref->offs);		/* Sprite# */
		write_sint16(buf, nref->xcenter);
		write_sint16(buf, nref->ycenter);
		debug(DEBUG_NODEREFS, "sprite: obj[%d]:%d, center %d,%d\n",
		    i, nref->offs, nref->xcenter, nref->ycenter);
		break;
	case NODEREF_ANIM:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == nref->pobj)
				break;
		}
		write_uint32(buf, i);			/* Object# */
		write_uint32(buf, nref->offs);		/* Anim# */
		write_sint16(buf, nref->xcenter);
		write_sint16(buf, nref->ycenter);
		write_uint32(buf, (Uint32)nref->data.anim.flags);
		debug(DEBUG_NODEREFS,
		    "anim: obj[%d]:%d, center %d,%d, aflags 0x%x\n",
		    i, nref->offs, nref->xcenter, nref->ycenter,
		    nref->data.anim.flags);
		break;
	case NODEREF_WARP:
		write_string(buf, nref->data.warp.map);
		write_uint32(buf, nref->data.warp.x);
		write_uint32(buf, nref->data.warp.y);
		write_uint8(buf, nref->data.warp.dir);
		debug(DEBUG_NODEREFS, "warp: to %s:[%d,%d], dir 0x%x",
		    nref->data.warp.map, nref->data.warp.x, nref->data.warp.y,
		    nref->data.warp.dir);
	default:
		debug(DEBUG_NODEREFS, "not saving %d node\n", nref->type);
		break;
	}

	/* Save the transforms. */
	ntrans_offs = buf->offs;
	write_uint32(buf, 0);					/* Skip count */
	SLIST_FOREACH(trans, &nref->transforms, transforms) {
		transform_save(buf, trans);
		ntrans++;
	}
	pwrite_uint32(buf, ntrans, ntrans_offs);		/* Save count */
}

void
node_save(struct netbuf *buf, struct object_table *deps, struct node *node)
{
	struct noderef *nref;
	off_t nrefs_offs;
	Uint32 nrefs = 0;

	/* Save the node properties. */
	write_uint32(buf, node->flags & ~(NODE_EPHEMERAL));
	write_uint32(buf, 0);				/* Pad: v1 */

	/* Save the node references. */
	nrefs_offs = buf->offs;
	write_uint32(buf, 0);				/* Skip */
	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
		MAP_CHECK_NODEREF(nref);
		if ((nref->flags & NODEREF_SAVEABLE) == 0)
			continue;
		noderef_save(buf, deps, nref);
		nrefs++;
	}
	pwrite_uint32(buf, nrefs, nrefs_offs);
}

static void
map_layer_save(struct netbuf *buf, struct map_layer *lay)
{
	write_string(buf, lay->name);
	write_uint8(buf, (Uint8)lay->visible);
	write_sint16(buf, lay->xinc);
	write_sint16(buf, lay->yinc);
	write_uint8(buf, lay->alpha);
}

int
map_save(void *p, struct netbuf *buf)
{
	struct map *m = p;
	struct object *pob;
	struct object_table deps;
	int x, y;
	
	version_write(buf, &map_ver);

	pthread_mutex_lock(&m->lock);
	debug(DEBUG_STATE,
	    "geo %ux%u, origin at [%d,%d,%d], %u%% zoom %ux%u tiles\n",
	    m->mapw, m->maph, m->origin.x, m->origin.y, m->origin.layer,
	    m->zoom, m->tilew, m->tileh);

	write_uint32(buf, 0);				/* Always 0 */
	write_uint32(buf, (Uint32)m->mapw);
	write_uint32(buf, (Uint32)m->maph);
	write_uint32(buf, (Uint32)m->origin.x);
	write_uint32(buf, (Uint32)m->origin.y);
	write_uint32(buf, (Uint32)m->tilew);
	write_uint32(buf, (Uint32)m->tileh);
	write_uint16(buf, m->zoom);
	if (map_ver.minor >= 1) {			/* Soft-scroll offs */
		write_sint16(buf, m->ssx);
		write_sint16(buf, m->ssy);
	}
	if (map_ver.minor >= 2) {			/* Multilayering */
		int i;

		write_uint32(buf, m->nlayers);
		for (i = 0; i < m->nlayers; i++)
			map_layer_save(buf, &m->layers[i]);
	}
	if (map_ver.minor >= 3) 			/* Edited layer */
		write_uint8(buf, m->cur_layer);
	if (map_ver.minor >= 4) 			/* Origin layer */
		write_uint8(buf, m->origin.layer);

	/* Write the dependencies. */
	object_table_init(&deps);
	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(pob, &world->wobjs, wobjs) {
		if ((pob->flags & OBJECT_ART) == 0 ||		/* XXX */
		     pob->flags & OBJECT_CANNOT_MAP) {
			continue;
		} 
		object_table_insert(&deps, pob);
	}
	pthread_mutex_unlock(&world->lock);
	object_table_save(&deps, buf);

	/* Write the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			MAP_CHECK_NODE(&m->map[y][x]);
			node_save(buf, &deps, &m->map[y][x]);
		}
	}
	pthread_mutex_unlock(&m->lock);

	object_table_destroy(&deps);
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
				MAP_CHECK_NODE(node);
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
noderef_draw_scaled(struct map *m, SDL_Surface *s, int rx, int ry)
{
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
				switch (s->format->BytesPerPixel) {
				case 4:
					SDL_GetRGBA(*(Uint32 *)src, s->format,
					    &r1, &g1, &b1, &a1);
					break;
				case 3:
				case 2:
					SDL_GetRGBA(*(Uint16 *)src, s->format,
					    &r1, &g1, &b1, &a1);
					break;
				case 1:
					SDL_GetRGBA(*src, s->format,
					    &r1, &g1, &b1, &a1);
					break;
				}
				view_alpha_blend(view->v, rx+x, ry+y,
				    r1, g1, b1, a1);
			} else {
				switch (s->format->BytesPerPixel) {
				case 4:
					SDL_GetRGB(*(Uint32 *)src, s->format,
					    &r1, &g1, &b1);
					break;
				case 3:
				case 2:
					SDL_GetRGB(*(Uint16 *)src, s->format,
					    &r1, &g1, &b1);
					break;
				case 1:
					SDL_GetRGB(*src, s->format,
					    &r1, &g1, &b1);
					break;
				}
				VIEW_PUT_PIXEL(view->v, rx+x, ry+y,
				    SDL_MapRGB(view->v->format, r1, g1, b1));
			}
		}
	}
	if (SDL_MUSTLOCK(view->v))
		SDL_UnlockSurface(view->v);
}

static __inline__ SDL_Surface *
noderef_draw_sprite(struct noderef *nref)
{
	struct art_cached_sprite *csprite;
	struct art_spritecl *spritecl;
	SDL_Surface *origsu = SPRITE(nref->pobj, nref->offs);

	if (SLIST_EMPTY(&nref->transforms)) {
		return (origsu);
	}

	spritecl = &nref->pobj->art->csprites[nref->offs];

	/* Look for a sprite with the same transforms, in the same order. */
	SLIST_FOREACH(csprite, &spritecl->sprites, sprites) {
		struct transform *tr1, *tr2;
				
		for (tr1 = SLIST_FIRST(&nref->transforms),
		     tr2 = SLIST_FIRST(&csprite->transforms);
		     tr1 != SLIST_END(&nref->transforms) &&
		     tr2 != SLIST_END(&csprite->transforms);
		     tr1 = SLIST_NEXT(tr1, transforms),
		     tr2 = SLIST_NEXT(tr2, transforms)) {
			if (transform_compare(tr1, tr2) != 0)
				break;
		}
		if (tr1 == SLIST_END(&nref->transforms) &&
		    tr2 == SLIST_END(&csprite->transforms))
			break;
	}
	if (csprite == NULL) {					/* Cache miss */
		struct transform *trans, *ntrans;
		SDL_Surface *su;
		Uint32 saflags = origsu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = origsu->format->alpha;
		Uint32 scflags = origsu->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 scolorkey = origsu->format->colorkey;
		struct art_cached_sprite *ncsprite;

		dprintf("cache miss\n");

		/* Allocate the new sprite surface. */
		su = SDL_CreateRGBSurface(SDL_SWSURFACE |
		    (origsu->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
		     origsu->w, origsu->h, origsu->format->BitsPerPixel,
		     origsu->format->Rmask,
		     origsu->format->Gmask,
		     origsu->format->Bmask,
		     origsu->format->Amask);
		if (su == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		
		ncsprite = Malloc(sizeof(struct art_cached_sprite));
		ncsprite->su = su;
		ncsprite->last_drawn = SDL_GetTicks();
		SLIST_INIT(&ncsprite->transforms);

		/* Copy the sprite as-is. */
		SDL_SetAlpha(origsu, 0, 0);
		SDL_SetColorKey(origsu, 0, 0);
		SDL_BlitSurface(origsu, NULL, su, NULL);
		SDL_SetColorKey(origsu, scflags, scolorkey);
		SDL_SetAlpha(origsu, saflags, salpha);

		/* Apply the transformations. */
		SLIST_FOREACH(trans, &nref->transforms, transforms) {
			SDL_LockSurface(su);
			trans->func(&su, trans->nargs, trans->args);
			SDL_UnlockSurface(su);

			ntrans = Malloc(sizeof(struct transform));
			transform_init(ntrans, trans->type, trans->nargs,
			    trans->args);
			SLIST_INSERT_HEAD(&ncsprite->transforms, ntrans,
			    transforms);
		}

		/* Cache the result. */
		SLIST_INSERT_HEAD(&spritecl->sprites, ncsprite, sprites);
		return (su);
	} else {						/* Cache hit */
		csprite->last_drawn = SDL_GetTicks();
		return (csprite->su);
	}
}

static __inline__ SDL_Surface *
noderef_draw_anim(struct noderef *nref)
{
	struct art_anim *anim = ANIM(nref->pobj, nref->offs);

	/* XXX do this somewhere else! */
	art_anim_tick(anim, nref);

	return (anim->frames[anim->frame]);
}

/*
 * Render a map node.
 * The node's parent map, if any, must be locked.
 *
 * XXX cache scaled nodes; doing this in real time is very expensive.
 * XXX opengl support and texture mgmt.
 */
__inline__ void
noderef_draw(struct map *m, struct noderef *nref, int rx, int ry)
{
	SDL_Rect rd;
	SDL_Surface *su;

	switch (nref->type) {
	case NODEREF_SPRITE:
		su = noderef_draw_sprite(nref);
		break;
	case NODEREF_ANIM:
		su = noderef_draw_anim(nref);
		break;
	default:				/* Not a drawable */
		return;
	}

	if (m->zoom != 100) {
		rd.x = rx +
		    (nref->xcenter * m->zoom / 100) +
		    (nref->xmotion * m->zoom / 100);
		rd.y = ry +
		    (nref->ycenter * m->zoom / 100) +
		    (nref->ymotion * m->zoom / 100);
		noderef_draw_scaled(m, su, rd.x, rd.y);
	} else {
		rd.x = rx + nref->xcenter + nref->xmotion;
		rd.y = ry + nref->ycenter + nref->ymotion;
		SDL_BlitSurface(su, NULL, view->v, &rd);
	}
}

