/*	$Csoft: mapedit.c,v 1.24 2002/02/10 05:03:19 vedge Exp $	*/

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
#include "command.h"
#include "mouse.h"
#include "joy.h"

static void	mapedit_destroy(void *);
static int	mapedit_shadow(struct mapedit *);
static Uint32	mapedit_time(Uint32, void *);
static void	mapedit_event(struct mapedit *, SDL_Event *);
static void	mapedit_bg(SDL_Surface *, int, int, int, int);

static const int stickykeys[] = {
	SDLK_a,	/* Add */
	SDLK_d,	/* Del */
	SDLK_b,	/* Block */
	SDLK_w,	/* Walk */
	SDLK_c,	/* Climb */
	SDLK_p	/* Slippery */
};

struct mapedit *curmapedit;

struct mapedit *
mapedit_create(char *name, char *desc, int mapw, int maph)
{
	char path[FILENAME_MAX];
	struct mapedit *med;
	struct map *map;
	struct fobj *fob;
	int fd;
	int tilew, tileh;

	strcpy(path, name);
	strcat(path, ".map");
	
	if ((fd = open(path, O_RDONLY, 0)) > 0) {
		close(fd);
		dprintf("editing %s from %s\n", name, path);
		/* The description, geometry and flags will be loaded. */
		map = map_create(name, NULL, MAP_2D, 128, 128);
		map_load(map, path);
	} else {
		/* Create a new map. */
		dprintf("creating %s anew\n", name);
		map = map_create(name, desc, MAP_2D, mapw, maph);
	}
	if (object_strfind(name) == NULL) {
		dprintf("%s is not in core\n", name);
		map_link(map);
	} else {
		dprintf("%s is in core\n", name);
	}

	med = (struct mapedit *)malloc(sizeof(struct mapedit));
	if (med == NULL) {
		perror("mapedit");
		return (NULL);
	}

	object_create(&med->obj, "mapedit", "Map editor", DESTROY_HOOK);
	med->obj.destroy_hook = mapedit_destroy;
	med->event_hook = mapedit_event;
	med->map = map;
	med->x = map->defx;
	med->y = map->defy;

	med->curobj = NULL;
	med->curflags = 0;

	direction_init(&med->cursor_dir, med, DIR_SCROLL, 2, 10);
	direction_init(&med->listw_dir, med, DIR_SCROLL, 2, 10);
	direction_init(&med->olistw_dir, med, 0, 2, 10);

	med->flags = MAPEDIT_TILELIST|MAPEDIT_TILESTACK|MAPEDIT_OBJLIST|
	    MAPEDIT_DRAWPROPS;

	tilew = map->view->tilew;
	tileh = map->view->tileh;

	med->tilelist = window_create(map->view,
	    (map->view->width - tilew), tileh,
	    tilew, map->view->height + tileh,
	    "Tile list");
	med->tilestack = window_create(map->view,
	    0, tileh,
	    tilew, map->view->height + tileh,
	    "Tile stack");
	med->objlist = window_create(map->view,
	    tilew, 0,
	    map->view->width - tilew, tileh,
	    "Object list");

	fob = fobj_load("../engine/mapedit/mapedit.fob");
	xcf_load(fob, MAPEDIT_XCF, (struct object *)med);
	fobj_free(fob);

	mapedit_shadow(med);
	dprintf("%s: editing %d object(s)\n", med->obj.name, med->neobjs);

	object_link(med);

	if (pthread_mutex_lock(&map->lock) == 0) {
		/* Position mapedit on the map. */
		MAPEDIT_PLOT(med, map, map->defx, map->defy);
		pthread_mutex_unlock(&map->lock);
	}

	curmapedit = med;
	
	/* XXX tune */
	med->timer = SDL_AddTimer(30, mapedit_time, med);
	if (med->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		return (NULL);
	}
	
	view_setmode(map->view);	/* XXX hack */
	view_center(map->view, map->defx, map->defy);
	
	map->redraw++;
	
	return (med);
}

