/*	$Csoft: object.c,v 1.47 2002/05/08 09:44:41 vedge Exp $	*/

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
	NANIMS_INIT =	1,
	NSPRITES_INIT =	1,
	NANIMS_GROW =	2,
	NSPRITES_GROW = 2,
};

static LIST_HEAD(, object_art) artsh =	LIST_HEAD_INITIALIZER(&artsh);
static LIST_HEAD(, object_audio) audiosh = LIST_HEAD_INITIALIZER(&audiosh);

static SDL_TimerID gctimer;

struct gc {
	struct	object *ob;
	LIST_ENTRY(gc) gcs;
};

static LIST_HEAD(, gc) gcsh = LIST_HEAD_INITIALIZER(&gcsh);
static pthread_mutex_t gc_lock = PTHREAD_MUTEX_INITIALIZER;

static const struct obvec null_obvec = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL,	/* link */
	NULL	/* unlink */
};

static struct object_art	*object_get_art(char *);
static struct object_audio	*object_get_audio(char *);
static void			 object_destroy(struct object *);

int
object_addanim(struct object_art *art, struct anim *anim)
{
	if (art->anims == NULL) {			/* Initialize */
		art->anims = (struct anim **)
		    emalloc(NANIMS_INIT * sizeof(struct anim *));
		art->maxanims = NANIMS_INIT;
		art->nanims = 0;
	} else if (art->nanims >= art->maxanims) {	/* Grow */
		struct anim **newanims;

		newanims = (struct anim **)erealloc(art->anims,
		    (NANIMS_GROW * art->maxanims) * sizeof(struct anim *));
		art->maxanims *= NANIMS_GROW;
		art->anims = newanims;
	}
	art->anims[art->nanims++] = anim;
	return (0);
}

int
object_addsprite(struct object_art *art, SDL_Surface *sprite)
{
	if (art->sprites == NULL) {			/* Initialize */
		art->sprites = (SDL_Surface **)
		    emalloc(NSPRITES_INIT * sizeof(SDL_Surface *));
		art->maxsprites = NSPRITES_INIT;
		art->nsprites = 0;
	} else if (art->nsprites >= art->maxsprites) {	/* Grow */
		SDL_Surface **newsprites;

		newsprites = (SDL_Surface **)erealloc(art->sprites,
		    (NSPRITES_GROW * art->maxsprites) * sizeof(SDL_Surface *));
		art->maxsprites *= NSPRITES_GROW;
		art->sprites = newsprites;
	}
	art->sprites[art->nsprites++] = sprite;
	return (0);
}

static struct object_art *
object_get_art(char *media)
{
	struct object_art *art = NULL, *eart;

	pthread_mutex_lock(&gc_lock);
	LIST_FOREACH(eart, &artsh, arts) {
		if (strcmp(eart->name, media) == 0) {
			art = eart;	/* Already in pool */
		}
	}

	if (art == NULL) {
		struct fobj *fob;
		Uint32 i;

		art = (struct object_art *)emalloc(sizeof(struct object_art));
		art->name = strdup(media);
		art->sprites = NULL;
		art->nsprites = 0;
		art->maxsprites = 0;
		art->anims = NULL;
		art->nanims = 0;
		art->maxanims = 0;
		art->used = 0;

		fob = fobj_load(savepath(media, "fob"));
		for (i = 0; i < fob->head.nobjs; i++) {	/* XXX broken */
			xcf_load(fob, i, art);
		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&artsh, art, arts);
	}
	pthread_mutex_unlock(&gc_lock);

	return (art);
}

