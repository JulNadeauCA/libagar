/*	$Csoft: char.c,v 1.16 2002/02/15 05:38:02 vedge Exp $	*/

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

static struct obvec char_vec = {
	char_destroy,
	char_event,
	char_load,
	char_save,
	char_link,
	char_unlink
};

static Uint32	char_time(Uint32, void *);
static void	char_joybutton(struct character *, SDL_Event *);
static void	char_key(struct character *, SDL_Event *);

struct character *curchar;

struct character *
char_create(char *name, char *desc, int maxhp, int maxmp, int flags)
{
	struct character *ch;
	
	ch = (struct character *)emalloc(sizeof(struct character));
	object_init(&ch->obj, name, flags, &char_vec);
	sprintf(ch->obj.desc, desc);

	ch->flags = 0;
	ch->level = 0;
	ch->exp = 0;
	ch->age = 0;
	ch->seed = lrand48();

	ch->maxhp = maxhp;
	ch->maxmp = maxmp;
	ch->hp = maxhp;
	ch->mp = maxmp;

	ch->map = NULL;
	mapdir_init(&ch->dir, (struct object *)ch, NULL, 0, -1);
	ch->x = -1;
	ch->y = -1;

	ch->curoffs = CHAR_UP;
	ch->curspeed = -1;
	ch->maxspeed = -1;

	dprintf("%s: new: hp %d/%d mp %d/%d\n",
	    ch->obj.name, ch->hp, ch->maxhp, ch->mp, ch->maxmp);
	
	return (ch);
}

int
char_load(void *p, int fd)
{
	struct character *ch = (struct character *)p;
	char magic[11];
	int vermin, vermaj;

	if ((read(fd, magic, 11) != 11) ||
	     strcmp(magic, "agar char ") != 0) {
		goto badmagic;
	}

	vermin = fobj_read_uint32(fd);
	vermaj = fobj_read_uint32(fd);
	if (vermaj > CHAR_VERMAJ ||
	   (vermaj == CHAR_VERMAJ && vermin > CHAR_VERMIN)) {
		fatal("%s: version %d.%d > %d.%d\n", ch->obj.name,
		    vermaj, vermin, CHAR_VERMAJ, CHAR_VERMIN);
		return (-1);
	}

	/* Read character properties. */
	free(fobj_read_string(fd));		/* Ignore name */
	ch->flags = fobj_read_uint32(fd);
	ch->level = fobj_read_uint32(fd);
	ch->exp = fobj_read_uint32(fd);
	ch->age = fobj_read_uint32(fd);
	ch->seed = fobj_read_uint64(fd);

	ch->maxhp = fobj_read_uint32(fd);
	ch->hp = fobj_read_uint32(fd);
	ch->maxmp = fobj_read_uint32(fd);
	ch->mp = fobj_read_uint32(fd);

	dprintf("flags 0x%x level %d exp %d age %d hp %d/%d mp %d/%d",
	    ch->flags, ch->level, ch->exp, ch->age,
	    ch->hp, ch->maxhp, ch->mp, ch->maxhp);

	if (ch->flags & CHAR_ONMAP) {
		char *mname;
		int i, ncoords;

		/* Read single map coordinates. */
		ncoords = fobj_read_uint32(fd);
		for (i = 0; i < ncoords; i++) {
			struct map *m;
			int x, y;
		
			mname = fobj_read_string(fd);
			x = fobj_read_uint32(fd);
			y = fobj_read_uint32(fd);
			ch->curoffs = fobj_read_uint32(fd);
			ch->curspeed = fobj_read_uint32(fd);
			ch->maxspeed = fobj_read_uint32(fd);

			m = (struct map *)object_strfind(mname);
			if (m != NULL) {
				struct node *node;

				node = &m->map[x][y];
				pthread_mutex_lock(&m->lock);
				char_add(ch, m, x, y);
				pthread_mutex_unlock(&m->lock);
				dprintf("at %s:%d,%d\n", m->obj.name, x, y);
			} else {
				fatal("no such map: \"%s\"\n", mname);
			}
			free(mname);
		}
		dprintf("%d coordinates\n", ncoords);
	}

	return (0);
badmagic:
	fatal("%s: mad magic\n", ch->obj.name);
	return (-1);
}

