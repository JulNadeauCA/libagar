/*	$Csoft: mapedit.c,v 1.85 2002/05/08 09:42:28 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/text.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "command.h"
#include "mouse.h"
#include "joy.h"

static const struct version mapedit_ver = {
	"agar map editor",
	1, 0
};

static const struct obvec mapedit_ops = {
	NULL,
	mapedit_load,
	mapedit_save,
	mapedit_link,
	mapedit_unlink
};

enum {
	DEFAULT_CURSOR_SPEED = 20,	/* Cursor speed timer */
	DEFAULT_LISTW_SPEED = 16	/* List scrolling timer */
};

struct mapedit *curmapedit;		/* Map editor currently controlled.
					   XXX unsafe */

static const int stickykeys[] = {	/* Keys applied after each move. */
	SDLK_a,	/* Add */
	SDLK_d,	/* Del */
	SDLK_b,	/* Block */
	SDLK_w,	/* Walk */
	SDLK_c,	/* Climb */
	SDLK_p	/* Slippery */
};

enum {
	BG_HORIZONTAL,
	BG_VERTICAL
};

static struct window *coords_win = NULL;
static struct label *coords_label;

static void	 mapedit_shadow(struct mapedit *);
static void	 mapedit_tilelist(struct mapedit *);
static void	 mapedit_tilestack(struct mapedit *);
static void	 mapedit_objlist(struct mapedit *);
static void	*mapedit_update(void *);
static void	 mapedit_bg(SDL_Surface *, SDL_Rect *, Uint32);
static void	 mapedit_state(struct mapedit *, SDL_Rect *);
static void	 mapedit_key(struct mapedit *, SDL_Event *);
static void	 mapedit_show_coords(struct mapedit *);

void
mapedit_init(struct mapedit *med, char *name)
{
	object_init(&med->obj, name, "mapedit", OBJ_ART, &mapedit_ops);
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
	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;
	med->curobj = NULL;
	med->curoffs = 0;
	med->curflags = 0;
	pthread_mutex_init(&med->lock, NULL);

	mapdir_init(&med->cursor_dir, OBJECT(med), NULL, -1, -1);
	gendir_init(&med->listw_dir);
	gendir_init(&med->olistw_dir);
}

/*
 * Construct a list of shadow objects and references.
 * Map editor must be locked.
 */
static void
mapedit_shadow(struct mapedit *med)
{
	struct object *ob;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		struct editobj *eob;
		
		if ((ob->flags & OBJ_ART) == 0 ||
		   (ob->art->nsprites < 1 && ob->art->nanims < 1)) {
			dprintf("skipping %s (no art)\n", ob->name);
			continue;
		}

		/* Create a shadow structure for this object. */
		eob = emalloc(sizeof(struct editobj));
		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefsh);
		eob->nrefs = 0;
		eob->nsprites = ob->art->nsprites;
		eob->nanims = ob->art->nanims;
		pthread_mutex_init(&eob->lock, NULL);

		dprintf("%s: %d sprites, %d anims\n", ob->name,
		    eob->nsprites, eob->nanims);

		/* XXX save editor sessions? */
		if (eob->nsprites > 0) {
			med->curobj = eob;
			med->curoffs = 1;
		}
	
		/* Create shadow references for sprites. */
		pthread_mutex_lock(&eob->lock);
		if (eob->nsprites > 0) {
			int y;

			for (y = 0; y < eob->nsprites; y++) {
				struct editref *eref;
		
				eref = emalloc(sizeof(struct editref));
				eref->animi = -1;
				eref->spritei = y;
				eref->p = SPRITE(ob, y);
				eref->type = EDITREF_SPRITE;
				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}
	
		/* Create shadow references for animations. */
		if (eob->nanims > 0) {
			int z;

			for (z = 0; z < eob->nanims; z++) {
				struct editref *eref;

				eref = emalloc(sizeof(struct editref));
				eref->animi = z;
				eref->spritei = -1;
				eref->p = ANIM(ob, z);
				eref->type = EDITREF_ANIM;
				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}
		pthread_mutex_unlock(&eob->lock);

		/* Insert into the list of shadow objects. */
		TAILQ_INSERT_HEAD(&med->eobjsh, eob, eobjs);
		med->neobjs++;
	}
	if (med->curobj == NULL) {
		fatal("%s: nothing to edit!\n", med->obj.name);
	}

	pthread_mutex_unlock(&world->lock);
}

