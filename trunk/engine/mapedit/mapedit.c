/*	$Csoft: mapedit.c,v 1.10 2002/02/05 06:38:03 vedge Exp $	*/

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <libfobj/fobj.h>
#include <engine/engine.h>

#include "mapedit.h"
#include "mapedit_offs.h"

static void	mapedit_destroy(struct object *);
static Uint32	mapedit_time(Uint32, void *);
static void	mapedit_event(struct mapedit *, SDL_Event *);
static void	mapedit_setflag(struct mapedit *, struct node *, int);
static void	mapedit_examine(struct map *, int, int);
static void	mapedit_pointer(struct mapedit *, int);

struct mapedit *curmapedit;

struct mapedit *
mapedit_create(char *name, char *desc)
{
	char path[FILENAME_MAX];
	struct mapedit *med;
	struct map *em;
	struct fobj *fob;
	struct object *ob;
	int fd;

	strcpy(path, name);
	strcat(path, ".map");

	if ((fd = open(path, O_RDONLY, 0)) > 0) {
		close(fd);
		
		dprintf("editing %s\n", path);
		/* The description, geometry and flags will be loaded. */
		em = map_create(name, NULL, MAP_2D, 128, 128, path);
	} else {
		/* Create a new map. */
		dprintf("creating %s\n", path);
		em = map_create(name, desc, MAP_2D, 128, 128, NULL);
	}

	med = (struct mapedit *)malloc(sizeof(struct mapedit));
	if (med == NULL) {
		perror("mapedit");
		return (NULL);
	}

	object_create(&med->obj, "mapedit", "Map editor", DESTROY_HOOK);
	med->obj.destroy_hook = mapedit_destroy;
	med->event_hook = mapedit_event;
	med->map = em;
	med->x = em->defx;
	med->y = em->defy;

	med->curobj = NULL;
	med->curflags = 0;
	med->listwdir = 0;
	med->listodir = 0;
	med->cursdir = 0;
	med->flags = MAPEDIT_TILELIST|MAPEDIT_TILESTACK|MAPEDIT_OBJLIST|
	    MAPEDIT_DRAWPROPS;

	med->tilelist = window_create(em->view,
	    (em->view->width - em->view->tilew), em->view->tileh,
	    em->view->tilew, em->view->height, "Tile list");
	med->tilestack = window_create(em->view,
	    0, 0,
	    em->view->tilew, em->view->height, "Tile stack");
	med->objlist = window_create(em->view,
	    em->view->tilew, 0,
	    em->view->width - em->view->tilew, em->view->tileh,
	    "Object list");

	fob = fobj_load("../engine/mapedit/mapedit.fob");
	xcf_load(fob, MAPEDIT_XCF, NULL, &med->obj);
	fobj_free(fob);

	/*
	 * Create mapedit's internal representation of objects.
	 * This is used for distinguishing anim structures from
	 * sprites, for instance.
	 */
	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		struct editobj *eob;
		
		if ((ob->flags & OBJ_EDITABLE) == 0) {
			continue;
		}

		eob = malloc(sizeof(struct editobj));
		if (eob == NULL) {
			perror("editobj");
			pthread_mutex_unlock(&world->lock);
			return (NULL);
		}

		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefsh);
		eob->nrefs = 0;
		eob->nsprites = ob->nsprites;
		eob->nanims = ob->nanims;
		if (pthread_mutex_init(&eob->lock, NULL) != 0) {
			perror("editobj");
			pthread_mutex_unlock(&world->lock);
			return (NULL);
		}

		dprintf("%s has %d sprites, %d anims\n", ob->name,
		    eob->nsprites, eob->nanims);
		
		/* XXX for now */
		if (eob->nsprites > 0) {
			med->curobj = eob;
			med->curoffs = 1;
		}
		
		if (ob->nsprites > 0) {
			int y;

			for (y = 0; y < ob->nsprites; y++) {
				struct editref *eref;
		
				eref = malloc(sizeof(struct editref));
				if (eref == NULL) {
					perror("editref");
					pthread_mutex_unlock(&world->lock);
					return (NULL);
				}
				eref->animi = -2;
				eref->spritei = y;
				eref->p = g_slist_nth_data(ob->sprites, y);
				eref->type = EDITREF_SPRITE;

				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}
	
		if (ob->nanims > 0) {
			int z;

			for (z = 0; z < ob->nanims; z++) {
				struct editref *eref;

				eref = malloc(sizeof(struct editref));
				if (eref == NULL) {
					perror("editref");
					pthread_mutex_unlock(&world->lock);
					return (NULL);
				}
				
				eref->animi = z;
				eref->spritei = -1;
				eref->p = g_slist_nth_data(ob->anims, z);
				eref->type = EDITREF_ANIM;

				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}
		TAILQ_INSERT_HEAD(&med->eobjsh, eob, eobjs);
		med->neobjs++;
	}
	pthread_mutex_unlock(&world->lock);

	object_link(med);

	if (pthread_mutex_lock(&em->lock) == 0) {
		/* Position mapedit on the map. */
		MAPEDIT_PLOT(med, em, em->defx, em->defy);
		pthread_mutex_unlock(&em->lock);
	}

	view_center(em->view, em->defx, em->defy);
	em->redraw++;
	
	curmapedit = med;
	
	/* XXX tune */
	med->timer = SDL_AddTimer(em->view->fps + 120, mapedit_time, med);
	if (med->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		object_destroy(med);
		object_unlink(med);
		return (NULL);
	}
	
	view_setmode(em->view);	/* XXX hack */

	return (med);
}

