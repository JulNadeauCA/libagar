/*	$Csoft: char.c,v 1.2 2002/01/25 11:15:22 vedge Exp $	*/

/*
 * These functions maintain character structures, they are called by
 * the actual character implementation. Characters are moved inside
 * the map with a controller.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <engine/debug.h>
#include <engine/view.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/char.h>
#include <engine/map.h>

static Uint32	char_time(Uint32, void *);

struct character *
char_create(char *name, char *desc, int maxhp, int maxmp, struct map *fm,
    int flags)
{
	struct character *nb;
	
	nb = (struct character *) malloc(sizeof(struct character));
	if (nb == NULL) {
		return (NULL);
	}

	object_create(&nb->obj, name, desc, EVENT_HOOK|flags);
	nb->obj.event_hook = char_event;
	nb->obj.map = fm;
	nb->obj.mapx = 0;
	nb->obj.mapy = 0;

	nb->flags = CHAR_ACTIVE;
	nb->curspeed = 1;
	nb->maxspeed = 50;
	nb->level = 0;
	nb->exp = 0;
	nb->age = 0;
	nb->maxhp = maxhp;
	nb->maxmp = maxmp;
	nb->hp = maxhp;
	nb->mp = maxmp;
	nb->seed = lrand48();
	nb->effect = 0;
	nb->direction = 0;

	dprintf("%s: hp %d/%d mp%d/%d seed 0x%lx\n",
	    nb->obj.name, nb->hp, nb->maxhp, nb->mp, nb->maxmp, nb->seed);

	/*
	 * Synchronize with both display and input.
	 */

	/* XXX avoid possible race with upper layer? */
	SDL_Delay(500);
	nb->timer = SDL_AddTimer(fm->view->fps + (nb->maxspeed - nb->curspeed),
	    char_time, nb);
	if (nb->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		free(nb);
		return (NULL);
	}

	return (nb);
}

int
char_link(void *objp)
{
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

	return (0);
}

void
char_destroy(struct object *ob)
{
	struct character *eb = (struct character *)ob;
	struct map *em = eb->obj.map;
	int x, y = 0;

	SDL_RemoveTimer(eb->timer);
	
	eb->flags = CHAR_ZOMBIE;

	/*
	 * XXX should optimize by using back references.
	 */
	if (pthread_mutex_lock(&em->lock) != 0) {
		perror(em->obj.name);
	}
	for (x = 0; x < em->mapw; x++) {
		for (y = 0; y < em->maph; y++) {
			struct map_aref *aref;
			struct map_entry *me;
			int i;

			me = &em->map[x][y];
			for (i = 0; i < me->nobjs; i++) {
				aref = map_entry_aref(me, i);
				if (aref != NULL && aref->pobj == ob) {
					map_entry_delref(me, aref);
				}
			}
		}
	}
	pthread_mutex_unlock(&em->lock);

	if (pthread_mutex_lock(&world->lock) == 0) {
		world->chars = g_slist_remove(world->chars, (void *)ob);
		world->nchars--;
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
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
		    ch->obj.map->view->fps + (ch->maxspeed - ch->curspeed),
		    char_time, ch);
	}
}

static Uint32
char_time(Uint32 ival, void *obp)
{
	struct object *ob = (struct object *)obp;
	struct character *ch = (struct character *)ob;
	int mapx, mapy;

	mapx = ob->mapx;
	mapy = ob->mapy;

	if (ch->effect & EFFECT_REGEN) {
		increase(&ch->hp, 1, ch->hp);
	}
	if (ch->effect & EFFECT_POISON) {
		decrease(&ch->hp, 1, 0);
	}

	if (ch->hp <= 0) {
		object_destroy(ob, NULL);
		curmap->redraw++;
	}

	if (ch->direction & CHAR_UP) {
		decrease(&mapy, 1, 1);
		if(ob->map->view->mapy - mapy >= 0) {
			SCROLL_UP(&ob->map);
		}
		ob->wmask |= CHAR_UP;
	} else if (ch->direction & CHAR_DOWN) {
		increase(&mapy, 1, ob->map->mapw - 1);
		if (ob->map->view->mapy - mapy <=
		    -ob->map->view->maph + 1) {
			SCROLL_DOWN(&ob->map);
		}
		ob->wmask |= CHAR_DOWN;
	}
		
	if (ch->direction & CHAR_LEFT) {
		decrease(&mapx, 1, 1);
		if(ob->map->view->mapx - mapx >= 0) {
			SCROLL_LEFT(&ob->map);
		}
		ob->wmask |= CHAR_LEFT;
	} else if (ch->direction & CHAR_RIGHT) {
		increase(&mapx, 1, ob->map->mapw - 1);
		if (ob->map->view->mapx - mapx <=
		    -ob->map->view->mapw + 1) {
			SCROLL_RIGHT(&ob->map);
		}
		ob->wmask |= CHAR_RIGHT;
	}

	if (mapx != ob->mapx || mapy != ob->mapy) {
		struct map_entry *nme = &ob->map->map[mapx][mapy];
			
		/* Verify the destination tile. */
		if (nme->flags & MAPENTRY_WALK) {
			map_entry_moveref(ob->map, ob, 1, mapx, mapy);
			ob->map->redraw++;
		}

		/* Assume various conditions. */
		if (nme->flags & MAPENTRY_BIO) {
			decrease(&ch->hp, 1, 1);
			dprintf("bio. hp = %d/%d\n",
			    ch->hp, ch->maxhp);
		} else if (nme->flags & MAPENTRY_REGEN) {
			increase(&ch->hp, 1, ch->maxhp);
			dprintf("regen. hp = %d/%d\n",
			    ch->hp, ch->maxhp);
		}

		if (nme->flags & MAPENTRY_SLOW) {
			/* XXX revisit later */
			nme->v1 = -10;
			char_setspeed(ch, ch->curspeed + nme->v1);
			dprintf("slow. speed = %d\n",
			    ch->curspeed);
		} else if (nme->flags & MAPENTRY_HASTE) {
			/* XXX revisit later */
			nme->v1 = 10;
			char_setspeed(ch, ch->curspeed + nme->v1);
			dprintf("haste. speed = %d\n",
			    ch->curspeed);
		}
	}
	
	return (ival);
}

#ifdef DEBUG

void
char_dump_char(void *ob, void *p)
{
	struct character *fc = (struct character *)ob;

	printf("%3d. %10s lvl %d hp %d/%d mp %d/%d flags 0x%x\n",
	    fc->obj.id, fc->obj.name, fc->level, fc->hp, fc->maxhp, fc->mp,
	    fc->maxmp, fc->flags);
	printf("\t\texp %.2f age %.2f effect 0x%x\n", fc->exp,
	    fc->age, fc->effect);
	printf("\t\t< ");
	if (fc->flags & CHAR_FOCUS)
		printf("focused ");
	if (fc->flags & CHAR_ACTIVE)
		printf("active ");
	if (fc->flags & CHAR_SMOTION)
		printf("sticky-motion ");
	if (fc->direction & CHAR_UP)
		printf("going-up ");
	if (fc->direction & CHAR_DOWN)
		printf("going-down ");
	if (fc->direction & CHAR_LEFT)
		printf("going-left ");
	if (fc->direction & CHAR_RIGHT)
		printf("going-right ");
	printf(">");
}

#endif /* DEBUG */