int
mapedit_link(void *p)
{
	static char caption[FILENAME_MAX];
	char path[FILENAME_MAX];
	struct mapedit *med = p;
	struct map *m;
	struct node *node;
	int fd, new = 0;
	pthread_t update_th;

	pthread_mutex_lock(&med->lock);
	m = med->map;

	/* XXX the renderer algorithm is insane. */
	if (med->margs.mapw < MAP_MINWIDTH ||
	    med->margs.maph < MAP_MINHEIGHT) {
		fatal("minimum size is %dx%d\n", MAP_MINWIDTH, MAP_MINHEIGHT);
	}
	
	/* Users must copy maps to udatadir in order to edit them. */
	sprintf(path, "%s/%s.m", world->udatadir, med->margs.name);

	if ((fd = open(path, O_RDONLY, 0)) > 0) {
		close(fd);

		/* Load the existing map from file. */
		m = emalloc(sizeof(struct map));
		map_init(m, med->margs.name, NULL, MAP_2D);

		pthread_mutex_lock(&world->lock);
		object_loadfrom(m, path);	/* Will lock map */
		pthread_mutex_unlock(&world->lock);
	} else {
		struct node *origin;

		/* Create a new map of the specified geometry. */
		m = emalloc(sizeof(struct map));
		map_init(m, med->margs.name, NULL, MAP_2D);

		pthread_mutex_lock(&m->lock);
		map_allocnodes(m, med->margs.mapw, med->margs.maph,
		    med->margs.tilew, med->margs.tileh);
		m->defx = med->margs.mapw / 2;
		m->defy = med->margs.maph - 2;
		origin = &m->map[m->defy][m->defx];
		origin->flags |= NODE_ORIGIN;
		pthread_mutex_unlock(&m->lock);

		new++;
	}


	/* Initialize the map editor. */
	med->map = m;
	pthread_mutex_lock(&m->lock);
	med->x = m->defx;
	med->y = m->defy;
	mapdir_init(&med->cursor_dir, OBJECT(med), m,
	    DIR_SCROLLVIEW|DIR_SOFTSCROLL|DIR_STATIC|DIR_PASSTHROUGH, 8);
	med->tilelist.x = m->view->width - m->tilew;
	med->tilelist.y = m->tileh;
	med->tilelist.w = m->tilew;
	med->tilelist.h = m->view->height - m->tilew;
	med->tilelist_offs = 0;
	med->tilestack.x = 0;
	med->tilestack.y = m->tileh;
	med->tilestack.w = m->tilew;
	med->tilestack.h = m->view->height - m->tilew;
	med->objlist.x = m->tilew;
	med->objlist.y = 0;
	med->objlist.w = m->view->width - m->tilew;
	med->objlist.h = m->tileh;
	med->objlist_offs = 0;

	/* Set the video mode. */
	view_setmode(m->view, m, VIEW_MAPEDIT, NULL);
	view_center(m->view, m->defx, m->defy);

	/* Set the window caption. */
	pthread_mutex_lock(&world->lock);
	if (object_strfind(med->margs.name) == NULL) {
		sprintf(caption, "%s [unknown] (%s)", OBJECT(m)->name, path);
		object_link(m);
	} else {
		sprintf(caption, "%s [%d] (%s)", OBJECT(m)->name,
		    OBJECT(m)->id, path);
	}
	pthread_mutex_unlock(&world->lock);
	SDL_WM_SetCaption(caption, "agar");

	/* Create the structures defining what is editable. */
	mapedit_shadow(med);

	/* Position a map edition cursor on the map. */
	node = &m->map[m->defy][m->defx];
	node_addref(node, med, MAPEDIT_SELECT,
	    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
	node->flags |= (NODE_ANIM|NODE_ORIGIN);

	pthread_mutex_unlock(&m->lock);
	
	/* Map is now in a consistent state, start displaying. */
	map_focus(m);

	curmapedit = med;
	dprintf("editing %d object(s)\n", med->neobjs);

	med->redraw++;

	pthread_mutex_unlock(&med->lock);

	/* curmapedit must be non-NULL when mapedit_update() starts. */
	pthread_create(&update_th, NULL, mapedit_update, med);

	return (0);
}

int
mapedit_unlink(void *p)
{
	struct mapedit *med = p;
	struct map *m;
	struct node *node;
	struct noderef *nref;
	struct editobj *eob, *nexteob;

	pthread_mutex_lock(&med->lock);
	m = med->map;

	curmapedit = NULL;	/* Will stop update thread. */

	/* Remove the map editor from the map. */
	pthread_mutex_lock(&m->lock);
	node = &m->map[med->y][med->x];
	nref = node_findref(node, med, MAPEDIT_SELECT, MAPREF_ANIM);
	if (nref != NULL) {
		node_delref(node, nref);
		node->flags &= ~(NODE_ANIM);
	}
	pthread_mutex_unlock(&m->lock);
	m->redraw++;

	/* Deallocate the shadow structures. */
	for (eob = TAILQ_FIRST(&med->eobjsh);
	     eob != TAILQ_END(&med->eobjsh);
	     eob = nexteob) {
		struct editref *eref, *nexteref;

		nexteob = TAILQ_NEXT(eob, eobjs);

		pthread_mutex_lock(&eob->lock);
		for (eref = SIMPLEQ_FIRST(&eob->erefsh);
		     eref != SIMPLEQ_END(&eob->erefsh);
		     eref = nexteref) {
			nexteref = SIMPLEQ_NEXT(eref, erefs);
			free(eref);
		}
		pthread_mutex_unlock(&eob->lock);

		pthread_mutex_destroy(&eob->lock);
		free(eob);
	}
	pthread_mutex_unlock(&med->lock);
	return (0);
}

/* XXX use widgets */
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
 * Map editor and map must be locked.
 */
static void
mapedit_state(struct mapedit *med, SDL_Rect *rd)
{
	if (med->flags & MAPEDIT_INSERT) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_INSERT_TXT),
		    NULL, med->map->view->v, rd);
	} else {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_REPLACE_TXT),
		    NULL, med->map->view->v, rd);
	}
	if (med->flags & MAPEDIT_DRAWPROPS)
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_PROPS_TXT),
		    NULL, med->map->view->v, rd);
	if (med->flags & MAPEDIT_DRAWGRID)
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_GRID_TXT),
		    NULL, med->map->view->v, rd);
	if (med->curflags & NODE_BLOCK) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_BLOCKED),
		    NULL, med->map->view->v, rd);
	} else {
		if (med->curflags & NODE_WALK) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_WALK),
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_CLIMB) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_CLIMB),
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_SLIP) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_SLIP),
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_BIO) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_BIO),
			    NULL, med->map->view->v, rd);
		} else if (med->curflags & NODE_REGEN) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_REGEN),
			    NULL, med->map->view->v, rd);
		}
		if (med->curflags & NODE_SLOW) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_SLOW),
			    NULL, med->map->view->v, rd);
		} else if (med->curflags & NODE_HASTE) {
			SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_HASTE),
			    NULL, med->map->view->v, rd);
		}
	}
}