static void
mapedit_destroy(struct object *obp)
{
	struct mapedit *med = (struct mapedit *)obp;
	struct editobj *eob;

	SDL_RemoveTimer(med->timer);

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		struct editref *eref;

		pthread_mutex_lock(&eob->lock);
		SIMPLEQ_FOREACH(eref, &eob->erefsh, erefs) {
			free(eref);
		}
		pthread_mutex_unlock(&eob->lock);
		pthread_mutex_destroy(&eob->lock);
		free(eob);
	}
}

/*
 * Enable or disable the edition cursor. The map entry parameter must
 * point to its actual location. Assumes map is already locked.
 */
static void
mapedit_pointer(struct mapedit *med, int enable)
{
	if (enable) {
		MAP_ADDANIM(med->map, med->x, med->y,
		    (struct object *)med, MAPEDIT_SELECT);
	} else {
		MAP_DELREF(med->map, med->x, med->y,
		    (struct object *)med, MAPEDIT_SELECT);
	}
}

/* Print a tile stack. Assumes the map is locked. */
static void
mapedit_examine(struct map *em, int x, int y)
{
	int i;
	struct map_aref *aref;
	struct node *me;

	me = &em->map[x][y];

	printf("%dx%d < ", x, y);

	/* Keep in sync with map.h */
	if (me->flags == MAPENTRY_BLOCK) {
		printf("block ");
	} else {
		if (me->v1 > 0)
			printf("(v1=%d) ", me->v1);
		if (me->flags & MAPENTRY_ORIGIN)
			printf("origin ");
		if (me->flags & MAPENTRY_WALK)
			printf("walk ");
		if (me->flags & MAPENTRY_CLIMB)
			printf("climb ");
		if (me->flags & MAPENTRY_SLIP)
			printf("slippery ");
		if (me->flags & MAPENTRY_BIO)
			printf("bio ");
		if (me->flags & MAPENTRY_REGEN)
			printf("regen ");
		if (me->flags & MAPENTRY_SLOW)
			printf("slow ");
		if (me->flags & MAPENTRY_HASTE)
			printf("haste ");
	}
	printf(">\n");

	i = 0;
	TAILQ_FOREACH(aref, &me->arefsh, marefs) {
		printf("\t[%2d] ", i++);
		printf("%s:%d ", aref->pobj->name, aref->offs);
		if (aref->flags & MAPREF_SAVE)
			printf("saveable ");
		if (aref->flags & MAPREF_SPRITE)
			printf("sprite ");
		if (aref->flags & MAPREF_ANIM)
			printf("anim ");
		printf("\n");
	}
}

