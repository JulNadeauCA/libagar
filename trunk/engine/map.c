/*	$Csoft: map.c,v 1.10 2002/02/03 11:27:12 vedge Exp $	*/

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

#define TILESHIFT	5	/* 32x32 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

struct	map *curmap;

static int	 map_link(void *);
static int	 map_load(void *, char *);
static int	 map_save(void *, char *);
static void	 map_destroy(struct object *);
static void	 node_destroy(struct node *);
static int	 node_init(struct node *, struct object *, int, int, int);
static void	 mapedit_drawflags(struct map *, int, int, int);
static Uint32	 map_draw(Uint32, void *);
static Uint32	 map_animate(Uint32, void *);

struct map *
map_create(char *name, char *desc, int flags, int width, int height, char *path)
{
	struct map *m;
	int x = 0, y = 0;

	m = (struct map *)malloc(sizeof(struct map));
	if (m == NULL) {
		perror("map");
		return (NULL);
	}

	/* Initialize the map structure. */
	object_create(&m->obj, name, desc,
	    DESTROY_HOOK|LOAD_FUNC|SAVE_FUNC|OBJ_DEFERGC);
	m->obj.destroy_hook = map_destroy;

	m->obj.load = map_load;
	m->obj.save = map_save;

	m->flags = flags;
	m->maph = height;
	m->mapw = width;
	m->defx = m->mapw / 2;
	m->defy = m->maph - 1;
	m->view = mainview;
	m->view->map = m;

	if (pthread_mutex_init(&m->lock, NULL) != 0) {
		perror(name);
		goto maperr;
	}

	/* Initialize an empty map. */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (node_init(&m->map[x][y], NULL, 0, 0, 0) < 0) {
				return (NULL);
			}
		}
	}
	
	if (path != NULL) {
		map_load(m, path);
	}
	
	dprintf("%s: geo %dx%d flags 0x%x\n", m->obj.name,
	    m->mapw, m->mapw, m->flags);
	dprintf("%s: tilegeo %dx%d origin %dx%d\n", m->obj.name,
	    m->view->tilew, m->view->tileh,
	    m->defx, m->defy);

	map_link(m);
	object_link(m);

	/* Synchronize map display. XXX tune */
	m->view->mapanimt = NULL;
	m->view->mapdrawt = SDL_AddTimer(15, map_draw, m);
	if (m->view->mapdrawt == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		goto maperr;
	}

	return (m);
maperr:
	object_destroy(m);
	return (NULL);
}

int
map_animset(struct map *m, int tog)
{
	if (tog > 0) {
		m->view->mapanimt = SDL_AddTimer(m->view->fps * 2,
		    map_animate, m);
		if (m->view->mapanimt == NULL) {
			fatal("SDL_AddTimer: %s\n", SDL_GetError());
			return (0);
		}
	} else {
		if (m->view->mapanimt != NULL) {
			SDL_RemoveTimer(m->view->mapanimt);
			m->view->mapanimt = NULL;
		}
	}

	return (0);
}

int
map_focus(struct map *m)
{
	curmap = m;

	dprintf("focusing on %s\n", m->obj.name);

	if (curmapedit != NULL) {
		char s[128];
		
		sprintf(s, "%s (edition)", m->obj.name);
		SDL_WM_SetCaption(s, "mapedit");
	} else {
		SDL_WM_SetCaption(m->obj.name, "mapedit");
	}
	
	map_animset(m, 1);
	return (0);
}

int
map_unfocus(struct map *m)
{
	dprintf("unfocusing %s\n", m->obj.name);
	
	if (curmapedit != NULL) {
		curmapedit->flags = 0;
	}
	pthread_mutex_lock(&m->lock);
	map_animset(m, 0);
	curmap = NULL;
	pthread_mutex_unlock(&m->lock);

	return (0);
}

static int
map_link(void *ob)
{
	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_INSERT_HEAD(&world->wmapsh, (struct map *)ob, wmaps);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}
	return (0);
}

/*
 * Initialize a map node.
 * Must be called on a locked map.
 */
static int
node_init(struct node *me, struct object *ob, int offs, int meflags, int rflags)
{
	memset(me, 0, sizeof(struct node));

	TAILQ_INIT(&me->arefsh);

	/* Used by the map editor to fill new maps. */
	if (ob != NULL) {
		node_addref(me, ob, offs, rflags);
		me->flags |= meflags;
	}
	return (0);
}

/*
 * Allocate and link a map node.
 * Must be called on a locked map.
 */