/*
 * Draw the tile selection.
 * Map editor and map must be locked.
 */
void
mapedit_tilelist(struct mapedit *med)
{
	struct map *m;
	static SDL_Rect rd;
	int i, sn;
	
	m = med->map;

	rd = med->tilelist;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_VERTICAL);
	rd.h = m->tilew;
	rd.y = m->tileh;
	
	pthread_mutex_lock(&med->curobj->lock);

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

		if (sn > -1) {
			/* Absolute */
			SIMPLEQ_INDEX(ref, &med->curobj->erefsh, erefs, sn);
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
			    SPRITE(curmapedit, MAPEDIT_CIRQSEL), NULL,
			    m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    SPRITE(curmapedit, MAPEDIT_GRID), NULL,
			    m->view->v, &rd);
		}
nextref:
		if (++sn >= med->curobj->nrefs) {
			sn = 0;
		}
	}
	
	pthread_mutex_unlock(&med->curobj->lock);

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
 * Map editor and map must be locked.
 */
void
mapedit_tilestack(struct mapedit *med)
{
	static SDL_Rect rd;
	struct noderef *nref;
	struct map *m;
	int i;
	
	m = med->map;

	rd = med->tilestack;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_VERTICAL);
	rd.h = m->tileh;

	i = 0;
	TAILQ_FOREACH(nref, &(&m->map[med->y][med->x])->nrefsh, nrefs) {
		if (++i > (med->tilestack.h / rd.h) - 1) {
			return;
		}
		if (nref->flags & MAPREF_ANIM) {
			static struct anim *anim;

			anim = ANIM(nref->pobj, nref->offs);
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
			SDL_BlitSurface(SPRITE(nref->pobj, nref->offs),
			    NULL, m->view->v, &rd);
		}

		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_GRID),
		    NULL, m->view->v, &rd);
		rd.y += m->tileh;
	}
	SDL_UpdateRect(m->view->v,
	    med->tilestack.x, med->tilestack.y,
	    med->tilestack.w, med->tilestack.h);
}

