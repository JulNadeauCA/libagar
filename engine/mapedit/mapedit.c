/*	$Csoft: mapedit.c,v 1.46 2002/02/19 01:50:51 vedge Exp $	*/

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

#include "mapedit.h"
#include "mapedit_offs.h"
#include "command.h"
#include "key.h"
#include "mouse.h"
#include "joy.h"

static struct obvec mapedit_vec = {
	mapedit_destroy,
	mapedit_event,
	mapedit_load,
	mapedit_save,
	mapedit_link,
	mapedit_unlink
};

enum {
	DEFAULT_CURSOR_SPEED = 50,	/* Cursor speed in ms */
	DEFAULT_LISTW_SPEED = 40	/* List scrolling speed in ms */
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

static int	mapedit_shadow(struct mapedit *);
static Uint32	mapedit_cursor_tick(Uint32, void *);
static Uint32	mapedit_listw_tick(Uint32, void *);
static void	mapedit_bg(SDL_Surface *, SDL_Rect *);

struct mapedit *
mapedit_create(char *name)
{
	struct mapedit *med;
	struct fobj *fob;

	med = (struct mapedit *)emalloc(sizeof(struct mapedit));
	object_init(&med->obj, strdup(name), 0, &mapedit_vec);
	med->obj.desc = strdup("map editor");
	med->flags = MAPEDIT_DRAWPROPS;
	med->map = NULL;
	med->x = -1;
	med->y = -1;
	med->cursor_speed = DEFAULT_CURSOR_SPEED;
	med->listw_speed = DEFAULT_LISTW_SPEED;
	
	med->curobj = NULL;
	med->curoffs = 0;
	med->curflags = 0;

	mapdir_init(&med->cursor_dir, (struct object *)med, NULL, -1, -1);
	gendir_init(&med->listw_dir);
	gendir_init(&med->olistw_dir);

	fob = fobj_load(savepath(name, "fob"));
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
		if (ob->nsprites < 1 || ob->nanims < 1) {
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
			int y;

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
			int z;

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
	med->tilelist.y = 0;
	med->tilelist.w = m->tilew;
	med->tilelist.h = m->view->height - m->tileh;
	
	med->tilestack.x = 0;
	med->tilestack.y = m->tileh;
	med->tilestack.w = m->tilew;
	med->tilestack.h = m->view->height - m->tileh;

	med->objlist.x = m->tilew;
	med->objlist.y = 0;
	med->objlist.w = m->view->width - m->tilew*2;
	med->objlist.h = m->tileh;

	view_setmode(m->view, m, VIEW_MAPEDIT, NULL);
	view_center(m->view, m->defx, m->defy);
	mapedit_setcaption(med, new ? "new" : path);

	/* Create the structures defining what is editable. */
	mapedit_shadow(med);

	pthread_mutex_lock(&m->lock);
	node = &m->map[m->defx][m->defy];
	node_addref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
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
	med->timer = SDL_AddTimer(med->listw_speed, mapedit_listw_tick, med);
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
	nref = node_findref(node, med, MAPEDIT_SELECT);
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
mapedit_bg(SDL_Surface *v, SDL_Rect *rd)
{
	static Uint32 col[2];
	Uint8 *dst = v->pixels;
	int x, y;

	col[0] = SDL_MapRGB(v->format, 0x66, 0x66, 0x66);
	col[1] = SDL_MapRGB(v->format, 0x99, 0x99, 0x99);

	for (y = rd->y; y < rd->h; y++) {
		for (x = rd->x; x < rd->w; x++) {
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

void
mapedit_tilelist(struct mapedit *med)
{
	struct map *m = med->map;
	static SDL_Rect rs, rd;
	int i, sn;

	rs.w = m->tilew;
	rs.h = m->tileh;
	rs.x = 0;
	rs.y = 0;

	mapedit_bg(m->view->v, &med->tilelist);

	rd = med->tilelist;	/* Structure copy */
	rd.h = m->tileh;
	
	/*
	 * Draw the sprite/anim list in a circular fashion. We must
	 * predict which sprite to draw first according to the window
	 * geometry.
	 */
	for (i = 0, sn = med->curoffs - ((med->tilelist.h / m->tileh) / 2);
	     i < (med->tilelist.h / rd.h) - 1;
	     i++, rd.y += m->tileh) {
		struct editref *ref;
		struct anim *anim;

		/*
		 * Obtain the mapedit reference at this offset. If the
		 * index is negative, wrap.
		 */
		/* XXX array */
		pthread_mutex_lock(&med->curobj->lock);
		if (sn > -1) {
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn);
		} else {
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs,
			    sn + med->curobj->nrefs);
			if (ref == NULL) {
				/* XXX hack */
				goto nextref;
			}
		}
		pthread_mutex_unlock(&med->curobj->lock);

		/* Plot the icon. */
		switch (ref->type) {
		case EDITREF_SPRITE:
			SDL_BlitSurface(ref->p, &rs, m->view->v, &rd);
			break;
		case EDITREF_ANIM:
			anim = (struct anim *)ref->p;
			SDL_BlitSurface(anim->frames[0], &rs, m->view->v,
			    &rd);
			break;
		}

		if (med->curoffs == sn) {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_CIRQSEL],
			    &rs, m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_GRID],
			    &rs, m->view->v, &rd);
		}
nextref:
		if (++sn >= med->curobj->nrefs) {
			sn = 0;
		}
	}
	SDL_UpdateRect(m->view->v,
	    med->tilelist.x, med->tilelist.y,
	    med->tilelist.w, med->tilelist.h);
}

/*
 * Draw the tile stack for the map entry under the cursor.
 * Must be called on a locked map.
 */
void
mapedit_tilestack(struct mapedit *med)
{
	static SDL_Rect rs, rd;
	struct map *m = med->map;
	struct noderef *nref;
	int i;

	rs.w = m->tilew;
	rs.h = m->tileh;
	rs.x = 0;
	rs.y = 0;

	rd = med->tilestack;	/* Structure copy */
	mapedit_bg(m->view->v, &rd);
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
			    &rs, m->view->v, &rd);
		} else if (nref->flags & MAPREF_SPRITE) {
			SDL_BlitSurface(nref->pobj->sprites[nref->offs],
			    &rs, m->view->v, &rd);
		}

		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_GRID],
		    &rs, m->view->v, &rd);
		rd.y += m->tileh;
	}
}