int
char_save(void *p, int fd)
{
	struct character *ch = (struct character *)p;
	struct fobj_buf *buf;

	buf = fobj_create_buf(128, 4);
	if (buf == NULL) {
		return (-1);
	}

	fobj_bwrite(buf, "agar char ", 11);
	fobj_bwrite_uint32(buf, CHAR_VERMAJ);
	fobj_bwrite_uint32(buf, CHAR_VERMIN);

	/* Write character properties. */
	fobj_bwrite_string(buf, ch->obj.name);
	fobj_bwrite_uint32(buf, ch->flags &= ~(CHAR_DONTSAVE));
	fobj_bwrite_uint32(buf, ch->level);
	fobj_bwrite_uint32(buf, ch->exp);
	fobj_bwrite_uint32(buf, ch->age);
	fobj_bwrite_uint64(buf, ch->seed);

	fobj_bwrite_uint32(buf, ch->maxhp);
	fobj_bwrite_uint32(buf, ch->hp);
	fobj_bwrite_uint32(buf, ch->maxmp);
	fobj_bwrite_uint32(buf, ch->mp);
	
	fobj_bwrite_uint32(buf, 1);
	if (ch->flags & CHAR_ONMAP) {
		/* Write single map coordinates. */
		fobj_bwrite_string(buf, ch->map->obj.name);
		fobj_bwrite_uint32(buf, ch->x);
		fobj_bwrite_uint32(buf, ch->y);
		fobj_bwrite_uint32(buf, ch->curoffs);
		fobj_bwrite_uint32(buf, ch->curspeed);
		fobj_bwrite_uint32(buf, ch->maxspeed);
	}

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

	ch->timer = SDL_AddTimer(ch->maxspeed - ch->curspeed, char_time, ch);
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

	if (ch->flags & CHAR_FOCUS) {
		char_unfocus(ch);
	}

	pthread_mutex_lock(&ch->map->lock);
	if (ch->map != NULL) {
		struct node *node = &ch->map->map[ch->x][ch->y];
		struct noderef *nref;

		while ((nref = node_findref(node, ch, -1))) {
			node_delref(node, nref);
		}
	}
	pthread_mutex_unlock(&ch->map->lock);

	pthread_mutex_lock(&world->lock);
	SLIST_REMOVE(&world->wcharsh, ch, character, wchars);
	pthread_mutex_unlock(&world->lock);

	return (0);
}

/* See if ch can move to nx:ny in its map. */
int
char_canmove(struct character *ch, int nx, int ny)
{
	struct node *me;

	me = &ch->map->map[nx][ny];

	return ((me->flags & NODE_WALK) ? 0 : -1);
}

void
char_event(void *p, SDL_Event *ev)
{
	struct character *ch = (struct character *)p;

	switch (ev->type) {
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		char_joybutton(ch, ev);
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		char_key(ch, ev);
		break;
	}
}

/* Change current sprite. */
int
char_setsprite(struct character *ch, int soffs)
{
	struct noderef *nref;

	nref = node_findref(&ch->map->map[ch->x][ch->y], ch, ch->curoffs);
	node_delref(&ch->map->map[ch->x][ch->y], nref);
	node_addref(&ch->map->map[ch->x][ch->y], ch, soffs, MAPREF_SPRITE);

	ch->flags &= ~(CHAR_ANIM);
	ch->curoffs = soffs;

	return (0);
}

/* Change current animation. */
int
char_setanim(struct character *ch, int aoffs)
{
	struct noderef *nref;
	
	nref = node_findref(&ch->map->map[ch->x][ch->y], ch, ch->curoffs);
	node_delref(&ch->map->map[ch->x][ch->y], nref);
	node_addref(&ch->map->map[ch->x][ch->y], ch, aoffs, MAPREF_ANIM);

	ch->flags |= CHAR_ANIM;
	ch->curoffs = aoffs;

	return (0);
}

/* Let the player control this character. */
int
char_focus(struct character *ch)
{
	ch->flags |= CHAR_FOCUS;
	curchar = ch;

	return (0);
}

int
char_unfocus(struct character *ch)
{
	ch->flags &= CHAR_FOCUS;
	curchar = NULL;
	
	return (0);
}

/* Position the character on m:x,y. */
int
char_add(struct character *ch, struct map *m, int x, int y)
{
	if (ch->flags & CHAR_ANIM) {
		node_addref(&m->map[x][y], ch, ch->curoffs, MAPREF_ANIM);
	} else {
		node_addref(&m->map[x][y], ch, ch->curoffs, MAPREF_SPRITE);
	}

	ch->map = m;
	if ((ch->flags & CHAR_ONMAP) == 0) {
		mapdir_init(&ch->dir, (struct object *)ch, m,
		    DIR_SCROLLVIEW|DIR_SOFTSCROLL, 5);
		ch->flags |= CHAR_ONMAP;
	}

	ch->x = x;
	ch->y = y;
	
	return (0);
}

/* Delete any reference to the character on m:x,y. */
int
char_del(struct character *ch, struct map *m, int x, int y)
{
	struct node *node = &m->map[x][y];
	struct noderef *nref;

	while ((nref = node_findref(node, ch, -1))) {
		node_delref(node, nref);
	}

	return (0);
}

/* Move the character from its current position to nx,ny. */
int
char_move(struct character *ch, int nx, int ny)
{
	struct noderef *nref;
	int oxoffs, oyoffs;

	/* XXX recurse */
	nref = node_findref(&ch->map->map[ch->x][ch->y], ch, -1);
	oxoffs = nref->xoffs;
	oyoffs = nref->yoffs;

	char_del(ch, ch->map, ch->x, ch->y);
	char_add(ch, ch->map, nx, ny);

	/* XXX recurse */
	nref = node_findref(&ch->map->map[nx][ny], ch, -1);
	nref->xoffs = oxoffs;
	nref->yoffs = oyoffs;

	ch->x = nx;
	ch->y = ny;

	return (0);
}