struct map_aref *
node_addref(struct node *me, struct object *ob, int offs, int rflags)
{
	struct map_aref *aref;

	aref = malloc(sizeof(struct map_aref));
	if (aref == NULL) {
		perror("map_aref");
		return (NULL);
	}

	aref->pobj = ob;
	aref->offs = offs;
	aref->flags = rflags;
	if (rflags & MAPREF_ANIM) {
		aref->frame = 0;
		aref->fwait = 0;
		me->nanims++;
	}
	aref->xoffs = 0;
	aref->yoffs = 0;

	TAILQ_INSERT_TAIL(&me->arefsh, aref, marefs);
	me->narefs++;

	return (aref);
}

/*
 * Pop a reference off the stack and return it.
 * Must be called on a locked map.
 */
struct map_aref *
node_popref(struct node *me)
{
	struct map_aref *aref;

	if (TAILQ_EMPTY(&me->arefsh)) {
		return (NULL);
	}
		
	aref = TAILQ_FIRST(&me->arefsh);
	TAILQ_REMOVE(&me->arefsh, aref, marefs);
	me->narefs--;

	if (aref->flags & MAPREF_ANIM) {
		me->nanims--;
	}

	return (aref);
}

/*
 * Push a reference onto the stack.
 * Must be called on a locked map.
 */
int
node_pushref(struct node *me, struct map_aref *aref)
{
	TAILQ_INSERT_HEAD(&me->arefsh, aref, marefs);
	me->narefs++;
	
	if (aref->flags & MAPREF_ANIM) {
		me->nanims++;
	}

	return (0);
}

/*
 * Delete a map entry reference.
 * Must be called on a locked map.
 */
int
node_delref(struct node *me, struct map_aref *aref)
{
	if (aref->flags & MAPREF_ANIM) {
		me->nanims--;
	}
	
	TAILQ_REMOVE(&me->arefsh, aref, marefs);
	me->narefs--;
	free(aref);

	return (0);
}

/*
 * Reinitialize a map to zero.
 * Must be called on a locked map.
 */
void
map_clean(struct map *m, struct object *ob, int offs, int flags, int rflags)
{
	int x = 0, y;

	/* Initialize the nodes. */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			node_destroy(&m->map[x][y]);
			if (node_init(&m->map[x][y], ob, offs, flags,
			    rflags) < 0) {
				return;
			}
		}
	}

	m->map[m->defx][m->defy].flags |= MAPENTRY_ORIGIN;
}

static void
map_destroy(struct object *ob)
{
	struct map *m = (struct map *)ob;
	int x = 0, y;

	map_animset(m, 0);

	SDL_RemoveTimer(m->view->mapdrawt);
	m->view->mapdrawt = NULL;

	/* Unlink the map from the active map list. */
	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_REMOVE(&world->wmapsh, m, map, wmaps);
		pthread_mutex_unlock(&world->lock);
	}

	if (pthread_mutex_lock(&m->lock) == 0) {
		for (y = 0; y < m->maph; y++) {
			for (x = 0; x < m->mapw; x++) {
				node_destroy(&m->map[x][y]);
			}
		}
		pthread_mutex_unlock(&m->lock);
		pthread_mutex_destroy(&m->lock);
	} else {
		perror(m->obj.name);
	}
}

/* Must be called on a locked map. */
static void
node_destroy(struct node *me)
{
	struct map_aref *aref1, *aref2;

	aref1 = TAILQ_FIRST(&me->arefsh);
	while (aref1 != NULL) {
		aref2 = TAILQ_NEXT(aref1, marefs);
		free(aref1);
		aref1 = aref2;
	}
	TAILQ_INIT(&me->arefsh);
}

/* Draw a sprite at any given location. */
static __inline__ void
map_plot_sprite(struct map *m, SDL_Surface *s, int x, int y)
{
	static SDL_Rect rs, rd;

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;

	if (m->flags & MAP_VARTILEGEO) {
		/*
		 * The sprite size should be a multiple of the tile size.
		 * XXX optimize math.
		 */
		if (rd.w > m->view->tilew) {
			x -= (rs.w / m->view->tilew) / 2;
		}
		if (rd.h > m->view->tileh) {
			y -= (rs.h / m->view->tileh) / 2;
		}
	}

	/* XXX pre-compute map/view coordinate pairs. */
	rd.x = x;
	rd.y = y;

	SDL_BlitSurface(s, &rs, m->view->v, &rd);
}