static struct object_audio *
object_get_audio(char *media)
{
	struct object_audio *audio = NULL, *eaudio;

	pthread_mutex_lock(&gc_lock);

	LIST_FOREACH(eaudio, &audiosh, audios) {
		if (strcmp(eaudio->name, media) == 0) {
			audio = eaudio;	/* Already in pool */
		}
	}

	if (audio == NULL) {
		struct fobj *fob;
		Uint32 i;

		audio = (struct object_audio *)
		    emalloc(sizeof(struct object_audio));
		audio->name = strdup(media);
		/* XXX todo */
		audio->samples = NULL;
		audio->nsamples = 0;
		audio->maxsamples = 0;

		fob = fobj_load(savepath(media, "fob"));
		for (i = 0; i < fob->head.nobjs; i++) {	/* XXX todo */
			/* ... */
		}
		fobj_free(fob);
		
		LIST_INSERT_HEAD(&audiosh, audio, audios);
	}

	pthread_mutex_unlock(&gc_lock);
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
object_new(char *name, char *media, int flags, const void *vecp)
{
	struct object *ob;

	ob = emalloc(sizeof(struct object));
	object_init(ob, name, media, flags, vecp);

	pthread_mutex_lock(&world->lock);
	object_link(ob);
	pthread_mutex_unlock(&world->lock);

	return (ob);
}

void
object_init(struct object *ob, char *name, char *media, int flags,
    const void *vecp)
{
	static int curid = 0;	/* The world has id 0 */

	ob->id = curid++;
	ob->name = strdup(name);
	ob->desc = NULL;
	sprintf(ob->saveext, "ag");
	ob->flags = flags;
	ob->pos = NULL;
	ob->vec = (vecp != NULL) ? vecp : &null_obvec;
	pthread_mutex_init(&ob->pos_lock, NULL);

	if (ob->flags & OBJ_ART) {
		ob->art = object_get_art(media);
		ob->art->used++;
	} else {
		ob->art = NULL;
	}
	if (ob->flags & OBJ_AUDIO) {
		ob->audio = object_get_audio(media);
		ob->audio->used++;
	} else {
		ob->audio = NULL;
	}
}

static void
object_destroy(struct object *ob)
{
	if (ob->name != NULL) {
		free(ob->name);
	}
	if (ob->desc != NULL) {
		free(ob->desc);
	}
	pthread_mutex_destroy(&ob->pos_lock);
	free(ob);
}

/* Perform deferred garbage collection. */
Uint32
object_start_gc(Uint32 ival, void *p)
{
	struct gc *gc, *nextgc;
	struct object_audio *audio;
	struct object_art *art;

#if 1
	return (0);
#endif

	pthread_mutex_lock(&gc_lock);

	/* Object pool */
	for (gc = LIST_FIRST(&gcsh);
	     gc != LIST_END(&gcsh);
	     gc = nextgc) {
	     	struct object *ob = gc->ob;
	
		nextgc = LIST_NEXT(gc, gcs);
		LIST_REMOVE(gc, gcs);

		dprintf("free %s\n", ob->name);
		if (ob->vec->destroy != NULL) {
			ob->vec->destroy(ob);
		}
		object_destroy(ob);
		free(gc);
	}
	LIST_INIT(&gcsh);

	/* Art pool */
	LIST_FOREACH(art, &artsh, arts) {
		if (art->used < 1) {
			/* gc */
			dprintf("gc art\n");
		}
	}

	/* Audio pool */
	LIST_FOREACH(audio, &audiosh, audios) {
		if (audio->used < 1) {
			/* gc */
			dprintf("gc audio\n");
		}
	}

	pthread_mutex_unlock(&gc_lock);
	return (ival);
}

/* Initialize the garbage collector. */
void
object_init_gc(void)
{
	gctimer = SDL_AddTimer(1000, object_start_gc, NULL);
	dprintf("started garbage collection\n");
}

void
object_destroy_gc(void)
{
	SDL_RemoveTimer(gctimer);
	dprintf("stopped garbage collection\n");
}

/*
 * Load an object from an alternate file.
 * World must be locked.
 */
int
object_loadfrom(void *p, char *path)
{
	struct object *ob = (struct object *)p;
	int fd;

	if (ob->vec->load == NULL) {
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		warning("%s: %s\n", path, strerror(errno));
		return (-1);
	}

	if (ob->vec->load(ob, fd) != 0) {
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
	struct object *ob = (struct object *)p;
	char *path;
	int fd, rv;

	if (ob->vec->load == NULL) {
		return (0);
	}

	path = savepath(ob->name, ob->saveext);
	if (path == NULL) {
		return (-1);
	}

	/* XXX mmap? */
	fd = open(path, O_RDONLY, 00600);
	if (fd < 0) {
		dperror(ob->name);
		return (-1);
	}
	rv = ob->vec->load(ob, fd);
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
	struct object *ob = (struct object *)p;
	char path[FILENAME_MAX];
	int fd;

	if (ob->vec->save == NULL) {
		return (0);
	}

	sprintf(path, "%s/%s.%s", world->udatadir, ob->name, ob->saveext);
	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 00600);
	if (fd < 0) {
		dperror(path);
		return (-1);
	}
	if (ob->vec->save(ob, fd) != 0) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}

/*
 * Link an object to the world, from this point, static members of
 * the object structure cannot be modified.
 *
 * World must be locked.
 */
int
object_link(void *p)
{
	struct object *ob = (struct object *)p;
	
	if (ob->vec->link != NULL) {
		ob->vec->link(ob);
	}

	SLIST_INSERT_HEAD(&world->wobjsh, ob, wobjs);
	world->nobjs++;
	return (0);
}

/* Queue an unlinked object for garbage collection. */
void
object_queue_gc(struct object *ob)
{
	struct gc *ngc;

	if (ob->vec->unlink != NULL) {
		ob->vec->unlink(ob);
	}
	
	/* Decrement usage counts for the media pool. */
	if (ob->art != NULL)
		ob->art->used--;
	if (ob->audio != NULL)
		ob->audio->used--;

	/* Queue for garbage collection. */
	ngc = emalloc(sizeof(struct gc));
	ngc->ob = ob;
	pthread_mutex_lock(&gc_lock);
	LIST_INSERT_HEAD(&gcsh, ngc, gcs);
	pthread_mutex_unlock(&gc_lock);
}

/* 
 * Mark an object structure inconsistent.
 * World must be locked.
 */
int
object_unlink(void *p)
{
	struct object *ob = (struct object *)p;

#ifdef DEBUG
	if (object_strfind(ob->name) == NULL) {
		fatal("%s not linked\n", ob->name);
	}
#endif

	world->nobjs--;
	SLIST_REMOVE(&world->wobjsh, ob, object, wobjs);

	object_queue_gc(ob);

	return (0);
}

void
increase_uint32(Uint32 *variable, Uint32 val, Uint32 bounds)
{
	*variable += val;
	if (*variable > bounds) {
		*variable = bounds;
	}
}

void
decrease_uint32(Uint32 *variable, Uint32 val, Uint32 bounds)
{
	*variable -= val;
	if (*variable < bounds) {
		*variable = bounds;
	}
}

/*
 * Search for an object matching the given string.
 * XXX hash
 */
struct object *
object_strfind(char *s)
{
	struct object *ob;

	pthread_mutex_assert(&world->lock);
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		if (strcmp(ob->name, s) == 0) {
			return (ob);
		}
	}
	return (NULL);
}

void
object_dump(void *p)
{
	struct object *ob = (struct object *)p;

	printf("--\n%d. %s", ob->id, ob->name);
	if (ob->desc != NULL) {
		printf(" (%s)", ob->desc);
	}
	printf("\n[ ");
	if (ob->vec->destroy != NULL)
		printf("destroy ");
	if (ob->vec->load != NULL)
		printf("load ");
	if (ob->vec->save != NULL)
		printf("save ");
	if (ob->vec->link != NULL)
		printf("link ");
	if (ob->vec->unlink != NULL)
		printf("unlink ");
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
	struct object *ob = (struct object *)p;
	struct node *node;
	struct mappos *pos;

	node = &m->map[y][x];
	pos = (struct mappos *)emalloc(sizeof(struct mappos));
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
	struct object *ob = (struct object *)obp;
	struct mappos *pos;
	
	pthread_mutex_lock(&ob->pos_lock);
	if (ob->pos != NULL) {
		pos = ob->pos;
		if (pos->map != NULL) {
			struct node *node;
	
			node = &pos->map->map[pos->y][pos->x];
			node_delref(node, pos->nref);
			node->flags &= ~(NODE_ANIM);
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

