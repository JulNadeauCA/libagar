/*	$Csoft: map.c,v 1.182 2003/06/21 06:50:18 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/config.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <engine/mapedit/mapedit.h>

const struct version map_ver = {
	"agar map",
	4, 4
};

const struct object_ops map_ops = {
	map_init,
	map_destroy,
	map_load,
	map_save,
	map_edit
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_NODEREFS	0x02
#define DEBUG_RESIZE	0x04
#define DEBUG_SCAN	0x08

int	map_debug = DEBUG_STATE|DEBUG_RESIZE;
int	map_nodesigs = 1;
#define engine_debug map_debug
#endif /* DEBUG */

static void	 map_layer_init(struct map_layer *lay, const char *);

void
node_init(struct node *node)
{
	TAILQ_INIT(&node->nrefs);
}

void
node_destroy(struct map *m, struct node *node)
{
	struct noderef *r, *nr;

	for (r = TAILQ_FIRST(&node->nrefs);
	     r != TAILQ_END(&node->nrefs);
	     r = nr) {
		nr = TAILQ_NEXT(r, nrefs);
		noderef_destroy(m, r);
	}
}

void
noderef_init(struct noderef *r, enum noderef_type type)
{
	r->type = type;
	r->flags = 0;
	r->layer = 0;
	r->r_gfx.xcenter = 0;
	r->r_gfx.ycenter = 0;
	r->r_gfx.xmotion = 0;
	r->r_gfx.ymotion = 0;
	r->r_gfx.edge = 0;
	SLIST_INIT(&r->transforms);
}

/*
 * Adjust noderef centering offsets.
 * The parent map, if any, must be locked.
 */
void
noderef_set_center(struct noderef *r, int xcenter, int ycenter)
{
	r->r_gfx.xcenter = (Sint16)xcenter;
	r->r_gfx.ycenter = (Sint16)ycenter;
}

/*
 * Adjust noderef motion offsets.
 * The parent map, if any, must be locked.
 */
void
noderef_set_motion(struct noderef *r, int xmotion, int ymotion)
{
	r->r_gfx.xmotion = (Sint16)xmotion;
	r->r_gfx.ymotion = (Sint16)ymotion;
}

void
noderef_destroy(struct map *m, struct noderef *r)
{
	struct transform *trans, *ntrans;

	for (trans = SLIST_FIRST(&r->transforms);
	     trans != SLIST_END(&r->transforms);
	     trans = ntrans) {
		ntrans = SLIST_NEXT(trans, transforms);
		transform_destroy(trans);
	}

	switch (r->type) {
	case NODEREF_SPRITE:
		object_del_dep(m, r->r_sprite.obj);
		break;
	case NODEREF_ANIM:
		object_del_dep(m, r->r_anim.obj);
		break;
	case NODEREF_WARP:
		free(r->r_warp.map);
		break;
	default:
		break;
	}
	free(r);
}

/* Allocate and initialize the node arrays. */
int
map_alloc_nodes(struct map *m, unsigned int w, unsigned int h)
{
	int x, y;
	
	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set("%ux%u nodes > %ux%u", w, h, MAP_MAX_WIDTH,
		    MAP_MAX_HEIGHT);
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

/* Release the node arrays. */
void
map_free_nodes(struct map *m)
{
	int x, y;
	struct node *node;

	pthread_mutex_lock(&m->lock);
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node = &m->map[y][x];
			node_destroy(m, node);
		}
		free(m->map[y]);
	}
	free(m->map);
	m->map = NULL;
	pthread_mutex_unlock(&m->lock);
}

/* Release the layer array. */
static void
map_free_layers(struct map *m)
{
	m->nlayers = 0;
	free(m->layers);
	m->layers = NULL;
}

