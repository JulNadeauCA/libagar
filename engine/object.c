/*	$Csoft: object.c,v 1.62 2002/06/10 04:27:06 vedge Exp $	*/

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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

enum {
	NANIMS_INIT =	4,
	NSPRITES_INIT =	4,
	NANIMS_GROW =	2,
	NSPRITES_GROW = 2,
};

static LIST_HEAD(, object_art) artsh =	LIST_HEAD_INITIALIZER(&artsh);
static LIST_HEAD(, object_audio) audiosh = LIST_HEAD_INITIALIZER(&audiosh);
static pthread_mutex_t media_lock = PTHREAD_MUTEX_INITIALIZER;
static SDL_TimerID gctimer;

static const struct object_ops null_ops = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL	/* save */
};

static struct object_art	*object_get_art(char *);
static struct object_audio	*object_get_audio(char *);

int
object_addanim(struct object_art *art, struct anim *anim)
{
	if (art->anims == NULL) {			/* Initialize */
		art->anims = emalloc(NANIMS_INIT * sizeof(struct anim *));
		art->maxanims = NANIMS_INIT;
		art->nanims = 0;
	} else if (art->nanims >= art->maxanims) {	/* Grow */
		struct anim **newanims;

		newanims = erealloc(art->anims,
		    (NANIMS_GROW * art->maxanims) * sizeof(struct anim *));
		art->maxanims *= NANIMS_GROW;
		art->anims = newanims;
	}
	art->anims[art->nanims++] = anim;
	return (0);
}

int
object_breaksprite(struct object_art *art, SDL_Surface *sprite)
{
	int x, y;
	SDL_Rect sd, rd;

	sd.w = TILEW;
	sd.h = TILEH;
	rd.x = 0;
	rd.y = 0;

	for (y = 0; y < sprite->h; y += TILEH) {
		for (x = 0; x < sprite->w; x += TILEW) {
			SDL_Surface *s;

			s = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
			    TILEW, TILEH, 32,
			    0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
			if (s == NULL) {
				fatal("SDL_AllocSurface: %s\n", SDL_GetError());
			}

			SDL_SetAlpha(sprite, 0, 0);
			sd.x = x;
			sd.y = y;
			SDL_BlitSurface(sprite, &sd, s, &rd);
			object_addsprite(art, s);
			SDL_SetAlpha(sprite, SDL_SRCALPHA,
			    SDL_ALPHA_TRANSPARENT);
		}
	}
	SDL_FreeSurface(sprite);

	return (0);
}

int
object_addsprite(struct object_art *art, SDL_Surface *sprite)
{
	if (art->sprites == NULL) {			/* Initialize */
		art->sprites = emalloc(NSPRITES_INIT * sizeof(SDL_Surface *));
		art->maxsprites = NSPRITES_INIT;
		art->nsprites = 0;
	} else if (art->nsprites >= art->maxsprites) {	/* Grow */
		SDL_Surface **newsprites;

		newsprites = erealloc(art->sprites,
		    (NSPRITES_GROW * art->maxsprites) * sizeof(SDL_Surface *));
		art->maxsprites *= NSPRITES_GROW;
		art->sprites = newsprites;
	}
	art->sprites[art->nsprites++] = sprite;
	return (0);
}

/* Load images into the media pool. */
static struct object_art *
object_get_art(char *media)
{
	struct object_art *art = NULL, *eart;

	pthread_mutex_lock(&media_lock);
	LIST_FOREACH(eart, &artsh, arts) {
		if (strcmp(eart->name, media) == 0) {
			art = eart;	/* Already in pool */
		}
	}

	if (art == NULL) {
		struct fobj *fob;
		char *obpath;
		Uint32 i;

		art = emalloc(sizeof(struct object_art));
		art->name = strdup(media);
		art->sprites = NULL;
		art->nsprites = 0;
		art->maxsprites = 0;
		art->anims = NULL;
		art->nanims = 0;
		art->maxanims = 0;
		art->used = 1;
		pthread_mutex_init(&art->used_lock, NULL);

		obpath = object_path(media, "fob");
		if (obpath == NULL) {
			fatal("%s: %s\n", media, AGAR_GetError());
		}
		fob = fobj_load(obpath);
		for (i = 0; i < fob->head.nobjs; i++) {	/* XXX broken */
			xcf_load(fob, i, art);

		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&artsh, art, arts);
	}
	pthread_mutex_unlock(&media_lock);

	return (art);
}

