/*	$Csoft: map.c,v 1.5 2002/01/30 17:53:37 vedge Exp $	*/

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
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <libfobj/fobj.h>

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

struct	map *curmap;
struct	mapedit	*curmapedit;
int	mapedit;

static int	 map_link(void *);
static int	 map_load(void *, char *);
static int	 map_save(void *, char *);
static void	 map_destroy(struct object *);
static void	 map_entry_destroy(struct map_entry *);
static int	 map_entry_init(struct map_entry *, struct object *,
		     int, int, int);
static void	 mapedit_drawflags(struct map *, int, int, int);
static Uint32	 map_draw(Uint32, void *);
static Uint32	 map_animate(Uint32, void *);

struct map *
map_create(char *name, char *desc, int flags, int width, int height, char *path)
{
	size_t mapsize = 0;
	struct map *em;
	int x = 0, y = 0;

	em = (struct map *)malloc(sizeof(struct map));
	if (em == NULL) {
		perror("map");
		return (NULL);
	}

	/* Initialize the map structure. */
	object_create(&em->obj, name, desc,
	    DESTROY_HOOK|LOAD_FUNC|SAVE_FUNC|OBJ_USED);
	em->obj.destroy_hook = map_destroy;

	em->obj.load = map_load;
	em->obj.save = map_save;

	em->flags = flags;
	em->maph = height;
	em->mapw = width;
	em->defx = em->mapw / 2;
	em->defy = em->maph - 1;
	em->view = mainview;
	em->view->map = em;

	if (pthread_mutex_init(&em->lock, NULL) != 0) {
		perror(name);
		goto maperr;
	}

	if (path != NULL) {
		/* Load this map from a file. */
		map_load(em, path);
		mapsize = (em->mapw * em->maph) * sizeof(struct map_entry);
	} else {
		/* Initialize an empty map. */
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (map_entry_init(&em->map[x][y], NULL,
				    0, 0, 0) < 0) {
					return (NULL);
				}
				mapsize += sizeof(struct map_entry);
			}
		}
	}
	
	dprintf("%s: geo %dx%d flags 0x%x base %dKb\n", em->obj.name,
	    em->mapw, em->mapw,
	    em->flags, (int)mapsize / 1024);
	dprintf("%s: tilegeo %dx%d origin %dx%d\n", em->obj.name,
	    em->view->tilew, em->view->tileh,
	    em->defx, em->defy);

	map_link(em);
	object_link(em);

	/* Synchronize map display. XXX tune */
	em->view->mapanimt = NULL;
	em->view->mapdrawt = SDL_AddTimer(15, map_draw, em);
	if (em->view->mapdrawt == NULL) {
		fatal("SDL_AddTimer: %s\n", SDL_GetError());
		goto maperr;
	}

	return (em);
maperr:
	object_destroy(em, NULL);
	return (NULL);
}

int
map_animset(struct map *em, int tog)
{
	if (tog > 0) {
		em->view->mapanimt = SDL_AddTimer(em->view->fps * 2,
		    map_animate, em);
		if (em->view->mapanimt == NULL) {
			fatal("SDL_AddTimer: %s\n", SDL_GetError());
			return (0);
		}
	} else {
		if (em->view->mapanimt != NULL) {
			SDL_RemoveTimer(em->view->mapanimt);
			em->view->mapanimt = NULL;
		}
	}

	return (0);
}

int
map_focus(struct map *em)
{
	curmap = em;

	if (mapedit) {
		char s[128];
		
		sprintf(s, "%s (edition)", em->obj.name);
		SDL_WM_SetCaption(s, "mapedit");
	} else {
		SDL_WM_SetCaption(em->obj.name, "mapedit");
	}
	return (0);
}

static int
map_link(void *objp)
{
	if (pthread_mutex_lock(&world->lock) == 0) {
		world->maps = g_slist_append(world->maps, objp);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}
	world->nmaps++;
	return (0);
}

/*
 * Initialize a map entry.
 */