/* Resize a map, initializing new nodes and destroying excess ones. */
int
map_resize(struct map *m, unsigned int w, unsigned int h)
{
	struct map tm;
	int x, y;

	debug(DEBUG_RESIZE, "%ux%u -> %ux%u\n", m->mapw, m->maph, w, h);

	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set("size exceeds %ux%u", MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
		return (-1);
	}

	pthread_mutex_lock(&m->lock);

	/* Save the nodes to a temporary map, to preserve dependencies. */
	map_init(&tm, "t");
	if (map_alloc_nodes(&tm, m->mapw, m->maph) == -1) {
		goto fail;
	}
	for (y = 0;
	     y < m->maph && y < h;
	     y++) {
		for (x = 0;
		     x < m->mapw && x < w;
		     x++) {
			node_copy(m, &m->map[y][x], -1, &tm, &tm.map[y][x], -1);
		}
	}

	/* Resize the map, restore the original nodes. */
	map_free_nodes(m);
	if (map_alloc_nodes(m, w, h) == -1) {
		goto fail;
	}
	for (y = 0;
	     y < tm.maph && y < m->maph;
	     y++) {
		for (x = 0;
		     x < tm.mapw && x < m->mapw;
		     x++) {
			node_copy(&tm, &tm.map[y][x], -1, m, &m->map[y][x], -1);
		}
	}

	/* Make sure the origin remains inside the map. */
	if (m->origin.x >= w)
		m->origin.x = w-1;
	if (m->origin.y >= h)
		m->origin.y = h-1;

	pthread_mutex_unlock(&m->lock);
	object_destroy(&tm);
	return (0);
fail:
	pthread_mutex_unlock(&m->lock);
	object_destroy(&tm);
	return (-1);
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

struct map *
map_new(void *parent, const char *name)
{
	struct map *m;

	m = Malloc(sizeof(struct map));
	map_init(m, name);

	if (parent != NULL)
		object_attach(parent, m);

	return (m);
}

void
map_init(void *obj, const char *name)
{
	struct map *m = obj;

	object_init(m, "map", name, &map_ops);
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
	map_layer_init(&m->layers[0], _("Layer 0"));
	pthread_mutex_init(&m->lock, &recursive_mutexattr);
}

static void
map_layer_init(struct map_layer *lay, const char *name)
{
	strlcpy(lay->name, name, sizeof(lay->name));
	lay->visible = 1;
	lay->xinc = 1;
	lay->yinc = 1;
	lay->alpha = SDL_ALPHA_OPAQUE;
}

/* Create a new layer. */
int
map_push_layer(struct map *m, const char *name)
{
	char layname[MAP_LAYER_NAME_MAX];

	if (name != NULL) {
		strlcpy(layname, name, sizeof(layname));
	} else {
		snprintf(layname, sizeof(layname), _("Layer %u"), m->nlayers);
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
 * The map must be locked.
 */
struct noderef *
node_add_sprite(struct map *map, struct node *node, void *pobj, Uint32 offs)
{
	struct noderef *r;

	r = Malloc(sizeof(struct noderef));
	noderef_init(r, NODEREF_SPRITE);
	r->r_sprite.obj = pobj;
	r->r_sprite.offs = offs;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	object_add_dep(map, pobj);
	return (r);
}

/*
 * Insert a reference to an animation at pobj:offs.
 * The map must be locked.
 */
struct noderef *
node_add_anim(struct map *map, struct node *node, void *pobj, Uint32 offs,
    Uint8 flags)
{
	struct noderef *r;

	r = Malloc(sizeof(struct noderef));
	noderef_init(r, NODEREF_ANIM);
	r->r_anim.obj = pobj;
	r->r_anim.offs = offs;
	r->r_anim.flags = flags;
	r->r_anim.frame = 0;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	object_add_dep(map, pobj);
	return (r);
}

/*
 * Insert a warp point.
 * The map must be locked.
 */
struct noderef *
node_add_warp(struct map *map, struct node *node, const char *mapname,
    int x, int y, Uint8 dir)
{
	struct noderef *r;

	r = Malloc(sizeof(struct noderef));
	noderef_init(r, NODEREF_WARP);
	r->r_warp.map = Strdup(mapname);
	r->r_warp.x = x;
	r->r_warp.y = y;
	r->r_warp.dir = dir;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

/*
 * Move a reference to dm:dn and associate with dlayer.
 * Both source and destination maps must be locked.
 */
void
node_move_ref(struct map *sm, struct node *sn, struct noderef *r,
    struct map *dm, struct node *dn, int dlayer)
{
	pthread_mutex_lock(&sm->lock);
	pthread_mutex_lock(&dm->lock);

	TAILQ_REMOVE(&sn->nrefs, r, nrefs);
	TAILQ_INSERT_TAIL(&dn->nrefs, r, nrefs);

	if (dlayer != -1)
		r->layer = dlayer;

	switch (r->type) {
	case NODEREF_SPRITE:
		object_del_dep(sm, r->r_sprite.obj);
		object_add_dep(dm, r->r_sprite.obj);
		break;
	case NODEREF_ANIM:
		object_del_dep(sm, r->r_anim.obj);
		object_add_dep(dm, r->r_anim.obj);
		break;
	default:
		break;
	}
	
	pthread_mutex_unlock(&dm->lock);
	pthread_mutex_unlock(&sm->lock);
}

/*
 * Copy references from source node sn which are associated with slayer (or
 * all references if slayer is -1) to destination node dn, and associate
 * the copy with dlayer (or the original layer, if dlayer is -1).
 */
void
node_copy(struct map *sm, struct node *sn, int slayer, struct map *dm,
    struct node *dn, int dlayer)
{
	struct noderef *sr;

	pthread_mutex_lock(&sm->lock);
	pthread_mutex_lock(&dm->lock);

	TAILQ_FOREACH(sr, &sn->nrefs, nrefs) {
		if (slayer != -1 &&
		    sr->layer != slayer) {
			continue;
		}
		node_copy_ref(sr, dm, dn, dlayer);
	}
	
	pthread_mutex_unlock(&dm->lock);
	pthread_mutex_unlock(&sm->lock);
}

/*
 * Copy a node reference from one node to another.
 * Both the source and destination maps must be locked.
 */
struct noderef *
node_copy_ref(struct noderef *sr, struct map *dm, struct node *dn, int dlayer)
{
	struct transform *trans;
	struct noderef *dr = NULL;

	/* Allocate a new noderef with the same data. */
	switch (sr->type) {
	case NODEREF_SPRITE:
		dr = node_add_sprite(dm, dn, sr->r_sprite.obj,
		    sr->r_sprite.offs);
		dr->r_gfx.xcenter = sr->r_gfx.xcenter;
		dr->r_gfx.ycenter = sr->r_gfx.ycenter;
		dr->r_gfx.xmotion = sr->r_gfx.xmotion;
		dr->r_gfx.ymotion = sr->r_gfx.ymotion;
		break;
	case NODEREF_ANIM:
		dr = node_add_anim(dm, dn, sr->r_anim.obj, sr->r_anim.offs,
		    sr->r_anim.flags);
		dr->r_gfx.xcenter = sr->r_gfx.xcenter;
		dr->r_gfx.ycenter = sr->r_gfx.ycenter;
		dr->r_gfx.xmotion = sr->r_gfx.xmotion;
		dr->r_gfx.ymotion = sr->r_gfx.ymotion;
		break;
	case NODEREF_WARP:
		dr = node_add_warp(dm, dn, sr->r_warp.map, sr->r_warp.x,
		    sr->r_warp.y, sr->r_warp.dir);
		break;
	}
	dr->flags = sr->flags;
	dr->layer = (dlayer == -1) ? sr->layer : dlayer;

	/* Inherit the transformations. */
	SLIST_FOREACH(trans, &sr->transforms, transforms) {
		struct transform *ntrans;

		ntrans = Malloc(sizeof(struct transform));
		transform_init(ntrans, trans->type, trans->nargs, trans->args);
		SLIST_INSERT_HEAD(&dr->transforms, ntrans, transforms);
	}
	return (dr);
}

/* Remove a noderef from a node and free it. */
void
node_remove_ref(struct map *m, struct node *node, struct noderef *r)
{
	pthread_mutex_lock(&m->lock);

	TAILQ_REMOVE(&node->nrefs, r, nrefs);
	noderef_destroy(m, r);

	pthread_mutex_unlock(&m->lock);
}

/* Remove all references associated with the given layer. */
void
node_clear(struct map *m, struct node *node, int layer)
{
	struct noderef *r, *nr;

	pthread_mutex_lock(&m->lock);

	for (r = TAILQ_FIRST(&node->nrefs);
	     r != TAILQ_END(&node->nrefs);
	     r = nr) {
		nr = TAILQ_NEXT(r, nrefs);
		if (r->layer != layer) {
			continue;
		}
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		noderef_destroy(m, r);
	}

	pthread_mutex_unlock(&m->lock);
}

/*
 * Move a noderef to the upper layer.
 * The map containing the node must be locked.
 */
void
node_moveup_ref(struct node *node, struct noderef *r)
{
	struct noderef *next = TAILQ_NEXT(r, nrefs);

	if (next != NULL) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_AFTER(&node->nrefs, next, r, nrefs);
	}
}

/*
 * Move a noderef to the lower layer.
 * The map containing the node must be locked.
 */
void
node_movedown_ref(struct node *node, struct noderef *r)
{
	struct noderef *prev = TAILQ_PREV(r, noderefq, nrefs);

	if (prev != NULL) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_BEFORE(prev, r, nrefs);
	}
}

/*
 * Move a noderef to the tail of the queue.
 * The map containing the node must be locked.
 */
void
node_movetail_ref(struct node *node, struct noderef *r)
{
	if (r != TAILQ_LAST(&node->nrefs, noderefq)) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	}
}

/*
 * Move a noderef to the head of the queue.
 * The map containing the node must be locked.
 */
void
node_movehead_ref(struct node *node, struct noderef *r)
{
	if (r != TAILQ_FIRST(&node->nrefs)) {
		TAILQ_REMOVE(&node->nrefs, r, nrefs);
		TAILQ_INSERT_HEAD(&node->nrefs, r, nrefs);
	}
}

void
map_destroy(void *p)
{
	struct map *m = p;

	if (m->map != NULL)
		map_free_nodes(m);
	if (m->layers != NULL)
		map_free_layers(m);

	pthread_mutex_destroy(&m->lock);
}

void
map_edit(void *p)
{
	struct map *m = p;
	struct window *win;

	win = mapedit_window(m);
	window_show(win);
}

/*
 * Load a node reference.
 * The map must be locked.
 */
int
noderef_load(struct map *m, struct netbuf *buf, struct object_table *deps,
    struct node *node, struct noderef **r)
{
	enum noderef_type type;
	Uint32 ntrans = 0;
	Uint8 flags;
	Uint8 layer;
	int i;

	/* Read the type of reference, flags and the layer#. */
	type = (enum noderef_type)read_uint32(buf);
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
				*r = node_add_sprite(m, node, deps->objs[obji],
				    offs);
				(*r)->flags = flags;
				(*r)->layer = layer;
				(*r)->r_gfx.xcenter = xcenter;
				(*r)->r_gfx.ycenter = ycenter;
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

				*r = node_add_anim(m, node, deps->objs[obji],
				    offs, animflags);
				(*r)->flags = flags;
				(*r)->layer = layer;
				(*r)->r_gfx.xcenter = xcenter;
				(*r)->r_gfx.ycenter = ycenter;
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

			*r = node_add_warp(m, node, map_id, ox, oy, dir);
			(*r)->flags = flags;
			(*r)->layer = layer;
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
		SLIST_INSERT_HEAD(&(*r)->transforms, trans, transforms);
	}
	return (0);
fail:
	if (*r != NULL) {
		noderef_destroy(m, *r);
		*r = NULL;
	}
	return (-1);
}

