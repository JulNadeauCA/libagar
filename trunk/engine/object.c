/*	$Csoft$	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <pthread.h>
#include <SDL.h>

#include <engine/debug.h>
#include <engine/view.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/map.h>

static GSList	*lategc;

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
	ob->map = NULL;
	ob->mapx = 1;
	ob->mapy = 1;
	ob->time_hook = NULL;
	ob->event_hook = NULL;
	ob->destroy_hook = NULL;

	return (0);
}

void
object_destroy(void *arg, void *p)
{
	struct object *ob = (struct object *)arg;
	int i;

	if (ob->flags & OBJ_USED) {
		lategc = g_slist_append(lategc, arg);
		dprintf("%s: deferred\n", ob->name);
		ob->flags &= ~(OBJ_USED);
		return;
	}

#ifdef DEBUG
	/* This should not happen. */
	if (arg == NULL) {
		dprintf("NULL arg\n");
		return;
	}
#endif

	/*
	 * Remove this object from the world while it is still in a
	 * consistent state, unless it is the world.
	 */
	if ((void *)arg != (void *)world) {
		if (pthread_mutex_lock(&world->lock) == 0) {
			world->objs = g_slist_remove(world->objs, (void *)ob);
			world->nobjs--;
			pthread_mutex_unlock(&world->lock);
		} else {
			perror("world");
			return;
		}
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

	if (ob->flags & DESTROY_HOOK) {
		ob->destroy_hook(ob);
	}
	
	dprintf("freed %s\n", ob->name);

	/* Free what's left. */
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
	g_slist_foreach(lategc, (GFunc)object_destroy, NULL);
}

/* Add an object to the object list, and mark it consistent. */
int
object_link(void *objp)
{
	if (pthread_mutex_lock(&world->lock) == 0) {
		world->objs = g_slist_append(world->objs, objp);
		pthread_mutex_unlock(&world->lock);
	} else {
		perror("world");
		return (-1);
	}
	
	world->nobjs++;

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

static gint
object_strcompare(void *arg, void *p)
{
	struct object *ob = (struct object *)arg;
	char *id = (char *)p;

	if (strcmp(ob->name, id) == 0) {
		return (0);
	}
	return (-1);
}

struct object *
object_strfind(char *s)
{
	GSList *li;

	li = g_slist_find_custom(world->objs, s,
	    (GCompareFunc)object_strcompare);

	return ((li != NULL) ? li->data : NULL);
}

int
object_wait(struct object *ob, int mask)
{

	int i = 30;

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
object_dump_obj(void *obp, void *p)
{
	struct object *ob = (struct object *)obp;

	printf("%3d. %10s (", ob->id, ob->name);
	if (ob->flags & EVENT_HOOK)
		printf(" event");
	if (ob->flags & DESTROY_HOOK)
		printf(" destroy");
	if (ob->flags & LOAD_FUNC)
		printf(" load");
	if (ob->flags & SAVE_FUNC)
		printf(" save");
	printf(" )");

	if (ob->map != NULL) {
		printf(" on %s:%dx%d", ob->map->obj.name, ob->mapx, ob->mapy);
	}
	printf("\n");
}

#endif /* DEBUG */