static void
mapedit_setflag(struct mapedit *med, struct node *me, int flag)
{
	if (me->narefs < 2) {
		dprintf("empty tile\n");
		return;
	}

	if (flag == 0) {
		me->flags = 0;
	} else {
		if (me->flags & flag) {
			me->flags &= ~(flag);
		} else {
			me->flags |= flag;
		}
	}
	med->curflags = me->flags;
}

static __inline void
mapedit_bg(SDL_Surface *v, int ax, int ay, int w, int h)
{
	static Uint32 col[2];
	Uint8 *dst = v->pixels;
	int x, y;

	col[0] = SDL_MapRGB(v->format, 0x66, 0x66, 0x66);
	col[1] = SDL_MapRGB(v->format, 0x99, 0x99, 0x99);

	for (y = ay; y < h; y++) {
		for (x = ax; x < w; x++) {
			static Uint32 c;
			
			c = col[((x ^ y) >> 3) & 1];

			SDL_LockSurface(v);
			switch (v->format->BytesPerPixel) {
			case 1:
				dst[x] = c;
				break;
			case 2:
				((Uint16 *)dst)[x] = c;
				break;
			case 3:
				if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
					dst[x * 3] = c;
					dst[x * 3 + 1] = c >> 8;
					dst[x * 3 + 2] = c >> 16;
				} else {
					dst[x * 3] = c >> 16;
					dst[x * 3 + 1] = c >> 8;
					dst[x * 3 + 2] = c;
				}
				break;
			case 4:
				((Uint32 *)dst)[x] = c;
				break;
			}
			SDL_UnlockSurface(v);
		}
		dst += v->pitch;
	}
}

void
mapedit_tilelist(struct mapedit *med)
{
	struct window *win = med->tilelist;
	static int tilew, tileh;
	static SDL_Rect rs, rd;
	int i, sn;

	tilew = med->map->view->tilew;
	tileh = med->map->view->tileh;
	
	rs.w = tilew;
	rs.h = tileh;
	rs.x = 0;
	rs.y = 0;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = tilew;
	rd.h = win->height;
	mapedit_bg(win->view->v, rd.x, rd.y, win->view->width,
	    win->view->height);
	rd.h = tileh;
		
	/*
	 * Draw the sprite/anim list in a circular fashion. We must
	 * predict which sprite to draw first according to the window
	 * geometry.
	 */
	for (i = 0, sn = med->curoffs - ((win->height / tileh) / 2);
	     i < (win->height / tileh) - 1;
	     i++, rd.y += tileh) {
		struct editref *ref;
		struct anim *anim;

		/*
		 * Obtain the mapedit reference at this offset. If the
		 * index is negative, wrap.
		 */
		/* XXX array */
		pthread_mutex_lock(&med->curobj->lock);
		if (sn > -1) {
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs, sn);
		} else {
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn + med->curobj->nrefs);
		}
		pthread_mutex_unlock(&med->curobj->lock);

		if (ref == NULL) {
			/* XXX should not happen */
			dprintf("NULL ref\n");
			continue;
		}

		/* Plot the icon. */
		switch (ref->type) {
		case EDITREF_SPRITE:
			SDL_BlitSurface(ref->p, &rs, win->view->v, &rd);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			if (anim->nframes > 0) {
				SDL_BlitSurface(g_slist_nth_data(
				    anim->frames, anim->gframe),
				    &rs, win->view->v, &rd);
				if (anim->delay > 0 &&
				    anim->gframedc++ > anim->delay) {
					if (anim->gframe++ >=
					    (anim->nframes - 1)) {
						anim->gframe = 0;
					}
					anim->gframedc = 0;
					/*
					 * XXX wait the number of
					 * ticks a blit usually
					 * takes.
					 */
				}
			}
			break;
		}

		/* Plot editor decorations. */
		if (med->curoffs == sn) {
			SDL_BlitSurface(g_slist_nth_data(
			    curmapedit->obj.sprites, MAPEDIT_CIRQSEL),
			    &rs, win->view->v, &rd);
		} else {
			SDL_BlitSurface(g_slist_nth_data(
			    curmapedit->obj.sprites, MAPEDIT_GRID), &rs,
			    win->view->v, &rd);
		}
		if (sn++ >= med->curobj->nrefs - 1) {
			sn = 0;
		}
	}
}