/*
 * Draw the current object list.
 * Must be called on a locked map editor and map.
 */
void
mapedit_objlist(struct mapedit *med)
{
	static SDL_Rect rd;
	struct editobj *eob;
	struct map *m;
	int i, sn;

	m = med->map;

	rd = med->objlist;	/* Structure copy */
	mapedit_bg(m->view->v, &rd, BG_HORIZONTAL);
	rd.w = m->tilew;
	rd.x = m->tilew;

	for (i = 0, sn = med->objlist_offs;
	     i < (med->objlist.w / m->tilew) - 1;
	     i++, rd.x += m->tilew) {

		if (sn > -1) {
			/* Absolute */
			TAILQ_INDEX(eob, &med->eobjsh, eobjs, sn);
		} else {
			/* Wrap */
			TAILQ_INDEX(eob, &med->eobjsh, eobjs,
			    sn + med->neobjs);
			if (eob == NULL) {
				goto nextref;
			}
		}
	
		SDL_BlitSurface(SPRITE(eob->pobj, 0), NULL, m->view->v, &rd);

		if (med->curobj == eob) {
			SDL_BlitSurface(
			    SPRITE(med, MAPEDIT_CIRQSEL),
			    NULL, m->view->v, &rd);
		} else {
			SDL_BlitSurface(
			    SPRITE(med, MAPEDIT_GRID),
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

/*
 * Process a map editor keystroke.
 * Map editor and map must be locked.
 */
static void
mapedit_key(struct mapedit *med, SDL_Event *ev)
{
	const int set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		if (med->y > 1) {
			mapdir_set(&med->cursor_dir, DIR_UP, set);
		}
		break;
	case SDLK_DOWN:
		if (med->y < med->map->maph - 2) {
			mapdir_set(&med->cursor_dir, DIR_DOWN, set);
		}
		break;
	case SDLK_LEFT:
		if (med->x > 1) {
			mapdir_set(&med->cursor_dir, DIR_LEFT, set);
		}
		break;
	case SDLK_RIGHT:
		if (med->x < med->map->mapw - 2) {
			mapdir_set(&med->cursor_dir, DIR_RIGHT, set);
		}
		break;
	case SDLK_PAGEUP:
		gendir_set(&med->listw_dir, DIR_UP, set);
		break;
	case SDLK_PAGEDOWN:
		gendir_set(&med->listw_dir, DIR_DOWN, set);
		break;
	case SDLK_DELETE:
		gendir_set(&med->olistw_dir, DIR_LEFT, set);
		break;
	case SDLK_END:
		gendir_set(&med->olistw_dir, DIR_RIGHT, set);
		break;
	default:
		break;
	}

	if (ev->type == SDL_KEYDOWN) {
		struct node *node;
		int mapx, mapy;

		mapx = med->x;
		mapy = med->y;
		node = &med->map->map[mapy][mapx];

		switch (ev->key.keysym.sym) {
		case SDLK_INSERT:
			mapedit_editflags(med, MAPEDIT_INSERT);
			break;
		case SDLK_a:
			mapedit_push(med, node, med->curoffs, med->curflags);
			break;
		case SDLK_d:
			mapedit_pop(med, node, ev->key.keysym.mod & KMOD_SHIFT);
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
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_show_coords(med);
			} else {
				mapedit_nodeflags(med, node, NODE_CLIMB);
			}
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
			}
			break;
		case SDLK_n:
			if (ev->key.keysym.mod & KMOD_CTRL) {
				mapedit_clearmap(med);
			}
			break;
		case SDLK_o:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_setorigin(med, &mapx, &mapy);
				mapedit_move(med, mapx, mapy);
			}
			break;
		case SDLK_l:
			/* Concurrent load. */
			mapedit_loadmap(med);
			break;
		case SDLK_s:
			if (ev->key.keysym.mod & KMOD_SHIFT) {
				mapedit_nodeflags(med, node, NODE_SLOW);
			} else {
				/* Concurrent save. */
				mapedit_savemap(med);
			}
			break;
		case SDLK_g:
			mapedit_editflags(med, MAPEDIT_DRAWGRID);
			break;
		case SDLK_x:
			mapedit_examine(med->map, mapx, mapy);
			break;
		default:
			break;
		}
	}
}

