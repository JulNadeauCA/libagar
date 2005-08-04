/*	$Csoft: gobject.c,v 1.2 2005/04/14 06:19:35 vedge Exp $	*/

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
	pthread_mutex_init(&go->lock, NULL);
	go->type = GOBJECT_NONE;
}

void
gobject_reinit(void *obj)
{
	struct gobject *go = obj;

	pthread_mutex_lock(&go->lock);
	pthread_mutex_unlock(&go->lock);
}

void
gobject_destroy(void *obj)
{
	struct gobject *go = obj;

	switch (go->type) {
	case GOBJECT_MAP:
		map_destroy(&go->g_map.fm);
		break;
	default:
		break;
	}
	pthread_mutex_destroy(&go->lock);
}

int
gobject_load(void *obj, struct netbuf *buf)
{
	struct gobject *go = obj;

	if (version_read(buf, &gobject_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&go->lock);
	go->type = (enum gobject_type)read_uint32(buf);
	switch (go->type) {
	case GOBJECT_MAP:
		go->g_map.x = (int)read_uint32(buf);
		go->g_map.y = (int)read_uint32(buf);
		go->g_map.layer = (int)read_uint32(buf);
		map_reinit(&go->g_map.fm);
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
	switch (go->type) {
	case GOBJECT_MAP:
		write_uint32(buf, (Uint32)go->g_map.x);
		write_uint32(buf, (Uint32)go->g_map.y);
		write_uint32(buf, (Uint32)go->g_map.layer);
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

struct window *
gobject_edit(void *obj)
{
	struct gobject *gobj = obj;
	struct window *win;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, _("Geometric object: %s"), OBJECT(gobj)->name);

	return (win);
}
