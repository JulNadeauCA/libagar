/*	$Csoft: perso.c,v 1.17 2003/03/12 07:59:00 vedge Exp $	*/

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

#include "compat/vasprintf.h"
#include "engine.h"

#include <libfobj/fobj.h>

#include "map.h"
#include "rootmap.h"
#include "perso.h"
#include "physics.h"
#include "input.h"
#include "version.h"
#include "view.h"
#include "world.h"

#include "widget/widget.h"
#include "widget/window.h"
#include "widget/label.h"
#include "widget/button.h"

enum {
	DEFAULT_HP	= 10,
	DEFAULT_MP	= 10,
	DEFAULT_SPEED	= 16,
	DEFAULT_ZUARS	= 0
};

static const struct version perso_ver = {
	"agar personage",
	2, 0
};

static const struct object_ops perso_ops = {
	perso_destroy,
	perso_load,
	perso_save
};

static Uint32	perso_time(Uint32, void *);

#ifdef DEBUG
#define	DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02

int	perso_debug = DEBUG_STATE|DEBUG_POSITION;
#define engine_debug	perso_debug
#endif

struct perso *
perso_new(char *name, char *media, Uint32 hp, Uint32 mp)
{
	struct perso *pers;

	pers = emalloc(sizeof(struct perso));
	perso_init(pers, name, media, hp, mp);

	world_attach(pers);
	return (pers);
}

void
perso_init(struct perso *pers, char *name, char *media, Uint32 hp, Uint32 mp)
{
	object_init(&pers->obj, "perso", name, media, OBJECT_ART, &perso_ops);

	pers->name = Strdup(name);
	pers->flags = 0;
	pers->level = 0;
	pers->exp = 0;
	pers->age = 0;
	pers->seed = (Uint32)lrand48();

	pers->hp = pers->maxhp = hp;
	pers->mp = pers->maxmp = mp;
	pers->nzuars = DEFAULT_ZUARS;

	pthread_mutex_init(&pers->lock, NULL);

	event_new(pers, "attached", perso_attached, NULL);
	event_new(pers, "detached", perso_detached, NULL);
}

void
perso_destroy(void *p)
{
	struct perso *perso = p;

	free(perso->name);
}

int
perso_load(void *p, int fd)
{
	struct perso *perso = p;
	struct input *input = NULL;

	if (version_read(fd, &perso_ver, NULL) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&perso->lock);

	/* Read the character status. */
	/* XXX use generic properties? */
	perso->name = read_string(fd, perso->name);
	perso->flags = read_uint32(fd);
	perso->level = read_uint32(fd);
	perso->exp = read_uint32(fd);
	perso->age = read_uint32(fd);
	perso->seed = read_uint32(fd);
	perso->maxhp = read_uint32(fd);
	perso->hp = read_uint32(fd);
	perso->maxmp = read_uint32(fd);
	perso->mp = read_uint32(fd);
	perso->nzuars = read_uint32(fd);

	debug(DEBUG_STATE, "%s (0x%x) lvl=%d exp=%d age=%d hp=%d/%d mp=%d/%d\n",
	    OBJECT(perso)->name, perso->flags, perso->level, perso->exp,
	    perso->age, perso->hp, perso->maxhp, perso->mp, perso->maxhp);

	/* Save positions. */
	if (read_uint32(fd) == 0) {			/* No position */
		debug(DEBUG_STATE, "%s is nowhere.\n",
		    OBJECT(perso)->name);
	} else {					/* One position */
		char *map_id, *input_id;
		int dst_x, dst_y;
		struct noderef *old_nref;
		struct map *dst_map;
		struct object_table *deps;

		/*
		 * Read the dependency table. This is used by noderef_load()
		 * to resolve object identifiers and allows the character to
		 * use sprite/animations from other objects.
		 */
		deps = object_table_load(fd, OBJECT(perso)->name);

		/* Read the map id, node coordinates and input device id. */
		map_id = read_string(fd, NULL);
		dst_x = (int)read_uint32(fd);
		dst_y = (int)read_uint32(fd);
		input_id = read_string(fd, NULL);
		debug(DEBUG_STATE, "%s is at %s:%d,%d, controlled by %s\n",
		    OBJECT(perso)->name, map_id, dst_x, dst_y, input_id);
		dst_map = world_find(map_id);
		if (dst_map == NULL) {
			fatal("no such map: `%s'", map_id);
		}

		pthread_mutex_lock(&dst_map->lock);

		if (dst_x > dst_map->mapw || dst_y > dst_map->maph) {
			fatal("%d,%d exceeds map boundaries", dst_x, dst_y);
		}

		/*
		 * Load the previously saved noderef directly on the new node,
		 * and set the old_nref pointer.
		 */
		noderef_load(fd, deps, &dst_map->map[dst_y][dst_x], &old_nref);

		/*
		 * Update the character's position (back reference).
		 * This removes the current noderef, if there is any.
		 */
		object_set_position(perso, old_nref, dst_map, dst_x, dst_y);

		dst_map->redraw++;			/* Noderefs changed */

		pthread_mutex_unlock(&dst_map->lock);
		
		/*
		 * Resolve the input device id and control the object.
		 * Center the view if the character is focused.
		 */
		if (strcmp(input_id, "") != 0) {
			input = input_find(input_id);
			if (input == NULL) {
				/* XXX assign a default input device or none. */
				fatal("no such input device: `%s'", input_id);
			}
			debug(DEBUG_STATE, "%s: controlling with %s\n",
			    OBJECT(perso)->name, OBJECT(input)->name);

			object_control(perso, input,
			    perso->flags & PERSO_FOCUSED);
		} else {
			debug(DEBUG_STATE, "%s: not controlled\n",
			    OBJECT(perso)->name);
		}

		object_table_destroy(deps);
		free(map_id);
		free(input_id);
	}

	pthread_mutex_unlock(&perso->lock);
	return (0);
}

