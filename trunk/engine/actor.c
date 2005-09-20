/*	$Csoft: actor.c,v 1.9 2005/08/27 04:39:59 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "actor.h"

const struct version actor_ver = {
	"agar actor",
	0, 0
};

void
actor_init(void *obj, const char *type, const char *name,
    const struct actor_ops *ops)
{
	char tname[OBJECT_TYPE_MAX];
	struct actor *go = obj;

	strlcpy(tname, "actor.", sizeof(tname));
	strlcat(tname, type, sizeof(tname));

	object_init(go, tname, name, ops);
	object_remain(go, OBJECT_REMAIN_DATA);
	pthread_mutex_init(&go->lock, &recursive_mutexattr);
	go->flags = 0;
	go->parent = NULL;
	go->type = ACTOR_MAP;
	go->g_map.x = 0;
	go->g_map.y = 0;
	go->g_map.l0 = 0;
	go->g_map.l1 = 0;
	go->g_map.x0 = 0;
	go->g_map.y0 = 0;
	go->g_map.x1 = 0;
	go->g_map.y1 = 0;
	go->g_map.xmot = 0;
	go->g_map.ymot = 0;
}

void
actor_reinit(void *obj)
{
}

void
actor_destroy(void *obj)
{
	struct actor *go = obj;

	pthread_mutex_destroy(&go->lock);
}

int
actor_load(void *obj, struct netbuf *buf)
{
	struct actor *go = obj;
	struct space *space;

	if (version_read(buf, &actor_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&go->lock);

#if 0
	if (go->parent != NULL) {
		dprintf("reattaching %s to %s\n", OBJECT(go)->name,
		    OBJECT(go->parent)->name);
		space = go->parent;
		pthread_mutex_lock(&space->lock);
		space_detach(space, go);
	} else {
		space = NULL;
	}
#endif

	go->type = (enum actor_type)read_uint32(buf);
	go->flags = (int)read_uint32(buf) & ACTOR_SAVED_FLAGS;
	switch (go->type) {
	case ACTOR_MAP:
		go->g_map.x = (int)read_uint32(buf);
		go->g_map.y = (int)read_uint32(buf);
		go->g_map.l0 = (int)read_uint8(buf);
		go->g_map.l1 = (int)read_uint8(buf);
		go->g_map.x0 = go->g_map.x;
		go->g_map.y0 = go->g_map.y;
		go->g_map.x1 = go->g_map.x;
		go->g_map.y1 = go->g_map.y;
		break;
	case ACTOR_SCENE:
		go->g_scene.x = read_double(buf);
		go->g_scene.y = read_double(buf);
		go->g_scene.z = read_double(buf);
		go->g_scene.dx = read_double(buf);
		go->g_scene.dy = read_double(buf);
		go->g_scene.dz = read_double(buf);
		break;
	default:
		break;
	}
#if 0
	if (space != NULL) {
		space_attach(space, go);
		dprintf("reattached %s to %s\n", OBJECT(go)->name,
		    OBJECT(go->parent)->name);
		pthread_mutex_unlock(&space->lock);
	}
#endif
	pthread_mutex_unlock(&go->lock);
	return (0);
}

int
actor_save(void *obj, struct netbuf *buf)
{
	struct actor *go = obj;

	version_write(buf, &actor_ver);

	pthread_mutex_lock(&go->lock);
	write_uint32(buf, (Uint32)go->type);
	write_uint32(buf, (Uint32)go->flags & ACTOR_SAVED_FLAGS);

	switch (go->type) {
	case ACTOR_MAP:
		write_uint32(buf, (Uint32)go->g_map.x);
		write_uint32(buf, (Uint32)go->g_map.y);
		write_uint8(buf, (Uint8)go->g_map.l0);
		write_uint8(buf, (Uint8)go->g_map.l1);
		break;
	case ACTOR_SCENE:
		write_double(buf, go->g_scene.x);
		write_double(buf, go->g_scene.y);
		write_double(buf, go->g_scene.z);
		write_double(buf, go->g_scene.dx);
		write_double(buf, go->g_scene.dy);
		write_double(buf, go->g_scene.dz);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&go->lock);
	return (0);
}

void
actor_update(void *obj)
{
}

static void
move_nodes(struct actor *go, int xo, int yo)
{
	struct map *m = go->parent;
	int x, y;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			struct node *n1 = &m->map[y][x];
			struct node *n2 = &m->map[y+yo][x+xo];
			struct noderef *r, *nr;

			for (r = TAILQ_FIRST(&n1->nrefs);
			     r != TAILQ_END(&n1->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1) {
					TAILQ_REMOVE(&n1->nrefs, r, nrefs);
					TAILQ_INSERT_TAIL(&n2->nrefs, r, nrefs);
					r->r_gfx.xmotion = go->g_map.xmot;
					r->r_gfx.ymotion = go->g_map.ymot;
					break;
				}
			}
		}
	}
	go->g_map.x += xo;
	go->g_map.y += yo;
	go->g_map.x0 += xo;
	go->g_map.y0 += yo;
	go->g_map.x1 += xo;
	go->g_map.y1 += yo;
}

void
actor_move_sprite(void *obj, int xo, int yo)
{
	struct actor *go = obj;
	struct map *m = go->parent;
	int x, y;

	pthread_mutex_lock(&go->lock);

	go->g_map.xmot += xo;
	go->g_map.ymot += yo;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *r, *nr;
		
			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->p != go ||
				    r->layer < go->g_map.l0 ||
				    r->layer > go->g_map.l1) {
					continue;
				}

				r->r_gfx.xmotion = go->g_map.xmot;
				r->r_gfx.ymotion = go->g_map.ymot;
					
				switch (go->g_map.da) {
				case 0:
					if (go->g_map.xmot < -TILESZ/2) {
						go->g_map.xmot = +TILESZ/2;
						move_nodes(go, -1, 0);
						goto out;
					}
					break;
				case 90:
					if (go->g_map.ymot < -TILESZ/2) {
						go->g_map.ymot = +TILESZ/2;
						move_nodes(go, 0, -1);
						goto out;
					}
					break;
				case 180:
					if (go->g_map.xmot > +TILESZ/2) {
						go->g_map.xmot = -TILESZ/2;
						move_nodes(go, +1, 0);
						goto out;
					}
					break;
				case 270:
					if (go->g_map.ymot > +TILESZ/2) {
						go->g_map.ymot = -TILESZ/2;
						move_nodes(go, 0, +1);
						goto out;
					}
					break;
				}
			}
		}
	}
out:
	pthread_mutex_unlock(&go->lock);
}

int
actor_set_sprite(void *obj, int x, int y, int l0, void *gfx_obj,
    const char *name)
{
	struct actor *go = obj;

	actor_unmap_sprite(go);
	return (actor_map_sprite(go, x, y, l0, gfx_obj, name));
}

int
actor_map_sprite(void *obj, int X0, int Y0, int L0, void *gfx_obj,
    const char *name)
{
	struct actor *go = obj;
	struct map *m = go->parent;
	struct gfx *gfx;
	Uint32 offs;
	struct sprite *spr;
	int x = go->g_map.x + X0;
	int y = go->g_map.y + Y0;
	int l0 = go->g_map.l0 + L0, l;
	int sx, sy, dx, dy;
	int dx0, dy0, xorig, yorig;
	SDL_Surface *su;
	int n = 0;

	if (gfx_obj == NULL || (gfx = OBJECT(gfx_obj)->gfx) == NULL) {
		error_set("NULL gfx");
		return (-1);
	}
	if (!sprite_find(gfx, name, &offs)) {
		return (-1);
	}
	spr = &gfx->sprites[offs];
	su = spr->su;
	dx0 = x - spr->xOrig/TILESZ;
	dy0 = y - spr->yOrig/TILESZ;
	xorig = spr->xOrig%TILESZ;
	yorig = spr->yOrig%TILESZ;

	for (sy = 0, dy = dy0;
	     sy < su->h;
	     sy += TILESZ, dy++) {
		for (sx = 0, dx = dx0;
		     sx < su->w;
		     sx += TILESZ, dx++) {
			struct node *dn;
			struct noderef *r;

			if (dx < 0 || dx >= m->mapw ||
			    dy < 0 || dy >= m->maph) {
				goto out;
			}
			dn = &m->map[dy][dx];

			r = node_add_sprite(m, dn, gfx_obj, offs);
			r->p = obj;
			r->r_gfx.rs.x = sx;
			r->r_gfx.rs.y = sy;
			r->r_gfx.rs.w = TILESZ;
			r->r_gfx.rs.h = TILESZ;
			r->r_gfx.xorigin = xorig;
			r->r_gfx.yorigin = yorig;
			r->r_gfx.xcenter = TILESZ/2;
			r->r_gfx.ycenter = TILESZ/2;
			r->r_gfx.xmotion = go->g_map.xmot;
			r->r_gfx.ymotion = go->g_map.ymot;
			r->flags |= spr->attrs[n];
			r->flags |= NODEREF_NOSAVE;

			l = l0 + spr->layers[n];
			if (l < 0) {
				l = 0;
			} else {
				while (m->nlayers <= l) {
					map_push_layer(m, "");
				}
			}
			r->layer = l;
	
			if (dx < go->g_map.x0) { go->g_map.x0 = dx; }
			else if (dx > go->g_map.x1) { go->g_map.x1 = dx; }
			if (dy < go->g_map.y0) { go->g_map.y0 = dy; }
			else if (dy > go->g_map.y1) { go->g_map.y1 = dy; }
			if (l < go->g_map.l0) { go->g_map.l0 = l; }
			else if (l > go->g_map.l1) { go->g_map.l1 = l; }
out:
			n++;
		}
	}
	return (0);
}

void
actor_unmap_sprite(void *obj)
{
	struct actor *go = obj;
	struct map *m = go->parent;
	int x, y;

	if (m == NULL)
		return;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			struct node *node = &m->map[y][x];
			struct noderef *r, *nr;
		
			for (r = TAILQ_FIRST(&node->nrefs);
			     r != TAILQ_END(&node->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1)
					node_remove_ref(m, node, r);
			}
		}
	}
}

#ifdef EDITION
void
actor_edit(struct actor *go, void *cont)
{
	label_new(cont, LABEL_POLLED, _("Type: %d"), &go->type);
	label_new(cont, LABEL_POLLED, _("Flags: 0x%x"), &go->flags);
	label_new(cont, LABEL_POLLED, _("Map layers: %d-%d"),
	    &go->g_map.l0, &go->g_map.l1);
	label_new(cont, LABEL_POLLED, _("Map position: [%d,%d]"),
	    &go->g_map.x, &go->g_map.y);
	label_new(cont, LABEL_POLLED, _("Map extent: [%d,%d]-[%d,%d]"),
	    &go->g_map.x0, &go->g_map.y0, &go->g_map.x1, &go->g_map.y1);
	label_new(cont, LABEL_POLLED, _("Map offset: [%d,%d]"),
	    &go->g_map.xmot, &go->g_map.ymot);
	label_new(cont, LABEL_POLLED, _("Direction: %d(%d)"),
	    &go->g_map.da, &go->g_map.dv);
}
#endif /* EDITION */