/*
 * Draw the tile stack for the map entry under the cursor.
 * Must be called on a locked map.
 */
void
mapedit_tilestack(struct mapedit *med)
{
	static SDL_Rect rs, rd;
	struct window *win = med->tilestack;
	struct node *me;
	struct map_aref *aref;
	static int tilew, tileh;
	int i;

	me = &med->map->map[med->x][med->y];
	tilew = med->map->view->tilew;
	tileh = med->map->view->tileh;

	rs.w = tilew;
	rs.h = tileh;
	rs.x = 0;
	rs.y = 0;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = tilew;
	rd.h = win->height;
	mapedit_bg(win->view->v, rd.x, rd.y, rd.w, rd.h);
	rd.h = tileh;

	i = 0;
	TAILQ_FOREACH(aref, &me->arefsh, marefs) {
		if (++i > (win->height / tileh) - 1) {
			return;
		}
		if (aref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			/* Only draw the first frame. */
			anim = g_slist_nth_data(aref->pobj->anims, aref->offs);
			SDL_BlitSurface(g_slist_nth_data(anim->frames,
			    0), &rs, win->view->v, &rd);
		} else if (aref->flags & MAPREF_SPRITE) {
			/* Draw this sprite. */
			SDL_BlitSurface(g_slist_nth_data(aref->pobj->sprites,
			    aref->offs), &rs, win->view->v, &rd);
		}

		/* Draw a grey grid. */
		SDL_BlitSurface(g_slist_nth_data(
		    curmapedit->obj.sprites, MAPEDIT_GRID), &rs,
		    win->view->v, &rd);
		rd.y += tileh;
	}
}

/*
 * Draw a list of objects in the object ring.
 * This must be called when the world is locked.
 */
void
mapedit_objlist(struct mapedit *med)
{
	SDL_Rect rs, rd;
	struct window *win = med->objlist;
	static int tilew, tileh;
	struct editobj *eob;

	tilew = med->map->view->tilew;
	tileh = med->map->view->tileh;
	
	rs.x = 0;
	rs.y = 0;
	rs.w = tilew;
	rs.h = tileh;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = win->width;
	rd.h = tileh;
	mapedit_bg(win->view->v, rd.x, rd.y, rd.w, rd.h);
	rd.x = tilew;

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		SDL_BlitSurface(g_slist_nth_data(eob->pobj->sprites, 0),
		    &rs, win->view->v, &rd);
		if (med->curobj == eob) {
			SDL_BlitSurface(g_slist_nth_data(
			    curmapedit->obj.sprites, MAPEDIT_CIRQSEL),
			    &rs, win->view->v, &rd);
		} else {
			SDL_BlitSurface(g_slist_nth_data(
			    curmapedit->obj.sprites, MAPEDIT_GRID), &rs,
			    win->view->v, &rd);
		}
		rd.x += tilew;
	}
}