static int
map_entry_init(struct map_entry *me, struct object *ob, int offs,
    int meflags, int rflags)
{
	me->objs = NULL;
	me->nobjs = 0;
	me->flags = 0;
	me->nanims = 0;

	/* Used by the map editor to fill new maps. */
	if (ob != NULL) {
		map_entry_addref(me, ob, offs, rflags);
		me->flags |= meflags;
	}
	return (0);
}

/* Add map entry reference. */
struct map_aref *
map_entry_addref(struct map_entry *me, struct object *ob, int offs, int rflags)
{
	struct map_aref *naref;

	naref = malloc(sizeof(struct map_aref));
	if (naref == NULL) {
		perror("map_aref");
		return (NULL);
	}
	naref->index = me->nobjs;
	naref->pobj = ob;
	naref->offs = offs;
	naref->flags = rflags;

	if (rflags & MAPREF_ANIM) {
		naref->frame = 0;
		naref->fwait = 0;
		me->nanims++;
	}

	me->objs = g_slist_append(me->objs, (void *)naref);
	me->nobjs++;

	return (naref);
}

/*
 * Delete a map entry reference.
 */
int
map_entry_delref(struct map_entry *me, struct map_aref *aref)
{
	if (aref->flags & MAPREF_ANIM) {
		me->nanims--;
	}

	me->objs = g_slist_remove(me->objs, aref);
	me->nobjs--;
	free(aref);
	return (0);
}

/* Reinitialize a map to zero. Must be called on a locked map. */
void
map_clean(struct map *em, struct object *ob, int offs, int flags, int rflags)
{
	size_t mapsize = 0;
	int x = 0, y;

	/* XXX redundant */
	for (y = 0; y < em->maph; y++) {
		for (x = 0; x < em->mapw ; x++) {
			map_entry_destroy(&em->map[x][y]);
		}
	}
	
	/* Initialize the nodes. */
	for (y = 0; y < em->maph; y++) {
		for (x = 0; x < em->mapw; x++) {
			if (map_entry_init(&em->map[x][y], ob, offs, flags,
			    rflags) < 0) {
				return;
			}
			mapsize += sizeof(struct map_entry);
		}
	}

	dprintf("initialized %dx%d map origin at %dx%d\n", x, y,
	    em->defx, em->defy);
	em->map[em->defx][em->defy].flags |= MAPENTRY_ORIGIN;
}

static void
map_destroy(struct object *ob)
{
	struct map *em = (struct map *)ob;
	int x = 0, y;

	/* Disable animations. */
	map_animset(em, 0);
	SDL_RemoveTimer(em->view->mapdrawt);
	em->view->mapdrawt = NULL;

	/* Unlink the map from the world. */
	if (pthread_mutex_lock(&world->lock) == 0) {
		world->maps = g_slist_remove(world->maps, (void *) ob);
		world->nmaps--;
		pthread_mutex_unlock(&world->lock);
	}

	/* XXX redundant */
	if (pthread_mutex_lock(&em->lock) == 0) {
		for (y = 0; y < em->maph; y++) {
			for (x = 0; x < em->mapw ; x++) {
				map_entry_destroy(&em->map[x][y]);
			}
		}
		pthread_mutex_unlock(&em->lock);
		pthread_mutex_destroy(&em->lock);
	} else {
		perror(em->obj.name);
	}
}

static void
map_entry_destroy(struct map_entry *me)
{
	int i;

	for (i = 0; i < me->nobjs; i++) {
		struct map_aref *aref;

		aref = g_slist_nth_data(me->objs, i);
		me->objs = g_slist_remove(me->objs, aref);
		free(aref);
	}
}

/* Draw a sprite at any given location. */
static __inline void
map_plot_sprite(struct map *em, SDL_Surface *s, int mapx, int mapy)
{
	static SDL_Rect rs, rd;

#ifdef DEBUG
	if (s == NULL) {
		fatal("NULL surface\n");
		abort();
	}
#endif

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;

	if (em->flags & MAP_VARTILEGEO) {
		/*
		 * The sprite size should be a multiple of the tile size.
		 * XXX optimize math.
		 */
		if (rd.w > em->view->tilew) {
			mapx -= (rs.w / em->view->tilew) / 2;
		}
		if (rd.h > em->view->tileh) {
			mapy -= (rs.h / em->view->tileh) / 2;
		}
	}

	/* XXX pre-compute map/view coordinate pairs. */
	rd.x = mapx * em->view->tilew;
	rd.y = mapy * em->view->tileh;

	SDL_BlitSurface(s, &rs, em->view->v, &rd);
}

