/*	$Csoft: char.c,v 1.54 2002/06/13 09:07:42 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/char.h>
#include <engine/physics.h>
#include <engine/input.h>
#include <engine/version.h>

#include <engine/widget/text.h>

enum {
	DEFAULT_HP	= 10,
	DEFAULT_MP	= 10,
	DEFAULT_SPEED	= 5,
	DEFAULT_ZUARS	= 0
};

static const struct version char_ver = {
	"agar char",
	1, 1
};

static const struct object_ops char_ops = {
	char_destroy,	/* destroy */
	char_load,
	char_save
};

static Uint32	char_time(Uint32, void *);

struct character *
char_new(char *name, char *media)
{
	struct character *ch;

	ch = emalloc(sizeof(struct character));
	char_init(ch, name, media);

	pthread_mutex_lock(&world->lock);
	world_attach(world, ch);
	pthread_mutex_unlock(&world->lock);

	return (ch);
}

void
char_init(struct character *ch, char *name, char *media)
{
	struct ailment_alive *alive;

	/* XXX audio */
	object_init(&ch->obj, "character", name, media,
	    OBJECT_ART|OBJECT_BLOCK, &char_ops);

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

	TAILQ_INIT(&ch->ailments);

	alive = emalloc(sizeof(struct ailment_alive));
	ailment_init(&alive->ail, AILMENT_ALIVE, NULL, AILMENT_SAVE);
	alive->ticks = 0;
	TAILQ_INSERT_HEAD(&ch->ailments, AILMENT(alive), ailments);

	pthread_mutex_init(&ch->lock, NULL);

	event_new(ch, "attached", 0, char_attached, NULL);
	event_new(ch, "detached", 0, char_detached, NULL);
}

void
char_destroy(void *p)
{
	struct character *ch = p;
	struct ailment *ail, *nail;

	for (ail = TAILQ_FIRST(&ch->ailments);
	     ail != TAILQ_END(&ch->ailments);
	     ail = nail) {
		nail = TAILQ_NEXT(ail, ailments);
		free(ail);
	}
}

