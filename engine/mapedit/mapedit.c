/*	$Csoft: mapedit.c,v 1.64 2002/03/14 04:50:13 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/text/text.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "command.h"
#include "key.h"
#include "mouse.h"
#include "joy.h"

static struct obvec mapedit_vec = {
	mapedit_destroy,
	mapedit_load,
	mapedit_save,
	mapedit_link,
	mapedit_unlink
};

enum {
	DEFAULT_CURSOR_SPEED = 40,	/* Cursor speed timer */
	DEFAULT_LISTW_SPEED = 16	/* List scrolling timer */
};

struct mapedit *curmapedit;		/* Map editor currently controlled. */
static const int stickykeys[] = {	/* Keys applied after each move. */
	SDLK_a,	/* Add */
	SDLK_d,	/* Del */
	SDLK_b,	/* Block */
	SDLK_w,	/* Walk */
	SDLK_c,	/* Climb */
	SDLK_p	/* Slippery */
};

static pthread_mutex_t keyslock =	/* Keys can be processed. */
    PTHREAD_MUTEX_INITIALIZER;

enum {
	BG_HORIZONTAL,
	BG_VERTICAL
};

static int	mapedit_shadow(struct mapedit *);
static Uint32	mapedit_cursor_tick(Uint32, void *);
static Uint32	mapedit_lists_tick(Uint32, void *);
static void	mapedit_bg(SDL_Surface *, SDL_Rect *, Uint32);
static void	mapedit_state(struct mapedit *, SDL_Rect *);

struct mapedit *
mapedit_create(char *name)
{
	struct mapedit *med;
	struct fobj *fob;

	med = (struct mapedit *)emalloc(sizeof(struct mapedit));
	object_init(&med->obj, name, 0, &mapedit_vec);
	med->obj.desc = strdup("map editor");
	med->flags = MAPEDIT_DRAWPROPS;
	med->map = NULL;
	med->x = 0;
	med->y = 0;
	med->mmapx = 0;
	med->mmapy = 0;
	med->mtmapx = 0;
	med->mtmapy = 0;
	med->cursor_speed = DEFAULT_CURSOR_SPEED;
	med->listw_speed = DEFAULT_LISTW_SPEED;
	
	med->curobj = NULL;
	med->curoffs = 0;
	med->curflags = 0;

	mapdir_init(&med->cursor_dir, (struct object *)med, NULL, -1, -1);
	gendir_init(&med->listw_dir);
	gendir_init(&med->olistw_dir);

	fob = fobj_load(savepath("mapedit", "fob"));
	if (fob == NULL) {
		return (NULL);
	}
	xcf_load(fob, MAPEDIT_XCF, (struct object *)med);
	fobj_free(fob);

	return (med);
}