int
node_load(struct map *m, struct netbuf *buf, struct object_table *deps,
    struct node *node)
{
	Uint32 nrefs;
	struct noderef *r;
	int i;
	
	if ((nrefs = read_uint32(buf)) > NODE_MAX_NODEREFS) {
		error_set("too many nrefs");
		return (-1);
	}
	for (i = 0; i < nrefs; i++) {
		if (noderef_load(m, buf, deps, node, &r) == -1) {
			node_destroy(m, node);
			node_init(node);
			return (-1);
		}
	}
	return (0);
}

static void
map_layer_load(struct netbuf *buf, struct map *m, struct map_layer *lay)
{
	copy_string(lay->name, buf, sizeof(lay->name));
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
	int i, x, y;

	if (version_read(buf, &map_ver, &ver) != 0)
		return (-1);

	object_table_init(&deps);

	pthread_mutex_lock(&m->lock);

	if (m->map != NULL)
		map_free_nodes(m);
	if (m->layers != NULL)
		map_free_layers(m);

	/* Read the map header. */
	read_uint32(buf);				/* Padding */
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
	m->ssx = read_sint16(buf);
	m->ssy = read_sint16(buf);

	/* Read the layer information. */
	if ((m->nlayers = read_uint32(buf)) > MAP_MAX_LAYERS) {
		error_set("too many layers");
		goto fail;
	}
	debug(DEBUG_STATE, "%d layers\n", m->nlayers);
	m->layers = Malloc(m->nlayers * sizeof(struct map_layer));
	for (i = 0; i < m->nlayers; i++) {
		map_layer_load(buf, m, &m->layers[i]);
	}
	m->cur_layer = (int)read_uint8(buf);
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
			if (node_load(m, buf, &deps, &m->map[y][x]) == -1)
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
noderef_save(struct map *m, struct netbuf *buf, struct object_table *deps,
    struct noderef *r)
{
	off_t ntrans_offs;
	Uint32 i, ntrans = 0;
	struct transform *trans;

	/* Save the type of reference and flags. */
	write_uint32(buf, (Uint32)r->type);
	write_uint32(buf, (Uint32)r->flags);
	write_uint8(buf, r->layer);

	debug(DEBUG_NODEREFS, "type %d, flags 0x%x, layer %u\n", r->type,
	    r->flags, r->layer);

	/* Save the reference. */
	switch (r->type) {
	case NODEREF_SPRITE:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == r->r_sprite.obj)
				break;
		}
		write_uint32(buf, i);
		write_uint32(buf, r->r_sprite.offs);
		write_sint16(buf, r->r_gfx.xcenter);
		write_sint16(buf, r->r_gfx.ycenter);
		debug(DEBUG_NODEREFS, "sprite: obj[%d]:%d, center %d,%d\n", i,
		    r->r_sprite.offs, r->r_gfx.xcenter, r->r_gfx.ycenter);
		break;
	case NODEREF_ANIM:
		for (i = 0; i < deps->nobjs; i++) {
			if (deps->objs[i] == r->r_anim.obj)
				break;
		}
		write_uint32(buf, i);
		write_uint32(buf, r->r_anim.offs);
		write_sint16(buf, r->r_gfx.xcenter);
		write_sint16(buf, r->r_gfx.ycenter);
		write_uint32(buf, r->r_anim.flags);
		debug(DEBUG_NODEREFS, "anim: o[%d]:%d, c[%d,%d], flags 0x%x\n",
		    i, r->r_anim.offs, r->r_gfx.xcenter, r->r_gfx.ycenter,
		    r->r_anim.flags);
		break;
	case NODEREF_WARP:
		write_string(buf, r->r_warp.map);
		write_uint32(buf, (Uint32)r->r_warp.x);
		write_uint32(buf, (Uint32)r->r_warp.y);
		write_uint8(buf, r->r_warp.dir);
		debug(DEBUG_NODEREFS, "warp: to %s:[%d,%d], dir 0x%x",
		    r->r_warp.map, r->r_warp.x, r->r_warp.y, r->r_warp.dir);
	default:
		debug(DEBUG_NODEREFS, "not saving %d node\n", r->type);
		break;
	}

	/* Save the transforms. */
	ntrans_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	SLIST_FOREACH(trans, &r->transforms, transforms) {
		transform_save(buf, trans);
		ntrans++;
	}
	pwrite_uint32(buf, ntrans, ntrans_offs);
}

