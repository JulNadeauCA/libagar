/*	$Csoft: char.c,v 1.20 2002/02/25 11:31:30 vedge Exp $	*/

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
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
#include <engine/version.h>

static struct obvec char_vec = {
	char_destroy,
	char_load,
	char_save,
	char_link,
	char_unlink,
	char_dump
};

static Uint32	char_time(Uint32, void *);

struct character *
char_create(char *name, char *desc, Uint32 maxhp, Uint32 maxmp, Uint32 flags)
{
	struct character *ch;
	
	ch = (struct character *)emalloc(sizeof(struct character));
	object_init(&ch->obj, name, OBJ_EDITABLE, &char_vec);
	sprintf(ch->obj.desc, desc);

	ch->flags = 0;
	ch->level = 0;
	ch->exp = 0;
	ch->age = 0;
	ch->seed = (Uint32)lrand48();

	ch->maxhp = maxhp;
	ch->maxmp = maxmp;
	ch->hp = maxhp;
	ch->mp = maxmp;
	ch->maxspeed = 30;

	dprintf("%s: new: hp %d/%d mp %d/%d\n",
	    ch->obj.name, ch->hp, ch->maxhp, ch->mp, ch->maxmp);
	
	return (ch);
}

int
char_load(void *p, int fd)
{
	struct character *ch = (struct character *)p;
	char *mname;
	Uint32 i, ncoords;

	if (version_read(fd, "agar char", 1, 0) != 0) {
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

	dprintf("flags 0x%x level %d exp %d age %d hp %d/%d mp %d/%d",
	    ch->flags, ch->level, ch->exp, ch->age,
	    ch->hp, ch->maxhp, ch->mp, ch->maxhp);

	/* Read single map coordinates. */
	ncoords = fobj_read_uint32(fd);
	for (i = 0; i < ncoords; i++) {
		struct map *m;
		Uint32 x, y, offs, flags, speed;
		
		mname = fobj_read_string(fd);
		x = fobj_read_uint32(fd);
		y = fobj_read_uint32(fd);
		offs = fobj_read_uint32(fd);
		flags = fobj_read_uint32(fd);
		speed = fobj_read_uint32(fd);

		m = (struct map *)object_strfind(mname);
		if (m != NULL) {
			struct node *node;
			struct map_bref *bref;

			pthread_mutex_lock(&m->lock);
			node = &m->map[x][y];
			bref = object_madd(ch, offs, flags, m, x, y);
			bref->speed = speed;
			pthread_mutex_unlock(&m->lock);

			dprintf("at %s:%d,%d\n", m->obj.name, x, y);
		} else {
			fatal("no such map: \"%s\"\n", mname);
		}
		free(mname);
	}
	dprintf("%d coordinates\n", ncoords);
	
	return (0);
}

int
char_save(void *p, int fd)
{
	struct character *ch = (struct character *)p;
	struct fobj_buf *buf;
	struct map_bref *bref;
	size_t soffs;
	Uint32 count;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	version_write(fd, "agar char", 1, 0);

	/* Write character properties. */
	fobj_bwrite_string(buf, ch->obj.name);
	fobj_bwrite_uint32(buf, ch->flags &= ~(CHAR_DONTSAVE));
	fobj_bwrite_uint32(buf, ch->level);
	fobj_bwrite_uint32(buf, ch->exp);
	fobj_bwrite_uint32(buf, ch->age);
	fobj_bwrite_uint32(buf, ch->seed);
	fobj_bwrite_uint32(buf, ch->maxspeed);

	fobj_bwrite_uint32(buf, ch->maxhp);
	fobj_bwrite_uint32(buf, ch->hp);
	fobj_bwrite_uint32(buf, ch->maxmp);
	fobj_bwrite_uint32(buf, ch->mp);

	/* Write map back references */
	soffs = buf->offs;
	fobj_bwrite_uint32(buf, 0);
	count = 0;
	SLIST_FOREACH(bref, &ch->obj.brefsh, brefs) {
		dprintf("%s: reference %s:%d,%d offs=%d flags=0x%x\n",
		    ch->obj.name, bref->map->obj.name, bref->x, bref->y,
		    bref->nref->offs, bref->nref->flags);
		fobj_bwrite_string(buf, bref->map->obj.name);
		fobj_bwrite_uint32(buf, bref->x);
		fobj_bwrite_uint32(buf, bref->y);
		fobj_bwrite_uint32(buf, bref->nref->offs);
		fobj_bwrite_uint32(buf, bref->nref->flags);
		fobj_bwrite_uint32(buf, bref->speed);
		count++;
	}
	fobj_bpwrite_uint32(buf, count, soffs);
	dprintf("%s: %d back references\n", ch->obj.name, count);

	fobj_flush_buf(buf, fd);
	return (0);
}

int
char_link(void *ob)
{
	struct character *ch = (struct character *)ob;

	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_INSERT_HEAD(&world->wcharsh, ch, wchars);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}

	/* XXX speed */
	ch->timer = SDL_AddTimer(ch->maxspeed / 2, char_time, ch);
	if (ch->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		return (-1);
	}

	return (0);
}

