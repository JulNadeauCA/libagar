/*	$Csoft: map.c,v 1.225 2004/05/06 06:22:31 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#ifdef EDITION
#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/button.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>
#include <engine/mapedit/layedit.h>
#endif

#include <string.h>

const struct version map_ver = {
	"agar map",
	6, 1
};

const struct object_ops map_ops = {
	map_init,
	map_reinit,
	map_destroy,
	map_load,
	map_save,
#ifdef EDITION
	map_edit
#endif
};

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
	r->friction = 0;
	r->r_gfx.xcenter = 0;
	r->r_gfx.ycenter = 0;
	r->r_gfx.xmotion = 0;
	r->r_gfx.ymotion = 0;
	r->r_gfx.edge = 0;
	TAILQ_INIT(&r->transforms);
	TAILQ_INIT(&r->masks);
}

/*
 * Adjust the centering offset of a given node reference.
 * The parent map, if any, must be locked.
 */
void
noderef_set_center(struct noderef *r, int xcenter, int ycenter)
{
	r->r_gfx.xcenter = (Sint16)xcenter;
	r->r_gfx.ycenter = (Sint16)ycenter;
}

/*
 * Adjust the motion offset of a given node reference.
 * The parent map, if any, must be locked.
 */
void
noderef_set_motion(struct noderef *r, int xmotion, int ymotion)
{
	r->r_gfx.xmotion = (Sint16)xmotion;
	r->r_gfx.ymotion = (Sint16)ymotion;
}

/*
 * Define the coefficient of friction/acceleration for a given node reference.
 * The parent map, if any, must be locked.
 */
void
noderef_set_friction(struct noderef *r, int coeff)
{
	r->friction = (Sint8)coeff;
}

void
noderef_destroy(struct map *m, struct noderef *r)
{
	struct transform *trans, *ntrans;
	struct nodemask *mask, *nmask;

	for (trans = TAILQ_FIRST(&r->transforms);
	     trans != TAILQ_END(&r->transforms);
	     trans = ntrans) {
		ntrans = TAILQ_NEXT(trans, transforms);
		transform_destroy(trans);
	}
	for (mask = TAILQ_FIRST(&r->masks);
	     mask != TAILQ_END(&r->masks);
	     mask = nmask) {
		nmask = TAILQ_NEXT(mask, masks);
		nodemask_destroy(m, mask);
	}

	switch (r->type) {
	case NODEREF_SPRITE:
		if (r->r_sprite.obj != NULL) {
			object_del_dep(m, r->r_sprite.obj);
			object_page_out(r->r_sprite.obj, OBJECT_GFX);
		}
		break;
	case NODEREF_ANIM:
		if (r->r_anim.obj != NULL) {
			object_del_dep(m, r->r_anim.obj);
			object_page_out(r->r_anim.obj, OBJECT_GFX);
		}
		break;
	case NODEREF_WARP:
		Free(r->r_warp.map, 0);
		break;
	default:
		break;
	}
	Free(r, M_MAP_NODEREF);
}

/* Allocate and initialize the node arrays. */
int
map_alloc_nodes(struct map *m, unsigned int w, unsigned int h)
{
	int x, y;
	
	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set(_("%ux%u nodes exceed %ux%u."), w, h, MAP_MAX_WIDTH,
		    MAP_MAX_HEIGHT);
		return (-1);
	}

	pthread_mutex_lock(&m->lock);
	m->mapw = w;
	m->maph = h;
	m->map = Malloc(h * sizeof(struct node *), M_MAP);
	for (y = 0; y < h; y++) {
		m->map[y] = Malloc(w * sizeof(struct node), M_MAP);
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
	if (m->map != NULL) {
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				node = &m->map[y][x];
				node_destroy(m, node);
			}
			Free(m->map[y], M_MAP);
		}
		Free(m->map, M_MAP);
		m->map = NULL;
	}
	pthread_mutex_unlock(&m->lock);
}

/* Reinitialize the layer array to the default. */
static void
map_free_layers(struct map *m)
{
	m->layers = Realloc(m->layers, 1 * sizeof(struct map_layer), M_MAP);
	m->nlayers = 1;
	map_init_layer(&m->layers[0], _("Layer 0"));
}