int
perso_save(void *p, int fd)
{
	struct perso *perso = p;
	struct fobj_buf *buf;
	struct mappos *pos;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	version_write(fd, &perso_ver);
	
	pthread_mutex_lock(&perso->lock);

	/* Save the character status. */
	/* XXX use generic properties? */
	buf_write_string(buf, perso->name);
	buf_write_uint32(buf, perso->flags & ~(PERSO_EPHEMERAL));
	buf_write_uint32(buf, perso->level);
	buf_write_uint32(buf, perso->exp);
	buf_write_uint32(buf, perso->age);
	buf_write_uint32(buf, perso->seed);
	buf_write_uint32(buf, perso->maxhp);
	buf_write_uint32(buf, perso->hp);
	buf_write_uint32(buf, perso->maxmp);
	buf_write_uint32(buf, perso->mp);
	buf_write_uint32(buf, perso->nzuars);
	
	pthread_mutex_lock(&OBJECT(perso)->pos_lock);
	pos = OBJECT(perso)->pos;

	/* Save positions. */
	if (pos == NULL) {
		debug(DEBUG_STATE, "%s: no position\n", OBJECT(perso)->name);
		buf_write_uint32(buf, 0);		/* No position */
	} else {
		struct object_table *deps;
		struct object *pob;
		
		buf_write_uint32(buf, 1);		/* One position */

		/*
		 * Generate the dependency table. This is used by noderef_save()
		 * to encode object identifiers and allows the character to
		 * use sprite/animations from other objects.
		 */
		deps = object_table_new();

		pthread_mutex_lock(&world->lock);
		SLIST_FOREACH(pob, &world->wobjs, wobjs) {
			debug_n(DEBUG_STATE, "%s: %s dependency ",
			    OBJECT(perso)->name, pob->name);
			if ((pob->flags & OBJECT_ART) == 0 ||
			     pob->flags & OBJECT_CANNOT_MAP) {
			     	debug_n(DEBUG_STATE, "skipped\n");
				continue;
			} 
			debug_n(DEBUG_STATE, "registered\n");
			object_table_insert(deps, pob);
		}
		pthread_mutex_unlock(&world->lock);

		/* Write the dependencies. */
		object_table_save(buf, deps);

		/* Save the map id, node coordinates and input device id. */
		buf_write_string(buf, OBJECT(pos->map)->name);
		buf_write_uint32(buf, pos->x);
		buf_write_uint32(buf, pos->y);
		/* XXX the same input device may not be available later. */
		buf_write_string(buf, (pos->input != NULL) ?
		    OBJECT(pos->input)->name : "");

		/* Save the current noderef in its native format. */
		pthread_mutex_lock(&pos->map->lock);
		noderef_save(buf, deps, pos->nref);
		pthread_mutex_unlock(&pos->map->lock);

		object_table_destroy(deps);
	}
	pthread_mutex_unlock(&OBJECT(perso)->pos_lock);

	pthread_mutex_unlock(&perso->lock);

	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	return (0);
}

void
perso_attached(int argc, union evarg *argv)
{
	struct perso *pers = argv[0].p;

	pthread_mutex_lock(&pers->lock);
	pers->timer = SDL_AddTimer(30, perso_time, pers);
	pthread_mutex_unlock(&pers->lock);
}

void
perso_detached(int argc, union evarg *argv)
{
	struct perso *pers = argv[0].p;

	pthread_mutex_lock(&pers->lock);
	SDL_RemoveTimer(pers->timer);
	pthread_mutex_unlock(&pers->lock);
}

static Uint32
perso_time(Uint32 ival, void *p)
{
	struct object *ob = p;
	struct mappos *pos;
	int moved = 0;

	/* XXX thread unsafe */

	pthread_mutex_lock(&ob->pos_lock);

	pos = ob->pos;
	if (pos == NULL) {
		pthread_mutex_unlock(&ob->pos_lock);
		debug(DEBUG_POSITION, "%s is nowhere!\n", ob->name);
		return (ival);
	}
	pthread_mutex_lock(&pos->map->lock);
	moved = mapdir_move(&pos->dir, &pos->x, &pos->y);
	if (moved != 0) {
		object_move(ob, pos->map, pos->x, pos->y);
		mapdir_postmove(&ob->pos->dir, &pos->x, &pos->y, moved);
	}
	pthread_mutex_unlock(&pos->map->lock);

	pthread_mutex_unlock(&ob->pos_lock);
	return (ival);
}

void
perso_say(struct perso *pers, const char *fmt, ...)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *button;
	va_list args;
	char *msg;

	win = window_generic_new(253, 140, NULL);
	if (win == NULL) {
		return;
	}
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	
	va_start(args, fmt);
	Vasprintf(&msg, fmt, args);
	va_end(args);

	lab = label_new(reg, 100, 60, msg);
	button = button_new(reg, "Ok", NULL, 0, 99, 40);
	WIDGET_FOCUS(button);

	event_new(button, "button-pushed", window_generic_detach, "%p", win);
	window_show(win);

	free(msg);
}