static int
mapedit_shadow(struct mapedit *med)
{
	struct object *ob;

	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;

	med->curobj = NULL;
	med->curoffs = 0;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		struct editobj *eob;
		
		if ((ob->flags & OBJ_EDITABLE) == 0) {
			dprintf("skipping %s (non-editable)\n", ob->name);
			continue;
		}
		if (ob->nsprites < 1 && ob->nanims < 1) {
			dprintf("skipping %s (no sprite/anim)\n", ob->name);
			continue;
		}

		eob = emalloc(sizeof(struct editobj));
		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefsh);
		eob->nrefs = 0;
		eob->nsprites = ob->nsprites;
		eob->nanims = ob->nanims;
		if (pthread_mutex_init(&eob->lock, NULL) != 0) {
			goto fail;
		}

		dprintf("%s: %d sprites, %d anims\n", ob->name,
		    eob->nsprites, eob->nanims);

		/* XXX for now */
		if (eob->nsprites > 0) {
			med->curobj = eob;
			med->curoffs = 1;
		}
		
		if (eob->nsprites > 0) {
			Uint32 y;

			for (y = 0; y < eob->nsprites; y++) {
				struct editref *eref;
		
				eref = emalloc(sizeof(struct editref));
				eref->animi = -1;
				eref->spritei = y;
				eref->p = ob->sprites[y];
				eref->type = EDITREF_SPRITE;

				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}
	
		if (eob->nanims > 0) {
			Uint32 z;

			for (z = 0; z < eob->nanims; z++) {
				struct editref *eref;

				eref = emalloc(sizeof(struct editref));
				eref->animi = z;
				eref->spritei = -1;
				eref->p = ob->anims[z];
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
	return (0);
fail:
	pthread_mutex_unlock(&world->lock);
	return (-1);
}

void
mapedit_setcaption(struct mapedit *med, char *path)
{
	static char caption[FILENAME_MAX];
	struct map *m = med->map;

	if (object_strfind(med->margs.name) == NULL) {
		sprintf(caption, "%s [unknown] (%s)", m->obj.name, path);
		object_link(m);
	} else {
		sprintf(caption, "%s [%d] (%s)", m->obj.name, m->obj.id, path);
	}
	SDL_WM_SetCaption(caption, "agar");
}

int
mapedit_link(void *p)
{
	char path[FILENAME_MAX];
	struct mapedit *med = (struct mapedit *)p;
	struct map *m = med->map;
	struct node *node;
	int fd, new = 0;
	
	/* Users must copy maps to udatadir in order to edit them. */
	sprintf(path, "%s/%s.m", world->udatadir, med->margs.name);

	/* XXX specify a view in arguments? */
	/* XXX do this in mapedit_link()? */
	if ((fd = open(path, O_RDONLY, 0)) > 0) {
		close(fd);

		m = map_create(med->margs.name, NULL, 0);
		object_loadfrom(m, path);
	} else {
		struct node *origin;
		
		/* Create a new map of the specified geometry. */
		m = map_create(med->margs.name, med->margs.desc, MAP_2D);
		map_allocnodes(m, med->margs.mapw, med->margs.maph,
		    med->margs.tilew, med->margs.tileh);

		m->defx = med->margs.mapw / 2;
		m->defy = med->margs.maph - 2;
		origin = &m->map[m->defx][m->defy];
		origin->flags |= NODE_ORIGIN;
		new++;
	}

	med->map = m;
	med->x = m->defx;
	med->y = m->defy;
	mapdir_init(&med->cursor_dir, (struct object *)med, m,
	    DIR_SCROLLVIEW|DIR_SOFTSCROLL, 9);
	med->tilelist.x = m->view->width - m->tilew;
	med->tilelist.y = m->tileh;
	med->tilelist.w = m->tilew;
	med->tilelist.h = m->view->height;
	med->tilelist_offs = 0;

	med->tilestack.x = 0;
	med->tilestack.y = m->tileh;
	med->tilestack.w = m->tilew;
	med->tilestack.h = m->view->height;

	med->objlist.x = m->tilew;
	med->objlist.y = 0;
	med->objlist.w = m->view->width - m->tilew;
	med->objlist.h = m->tileh;
	med->objlist_offs = 0;

	view_setmode(m->view, m, VIEW_MAPEDIT, NULL);
	view_center(m->view, m->defx, m->defy);
	mapedit_setcaption(med, new ? "new" : path);

	text_msg(1000, TEXT_SLEEP,
	    "Editing \"%s\" (%s)\n", m->obj.name, new ? "new" : path);

	/* Create the structures defining what is editable. */
	mapedit_shadow(med);

	pthread_mutex_lock(&m->lock);
	node = &m->map[m->defx][m->defy];
	node_addref(node, med, MAPEDIT_SELECT,
	    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
	node->flags |= (NODE_ANIM|NODE_ORIGIN);
	pthread_mutex_unlock(&m->lock);
	
	/* Map is now in a consistent state. */
	map_focus(m);
	m->redraw++;

	med->timer = SDL_AddTimer(med->cursor_speed, mapedit_cursor_tick, med);
	if (med->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		return (-1);
	}
	med->timer = SDL_AddTimer(med->listw_speed, mapedit_lists_tick, med);
	if (med->timer == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		return (-1);
	}

	curmapedit = med;
	dprintf("editing %d object(s)\n", med->neobjs);

	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);
	mapedit_objlist(med);
	mapedit_tilelist(med);
	pthread_mutex_unlock(&med->map->lock);
	pthread_mutex_unlock(&keyslock);

	return (0);
}

int
mapedit_unlink(void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	struct map *m = med->map;
	struct node *node;
	struct noderef *nref;

	SDL_RemoveTimer(med->timer);
	SDL_Delay(100);	/* XXX */
	curmapedit = NULL;

	pthread_mutex_lock(&m->lock);
	node = &m->map[med->x][med->y];
	nref = node_findref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
	if (nref != NULL) {
		node_delref(node, nref);
		node->flags &= ~(NODE_ANIM);
	}
	pthread_mutex_unlock(&m->lock);

	m->redraw++;
	return (0);
}

int
mapedit_destroy(void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	struct editobj *eob;

	mapedit_unlink(p);

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
	return (0);
}

static void
mapedit_bg(SDL_Surface *v, SDL_Rect *rd, Uint32 flags)
{
	Uint8 *dst = v->pixels;
	Uint32 x, y, cx, cy;

	for (y = rd->y, cy = 0; y < rd->y + rd->h; y++, cy++) {
		for (x = rd->x, cx = 0; x < rd->x + rd->w; x++, cx++) {
			static Uint32 c;
			
			if (flags & BG_VERTICAL) {
				c = SDL_MapRGB(v->format, cx, 0, cy >> 2);
			} else {
				c = SDL_MapRGB(v->format, cy, 0, cx >> 2);
			}

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
					dst[x*3] = c;
					dst[x*3 + 1] = c>>8;
					dst[x*3 + 2] = c>>16;
				} else {
					dst[x*3] = c>>16;
					dst[x*3 + 1] = c>>8;
					dst[x*3 + 2] = c;
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

/*
 * Draw the map edition status indicator.
 */
static void
mapedit_state(struct mapedit *med, SDL_Rect *rd)
{
	if (med->flags & MAPEDIT_INSERT) {
		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_INSERT_TXT],
		    NULL, med->map->view->v, rd);
	} else {
		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_REPLACE_TXT],
		    NULL, med->map->view->v, rd);
	}
	if (med->flags & MAPEDIT_DRAWPROPS)
		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_PROPS_TXT],
		    NULL, med->map->view->v, rd);
	if (med->flags & MAPEDIT_DRAWGRID)
		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_GRID_TXT],
		    NULL, med->map->view->v, rd);
	if (med->curflags & NODE_BLOCK) {
		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_BLOCKED],
		    NULL, med->map->view->v, rd);
	} else {
		if (med->curflags & NODE_WALK) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_WALK],
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_CLIMB) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_CLIMB],
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_SLIP) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_SLIP],
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_BIO) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_BIO],
			    NULL, med->map->view->v, rd);
		} else if (med->curflags & NODE_REGEN) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_REGEN],
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_SLOW) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_SLOW],
			    NULL, med->map->view->v, rd);
		} else if (med->curflags & NODE_HASTE) {
			SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_HASTE],
			    NULL, med->map->view->v, rd);
		}
	}
}