void
node_save(struct map *m, struct netbuf *buf, struct object_table *deps,
    struct node *node)
{
	struct noderef *r;
	off_t nrefs_offs;
	Uint32 nrefs = 0;

	nrefs_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		noderef_save(m, buf, deps, r);
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
	struct object_table deps;
	int i, x, y;
	
	version_write(buf, &map_ver);

	pthread_mutex_lock(&m->lock);
	debug(DEBUG_STATE,
	    "geo %ux%u, origin at [%d,%d,%d], %u%% zoom %ux%u tiles\n",
	    m->mapw, m->maph, m->origin.x, m->origin.y, m->origin.layer,
	    m->zoom, m->tilew, m->tileh);

	/* Write the map header. */
	write_uint32(buf, 0);				/* Padding */
	write_uint32(buf, (Uint32)m->mapw);
	write_uint32(buf, (Uint32)m->maph);
	write_uint32(buf, (Uint32)m->origin.x);
	write_uint32(buf, (Uint32)m->origin.y);
	write_uint32(buf, (Uint32)m->tilew);
	write_uint32(buf, (Uint32)m->tileh);
	write_uint16(buf, m->zoom);
	write_sint16(buf, m->ssx);
	write_sint16(buf, m->ssy);

	/* Write the layer information. */
	write_uint32(buf, m->nlayers);
	for (i = 0; i < m->nlayers; i++) {
		map_layer_save(buf, &m->layers[i]);
	}
	write_uint8(buf, m->cur_layer);
	write_uint8(buf, m->origin.layer);