/* Resize a map, initializing new nodes and destroying excess ones. */
int
map_resize(struct map *m, unsigned int w, unsigned int h)
{
	struct map tm;
	int x, y;

	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT) {
		error_set(_("%ux%u nodes exceed %ux%u."), w, h, MAP_MAX_WIDTH,
		    MAP_MAX_HEIGHT);
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

/* Set the zoom factor. */
void
map_set_zoom(struct map *m, Uint16 zoom)
{
	pthread_mutex_lock(&m->lock);
	m->zoom = zoom;
	if ((m->tilesz = m->zoom*TILESZ/100) > MAP_MAX_TILESZ) {
		m->tilesz = MAP_MAX_TILESZ;
	}
	pthread_mutex_unlock(&m->lock);
}

struct map *
map_new(void *parent, const char *name)
{
	struct map *m;

	m = Malloc(sizeof(struct map), M_OBJECT);
	map_init(m, name);

	if (parent != NULL)
		object_attach(parent, m);

	return (m);
}

void
map_init_layer(struct map_layer *lay, const char *name)
{
	strlcpy(lay->name, name, sizeof(lay->name));
	lay->visible = 1;
	lay->xinc = 1;
	lay->yinc = 1;
	lay->alpha = SDL_ALPHA_OPAQUE;
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
	m->tilesz = TILESZ;
	m->zoom = 100;
	m->ssx = TILESZ;
	m->ssy = TILESZ;
	m->cur_layer = 0;

	m->layers = Malloc(1 * sizeof(struct map_layer), M_MAP);
	m->nlayers = 1;
	map_init_layer(&m->layers[0], _("Layer 0"));
	pthread_mutex_init(&m->lock, &recursive_mutexattr);
}

/* Create a new layer. */
int
map_push_layer(struct map *m, const char *name)
{
	char layname[MAP_LAYER_NAME_MAX];

	if (name[0] == '\0') {
		snprintf(layname, sizeof(layname), _("Layer %u"), m->nlayers);
	} else {
		strlcpy(layname, name, sizeof(layname));
	}

	if (m->nlayers+1 > MAP_MAX_LAYERS) {
		error_set(_("Too many layers."));
		return (-1);
	}
	m->layers = Realloc(m->layers,
	    (m->nlayers+1) * sizeof(struct map_layer), M_MAP);
	map_init_layer(&m->layers[m->nlayers], layname);
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

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_SPRITE);
	r->r_sprite.obj = pobj;
	r->r_sprite.offs = offs;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);

	if (pobj != NULL) {
		object_add_dep(map, pobj);
		if (object_page_in(pobj, OBJECT_GFX) == -1)
			fatal("paging gfx: %s", error_get());
	}
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
	
	if (pobj != NULL) {
		object_add_dep(map, pobj);
		if (object_page_in(pobj, OBJECT_GFX) == -1)
			fatal("paging gfx: %s", error_get());
	}

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_ANIM);
	r->r_anim.obj = pobj;
	r->r_anim.offs = offs;
	r->r_anim.flags = flags;
	r->r_anim.frame = 0;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
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

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_WARP);
	r->r_warp.map = Strdup(mapname);
	r->r_warp.x = x;
	r->r_warp.y = y;
	r->r_warp.dir = dir;
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