int
char_load(void *p, int fd)
{
	struct character *ch = p;
	struct input *input = NULL;
	struct map *m;

	if (version_read(fd, &char_ver) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&ch->lock);

	/* Read character properties. */
	free(fobj_read_string(fd));		/* Ignore name */
	ch->flags = fobj_read_uint32(fd);
	ch->level = fobj_read_uint32(fd);
	ch->exp = fobj_read_uint32(fd);
	ch->age = fobj_read_uint32(fd);
	ch->seed = fobj_read_uint32(fd);
	ch->maxspeed = fobj_read_uint32(fd);

	ch->maxhp = fobj_read_uint32(fd);
	ch->hp = fobj_read_uint32(fd);
	ch->maxmp = fobj_read_uint32(fd);
	ch->mp = fobj_read_uint32(fd);

	ch->nzuars = fobj_read_uint32(fd);

	dprintf("%s (0x%x) lvl=%d exp=%d age=%d hp=%d/%d mp=%d/%d\n",
	    OBJECT(ch)->name, ch->flags, ch->level, ch->exp, ch->age,
	    ch->hp, ch->maxhp, ch->mp, ch->maxhp);

	if (fobj_read_uint32(fd) > 0) {
		char *mname, *minput;
		Uint32 x, y, offs, flags, speed;

		mname = fobj_read_string(fd);
		x = fobj_read_uint32(fd);
		y = fobj_read_uint32(fd);
		offs = fobj_read_uint32(fd);
		flags = fobj_read_uint32(fd);
		speed = fobj_read_uint32(fd);
		minput = fobj_read_string(fd);

		/* XXX input devices should be linked */
		if (strcmp(minput, "keyboard0") == 0) {
			input = keyboard;
		} else if (strcmp(minput, "joy0") == 0) {
			input = joy;
		} else if (strcmp(minput, "mouse0") == 0) {
			input = mouse;
		}
		
		dprintf("%s is at %s:%d,%d[%d] (flags 0x%x, speed %d).\n",
		    OBJECT(ch)->name, mname, x, y, offs, flags, speed);

		m = (struct map *)object_strfind(mname);
		if (m != NULL) {
			struct mappos *npos;

			pthread_mutex_lock(&m->lock);
			npos = object_addpos(ch, offs, flags, input, m, x, y);
			npos->speed = speed;
			if (ch->flags & CHAR_FOCUS) {
				view_center(m, x, y);
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
char_save(void *p, int fd)
{
	struct character *ch = (struct character *)p;
	struct fobj_buf *buf;
	struct mappos *pos = OBJECT(ch)->pos;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	version_write(fd, &char_ver);
	
	pthread_mutex_lock(&ch->lock);

	/* Write character properties. */
	fobj_bwrite_string(buf, ch->obj.name);
	fobj_bwrite_uint32(buf, ch->flags & ~(CHAR_DONTSAVE));
	fobj_bwrite_uint32(buf, ch->level);
	fobj_bwrite_uint32(buf, ch->exp);
	fobj_bwrite_uint32(buf, ch->age);
	fobj_bwrite_uint32(buf, ch->seed);
	fobj_bwrite_uint32(buf, ch->maxspeed);

	fobj_bwrite_uint32(buf, ch->maxhp);
	fobj_bwrite_uint32(buf, ch->hp);
	fobj_bwrite_uint32(buf, ch->maxmp);
	fobj_bwrite_uint32(buf, ch->mp);
	
	fobj_bwrite_uint32(buf, ch->nzuars);

	if (pos != NULL) {
		fobj_bwrite_uint32(buf, 1);
		fobj_bwrite_string(buf, OBJECT(pos->map)->name);
		fobj_bwrite_uint32(buf, pos->x);
		fobj_bwrite_uint32(buf, pos->y);
		fobj_bwrite_uint32(buf, pos->nref->offs);
		fobj_bwrite_uint32(buf, pos->nref->flags);
		fobj_bwrite_uint32(buf, pos->speed);
		fobj_bwrite_string(buf,
		    (pos->input != NULL) ? OBJECT(pos->input)->name : "");
		dprintf("%s: reference %s:%d,%d offs=%d flags=0x%x\n",
		    OBJECT(ch)->name, OBJECT(pos->map)->name, pos->x, pos->y,
		    pos->nref->offs, pos->nref->flags);
	} else {
		fobj_bwrite_uint32(buf, 0);
	}
	pthread_mutex_unlock(&ch->lock);
	fobj_flush_buf(buf, fd);
	return (0);
}

void
char_attached(int argc, union evarg *argv)
{
	struct character *ch = argv[0].p;

	pthread_mutex_lock(&ch->lock);
	ch->timer = SDL_AddTimer(ch->maxspeed, char_time, ch);
	pthread_mutex_unlock(&ch->lock);
}

void
char_detached(int argc, union evarg *argv)
{
	struct character *ch = argv[0].p;

	pthread_mutex_lock(&ch->lock);
	SDL_RemoveTimer(ch->timer);
	pthread_mutex_unlock(&ch->lock);
}

static Uint32
char_time(Uint32 ival, void *p)
{
	struct object *ob = p;
	struct map *m;
	struct mappos *pos;
	Uint32 x, y, moved = 0;

	if (ob->pos == NULL) {
		return (ival);
	}

	/* Obtain the position. XXX array */
	pthread_mutex_lock(&ob->pos_lock);
	pos = ob->pos;
	pthread_mutex_unlock(&ob->pos_lock);

	m = pos->map;
	x = pos->x;
	y = pos->y;
	
	pthread_mutex_lock(&m->lock);

	/* Move the character if necessary. */
	moved = mapdir_move(&pos->dir, &x, &y);
	if (moved != 0) {
		struct mappos *newpos;

		newpos = object_movepos(ob, m, x, y);
		mapdir_postmove(&newpos->dir, &x, &y, moved);
	}

	pthread_mutex_unlock(&m->lock);

	return (ival);
}

