/*	$Csoft: char.c,v 1.9 2002/02/01 06:04:57 vedge Exp $	*/

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <engine/engine.h>

#define JOY_UP		0x01
#define JOY_DOWN	0x02
#define JOY_LEFT	0x04
#define JOY_RIGHT	0x08

static Uint32	char_time(Uint32, void *);
static void	char_cntrl_event(struct character *, SDL_Event *);

struct character *curchar;

struct character *
char_create(char *name, char *desc, int maxhp, int maxmp, int flags)
{
	struct character *ch;
	
	ch = (struct character *)malloc(sizeof(struct character));
	if (ch == NULL) {
		return (NULL);
	}

	object_create(&ch->obj, name, desc, flags);
	ch->event_hook = char_cntrl_event;
	ch->map = NULL;
	ch->x = -1;
	ch->y = -1;

	ch->flags = 0;
	ch->curspeed = 1;
	ch->maxspeed = 50;
	ch->level = 0;
	ch->exp = 0;
	ch->age = 0;
	ch->maxhp = maxhp;
	ch->maxmp = maxmp;
	ch->hp = maxhp;
	ch->mp = maxmp;
	ch->seed = lrand48();
	ch->effect = 0;

	char_setanim(ch, 0);	 /* XXX */

	dprintf("%s: hp %d/%d mp %d/%d seed 0x%lx\n",
	    ch->obj.name, ch->hp, ch->maxhp, ch->mp, ch->maxmp, ch->seed);

	return (ch);
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

	if (object_link(ob) < 0) {
		return (-1);
	}
	
	ch->timer = SDL_AddTimer(60 + (ch->maxspeed - ch->curspeed),
	    char_time, ch);
	if (ch->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		return (-1);
	}

	return (0);
}

void
char_destroy(struct object *ob)
{
	struct character *ch = (struct character *)ob;

	SDL_RemoveTimer(ch->timer);

	if (ch->flags & CHAR_FOCUS) {
		char_unfocus(ch);
	}

	if (pthread_mutex_lock(&ch->map->lock) == 0) {
		if (ch->map != NULL) {
			MAP_DELREF(ch->map, ch->x, ch->y, ob, -1);
		}
		pthread_mutex_unlock(&ch->map->lock);
	} else {
		perror(ch->map->obj.name);
	}

	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_REMOVE(&world->wcharsh, ch, character, wchars);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror(world->obj.name);
	}
}

/* See if ch can move to nx:ny in its map. */
int
char_canmove(struct character *ch, int nx, int ny)
{
	struct node *me;

	me = &ch->map->map[nx][ny];

	/* TODO check `levels'? */
	
	return ((me->flags & MAPENTRY_WALK) ? 0 : -1);
}