void
char_setspeed(struct character *ch, Uint32 speed)
{
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
}

static Uint32
char_time(Uint32 ival, void *obp)
{
	struct object *ob = (struct object *)obp;
	struct character *ch = (struct character *)ob;
	int moved = 0;
	int x, y;

	if (ch->map == NULL) {
		/* Nothing to do. age, perhaps. */
	}

	x = ch->x;
	y = ch->y;
	
	/* Move the character. */
	moved = mapdir_move(&ch->dir, &x, &y);
	if (moved != 0) {
#if 0
		SDL_Event nev;
		int i, nkeys;
#endif
		if (moved & DIR_UP)
			char_setanim(ch, CHAR_WALKUP);
		if (moved & DIR_DOWN)
			char_setanim(ch, CHAR_WALKDOWN);
		if (moved & DIR_LEFT)
			char_setanim(ch, CHAR_WALKLEFT);
		if (moved & DIR_RIGHT)
			char_setanim(ch, CHAR_WALKRIGHT);

		pthread_mutex_lock(&ch->map->lock);
		char_move(ch, x, y);
		mapdir_postmove(&ch->dir, &x, &y, moved);
		pthread_mutex_unlock(&ch->map->lock);
		ch->map->redraw++;
	
		

#if 0
		for (i = 0; i < sizeof(stickykeys) / sizeof(int); i++) {
			if ((SDL_GetKeyState(&nkeys))[stickykeys[i]]) {
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = stickykeys[i];
				SDL_PushEvent(&nev);
			}
		}
#endif

#if 0
		/* Assume various status ailments. */

		if (nme->flags & NODE_BIO) {
			decrease(&ch->hp, 1, 1);
			dprintf("bio. hp = %d/%d\n", ch->hp, ch->maxhp);
		} else if (nme->flags & NODE_REGEN) {
			increase(&ch->hp, 1, ch->maxhp);
			dprintf("regen. hp = %d/%d\n", ch->hp, ch->maxhp);
		}

		if (nme->flags & NODE_SLOW) {
			/* XXX rate */
			nme->v1 = -10;
			char_setspeed(ch, ch->curspeed + nme->v1);
			dprintf("slow. speed = %d\n", ch->curspeed);
		} else if (nme->flags & NODE_HASTE) {
			/* XXX rate */
			nme->v1 = 10;
			char_setspeed(ch, ch->curspeed + nme->v1);
			dprintf("haste. speed = %d\n", ch->curspeed);
		}
#endif
	}
	
	return (ival);
}

static void
char_joybutton(struct character *ch, SDL_Event *ev)
{
	static SDL_Event nev;
	
	dprintf("key %d\n", ev->jbutton.button);
	
	nev.type = (ev->type == SDL_JOYBUTTONUP) ?
	    SDL_KEYUP : SDL_KEYDOWN;
	
	/* XXX remap */
	switch (ev->jbutton.button) {
	case 2:
		nev.key.keysym.sym = SDLK_d;	/* Dash */
		break;
	/* ... */
	}
	SDL_PushEvent(&nev);
}

static void
char_key(struct character *ch, SDL_Event *ev)
{
	int set;

	set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	pthread_mutex_lock(&ch->map->lock);

	switch (ev->key.keysym.sym) {
	case SDLK_d:
		/* Dash */
		if (ev->type == SDL_KEYDOWN) {
			ch->flags |= CHAR_DASH;
			char_setspeed(ch, 40);	/* XXX use v1 */
		} else if (ev->type == SDL_KEYUP) {
			ch->flags &= ~(CHAR_DASH);
			char_setspeed(ch, 1);	/* XXX restore speed */
		}
		break;
	case SDLK_UP:
		mapdir_set(&ch->dir, DIR_UP, set);
		break;
	case SDLK_DOWN:
		mapdir_set(&ch->dir, DIR_DOWN, set);
		break;
	case SDLK_LEFT:
		mapdir_set(&ch->dir, DIR_LEFT, set);
		break;
	case SDLK_RIGHT:
		mapdir_set(&ch->dir, DIR_RIGHT, set);
		break;
	default:
		break;
	}
	
	pthread_mutex_unlock(&ch->map->lock);
}

#ifdef DEBUG

void
char_dump(struct character *ch)
{
	printf("%3d. %10s lvl %d (exp %d) hp %d/%d mp %d/%d at %s:%dx%d\n",
	    ch->obj.id, ch->obj.name, ch->level, ch->exp, ch->hp, ch->maxhp,
	    ch->mp, ch->maxmp, ch->map->obj.name, ch->x, ch->y);
	printf("\t\t< ");
	if (ch->flags & CHAR_FOCUS)
		printf("focused ");
	printf(">");
}

#endif /* DEBUG */