static void
mapedit_event(struct mapedit *med, SDL_Event *ev)
{
	struct map *em = med->map;
	int mapx, mapy;
	char path[FILENAME_MAX];

	/*
	 * Mouse scrolling.
	 */
	if (ev->type == SDL_MOUSEMOTION) {
		static int ommapx, ommapy;

		if (ev->motion.state != SDL_PRESSED) {
			return;
		}
		ommapx = med->mmapx;
		ommapy = med->mmapy;
		med->mmapx = ev->motion.x / em->view->tilew;
		med->mmapy = ev->motion.y / em->view->tileh;
		if (ommapx < med->mmapx) {
			SCROLL_LEFT(&med->map);
			med->map->redraw++;
		} else if (med->mmapx < ommapx) {
			SCROLL_RIGHT(&med->map);
			med->map->redraw++;
		}
		if (ommapy < med->mmapy) {
			SCROLL_UP(&med->map);
			med->map->redraw++;
		} else if (med->mmapy < ommapy) {
			SCROLL_DOWN(&med->map);
			med->map->redraw++;
		}
		return;
	}

	/*
	 * Mouse edition.
	 */
	if (ev->type == SDL_MOUSEBUTTONDOWN) {
		if (ev->button.button == 1) {
			return;
		}
		if (pthread_mutex_lock(&em->lock) == 0) {
			MAPEDIT_MOVE(med,
			    em->view->mapx + ev->button.x / em->view->tilew,
			    em->view->mapy + ev->button.y / em->view->tileh);
			pthread_mutex_unlock(&em->lock);
			em->redraw++;
		} else {
			perror(em->obj.name);
			return;
		}
		if (ev->button.button == 3) {
			do {
				SDL_Event ev;
				/* Fake the add command. */
				ev.type = SDL_KEYDOWN;
				ev.key.keysym.sym = SDLK_a;
				SDL_PushEvent(&ev);
			} while (/*CONSTCOND*/0);
		}
		return;
	}

	/*
	 * Joystick edition.
	 */
	if (ev->type == SDL_JOYAXISMOTION) {
		static SDL_Event nev;
		static int lastdir = 0;

		switch (ev->jaxis.axis) {
		case 0:	/* X */
			if (ev->jaxis.value < 0) {
				lastdir |= MAPEDIT_LEFT;
				lastdir &= ~(MAPEDIT_RIGHT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_LEFT;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= MAPEDIT_RIGHT;
				lastdir &= ~(MAPEDIT_LEFT);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_RIGHT;
				SDL_PushEvent(&nev);
			} else {
				object_wait(med, lastdir);
				if (lastdir & MAPEDIT_LEFT) {
					med->cursdir &= ~(MAPEDIT_LEFT);
				} else if (lastdir & MAPEDIT_RIGHT) {
					med->cursdir &= ~(MAPEDIT_RIGHT);
				}
			}
			break;
		case 1:	/* Y */
			if (ev->jaxis.value < 0) {
				lastdir |= MAPEDIT_UP;
				lastdir &= ~(MAPEDIT_DOWN);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_UP;
				SDL_PushEvent(&nev);
			} else if (ev->jaxis.value > 0) {
				lastdir |= MAPEDIT_DOWN;
				lastdir &= ~(MAPEDIT_UP);
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = SDLK_DOWN;
				SDL_PushEvent(&nev);
			} else {
				object_wait(med, lastdir);
				if (lastdir & MAPEDIT_UP) {
					med->cursdir &= ~(MAPEDIT_UP);
				} else if (lastdir & MAPEDIT_DOWN) {
					med->cursdir &= ~(MAPEDIT_DOWN);
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

		/* XXX customize */
		switch (ev->jbutton.button) {
		case 1:	/* Add */
			nev.key.keysym.sym = SDLK_a;
			break;
		case 2: /* Delete */
			nev.key.keysym.sym = SDLK_d;
			break;
		case 4: /* Tile list up */
			nev.key.keysym.sym = SDLK_PAGEUP;
			break;
		case 5: /* Tile list up */
			nev.key.keysym.sym = SDLK_PAGEDOWN;
			break;
		}
		SDL_PushEvent(&nev);
		return;
	}

	mapx = med->x;
	mapy = med->y;

	/*
	 * Editor hotkeys.
	 */
	if (ev->type == SDL_KEYDOWN) {
		struct node *me;
		struct map_aref *aref = NULL;
		struct editref *eref;
		int moved = 0;

		if (pthread_mutex_lock(&em->lock) != 0) {
			perror(em->obj.name);
			return;
		}
		me = &em->map[mapx][mapy];

		switch (ev->key.keysym.sym) {
		case SDLK_a:
			/*
			 * Push a reference onto the stack. The mapedit
			 * cursor must be out of the way.
			 */
			mapedit_pointer(med, 0);
			pthread_mutex_lock(&med->curobj->lock);
			SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs,
			    med->curoffs);
			pthread_mutex_unlock(&med->curobj->lock);
#ifdef DEBUG
			/* XXX should not happen */
			if (eref == NULL) {
				fatal("no editor ref at %d\n", med->curoffs);

			}
#endif

			switch (eref->type) {
			case EDITREF_SPRITE:
				node_addref(me, med->curobj->pobj,
				    eref->spritei, MAPREF_SPRITE|MAPREF_SAVE);
				break;
			case EDITREF_ANIM:
				node_addref(me, med->curobj->pobj,
				    eref->animi, MAPREF_ANIM|MAPREF_SAVE);
				break;
			}
			me->flags = med->curflags;
			mapedit_pointer(med, 1);
			em->redraw++;
			break;
		case SDLK_d:
			/* Pop a reference off the stack. */
			if (me->narefs < 2) {
				dprintf("%dx%d: empty\n", mapx, mapy);
				break;
			}
			aref = node_arefindex(me, me->narefs - 2);
			node_delref(me, aref);
			em->redraw++;
			break;
		case SDLK_b:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setflag(med, me, MAPENTRY_BIO);
			} else {
				mapedit_setflag(med, me, MAPENTRY_BLOCK);
			}
			em->redraw++;
			break;
		case SDLK_w:
			mapedit_setflag(med, me, MAPENTRY_WALK);
			em->redraw++;
			break;
		case SDLK_c:
			mapedit_setflag(med, me, MAPENTRY_CLIMB);
			em->redraw++;
			break;
		case SDLK_p:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Toggle the map grid. */
				if (med->flags & MAPEDIT_DRAWPROPS) {
					med->flags &= ~(MAPEDIT_DRAWPROPS);
				} else {
					med->flags |= MAPEDIT_DRAWPROPS;
				}
				em->redraw++;
			} else {
				mapedit_setflag(med, me, MAPENTRY_SLIP);
				em->redraw++;
			}
			break;
		case SDLK_h:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setflag(med, me, MAPENTRY_HASTE);
				em->redraw++;
			}
			break;
		case SDLK_r:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setflag(med, me, MAPENTRY_REGEN);
				em->redraw++;
			}
			break;
		case SDLK_i:
			mapedit_pointer(med, 0);
			if (ev->key.keysym.mod & KMOD_CTRL) {
				struct editref *eref;

				pthread_mutex_lock(&med->curobj->lock);
				SIMPLEQ_INDEX(eref, &med->curobj->erefsh, erefs,
				    med->curoffs);
				pthread_mutex_unlock(&med->curobj->lock);

				switch (eref->type) {
				case EDITREF_SPRITE:
					map_clean(em, med->curobj->pobj,
					    eref->spritei, med->curflags,
					    MAPREF_SAVE|MAPREF_SPRITE);
					break;
				case EDITREF_ANIM:
					map_clean(em, med->curobj->pobj,
					    eref->animi, med->curflags,
					    MAPREF_SAVE|MAPREF_ANIM);
					break;
				}
			} else {
				map_clean(med->map, NULL, 0, 0, 0);
			}
			mapedit_pointer(med, 1);
			em->redraw++;
			break;
		case SDLK_o:
			/* Toggle the object list window. */
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Toggle the tile list window. */
				if (med->flags & MAPEDIT_OBJLIST) {
					med->flags &= ~(MAPEDIT_OBJLIST);
				} else {
					med->flags |= MAPEDIT_OBJLIST;
				}
				em->redraw++;
				break;
			}
			/* Goto the map origin. */
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				/* Become the map origin. */
				em->map[em->defx][em->defy].flags &=
				    ~MAPENTRY_ORIGIN;

				em->defx = mapx;
				em->defy = mapy;
				
				em->map[mapx][mapy].flags |= MAPENTRY_ORIGIN;
			}
			mapx = em->defx;
			mapy = em->defy;
			view_center(em->view, mapx, mapy);
			moved++;
			break;
		case SDLK_l:
			/* Load this map from file. */
			mapedit_pointer(med, 0);
			map_clean(em, NULL, 0, 0, 0);
			sprintf(path, "%s.map", em->obj.name);
			if (em->obj.load(med->map, path) == 0) {
				mapx = em->defx;
				mapy = em->defy;
				em->map[mapx][mapy].flags |= MAPENTRY_ORIGIN;
				view_center(em->view, mapx, mapy);
			}
			mapedit_pointer(med, 1);
			em->redraw++;
			break;
		case SDLK_t:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Toggle the tile list window. */
				if (med->flags & MAPEDIT_TILELIST) {
					med->flags &= ~(MAPEDIT_TILELIST);
				} else {
					med->flags |= MAPEDIT_TILELIST;
				}
				view_setmode(em->view);	/* XXX hack */
				em->redraw++;
			}
			break;
		case SDLK_s:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				/* Toggle the 'slow' attribute. */
				mapedit_setflag(med, me, MAPENTRY_SLOW);
				em->redraw++;
			} else if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Toggle tile stack window. */
				if (med->flags & MAPEDIT_TILESTACK) {
					med->flags &= ~(MAPEDIT_TILESTACK);
				} else {
					med->flags |= MAPEDIT_TILESTACK;
				}
				em->redraw++;
			} else {
				/* Save this map to file. */
				sprintf(path, "%s.map", em->obj.name);
				dprintf("saving %s...\n", path);
				em->obj.save((struct object *)em, path);
				dprintf("done\n");
			}
			break;
		case SDLK_g:
			/* Toggle the map grid. */
			if (med->flags & MAPEDIT_DRAWGRID) {
				med->flags &= ~(MAPEDIT_DRAWGRID);
			} else {
				med->flags |= MAPEDIT_DRAWGRID;
			}
			em->redraw++;
			break;
		case SDLK_x:
			mapedit_examine(em, mapx, mapy);
			break;
		default:
			break;
		}
	
		if (moved) {
			MAPEDIT_MOVE(med, mapx, mapy);
			em->redraw++;
		}

		pthread_mutex_unlock(&em->lock);
	}

	/*
	 * Directional keys.
	 */
	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		if (ev->type == SDL_KEYDOWN) {
			/* Move cursor up. */
			med->cursdir |= MAPEDIT_UP;
			med->cursdir &= ~(MAPEDIT_DOWN);
		} else if (ev->type == SDL_KEYUP) {
			object_wait(med, MAPEDIT_UP);
			med->cursdir &= ~(MAPEDIT_UP);
		}
		break;
	case SDLK_DOWN:
		if (ev->type == SDL_KEYDOWN) {
			/* Move cursor down. */
			med->cursdir |= MAPEDIT_DOWN;
			med->cursdir &= ~(MAPEDIT_UP);
		} else if (ev->type == SDL_KEYUP) {
			object_wait(med, MAPEDIT_DOWN);
			med->cursdir &= ~(MAPEDIT_DOWN);
		}
		break;
	case SDLK_LEFT:
		if (ev->type == SDL_KEYDOWN) {
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Move object list window left. */
				med->listodir |= MAPEDIT_CTRLLEFT;
				med->listodir &= ~(MAPEDIT_CTRLRIGHT);
			} else {
				/* Move cursor left. */
				med->cursdir |= MAPEDIT_LEFT;
				med->cursdir &= ~(MAPEDIT_RIGHT);
			}
			return;
		} else if (ev->type == SDL_KEYUP) {
			if (med->listodir != 0) {
				med->listodir &= ~(MAPEDIT_CTRLLEFT);
				object_wait(med, MAPEDIT_CTRLLEFT);
			} else {
				object_wait(med, MAPEDIT_LEFT);
				med->cursdir &= ~(MAPEDIT_LEFT);
			}
		}
		break;
	case SDLK_RIGHT:
		if (ev->type == SDL_KEYDOWN) {
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Move object list window right. */
				med->listodir |= MAPEDIT_CTRLRIGHT;
				med->listodir &= ~(MAPEDIT_CTRLLEFT);
			} else {
				/* Move cursor right */
				med->cursdir |= MAPEDIT_RIGHT;
				med->cursdir &= ~(MAPEDIT_LEFT);
			}
			return;
		} else if (ev->type == SDL_KEYUP) {
			if (med->listodir != 0) {
				med->listodir &= ~(MAPEDIT_CTRLRIGHT);
				object_wait(med, MAPEDIT_CTRLRIGHT);
			} else {
				object_wait(med, MAPEDIT_RIGHT);
				med->cursdir &= ~(MAPEDIT_RIGHT);
			}
		}
		break;
	case SDLK_PAGEUP:
		if (ev->type == SDL_KEYDOWN) {
			/* Move tile list window up. */
			med->listwdir |= MAPEDIT_UP;
			med->listwdir &= ~(MAPEDIT_DOWN);
		} else if (ev->type == SDL_KEYUP) {
			object_wait(med, MAPEDIT_PAGEUP);
			med->listwdir &= ~(MAPEDIT_UP);
		}
		break;
	case SDLK_PAGEDOWN:
		if (ev->type == SDL_KEYDOWN) {
			/* Move tile list window down. */
			med->listwdir |= MAPEDIT_DOWN;
			med->listwdir &= ~(MAPEDIT_UP);
		} else if (ev->type == SDL_KEYUP) {
			object_wait(med, MAPEDIT_PAGEDOWN);
			med->listwdir &= ~(MAPEDIT_DOWN);
		}
		break;
	default:
		break;
	}
}

