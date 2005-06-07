/*	$Csoft: map.c,v 1.10 2005/05/31 04:03:26 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef MAP

#include <engine/config.h>
#include <engine/view.h>
#include <engine/map/map.h>

#ifdef EDITION
#include <engine/map/mapedit.h>
#include <engine/map/mapview.h>
#include <engine/map/layedit.h>

#include <engine/rg/tileset.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/button.h>
#include <engine/widget/menu.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/separator.h>
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

int map_smooth_scaling = 0;

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
		Free(r, M_MAP_NODEREF);
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

	switch (type) {
	case NODEREF_SPRITE:
		r->r_sprite.obj = NULL;
		r->r_sprite.offs = 0;
		break;
	case NODEREF_ANIM:
		r->r_anim.obj = NULL;
		r->r_anim.offs = 0;
		r->r_anim.flags = 0;
		r->r_anim.frame = 0;
		break;
	case NODEREF_WARP:
		r->r_warp.map = NULL;
		r->r_warp.x = 0;
		r->r_warp.y = 0;
		r->r_warp.dir = 0;
		break;
	case NODEREF_GOBJ:
		r->r_gobj.p = NULL;
		r->r_gobj.flags = 0;
		break;
	}
	
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
		noderef_set_sprite(r, m, NULL, 0);
		break;
	case NODEREF_ANIM:
		noderef_set_anim(r, m, NULL, 0, 0);
		break;
	case NODEREF_WARP:
		Free(r->r_warp.map, 0);
		break;
	case NODEREF_GOBJ:
		if (r->r_gobj.p != NULL) {
			object_del_dep(m, r->r_gobj.p);
			object_page_out(r->r_gobj.p, OBJECT_GFX);
		}
		break;
	default:
		break;
	}
}

/* Allocate and initialize the node map. */
int
map_alloc_nodes(struct map *m, u_int w, u_int h)
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

/* Release the node map. */
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
	m->layers = Realloc(m->layers, 1*sizeof(struct map_layer));
	m->nlayers = 1;
	map_init_layer(&m->layers[0], _("Layer 0"));
}

/* Resize a map, initializing new nodes and destroying any excess ones. */
int
map_resize(struct map *m, u_int w, u_int h)
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

/* Set the display scaling factor. */
void
map_set_zoom(struct map *m, u_int zoom)
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

	space_init(m, "map", name, &map_ops);
	m->redraw = 0;
	m->mapw = 0;
	m->maph = 0;
	m->origin.x = 0;
	m->origin.y = 0;
	m->origin.layer = 0;
	m->map = NULL;
	m->tilesz = TILESZ;
	m->zoom = 100;
	m->ssx = 0;
	m->ssy = 0;
	m->cur_layer = 0;

	m->layers = Malloc(sizeof(struct map_layer), M_MAP);
	m->nlayers = 1;
	map_init_layer(&m->layers[0], _("Layer 0"));
	pthread_mutex_init(&m->lock, &recursive_mutexattr);

#ifdef EDITION
	if (mapedition) {
		extern int mapedit_def_mapw, mapedit_def_maph;

		map_alloc_nodes(m, mapedit_def_mapw, mapedit_def_maph);
	}
#endif
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
	m->layers = Realloc(m->layers, (m->nlayers+1)*sizeof(struct map_layer));
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

void
noderef_set_sprite(struct noderef *r, struct map *map, void *pobj, Uint32 offs)
{
	if (r->r_sprite.obj != NULL) {
		extern int objmgr_exiting;

		if (r->r_sprite.obj->gfx != NULL &&
		    r->r_sprite.obj->gfx->type == GFX_PRIVATE &&
		    !objmgr_exiting) {
			object_page_out(r->r_sprite.obj, OBJECT_DATA);
		}
		object_del_dep(map, r->r_sprite.obj);
		object_page_out(r->r_sprite.obj, OBJECT_GFX);
	}

	if (pobj != NULL) {
		object_add_dep(map, pobj);
		if (object_page_in(pobj, OBJECT_GFX) == -1) {
			fatal("paging gfx: %s", error_get());
		}
		if (OBJECT(pobj)->gfx->type == GFX_PRIVATE &&
		    object_page_in(pobj, OBJECT_DATA) == -1)
			fatal("paging data: %s", error_get());
	}

	r->r_sprite.obj = pobj;
	r->r_sprite.offs = offs;
}

/*
 * Insert a reference to a sprite at pobj:offs.
 * The map must be locked.
 */
struct noderef *
node_add_sprite(struct map *map, struct node *node, void *p, Uint32 offs)
{
	struct object *pobj = p;
	struct noderef *r;

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_SPRITE);
	noderef_set_sprite(r, map, pobj, offs);
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