/*
 * Draw the tile selection.
 * Must be called on a locked map.
 */
void
mapedit_tilelist(struct mapedit *med)
{
	struct map *m = med->map;
	static SDL_Rect rd;
	Uint32 i;
	int sn;
	
	rd = med->tilelist;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_VERTICAL);
	rd.h = m->tilew;
	rd.y = m->tileh;

	/*
	 * Draw the sprite/anim list in a circular fashion. We must
	 * predict which sprite to draw first according to the window
	 * geometry.
	 */
	for (i = 0, sn = med->tilelist_offs;
	     i < (med->tilelist.h / rd.h) - 1;
	     i++, rd.y += m->tileh) {
		struct editref *ref;
		struct anim *anim;

		pthread_mutex_lock(&med->curobj->lock);
		if (sn > -1) {
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn);
		} else {
			/* Wrap */
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn + med->curobj->nrefs);
			if (ref == NULL) {
				/* XXX hack */
				pthread_mutex_unlock(&med->curobj->lock);
				goto nextref;
			}
		}
		pthread_mutex_unlock(&med->curobj->lock);

		if (ref == NULL) {
			goto nextref;
		}

		switch (ref->type) {
		case EDITREF_SPRITE:
			SDL_BlitSurface(ref->p, NULL, m->view->v, &rd);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			SDL_BlitSurface(anim->frames[0], NULL, m->view->v, &rd);
			break;
		}

		if (med->curoffs == sn) {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_CIRQSEL], NULL,
			    m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_GRID], NULL,
			    m->view->v, &rd);
		}
nextref:
		if (++sn >= med->curobj->nrefs) {
			sn = 0;
		}
	}

	rd.x = med->tilelist.x;
	rd.y = med->tilelist.y - m->tileh;
	rd.w = m->tilew;
	rd.h = m->tileh;
	mapedit_state(med, &rd);
	
	SDL_UpdateRect(m->view->v,
	    med->tilelist.x - m->tilew, med->tilelist.y - m->tileh,
	    med->tilelist.w + m->tilew, med->tilelist.h + m->tileh);
}

/*
 * Draw the tile stack for the map entry under the cursor.
 * Must be called on a locked map.
 */