/* Draw an animation's current frame. */
static __inline void
map_plot_anim(struct map *em, struct anim *anim, int frame, int mapx, int mapy)
{
	static SDL_Rect rs, rd;
	static SDL_Surface *s;

	s = g_slist_nth_data(anim->frames, frame);

#ifdef DEBUG
	if (s == NULL) {
		fatal("NULL surface (frame %d)\n", frame);
	}
#endif

	rs.w = s->w;
	rs.h = s->h;
	rs.x = 0;
	rs.y = 0;

	rd.w = s->w;
	rd.h = s->h;

	/*
	 * XXX don't bother, animations must be tile-sized for now.
	 */
#if 0
	if (em->flags & MAP_VARTILEGEO) {
		if (rd.w > em->view->tilew) {
			mapx -= (rs.w / em->view->tilew) / 2;
		}
		if (rd.h > em->view->tileh) {
			mapy -= (rs.h / em->view->tileh) / 2;
		}
	}
#endif

	rd.x = mapx * em->view->tilew;
	rd.y = mapy * em->view->tileh;

	SDL_BlitSurface(s, &rs, em->view->v, &rd);
}

/* Update all animations in the map view. */
static Uint32
map_animate(Uint32 ival, void *p)
{
	struct map *em = (struct map *)p;
	int x, y;
	int vx, vy;

	if (pthread_mutex_lock(&em->lock) != 0) {
		perror(em->obj.name);
		return (ival);
	}
	for (y = em->view->mapy, vy = 0;
	    (y < em->view->maph + em->view->mapy + 1);
	     y++, vy++) {
#if 1
		if (mapedit && curmapedit->flags & MAPEDIT_OBJLIST &&
		    vy == 0) {
			continue;
		}
#endif
		for (x = em->view->mapx, vx = 0;
		     x < em->view->mapw + em->view->mapx + 1;
		     x++, vx++) {
			struct map_entry *me;
			int i;
#if 1
			if (mapedit &&
			    curmapedit->flags & MAPEDIT_TILELIST &&
			    vx == em->view->mapw) {
				continue;
			}
#endif

			me = &em->map[x][y];

			if (me->nanims < 1) {
				continue;
			}
#if 1
			if (mapedit && curmapedit->flags & MAPEDIT_TILESTACK &&
			    vx == 0) {
				continue;
			}
#endif

			for (i = 0; i < me->nobjs; i++) {
				static struct map_aref *taref;

				taref = g_slist_nth_data(me->objs, i);

				if (pthread_mutex_lock(&taref->pobj->lock)
				    != 0) {
					perror(taref->pobj->name);
					continue;
				}
				/*
				 * We must also redraw underlying sprites.
				 * XXX save a mask instead?
				 */
				if (taref->flags & MAPREF_SPRITE) {
					map_plot_sprite(em,
					    g_slist_nth_data(
					    taref->pobj->sprites, taref->offs),
					    vx, vy);
				} else if (taref->flags & MAPREF_ANIM) {
					struct anim *anim;

					anim = g_slist_nth_data(
					    taref->pobj->anims, taref->offs);
					map_plot_anim(em,
					    g_slist_nth_data(
					    taref->pobj->anims, taref->offs),
					    taref->frame,
					    vx, vy);
					
					if (anim->delay > 0 &&
					    taref->fwait++ > anim->delay) {
						if (taref->frame++ >=
						    anim->nframes - 1) {
							taref->frame = 0;
						}
						taref->fwait = 0;
						/*
						 * XXX wait the number of
						 * ticks a blit usually
						 * takes.
						 */
					}
				}
				pthread_mutex_unlock(&taref->pobj->lock);
			}
			if (mapedit) {
				mapedit_drawflags(em, me->flags, vx, vy);
			}
		}
	}
	pthread_mutex_unlock(&em->lock);
	SDL_UpdateRect(em->view->v, 0, 0, 0, 0);

	return (ival);
}

