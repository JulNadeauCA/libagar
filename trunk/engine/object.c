/*	$Csoft: object.c,v 1.8 2002/02/05 06:05:50 vedge Exp $	*/

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

#include <glib.h>
#include <pthread.h>
#include <SDL.h>

#include <engine/engine.h>

static SLIST_HEAD(lategc_head, gcref) lategch =
    SLIST_HEAD_INITIALIZER(&lategch);

struct gcref {
	struct	object *ob;
	int	order;

	SLIST_ENTRY(gcref) gcrefs;
};

int
object_create(struct object *ob, char *name, char *desc, int flags)
{
	static int curid = 0;	/* The world has id 0 */

	ob->name = (name != NULL) ? strdup(name) : NULL;
	ob->desc = (desc != NULL) ? strdup(desc) : NULL;
	ob->id = curid++;
	ob->flags = flags;
	ob->wmask = 0;
	ob->sprites = NULL;
	ob->nsprites = 0;
	ob->anims = NULL;
	ob->nanims = 0;
	if (pthread_mutex_init(&ob->lock, NULL) != 0) {
		perror("object");
		return (-1);
	}
	ob->destroy_hook = NULL;

	return (0);
}

int
object_destroy(void *arg)
{
	struct object *ob = (struct object *)arg;
	int i;

	if (ob->flags & OBJ_DEFERGC) {
		struct gcref *or = malloc(sizeof(struct gcref));

		or->ob = ob;
		SLIST_INSERT_HEAD(&lategch, or, gcrefs);
		dprintf("%s: deferred\n", ob->name);
		ob->flags &= ~(OBJ_DEFERGC);
		return (-1);
	}
	
	if (ob->flags & DESTROY_HOOK) {
		ob->destroy_hook(ob);
	}
	
	if (pthread_mutex_lock(&ob->lock) == 0) {
		for(i = 0; i < ob->nsprites; i++) {
			SDL_FreeSurface(g_slist_nth_data(ob->sprites, i));
		}
		g_slist_free(ob->sprites);
		pthread_mutex_unlock(&ob->lock);
	} else {
		perror("object");
	}

	pthread_mutex_destroy(&ob->lock);
	
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
		dprintf("%s\n", gcr->ob->name);
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

int
object_wait(void *obp, int mask)
{
	struct object *ob = (struct object *)obp;
	int i = 300;

	while (i-- > 0) {
		if (ob->wmask & mask) {
			ob->wmask &= ~(mask);
			return (1);
		}
		SDL_Delay(1);
	}

	ob->wmask &= ~(mask);
	dprintf("%s: timeout waiting on mask 0x%x\n", ob->name, mask);

	return (0);
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
}

#endif /* DEBUG */