static void
char_cntrl_event(struct character *ch, SDL_Event *ev)
{
	struct map_aref *aref;

	/* XXX limit one sprite/anim. */
	aref = node_arefobj(&ch->map->map[ch->x][ch->y],
	    (struct object *)ch, -1);

	/*
	 * Joystick control.
	 */
	if (ev->type == SDL_JOYAXISMOTION) {
		static SDL_Event nev;
		static int lastdir = 0;

		switch (ev->jaxis.axis) {
		case 0:	/* X */
			if (ev->jaxis.value < 0) {
				lastdir |= JOY_LEFT;
				lastdir &= ~(JOY_RIGHT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_LEFT;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= JOY_RIGHT;
				lastdir &= ~(JOY_LEFT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_RIGHT;
				SDL_PushEvent(&nev);
			} else {
				/* Axis is 0, stop moving. */
				object_wait(ch, lastdir);
				aref->xoffs = 0;
			}
			break;
		case 1:	/* Y */
			if (ev->jaxis.value < 0) {
				lastdir |= JOY_UP;
				lastdir &= ~(JOY_DOWN);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_UP;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= JOY_DOWN;
				lastdir &= ~(JOY_UP);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_DOWN;
				SDL_PushEvent(&nev);
			} else {
				object_wait(ch, lastdir);
				aref->yoffs = 0;
			}
			break;
		}
		return;
	}
	if (ev->type == SDL_JOYBUTTONDOWN || ev->type == SDL_JOYBUTTONUP) {
		static SDL_Event nev;

		dprintf("key %d\n", ev->jbutton.button);

		nev.type = (ev->type == SDL_JOYBUTTONUP) ?
		    SDL_KEYUP : SDL_KEYDOWN;
		nev.key.keysym.sym = SDLK_x;
		SDL_PushEvent(&nev);
		return;
	}

	/*
	 * Keyboard motion.
	 */
	if (ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP) {
		switch (ev->key.keysym.sym) {
			case SDLK_d:
				if (ev->type == SDL_KEYDOWN) {
					ch->flags |= CHAR_DASH;
					char_setspeed(ch, 40);
				} else if (ev->type == SDL_KEYUP) {
					ch->flags &= ~(CHAR_DASH);
					char_setspeed(ch, 1);
				}
				break;
			case SDLK_UP:
				if (ev->type == SDL_KEYDOWN) {
					aref->yoffs = -1;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(ch, WMASK_UP);
					aref->yoffs = 0;
				}
	
				break;
			case SDLK_DOWN:
				if (ev->type == SDL_KEYDOWN) {
					aref->yoffs = 1;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(ch, WMASK_DOWN);
					aref->yoffs = 0;
				}
				break;
			case SDLK_LEFT:
				if (ev->type == SDL_KEYDOWN) {
					aref->xoffs = -1;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(ch, WMASK_LEFT);
					aref->xoffs = 0;
				}
				break;
			case SDLK_RIGHT:
				if (ev->type == SDL_KEYDOWN) {
					aref->xoffs = 1;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(ch, WMASK_RIGHT);
					aref->xoffs = 0;
				}
				break;
			default:
				break;
		}
	}
}

/* Change current animation. */
int
char_setsprite(struct character *ch, int soffs)
{
	ch->flags &= ~(CHAR_ANIM);
	ch->curoffs = soffs;

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

/* Change current sprite. */
int
char_setanim(struct character *ch, int aoffs)
{
	ch->flags |= CHAR_ANIM;
	ch->curoffs = aoffs;

	return (0);
}

/* Position the character on m:x,y. */
int
char_add(struct character *ch, struct map *m, int x, int y)
{
	if (ch->flags & CHAR_ANIM) {
		MAP_ADDANIM(m, x, y, (struct object *)ch, ch->curoffs);
	} else {
		MAP_ADDSPRITE(m, x, y, (struct object *)ch, ch->curoffs);
	}
	
	ch->map = m;
	ch->x = x;
	ch->y = y;

	return (0);
}

/* Delete any reference to the character on m:x,y. */
int
char_del(struct character *ch, struct map *m, int x, int y)
{
	MAP_DELREF(m, x, y, (struct object *)ch, -1);

	/* Map reference (m:x,y) is now undefined. */

	return (0);
}

/* Move the character from its current position to nx,ny. */
int
char_move(struct character *ch, int nx, int ny)
{
	char_del(ch, ch->map, ch->x, ch->y);
	char_add(ch, ch->map, nx, ny);

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
		ch->timer = SDL_AddTimer(
		    ch->map->view->fps + (ch->maxspeed - ch->curspeed),
		    char_time, ch);
	}
}

static Uint32
char_time(Uint32 ival, void *obp)
{
	struct object *ob = (struct object *)obp;
	struct character *ch = (struct character *)ob;
	struct map_aref *aref;

	/* XXX limit one sprite/anim. */
	aref = node_arefobj(&ch->map->map[ch->x][ch->y], ob, -1);
#ifdef DEBUG
	if (aref == NULL) {
		dprintf("%s is not at %dx%d\n", ob->name, ch->x, ch->y);
	}
#endif

	/*
	 * Vertical soft scroll.
	 */
	if (aref->yoffs < 0) {
		ch->map->redraw++;
		dprintf("up move\n");
		if (aref->yoffs == -1) {
			if (char_canmove(ch, ch->x, ch->y - 1) < 0) {
				dprintf("blocked!\n");
				aref->yoffs = 0;
				goto xoffsck;
			}
			char_setanim(ch, 1);
		}
		aref->yoffs--;
		dprintf("yoffs is now %d\n", aref->yoffs);
		if (aref->yoffs <= -ch->map->view->tileh) {
			char_move(ch, ch->x, ch->y - 1);
			aref->yoffs = 0;
			ob->wmask |= WMASK_UP;
		}
	} else if (aref->yoffs > 0) {
		dprintf("down move\n");
		if (aref->yoffs == 1) {
			if (char_canmove(ch, ch->x, ch->y + 1) < 0) {
				dprintf("blocked!\n");
				aref->yoffs = 0;
				goto xoffsck;
			}
			char_setanim(ch, 2);
		}
		aref->yoffs++;
		if (aref->yoffs >= ch->map->view->tileh) {
			char_move(ch, ch->x, ch->y + 1);
			aref->yoffs = 0;
			ob->wmask |= WMASK_DOWN;
		}
	}

xoffsck:
	/*
	 * Horizontal soft scroll.
	 */
	if (aref->xoffs < 0) {
		dprintf("left move\n");
		if (aref->xoffs == -1) {
			if (char_canmove(ch, ch->x - 1, ch->y) < 0) {
				dprintf("blocked!\n");
				aref->xoffs = 0;
				goto xoffsck;
			}
			char_setanim(ch, 3);
		}
		aref->xoffs--;
		if (aref->xoffs <= -ch->map->view->tilew) {
			char_move(ch, ch->x - 1, ch->y);
			aref->xoffs = 0;
			ob->wmask |= WMASK_LEFT;
		}
	} else if (aref->xoffs > 0) {
		dprintf("right move\n");
		if (aref->xoffs == 1) {
			if (char_canmove(ch, ch->x + 1, ch->y) < 0) {
				dprintf("blocked!\n");
				aref->xoffs = 0;
				goto ailmentck;
			}
			char_setanim(ch, 4);
		}
		aref->xoffs++;
		if (aref->xoffs >= ch->map->view->tilew) {
			char_move(ch, ch->x + 1, ch->y);
			aref->xoffs = 0;
			ob->wmask |= WMASK_RIGHT;
		}
	}

ailmentck:
	/* Assume various status ailments. */

#if 0
	if (nme->flags & MAPENTRY_BIO) {
		decrease(&ch->hp, 1, 1);
		dprintf("bio. hp = %d/%d\n", ch->hp, ch->maxhp);
	} else if (nme->flags & MAPENTRY_REGEN) {
		increase(&ch->hp, 1, ch->maxhp);
		dprintf("regen. hp = %d/%d\n", ch->hp, ch->maxhp);
	}

	if (nme->flags & MAPENTRY_SLOW) {
		/* XXX rate */
		nme->v1 = -10;
		char_setspeed(ch, ch->curspeed + nme->v1);
		dprintf("slow. speed = %d\n", ch->curspeed);
	} else if (nme->flags & MAPENTRY_HASTE) {
		/* XXX rate */
		nme->v1 = 10;
		char_setspeed(ch, ch->curspeed + nme->v1);
		dprintf("haste. speed = %d\n", ch->curspeed);
	}
#endif

	return (ival);
}

#ifdef DEBUG

void
char_dump(struct character *ch)
{
	printf("%3d. %10s lvl %d (ex %.2f) hp %d/%d mp %d/%d at %s:%dx%d\n",
	    ch->obj.id, ch->obj.name, ch->level, ch->exp, ch->hp, ch->maxhp,
	    ch->mp, ch->maxmp, ch->map->obj.name, ch->x, ch->y);
	printf("\t\t< ");
	if (ch->flags & CHAR_FOCUS)
		printf("focused ");
	printf(">");
}

#endif /* DEBUG */

