/*	$Csoft: object.c,v 1.40 2002/04/25 12:48:08 vedge Exp $	*/

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

static SLIST_HEAD(, object_art) artsh =	SLIST_HEAD_INITIALIZER(&artsh);
static pthread_mutex_t arts_lock = PTHREAD_MUTEX_INITIALIZER;

static SLIST_HEAD(, object_audio) audiosh = SLIST_HEAD_INITIALIZER(&audiosh);
static pthread_mutex_t audios_lock = PTHREAD_MUTEX_INITIALIZER;

static SDL_TimerID gctimer;

struct gcref {
	struct	object *ob;
	int	order;

	SLIST_ENTRY(gcref) gcrefs;
};

static SLIST_HEAD(lategc_head, gcref) lategch =
    SLIST_HEAD_INITIALIZER(&lategch);

static const struct obvec null_obvec = {
	NULL,	/* destroy */
	NULL,	/* load */
	NULL,	/* save */
	NULL,	/* link */
	NULL	/* unlink */
};

static Uint32			 object_mediapool_gc(Uint32 , void *);
static struct object_art	*object_get_art(char *);
static struct object_audio	*object_get_audio(char *);

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

		newanims = (struct anim **)realloc(art->anims,
		    (NANIMS_GROW * art->maxanims) * sizeof(struct anim *));
		if (newanims == NULL) {
			dperror("realloc");
			return (-1);
		}
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

		newsprites = (SDL_Surface **)realloc(art->sprites,
		    (NSPRITES_GROW * art->maxsprites) * sizeof(SDL_Surface *));
		if (newsprites == NULL) {
			dperror("realloc");
			return (-1);
		}
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

	pthread_mutex_lock(&arts_lock);

	SLIST_FOREACH(eart, &artsh, arts) {
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
		
		SLIST_INSERT_HEAD(&artsh, art, arts);
	}

	pthread_mutex_unlock(&arts_lock);
	return (art);
}

static struct object_audio *
object_get_audio(char *media)
{
	struct object_audio *audio = NULL, *eaudio;

	pthread_mutex_lock(&audios_lock);

	SLIST_FOREACH(eaudio, &audiosh, audios) {
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
		
		SLIST_INSERT_HEAD(&audiosh, audio, audios);
	}

	pthread_mutex_unlock(&audios_lock);
	return (audio);
}

void
object_mediapool_init(void)
{
	gctimer = SDL_AddTimer(1000, object_mediapool_gc, NULL);
}

void
object_mediapool_quit(void)
{
	SDL_RemoveTimer(gctimer);
}

static Uint32
object_mediapool_gc(Uint32 ival, void *p)
{
	struct object_audio *audio;
	struct object_art *art;
	
	pthread_mutex_lock(&arts_lock);
	SLIST_FOREACH(art, &artsh, arts) {
		if (art->used < 1) {
			/* gc */
			dprintf("gc art\n");
		}
	}
	pthread_mutex_unlock(&arts_lock);

	pthread_mutex_lock(&audios_lock);
	SLIST_FOREACH(audio, &audiosh, audios) {
		if (audio->used < 1) {
			/* gc */
			dprintf("gc audio\n");
		}
	}
	pthread_mutex_unlock(&audios_lock);

	return (ival);
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
	if (vecp != NULL) {
		ob->vec = vecp;
	} else {
		ob->vec = &null_obvec;
	}

	ob->pos = NULL;
	ob->art = NULL;
	ob->audio = NULL;

	if (ob->flags & OBJ_ART) {
		ob->art = object_get_art(media);
		ob->art->used++;
	}
	if (ob->flags & OBJ_AUDIO) {
		ob->audio = object_get_audio(media);
		ob->art->used++;
	}
}

void
object_destroy(void *arg)
{
	struct object *ob = (struct object *)arg;

	if (ob->flags & OBJ_DEFERGC) {
		struct gcref *or;
		
		or = emalloc(sizeof(struct gcref));
		or->ob = ob;
		SLIST_INSERT_HEAD(&lategch, or, gcrefs);
		ob->flags &= ~(OBJ_DEFERGC);
		return;
	}

	if (ob->vec->destroy != NULL) {
		ob->vec->destroy(ob);
	}

	/* Decrement usage counts for the media pool. */
	if (ob->art != NULL) {
		ob->art->used--;
	}
	if (ob->audio != NULL) {
		ob->art->audio--;
	}

	/* XXX gc */
#if 0
	for(i = 0; i < ob->art->nsprites; i++)
		SDL_FreeSurface(SPRITE(ob, i));
	if (ob->art->sprites != NULL)
		free(ob->art->sprites);
	for (i = 0; i < ob->nanims; i++)
		anim_destroy(ANIM(ob, i));
	if (ob->anims != NULL)
		free(ob->anims);
#endif
	
	if (ob->name != NULL)
		free(ob->name);
	if (ob->desc != NULL)
		free(ob->desc);
	free(ob);
}

/* Perform deferred garbage collection. */
void
object_lategc(void)
{
	struct gcref *gcr;

	SLIST_FOREACH(gcr, &lategch, gcrefs) {
		object_destroy(gcr->ob);
		free(gcr);
	}
}

/* Must be called on a locked world. */
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

/* Must be called on a locked world. */
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

/* Must be called on a locked world. */
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
 * Add an object to the object list, and mark it consistent.
 * The world's object list must be locked.
 */
int
object_link(void *p)
{
	struct object *ob = (struct object *)p;
	
	if (ob->vec->link != NULL &&
	    ob->vec->link(ob) != 0) {
		return (-1);
	}

	SLIST_INSERT_HEAD(&world->wobjsh, ob, wobjs);
	world->nobjs++;
	return (0);
}

/*
 * Unlink an object from the world and all maps.
 * The world's object list must be locked.
 */
int
object_unlink(void *p)
{
	struct object *ob = p;

	world->nobjs--;
	SLIST_REMOVE(&world->wobjsh, ob, object, wobjs);

	if (ob->vec->unlink != NULL &&
	    ob->vec->unlink(ob) != 0) {
		return (-1);
	}

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
 * The world's object list must be locked.
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

/* XXX move to map */

/*
 * Add a reference to ob:offs(flags) at m:x,y, and a back
 * reference (mappos) structure.
 *
 * Must be called on a locked map.
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
	pos->nref = node_addref(node, ob, offs, flags);
	if (pos->nref == NULL) {
		free(pos);
		return (NULL);
	}

	node->flags |= NODE_ANIM;	/* For soft-scrolling */

	/* XXX */
	mapdir_init(&pos->dir, ob, m, DIR_SCROLLVIEW|DIR_SOFTSCROLL, 3);
	pos->input = in;
	if (in != NULL) {
		in->pos = pos;
	}
	ob->pos = pos;
	
	return (pos);
}

/*
 * Destroy the mappos structure associated with an object.
 * Must be called on a locked map.
 */
void
object_delpos(void *obp)
{
	struct object *ob = (struct object *)obp;
	struct mappos *pos = ob->pos;

	if (pos == NULL) {
		dprintf("%s has no position\n", ob->name);
		return;
	}

	if (pos->map != NULL) {
		struct node *node = &pos->map->map[pos->y][pos->x];

		node_delref(node, pos->nref);
		node->flags &= ~(NODE_ANIM);
	}

	free(pos);
	ob->pos = NULL;
}