static int
mapedit_shadow(struct mapedit *med)
{
	struct object *ob;

	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;

	med->curobj = NULL;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		struct editobj *eob;
		
		dprintf("shadow \"%s\"\n", ob->name);

		if ((ob->flags & OBJ_EDITABLE) == 0) {
			dprintf("skipping %s (non-editable)\n", ob->name);
			continue;
		}
		if (ob->nsprites < 1 || ob->nanims < 1) {
			dprintf("skipping %s (no sprite/anim)\n", ob->name);
			continue;
		}

		eob = malloc(sizeof(struct editobj));
		if (eob == NULL) {
			goto fail;
		}

		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefsh);
		eob->nrefs = 0;
		eob->nsprites = ob->nsprites;
		eob->nanims = ob->nanims;
		if (pthread_mutex_init(&eob->lock, NULL) != 0) {
			goto fail;
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
					goto fail;
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
					goto fail;
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

	if (med->curobj == NULL) {
		fatal("%s: nothing to edit!\n", med->obj.name);
		return (-1);
	}
fail:
	pthread_mutex_unlock(&world->lock);
	return (-1);
}

static void
mapedit_destroy(void *p)
{
	struct mapedit *med = (struct mapedit *)p;
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

static void
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
	    win->view->height + tileh); /* XXX */
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

#ifdef DEBUG
		if (ref == NULL) {
			/* XXX should not happen */
			fatal("NULL ref\n");
		}
#endif

		/* Plot the icon. */
		switch (ref->type) {
		case EDITREF_SPRITE:
			SDL_BlitSurface(ref->p, &rs, win->view->v, &rd);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			SDL_BlitSurface(anim->frames[0],
			    &rs, win->view->v, &rd);
			break;
		}

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
	SDL_UpdateRect(med->map->view->v,
	    (med->map->view->width - med->map->view->tileh), 0,
	    med->map->view->tilew, med->map->view->height);
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

			anim = g_slist_nth_data(aref->pobj->anims, aref->offs);
			SDL_BlitSurface(anim->frames[0],
			    &rs, win->view->v, &rd);
		} else if (aref->flags & MAPREF_SPRITE) {
			SDL_BlitSurface(g_slist_nth_data(aref->pobj->sprites,
			    aref->offs), &rs, win->view->v, &rd);
		}

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
	SDL_UpdateRect(med->map->view->v,
	    0, (med->map->view->width - tilew),
	    med->map->view->width, tileh);
}

