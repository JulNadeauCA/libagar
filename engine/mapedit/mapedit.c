/*	$Csoft: mapedit.c,v 1.3 2002/01/25 15:06:51 vedge Exp $	*/

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
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <libfobj/fobj.h>

#include <engine/debug.h>
#include <engine/view.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/map.h>
#include <engine/xcf.h>

#include "mapedit.h"
#include "mapedit_offs.h"

static void	mapedit_destroy(struct object *);
static Uint32	mapedit_time(Uint32, void *);
static void	mapedit_event(struct object *, SDL_Event *);
static void	mapedit_toggle(struct mapedit *, struct map_entry *, int);
static void	mapedit_examine(struct map *, int, int);
static void	mapedit_pointer(struct object *, struct map_entry *, int);

struct mapedit *
mapedit_create(struct map *em, int x, int y)
{
	struct mapedit *med;
	struct fobj *fob;
	int i;

	med = (struct mapedit *) malloc(sizeof(struct mapedit));
	if (med == NULL) {
		perror("mapedit");
		return (NULL);
	}

	object_create(&med->obj, "mapedit", "Map editor",
	    EVENT_HOOK|DESTROY_HOOK);
	med->obj.event_hook = mapedit_event;
	med->obj.destroy_hook = mapedit_destroy;
	med->obj.map = em;
	med->obj.mapx = em->defx;
	med->obj.mapy = em->defy;

	med->curobj = NULL;
	med->curflags = 0;
	med->listwdir = 0;
	med->listodir = 0;
	med->cursdir = 0;
	med->flags = MAPEDIT_TILELIST|MAPEDIT_TILESTACK|MAPEDIT_OBJLIST;

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
	med->eobjs = NULL;
	med->neobjs = 0;

	for (i = 0; i < world->nobjs; i++) {
		struct object *ob;
		struct editobj *eob;
		
		ob = g_slist_nth_data(world->objs, i);

		if ((ob->flags & OBJ_EDITABLE) == 0) {
			continue;
		}

		eob = malloc(sizeof(struct editobj));
		if (eob == NULL) {
			perror("editobj");
			return (NULL);
		}

		eob->pobj = ob;
		eob->refs = NULL;
		eob->nsprites = ob->nsprites;
		eob->nanims = ob->nanims;
		eob->nrefs = ob->nsprites + ob->nanims;

		if (i == 0) {
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
					return (NULL);
				}
				eref->animi = -1;
				eref->spritei = y;
				eref->p = g_slist_nth_data(ob->sprites, y);
				eref->type = EDITREF_SPRITE;

				eob->refs = g_slist_append(eob->refs, eref);
			}
		
			if (ob->nanims > 0) {
				int z;

				for (z = 0; z < ob->nanims; z++) {
					struct editref *eref;

					eref = malloc(sizeof(struct editref));
					if (eref == NULL) {
						perror("editref");
						return (NULL);
					}
				
					eref->animi = z;
					eref->spritei = -1;
					eref->p =
					    g_slist_nth_data(ob->anims, z);
					eref->type = EDITREF_ANIM;

					eob->refs = g_slist_append(eob->refs,
					    eref);
				}
			}
		}
		med->eobjs = g_slist_append(med->eobjs, eob);
		med->neobjs++;
	}

	object_link(med);

	/* For tile property sprites. */
	curmapedit = med;

	/* Position mapedit within its parent map, at the origin. */
	med->obj.mapx = x;
	med->obj.mapy = y;

	if (pthread_mutex_lock(&em->lock) == 0) {
		struct map_entry *me;
		
		me = &em->map[em->defx][em->defy - 1];
		map_entry_addref(me, (struct object *)med,
		    MAPEDIT_SELECT, MAPREF_ANIM);
		pthread_mutex_unlock(&em->lock);
	} else {
		perror(em->obj.name);
	}

	view_center(em->view, em->defx, em->defy);
	em->redraw++;
	
	/* XXX tune */
	med->timer = SDL_AddTimer(em->view->fps + 120, mapedit_time, med);
	if (med->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		object_destroy(med, NULL);
		return (NULL);
	}

	/* XXX condition wait timeouts if an event occurs too early */

	return (med);
}

static void
mapedit_destroy(struct object *obp)
{
	struct mapedit *med = (struct mapedit *)obp;
	int i;

	SDL_RemoveTimer(med->timer);

	for (i = 0; i < med->neobjs; i++) {
		struct editobj *eob;
		int z;

		eob = g_slist_nth_data(med->eobjs, i);
		for (z = 0; z < eob->nrefs; z++) {
			free(g_slist_nth_data(eob->refs, z));
		}
	}
}