/* Load audio into the media pool. */
static struct object_audio *
object_get_audio(char *media)
{
	struct object_audio *audio = NULL, *eaudio;

	pthread_mutex_lock(&media_lock);

	LIST_FOREACH(eaudio, &audiosh, audios) {
		if (strcmp(eaudio->name, media) == 0) {
			audio = eaudio;	/* Already in pool */
		}
	}

	if (audio == NULL) {
		struct fobj *fob;
		Uint32 i;
		char *obpath;

		audio = emalloc(sizeof(struct object_audio));
		audio->name = strdup(media);
		/* XXX todo */
		audio->samples = NULL;
		audio->nsamples = 0;
		audio->maxsamples = 0;
		audio->used = 0;

		obpath = object_path(media, "fob");
		if (obpath == NULL) {
			fatal("%s: %s\n", media, AGAR_GetError());
		}

		fob = fobj_load(obpath);
		for (i = 0; i < fob->head.nobjs; i++) {	/* XXX todo */
			/* ... */
		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&audiosh, audio, audios);
	}

	pthread_mutex_unlock(&media_lock);
	return (audio);
}

char *
object_name(char *base, int num)
{
	char *name;

	name = emalloc(strlen(base) + 16);
	sprintf(name, "%s%d", base, num);
	return (name);
}

struct object *
object_new(char *type, char *name, char *media, int flags, const void *opsp)
{
	struct object *ob;

	ob = emalloc(sizeof(struct object));
	object_init(ob, type, name, media, flags, opsp);

	pthread_mutex_lock(&world->lock);
	world_attach(world, ob);
	pthread_mutex_unlock(&world->lock);

	return (ob);
}

void
object_init(struct object *ob, char *type, char *name, char *media, int flags,
    const void *opsp)
{
	static int curid = 0;	/* The world has id 0. XXX safe? */

	ob->id = curid++;
	ob->type = strdup(type);
	ob->name = strdup(name);
	ob->desc = NULL;
	sprintf(ob->saveext, "ag");
	ob->ops = (opsp != NULL) ? opsp : &null_ops;
	ob->flags = flags;
	ob->pos = NULL;
	ob->maps = NULL;
	TAILQ_INIT(&ob->events);
	pthread_mutex_init(&ob->pos_lock, NULL);
	pthread_mutex_init(&ob->events_lock, NULL);
	pthread_mutex_init(&ob->maps_lock, NULL);

	ob->art = (ob->flags & OBJ_ART) ? object_get_art(media) : NULL;
	ob->audio = (ob->flags & OBJ_AUDIO) ? object_get_audio(media) : NULL;
}

/* Object must not be attached. */
void
object_destroy(void *p)
{
	struct object *ob = p;
	struct event *eev, *nexteev;
	
	if (OBJECT_OPS(ob)->destroy != NULL) {
		OBJECT_OPS(ob)->destroy(ob);
	}
	
	pthread_mutex_lock(&ob->events_lock);	/* XXX */
	for (eev = TAILQ_FIRST(&ob->events);
	     eev != TAILQ_END(&ob->events);
	     eev = nexteev) {
		nexteev = TAILQ_NEXT(eev, events);
		free(eev);
	}
	pthread_mutex_unlock(&ob->events_lock);
	pthread_mutex_destroy(&ob->events_lock);

	if ((ob->flags & OBJ_KEEPMEDIA) == 0) {
		if (ob->art != NULL) {
			OBJECT_UNUSED(ob, art);
		}
		if (ob->audio != NULL) {
			OBJECT_UNUSED(ob, audio);
		}
	}
	
	ob->pos = NULL;
	ob->maps = NULL;
	pthread_mutex_destroy(&ob->pos_lock);
	pthread_mutex_destroy(&ob->maps_lock);

	if (ob->name != NULL)
		free(ob->name);
	if (ob->type != NULL)
		free(ob->type);
	if (ob->desc != NULL)
		free(ob->desc);

	free(ob);
}