void
noderef_set_anim(struct noderef *r, struct map *map, void *pobj, Uint32 offs,
    Uint8 flags)
{
	if (r->r_anim.obj != NULL) {
		extern int objmgr_exiting;

		if (r->r_anim.obj->gfx != NULL &&
		    r->r_anim.obj->gfx->type == GFX_PRIVATE &&
		    !objmgr_exiting) {
			object_page_out(r->r_anim.obj, OBJECT_DATA);
		}
		object_del_dep(map, r->r_anim.obj);
		object_page_out(r->r_anim.obj, OBJECT_GFX);
	}

	if (pobj != NULL) {
		object_add_dep(map, pobj);
		if (object_page_in(pobj, OBJECT_GFX) == -1)
			fatal("paging gfx: %s", error_get());
	}

	r->r_anim.obj = pobj;
	r->r_anim.offs = offs;
	r->r_anim.flags = flags;
	r->r_anim.frame = 0;
}

/*
 * Insert a reference to an animation.
 * The map must be locked.
 */
struct noderef *
node_add_anim(struct map *map, struct node *node, void *pobj, Uint32 offs,
    Uint8 flags)
{
	struct noderef *r;

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_ANIM);
	noderef_set_anim(r, map, pobj, offs, flags);
	TAILQ_INSERT_TAIL(&node->nrefs, r, nrefs);
	return (r);
}

/*
 * Insert a reference to a location on another map.
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
 * Insert a reference to a geometric (dynamic) object.
 * The map must be locked.
 */
struct noderef *
node_add_gobj(struct map *map, struct node *node, void *gobj)
{
	struct noderef *r;

	if (gobj != NULL) {
		object_add_dep(map, gobj);
		if (object_page_in(gobj, OBJECT_GFX) == -1)
			fatal("page gobj: %s", error_get());
	}

	r = Malloc(sizeof(struct noderef), M_MAP_NODEREF);
	noderef_init(r, NODEREF_GOBJ);
	r->r_gobj.p = gobj;
	r->r_gobj.flags = 0;
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
	case NODEREF_GOBJ:
		dr = node_add_gobj(dm, dn, sr->r_gobj.p);
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
	Free(r, M_MAP_NODEREF);
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
		Free(r, M_MAP_NODEREF);
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
	
	space_reinit(m);
}