/*
 * Enable or disable the edition cursor. The map entry parameter must
 * point to its actual location. Assumes map is already locked.
 */
static void
mapedit_pointer(struct object *med, struct map_entry *me, int enable)
{
	if (enable) {
		map_entry_addref(me, med, MAPEDIT_SELECT, MAPREF_ANIM);
	} else {
		struct map_aref *aref;

		/* Look for a reference to mapedit:0, and remove it. */
		aref = map_entry_arefobj(me, med, MAPEDIT_SELECT);
#ifdef DEBUG
		if (aref == NULL) {
			fatal("cannot find mapedit:%d\n", MAPEDIT_SELECT);
			exit(1);
		}
#endif
		map_entry_delref(me, aref);
	}
}

/* Print a tile stack. Assumes the map is locked. */
static void
mapedit_examine(struct map *em, int x, int y)
{
	int i;
	struct map_aref *aref;
	struct map_entry *me;

	me = &em->map[x][y];

	printf("%dx%d: %d objects. < ", x, y, me->nobjs);

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
	for (i = 0; i < me->nobjs; i++) {
		aref = map_entry_aref(me, i);
		printf("\t[%2d] ", i);
		if (aref != NULL) {
			printf("%s:%d ", aref->pobj->name, aref->offs);
			if (aref->flags & MAPREF_SAVE)
				printf("saveable ");
			if (aref->flags & MAPREF_SPRITE)
				printf("sprite ");
			if (aref->flags & MAPREF_ANIM)
				printf("anim ");
		} else {
			printf("NULL");
		}
		printf("\n");
	}
}

