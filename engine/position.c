/*	$Csoft: position.c,v 1.6 2004/03/18 21:27:47 vedge Exp $	*/

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

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02
#define DEBUG_INPUT	0x04
#define DEBUG_MOVE	0x08
#define DEBUG_COLLISION	0x10
#define DEBUG_VELVEC	0x20
#define DEBUG_PROJMAP	0x40
int	position_debug = DEBUG_STATE|DEBUG_POSITION|DEBUG_INPUT|DEBUG_MOVE|
			 DEBUG_COLLISION|DEBUG_VELVEC|DEBUG_PROJMAP;
#define engine_debug position_debug
#endif

void
position_init(struct position *pos)
{
	pos->map = NULL;
	pos->map_name[0] = '\0';
	pos->x = 0;
	pos->y = 0;
	pos->z = 0;
	pos->projmap = NULL;
	pos->projmap_name[0] = '\0';
	pos->input = NULL;
	pos->flags = 0;
	pos->dir = 0;
	pos->vel = 0;
}

/* Assign a position m[x,y,z] to a given object. */
int
position_set(void *p, struct map *m, int x, int y, int z)
{
	char path[OBJECT_PATH_MAX];
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	pthread_mutex_lock(&m->lock);

	if (x < 0 || x > m->mapw ||
	    y < 0 || y > m->maph ||
	    z < 0 || z > m->nlayers) {
		error_set(_("Illegal coordinates: %s[%d,%d,%d]"),
		    OBJECT(m)->name, x, y, z);
		goto fail;
	}
	
	if (ob->pos == NULL) {
		debug(DEBUG_POSITION, "%s: creating position\n", ob->name);
		ob->pos = Malloc(sizeof(struct position), M_POSITION);
		position_init(ob->pos);
	} else {
		debug(DEBUG_POSITION, "%s: updating position\n", ob->name);
		position_unproject(ob->pos);
	}
	
	ob->pos->map = m;
	object_copy_name(m, ob->pos->map_name, sizeof(ob->pos->map_name));

	ob->pos->x = x;
	ob->pos->y = y;
	ob->pos->z = z;

	debug(DEBUG_STATE,
	    "%s: at %s:[%d,%d,%d] projmap `%s' dir %d vel %d flags 0x%x\n",
	    ob->name, ob->pos->map_name, ob->pos->x, ob->pos->y, ob->pos->z,
	    ob->pos->projmap_name, ob->pos->dir, ob->pos->vel, ob->pos->flags);

	position_project(ob->pos);

	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&m->lock);
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

/* Destroy the current position of an object if there is one. */
void
position_unset(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	if (ob->pos != NULL) {
		dprintf("%s: destroying position\n", ob->name);
		position_unproject(ob->pos);
		Free(ob->pos, M_POSITION);
		ob->pos = NULL;
	} else {
		dprintf("%s: no position\n", ob->name);
	}
	pthread_mutex_unlock(&ob->lock);
}

/*
 * Control an object's position with an input device.
 * A position must exist.
 */
void
position_set_input(void *p, struct input *indev)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	if (indev != NULL) {
		debug(DEBUG_INPUT, "%s: using %s\n", ob->name,
		    OBJECT(indev)->name);
	}
	ob->pos->input = indev;
	pthread_mutex_unlock(&ob->lock);
}

/*
 * Select the projection map of an object. If one is set, remove its projection.
 * A position must exist.
 */
void
position_set_projmap(void *p, struct map *projmap)
{
	char path[OBJECT_PATH_MAX];
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	position_unproject(ob->pos);

	ob->pos->projmap = projmap;
	object_copy_name(projmap, ob->pos->projmap_name,
	    sizeof(ob->pos->projmap_name));

	position_project(ob->pos);
	pthread_mutex_unlock(&ob->lock);
}

/* Copy the object's projection map onto a level map. */
void
position_project(struct position *pos)
{
	struct map *sm = pos->projmap;
	struct map *dm = pos->map;
	int sx, sy, dx, dy;
	
	if (sm == NULL || dm == NULL)
		return;

	object_page_in(sm, OBJECT_DATA);
	object_page_in(dm, OBJECT_DATA);

	for (sy = 0, dy = pos->y;
	     sy < sm->maph && dy < dm->maph;
	     sy++, dy++) {
		for (sx = 0, dx = pos->x;
		     sx < sm->mapw && dx < dm->mapw;
		     sx++, dx++) {
			struct node *dn = &dm->map[dy][dx];

			node_copy(sm, &sm->map[sy][sx], -1, dm, dn, pos->z);
		}
	}
	
	object_page_out(sm, OBJECT_DATA);
	object_page_out(dm, OBJECT_DATA);
}

/* Remove an object's projection from its level map. */
void
position_unproject(struct position *pos)
{
	int x, y;

	if (pos->map == NULL || pos->projmap == NULL)
		return;

	object_page_in(pos->map, OBJECT_DATA);
	object_page_in(pos->projmap, OBJECT_DATA);
	for (y = 0;
	     y < pos->projmap->maph && pos->y+y < pos->map->maph;
	     y++) {
		for (x = 0;
		     x < pos->projmap->mapw && pos->x+x < pos->map->mapw;
		     x++) {
			node_clear(pos->map, &pos->map->map[pos->y+y][pos->x+x],
			    pos->z);
		}
	}
	object_page_out(pos->map, OBJECT_DATA);
	object_page_out(pos->projmap, OBJECT_DATA);
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

	position_unproject(ob->pos);
	position_project(ob->pos);

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
position_load(struct object *ob, struct netbuf *buf)
{
	char input_name[OBJECT_NAME_MAX];

	dprintf("%s\n", ob->name);

	if (ob->pos == NULL) {
		debug(DEBUG_POSITION, "%s: creating position\n", ob->name);
		ob->pos = Malloc(sizeof(struct position), M_POSITION);
		position_init(ob->pos);
	} else {
		debug(DEBUG_POSITION, "%s: updating position\n", ob->name);
	}

	copy_string(ob->pos->map_name, buf, sizeof(ob->pos->map_name));
	copy_string(ob->pos->projmap_name, buf, sizeof(ob->pos->projmap_name));
	copy_string(input_name, buf, sizeof(input_name));

	ob->pos->x = (int)read_uint32(buf);
	ob->pos->y = (int)read_uint32(buf);
	ob->pos->z = (int)read_uint32(buf);
	ob->pos->dir = (int)read_uint32(buf);
	ob->pos->vel = (int)read_uint32(buf);
	ob->pos->flags = (int)read_uint32(buf);
		
	debug(DEBUG_STATE,
	    "%s: at %s:[%d,%d,%d] projmap `%s' dir %d vel %d flags 0x%x\n",
	    ob->name, ob->pos->map_name, ob->pos->x, ob->pos->y, ob->pos->z,
	    ob->pos->projmap_name, ob->pos->dir, ob->pos->vel, ob->pos->flags);

	if (input_name[0] != '\0') {
		struct input *indev;

		if ((indev = input_find(input_name)) == NULL) {
			goto fail;
		}
		position_set_input(ob, indev);
	}
	return (0);
fail:
	position_unset(ob);
	return (-1);
}

