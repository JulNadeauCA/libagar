/*	$Csoft: gobject.c,v 1.3 2005/08/04 07:36:30 vedge Exp $	*/

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
	pthread_mutex_init(&go->lock, &recursive_mutexattr);
	go->type = GOBJECT_NONE;
	go->flags = 0;
	go->parent = NULL;
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

	if (go->parent != NULL) {
		dprintf("reattaching %s to %s\n", OBJECT(go)->name,
		    OBJECT(go->parent)->name);
		space = go->parent;
		pthread_mutex_lock(&space->lock);
		space_detach(space, go);
	} else {
		space = NULL;
	}
	
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

	if (space != NULL) {
		space_attach(space, go);
		dprintf("reattached %s to %s\n", OBJECT(go)->name,
		    OBJECT(go->parent)->name);
		pthread_mutex_unlock(&space->lock);
	}
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

struct noderef *
go_map_sprite(void *obj, struct map *m, int x0, int y0, int l0,
    void *gfx_obj, const char *name)
{
	struct gobject *go = obj;
	struct gfx *gfx;
	struct noderef *r;
	Uint32 offs;
	int x = go->g_map.x + x0;
	int y = go->g_map.y + y0;
	int l = go->g_map.l0 + l0;
	struct node *node = &m->map[y][x];

	if (gfx_obj == NULL || (gfx = OBJECT(gfx_obj)->gfx) == NULL) {
		error_set("NULL gfx");
		return (NULL);
	}
	if (!sprite_find(gfx, name, &offs)) {
		return (NULL);
	}
	while (l >= m->nlayers) {
		if (map_push_layer(m, "") == -1)
			return (NULL);
	}

	r = node_add_sprite(m, node, gfx_obj, offs);
	r->p = obj;
	r->layer = l;

	if (x < go->g_map.x0) { go->g_map.x0 = x; }
	else if (x > go->g_map.x1) { go->g_map.x1 = x; }
	if (y < go->g_map.y0) { go->g_map.y0 = y; }
	else if (y > go->g_map.y1) { go->g_map.y1 = y; }
	
	return (r);
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