/* Perform deferred garbage collection. */
Uint32
object_start_gc(Uint32 ival, void *p)
{
	struct object_audio *audio;
	struct object_art *art, *nextart;

	pthread_mutex_lock(&media_lock);

	/* Art pool */
	for (art = LIST_FIRST(&artsh);
	     art != LIST_END(&artsh);
	     art = nextart) {
		nextart = LIST_NEXT(art, arts);
		if (art->used < 1) {
			int i;

			for (i = 0; i < art->nsprites; i++) {
				free(art->sprites[i]);
			}
			for (i = 0; i < art->nanims; i++) {
				anim_destroy(art->anims[i]);
			}
#if 0
			dprintf("freed: %s (%d sprites, %d anims)\n",
			    art->name, art->nsprites, art->nanims);
#endif

			free(art->name);
			free(art);
		}
	}

	/* Audio pool */
	LIST_FOREACH(audio, &audiosh, audios) {
		if (audio->used < 1) {
			/* gc */
			dprintf("gc audio\n");
		}
	}

	pthread_mutex_unlock(&media_lock);
	return (ival);
}

/* Initialize the garbage collector. */
void
object_init_gc(void)
{
	gctimer = SDL_AddTimer(1000, object_start_gc, NULL);
}

void
object_destroy_gc(void)
{
	SDL_RemoveTimer(gctimer);
}

/*
 * Load an object from an alternate file.
 * World must be locked.
 */
int
object_loadfrom(void *p, char *path)
{
	struct object *ob = p;
	int fd;

	if (OBJECT_OPS(ob)->load == NULL) {
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		dprintf("%s: %s\n", path, strerror(errno));
		return (-1);
	}

	if (OBJECT_OPS(ob)->load(ob, fd) != 0) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}

/*
 * Load an object from its default location.
 * World must be locked.
 */
int
object_load(void *p)
{
	struct object *ob = p;
	char *path;
	int fd, rv;

	if (OBJECT_OPS(ob)->load == NULL) {
		return (0);
	}

	path = object_path(ob->name, ob->saveext);
	if (path == NULL) {
		dprintf("%s.%s: %s\n", ob->name, ob->saveext, AGAR_GetError());
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		dprintf("%s: %s\n", path, strerror(errno));
		return (-1);
	}
	rv = OBJECT_OPS(ob)->load(ob, fd);
	close(fd);
	return (rv);
}

/*
 * Save an object to its default location.
 * World must be locked.
 */
int
object_save(void *p)
{
	struct object *ob = p;
	char path[FILENAME_MAX];
	int fd;

	if (OBJECT_OPS(ob)->save == NULL) {
		return (0);
	}

	if (strcmp(ob->saveext, "m") == 0) {
		sprintf(path, "%s/maps/%s.m", world->udatadir, ob->name);
	} else {
		sprintf(path, "%s/%s.%s", world->udatadir, ob->name,
		    ob->saveext);
	}

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 00600);
	if (fd < 0) {
		fatal("%s: %s\n", path, strerror(errno));
		return (-1);
	}
	if (OBJECT_OPS(ob)->save(ob, fd) != 0) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}

/*
 * Search for an object matching the given string.
 * The world must be locked.
 * XXX hash
 */
struct object *
object_strfind(char *s)
{
	struct object *ob;

	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		if (strcmp(ob->name, s) == 0) {
			return (ob);
		}
	}
	return (NULL);
}

char *
object_path(char *obname, const char *suffix)
{
	struct stat sta;
	char *p, *last;
	char *datapath, *datapathp, *path;

	path = emalloc((size_t)FILENAME_MAX);
	datapathp = datapath = strdup(world->datapath);

	for (p = strtok_r(datapath, ":;", &last);
	     p != NULL;
	     p = strtok_r(NULL, ":;", &last)) {
	     	if (strcmp(suffix, ".m") == 0) {
			sprintf(path, "%s/maps/%s.m", p, obname);
		} else {
			sprintf(path, "%s/%s.%s", p, obname, suffix);
		}
		if (stat(path, &sta) == 0) {
			free(datapathp);
			return (path);
		}
	}
	free(datapathp);
	free(path);

	AGAR_SetError("cannot find data file");
	return (NULL);
}