/* Draw an animation's current frame. */
static __inline__ void
map_plot_anim(struct map *m, struct anim *anim, int frame, int x, int y)
{
	static SDL_Rect rs, rd;
	static SDL_Surface *s;

	s = g_slist_nth_data(anim->frames, frame);

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;

	rd.x = x;
	rd.y = y;

	SDL_BlitSurface(s, &rs, m->view->v, &rd);
}

/*
 * Update all animations in the map view, and move references
 * with a nonzero x/y offset value.
 */
static Uint32
map_animate(Uint32 ival, void *p)
{
	struct map *m = (struct map *)p;
	int x, y;
	int vx, vy;

	if (pthread_mutex_lock(&m->lock) != 0) {
		perror(m->obj.name);
		return (0);
	}
	for (y = m->view->mapy, vy = 0;
	    (y < m->view->maph + m->view->mapy + 1);
	     y++, vy++) {

		/* XXX */
		if (curmapedit != NULL &&
		    vy == 0 && curmapedit->flags & MAPEDIT_OBJLIST) {
			continue;
		}
	
		for (x = m->view->mapx, vx = 0;
		     x < m->view->mapw + m->view->mapx + 1;
		     x++, vx++) {
			struct node *me;
			struct map_aref *aref;

			/* XXX */
			if (curmapedit != NULL) {
				if (vx == m->view->mapw &&
				    curmapedit->flags & MAPEDIT_TILELIST) {
					continue;
				}
				if (vx == 0 &&
				    curmapedit->flags & MAPEDIT_TILESTACK) {
					continue;
				}
			}

			me = &m->map[x][y];

			if (me->nanims < 1) {
				/* map_draw() shall handle this. */
				continue;
			}

			TAILQ_FOREACH(aref, &me->arefsh, marefs) {
				if (pthread_mutex_lock(&aref->pobj->lock)
				    != 0) {
					perror(aref->pobj->name);
					continue;
				}
				/*
				 * The map rendering routine does not
				 * bother drawing sprites on tiles
				 * containing animations.
				 */
				if (aref->flags & MAPREF_SPRITE) {
					map_plot_sprite(m,
					    g_slist_nth_data(
					    aref->pobj->sprites, aref->offs),
					    (vx << TILESHIFT) + aref->xoffs,
					    (vy << TILESHIFT) + aref->yoffs);
				} else if (aref->flags & MAPREF_ANIM) {
					struct anim *anim;
			
					anim = g_slist_nth_data(
					    aref->pobj->anims, aref->offs);
					if (anim == NULL) {
						fatal("NULL anim!\n");
					}
					map_plot_anim(m,
					    g_slist_nth_data(
					    aref->pobj->anims, aref->offs),
					    aref->frame,
					    (vx << TILESHIFT) + aref->xoffs,
					    (vy << TILESHIFT) + aref->yoffs);
					
					if (anim->delay > 0 &&
					    aref->fwait++ > anim->delay) {
						if (aref->frame++ >=
						    anim->nframes - 1) {
							aref->frame = 0;
						}
						aref->fwait = 0;
						/*
						 * XXX wait the number of
						 * ticks a blit usually
						 * takes.
						 */
					}
				}
				pthread_mutex_unlock(&aref->pobj->lock);
			}
			if (curmapedit != NULL) {
				mapedit_drawflags(m, me->flags, vx, vy);
				if (curmapedit->flags & MAPEDIT_TILELIST)
					mapedit_tilelist(curmapedit);
			}
		}
	}
	pthread_mutex_unlock(&m->lock);
	SDL_UpdateRect(m->view->v, 0, 0, 0, 0);

	return (ival);
}

static void
mapedit_drawflags(struct map *m, int flags, int vx, int vy)
{
	vx <<= TILESHIFT;
	vy <<= TILESHIFT;

	if (curmapedit->flags & MAPEDIT_DRAWGRID)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_GRID), vx, vy);
	if ((curmapedit->flags & MAPEDIT_DRAWPROPS) == 0)
		return;

	if (flags == 0) {
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_BLOCKED), vx, vy);
		return;
	}
	if (flags & MAPENTRY_ORIGIN)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_ORIGIN), vx, vy);
#if 0
	if (flags & MAPENTRY_WALK)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_WALK), vx, vy);
#endif
	if (flags & MAPENTRY_CLIMB)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_CLIMB), vx, vy);
	if (flags & MAPENTRY_SLIP)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_SLIP), vx, vy);
	if (flags & MAPENTRY_BIO)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_BIO), vx, vy);
	if (flags & MAPENTRY_REGEN)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_REGEN), vx, vy);
	if (flags & MAPENTRY_SLOW)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_SLOW), vx, vy);
	if (flags & MAPENTRY_HASTE)
		map_plot_sprite(m, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_HASTE), vx, vy);
}

