/*	$Csoft: char.c,v 1.62 2002/11/14 05:58:59 vedge Exp $	*/

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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>

#include <libfobj/fobj.h>
#include <libfobj/buf.h>

#include "engine.h"
#include "map.h"
#include "rootmap.h"
#include "perso.h"
#include "physics.h"
#include "input.h"
#include "version.h"

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
perso_new(char *name, char *media)
{
	struct perso *ch;

	ch = emalloc(sizeof(struct perso));
	perso_init(ch, name, media);

	world_attach(world, ch);
	return (ch);
}

void
perso_init(struct perso *ch, char *name, char *media)
{
	object_init(&ch->obj, "perso", name, media, OBJECT_ART|OBJECT_BLOCK,
	    &perso_ops);

	ch->flags = 0;
	ch->level = 0;
	ch->exp = 0;
	ch->age = 0;
	ch->seed = (Uint32)lrand48();

	ch->maxhp = DEFAULT_HP;
	ch->maxmp = DEFAULT_MP;
	ch->hp = ch->maxhp;
	ch->mp = ch->maxmp;
	ch->maxspeed = DEFAULT_SPEED;
	ch->nzuars = DEFAULT_ZUARS;

	pthread_mutex_init(&ch->lock, NULL);

	event_new(ch, "attached", perso_attached, NULL);
	event_new(ch, "detached", perso_detached, NULL);
}

int
perso_load(void *p, int fd)
{
	struct perso *ch = p;
	struct input *input = NULL;
	struct map *m;

	if (version_read(fd, &perso_ver) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&ch->lock);

	/* Read perso properties. */
	free(read_string(fd));		/* Ignore name */
	ch->flags = read_uint32(fd);
	ch->level = read_uint32(fd);
	ch->exp = read_uint32(fd);
	ch->age = read_uint32(fd);
	ch->seed = read_uint32(fd);
	ch->maxspeed = read_uint32(fd);

	ch->maxhp = read_uint32(fd);
	ch->hp = read_uint32(fd);
	ch->maxmp = read_uint32(fd);
	ch->mp = read_uint32(fd);

	ch->nzuars = read_uint32(fd);

	dprintf("%s (0x%x) lvl=%d exp=%d age=%d hp=%d/%d mp=%d/%d\n",
	    OBJECT(ch)->name, ch->flags, ch->level, ch->exp, ch->age,
	    ch->hp, ch->maxhp, ch->mp, ch->maxhp);

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
			    OBJECT(ch)->name, OBJECT(input)->name);
		}
		dprintf("%s is at %s:%d,%d[%d] (flags 0x%x, speed %d).\n",
		    OBJECT(ch)->name, mname, x, y, offs, flags, speed);

		m = (struct map *)world_find(mname);
		if (m != NULL) {
			struct mappos *npos;

			pthread_mutex_lock(&m->lock);
			npos = object_addpos(ch, offs, flags, input, m, x, y);
			npos->speed = speed;
			if (view->gfx_engine == GFX_ENGINE_TILEBASED &&
			    ch->flags & PERSO_FOCUSED) {
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
		dprintf("%s is nowhere.\n", OBJECT(ch)->name);
	}
	
	pthread_mutex_unlock(&ch->lock);
	
	return (0);
}

int
perso_save(void *p, int fd)
{
	struct perso *ch = (struct perso *)p;
	struct fobj_buf *buf;
	struct mappos *pos = OBJECT(ch)->pos;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	version_write(fd, &perso_ver);
	
	pthread_mutex_lock(&ch->lock);

	/* Write perso properties. */
	buf_write_string(buf, ch->obj.name);
	buf_write_uint32(buf, ch->flags & ~(PERSO_DONTSAVE));
	buf_write_uint32(buf, ch->level);
	buf_write_uint32(buf, ch->exp);
	buf_write_uint32(buf, ch->age);
	buf_write_uint32(buf, ch->seed);
	buf_write_uint32(buf, ch->maxspeed);

	buf_write_uint32(buf, ch->maxhp);
	buf_write_uint32(buf, ch->hp);
	buf_write_uint32(buf, ch->maxmp);
	buf_write_uint32(buf, ch->mp);
	
	buf_write_uint32(buf, ch->nzuars);

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
		    OBJECT(ch)->name, OBJECT(pos->map)->name, pos->x, pos->y,
		    pos->nref->offs, pos->nref->flags);
	} else {
		buf_write_uint32(buf, 0);
	}
	pthread_mutex_unlock(&ch->lock);
	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	return (0);
}

void
perso_attached(int argc, union evarg *argv)
{
	struct perso *ch = argv[0].p;

	pthread_mutex_lock(&ch->lock);
	ch->timer = SDL_AddTimer(ch->maxspeed, perso_time, ch);
	pthread_mutex_unlock(&ch->lock);
}

void
perso_detached(int argc, union evarg *argv)
{
	struct perso *ch = argv[0].p;

	pthread_mutex_lock(&ch->lock);
	SDL_RemoveTimer(ch->timer);
	pthread_mutex_unlock(&ch->lock);
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