int
char_unlink(void *ob)
{
	struct character *ch = (struct character *)ob;

	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_REMOVE(&world->wcharsh, ch, character, wchars);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}

	if (ch->timer != NULL) {
		SDL_RemoveTimer(ch->timer);
		ch->timer = NULL;
	}

	return (0);
}

int
char_destroy(void *p)
{
	struct character *ch = (struct character *)p;

	SDL_RemoveTimer(ch->timer);

	pthread_mutex_lock(&world->lock);
	SLIST_REMOVE(&world->wcharsh, ch, character, wchars);
	pthread_mutex_unlock(&world->lock);

	return (0);
}

void
char_setspeed(struct character *ch, Uint32 speed)
{
#if 1
	dprintf("not yet\n");
#else
	ch->curspeed = speed;

	if (ch->curspeed >= ch->maxspeed) {
		ch->curspeed = ch->maxspeed;
	} else if (ch->curspeed <= 0) {
		ch->curspeed = 1;
	}
	if (SDL_RemoveTimer(ch->timer)) {
		ch->timer = SDL_AddTimer(ch->maxspeed - ch->curspeed,
		    char_time, ch);
	}
#endif
}

/* XXX none of this is character-dependent */
static Uint32
char_time(Uint32 ival, void *obp)
{
	struct object *ob = (struct object *)obp;
	struct map_bref *bref;
	Uint32 moved = 0;
	Uint32 x, y;

	SLIST_FOREACH(bref, &ob->brefsh, brefs) {
		x = bref->x;
		y = bref->y;

		/* Move a character. */
		moved = mapdir_move(&bref->dir, &x, &y);
		if (moved != 0) {
			struct mapdir dir;
			struct map_bref *nbref;

			pthread_mutex_lock(&bref->map->lock);

			dir = bref->dir;	/* Structure copy */

			/* Remove old back reference. */
			object_mdel(ob, 0, MAPREF_SPRITE,
			    bref->map, bref->x, bref->y);

			/* Insert new back reference. */
			nbref = object_madd(ob, 0, MAPREF_SPRITE,
			    bref->map, bref->x, bref->y);

			nbref->dir = dir;	/* Structure copy */

			/* Ensure movement continuation, if needed. */
			mapdir_postmove(&nbref->dir, &x, &y, moved);

			pthread_mutex_unlock(&bref->map->lock);

			bref->map->redraw++;	/* XXX waste */
		}
	}

	return (ival);
}

void
char_dump(void *p)
{
	struct character *ch = (struct character *)p;
	
	printf("lvl %d (exp %d) hp %d/%d mp %d/%d\n",
	    ch->level, ch->exp, ch->hp, ch->maxhp,
	    ch->mp, ch->maxmp);
}