static void
mapedit_drawflags(struct map *em, int flags, int vx, int vy)
{
	if (flags == 0)	{
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_BLOCKED), vx, vy);
		return;
	}
	if (flags & MAPENTRY_ORIGIN)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_ORIGIN), vx, vy);
	if (flags & MAPENTRY_WALK)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_WALK), vx, vy);
	if (flags & MAPENTRY_CLIMB)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_CLIMB), vx, vy);
	if (flags & MAPENTRY_SLIP)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_SLIP), vx, vy);
	if (flags & MAPENTRY_BIO)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_BIO), vx, vy);
	if (flags & MAPENTRY_REGEN)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_REGEN), vx, vy);
	if (flags & MAPENTRY_SLOW)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_SLOW), vx, vy);
	if (flags & MAPENTRY_HASTE)
		map_plot_sprite(em, g_slist_nth_data(curmapedit->obj.sprites,
		    MAPEDIT_HASTE), vx, vy);
}

/* Draw all sprites in the map view. */
static Uint32
map_draw(Uint32 ival, void *p)
{
	struct map *em = (struct map *)p;
	int x, y;
	int vx, vy;

	if (em->redraw == 0) {
		return (ival);
	}
	em->redraw = 0;

	if (pthread_mutex_lock(&em->lock) != 0) {
		perror(em->obj.name);
		return (ival);
	}

	for (y = em->view->mapy, vy = 0;
	     y < em->view->maph + em->view->mapy + 1;
	     y++, vy++) {
#if 1
		if (mapedit && curmapedit->flags & MAPEDIT_OBJLIST &&
		    vy == 0) {
			continue;
		}
#endif
		for (x = em->view->mapx, vx = 0;
		     x < em->view->mapw + em->view->mapx + 1;
		     x++, vx++) {
			struct map_entry *me;
			struct map_aref *taref;
			int i;

#if 1
			if (mapedit && curmapedit->flags & MAPEDIT_TILESTACK &&
			    vx == 0) {
				continue;
			}
#endif

			me = &em->map[x][y];

			if (me->nanims > 0) {
				continue;
			}
	
			if (mapedit) {
				SDL_Rect erd;

				erd.w = em->view->tilew;
				erd.h = em->view->tileh;
				erd.x = vx * erd.w;
				erd.y = vy * erd.h;
				SDL_FillRect(em->view->v, &erd, 25);
			}

			/* XXX inefficient */
			for (i = 0; i < me->nobjs; i++) {
				taref = g_slist_nth_data(me->objs, i);

				if (taref->flags & MAPREF_SPRITE) {
					struct object *ob = taref->pobj;

					map_plot_sprite(em,
					    g_slist_nth_data(ob->sprites,
					    taref->offs), vx, vy);
				}
			}

			if (mapedit) {
				mapedit_drawflags(em, me->flags, vx, vy);
			}
		}
	}
	if (mapedit) {
		if (curmapedit->flags & MAPEDIT_TILELIST)
			mapedit_tilelist(curmapedit);
		if (curmapedit->flags & MAPEDIT_TILESTACK)
			mapedit_tilestack(curmapedit);
		if (curmapedit->flags & MAPEDIT_OBJLIST)
			mapedit_objlist(curmapedit);
	}
	pthread_mutex_unlock(&em->lock);

	SDL_UpdateRect(em->view->v, 0, 0, 0, 0);

	return (ival);
}

/*
 * Return the map entry reference for the given index.
 */
struct map_aref *
map_entry_aref(struct map_entry *me, int index)
{
#if 0
	int i;

	/* XXX very stupid */
	for (i = 0; i < me->nobjs; i++) {
		struct map_aref *aref;

		aref = g_slist_nth_data(me->objs, i);
#ifdef DEBUG
		if (aref == NULL) {
			/* XXX should not happen */
			dprintf("NULL aref\n");
			continue;
		}
#endif
		if (aref->index == index) {
			return (aref);
		}
	}
#else
	return (g_slist_nth_data(me->objs, index));
#endif
	return (NULL);
}

