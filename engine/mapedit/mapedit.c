/*	$Csoft: mapedit.c,v 1.42 2002/02/17 10:39:36 vedge Exp $	*/

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

#include "pathnames.h"
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
static void	mapedit_bg(SDL_Surface *, int, int, int, int);

struct mapedit *
mapedit_create(char *name)
{
	struct mapedit *med;
	struct fobj *fob;
	char fobpath[FILENAME_MAX];

	med = (struct mapedit *)emalloc(sizeof(struct mapedit));
	object_init(&med->obj, strdup(name), 0, &mapedit_vec);
	med->obj.desc = strdup("map editor");
	med->flags = MAPEDIT_TILELIST|MAPEDIT_TILESTACK|MAPEDIT_OBJLIST|
	    MAPEDIT_DRAWPROPS;
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

	sprintf(fobpath, MAPEDIT_FOB_PATH, PREFIX);
	fob = fobj_load(fobpath);
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
		
		dprintf("shadow \"%s\"\n", ob->name);

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

		dprintf("%s has %d sprites, %d anims\n", ob->name,
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

int
mapedit_link(void *p)
{
	char path[FILENAME_MAX];
	char caption[FILENAME_MAX];
	struct mapedit *med = (struct mapedit *)p;
	struct map *m = med->map;
	struct node *node;
	int fd, new = 0;
	
	/* Users must copy maps to udatadir in order to edit them. */
	sprintf(path, "%s/%s.o", world->udatadir, med->margs.name);

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
		m->defy = med->margs.maph - 1;
		origin = &m->map[m->defx][m->defy];
		origin->flags |= NODE_ORIGIN;

		new++;
	}

	med->map = m;
	med->x = m->defx;
	med->y = m->defy;
	mapdir_init(&med->cursor_dir, (struct object *)med, m,
	    DIR_SCROLLVIEW|DIR_SOFTSCROLL, 9);
	med->tilelist = window_create(m->view,
	    (m->view->width - m->tilew), m->tileh,
	    m->tilew, m->view->height + m->tileh,
	    "Tile list");
	med->tilestack = window_create(m->view,
	    0, m->tileh,
	    m->tilew, m->view->height + m->tileh,
	    "Tile stack");
	med->objlist = window_create(m->view,
	    m->tilew, 0,
	    m->view->width - m->tilew, m->tileh,
	    "Object list");

	if (object_strfind(med->margs.name) == NULL) {
		sprintf(caption, "[unknown]: %s (%s)",
		    m->obj.name, new ? "new" : path);
		object_link(m);
	} else {
		sprintf(caption, "[%s(%d)]: %s (%s)",
		    m->obj.name, m->obj.id, m->obj.name,
		    new ? "new" : path);
	}

	view_setmode(m->view, m, VIEW_MAPEDIT, caption);
	view_center(m->view, m->defx, m->defy);

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
	return (0);
}

int
mapedit_unlink(void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	struct map *m = med->map;
	struct node *node;
	struct noderef *nref;

	curmapedit = NULL;

	SDL_RemoveTimer(med->timer);

	pthread_mutex_lock(&m->lock);
	node = &m->map[med->x][med->y];
	nref = node_findref(node, med, MAPEDIT_SELECT);
	if (nref != NULL) {
		node_delref(node, nref);
		node->flags &= ~(NODE_ANIM);
	}
	pthread_mutex_unlock(&m->lock);

	window_destroy(med->tilelist);
	window_destroy(med->tilestack);
	window_destroy(med->objlist);

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
	struct map *m = med->map;
	static SDL_Rect rs, rd;
	int i, sn;

	rs.w = m->tilew;
	rs.h = m->tileh;
	rs.x = 0;
	rs.y = 0;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = m->tilew;
	rd.h = win->height;
	mapedit_bg(win->view->v, rd.x, rd.y, win->view->width,
	    win->view->height + m->tileh); /* XXX */
	rd.h = m->tileh;
	
	/*
	 * Draw the sprite/anim list in a circular fashion. We must
	 * predict which sprite to draw first according to the window
	 * geometry.
	 */
	for (i = 0, sn = med->curoffs - ((win->height / m->tileh) / 2);
	     i < (win->height / m->tileh) - 1;
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
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_CIRQSEL],
			    &rs, win->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_GRID], &rs,
			    win->view->v, &rd);
		}
		if (sn++ >= med->curobj->nrefs - 1) {
			sn = 0;
		}
	}
	SDL_UpdateRect(m->view->v,
	    (m->view->width - m->tileh), 0,
	    m->tilew, m->view->height);
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
	struct window *win = med->tilestack;
	struct noderef *nref;
	int i;

	rs.w = m->tilew;
	rs.h = m->tileh;
	rs.x = 0;
	rs.y = 0;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = m->tilew;
	rd.h = win->height;
	mapedit_bg(win->view->v, rd.x, rd.y, rd.w, rd.h);
	rd.h = m->tileh;

	i = 0;
	TAILQ_FOREACH(nref, &(&m->map[med->x][med->y])->nrefsh, nrefs) {
		if (++i > (win->height / m->tileh) - 1) {
			return;
		}
		if (nref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			anim = nref->pobj->anims[nref->offs];
			SDL_BlitSurface(anim->frames[0],
			    &rs, win->view->v, &rd);
		} else if (nref->flags & MAPREF_SPRITE) {
			SDL_BlitSurface(nref->pobj->sprites[nref->offs],
			    &rs, win->view->v, &rd);
		}

		SDL_BlitSurface(curmapedit->obj.sprites[MAPEDIT_GRID],
		    &rs, win->view->v, &rd);
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
	struct window *win = med->objlist;
	struct map *m = med->map;
	struct editobj *eob;

	rs.x = 0;
	rs.y = 0;
	rs.w = m->tilew;
	rs.h = m->tileh;

	rd.x = win->x;
	rd.y = win->y;
	rd.w = win->width;
	rd.h = m->tileh;
	mapedit_bg(win->view->v, rd.x, rd.y, rd.w, rd.h);
	rd.x = m->tilew;

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		SDL_BlitSurface(eob->pobj->sprites[0],
		    &rs, win->view->v, &rd);
		if (med->curobj == eob) {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_CIRQSEL],
			    &rs, win->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    curmapedit->obj.sprites[MAPEDIT_GRID],
			    &rs, win->view->v, &rd);
		}
		rd.x += m->tilew;
	}
	SDL_UpdateRect(m->view->v,
	    0, (m->view->width - m->tilew),
	    m->view->width, m->tileh);
}