static void
mapedit_event(struct mapedit *med, SDL_Event *ev)
{
	struct map *map = med->map;
	struct node *node;
	int mapx, mapy;
	
	switch (ev->type) {
	case SDL_MOUSEMOTION:
		if (ev->motion.state == SDL_PRESSED) {
			mouse_motion(med, ev);
		}
		return;
	case SDL_MOUSEBUTTONDOWN:
		if (ev->button.button != 1) {
			mouse_button(med, ev);
		}
		return;
	case SDL_JOYAXISMOTION:
		joy_axismotion(med, ev);
		return;
	case SDL_JOYBUTTONUP:
	case SDL_JOYBUTTONDOWN:
		joy_button(med, ev);
		return;
	}

	mapx = med->x;
	mapy = med->y;

	pthread_mutex_lock(&map->lock);

	node = &map->map[mapx][mapy];

	/*
	 * Editor hotkeys.
	 */
	if (ev->type == SDL_KEYDOWN) {
		switch (ev->key.keysym.sym) {
		case SDLK_a:
			mapedit_push(med, node);
			break;
		case SDLK_d:
			mapedit_pop(med, node);
			break;
		case SDLK_b:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_BIO);
			} else {
				mapedit_nodeflags(med, node, NODE_BLOCK);
			}
			break;
		case SDLK_w:
			mapedit_nodeflags(med, node, NODE_WALK);
			break;
		case SDLK_c:
			mapedit_nodeflags(med, node, NODE_CLIMB);
			break;
		case SDLK_p:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_editflags(med, MAPEDIT_DRAWPROPS);
			} else {
				mapedit_nodeflags(med, node, NODE_SLIP);
			}
			break;
		case SDLK_h:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_HASTE);
			}
			break;
		case SDLK_r:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_REGEN);
			}
			break;
		case SDLK_i:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_fillmap(med);
			} else {
				map_clean(med->map, NULL, 0, 0, 0);
				med->map->redraw++;
			}
			break;
		case SDLK_o:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_editflags(med, MAPEDIT_OBJLIST);
			} else if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setorigin(med, &mapx, &mapy);
				MAPEDIT_MOVE(med, mapx, mapy);
			}
			break;
		case SDLK_t:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_editflags(med, MAPEDIT_TILELIST);
			}
			break;
		case SDLK_l:
			mapedit_load(med);
			break;
		case SDLK_s:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_SLOW);
			} else if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_editflags(med, MAPEDIT_TILESTACK);
			} else {
				mapedit_save(med);
			}
			break;
		case SDLK_g:
			mapedit_editflags(med, MAPEDIT_DRAWGRID);
			break;
		case SDLK_x:
			mapedit_examine(map, mapx, mapy);
			break;
		default:
			break;
		}
	}

	if (ev->type == SDL_KEYDOWN || ev->type == SDL_KEYUP) {
		int set;
		struct map_aref *aref;

		set = (ev->type == SDL_KEYDOWN) ? 1 : 0;
		aref = node_arefobj(&map->map[mapx][mapy],
		    (struct object *)med, -1);

		switch (ev->key.keysym.sym) {
		case SDLK_UP:
			direction_set(&med->cursor_dir, aref, DIR_UP, set);
			break;
		case SDLK_DOWN:
			direction_set(&med->cursor_dir, aref, DIR_DOWN, set);
			break;
		case SDLK_LEFT:
			direction_set(&med->cursor_dir, aref, DIR_LEFT, set);
			break;
		case SDLK_RIGHT:
			direction_set(&med->cursor_dir, aref, DIR_RIGHT, set);
			break;
		case SDLK_PAGEUP:
			direction_set(&med->listw_dir, aref, DIR_UP, set);
			break;
		case SDLK_PAGEDOWN:
			direction_set(&med->listw_dir, aref, DIR_DOWN, set);
			break;
		case SDLK_DELETE:
			direction_set(&med->olistw_dir, aref, DIR_LEFT, set);
			break;
		case SDLK_END:
			direction_set(&med->olistw_dir, aref, DIR_RIGHT, set);
			break;
		default:
			break;
		}
	}
	pthread_mutex_unlock(&map->lock);
}

static Uint32
mapedit_time(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	struct map *map = med->map;
	struct map_aref *aref;
	int mapx, mapy, moved;
	
	mapx = med->x;
	mapy = med->y;

	aref = node_arefobj(&map->map[mapx][mapy], (struct object *)med, -1);

	pthread_mutex_lock(&map->lock);

	moved = direction_update(&med->cursor_dir, map, &mapx, &mapy, aref);
	if (moved != 0) {
		static int i, nkeys;
		static SDL_Event nev;

		MAPEDIT_MOVE(med, mapx, mapy);
		map->redraw++;

		for (i = 0; i < sizeof(stickykeys) / sizeof(int); i++) {
			if ((SDL_GetKeyState(&nkeys))[stickykeys[i]]) {
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = stickykeys[i];
				SDL_PushEvent(&nev);
			}
		}
	}

#if 0
	moved = direction_update(&med->listw_dir, NULL, NULL, NULL, NULL);
	if (moved & DIR_UP) {
		if (med->curoffs-- < 0) {
			med->curoffs = med->curobj->nrefs - 1;
		}
		mapedit_tilelist(med);
	}
	if (moved & DIR_DOWN) {
		if (med->curoffs++ > med->curobj->nrefs - 1) {
			med->curoffs = 0;
		}
		mapedit_tilelist(med);
	}

	moved = direction_update(&med->olistw_dir, NULL, NULL, NULL, NULL);
	if (moved & DIR_LEFT) {
		if (!TAILQ_EMPTY(&med->eobjsh)) {
			med->curobj = TAILQ_PREV(med->curobj, eobjs_head,
			    eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_LAST(&med->eobjsh,
				     eobjs_head);
			}
		}
		mapedit_objlist(med);
	}
	if (moved & DIR_RIGHT) {
		if (!TAILQ_EMPTY(&med->eobjsh)) {
			med->curobj = TAILQ_NEXT(med->curobj, eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_FIRST(&med->eobjsh);
			}
		}
		mapedit_objlist(med);
	}
#endif
	pthread_mutex_unlock(&map->lock);

	return (ival);
}
