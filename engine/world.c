/*	$Csoft: world.c,v 1.19 2002/03/17 09:15:00 vedge Exp $	*/

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/mapedit/mapedit.h>

static struct obvec world_vec = {
	world_destroy,
	world_load,
	world_save
};

char *
savepath(char *obname, const char *suffix)
{
	static char path[FILENAME_MAX];
	static struct stat sta;
	char *p, *last;
	char *datapath, *datapathp;
	
	datapathp = datapath = strdup(world->datapath);

	for (p = strtok_r(datapath, ":;", &last);
	     p != NULL;
	     p = strtok_r(NULL, ":;", &last)) {
		sprintf(path, "%s/%s.%s", p, obname, suffix);
		if (stat(path, &sta) == 0) {
			free(datapathp);
			return (path);
		}
	}

	fatal("%s.%s not in %s\n", obname, suffix, world->datapath);
	free(datapathp);
	return (NULL);
}

struct world *
world_create(char *name)
{
	struct passwd *pwd;
	struct stat sta;
	char *pathenv;
	
	pwd = getpwuid(getuid());

	world = (struct world *)emalloc(sizeof(struct world));
	object_init(&world->obj, name, OBJ_DEFERGC, &world_vec);

	world->udatadir = (char *)
	    emalloc(strlen(pwd->pw_dir) + strlen(name) + 4);
	world->sysdatadir = (char *)
	    emalloc(strlen(SHAREDIR) + strlen(name) + 4);

	sprintf(world->udatadir, "%s/.%s", pwd->pw_dir, name);
	sprintf(world->sysdatadir, SHAREDIR);

	pathenv = getenv("AGAR_STATE_PATH");
	if (pathenv != NULL) {
		world->datapath = strdup(pathenv);
	} else {
		world->datapath = (char *)
		    emalloc(strlen(world->udatadir) +
		    strlen(world->sysdatadir) + 4);
		sprintf(world->datapath, "%s:%s", world->udatadir,
		    world->sysdatadir);
	}

	if (stat(world->sysdatadir, &sta) != 0) {
		warning("%s: %s\n", world->sysdatadir, strerror(errno));
	}
	if (stat(world->udatadir, &sta) != 0 &&
	    mkdir(world->udatadir, 00700) != 0) {
		fatal("%s: %s\n", world->udatadir, strerror(errno));
	}

	world->curmap = NULL;

	SLIST_INIT(&world->wobjsh);
	SLIST_INIT(&world->wcharsh);
	if (pthread_mutex_init(&world->lock, NULL) != 0) {
		dperror("world");
		return (NULL);
	}
	
	return (world);
}

int
world_load(void *p, int fd)
{
	struct world *wo = (struct world *)p;
	struct object *ob;

	/* XXX load the state map */

	dprintf("loading state\n");
	pthread_mutex_lock(&wo->lock);
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		if (curmapedit != NULL && !strcmp(ob->saveext, "m")) {
			/* XXX map editor hack */
			continue;
		}
		object_load(ob);
	}
	pthread_mutex_unlock(&wo->lock);
	return (0);
}

int
world_save(void *p, int fd)
{
	struct world *wo = (struct world *)p;
	struct object *ob;
	size_t soffs;
	Uint32 nobjs = 0;

	dprintf("saving state\n");

	/* Write the state map. */
	soffs = lseek(fd, 0, SEEK_SET);
	fobj_write_uint32(fd, 0);
	
	pthread_mutex_lock(&wo->lock);
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		if (curmapedit != NULL && !strcmp(ob->saveext, "m")) {
			/* XXX map editor hack */
			continue;
		}
		fobj_write_uint32(fd, ob->id);
		fobj_write_string(fd, ob->name);
		fobj_write_string(fd, (ob->desc != NULL) ? ob->desc : "");
		object_save(ob);
		nobjs++;
	}
	pthread_mutex_unlock(&wo->lock);
	fobj_pwrite_uint32(fd, nobjs, soffs);
	dprintf("%s: %d objects\n", wo->obj.name, nobjs);
	return (0);
}

int
world_destroy(void *p)
{
	struct world *wo = (struct world *)p;
	struct object *ob;

	if (world->curmap != NULL) {
		map_unfocus(world->curmap);
	}

	pthread_mutex_lock(&wo->lock);

	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		if (ob->vec->unlink != NULL) {
			ob->vec->unlink(ob);
		}
	}

	printf("freed:");
	fflush(stdout);
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		printf(" %s", ob->name);
		fflush(stdout);
		object_destroy(ob);
	}
	object_lategc();
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		SLIST_REMOVE(&world->wobjsh, ob, object, wobjs);
	}
	printf(".\n");
	
	pthread_mutex_unlock(&wo->lock);

	free(wo->datapath);
	free(wo->udatadir);
	free(wo->sysdatadir);
	pthread_mutex_destroy(&wo->lock);

	return (0);
}