static void
mapedit_toggle(struct mapedit *med, struct map_entry *me, int flag)
{
	if (me->nobjs < 2) {
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
			Uint32 c = col[((x ^ y) >> 3) & 1];

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
	static struct window *win;
	static int tilew, tileh;
	static SDL_Rect rs, rd;
	int i, sn;

	win = med->tilelist;
	tilew = med->obj.map->view->tilew;
	tileh = med->obj.map->view->tileh;
	
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
		 * index is negative, wrap. We always exclude the first
		 * reference (the map edition icon).
		 */
		if (sn > -1) {
			ref = g_slist_nth_data(med->curobj->refs, sn);
		} else {
			ref = g_slist_nth_data(med->curobj->refs,
			    sn + med->curobj->nrefs);
		}

		/* Plot the icon. */
		switch (ref->type) {
		case EDITREF_SPRITE:
			SDL_BlitSurface(ref->p, &rs, win->view->v, &rd);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			if (anim->nframes > 0) {
				SDL_BlitSurface(g_slist_nth_data(anim->frames,
				    0), &rs, win->view->v, &rd);
			}
			SDL_BlitSurface(g_slist_nth_data(
			    curmapedit->obj.sprites, MAPEDIT_ANIM),
			    &rs, win->view->v, &rd);
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
 * This must be called when the map is locked.
 */
void
mapedit_tilestack(struct mapedit *med)
{
	SDL_Rect rs, rd;
	struct window *win = med->tilestack;
	struct map_entry *me;
	static int tilew, tileh;
	int i, sn;

	me = &med->obj.map->map[med->obj.mapx][med->obj.mapy];
	tilew = med->obj.map->view->tilew;
	tileh = med->obj.map->view->tileh;

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

	/* Draw the tile stack. */
	for (i = 0, sn = me->nobjs;
	    sn >= 0 && (i < (win->height / tileh) - 1);
	    i++, sn--) {
		static struct map_aref *aref;
		
		aref = map_entry_aref(me, sn);
		if (aref == NULL) {
			continue;
		}
		if (aref->flags & MAPREF_ANIM) {
			struct anim *anim;
			/* Draw the first frame. */
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
	int i, sn;

	tilew = med->obj.map->view->tilew;
	tileh = med->obj.map->view->tileh;
	
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

	sn = g_slist_index(med->eobjs, med->curobj);

	/* Draw the circular object ring. */
	for (i = 0; rd.x <= win->width - tilew; i++) {
		struct editobj *eob;

		if (i >= med->neobjs) {
			i = 0;
		}

		eob = g_slist_nth_data(med->eobjs, i);

		SDL_BlitSurface(g_slist_nth_data(eob->pobj->sprites, 0), &rs,
		    win->view->v, &rd);
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
mapedit_event(struct object *ob, SDL_Event *ev)
{
	struct mapedit *med = (struct mapedit *)ob;
	struct map *em = ob->map;
	int mapx, mapy;
	char *path;

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
			SCROLL_LEFT(&ob->map);
			ob->map->redraw++;
		} else if (med->mmapx < ommapx) {
			SCROLL_RIGHT(&ob->map);
			ob->map->redraw++;
		}
		if (ommapy < med->mmapy) {
			SCROLL_UP(&ob->map);
			ob->map->redraw++;
		} else if (med->mmapy < ommapy) {
			SCROLL_DOWN(&ob->map);
			ob->map->redraw++;
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
			map_entry_moveref(em, ob,
			    MAPEDIT_SELECT,
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

	med = (struct mapedit *)ob;
	mapx = ob->mapx;
	mapy = ob->mapy;

	/*
	 * Editor hotkeys.
	 */
	if (ev->type == SDL_KEYDOWN) {
		struct map_entry *me;
		struct map_aref *aref = NULL;
		struct editref *edref;
		int redraw = 0;

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
			mapedit_pointer(ob, me, 0);
			edref = g_slist_nth_data(med->curobj->refs,
			    med->curoffs);

			switch (edref->type) {
			case EDITREF_SPRITE:
				map_entry_addref(me, med->curobj->pobj,
				    edref->spritei, MAPREF_SPRITE|MAPREF_SAVE);
				break;
			case EDITREF_ANIM:
				map_entry_addref(me, med->curobj->pobj,
				    edref->animi, MAPREF_ANIM|MAPREF_SAVE);
				break;
			}
			me->flags = med->curflags;	/* XXX? */
			mapedit_pointer(ob, me, 1);
			redraw++;
			break;
		case SDLK_d:
			/* Pop a reference off the stack. */
			if (me->nobjs < 2) {
				dprintf("%dx%d: empty\n", mapx, mapy);
				break;
			}
			aref = map_entry_aref(me, me->nobjs - 2);
			map_entry_delref(me, aref);
			redraw++;
			break;
		case SDLK_b:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_toggle(med, me, MAPENTRY_BIO);
			} else {
				mapedit_toggle(med, me, MAPENTRY_BLOCK);
			}
			redraw++;
			break;
		case SDLK_w:
			mapedit_toggle(med, me, MAPENTRY_WALK);
			redraw++;
			break;
		case SDLK_c:
			mapedit_toggle(med, me, MAPENTRY_CLIMB);
			redraw++;
			break;
		case SDLK_p:
			mapedit_toggle(med, me, MAPENTRY_SLIP);
			redraw++;
			break;
		case SDLK_h:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_toggle(med, me, MAPENTRY_HASTE);
				redraw++;
			}
			break;
		case SDLK_r:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_toggle(med, me, MAPENTRY_REGEN);
				redraw++;
			}
			break;
		case SDLK_i:
			mapedit_pointer(ob, me, 0);
			if (ev->key.keysym.mod & KMOD_CTRL) {
				struct editref *edref;

				edref = g_slist_nth_data(med->curobj->refs,
				    med->curoffs);
#ifdef DEBUG
				if (edref == NULL) {
					dprintf("no %d in %s\n", med->curoffs,
					    med->curobj->pobj->name);
				}
#endif

				switch (edref->type) {
				case EDITREF_SPRITE:
					map_clean(em, med->curobj->pobj,
					    edref->spritei, med->curflags,
					    MAPREF_SAVE|MAPREF_SPRITE);
					break;
				case EDITREF_ANIM:
					map_clean(em, med->curobj->pobj,
					    edref->animi, med->curflags,
					    MAPREF_SAVE|MAPREF_ANIM);
					break;
				}

			} else {
				map_clean(ob->map, NULL, 0, 0, 0);
			}
			mapedit_pointer(ob, me, 1);
			redraw++;
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
			redraw++;
			break;
		case SDLK_l:
			/* Load this map from file. */
			mapedit_pointer(ob, me, 0);
			map_clean(em, NULL, 0, 0, 0);
			path = g_strdup_printf("%s.map", em->obj.name);
			if (em->obj.load(ob->map, path) == 0) {
				mapx = em->defx;
				mapy = em->defy;
				em->map[mapx][mapy].flags |= MAPENTRY_ORIGIN;
				view_center(em->view, mapx, mapy);
			}
			free(path);
			mapedit_pointer(ob, me, 1);
			redraw++;
			break;
		case SDLK_t:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				/* Toggle the tile list window. */
				if (med->flags & MAPEDIT_TILELIST) {
					med->flags &= ~(MAPEDIT_TILELIST);
				} else {
					med->flags |= MAPEDIT_TILELIST;
				}
				em->redraw++;
			}
			break;
		case SDLK_s:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				/* Toggle the 'slow' attribute. */
				mapedit_toggle(med, me, MAPENTRY_SLOW);
				redraw++;
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
				path = g_strdup_printf("%s.map", em->obj.name);
				em->obj.save((struct object *)em, path);
				free(path);
			}
			break;
		case SDLK_x:
			mapedit_examine(em, mapx, mapy);
			break;
		default:
			break;
		}
	
		if (redraw) {
			map_entry_moveref(em, ob, MAPEDIT_SELECT, mapx, mapy);
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
		} else if (ev->type == SDL_KEYUP) {
			object_wait(med, MAPEDIT_RIGHT);
			med->cursdir &= ~(MAPEDIT_RIGHT);
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
	struct map_entry *me;
	int mapx, mapy;

	if (pthread_mutex_lock(&ob->map->lock) != 0) {
		perror(ob->map->obj.name);
		return (ival);
	}

	mapx = ob->mapx;
	mapy = ob->mapy;
	me = &ob->map->map[mapx][mapy];

	/*
	 * Cursor direction.
	 */
	if (med->cursdir != 0) {
		if (med->cursdir & MAPEDIT_UP) {
			decrease(&mapy, 1, 0);
			if(ob->map->view->mapy - mapy >= 0) {
				SCROLL_UP(&ob->map);
			}
			ob->wmask |= MAPEDIT_UP;
		} else if (med->cursdir & MAPEDIT_DOWN) {
			increase(&mapy, 1, ob->map->maph - 1);
			if (ob->map->view->mapy - mapy <=
			    -ob->map->view->maph) {
				SCROLL_DOWN(&ob->map);
			}
			ob->wmask |= MAPEDIT_DOWN;
		}
		if (med->cursdir & MAPEDIT_LEFT) {
			decrease(&mapx, 1, 0);
			if(ob->map->view->mapx - mapx >= 0) {
				SCROLL_LEFT(&ob->map);
			}
			ob->wmask |= MAPEDIT_LEFT;
		} else if (med->cursdir & MAPEDIT_RIGHT) {
			increase(&mapx, 1, ob->map->mapw - 1);
			if (ob->map->view->mapx - mapx <=
			    -ob->map->view->mapw) {
				SCROLL_RIGHT(&ob->map);
			}
			ob->wmask |= MAPEDIT_RIGHT;
		}
		map_entry_moveref(ob->map, ob, MAPEDIT_SELECT, mapx, mapy);
		ob->map->redraw++;
	}

	/*
	 * Tile list window direction.
	 */
	if (med->listwdir != 0) {
		struct viewport *view;

		if (med->listwdir & MAPEDIT_UP) {
			med->curoffs--;
			if (med->curoffs < 0) {
				med->curoffs = med->curobj->nrefs - 1;
			}
			ob->wmask |= MAPEDIT_PAGEUP;
		} else if (med->listwdir & MAPEDIT_DOWN) {
			med->curoffs++;
			if (med->curoffs > med->curobj->nrefs) {
				med->curoffs = 0;
			}
			ob->wmask |= MAPEDIT_PAGEDOWN;
		}
		mapedit_tilelist(med);
		view = ob->map->view;
		SDL_UpdateRect(view->v,
		    (view->width - ob->map->view->tileh), 0,
		    ob->map->view->tilew, view->height);
	}
	
	/*
	 * Object list window direction.
	 */
	if (med->listodir != 0) {
		struct viewport *view;

		if (med->listodir & MAPEDIT_CTRLLEFT) {
			static int pobjnum;

			pobjnum = g_slist_index(med->eobjs, med->curobj);
			dprintf("pobjnum %d..\n", pobjnum);
			if (--pobjnum < 0) {
				pobjnum = med->neobjs - 1;
			}
			dprintf("-> %d\n", pobjnum);
			med->curobj = g_slist_nth_data(med->eobjs, pobjnum);
			dprintf("curobj is now %s\n", med->curobj->pobj->name);
			med->curoffs = 1;

			ob->wmask |= MAPEDIT_CTRLLEFT;
			ob->map->redraw++;
		}
		view = ob->map->view;
		SDL_UpdateRect(view->v,
		    0, (view->width - ob->map->view->tilew),
		    view->width, ob->map->view->tileh);
	}
	pthread_mutex_unlock(&ob->map->lock);

	return (ival);
}