/*
 * Move a reference to a specified node and optionally assign to a
 * specified layer.
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
node_copy(struct map *sm, struct node *sn, int slayer,
    struct map *dm, struct node *dn, int dlayer)
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
node_copy_ref(const struct noderef *sr, struct map *dm, struct node *dn,
    int dlayer)
{
	struct transform *trans;
	struct nodemask *mask;
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
	dr->friction = sr->friction;

	/* Inherit the transformations. */
	TAILQ_FOREACH(trans, &sr->transforms, transforms) {
		struct transform *ntrans;

		ntrans = Malloc(sizeof(struct transform), M_NODEXFORM);
		transform_init(ntrans, trans->type, trans->nargs, trans->args);
		TAILQ_INSERT_TAIL(&dr->transforms, ntrans, transforms);
	}
	
	/* Inherit the node masks. */
	TAILQ_FOREACH(mask, &sr->masks, masks) {
		struct nodemask *nmask;

		nmask = Malloc(sizeof(struct nodemask), M_NODEMASK);
		nodemask_init(nmask, mask->type);
		nodemask_copy(mask, dm, nmask);
		TAILQ_INSERT_TAIL(&dr->masks, nmask, masks);
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
map_reinit(void *p)
{
	struct map *m = p;
	
	if (m->map != NULL)
		map_free_nodes(m);
	if (m->layers != NULL)
		map_free_layers(m);
}

void
map_destroy(void *p)
{
	struct map *m = p;

#ifdef THREADS
	pthread_mutex_destroy(&m->lock);
#endif
	Free(m->layers, M_MAP);
}

/*
 * Load a node reference.
 * The map must be locked.
 */
int
noderef_load(struct map *m, struct netbuf *buf, struct node *node,
    struct noderef **r)
{
	enum noderef_type type;
	Uint32 ntrans = 0, nmasks = 0;
	Uint8 flags;
	Uint8 layer;
	Sint8 friction;
	int i;

	/* Read the type of reference, flags and the layer#. */
	type = (enum noderef_type)read_uint32(buf);
	flags = (Uint8)read_uint32(buf);
	layer = read_uint8(buf);
	friction = read_sint8(buf);

	/* Read the reference data. */
	switch (type) {
	case NODEREF_SPRITE:
		{
			Uint32 ref, offs;
			Sint16 xcenter, ycenter;
			struct object *pobj;

			ref = read_uint32(buf);
			offs = read_uint32(buf);
			xcenter = read_sint16(buf);
			ycenter = read_sint16(buf);

			if ((pobj = object_find_dep(m, ref)) == NULL) {
				error_set(_("Cannot resolve object: %u."), ref);
				return (-1);
			}
			*r = node_add_sprite(m, node, pobj, offs);
			(*r)->flags = flags;
			(*r)->layer = layer;
			(*r)->friction = friction;
			(*r)->r_gfx.xcenter = xcenter;
			(*r)->r_gfx.ycenter = ycenter;
		}
		break;
	case NODEREF_ANIM:
		{
			Uint32 ref, offs, aflags;
			Sint16 xcenter, ycenter;
			struct object *pobj;

			ref = read_uint32(buf);
			offs = read_uint32(buf);
			xcenter = read_sint16(buf);
			ycenter = read_sint16(buf);
			aflags = read_uint32(buf);
			
			if ((pobj = object_find_dep(m, ref)) == NULL) {
				error_set(_("Cannot resolve object: %u."), ref);
				return (-1);
			}
			*r = node_add_anim(m, node, pobj, offs, aflags);
			(*r)->flags = flags;
			(*r)->layer = layer;
			(*r)->friction = friction;
			(*r)->r_gfx.xcenter = xcenter;
			(*r)->r_gfx.ycenter = ycenter;
		}
		break;
	case NODEREF_WARP:
		{
			char map_id[OBJECT_NAME_MAX];
			Uint32 ox, oy;
			Uint8 dir;

			if (copy_string(map_id, buf, sizeof(map_id)) >=
			    sizeof(map_id)) {
				error_set(_("Warp map name is too big."));
				return (-1);
			}
			ox = (int)read_uint32(buf);
			oy = (int)read_uint32(buf);
			if (ox < 0 || ox > MAP_MAX_WIDTH || 
			    ox < 0 || oy > MAP_MAX_HEIGHT) {
				error_set(_("Invalid warp coordinates."));
				return (-1);
			}
			dir = read_uint8(buf);
			*r = node_add_warp(m, node, map_id, ox, oy, dir);
			(*r)->flags = flags;
			(*r)->layer = layer;
			(*r)->friction = friction;
		}
		break;
	default:
		error_set(_("Unknown type of noderef."));
		return (-1);
	}

	/* Read the transforms. */
	if ((ntrans = read_uint32(buf)) > NODEREF_MAX_TRANSFORMS) {
		error_set(_("Too many transforms."));
		goto fail;
	}
	for (i = 0; i < ntrans; i++) {
		struct transform *trans;

		trans = Malloc(sizeof(struct transform), M_NODEXFORM);
		transform_init(trans, 0, 0, NULL);
		if (transform_load(buf, trans) == -1) {
			Free(trans, M_NODEXFORM);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&(*r)->transforms, trans, transforms);
	}
	
	/* Read the node masks. */
	if ((nmasks = read_uint32(buf)) > NODEREF_MAX_MASKS) {
		error_set(_("Too many node masks."));
		goto fail;
	}
	for (i = 0; i < nmasks; i++) {
		struct nodemask *mask;

		mask = Malloc(sizeof(struct nodemask), M_NODEMASK);
		nodemask_init(mask, 0);
		if (nodemask_load(m, buf, mask) == -1) {
			Free(mask, M_NODEMASK);
			goto fail;
		}
		TAILQ_INSERT_TAIL(&(*r)->masks, mask, masks);
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
node_load(struct map *m, struct netbuf *buf, struct node *node)
{
	Uint32 nrefs;
	struct noderef *r;
	int i;
	
	if ((nrefs = read_uint32(buf)) > NODE_MAX_NODEREFS) {
		error_set(_("Too many noderefs."));
		return (-1);
	}
	for (i = 0; i < nrefs; i++) {
		if (noderef_load(m, buf, node, &r) == -1) {
			node_destroy(m, node);
			node_init(node);
			return (-1);
		}
	}
	return (0);
}

int
map_load(void *ob, struct netbuf *buf)
{
	struct map *m = ob;
	Uint32 w, h, origin_x, origin_y, tilesz;
	int i, x, y;

	if (version_read(buf, &map_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&m->lock);
	w = read_uint32(buf);
	h = read_uint32(buf);
	origin_x = read_uint32(buf);
	origin_y = read_uint32(buf);
	tilesz = read_uint32(buf);
	read_uint32(buf);
	if (w > MAP_MAX_WIDTH || h > MAP_MAX_HEIGHT ||
	    tilesz > MAP_MAX_TILESZ ||
	    origin_x > MAP_MAX_WIDTH || origin_y > MAP_MAX_HEIGHT) {
		error_set(_("Invalid map geometry."));
		goto fail;
	}
	m->mapw = (unsigned int)w;
	m->maph = (unsigned int)h;
	m->tilesz = (unsigned int)tilesz;
	m->origin.x = (int)origin_x;
	m->origin.y = (int)origin_y;
	m->zoom = read_uint16(buf);
	m->ssx = read_sint16(buf);
	m->ssy = read_sint16(buf);
	
	/* Read the layer information. */
	if ((m->nlayers = read_uint32(buf)) > MAP_MAX_LAYERS) {
		error_set(_("Too many map layers."));
		goto fail;
	}
	if (m->nlayers < 1) {
		error_set(_("Missing layer zero."));
		goto fail;
	}
	m->layers = Realloc(m->layers, m->nlayers*sizeof(struct map_layer),
	    M_MAP);
	for (i = 0; i < m->nlayers; i++) {
		struct map_layer *lay = &m->layers[i];

		copy_string(lay->name, buf, sizeof(lay->name));
		lay->visible = (int)read_uint8(buf);
		lay->xinc = read_sint16(buf);
		lay->yinc = read_sint16(buf);
		lay->alpha = read_uint8(buf);
	}
	m->cur_layer = (int)read_uint8(buf);
	m->origin.layer = (int)read_uint8(buf);

	/* Allocate and load the nodes. */
	if (map_alloc_nodes(m, m->mapw, m->maph) == -1) {
		goto fail;
	}
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			if (node_load(m, buf, &m->map[y][x]) == -1)
				goto fail;
		}
	}
	pthread_mutex_unlock(&m->lock);
	return (0);
fail:
	pthread_mutex_unlock(&m->lock);
	return (-1);
}

/*
 * Save a node reference.
 * The noderef's parent map must be locked.
 */
void
noderef_save(struct map *m, struct netbuf *buf, struct noderef *r)
{
	off_t ntrans_offs, nmasks_offs;
	Uint32 ntrans = 0, nmasks = 0;
	struct transform *trans;
	struct nodemask *mask;

	/* Save the type of reference, flags and layer information. */
	write_uint32(buf, (Uint32)r->type);
	write_uint32(buf, (Uint32)r->flags);
	write_uint8(buf, r->layer);
	write_sint8(buf, r->friction);

	/* Save the reference. */
	switch (r->type) {
	case NODEREF_SPRITE:
		write_uint32(buf, object_dep_index(m, r->r_sprite.obj));
		write_uint32(buf, r->r_sprite.offs);
		write_sint16(buf, r->r_gfx.xcenter);
		write_sint16(buf, r->r_gfx.ycenter);
		break;
	case NODEREF_ANIM:
		write_uint32(buf, object_dep_index(m, r->r_anim.obj));
		write_uint32(buf, r->r_anim.offs);
		write_sint16(buf, r->r_gfx.xcenter);
		write_sint16(buf, r->r_gfx.ycenter);
		write_uint32(buf, r->r_anim.flags);
		break;
	case NODEREF_WARP:
		write_string(buf, r->r_warp.map);
		write_uint32(buf, (Uint32)r->r_warp.x);
		write_uint32(buf, (Uint32)r->r_warp.y);
		write_uint8(buf, r->r_warp.dir);
	default:
		dprintf("not saving %d node\n", r->type);
		break;
	}

	/* Save the transforms. */
	ntrans_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(trans, &r->transforms, transforms) {
		transform_save(buf, trans);
		ntrans++;
	}
	pwrite_uint32(buf, ntrans, ntrans_offs);
	
	/* Save the masks. */
	nmasks_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(mask, &r->masks, masks) {
		nodemask_save(m, buf, mask);
		nmasks++;
	}
	pwrite_uint32(buf, nmasks, nmasks_offs);
}

void
node_save(struct map *m, struct netbuf *buf, struct node *node)
{
	struct noderef *r;
	off_t nrefs_offs;
	Uint32 nrefs = 0;

	nrefs_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->flags & NODEREF_NOSAVE) {
			continue;
		}
		noderef_save(m, buf, r);
		nrefs++;
	}
	pwrite_uint32(buf, nrefs, nrefs_offs);
}

