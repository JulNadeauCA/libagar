/*	$Csoft: gobject.c,v 1.7 2005/08/15 03:52:25 vedge Exp $	*/

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

#include "gobject.h"

const struct version gobject_ver = {
	"agar geometric object",
	0, 0
};

void
gobject_init(void *obj, const char *type, const char *name,
    const struct gobject_ops *ops)
{
	char tname[OBJECT_TYPE_MAX];
	struct gobject *go = obj;

	strlcpy(tname, "gobject.", sizeof(tname));
	strlcat(tname, type, sizeof(tname));

	object_init(go, tname, name, ops);
	object_remain(go, OBJECT_REMAIN_DATA);
	pthread_mutex_init(&go->lock, &recursive_mutexattr);
	go->flags = 0;
	go->parent = NULL;
	go->type = GOBJECT_MAP;
	go->g_map.x = 0;
	go->g_map.y = 0;
	go->g_map.l0 = 0;
	go->g_map.l1 = 0;
	go->g_map.x0 = 0;
	go->g_map.y0 = 0;
	go->g_map.x1 = 0;
	go->g_map.y1 = 0;
}

void
gobject_reinit(void *obj)
{
}

void
gobject_destroy(void *obj)
{
	struct gobject *go = obj;

	pthread_mutex_destroy(&go->lock);
}

int
gobject_load(void *obj, struct netbuf *buf)
{
	struct gobject *go = obj;
	struct space *space;

	if (version_read(buf, &gobject_ver, NULL) != 0)
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

	go->type = (enum gobject_type)read_uint32(buf);
	go->flags = (int)read_uint32(buf) & GOBJECT_SAVED_FLAGS;
	switch (go->type) {
	case GOBJECT_MAP:
		go->g_map.x = (int)read_uint32(buf);
		go->g_map.y = (int)read_uint32(buf);
		go->g_map.l0 = (int)read_uint8(buf);
		go->g_map.l1 = (int)read_uint8(buf);
		go->g_map.x0 = go->g_map.x;
		go->g_map.y0 = go->g_map.y;
		go->g_map.x1 = go->g_map.x;
		go->g_map.y1 = go->g_map.y;
		break;
	case GOBJECT_SCENE:
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
gobject_save(void *obj, struct netbuf *buf)
{
	struct gobject *go = obj;

	version_write(buf, &gobject_ver);

	pthread_mutex_lock(&go->lock);
	write_uint32(buf, (Uint32)go->type);
	write_uint32(buf, (Uint32)go->flags & GOBJECT_SAVED_FLAGS);

	switch (go->type) {
	case GOBJECT_MAP:
		write_uint32(buf, (Uint32)go->g_map.x);
		write_uint32(buf, (Uint32)go->g_map.y);
		write_uint8(buf, (Uint8)go->g_map.l0);
		write_uint8(buf, (Uint8)go->g_map.l1);
		break;
	case GOBJECT_SCENE:
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
gobject_update(void *obj)
{
}

int
go_map_sprite(void *obj, struct map *m, int X0, int Y0, int L0,
    void *gfx_obj, const char *name)
{
	struct gobject *go = obj;
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
	     sy < su->h && dy < m->maph;
	     sy += TILESZ, dy++) {
		for (sx = 0, dx = dx0;
		     sx < su->w && dx < m->mapw;
		     sx += TILESZ, dx++) {
			struct node *dn = &m->map[dy][dx];
			struct noderef *r;
	
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
			r->flags |= spr->attrs[n];

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

			n++;
		}
	}
	return (0);
}

#ifdef EDITION
void
gobject_edit(struct gobject *go, void *cont)
{
	label_new(cont, LABEL_POLLED, _("Type: %d"), &go->type);
	label_new(cont, LABEL_POLLED, _("Flags: 0x%x"), &go->flags);
	label_new(cont, LABEL_POLLED, _("Map layers: %d-%d"),
	    &go->g_map.l0, &go->g_map.l1);
	label_new(cont, LABEL_POLLED, _("Map position: [%d,%d]"),
	    &go->g_map.x, &go->g_map.y);
	label_new(cont, LABEL_POLLED, _("Map extent: [%d,%d]-[%d,%d]"),
	    &go->g_map.x0, &go->g_map.y0, &go->g_map.x1, &go->g_map.y1);
}
#endif /* EDITION */