/*
 * Return the map entry reference for ob:offs, or the first match
 * for ob if offs is -1.
 */
struct map_aref *
map_entry_arefobj(struct map_entry *me, struct object *ob, int offs)
{
	int i;

	/* XXX inefficient */
	for (i = 0; i < me->nobjs; i++) {
		struct map_aref *aref;

		aref = g_slist_nth_data(me->objs, i);
		if (aref->pobj == ob && (aref->offs == offs || offs < 0)) {
			return (aref);
		}
	}

#ifdef DEBUG
	if (offs > -1) {
		dprintf("no reference to %s:%d\n", ob->name, offs);
	}
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
	struct map *em = (struct map *)ob;
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
	em->flags = (int) fobj_read_uint32(f);
	em->mapw  = (int) fobj_read_uint32(f);
	em->maph  = (int) fobj_read_uint32(f);
	em->defx  = (int) fobj_read_uint32(f);
	em->defy  = (int) fobj_read_uint32(f);
	em->view->tilew = (int) fobj_read_uint32(f);
	em->view->tileh = (int) fobj_read_uint32(f);

	dprintf("%s: v%d.%d flags 0x%x geo %dx%d tilegeo %dx%d\n",
	    path, vermaj, vermin, em->flags, em->mapw, em->maph,
	    em->view->tilew, em->view->tileh);

	/* Adapt the viewport to this tile geometry. */
	view_setmode(em->view);

	for (y = 0; y < em->maph; y++) {
		for (x = 0; x < em->mapw; x++) {
			struct map_entry *me = &em->map[x][y];
			int i, nobjs;
			
			/* Read the map entry flags. */
			me->flags = fobj_read_uint32(f);

			/* Read the optional integer value. */
			me->v1 = fobj_read_uint32(f);

			/* Read the reference count. */
			nobjs = fobj_read_uint32(f);

			for (i = 0; i < nobjs; i++) {
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

				naref = map_entry_addref(me, pobj, offs,
				    rflags);
				if (naref == NULL) {
					return (-1);
				}
				naref->frame = frame;

				refs++;
			}
		}
	}

	dprintf("%s: %d refs, origin: %dx%d\n", path, refs, em->defx, em->defy);
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
	struct map *em = (struct map *)ob;
	int x = 0, y;
	int f;
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
	fobj_write_uint32(f, em->flags);
	fobj_write_uint32(f, em->mapw);
	fobj_write_uint32(f, em->maph);
	fobj_write_uint32(f, em->defx);
	fobj_write_uint32(f, em->defy);
	fobj_write_uint32(f, em->view->tilew);
	fobj_write_uint32(f, em->view->tileh);

	/*
	 * Write the map entries.
	 */
	for (y = 0; y < em->maph; y++) {
		for (x = 0; x < em->mapw; x++) {
			struct map_entry *me = &em->map[x][y];
			off_t soffs;
			int i, count;
			
			/* Write the map entry flags. */
			fobj_write_uint32(f, me->flags);
			
			/* Write the optional integer value. */
			fobj_write_uint32(f, me->v1);

			/* We do not know the reference count yet. */
			soffs = lseek(f, 0, SEEK_CUR);
			lseek(f, sizeof(Uint32), SEEK_CUR);

			for (i = 0, count = 0; i < me->nobjs + 1; i++) {
				struct map_aref *aref;

				aref = map_entry_aref(me, i);
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
	dprintf("%s: %dx%d, %d refs\n", path, em->mapw, em->maph, refs);

	return (0);
}

#ifdef DEBUG
void
map_dump_map(void *p, void *arg)
{
	struct map *fm = (struct map *)p;

	printf("%3d. %10s geo %dx%d flags 0x%x\n", fm->obj.id, fm->obj.name,
	    fm->mapw, fm->maph, fm->flags);
}
#endif /* DEBUG */

