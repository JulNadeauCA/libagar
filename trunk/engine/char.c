/*	$Csoft: char.c,v 1.7 2002/01/30 17:48:59 vedge Exp $	*/

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

static Uint32	char_time(Uint32, void *);

struct character *
char_create(char *name, char *desc, int maxhp, int maxmp, int flags)
{
	struct character *ch;
	
	ch = (struct character *)malloc(sizeof(struct character));
	if (ch == NULL) {
		return (NULL);
	}

	object_create(&ch->obj, name, desc, EVENT_HOOK|flags);
	ch->obj.event_hook = char_event;
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
	ch->direction = 0;

	char_setsprite(ch, 2);	 /* XXX */

	dprintf("%s: hp %d/%d mp %d/%d seed 0x%lx\n",
	    ch->obj.name, ch->hp, ch->maxhp, ch->mp, ch->maxmp, ch->seed);

	return (ch);
}

int
char_link(void *objp)
{
	struct character *ch = (struct character *)objp;
	
	if (pthread_mutex_lock(&world->lock) == 0) {
		world->chars = g_slist_append(world->chars, objp);
		world->nchars++;
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}

	if (object_link(objp) < 0) {
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
	
	ch->flags &= ~CHAR_FOCUS;

	if (pthread_mutex_lock(&ch->map->lock) == 0) {
		if (ch->map != NULL) {
			MAP_DELREF(ch->map, ch->x, ch->y, ob, -1);
		}
		pthread_mutex_unlock(&ch->map->lock);
	} else {
		perror(ch->map->obj.name);
	}

	if (pthread_mutex_lock(&world->lock) == 0) {
		world->chars = g_slist_remove(world->chars, (void *)ob);
		world->nchars--;
		pthread_mutex_unlock(&world->lock);
	} else {
		perror(world->obj.name);
	}
}

void
char_event(struct object *ob, SDL_Event *ev)
{
	struct character *fc = (struct character *)ob;

	if ((fc->flags & CHAR_FOCUS) == 0) {
		/* We are not being controlled. */
		return;
	}

	/*
	 * Joystick control.
	 */
	if (ev->type == SDL_JOYAXISMOTION) {
		static SDL_Event nev;
		static int lastdir = 0;

		switch (ev->jaxis.axis) {
		case 0:	/* X */
			if (ev->jaxis.value < 0) {
				lastdir |= CHAR_LEFT;
				lastdir &= ~(CHAR_RIGHT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_LEFT;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= CHAR_RIGHT;
				lastdir &= ~(CHAR_LEFT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_RIGHT;
				SDL_PushEvent(&nev);
			} else {
				object_wait(fc, lastdir);
				if (lastdir & CHAR_LEFT) {
					fc->direction &= ~(CHAR_LEFT);
				} else if (lastdir & CHAR_RIGHT) {
					fc->direction &= ~(CHAR_RIGHT);
				}
			}
			break;
		case 1:	/* Y */
			if (ev->jaxis.value < 0) {
				lastdir |= CHAR_UP;
				lastdir &= ~(CHAR_DOWN);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_UP;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= CHAR_DOWN;
				lastdir &= ~(CHAR_UP);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_DOWN;
				SDL_PushEvent(&nev);
			} else {
				object_wait(fc, lastdir);
				if (lastdir & CHAR_UP) {
					fc->direction &= ~(CHAR_UP);
				} else if (lastdir & CHAR_DOWN) {
					fc->direction &= ~(CHAR_DOWN);
				}
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
					fc->flags |= CHAR_DASH;
					char_setspeed(fc, 40);
				} else if (ev->type == SDL_KEYUP) {
					fc->flags &= ~(CHAR_DASH);
					char_setspeed(fc, 1);
				}
				break;
			case SDLK_UP:
				if (ev->type == SDL_KEYDOWN) {
					fc->direction &= ~(CHAR_DOWN);
					fc->direction |= CHAR_UP;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(fc, CHAR_UP);
					fc->direction &= ~(CHAR_UP);
				}
	
				break;
			case SDLK_DOWN:
				if (ev->type == SDL_KEYDOWN) {
					fc->direction &= ~(CHAR_UP);
					fc->direction |= CHAR_DOWN;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(fc, CHAR_DOWN);
					fc->direction &= ~(CHAR_DOWN);
				}
				break;
			case SDLK_LEFT:
				if (ev->type == SDL_KEYDOWN) {
					fc->direction &= ~(CHAR_RIGHT);
					fc->direction |= CHAR_LEFT;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(fc, CHAR_LEFT);
					fc->direction &= ~(CHAR_LEFT);
				}
				break;
			case SDLK_RIGHT:
				if (ev->type == SDL_KEYDOWN) {
					fc->direction &= ~(CHAR_LEFT);
					fc->direction |= CHAR_RIGHT;
				} else if (ev->type == SDL_KEYUP) {
					object_wait(fc, CHAR_RIGHT);
					fc->direction &= ~(CHAR_RIGHT);
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
	static int mapx, mapy;

	mapx = ch->x;
	mapy = ch->y;

	if (ch->effect & EFFECT_REGEN) {
		/* XXX rate? */
		increase(&ch->hp, 1, ch->hp);
	}
	if (ch->effect & EFFECT_POISON) {
		/* XXX rate? */
		decrease(&ch->hp, 1, 0);
	}

	if (ch->hp <= 0) {
		object_destroy(ob, NULL);
		curmap->redraw++;
	}

	if (ch->direction & CHAR_UP) {
		char_setsprite(ch, 2);
		decrease(&mapy, 1, 1);
		if(ch->map->view->mapy - mapy >= 0) {
			SCROLL_UP(&ch->map);
		}
		ob->wmask |= CHAR_UP;
	} else if (ch->direction & CHAR_DOWN) {
		char_setsprite(ch, 1);
		increase(&mapy, 1, ch->map->mapw - 1);
		if (ch->map->view->mapy - mapy <=
		    -ch->map->view->maph + 1) {
			SCROLL_DOWN(&ch->map);
		}
		ob->wmask |= CHAR_DOWN;
	}
		
	if (ch->direction & CHAR_LEFT) {
		char_setsprite(ch, 3);
		decrease(&mapx, 1, 1);
		if(ch->map->view->mapx - mapx >= 0) {
			SCROLL_LEFT(&ch->map);
		}
		ob->wmask |= CHAR_LEFT;
	} else if (ch->direction & CHAR_RIGHT) {
		char_setsprite(ch, 4);
		increase(&mapx, 1, ch->map->mapw - 1);
		if (ch->map->view->mapx - mapx <=
		    -ch->map->view->mapw + 1) {
			SCROLL_RIGHT(&ch->map);
		}
		ob->wmask |= CHAR_RIGHT;
	}

	if (mapx != ch->x || mapy != ch->y) {
		struct map_entry *nme = &ch->map->map[mapx][mapy];
			
		/* Walk on the destination tile, if possible. */
		if (nme->flags & MAPENTRY_WALK) {
			char_move(ch, mapx, mapy);
			ch->map->redraw++;
		}

		/* Assume various conditions. */
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
	}
	
	return (ival);
}

#ifdef DEBUG

void
char_dump_char(void *ob, void *p)
{
	struct character *ch= (struct character *)ob;

	printf("%3d. %10s lvl %d (ex %.2f) hp %d/%d mp %d/%d at %s:%dx%d\n",
	    ch->obj.id, ch->obj.name, ch->level, ch->exp, ch->hp, ch->maxhp,
	    ch->mp, ch->maxmp, ch->map->obj.name, ch->x, ch->y);
	printf("\t\t< ");
	if (ch->flags & CHAR_FOCUS)
		printf("focused ");
	if (ch->direction & CHAR_UP)
		printf("going-up ");
	if (ch->direction & CHAR_DOWN)
		printf("going-down ");
	if (ch->direction & CHAR_LEFT)
		printf("going-left ");
	if (ch->direction & CHAR_RIGHT)
		printf("going-right ");
	printf(">");
}

#endif /* DEBUG */