static Uint32
mapedit_time(Uint32 ival, void *obp)
{
	struct object *ob = (struct object *)obp;
	struct mapedit *med = (struct mapedit *)ob;
	struct map *em = med->map;
	int mapx, mapy;

	if (pthread_mutex_lock(&em->lock) != 0) {
		perror(em->obj.name);
		return (ival);
	}

	mapx = med->x;
	mapy = med->y;

	/*
	 * Cursor direction.
	 */
	if (med->cursdir != 0) {
		if (med->cursdir & MAPEDIT_UP) {
			ob->wmask |= MAPEDIT_UP;
			decrease(&mapy, 1, 0);
			if(em->view->mapy - mapy >= 0) {
				SCROLL_UP(&em);
			}
		} else if (med->cursdir & MAPEDIT_DOWN) {
			ob->wmask |= MAPEDIT_DOWN;
			increase(&mapy, 1, em->maph - 1);
			if (em->view->mapy - mapy <=
			    -em->view->maph) {
				SCROLL_DOWN(&em);
			}
		}
		if (med->cursdir & MAPEDIT_LEFT) {
			ob->wmask |= MAPEDIT_LEFT;
			decrease(&mapx, 1, 0);
			if(em->view->mapx - mapx >= 0) {
				SCROLL_LEFT(&em);
			}
		} else if (med->cursdir & MAPEDIT_RIGHT) {
			ob->wmask |= MAPEDIT_RIGHT;
			increase(&mapx, 1, em->mapw - 1);
			if (em->view->mapx - mapx <=
			    -em->view->mapw) {
				SCROLL_RIGHT(&em);
			}
		}
		MAPEDIT_MOVE(med, mapx, mapy);
		em->redraw++;
	}

	/*
	 * Tile list window direction.
	 */
	if (med->listwdir != 0) {
		struct viewport *view;

		if (med->listwdir & MAPEDIT_UP) {
			ob->wmask |= MAPEDIT_PAGEUP;
			med->curoffs--;
			if (med->curoffs < 0) {
				med->curoffs = med->curobj->nrefs - 1;
			}
		} else if (med->listwdir & MAPEDIT_DOWN) {
			ob->wmask |= MAPEDIT_PAGEDOWN;
			med->curoffs++;
			if (med->curoffs > med->curobj->nrefs) {
				med->curoffs = 0;
			}
		}
		mapedit_tilelist(med);
		view = em->view;
		SDL_UpdateRect(view->v,
		    (view->width - em->view->tileh), 0,
		    em->view->tilew, view->height);
	}
	
	/*
	 * Object list window direction.
	 */
	if (med->listodir != 0) {
		if (TAILQ_EMPTY(&med->eobjsh)) {
			return (ival);
		}

		med->curoffs = 1;
		if (med->listodir & MAPEDIT_CTRLLEFT) {
			ob->wmask |= MAPEDIT_CTRLLEFT;

			med->curobj = TAILQ_PREV(med->curobj, eobjs_head,
			    eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_LAST(&med->eobjsh,
				    eobjs_head);
			}
		}
		if (med->listodir & MAPEDIT_CTRLRIGHT) {
			ob->wmask |= MAPEDIT_CTRLRIGHT;
			
			med->curobj = TAILQ_NEXT(med->curobj, eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_FIRST(&med->eobjsh);
			}
		}
		mapedit_objlist(med);
		SDL_UpdateRect(em->view->v,
		    0, (em->view->width - em->view->tilew),
		    em->view->width, em->view->tileh);
	}
	pthread_mutex_unlock(&em->lock);

	return (ival);
}