	/* Write the dependencies. */
	lock_linkage();
	object_table_init(&deps);
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *r;

			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				switch (r->type) {
				case NODEREF_SPRITE:
					object_table_insert(&deps,
					    r->r_sprite.obj);
					break;
				case NODEREF_ANIM:
					object_table_insert(&deps,
					    r->r_anim.obj);
					break;
				default:
					break;
				}
			}
		}
	}
	unlock_linkage();
	object_table_save(&deps, buf);

	/* Write the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_save(m, buf, &deps, &m->map[y][x]);
		}
	}
	pthread_mutex_unlock(&m->lock);

	object_table_destroy(&deps);
	return (0);
}

/* Render surface s, scaled to rx,ry pixels. */
/* XXX use something more sophisticated (or cache); fix rleaccel */
static void
noderef_draw_scaled(struct map *m, SDL_Surface *s, int rx, int ry)
{
	int x, y, dh, dw, sx, sy;
	Uint8 *src, r1, g1, b1, a1;

	dw = s->w * m->zoom / 100;
	dh = s->h * m->zoom / 100;

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
				    SDL_MapRGB(vfmt, r1, g1, b1));
			}
		}
	}
	if (SDL_MUSTLOCK(view->v))
		SDL_UnlockSurface(view->v);
}

/* Render a sprite, possibly applying transforms. */
static __inline__ SDL_Surface *
noderef_draw_sprite(struct noderef *r)
{
	struct gfx_cached_sprite *csprite;
	struct gfx_spritecl *spritecl;
	SDL_Surface *origsu = SPRITE(r->r_sprite.obj, r->r_sprite.offs);

	if (SLIST_EMPTY(&r->transforms)) {
		return (origsu);
	}

	/*
	 * Look for a cached sprite with the same transforms, in the same
	 * order.
	 */
	spritecl = &r->r_sprite.obj->gfx->csprites[r->r_sprite.offs];
	SLIST_FOREACH(csprite, &spritecl->sprites, sprites) {
		struct transform *tr1, *tr2;
				
		for (tr1 = SLIST_FIRST(&r->transforms),
		     tr2 = SLIST_FIRST(&csprite->transforms);
		     tr1 != SLIST_END(&r->transforms) &&
		     tr2 != SLIST_END(&csprite->transforms);
		     tr1 = SLIST_NEXT(tr1, transforms),
		     tr2 = SLIST_NEXT(tr2, transforms)) {
			if (transform_compare(tr1, tr2) != 0)
				break;
		}
		if (tr1 == SLIST_END(&r->transforms) &&
		    tr2 == SLIST_END(&csprite->transforms))
			break;
	}
	if (csprite == NULL) {
		struct transform *trans, *ntrans;
		SDL_Surface *su;
		Uint32 saflags = origsu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = origsu->format->alpha;
		Uint32 scflags = origsu->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 scolorkey = origsu->format->colorkey;
		struct gfx_cached_sprite *ncsprite;

		dprintf("transform cache miss\n");

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
		
		ncsprite = Malloc(sizeof(struct gfx_cached_sprite));
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
		SLIST_FOREACH(trans, &r->transforms, transforms) {
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
	} else {
		/* Update the timestamp and return the cached version. */
		csprite->last_drawn = SDL_GetTicks();
		return (csprite->su);
	}
}

/* Render an animation, possibly applying transforms. */
static __inline__ SDL_Surface *
noderef_draw_anim(struct noderef *r)
{
	struct gfx_anim *anim = ANIM(r->r_anim.obj, r->r_anim.offs);

	/* XXX do this somewhere else! */
	gfx_anim_tick(anim, r);

	/* XXX transforms */
	return (anim->frames[anim->frame]);
}

/*
 * Render a graphical noderef to absolute view coordinates rx,ry.
 * The map must be locked.
 */
__inline__ void
noderef_draw(struct map *m, struct noderef *r, int rx, int ry)
{
	SDL_Surface *su;

	switch (r->type) {
	case NODEREF_SPRITE:
		su = noderef_draw_sprite(r);
		break;
	case NODEREF_ANIM:
		su = noderef_draw_anim(r);
		break;
	default:				/* Not a drawable */
		return;
	}

	if (m->zoom != 100) {
		noderef_draw_scaled(m, su,
		    (rx + r->r_gfx.xcenter*m->zoom/100 +
		     r->r_gfx.xmotion*m->zoom/100),
		    (ry + r->r_gfx.ycenter*m->zoom/100 +
		     r->r_gfx.ymotion*m->zoom/100));
	} else {
		SDL_Rect rd;

		rd.x = rx + r->r_gfx.xcenter + r->r_gfx.xmotion;
		rd.y = ry + r->r_gfx.ycenter + r->r_gfx.ymotion;
		SDL_BlitSurface(su, NULL, view->v, &rd);
	}
}

