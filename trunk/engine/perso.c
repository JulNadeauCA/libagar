/*	$Csoft: perso.c,v 1.3 2002/11/22 08:56:49 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include "engine.h"
#include "compat/vasprintf.h"

#include <libfobj/fobj.h>
#include <libfobj/buf.h>

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
	DEFAULT_SPEED	= 5,
	DEFAULT_ZUARS	= 0
};

static const struct version perso_ver = {
	"agar personage",
	1, 0
};

static const struct object_ops perso_ops = {
	NULL,		/* destroy */
	perso_load,
	perso_save
};

static Uint32	perso_time(Uint32, void *);

struct perso *
perso_new(char *name, char *media, Uint32 hp, Uint32 mp)
{
	struct perso *pers;

	pers = emalloc(sizeof(struct perso));
	perso_init(pers, name, media, hp, mp);

	world_attach(world, pers);
	return (pers);
}

void
perso_init(struct perso *pers, char *name, char *media, Uint32 hp, Uint32 mp)
{
	object_init(&pers->obj, "perso", name, media, OBJECT_ART|OBJECT_BLOCK,
	    &perso_ops);

	pers->flags = 0;
	pers->level = 0;
	pers->exp = 0;
	pers->age = 0;
	pers->seed = (Uint32)lrand48();

	pers->hp = pers->maxhp = hp;
	pers->mp = pers->maxmp = mp;
	pers->maxspeed = DEFAULT_SPEED;
	pers->nzuars = DEFAULT_ZUARS;

	pthread_mutex_init(&pers->lock, NULL);

	event_new(pers, "attached", perso_attached, NULL);
	event_new(pers, "detached", perso_detached, NULL);
}

int
perso_load(void *p, int fd)
{
	struct perso *pers = p;
	struct input *input = NULL;
	struct map *m;

	if (version_read(fd, &perso_ver) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&pers->lock);

	/* Read perso properties. */
	free(read_string(fd));		/* Ignore name */
	pers->flags = read_uint32(fd);
	pers->level = read_uint32(fd);
	pers->exp = read_uint32(fd);
	pers->age = read_uint32(fd);
	pers->seed = read_uint32(fd);
	pers->maxspeed = read_uint32(fd);

	pers->maxhp = read_uint32(fd);
	pers->hp = read_uint32(fd);
	pers->maxmp = read_uint32(fd);
	pers->mp = read_uint32(fd);

	pers->nzuars = read_uint32(fd);

	dprintf("%s (0x%x) lvl=%d exp=%d age=%d hp=%d/%d mp=%d/%d\n",
	    OBJECT(pers)->name, pers->flags, pers->level, pers->exp, pers->age,
	    pers->hp, pers->maxhp, pers->mp, pers->maxhp);

	if (read_uint32(fd) > 0) {
		char *mname, *minput;
		Uint32 x, y, offs, flags, speed;

		mname = read_string(fd);
		x = read_uint32(fd);
		y = read_uint32(fd);
		offs = read_uint32(fd);
		flags = read_uint32(fd);
		speed = read_uint32(fd);
		minput = read_string(fd);

		input = input_find_str(minput);
		if (input != NULL) {
			dprintf("%s is controlled by %s\n",
			    OBJECT(pers)->name, OBJECT(input)->name);
		}
		dprintf("%s is at %s:%d,%d[%d] (flags 0x%x, speed %d).\n",
		    OBJECT(pers)->name, mname, x, y, offs, flags, speed);

		m = (struct map *)world_find(mname);
		if (m != NULL) {
			struct mappos *npos;

			pthread_mutex_lock(&m->lock);
			npos = object_addpos(pers, offs, flags, input, m, x, y);
			npos->speed = speed;
			if (view->gfx_engine == GFX_ENGINE_TILEBASED &&
			    pers->flags & PERSO_FOCUSED) {
				rootmap_center(m, x, y);
			}
			pthread_mutex_unlock(&m->lock);

			dprintf("at %s:%d,%d\n", OBJECT(m)->name, x, y);
			m->redraw++;
		} else {
			fatal("no such map: \"%s\"\n", mname);
		}
		free(mname);
		free(minput);
	} else {
		dprintf("%s is nowhere.\n", OBJECT(pers)->name);
	}
	
	pthread_mutex_unlock(&pers->lock);
	
	return (0);
}

int
perso_save(void *p, int fd)
{
	struct perso *pers = p;
	struct fobj_buf *buf;
	struct mappos *pos = OBJECT(pers)->pos;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	version_write(fd, &perso_ver);
	
	pthread_mutex_lock(&pers->lock);

	/* Write perso properties. */
	buf_write_string(buf, pers->obj.name);
	buf_write_uint32(buf, pers->flags & ~(PERSO_DONTSAVE));
	buf_write_uint32(buf, pers->level);
	buf_write_uint32(buf, pers->exp);
	buf_write_uint32(buf, pers->age);
	buf_write_uint32(buf, pers->seed);
	buf_write_uint32(buf, pers->maxspeed);

	buf_write_uint32(buf, pers->maxhp);
	buf_write_uint32(buf, pers->hp);
	buf_write_uint32(buf, pers->maxmp);
	buf_write_uint32(buf, pers->mp);
	
	buf_write_uint32(buf, pers->nzuars);

	if (pos != NULL) {
		buf_write_uint32(buf, 1);
		buf_write_string(buf, OBJECT(pos->map)->name);
		buf_write_uint32(buf, pos->x);
		buf_write_uint32(buf, pos->y);
		buf_write_uint32(buf, pos->nref->offs);
		buf_write_uint32(buf, pos->nref->flags);
		buf_write_uint32(buf, pos->speed);
		buf_write_string(buf,
		    (pos->input != NULL) ? OBJECT(pos->input)->name : "");
		dprintf("%s: reference %s:%d,%d offs=%d flags=0x%x\n",
		    OBJECT(pers)->name, OBJECT(pos->map)->name, pos->x, pos->y,
		    pos->nref->offs, pos->nref->flags);
	} else {
		buf_write_uint32(buf, 0);
	}
	pthread_mutex_unlock(&pers->lock);
	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	return (0);
}

void
perso_attached(int argc, union evarg *argv)
{
	struct perso *pers = argv[0].p;

	pthread_mutex_lock(&pers->lock);
	pers->timer = SDL_AddTimer(pers->maxspeed, perso_time, pers);
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
	struct map *m;
	struct mappos *pos;
	Uint32 x, y, moved = 0;

	pos = object_get_pos(ob);

	if (pos == NULL) {
		/* Character is nowhere. */
		return (ival);
	}

	m = pos->map;
	x = pos->x;
	y = pos->y;
	
	pthread_mutex_lock(&m->lock);
	moved = mapdir_move(&pos->dir, &x, &y);
	if (moved != 0) {
		struct mappos *newpos;

		newpos = object_movepos(ob, m, x, y);
		mapdir_postmove(&newpos->dir, &x, &y, moved);
	}
	pthread_mutex_unlock(&m->lock);

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
	if (vasprintf(&msg, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);

	lab = label_new(reg, 100, 60, msg);
	button = button_new(reg, "Ok", NULL, 0, 99, 40);
	WIDGET_FOCUS(button);

	event_new(button, "button-pushed", window_generic_detach, "%p", win);
	window_show(win);

	free(msg);
}