/* Draw all sprites in the map view. */
static Uint32
map_draw(Uint32 ival, void *p)
{
	struct map *m = (struct map *)p;
	int x, y;
	int vx, vy;

	if (m->redraw == 0) {
		return (ival);
	}
	m->redraw = 0;

	if (pthread_mutex_lock(&m->lock) != 0) {
		perror(m->obj.name);
		return (ival);
	}

	for (y = m->view->mapy, vy = 0;
	     y < m->view->maph + m->view->mapy + 1;
	     y++, vy++) {

	     	/* XXX */
		if (curmapedit != NULL &&
		    vy == 0 && curmapedit->flags & MAPEDIT_OBJLIST) {
			continue;
		}

		for (x = m->view->mapx, vx = 0;
		     x < m->view->mapw + m->view->mapx + 1;
		     x++, vx++) {
			static struct node *me;
			static struct map_aref *aref;

			/* XXX */
			if (curmapedit != NULL) {
				if (vx == 0 &&
				    curmapedit->flags & MAPEDIT_TILESTACK) {
					continue;
				}
				if (vx == m->view->mapw &&
				    curmapedit->flags & MAPEDIT_TILELIST) {
					continue;
				}
			}

			me = &m->map[x][y];

			if (me->nanims > 0) {
				/* map_animate() shall handle this. */
				continue;
			}
	
			if (curmapedit != NULL) {
				static SDL_Rect erd;

				/* XXX draw a background on view area. */
				erd.w = m->view->tilew;
				erd.h = m->view->tileh;
				erd.x = vx * erd.w;
				erd.y = vy * erd.h;
				SDL_FillRect(m->view->v, &erd, 25);
			}

			TAILQ_FOREACH(aref, &me->arefsh, marefs) {
				if (aref->flags & MAPREF_SPRITE) {
					map_plot_sprite(m,
					    g_slist_nth_data(
					    aref->pobj->sprites, aref->offs),
					    (vx << TILESHIFT) + aref->xoffs,
					    (vy << TILESHIFT) + aref->yoffs);
				}
			}

			if (curmapedit != NULL) {
				mapedit_drawflags(m, me->flags, vx, vy);
			}
		}
	}

	if (curmapedit != NULL) {
		if (curmapedit->flags & MAPEDIT_OBJLIST)
			mapedit_objlist(curmapedit);
		if (curmapedit->flags & MAPEDIT_TILESTACK)
			mapedit_tilestack(curmapedit);
	}
	pthread_mutex_unlock(&m->lock);

	SDL_UpdateRect(m->view->v, 0, 0, 0, 0);

	return (ival);
}

/*
 * Return the map entry reference for the given index.
 */
struct map_aref *
node_arefindex(struct node *me, int index)
{
	static int i;
	struct map_aref *aref;

	/* XXX bsearch */
	i = 0;
	TAILQ_FOREACH(aref, &me->arefsh, marefs) {
		if (i++ == index) {
			return (aref);
		}
	}

#ifdef DEBUG
	fatal("nothing at %d\n", index);
#endif
	return (NULL);
}

/*
 * Return the map entry reference for ob:offs, or the first match
 * for ob if offs is -1.
 */
struct map_aref *
node_arefobj(struct node *me, struct object *ob, int offs)
{
	struct map_aref *aref;

	/* XXX bsearch */
	TAILQ_FOREACH(aref, &me->arefsh, marefs) {
		if (aref->pobj == ob && (aref->offs == offs || offs < 0)) {
			return (aref);
		}
	}

#if 0
	dprintf("no reference to %s:%d\n", ob->name, offs);
#endif
	return (NULL);
}

/*
 * Load a map from file. The map must be already locked.
 */