void
mapedit_event(void *ob, SDL_Event *ev)
{
	struct mapedit *med = ob;

	pthread_mutex_lock(&med->lock);
	pthread_mutex_lock(&med->map->lock);

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		if (coords_win != NULL) {
			if (ev->motion.y <= med->map->tileh ||
			    med->mmapx >= med->map->view->mapw-1) {
				sprintf(coords_label->caption,
				    "%s:%d (0x%x)",
				    OBJECT(med->curobj->pobj)->name,
				    med->curoffs, med->curflags);
			} else {
				sprintf(coords_label->caption,
				    "%d,%d [%s:%d,%d]",
				    ev->motion.x, ev->motion.y,
				    OBJECT(med->map)->name,
				    med->map->view->mapx + med->mmapx - 1,
				    med->map->view->mapy + med->mmapy - 1);
			}
			coords_win->redraw++;
		}
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
	pthread_mutex_unlock(&med->lock);
}

int
mapedit_load(void *p, int fd)
{
	struct mapedit *med = p;

	if (version_read(fd, &mapedit_ver) != 0) {
		return (-1);
	}

	pthread_mutex_lock(&med->lock);
	med->flags = fobj_read_uint32(fd);
	med->cursor_speed = fobj_read_uint32(fd);
	med->listw_speed = fobj_read_uint32(fd);
	med->redraw++;
	pthread_mutex_unlock(&med->lock);

	return (0);
}

int
mapedit_save(void *p, int fd)
{
	struct mapedit *med = (struct mapedit *)p;

	version_write(fd, &mapedit_ver);

	pthread_mutex_lock(&med->lock);
	fobj_write_uint32(fd, med->flags);
	fobj_write_uint32(fd, med->cursor_speed);
	fobj_write_uint32(fd, med->listw_speed);
	pthread_mutex_unlock(&med->lock);

	return (0);
}

/*
 * Move the map edition cursor.
 * Map editor and map must be locked.
 */
