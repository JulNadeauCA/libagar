/*	$Csoft: position.c,v 1.2 2004/02/26 10:14:36 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#include <engine/rootmap.h>
#include <engine/input.h>
#include <engine/view.h>

#include <compat/math.h>

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02
#define DEBUG_CONTROL	0x04
#define DEBUG_MOVE	0x08
#define DEBUG_COLLISION	0x10
#define DEBUG_VELVEC	0x20
#define DEBUG_PROJECTION 0x40
int	position_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_CONTROL|DEBUG_MOVE|
			 DEBUG_COLLISION|DEBUG_VELVEC|DEBUG_PROJECTION;
#define engine_debug position_debug
#endif

void
position_init(struct position *pos)
{
	pos->map = NULL;
	pos->map_name = NULL;
	pos->x = 0;
	pos->y = 0;
	pos->z = 0;
	pos->projmap = NULL;
	pos->projmap_name = NULL;
	pos->input = NULL;
	pos->flags = 0;
	pos->dir = 0;
	pos->vel = 0;
}

void
position_destroy(struct position *pos)
{
	position_unproject(pos);
	Free(pos->map_name);
	Free(pos->projmap_name);
}

/* Assign position m:[x,y] to an object and select the projection map. */
int
position_set(void *p, struct map *m, int x, int y, int z, struct map *projmap)
{
	struct object *ob = p;
	struct position *pos = ob->pos;

	pthread_mutex_lock(&ob->lock);
	pthread_mutex_lock(&m->lock);
	if (x < 0 || x > m->mapw ||
	    y < 0 || y > m->maph ||
	    z < 0 || z > m->nlayers) {
		error_set(_("Illegal coordinates: %s[%d,%d,%d]"),
		    OBJECT(m)->name, x, y, z);
		goto fail;
	}
	
	if (pos == NULL) {
		debug(DEBUG_POSITION, "%s: creating position (proj=%s)\n",
		    ob->name, OBJECT(projmap)->name);
		pos = ob->pos = Malloc(sizeof(struct position));
		position_init(pos);
	} else {
		debug(DEBUG_POSITION, "%s: updating position\n", ob->name);
		position_unproject(pos);
	}
	

	debug(DEBUG_POSITION, "%s: position -> %s:[%d,%d,%d]\n", ob->name,
	    OBJECT(m)->name, x, y, z);
	pos->map = m;
	pos->map_name = Strdup(OBJECT(m)->name);
	pos->projmap = projmap;
	pos->projmap_name = Strdup(OBJECT(projmap)->name);
	pos->x = x;
	pos->y = y;
	pos->z = z;
	position_project(pos);
	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	position_unset(ob);
	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Unset an object's position, if there is one. */
void
position_unset(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	debug(DEBUG_POSITION, "%s: unset %p\n", ob->name, ob->pos);
	if (ob->pos != NULL) {
		position_destroy(ob->pos);
		free(ob->pos);
		ob->pos = NULL;
	}
	pthread_mutex_unlock(&ob->lock);
}

/* Control an object's position with an input device. */
int
position_set_input(void *p, const char *inname)
{
	struct object *ob = p;
	struct input *in;

	if ((in = input_find(inname)) == NULL) {
		error_set(_("There is no input device named `%s'."), inname);
		return (-1);
	}

	pthread_mutex_lock(&ob->lock);

	if (ob->pos == NULL) {
		error_set(_("Object `%s' has no position."), ob->name);
		goto fail;
	}
	debug(DEBUG_CONTROL, "%s: <%s>\n", ob->name, OBJECT(in)->name);
	ob->pos->input = in;

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Select the projection map of an object by name. */
int
position_set_projmap(void *p, const char *name)
{
	struct object *ob = p;
	struct object *projmap;

	pthread_mutex_lock(&ob->lock);
	
	debug(DEBUG_PROJECTION, "%s: %s -> %s\n", ob->name,
	    ob->pos != NULL ? OBJECT(ob->pos->projmap)->name : "none", name);

	TAILQ_FOREACH(projmap, &ob->children, cobjs) {
		if (strcmp(projmap->name, name) == 0)
			break;
	}
	if (projmap == NULL) {
		error_set(_("There is no projmap named `%s'."), name);
		goto fail;
	}
	if (ob->pos != NULL) {
		position_unproject(ob->pos);
	}
	ob->pos->projmap = (struct map *)projmap;
	ob->pos->projmap_name = Strdup(name);

	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Project the object onto a level map. */
void
position_project(struct position *pos)
{
	struct map *sm = pos->projmap;
	struct map *dm = pos->map;
	int sx, sy, dx, dy;

	object_add_dep(dm, sm);
	object_page_in(sm, OBJECT_DATA);
	object_page_in(dm, OBJECT_DATA);

	for (sy = 0, dy = pos->y;
	     sy < sm->maph && dy < pos->y+dm->maph;
	     sy++, dy++) {
		for (sx = 0, dx = pos->x;
		     sx < sm->mapw && dx < pos->x+dm->mapw;
		     sx++, dx++)
			node_copy(sm, &sm->map[sy][sx], -1, dm,
			    &dm->map[dy][dx], pos->z);
	}
	
	object_del_dep(dm, sm);
	object_page_out(sm, OBJECT_DATA);
	object_page_out(dm, OBJECT_DATA);
}

/* Remove a projection from the level map. */
void
position_unproject(struct position *pos)
{
	int x, y;
	
	object_page_in(pos->map, OBJECT_DATA);
	for (y = pos->y; y < pos->y+pos->projmap->maph; y++) {
		for (x = pos->x; x < pos->x+pos->projmap->mapw; x++)
			node_clear(pos->map, &pos->map->map[y][x], pos->z);
	}
	object_page_out(pos->map, OBJECT_DATA);
}

/*
 * Set the velocity vector for an object.
 * An object position must exist.
 */
void
position_set_velvec(void *p, int dir, int vel)
{
	struct object *ob = p;
	struct position *pos = ob->pos;

	pthread_mutex_lock(&ob->lock);
	if (dir >= 0) {
		debug(DEBUG_VELVEC, "%s: direction -> %d\n", ob->name, dir);
		pos->dir = (Uint8)dir;
	}
	if (vel >= 0) {
		debug(DEBUG_VELVEC, "%s: velocity -> %d\n", ob->name, vel);
		pos->vel = (Uint8)vel;
	}
	pthread_mutex_unlock(&ob->lock);
}

int
position_save(struct position *pos, struct netbuf *buf)
{
	char map_id[OBJECT_NAME_MAX];
	char projmap_id[OBJECT_NAME_MAX];

	object_copy_name(pos->map, map_id, sizeof(map_id));
	object_copy_name(pos->projmap, projmap_id, sizeof(projmap_id));

	write_string(buf, map_id);
	write_string(buf, projmap_id);
	write_string(buf, (pos->input != NULL) ? OBJECT(pos->input)->name : "");

	write_uint32(buf, (Uint32)pos->x);
	write_uint32(buf, (Uint32)pos->y);
	write_uint32(buf, (Uint32)pos->z);
	write_uint32(buf, (Uint32)pos->dir);
	write_uint32(buf, (Uint32)pos->vel);
	write_uint32(buf, (Uint32)pos->flags);

	return (0);
}

int
position_load(struct object *ob, struct position *pos, struct netbuf *buf)
{
	char map_name[OBJECT_PATH_MAX];
	char projmap_name[OBJECT_PATH_MAX];
	char input_name[OBJECT_NAME_MAX];
	struct map *map, *projmap;
	int x, y, z;
	int dir, vel, flags;

	if (ob->pos != NULL)
		position_unset(ob);

	copy_string(map_name, buf, sizeof(map_name));
	copy_string(projmap_name, buf, sizeof(projmap_name));
	copy_string(input_name, buf, sizeof(input_name));

	x = (int)read_uint32(buf);
	y = (int)read_uint32(buf);
	z = (int)read_uint32(buf);
	dir = (int)read_uint32(buf);
	vel = (int)read_uint32(buf);
	flags = (int)read_uint32(buf);
		
	debug(DEBUG_STATE,
	    "%s: at %s:[%d,%d,%d] proj %s dir %d vel %d flags 0x%x\n",
	    ob->name, map_name, x, y, z, projmap_name, dir, vel, flags);

	if ((map = object_find(map_name)) == NULL) {
		error_set(_("No such level map: `%s'"), map_name);
		return (-1);
	}
	if ((projmap = object_find(projmap_name)) == NULL) {
		error_set(_("No such projection map: `%s'"), projmap_name);
		return (-1);
	}

	pthread_mutex_lock(&map->lock);
	if (position_set(ob, map, x, y, z, projmap) == -1) {
		goto fail;
	}
	if (input_name != NULL &&
	    position_set_input(ob, input_name) == -1) {
		goto fail;
	}
	pthread_mutex_unlock(&map->lock);
	return (0);
fail:
	pthread_mutex_unlock(&map->lock);
	return (-1);
}