static int
map_load(void *ob, char *path)
{
	char magic[9];
	struct map *m = (struct map *)ob;
	int vermin, vermaj;
	int f, x, y, refs = 0;

	f = open(path, O_RDONLY);
	if (f < 0) {
		perror(path);
		return (-1);
	}

	/* Verify the signature and version major. */
	if (read(f, magic, 10) != 10)
		goto badmagic;
	if (strncmp(magic, MAP_MAGIC, 10) != 0)
		goto badmagic;
	vermaj = (int) fobj_read_uint32(f);
	vermin = (int) fobj_read_uint32(f);
	if (vermaj > MAP_VERMAJ ||
	    (vermaj == MAP_VERMAJ && vermin > MAP_VERMIN))
		goto badver;

	/* Read map information. */
	m->flags = (int) fobj_read_uint32(f);
	m->mapw  = (int) fobj_read_uint32(f);
	m->maph  = (int) fobj_read_uint32(f);
	m->defx  = (int) fobj_read_uint32(f);
	m->defy  = (int) fobj_read_uint32(f);
	m->view->tilew = (int) fobj_read_uint32(f);
	m->view->tileh = (int) fobj_read_uint32(f);

	dprintf("%s: v%d.%d flags 0x%x geo %dx%d tilegeo %dx%d\n",
	    path, vermaj, vermin, m->flags, m->mapw, m->maph,
	    m->view->tilew, m->view->tileh);

	/* Adapt the viewport to this tile geometry. */
	view_setmode(m->view);

	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *me = &m->map[x][y];
			int i, narefs;
			
			/* Read the map entry flags. */
			me->flags = fobj_read_uint32(f);

			/* Read the optional integer value. */
			me->v1 = fobj_read_uint32(f);

			/* Read the reference count. */
			narefs = fobj_read_uint32(f);

			for (i = 0; i < narefs; i++) {
				struct object *pobj;
				struct map_aref *naref;
				char *pobjstr;
				int offs, frame, rflags;

				/* Read object:offset reference. */
				pobjstr = fobj_read_string(f);
				offs = fobj_read_uint32(f);
				frame = fobj_read_uint32(f);
				rflags = fobj_read_uint32(f);
				pobj = object_strfind(pobjstr);

				if (pobj == NULL) {
					fatal("no match for \"%s\"\n", pobjstr);
					return (-1);
				}
				free(pobjstr);

				naref = node_addref(me, pobj, offs, rflags);
				if (naref == NULL) {
					return (-1);
				}
				naref->frame = frame;

				refs++;
			}
		}
	}

	dprintf("%s: %d refs, origin: %dx%d\n", path, refs, m->defx, m->defy);
	return (0);

badver:
	fatal("map version %d.%d > %d.%d\n", vermaj, vermin,
	    MAP_VERMAJ, MAP_VERMIN);
	close(f);
	return (-1);
badmagic:
	fatal("bad magic\n");
	close(f);
	return (-1);
}

/*
 * Save a map to a file. The map must be already locked.
 */
int
map_save(void *ob, char *path)
{
	struct map *m = (struct map *)ob;
	int x = 0, y, f;
	int refs = 0;

	f = open(path, O_WRONLY|O_CREAT|O_TRUNC, 00600);
	if (f < 0) {
		perror(path);
		return (-1);
	}

	/* Write the signature and version. */
	write(f, MAP_MAGIC, 10);
	fobj_write_uint32(f, MAP_VERMAJ);
	fobj_write_uint32(f, MAP_VERMIN);

	/* Write map information. */
	fobj_write_uint32(f, m->flags);
	fobj_write_uint32(f, m->mapw);
	fobj_write_uint32(f, m->maph);
	fobj_write_uint32(f, m->defx);
	fobj_write_uint32(f, m->defy);
	fobj_write_uint32(f, m->view->tilew);
	fobj_write_uint32(f, m->view->tileh);

	/*
	 * Write the map entries.
	 */
	for (y = 0; y < m->maph; y++) {
		for (x = 0; x < m->mapw; x++) {
			struct node *me = &m->map[x][y];
			off_t soffs;
			int i, count;
			
			/* Write the map entry flags. */
			fobj_write_uint32(f, me->flags);
			
			/* Write the optional integer value. */
			fobj_write_uint32(f, me->v1);

			/* We do not know the reference count yet. */
			soffs = lseek(f, 0, SEEK_CUR);
			lseek(f, sizeof(Uint32), SEEK_CUR);

			for (i = 0, count = 0; i < me->narefs + 1; i++) {
				struct map_aref *aref;

				aref = node_arefindex(me, i);
				if (aref != NULL && aref->flags & MAPREF_SAVE) {
					fobj_write_string(f, aref->pobj->name);
					fobj_write_uint32(f, aref->offs);
					fobj_write_uint32(f, aref->frame);
					fobj_write_uint32(f, aref->flags);

					count++;
					refs++;
				}
			}

			/* Write the count. */
			fobj_pwrite_uint32(f, count, soffs);
		}
	}

	close(f);
	dprintf("%s: %dx%d, %d refs\n", path, m->mapw, m->maph, refs);

	return (0);
}

#ifdef DEBUG
void
map_dump(struct map *m)
{
	printf("%3d. %10s geo %dx%d flags 0x%x\n", m->obj.id, m->obj.name,
	    m->mapw, m->maph, m->flags);
}
#endif /* DEBUG */