void
object_dump(void *p)
{
	struct object *ob = p;

	printf("--\n%d. %s", ob->id, ob->name);
	if (ob->desc != NULL) {
		printf(" (%s)", ob->desc);
	}
	printf("\n[ ");
	if (OBJECT_OPS(ob)->destroy != NULL)
		printf("destroy ");
	if (OBJECT_OPS(ob)->load != NULL)
		printf("load ");
	if (OBJECT_OPS(ob)->save != NULL)
		printf("save ");
	printf("]\n");
}

/* XXX too intricate */

/*
 * Add a noderef at m:x,y, and a back reference to it.
 * Map must be locked, ob->pos must not.
 */
struct mappos *
object_addpos(void *p, Uint32 offs, Uint32 flags, struct input *in,
    struct map *m, Uint32 x, Uint32 y)
{
	struct object *ob = p;
	struct node *node;
	struct mappos *pos;

	node = &m->map[y][x];
	pos = emalloc(sizeof(struct mappos));
	pos->map = m;
	pos->x = x;
	pos->y = y;
	pos->speed = 1;

	/* Add a noderef at m:x,y. */
	pos->nref = node_addref(node, ob, offs, flags);
	if (pos->nref == NULL) {
		free(pos);
		return (NULL);
	}

	/* Display smooth transitions from one node to another. */
	node->flags |= NODE_ANIM;
	if (y > 1) {
		m->map[y - 1][x].overlap++;
	}
	mapdir_init(&pos->dir, ob, m, DIR_SCROLLVIEW|DIR_SOFTSCROLL, 3);

	/* Set the input device. */
	pos->input = in;
	if (in != NULL) {
		in->pos = pos;	/* XXX lock */
	}

	/* Link this back reference to its object. */
	pthread_mutex_lock(&ob->pos_lock);
	ob->pos = pos;
	pthread_mutex_unlock(&ob->pos_lock);
	
	return (pos);
}

/*
 * Remove the only back reference of this object.
 * Map must be locked.
 */
void
object_delpos(void *obp)	/* XXX will change */
{
	struct object *ob = obp;
	struct mappos *pos;
	
	pthread_mutex_lock(&ob->pos_lock);
	if (ob->pos != NULL) {
		pos = ob->pos;
		if (pos->map != NULL) {
			struct node *node;
	
			node = &pos->map->map[pos->y][pos->x];
			node_delref(node, pos->nref);
			node->flags &= ~(NODE_ANIM);
			if (pos->y > 1) {
				pos->map->map[pos->y - 1][pos->x].overlap--;
			}
		}
		free(pos);
		ob->pos = NULL;
	} else {
		dprintf("%s: no position\n", ob->name);
	}
	pthread_mutex_unlock(&ob->pos_lock);
}

/*
 * Move a noderef from its current node to m:x,y and update
 * the back reference. Return the new position.
 *
 * Map must be locked, ob->pos must not.
 */
struct mappos *
object_movepos(void *obp, struct map *m, int x, int y)
{
	struct mappos oldpos, *pos, *newpos;
	struct noderef *oldnref;
	struct object *ob = obp;

	/* Obtain the object's position. */
	pthread_mutex_lock(&ob->pos_lock);
	pos = ob->pos;
	if (pos == NULL) {
		dprintf("%s: no position\n", ob->name);
		pthread_mutex_unlock(&ob->pos_lock);
		return (NULL);
	}
	oldpos = *pos;
	pthread_mutex_unlock(&ob->pos_lock);
	
	/* Save and remove the old reference. */
	oldnref = oldpos.nref;
	object_delpos(oldnref->pobj);

	/* Insert at the new position. */
	newpos = object_addpos(oldnref->pobj, oldnref->offs, oldnref->flags,
	    oldpos.input, m, x, y);
	newpos->dir = oldpos.dir;

	m->redraw++;

	return (newpos);
}