int
map_save(void *p, struct netbuf *buf)
{
	struct map *m = p;
	int i, x, y;
	
	version_write(buf, &map_ver);

	pthread_mutex_lock(&m->lock);

	/* Write the map header. */
	write_uint32(buf, (Uint32)m->mapw);
	write_uint32(buf, (Uint32)m->maph);
	write_uint32(buf, (Uint32)m->origin.x);
	write_uint32(buf, (Uint32)m->origin.y);
	write_uint32(buf, (Uint32)m->tilesz);
	write_uint32(buf, (Uint32)m->tilesz);
	write_uint16(buf, m->zoom);
	write_sint16(buf, m->ssx);
	write_sint16(buf, m->ssy);

	/* Write the layer information. */
	write_uint32(buf, m->nlayers);
	for (i = 0; i < m->nlayers; i++) {
		struct map_layer *lay = &m->layers[i];

		write_string(buf, lay->name);
		write_uint8(buf, (Uint8)lay->visible);
		write_sint16(buf, lay->xinc);
		write_sint16(buf, lay->yinc);
		write_uint8(buf, lay->alpha);
	}
	write_uint8(buf, m->cur_layer);
	write_uint8(buf, m->origin.layer);

	/* Write the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_save(m, buf, &m->map[y][x]);
		}
	}
	pthread_mutex_unlock(&m->lock);
	return (0);
}

/* Render surface s, scaled to rx,ry pixels. */
/* XXX use something more sophisticated (or cache); fix rleaccel */
static void
draw_scaled(struct map *m, SDL_Surface *s, int rx, int ry)
{
	int x, y, dh, dw, sx, sy;
	Uint8 *src, r1, g1, b1, a1;

	dw = s->w * m->zoom / 100;
	dh = s->h * m->zoom / 100;

	if (SDL_MUSTLOCK(view->v))
		SDL_LockSurface(view->v);
	for (y = 0; y < dh; y++) {
		if ((sy = y*TILESZ/m->tilesz) >= s->h)
			break;
		for (x = 0; x < dw; x++) {
			if ((sx = x*TILESZ/m->tilesz) >= s->w)
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

/*
 * Return a pointer to the referenced sprite surface.
 * If there are transforms to apply, return a pointer to a matching
 * entry in the sprite transformation cache, or allocate a new one.
 */
static __inline__ SDL_Surface *
draw_sprite(struct noderef *r)
{
	struct gfx_cached_sprite *csprite;
	struct gfx_spritecl *spritecl;
	SDL_Surface *origsu = SPRITE(r->r_sprite.obj, r->r_sprite.offs);

	if (TAILQ_EMPTY(&r->transforms))
		return (origsu);

	/*
	 * Look for a cached sprite with the same transforms applied
	 * in the same order.
	 */
	spritecl = &r->r_sprite.obj->gfx->csprites[r->r_sprite.offs];
	SLIST_FOREACH(csprite, &spritecl->sprites, sprites) {
		struct transform *tr1, *tr2;
				
		for (tr1 = TAILQ_FIRST(&r->transforms),
		     tr2 = TAILQ_FIRST(&csprite->transforms);
		     tr1 != TAILQ_END(&r->transforms) &&
		     tr2 != TAILQ_END(&csprite->transforms);
		     tr1 = TAILQ_NEXT(tr1, transforms),
		     tr2 = TAILQ_NEXT(tr2, transforms)) {
			if (!transform_compare(tr1, tr2))
				break;
		}
		if (tr1 == TAILQ_END(&r->transforms) &&
		    tr2 == TAILQ_END(&csprite->transforms))
			break;
	}
	if (csprite != NULL) {
		csprite->last_drawn = SDL_GetTicks();
		return (csprite->su);
	} else {
		struct transform *trans, *ntrans;
		SDL_Surface *su;
		Uint32 saflags = origsu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = origsu->format->alpha;
		Uint32 scflags = origsu->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 scolorkey = origsu->format->colorkey;
		struct gfx_cached_sprite *ncsprite;

		su = SDL_CreateRGBSurface(SDL_SWSURFACE |
		    (origsu->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
		     origsu->w, origsu->h, origsu->format->BitsPerPixel,
		     origsu->format->Rmask,
		     origsu->format->Gmask,
		     origsu->format->Bmask,
		     origsu->format->Amask);
		if (su == NULL)
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		
		ncsprite = Malloc(sizeof(struct gfx_cached_sprite), M_GFX);
		ncsprite->su = su;
		ncsprite->last_drawn = SDL_GetTicks();
		TAILQ_INIT(&ncsprite->transforms);

		SDL_SetAlpha(origsu, 0, 0);
		SDL_SetColorKey(origsu, 0, 0);
		SDL_BlitSurface(origsu, NULL, su, NULL);
		SDL_SetColorKey(origsu, scflags, scolorkey);
		SDL_SetAlpha(origsu, saflags, salpha);

		SDL_LockSurface(su);
		TAILQ_FOREACH(trans, &r->transforms, transforms) {
			trans->func(&su, trans->nargs, trans->args);

			ntrans = Malloc(sizeof(struct transform), M_NODEXFORM);
			transform_init(ntrans, trans->type, trans->nargs,
			    trans->args);
			TAILQ_INSERT_TAIL(&ncsprite->transforms, ntrans,
			    transforms);
		}
		SDL_UnlockSurface(su);
		SLIST_INSERT_HEAD(&spritecl->sprites, ncsprite, sprites);
		return (su);
	}
}

/*
 * Return a pointer to the referenced animation frame.
 * If there are transforms to apply, return a pointer to a matching
 * entry in the anim transformation cache, or allocate a new one.
 */
static __inline__ SDL_Surface *
draw_anim(struct noderef *r)
{
	struct gfx_anim *oanim = ANIM(r->r_anim.obj, r->r_anim.offs);
	struct gfx_cached_anim *canim;
	struct gfx_animcl *animcl;

	if (TAILQ_EMPTY(&r->transforms))
		return (GFX_ANIM_FRAME(r, oanim));

	/*
	 * Look for a cached animation with the same transforms applied
	 * in the same order.
	 */
	animcl = &r->r_anim.obj->gfx->canims[r->r_anim.offs];
	SLIST_FOREACH(canim, &animcl->anims, anims) {
		struct transform *tr1, *tr2;
				
		for (tr1 = TAILQ_FIRST(&r->transforms),
		     tr2 = TAILQ_FIRST(&canim->transforms);
		     tr1 != TAILQ_END(&r->transforms) &&
		     tr2 != TAILQ_END(&canim->transforms);
		     tr1 = TAILQ_NEXT(tr1, transforms),
		     tr2 = TAILQ_NEXT(tr2, transforms)) {
			if (!transform_compare(tr1, tr2))
				break;
		}
		if (tr1 == TAILQ_END(&r->transforms) &&
		    tr2 == TAILQ_END(&canim->transforms))
			break;
	}
	if (canim != NULL) {
		canim->last_drawn = SDL_GetTicks();
		return (GFX_ANIM_FRAME(r, canim->anim));
	} else {
		struct transform *trans, *ntrans;
		struct gfx_anim *nanim;
		struct gfx_cached_anim *ncanim;
		Uint32 i;
		
		ncanim = Malloc(sizeof(struct gfx_cached_anim), M_GFX);
		ncanim->last_drawn = SDL_GetTicks();
		TAILQ_INIT(&ncanim->transforms);

		nanim = ncanim->anim = Malloc(sizeof(struct gfx_anim), M_GFX);
		nanim->frames = Malloc(oanim->nframes*sizeof(SDL_Surface *),
		    M_GFX);
		nanim->nframes = oanim->nframes;
		nanim->maxframes = oanim->nframes;
		nanim->frame = oanim->frame;

		for (i = 0; i < nanim->nframes; i++) {
			SDL_Surface *oframe = oanim->frames[i], *nframe;
			Uint32 saflags = oframe->flags &
			    (SDL_SRCALPHA|SDL_RLEACCEL);
			Uint8 salpha = oframe->format->alpha;
			Uint32 scflags = oframe->flags &
			    (SDL_SRCCOLORKEY|SDL_RLEACCEL);
			Uint32 scolorkey = oframe->format->colorkey;

			nframe = nanim->frames[i] =
			    SDL_CreateRGBSurface(SDL_SWSURFACE |
			    (oframe->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|
			                    SDL_RLEACCEL)),
			     oframe->w, oframe->h, oframe->format->BitsPerPixel,
			     oframe->format->Rmask, oframe->format->Gmask,
			     oframe->format->Bmask, oframe->format->Amask);
			if (nframe == NULL)
				fatal("SDL_CreateRGBSurface: %s",
				    SDL_GetError());
		
			SDL_SetAlpha(oframe, 0, 0);
			SDL_SetColorKey(oframe, 0, 0);
			SDL_BlitSurface(oframe, NULL, nframe, NULL);
			SDL_SetColorKey(oframe, scflags, scolorkey);
			SDL_SetAlpha(oframe, saflags, salpha);

			SDL_LockSurface(nframe);
			TAILQ_FOREACH(trans, &r->transforms, transforms) {
				trans->func(&nframe, trans->nargs, trans->args);
			}
			SDL_UnlockSurface(nframe);
		}
		TAILQ_FOREACH(trans, &r->transforms, transforms) {
			ntrans = Malloc(sizeof(struct transform), M_NODEXFORM);
			transform_init(ntrans, trans->type, trans->nargs,
			    trans->args);
			TAILQ_INSERT_TAIL(&ncanim->transforms, ntrans,
			    transforms);
		}
		SLIST_INSERT_HEAD(&animcl->anims, ncanim, anims);
		return (GFX_ANIM_FRAME(r, nanim));
	}
}

/*
 * Render a graphical noderef to absolute view coordinates rx,ry.
 * The map must be locked.
 */
void
noderef_draw(struct map *m, struct noderef *r, int rx, int ry)
{
	SDL_Surface *su;

	switch (r->type) {
	case NODEREF_SPRITE:
#if defined(DEBUG) || defined(EDITION)
		if (r->r_sprite.obj->gfx == NULL ||
		    r->r_sprite.offs >= r->r_sprite.obj->gfx->nsprites) {
			dprintf("bad sprite offs %d\n", (int)r->r_sprite.offs);
			/* XXX render a number */
			return;
		}
#endif
		su = draw_sprite(r);
		break;
	case NODEREF_ANIM:
#if defined(DEBUG) || defined(EDITION)
		if (r->r_anim.obj->gfx == NULL ||
		    r->r_anim.offs >= r->r_anim.obj->gfx->nanims) {
			dprintf("bad anim offs %d\n", (int)r->r_anim.offs);
			/* XXX render a number */
			return;
		}
#endif
		su = draw_anim(r);
		break;
	default:				/* Not a drawable */
		return;
	}

	if (m->zoom != 100 && ((r->flags & NODEREF_NOSCALE) == 0)) {
		draw_scaled(m, su,
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

#ifdef EDITION

static void
create_view(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win;

	win = window_new(NULL);
	window_set_caption(win, _("%s map view"), OBJECT(mv->map)->name);
	mapview_new(win, mv->map, MAPVIEW_INDEPENDENT, NULL, NULL);
	window_attach(pwin, win);
	window_show(win);
}

struct window *
map_edit(void *p)
{
	extern const struct tool mediasel_tool, layedit_tool;
	extern const struct tool stamp_tool, eraser_tool, magnifier_tool,
	    resize_tool, position_tool, propedit_tool, select_tool,
	    shift_tool, merge_tool, fill_tool, flip_tool, invert_tool;
	struct map *m = p;
	struct window *win;
	struct toolbar *toolbar;
	struct statusbar *statbar;
	struct combo *laysel;
	struct mapview *mv;
	int flags = MAPVIEW_PROPS|MAPVIEW_INDEPENDENT|MAPVIEW_GRID;

	if ((OBJECT(m)->flags & OBJECT_READONLY) == 0)
		flags |= MAPVIEW_EDIT;
	
	win = window_new(NULL);
	window_set_caption(win, _("%s map edition"), OBJECT(m)->name);
	window_set_closure(win, WINDOW_DETACH);
	
	mv = Malloc(sizeof(struct mapview), M_WIDGET);

	toolbar = toolbar_new(win, TOOLBAR_HORIZ, 1);
	toolbar_add_button(toolbar, 0, ICON(NEW_VIEW_ICON), 0, 0,
	    create_view, "%p, %p", mv, win);

	statbar = Malloc(sizeof(struct statusbar), M_OBJECT);
	statusbar_init(statbar);
	statusbar_add_label(statbar, LABEL_STATIC, ".");

	mapview_init(mv, m, flags, toolbar, statbar);
	mapview_reg_stdtools(mv);
	mapview_reg_tool(mv, &mediasel_tool, m);
	mapview_reg_tool(mv, &layedit_tool, m);
	mapview_reg_tool(mv, &stamp_tool, m);
	mapview_reg_tool(mv, &eraser_tool, m);
	mapview_reg_tool(mv, &magnifier_tool, m);
	mapview_reg_tool(mv, &resize_tool, m);
	mapview_reg_tool(mv, &position_tool, m);
	mapview_reg_tool(mv, &propedit_tool, m);
	mapview_reg_tool(mv, &select_tool, m);
	mapview_reg_tool(mv, &shift_tool, m);
	mapview_reg_tool(mv, &merge_tool, m);
	mapview_reg_tool(mv, &fill_tool, m);
	mapview_reg_tool(mv, &flip_tool, m);
	mapview_reg_tool(mv, &invert_tool, m);

	laysel = combo_new(win, COMBO_POLL, _("Layer:"));
	textbox_printf(laysel->tbox, "%d. %s", m->cur_layer,
	    m->layers[m->cur_layer].name);
	event_new(laysel->list, "tlist-poll", layedit_poll, "%p", m);
	event_new(laysel, "combo-selected", mapview_selected_layer, "%p", mv);
	
	object_attach(win, mv);
	object_attach(win, statbar);
	widget_focus(mv);
	return (win);
}
#endif /* EDITION */