void
map_destroy(void *p)
{
	struct map *m = p;

#ifdef THREADS
	pthread_mutex_destroy(&m->lock);
#endif
	Free(m->layers, M_MAP);

	space_destroy(m);
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
	case NODEREF_GOBJ:
		{
			struct object *gobj;
			Uint32 gobj_id;
			
			gobj_id = read_uint32(buf);
			if ((gobj = object_find_dep(m, gobj_id)) == NULL) {
				error_set("No such gobject: %u.", gobj_id);
				return (-1);
			}
			*r = node_add_gobj(m, node, gobj);
			(*r)->r_gobj.flags = read_uint32(buf);
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
		Free(*r, M_MAP_NODEREF);
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

	if (space_load(m, buf) == -1)
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
	m->mapw = (u_int)w;
	m->maph = (u_int)h;
	m->tilesz = (u_int)tilesz;
	m->origin.x = (int)origin_x;
	m->origin.y = (int)origin_y;
	m->zoom = (u_int)read_uint16(buf);
	m->ssx = (int)read_sint16(buf);
	m->ssy = (int)read_sint16(buf);
	
	/* Read the layer information. */
	if ((m->nlayers = read_uint32(buf)) > MAP_MAX_LAYERS) {
		error_set(_("Too many map layers."));
		goto fail;
	}
	if (m->nlayers < 1) {
		error_set(_("Missing layer zero."));
		goto fail;
	}
	m->layers = Realloc(m->layers, m->nlayers*sizeof(struct map_layer));
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
		break;
	case NODEREF_GOBJ:
		write_uint32(buf, object_dep_index(m, r->r_gobj.p));
		write_uint32(buf, r->r_gobj.flags);
		break;
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
	
	if (space_save(m, buf) == -1)
		return (-1);

	pthread_mutex_lock(&m->lock);

	/* Write the map header. */
	write_uint32(buf, (Uint32)m->mapw);
	write_uint32(buf, (Uint32)m->maph);
	write_uint32(buf, (Uint32)m->origin.x);
	write_uint32(buf, (Uint32)m->origin.y);
	write_uint32(buf, (Uint32)m->tilesz);
	write_uint32(buf, (Uint32)m->tilesz);
	write_uint16(buf, (Uint16)m->zoom);
	write_sint16(buf, (Sint16)m->ssx);
	write_sint16(buf, (Sint16)m->ssy);

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
/* XXX efficient with shrinking but inefficient with growing. */
static void
blit_scaled(struct map *m, SDL_Surface *s, int rx, int ry, int tilesz)
{
	int x, y, sx, sy;
	Uint8 r1, g1, b1, a1;
	Uint32 c;
	u_int wDst = (u_int)(s->w*tilesz/TILESZ);
	u_int hDst = (u_int)(s->h*tilesz/TILESZ);
	int same_fmt = view_same_pixel_fmt(s, view->v);

	if (SDL_MUSTLOCK(s)) {
		SDL_LockSurface(s);
	}
	SDL_LockSurface(view->v);
	
	for (y = 0; y < hDst; y++) {
		if ((sy = y*TILESZ/tilesz) >= s->h)
			break;

		for (x = 0; x < wDst; x++) {
			if ((sx = x*TILESZ/tilesz) >= s->w)
				break;
		
			c = GET_PIXEL(s, (Uint8 *)s->pixels +
			    sy*s->pitch +
			    sx*s->format->BytesPerPixel);
			
			if ((s->flags & SDL_SRCCOLORKEY) &&
			    c == s->format->colorkey)
				continue;
		
			if (s->flags & SDL_SRCALPHA) {
				SDL_GetRGBA(c, s->format, &r1, &g1, &b1, &a1);
				BLEND_RGBA2_CLIPPED(view->v, rx+x, ry+y,
				    r1, g1, b1, a1, ALPHA_OVERLAY);
			} else {
				if (same_fmt) {
					VIEW_PUT_PIXEL2_CLIPPED(rx+x, ry+y, c);
				} else {
					SDL_GetRGB(c, s->format,
					    &r1, &g1, &b1);
					VIEW_PUT_PIXEL2_CLIPPED(rx+x, ry+y,
					    SDL_MapRGB(vfmt, r1, g1, b1));
				}
			}
		}
	}
	if (SDL_MUSTLOCK(s)) {
		SDL_UnlockSurface(s);
	}
	SDL_UnlockSurface(view->v);
}
					
/*
 * Return a pointer to the referenced sprite surface.
 * If there are transforms to apply, return a pointer to a matching
 * entry in the sprite transformation cache, or allocate a new one.
 */
static __inline__ void
draw_sprite(struct noderef *r, SDL_Surface **pSu, u_int *pTexture)
{
	struct sprite *spr = &SPRITE(r->r_sprite.obj,r->r_sprite.offs);
	struct gfx_cached_sprite *csp;

	if (TAILQ_EMPTY(&r->transforms)) {
		*pSu = spr->su;
#ifdef HAVE_OPENGL
		*pTexture = spr->texture;
#endif
		return;
	}

	/*
	 * Look for a cached sprite with the same transforms applied
	 * in the same order.
	 */
	SLIST_FOREACH(csp, &spr->csprites, sprites) {
		struct transform *tr1, *tr2;
				
		for (tr1 = TAILQ_FIRST(&r->transforms),
		     tr2 = TAILQ_FIRST(&csp->transforms);
		     tr1 != TAILQ_END(&r->transforms) &&
		     tr2 != TAILQ_END(&csp->transforms);
		     tr1 = TAILQ_NEXT(tr1, transforms),
		     tr2 = TAILQ_NEXT(tr2, transforms)) {
			if (!transform_compare(tr1, tr2))
				break;
		}
		if (tr1 == TAILQ_END(&r->transforms) &&
		    tr2 == TAILQ_END(&csp->transforms))
			break;
	}
	if (csp != NULL) {
		csp->last_drawn = SDL_GetTicks();
		*pSu = csp->su;
#ifdef HAVE_OPENGL
		*pTexture = csp->texture;
#endif
		return;
	} else {
		struct transform *tr;
		SDL_Surface *sOrig = spr->su;
		SDL_Surface *su;
		Uint32 saflags = sOrig->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = sOrig->format->alpha;
		Uint32 scflags = sOrig->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 scolorkey = sOrig->format->colorkey;

		su = SDL_CreateRGBSurface(SDL_SWSURFACE |
		    (sOrig->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
		     sOrig->w, sOrig->h, sOrig->format->BitsPerPixel,
		     sOrig->format->Rmask,
		     sOrig->format->Gmask,
		     sOrig->format->Bmask,
		     sOrig->format->Amask);
		if (su == NULL)
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		
		csp = Malloc(sizeof(struct gfx_cached_sprite), M_GFX);
		csp->last_drawn = SDL_GetTicks();
		TAILQ_INIT(&csp->transforms);

		SDL_SetAlpha(sOrig, 0, 0);
		SDL_SetColorKey(sOrig, 0, 0);
		SDL_BlitSurface(sOrig, NULL, su, NULL);
		SDL_SetColorKey(sOrig, scflags, scolorkey);
		SDL_SetAlpha(sOrig, saflags, salpha);

		TAILQ_FOREACH(tr, &r->transforms, transforms) {
			SDL_Surface *sNew;
			struct transform *trNew;

			sNew = tr->func(su, tr->nargs, tr->args);
			if (su != sNew) {
				SDL_FreeSurface(su);
				su = sNew;
			}

			trNew = Malloc(sizeof(struct transform), M_NODEXFORM);
			transform_init(trNew, tr->type, tr->nargs, tr->args);
			TAILQ_INSERT_TAIL(&csp->transforms, trNew, transforms);
		}
		SLIST_INSERT_HEAD(&spr->csprites, csp, sprites);
		csp->su = su;
#ifdef HAVE_OPENGL
		if (view->opengl) {
			csp->texture = view_surface_texture(su, csp->texcoords);
		} else {
			csp->texture = 0;
		}
		*pTexture = csp->texture;
#endif
		*pSu = csp->su;
		return;
	}
}

/*
 * Return a pointer to the referenced animation frame.
 * If there are transforms to apply, return a pointer to a matching
 * entry in the anim transformation cache, or allocate a new one.
 */
static void
draw_anim(struct noderef *r, SDL_Surface **pSurface, u_int *pTexture)
{
	struct gfx_anim *oanim = ANIM(r->r_anim.obj, r->r_anim.offs);
	struct gfx_cached_anim *canim;
	struct gfx_animcl *animcl;

	if (TAILQ_EMPTY(&r->transforms)) {
		*pSurface = GFX_ANIM_FRAME(r, oanim);
#ifdef HAVE_OPENGL
		*pTexture = GFX_ANIM_TEXTURE(r, oanim);
#endif
		return;
	}

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
		*pSurface = GFX_ANIM_FRAME(r, canim->anim);
#ifdef HAVE_OPENGL
		*pTexture = GFX_ANIM_TEXTURE(r, canim->anim);
#endif
		return;
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
#ifdef HAVE_OPENGL
		nanim->textures = Malloc(oanim->nframes*sizeof(GLuint),
		    M_GFX);
#endif
		nanim->nframes = oanim->nframes;
		nanim->maxframes = oanim->nframes;
		nanim->frame = oanim->frame;

		for (i = 0; i < nanim->nframes; i++) {
			SDL_Surface *oframe = oanim->frames[i], *sFrame;
			Uint32 saflags = oframe->flags &
			    (SDL_SRCALPHA|SDL_RLEACCEL);
			Uint8 salpha = oframe->format->alpha;
			Uint32 scflags = oframe->flags &
			    (SDL_SRCCOLORKEY|SDL_RLEACCEL);
			Uint32 scolorkey = oframe->format->colorkey;

			sFrame =
			    SDL_CreateRGBSurface(SDL_SWSURFACE |
			    (oframe->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|
			                    SDL_RLEACCEL)),
			     oframe->w, oframe->h, oframe->format->BitsPerPixel,
			     oframe->format->Rmask, oframe->format->Gmask,
			     oframe->format->Bmask, oframe->format->Amask);
			if (sFrame == NULL)
				fatal("SDL_CreateRGBSurface: %s",
				    SDL_GetError());
		
			SDL_SetAlpha(oframe, 0, 0);
			SDL_SetColorKey(oframe, 0, 0);
			SDL_BlitSurface(oframe, NULL, sFrame, NULL);
			SDL_SetColorKey(oframe, scflags, scolorkey);
			SDL_SetAlpha(oframe, saflags, salpha);

			TAILQ_FOREACH(trans, &r->transforms, transforms) {
				SDL_Surface *sNew;

				sNew = trans->func(sFrame, trans->nargs,
				    trans->args);
				if (sNew != sFrame) {
					SDL_FreeSurface(sFrame);
					sFrame = sNew;
				}
			}
			nanim->frames[i] = sFrame;
#ifdef HAVE_OPENGL
			if (view->opengl) {
				nanim->textures[i] =
				    view_surface_texture(sFrame, NULL);
			}
#endif
		}
		TAILQ_FOREACH(trans, &r->transforms, transforms) {
			ntrans = Malloc(sizeof(struct transform), M_NODEXFORM);
			transform_init(ntrans, trans->type, trans->nargs,
			    trans->args);
			TAILQ_INSERT_TAIL(&ncanim->transforms, ntrans,
			    transforms);
		}
		SLIST_INSERT_HEAD(&animcl->anims, ncanim, anims);
		*pSurface = GFX_ANIM_FRAME(r, nanim);
#ifdef HAVE_OPENGL
		*pTexture = GFX_ANIM_TEXTURE(r, nanim);
#endif
		return;
	}
}

/*
 * Render a graphical noderef to absolute view coordinates rx,ry.
 * The map must be locked.
 */
void
noderef_draw(struct map *m, struct noderef *r, int rx, int ry, int tilesz)
{
#if defined(DEBUG) || defined(EDITION)
	int freesu = 0;
#endif
	SDL_Surface *su;
#ifdef HAVE_OPENGL
	u_int texture;
	GLfloat texcoord[4];
#endif

	switch (r->type) {
	case NODEREF_SPRITE:
#if defined(DEBUG) || defined(EDITION)
		if (r->r_sprite.obj->gfx == NULL ||
		    r->r_sprite.offs >= r->r_sprite.obj->gfx->nsprites) {
			char num[16];

			snprintf(num, sizeof(num), "(s%u)", r->r_sprite.offs);
			su = text_render(NULL, -1,
			    SDL_MapRGBA(vfmt, 250, 250, 50, 150), num);
			freesu++;
			goto draw;
		}
#endif
#ifdef HAVE_OPENGL
		draw_sprite(r, &su, &texture);
#else
		draw_sprite(r, &su, NULL);
#endif
		break;
	case NODEREF_ANIM:
#if defined(DEBUG) || defined(EDITION)
		if (r->r_anim.obj->gfx == NULL ||
		    r->r_anim.offs >= r->r_anim.obj->gfx->nanims) {
			char num[16];

			snprintf(num, sizeof(num), "(a%u)", r->r_anim.offs);
			su = text_render(NULL, -1,
			    SDL_MapRGBA(vfmt, 250, 250, 50, 150), num);
			freesu++;
			goto draw;
		}
#endif
#ifdef HAVE_OPENGL
		draw_anim(r, &su, &texture);
#else
		draw_anim(r, &su, NULL);
#endif
		break;
	default:				/* Not a drawable */
		return;
	}

draw:

	if (!view->opengl) {
		if (tilesz != TILESZ && ((r->flags & NODEREF_NOSCALE) == 0)) {
			int dx = rx + r->r_gfx.xcenter*tilesz/TILESZ +
			         r->r_gfx.xmotion*tilesz/TILESZ;
			int dy = ry + r->r_gfx.ycenter*tilesz/TILESZ +
			         r->r_gfx.ymotion*tilesz/TILESZ;
	
			blit_scaled(m, su, dx, dy, tilesz);
		} else {
			SDL_Rect rd;
	
			rd.x = rx + r->r_gfx.xcenter + r->r_gfx.xmotion;
			rd.y = ry + r->r_gfx.ycenter + r->r_gfx.ymotion;
			SDL_BlitSurface(su, NULL, view->v, &rd);
		}
	} else {
#ifdef HAVE_OPENGL
		GLfloat texcoord[4];
		SDL_Rect rd;

		texcoord[0] = 0.0f;
		texcoord[1] = 0.0f;
		texcoord[2] = (GLfloat)su->w / powof2(su->w);
		texcoord[3] = (GLfloat)su->h / powof2(su->h);
		
		if (tilesz != TILESZ && ((r->flags & NODEREF_NOSCALE) == 0)) {
			rd.x = rx + r->r_gfx.xcenter*tilesz/TILESZ +
			    r->r_gfx.xmotion*tilesz/TILESZ;
			rd.y = ry + r->r_gfx.ycenter*tilesz/TILESZ +
			    r->r_gfx.ymotion*tilesz/TILESZ;
			rd.w = su->w*tilesz/TILESZ;
			rd.h = su->h*tilesz/TILESZ;
		} else {
			rd.x = rx + r->r_gfx.xcenter + r->r_gfx.xmotion;
			rd.y = ry + r->r_gfx.ycenter + r->r_gfx.ymotion;
			rd.w = su->w;
			rd.h = su->h;
		}

		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(texcoord[0],	texcoord[1]);
			glVertex2i(rd.x,		rd.y);
			glTexCoord2f(texcoord[2],	texcoord[1]);
			glVertex2i(rd.x+rd.w,		rd.y);
			glTexCoord2f(texcoord[0],	texcoord[3]);
			glVertex2i(rd.x,		rd.y+rd.h);
			glTexCoord2f(texcoord[2],	texcoord[3]);
			glVertex2i(rd.x+rd.w,		rd.y+rd.h);
		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
#endif /* HAVE_OPENGL */
	}

#if defined(DEBUG) || defined(EDITION)
	if (freesu)
		SDL_FreeSurface(su);
#endif
}

#ifdef EDITION

static void
create_view(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win;

	win = window_new(0, NULL);
	window_set_caption(win, _("%s map view"), OBJECT(mv->map)->name);
	mapview_new(win, mv->map, 0, NULL, NULL);
	window_attach(pwin, win);
	window_show(win);
}

static void
revert_map(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;

	if (object_load(m) == 0) {
		text_tmsg(MSG_INFO, 1000,
		    _("Map `%s' reverted successfully."),
		    OBJECT(m)->name);
	} else {
		text_msg(MSG_ERROR, "%s: %s", OBJECT(m)->name,
		    error_get());
	}
}

static void
save_map(int argc, union evarg *argv)
{
	struct map *m = argv[1].p;

	if (object_save(m) == 0) {
		text_tmsg(MSG_INFO, 1250, _("Map `%s' saved successfully."),
		    OBJECT(m)->name);
	} else {
		text_msg(MSG_ERROR, "%s: %s", OBJECT(m)->name,
		    error_get());
	}
}

static void
switch_tool(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct tool *ntool = argv[2].p;

	mapview_select_tool(mv, ntool, mv->map);
	widget_focus(mv);
}

static void
resize_map(int argc, union evarg *argv)
{
	struct mspinbutton *msb = argv[0].p;
	struct map *m = argv[1].p;

	map_resize(m, msb->xvalue, msb->yvalue);
}

static void
edit_properties(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct map *m = mv->map;
	struct window *pwin = argv[2].p;
	struct window *win;
	struct box *bo;
	struct mspinbutton *msb;
	struct spinbutton *sb;
	struct checkbox *cbox;

	if ((win = window_new(WINDOW_NO_RESIZE, "map-props-%s",
	    OBJECT(m)->name)) == NULL) {
		return;
	}
	window_set_caption(win, _("Properties of \"%s\""), OBJECT(m)->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	
	bo = box_new(win, BOX_VERT, 0);
	{
		cbox = checkbox_new(bo, _("Smooth scaling"));
		widget_bind(cbox, "state", WIDGET_INT, &map_smooth_scaling);
	}

	bo = box_new(win, BOX_VERT, 0);
	{
		msb = mspinbutton_new(bo, "x", _("Map size: "));
		mspinbutton_set_range(msb, 1, MAP_MAX_WIDTH);
		msb->xvalue = m->mapw;
		msb->yvalue = m->maph;
		event_new(msb, "mspinbutton-changed", resize_map, "%p", m);
	
		msb = mspinbutton_new(bo, "x", _("Node offset: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &mv->mx);
		widget_bind(msb, "yvalue", WIDGET_INT, &mv->my);
		mspinbutton_set_range(msb, 1, MAP_MAX_WIDTH);
	
		msb = mspinbutton_new(bo, "x", _("Display offset: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &mv->xoffs);
		widget_bind(msb, "yvalue", WIDGET_INT, &mv->yoffs);
		mspinbutton_set_range(msb, 1, MAP_MAX_TILESZ);
	
		msb = mspinbutton_new(bo, "x", _("Display area: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &mv->mw);
		widget_bind(msb, "yvalue", WIDGET_INT, &mv->mh);
		mspinbutton_set_range(msb, 1, MAP_MAX_WIDTH);
		
		label_new(bo, LABEL_POLLED, _("Fit w=%[ibool] h=%[ibool]"),
		    &mv->wfit, &mv->hfit);
		
		label_new(bo, LABEL_POLLED, _("Modulo w=%i h=%i"),
		    &mv->wmod, &mv->hmod);
	}

	bo = box_new(win, BOX_VERT, 0);
	{
		label_new(bo, LABEL_POLLED_MT,
		    _("Zoom factor: %i%% (map=%i%%)"), &m->lock,
		    &mv->zoom, &m->zoom);
		label_new(bo, LABEL_POLLED_MT,
		    _("Tile size : %ux%u (map=%ux%u)"), &m->lock,
		    &mv->tilesz, &mv->tilesz, &m->tilesz, &m->tilesz);
		label_new(bo, LABEL_POLLED_MT, _("Edited layer: %i"), &m->lock,
		    &m->cur_layer);
	}

	bo = box_new(win, BOX_VERT, 0);
	{
		label_new(win, LABEL_POLLED, _("Cursor position: %ix%i"),
		    &mv->cx, &mv->cy);
		label_new(win, LABEL_POLLED, _("Cursor delta: %ix%i"),
		    &mv->cxrel, &mv->cyrel);
		label_new(win, LABEL_POLLED, _("Mouse scrolling: %[ibool]"),
		    &mv->mouse.scrolling);
		label_new(win, LABEL_POLLED, _("Mouse centering: %[ibool]"),
		    &mv->mouse.centering);
		label_new(win, LABEL_POLLED,
		    _("Mouse selection: %[ibool] (%i+%i,%i+%i)"), &mv->msel.set,
		    &mv->msel.x, &mv->msel.xoffs, &mv->msel.y, &mv->msel.yoffs);
		label_new(win, LABEL_POLLED,
		    _("Effective selection: %[ibool] (%ix%i at %i,%i)"),
		    &mv->esel.set,
		    &mv->esel.w, &mv->esel.h, &mv->esel.x, &mv->esel.y);
	}

	bo = box_new(win, BOX_VERT, 0);
	{
		msb = mspinbutton_new(win, "x", _("Origin position: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &m->origin.x);
		widget_bind(msb, "yvalue", WIDGET_INT, &m->origin.y);
		mspinbutton_set_range(msb, 0, MAP_MAX_WIDTH);

		sb = spinbutton_new(win, _("Origin layer: "));
		widget_bind(sb, "value", WIDGET_INT, &m->origin.layer);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
find_art(struct tlist *tl, struct object *pob)
{
	struct object *cob;
	struct tlist_item *it;

	if (OBJECT_TYPE(pob, "tileset")) {
		struct tileset *ts = (struct tileset *)pob;
		struct tlist_item *sit;
		struct tile *t;
		struct animation *anim;

		it = tlist_insert(tl, object_icon(ts), "%s%s", pob->name,
		    (pob->flags&OBJECT_DATA_RESIDENT) ? _(" (resident)") : "");
		it->p1 = pob;
		it->depth = 0;
		it->class = "tileset";
		
		pthread_mutex_lock(&ts->lock);
		
		if (!TAILQ_EMPTY(&ts->tiles) ||
		    !TAILQ_EMPTY(&ts->animations))
			it->flags |= TLIST_HAS_CHILDREN;

		if (it->flags & TLIST_HAS_CHILDREN &&
		    tlist_visible_children(tl, it)) {
			TAILQ_FOREACH(t, &ts->tiles, tiles) {
				sit = tlist_insert(tl, t->su, "%s (%ux%u)",
				    t->name, t->su->w, t->su->h);
				sit->depth = 1;
				sit->class = "tile";
				sit->p1 = t;
			}
			TAILQ_FOREACH(anim, &ts->animations, animations) {
				sit = tlist_insert(tl,
				    anim->nframes>0 ? anim->frames[0].su : NULL,
				    "%s (%ux%u*%u)",
				    anim->name, anim->w, anim->h,
				    anim->nframes);
				sit->depth = 1;
				sit->class = "anim";
				sit->p1 = anim;
			}
		}
		pthread_mutex_unlock(&ts->lock);
	}

	TAILQ_FOREACH(cob, &pob->children, cobjs)
		find_art(tl, cob);
}

static void
find_objs(struct tlist *tl, struct object *pob, int depth)
{
	struct object *cob;
	struct tlist_item *it;
	SDL_Surface *icon;

	it = tlist_insert(tl, object_icon(pob), "%s%s", pob->name,
	    (pob->flags & OBJECT_DATA_RESIDENT) ? " (resident)" : "");
	it->depth = depth;
	it->class = "object";
	it->p1 = pob;

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
		if (object_root(pob) == pob)
			it->flags |= TLIST_VISIBLE_CHILDREN;
	}
	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_children(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			find_objs(tl, cob, depth+1);
	}
}

static void
poll_art(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *pob = argv[1].p;
	struct tlist_item *it;

	tlist_clear_items(tl);
	lock_linkage();
	find_art(tl, pob);
	unlock_linkage();
	tlist_restore_selections(tl);
}

static void
selected_art(int argc, union evarg *argv)
{
	extern const struct tool stamp_tool;
	extern enum gfx_snap_mode stamp_snap_mode;
	struct tlist *tl = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct tlist_item *it = argv[2].p;

	if (it->p1 != NULL &&
	    strcmp(it->class, "tile") == 0) {
		struct tile *t = it->p1;
		struct sprite *spr = &SPRITE(t->ts,t->sprite);
	
		stamp_snap_mode = spr->snap_mode;
	}

}

static void
poll_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *pob = argv[1].p;
	struct tlist_item *it;

	tlist_clear_items(tl);
	lock_linkage();
	find_objs(tl, pob, 0);
	unlock_linkage();
	tlist_restore_selections(tl);
}

static void
poll_layers(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct map *m = argv[1].p;
	struct tlist_item *it;
	int i;

	tlist_clear_items(tl);
	for (i = 0; i < m->nlayers; i++) {
		struct map_layer *lay = &m->layers[i];

		it = tlist_insert(tl, NULL, "%s%s%s",
		    (i == m->cur_layer) ? "[*] " : "", lay->name,
		    lay->visible ? "" : _(" (hidden)"));
		it->p1 = lay;
		it->class = "layer";
	}
	tlist_restore_selections(tl);
}

struct window *
map_edit(void *p)
{
	extern const struct tool mediasel_tool, layedit_tool;
	extern const struct tool stamp_tool, eraser_tool, magnifier_tool,
	    resize_tool, propedit_tool, select_tool, shift_tool, merge_tool,
	    fill_tool, flip_tool, invert_tool;
	struct map *m = p;
	struct window *win;
	struct toolbar *toolbar;
	struct statusbar *statbar;
	struct combo *laysel;
	struct mapview *mv;
	struct AGMenu *menu;
	struct AGMenuItem *pitem;
	struct box *box_h, *box_v;
	int flags = 0;

	if ((OBJECT(m)->flags & OBJECT_READONLY) == 0)
		flags |= MAPVIEW_EDIT;
	
	win = window_new(0, NULL);
	window_set_caption(win, "%s", OBJECT(m)->name);

	mv = Malloc(sizeof(struct mapview), M_WIDGET);

	toolbar = Malloc(sizeof(struct toolbar), M_OBJECT);
	toolbar_init(toolbar, TOOLBAR_HORIZ, 1, 0);

	statbar = Malloc(sizeof(struct statusbar), M_OBJECT);
	statusbar_init(statbar);

	mapview_init(mv, m, flags, toolbar, statbar);
	mapview_prescale(mv, 2, 2);

	menu = menu_new(win);
	pitem = menu_add_item(menu, _("Map"));
	{
		menu_action(pitem, _("Save"), OBJSAVE_ICON, save_map, "%p", m);
		menu_action(pitem, _("Revert"), OBJLOAD_ICON, revert_map,
		    "%p", m);

		menu_separator(pitem);

		menu_action(pitem, _("Properties..."), SETTINGS_ICON,
		    edit_properties, "%p, %p", mv, win);
		menu_action(pitem, _("Import media..."), MEDIASEL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &mediasel_tool, m, 0));
		
		menu_separator(pitem);

		menu_int_flags_mp(pitem, _("Editable"), EDIT_ICON,
		    &OBJECT(m)->flags, OBJECT_READONLY, 1,
		    &OBJECT(m)->lock);
		menu_int_flags_mp(pitem, _("Indestructible"), TRASH_ICON,
		    &OBJECT(m)->flags, OBJECT_INDESTRUCTIBLE, 0,
		    &OBJECT(m)->lock);
	}

	pitem = menu_add_item(menu, _("View"));
	{
		extern int mapview_bg, mapview_bg_moving;

		menu_action(pitem, _("New view"), NEW_VIEW_ICON,
		    create_view, "%p, %p", mv, win);

		menu_separator(pitem);

		menu_int_flags(pitem, _("Display grid"), GRID_ICON,
		    &mv->flags, MAPVIEW_GRID, 0);
		menu_int_flags(pitem, _("Display node properties"), PROPS_ICON,
		    &mv->flags, MAPVIEW_PROPS, 0);
		menu_int_flags(pitem, _("Cursor"), SELECT_TOOL_ICON,
		    &mv->flags, MAPVIEW_NO_CURSOR, 1);
		
		menu_int_bool(pitem, _("Tiled background"), GRID_ICON,
		    &mapview_bg, 0);
		menu_int_bool(pitem, _("Moving background"), GRID_ICON,
		    &mapview_bg_moving, 0);
#ifdef DEBUG
		menu_int_flags(pitem, _("Clipped edges"), GRID_ICON,
		    &WIDGET(mv)->flags, WIDGET_CLIPPING, 1);
#endif		
		menu_separator(pitem);

		menu_action(pitem, _("Zoom settings..."), MAGNIFIER_CURSORBMP,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &magnifier_tool, m, 0));
	}
	
	pitem = menu_add_item(menu, _("Layers"));
	{
		menu_action(pitem, _("Edit layers"), MAGNIFIER_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &layedit_tool, m, 0));
	}

	pitem = menu_add_item(menu, _("Tools"));
	{
		menu_action(pitem, _("Select"), SELECT_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &select_tool, m, 1));
		menu_action(pitem, _("Stamp"), STAMP_TOOL_ICON,
		    0, 0, switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &stamp_tool, m, 1));
		menu_action(pitem, _("Eraser"), ERASER_TOOL_ICON,
		    0, 0, switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &eraser_tool, m, 1));
		menu_action(pitem, _("Resize"), RESIZE_TOOL_ICON,
		    0, 0, switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &resize_tool, m, 1));

		menu_action_kb(pitem, _("Fill region"), FILL_TOOL_ICON,
		    KMOD_CTRL, SDLK_f, switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &fill_tool, m, 1));
		menu_action(pitem, _("Apply texture"), MERGE_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &merge_tool, m, 1));
		
		menu_action(pitem, _("Entity properties"), PROPEDIT_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &propedit_tool, m, 01));
		menu_action(pitem, _("Displace sprite"), SHIFT_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &shift_tool, m, 1));
		menu_action(pitem, _("Flip/mirror sprite"), FLIP_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &flip_tool, m, 1));
		menu_action(pitem, _("Invert sprite"), INVERT_TOOL_ICON,
		    switch_tool, "%p, %p", mv,
		    mapview_reg_tool(mv, &invert_tool, m, 1));
	}
	
	object_attach(win, toolbar);
	separator_new(win, SEPARATOR_HORIZ);

	box_h = box_new(win, BOX_HORIZ, BOX_WFILL|BOX_HFILL);
	{
		struct notebook *nb;
		struct notebook_tab *ntab;
		struct tlist *tl;
	
		nb = notebook_new(box_h, NOTEBOOK_HFILL);

		ntab = notebook_add_tab(nb, _("Artwork"), BOX_VERT);
		{
			tl = tlist_new(ntab, TLIST_POLL|TLIST_TREE);
			event_new(tl, "tlist-poll", poll_art, "%p", world);
			event_new(tl, "tlist-selected", selected_art, "%p", mv);
			mv->art_tl = tl;
			WIDGET(tl)->flags &= ~(WIDGET_FOCUSABLE);
		}

		ntab = notebook_add_tab(nb, _("Objects"), BOX_VERT);
		{
			tl = tlist_new(ntab, TLIST_POLL|TLIST_TREE);
			event_new(tl, "tlist-poll", poll_objs, "%p", world);
			mv->objs_tl = tl;
			WIDGET(tl)->flags &= ~(WIDGET_FOCUSABLE);
		}
		
		ntab = notebook_add_tab(nb, _("Layers"), BOX_VERT);
		{
			tl = tlist_new(ntab, TLIST_POLL);
			event_new(tl, "tlist-poll", poll_layers, "%p", m);
			mv->layers_tl = tl;
			WIDGET(tl)->flags &= ~(WIDGET_FOCUSABLE);
		}
		
		box_v = box_new(box_h, BOX_VERT, BOX_WFILL|BOX_HFILL);
		{
			laysel = combo_new(box_v, COMBO_POLL, _("Layer:"));
			event_new(laysel->list, "tlist-poll", layedit_poll,
			    "%p", m);
			event_new(laysel, "combo-selected",
			    mapview_selected_layer, "%p", mv);
			combo_select_pointer(laysel, &m->layers[m->cur_layer]);
			object_attach(box_v, mv);
			WIDGET(laysel)->flags &= ~(WIDGET_FOCUSABLE);
			WIDGET(laysel->list)->flags &= ~(WIDGET_FOCUSABLE);
		}
	}

	object_attach(win, statbar);

	window_scale(win, -1, -1);
	window_set_geometry(win,
	    view->w/4, view->h/4,
	    view->w/2, view->h/2);

	widget_replace_surface(mv->status, mv->status->surface,
	    text_render(NULL, -1, COLOR(TEXT_COLOR), _("Select a tool.")));
	widget_focus(mv);
	return (win);
}
#endif /* EDITION */
#endif /* MAP */
