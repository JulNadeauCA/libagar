/*	$Csoft: char.c,v 1.43 2002/04/28 14:21:02 vedge Exp $	*/

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
	DEFAULT_SPEED	= 30,
	DEFAULT_ZUARS	= 0
};

static const struct version char_ver = {
	"agar char",
	1, 1
};

static const struct obvec char_vec = {
	NULL,		/* destroy */
	char_load,
	char_save,
	char_link,
	char_unlink
};

static Uint32	char_time(Uint32, void *);


void
char_init(struct character *ch, char *name, char *media)
{
	object_init(&ch->obj, name, media, OBJ_ART, &char_vec);

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
}

int
char_load(void *p, int fd)
{
	struct object *ob = (struct object *)p;
	struct character *ch = (struct character *)ob;
	struct input *input = NULL;
	struct map *m;

	if (version_read(fd, &char_ver) != 0) {
		return (-1);
	}

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
		    ob->name, mname, x, y, offs, flags, speed);

		m = (struct map *)object_strfind(mname);
		if (m != NULL) {
			struct node *node;
			struct mappos *npos;

			pthread_mutex_lock(&m->lock);
			node = &m->map[y][x];
			npos = object_addpos(ch, offs, flags, input, m, x, y);
			npos->speed = speed;
			pthread_mutex_unlock(&m->lock);

			if (ch->flags & CHAR_FOCUS) {
				view_center(m->view, x, y);
			}

			dprintf("at %s:%d,%d\n", OBJECT(m)->name, x, y);
			m->redraw++;
		} else {
			fatal("no such map: \"%s\"\n", mname);
		}
		free(mname);
		free(minput);
	} else {
		dprintf("%s is nowhere.\n", ob->name);
	}
	
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

	fobj_flush_buf(buf, fd);
	return (0);
}

int
char_link(void *ob)
{
	struct character *ch = (struct character *)ob;

	/* Assume world->lock is held */
	SLIST_INSERT_HEAD(&world->wcharsh, ch, wchars);

	ch->timer = SDL_AddTimer(ch->maxspeed, char_time, ch);

	return (0);
}

int
char_unlink(void *ob)
{
	struct character *ch = (struct character *)ob;

	SDL_RemoveTimer(ch->timer);

	/* Assume world->lock is held */
	SLIST_REMOVE(&world->wcharsh, ch, character, wchars);

	return (0);
}

static Uint32
char_time(Uint32 ival, void *p)
{
	struct object *ob = (struct object *)p;
	struct map *m;
	Uint32 x, y, moved = 0;

	if (ob->pos == NULL) {
		return (ival);
	}

	m = ob->pos->map;
	x = ob->pos->x;
	y = ob->pos->y;
	
	pthread_mutex_lock(&m->lock);

	moved = mapdir_move(&ob->pos->dir, &x, &y);
	if (moved != 0) {
		static struct mappos opos;
		struct mappos *npos;
		struct noderef *nref;
		
		opos = *ob->pos;
		nref = opos.nref;
	
		object_delpos(nref->pobj);
		npos = object_addpos(nref->pobj, nref->offs, nref->flags,
		    opos.input, m, x, y);
		npos->dir = opos.dir;

		m->redraw++;
		mapdir_postmove(&npos->dir, &x, &y, moved);
	}
	pthread_mutex_unlock(&m->lock);

	return (ival);
}