/*
 * Draw a list of objects in the object ring.
 * This must be called when the world is locked.
 */
void
mapedit_objlist(struct mapedit *med)
{
	static SDL_Rect rs, rd;
	struct map *m = med->map;
	struct editobj *eob;

	rs.x = 0;
	rs.y = 0;
	rs.w = m->tilew;
	rs.h = m->tileh;

	rd = med->objlist;	/* Structure copy */
	mapedit_bg(m->view->v, &rd);
	rd.x = m->tilew;

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		SDL_BlitSurface(eob->pobj->sprites[0],
		    &rs, m->view->v, &rd);
		if (med->curobj == eob) {
			SDL_BlitSurface(
			    med->obj.sprites[MAPEDIT_CIRQSEL],
			    &rs, m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    med->obj.sprites[MAPEDIT_GRID],
			    &rs, m->view->v, &rd);
		}
		rd.x += m->tilew;
	}
	SDL_UpdateRect(m->view->v,
	    med->tilelist.x, med->tilelist.y,
	    med->tilelist.w, med->tilelist.h);
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
		if (ev->button.button != SDL_BUTTON_LEFT) {
			mouse_button(med, ev);
		}
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
mapedit_move(struct mapedit *med, int x, int y)
{
	struct node *node;
	
	node = &med->map->map[med->x][med->y];
	node->flags &= ~(NODE_ANIM);
	node_delref(node, node_findref(node, med, MAPEDIT_SELECT));
	
	node = &med->map->map[x][y];
	node_addref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
	node->flags |= NODE_ANIM;

	med->x = x;
	med->y = y;
}

static Uint32
mapedit_cursor_tick(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	int x, y, moved;

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
	int i, nkeys;
	static SDL_Event nev;

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
mapedit_listw_tick(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	int moved;

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
		med->curoffs = 1;
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
mapedit_predraw(struct map *m, int flags, int vx, int vy)
{
	SDL_Rect rd;

	rd.x = vx << m->shtilex;
	rd.y = vy << m->shtiley;
	rd.w = 32;
	rd.h = 32;
	SDL_FillRect(m->view->v, &rd, 0);
}

void
mapedit_postdraw(struct map *m, int flags, int vx, int vy)
{
	vx <<= m->shtilex;
	vy <<= m->shtiley;

	if (curmapedit->flags & MAPEDIT_DRAWGRID) {
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_GRID],
		    vx, vy);
	}
	if ((curmapedit->flags & MAPEDIT_DRAWPROPS) == 0)
		return;

	if (flags == 0) {
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_BLOCKED],
		    vx, vy);
		return;
	}
	if (flags & NODE_ORIGIN)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_ORIGIN],
		    vx, vy);
#if 0
	if (flags & NODE_WALK)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_WALK],
		    vx, vy);
#endif
	if (flags & NODE_CLIMB)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_CLIMB],
		    vx, vy);
	if (flags & NODE_SLIP)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_SLIP],
		    vx, vy);
	if (flags & NODE_BIO)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_BIO],
		    vx, vy);
	if (flags & NODE_REGEN)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_REGEN],
		    vx, vy);
	if (flags & NODE_SLOW)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_SLOW],
		    vx, vy);
	if (flags & NODE_HASTE)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_HASTE],
		    vx, vy);
#if 1
	if (flags & NODE_ANIM)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_ANIM],
		    vx, vy);
#endif
}