void
mapedit_tilestack(struct mapedit *med)
{
	static SDL_Rect rd;
	struct map *m = med->map;
	struct noderef *nref;
	Uint32 i;

	rd = med->tilestack;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_VERTICAL);
	rd.h = m->tileh;

	i = 0;
	TAILQ_FOREACH(nref, &(&m->map[med->x][med->y])->nrefsh, nrefs) {
		if (++i > (med->tilestack.h / rd.h) - 1) {
			return;
		}
		if (nref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			anim = nref->pobj->anims[nref->offs];
			SDL_BlitSurface(anim->frames[0],
			    NULL, m->view->v, &rd);
			if (nref->flags & MAPREF_ANIM_DELTA) {
				SDL_BlitSurface(
				    SPRITE(med, MAPEDIT_ANIM_DELTA_TXT), NULL,
				    m->view->v, &rd);
			} else if (nref->flags & MAPREF_ANIM_INDEPENDENT) {
				SDL_BlitSurface(
				    SPRITE(med, MAPEDIT_ANIM_INDEPENDENT_TXT),
				    NULL, m->view->v, &rd);
			}
			SDL_BlitSurface(SPRITE(med, MAPEDIT_ANIM_TXT), NULL,
			    m->view->v, &rd);
		} else if (nref->flags & MAPREF_SPRITE) {
			SDL_BlitSurface(nref->pobj->sprites[nref->offs],
			    NULL, m->view->v, &rd);
		}

		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_GRID],
		    NULL, m->view->v, &rd);
		rd.y += m->tileh;
	}
	SDL_UpdateRect(m->view->v,
	    med->tilestack.x, med->tilestack.y,
	    med->tilestack.w, med->tilestack.h);
}

/*
 * Draw a list of objects in the object ring.
 * This must be called when the world is locked.
 */
void
mapedit_objlist(struct mapedit *med)
{
	static SDL_Rect rd;
	struct map *m = med->map;
	struct editobj *eob;
	Uint32 i;
	int sn;

	rd = med->objlist;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_HORIZONTAL);
	rd.w = m->tilew;
	rd.x = m->tilew;

	for (i = 0, sn = med->objlist_offs;
	     i < (med->objlist.w / m->tilew) - 1;
	     i++, rd.x += m->tilew) {

		if (sn > -1) {
			TAILQ_INDEX(eob, &med->eobjsh, eobjs, sn);
		} else {
			/* Wrap */
			TAILQ_INDEX(eob, &med->eobjsh, eobjs,
			    sn + med->neobjs);
			if (eob == NULL) {
				/* XXX hack */
				goto nextref;
			}
		}
	
		SDL_BlitSurface(eob->pobj->sprites[0], NULL, m->view->v, &rd);

		if (med->curobj == eob) {
			SDL_BlitSurface(
			    med->obj.sprites[MAPEDIT_CIRQSEL],
			    NULL, m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    med->obj.sprites[MAPEDIT_GRID],
			    NULL, m->view->v, &rd);
		}
nextref:
		if (++sn >= med->neobjs) {
			sn = 0;
		}
	}
	SDL_UpdateRect(m->view->v,
	    med->objlist.x, med->objlist.y,
	    med->objlist.w, med->objlist.h);
}

void
mapedit_event(void *ob, SDL_Event *ev)
{
	struct mapedit *med = ob;

	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);
	
	switch (ev->type) {
	case SDL_MOUSEMOTION:
		mouse_motion(med, ev);
		break;
	case SDL_MOUSEBUTTONDOWN:
		mouse_button(med, ev);
		break;
	case SDL_JOYAXISMOTION:
		joy_axismotion(med, ev);
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		joy_button(med, ev);
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		mapedit_key(med, ev);
		break;
	}

	pthread_mutex_unlock(&med->map->lock);
	pthread_mutex_unlock(&keyslock);
}

int
mapedit_load(void *p, int fd)
{
	struct mapedit *med = (struct mapedit *)p;

	if (version_read(fd, "mapedit", 1, 0) != 0) {
		return (-1);
	}
	med->flags = fobj_read_uint32(fd);
	med->cursor_speed = fobj_read_uint32(fd);
	med->listw_speed = fobj_read_uint32(fd);

	mapedit_tilelist(med);

	return (0);
}

int
mapedit_save(void *p, int fd)
{
	struct mapedit *med = (struct mapedit *)p;

	version_write(fd, "mapedit", 1, 0);
	fobj_write_uint32(fd, med->flags);
	fobj_write_uint32(fd, med->cursor_speed);
	fobj_write_uint32(fd, med->listw_speed);

	return (0);
}

/*
 * Move the map edition cursor.
 * Must be called on a locked map.
 */