void
mapedit_move(struct mapedit *med, Uint32 x, Uint32 y)
{
	struct node *node;

	node = &med->map->map[med->y][med->x];
	node_delref(node, node_findref(node, med, MAPEDIT_SELECT, MAPREF_ANIM));
	node->flags &= ~(NODE_ANIM);
	
	node = &med->map->map[y][x];
	node_addref(node, med, MAPEDIT_SELECT,
	    MAPREF_ANIM|MAPREF_ANIM_INDEPENDENT);
	node->flags |= NODE_ANIM;

	med->x = x;
	med->y = y;
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
			nev.key.keysym.mod = SDL_GetModState();
			SDL_PushEvent(&nev);
		}
	}
}

static void *
mapedit_update(void *p)
{
	struct mapedit *med = p;
	Uint32 x, y, moved;

	while (curmapedit != NULL) {	/* XXX unsafe */
		pthread_mutex_lock(&med->lock);
		pthread_mutex_lock(&med->map->lock);

		/* Update the tile list. */
		moved = gendir_move(&med->listw_dir);
		if (moved != 0) {
			if (moved & DIR_UP && --med->curoffs < 0) {
				med->curoffs = med->curobj->nrefs - 1;
			}
			if (moved & DIR_DOWN &&
			    ++med->curoffs > med->curobj->nrefs - 1) {
				med->curoffs = 0;
			}
			gendir_postmove(&med->listw_dir, moved);
			med->redraw++;
		}

		/* Update the object list. */
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
			gendir_postmove(&med->olistw_dir, moved);
			med->redraw++;
		}

		/* Update the cursor position. */
		x = med->x;
		y = med->y;
		moved = mapdir_move(&med->cursor_dir, &x, &y);
		if (moved != 0) {
			mapedit_move(med, x, y);
			mapdir_postmove(&med->cursor_dir, &x, &y, moved);
			mapedit_sticky(med);
			med->redraw++;
		}

		/* Redraw the map and lists. */
		if (med->redraw != 0) {
			med->redraw = 0;
			mapedit_tilestack(med);
			mapedit_tilelist(med);
			mapedit_objlist(med);
			med->map->redraw++;
		}

		pthread_mutex_unlock(&med->map->lock);
		pthread_mutex_unlock(&med->lock);
		SDL_Delay(100);
	}

	return (NULL);
}

/* XXX macro; use rects */
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

/*
 * Draw node-specific attributes.
 * Map must be locked.
 */
void
mapedit_postdraw(struct map *m, Uint32 flags, Uint32 vx, Uint32 vy)
{
	SDL_Rect *rd = &m->view->maprects[vy][vx];

	pthread_mutex_lock(&curmapedit->lock);

	if (curmapedit->flags & MAPEDIT_DRAWGRID) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_GRID), NULL,
		    m->view->v, rd);
	}
	if ((curmapedit->flags & MAPEDIT_DRAWPROPS) == 0) {
		goto done;
	}
	
	/* XXX prefs */

	/* Origin node. */
	if (flags & NODE_ORIGIN) {
		SDL_BlitSurface(SPRITE(curmapedit, MAPEDIT_ORIGIN), NULL,
		    m->view->v, rd);
	}

	/* Physical attributes. */
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

	/* State attributes. */
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
done:
	pthread_mutex_unlock(&curmapedit->lock);
}

/*
 * Toggle the coordinates window.
 * Map editor and map must be locked.
 */
static void
mapedit_show_coords(struct mapedit *med)
{
	struct window *nw;

	if (coords_win == NULL) {
		/* Coordinates window/label. */
		nw = window_new(med->map->view, "Coordinates",
		    WINDOW_SOLID, 0, 64, 64, 224, 32);
		coords_label = label_new(nw, "...", 0, 7, 7);
		coords_win = nw;
	} else {
		nw = coords_win;
		coords_win = NULL;

		/* Destroy the coordinates window. */
		pthread_mutex_lock(&world->lock);
		object_unlink(nw);
		pthread_mutex_unlock(&world->lock);
	}
}