void
mapedit_event(void *ob, SDL_Event *ev)
{
	struct mapedit *med = ob;

	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);
	
	switch (ev->type) {
	case SDL_MOUSEMOTION:
		if (ev->motion.state == SDL_PRESSED) {
			mouse_motion(med, ev);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (ev->button.button != 1) {
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
	
	x = med->x;
	y = med->y;
	
	pthread_mutex_lock(&keyslock);
	pthread_mutex_lock(&med->map->lock);

	moved = mapdir_move(&med->cursor_dir, &x, &y);
	if (moved != 0) {
		SDL_Event nev;
		int i, nkeys;

		mapedit_move(med, x, y);
		mapdir_postmove(&med->cursor_dir, &x, &y, moved);
		med->map->redraw++;

		for (i = 0; i < sizeof(stickykeys) / sizeof(int); i++) {
			if ((SDL_GetKeyState(&nkeys))[stickykeys[i]]) {
				nev.type = SDL_KEYDOWN;
				nev.key.keysym.sym = stickykeys[i];
				nev.key.keysym.mod = 0;
				SDL_PushEvent(&nev);
			}
		}
	}

	pthread_mutex_unlock(&med->map->lock);
	pthread_mutex_unlock(&keyslock);

	return (ival);
}

static Uint32
mapedit_listw_tick(Uint32 ival, void *p)
{
	struct mapedit *med = (struct mapedit *)p;
	int moved;
#if 0
	static int tick = 0;

	if(med->listw_dir.clear == 0) {		/* Keyups have precedence */
		if (++tick < 8) {
			return (ival);
		} else {
			tick = 0;
		}
	}
#endif

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
			if (!TAILQ_EMPTY(&med->eobjsh)) {
				med->curobj = TAILQ_PREV(med->curobj,
				    eobjs_head, eobjs);
				if (med->curobj == NULL) {
					med->curobj = TAILQ_LAST(&med->eobjsh,
					     eobjs_head);
				}
			}
		}
		if (moved & DIR_RIGHT) {
			if (!TAILQ_EMPTY(&med->eobjsh)) {
				med->curobj = TAILQ_NEXT(med->curobj, eobjs);
				if (med->curobj == NULL) {
					med->curobj = TAILQ_FIRST(&med->eobjsh);
				}
			}
		}
		mapedit_objlist(med);
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

	if (curmapedit->flags & MAPEDIT_DRAWGRID)
		map_plot_sprite(m, curmapedit->obj.sprites[MAPEDIT_GRID],
		    vx, vy);
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