void
mapedit_move(struct mapedit *med, Uint32 x, Uint32 y)
{
	struct node *node;

	node = &med->map->map[med->x][med->y];
	node_delref(node, node_findref(node, med, MAPEDIT_SELECT, MAPREF_ANIM));
	node->flags &= ~(NODE_ANIM);
	
	node = &med->map->map[x][y];
	node_addref(node, med, MAPEDIT_SELECT,
	    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
	node->flags |= NODE_ANIM;

	med->x = x;
	med->y = y;
}

static Uint32
mapedit_cursor_tick(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	Uint32 x, y, moved;

	if (curmapedit == NULL) {
		return (0);
	}

	x = med->x;
	y = med->y;
	
	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);

	moved = mapdir_move(&med->cursor_dir, &x, &y);
	if (moved != 0) {
		mapedit_move(med, x, y);
		mapdir_postmove(&med->cursor_dir, &x, &y, moved);
		mapedit_sticky(med);
		med->map->redraw++;
	}

	pthread_mutex_unlock(&med->map->lock);
	pthread_mutex_unlock(&keyslock);

	return (ival);
}

void
mapedit_sticky(struct mapedit *med)
{
	static SDL_Event nev;
	int i, nkeys;

	for (i = 0; i < sizeof(stickykeys) / sizeof(int); i++) {
		if ((SDL_GetKeyState(&nkeys))[stickykeys[i]]) {
			nev.type = SDL_KEYDOWN;
			nev.key.keysym.sym = stickykeys[i];
			nev.key.keysym.mod = 0;
			SDL_PushEvent(&nev);
		}
	}
}

static Uint32
mapedit_lists_tick(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	Uint32 moved;

	if (curmapedit == NULL) {
		return (0);
	}

	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);

	moved = gendir_move(&med->listw_dir);
	if (moved != 0) {
		if (moved & DIR_UP && --med->curoffs < 0) {
			med->curoffs = med->curobj->nrefs - 1;
		}
		if (moved & DIR_DOWN &&
		    ++med->curoffs > med->curobj->nrefs - 1) {
			med->curoffs = 0;
		}
		mapedit_tilelist(med);
		gendir_postmove(&med->listw_dir, moved);
	}
	
	moved = gendir_move(&med->olistw_dir);
	if (moved != 0) {
		if (moved & DIR_LEFT) {
			med->curobj = TAILQ_PREV(med->curobj,
			    eobjs_head, eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_LAST(&med->eobjsh,
				    eobjs_head);
			}
		}
		if (moved & DIR_RIGHT) {
			med->curobj = TAILQ_NEXT(med->curobj, eobjs);
			if (med->curobj == NULL) {
				med->curobj = TAILQ_FIRST(&med->eobjsh);
			}
		}
		med->tilelist_offs = 0;
		med->curoffs = 0;
		mapedit_objlist(med);
		med->map->redraw++;
		mapedit_tilelist(med);
		gendir_postmove(&med->olistw_dir, moved);
	}

	pthread_mutex_unlock(&med->map->lock);
	pthread_mutex_unlock(&keyslock);

	return (ival);
}

void
mapedit_predraw(struct map *m, Uint32 flags, Uint32 vx, Uint32 vy)
{
	static SDL_Rect rd;

	rd.x = vx << m->shtilex;
	rd.y = vy << m->shtiley;
	rd.w = m->tilew;
	rd.h = m->tileh;

	SDL_FillRect(m->view->v, &rd, SDL_MapRGB(m->view->v->format,
	    rd.y >> 3, 0, rd.x >> 3));
}

void
mapedit_postdraw(struct map *m, Uint32 flags, Uint32 vx, Uint32 vy)
{
	SDL_Rect *rd = &m->view->maprects[vy][vx];

	if (curmapedit->flags & MAPEDIT_DRAWGRID) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_GRID), NULL,
		    m->view->v, rd);
	}

	if ((curmapedit->flags & MAPEDIT_DRAWPROPS) == 0) {
		return;
	}

	if (flags & NODE_ORIGIN) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_ORIGIN), NULL,
		    m->view->v, rd);
	}
	if (flags & NODE_BLOCK) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_BLOCKED), NULL,
		    m->view->v, rd);
	} else {
#if 0
		if (flags & NODE_WALK)
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_WALK), NULL,
			    m->view->v, rd);
#endif
		if (flags & NODE_CLIMB)
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_CLIMB), NULL,
			    m->view->v, rd);
		if (flags & NODE_SLIP)
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_SLIP), NULL,
			    m->view->v, rd);
	}

	if (flags & NODE_BIO) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_BIO), NULL,
		    m->view->v, rd);
	} else if (flags & NODE_REGEN) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_REGEN), NULL,
		    m->view->v, rd);
	}
	if (flags & NODE_SLOW) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_SLOW), NULL,
		    m->view->v, rd);
	} else if (flags & NODE_HASTE) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_HASTE), NULL,
		    m->view->v, rd);
	}
}
