/*	$Csoft: object.c,v 1.15 2002/02/14 06:29:50 vedge Exp $	*/

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
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>

enum {
	NANIMS_INIT =	1,
	NSPRITES_INIT =	1,
	NANIMS_GROW =	2,
	NSPRITES_GROW = 2,
};

static SLIST_HEAD(lategc_head, gcref) lategch =
    SLIST_HEAD_INITIALIZER(&lategch);

struct gcref {
	struct	object *ob;
	int	order;

	SLIST_ENTRY(gcref) gcrefs;
};

int
object_addanim(struct object *ob, struct anim *anim)
{
	if (ob->anims == NULL) {			/* Initialize */
		ob->anims = (struct anim **)
		    emalloc(NANIMS_INIT * sizeof(struct anim *));
		ob->maxanims = NANIMS_INIT;
		ob->nanims = 0;
	} else if (ob->nanims >= ob->maxanims) {	/* Grow */
		struct anim **newanims;

		newanims = (struct anim **)realloc(ob->anims,
		    (NANIMS_GROW * ob->maxanims) * sizeof(struct anim *));
		if (newanims == NULL) {
			perror("realloc");
			return (-1);
		}
		ob->maxanims *= NANIMS_GROW;
		ob->anims = newanims;
	}
	ob->anims[ob->nanims++] = anim;
	return (0);
}

int
object_addsprite(struct object *ob, SDL_Surface *sprite)
{
	if (ob->sprites == NULL) {			/* Initialize */
		ob->sprites = (SDL_Surface **)
		    malloc(NSPRITES_INIT * sizeof(SDL_Surface *));
		if (ob->sprites == NULL) {
			perror("malloc");
			return (-1);
		}
		ob->maxsprites = NSPRITES_INIT;
		ob->nsprites = 0;
	} else if (ob->nsprites >= ob->maxsprites) {	/* Grow */
		SDL_Surface **newsprites;

		newsprites = (SDL_Surface **)realloc(ob->sprites,
		    (NSPRITES_GROW * ob->maxsprites) * sizeof(SDL_Surface *));
		if (newsprites == NULL) {
			perror("realloc");
			return (-1);
		}
		ob->maxsprites *= NSPRITES_GROW;
		ob->sprites = newsprites;
	}
	ob->sprites[ob->nsprites++] = sprite;
	return (0);
}

int
object_create(struct object *ob, char *name, char *desc, int flags)
{
	static int curid = 0;	/* The world has id 0 */

	ob->name = (name != NULL) ? strdup(name) : NULL;
	ob->desc = (desc != NULL) ? strdup(desc) : NULL;
	ob->id = curid++;
	ob->flags = flags;
	ob->sprites = NULL;
	ob->nsprites = 0;
	ob->maxsprites = 0;
	ob->anims = NULL;
	ob->nanims = 0;
	ob->maxanims = 0;
	ob->destroy_hook = NULL;

	return (0);
}

int
object_destroy(void *arg)
{
	struct object *ob = (struct object *)arg;
	int i;

	if (ob->flags & OBJ_DEFERGC) {
		struct gcref *or;
		
		or = emalloc(sizeof(struct gcref));
		or->ob = ob;
		SLIST_INSERT_HEAD(&lategch, or, gcrefs);
		dprintf("%s: deferred\n", ob->name);
		ob->flags &= ~(OBJ_DEFERGC);
		return (-1);
	}
	
	if (ob->flags & DESTROY_HOOK) {
		ob->destroy_hook(ob);
	}
	
	for(i = 0; i < ob->nsprites; i++)
		SDL_FreeSurface(ob->sprites[i]);
	for(i = 0; i < ob->nanims; i++)
		anim_destroy(ob->anims[i]);
	free(ob->sprites);
	free(ob->anims);
	
	dprintf("freed %s\n", ob->name);

	if (ob->name != NULL) {
		free(ob->name);
	}
	if (ob->desc != NULL) {
		free(ob->desc);
	}
	free(ob);

	return (0);
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

/* Add an object to the object list, and mark it consistent. */
int
object_link(void *ob)
{
	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_INSERT_HEAD(&world->wobjsh, (struct object *)ob, wobjs);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}

	return (0);
}

int
object_unlink(void *ob)
{
	if (pthread_mutex_lock(&world->lock) == 0) {
		SLIST_REMOVE(&world->wobjsh, ob, object, wobjs);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}

	return (0);
}

void
increase(int *variable, int val, int bounds)
{
	*variable += val;
	if (*variable > bounds) {
		*variable = bounds;
	}
}

void
decrease(int *variable, int val, int bounds)
{
	*variable -= val;
	if (*variable < bounds) {
		*variable = bounds;
	}
}

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

#ifdef DEBUG

void
object_dump(struct object *ob)
{
	printf("%3d. %10s (", ob->id, ob->name);
	if (ob->flags & DESTROY_HOOK)
		printf(" destroy");
	if (ob->flags & LOAD_FUNC)
		printf(" load");
	if (ob->flags & SAVE_FUNC)
		printf(" save");
	printf(" )\n");
	if (ob->desc != NULL) {
		printf("                ( %s )\n", ob->desc);
	}
}

#endif /* DEBUG */

