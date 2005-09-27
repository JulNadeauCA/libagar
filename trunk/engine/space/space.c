/*	$Csoft: space.c,v 1.9 2005/09/20 13:46:33 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include <engine/engine.h>
#include <engine/actor.h>

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/menu.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "space.h"

const AG_Version agSpaceVer = {
	"agar space",
	1, 0
};

void
AG_SpaceInit(void *obj, const char *type, const char *name, const void *ops)
{
	AG_Space *sp = obj;

	AG_ObjectInit(sp, type, name, ops);
	pthread_mutex_init(&sp->lock, &agRecursiveMutexAttr);
	TAILQ_INIT(&sp->actors);
}

void
AG_SpaceReinit(void *obj)
{
	AG_Space *sp = obj;
	AG_Actor *ac;
	
	pthread_mutex_lock(&sp->lock);
	TAILQ_FOREACH(ac, &sp->actors, actors) {
		AG_SpaceDetach(sp, ac);
	}
	TAILQ_INIT(&sp->actors);
	pthread_mutex_unlock(&sp->lock);
}

void
AG_SpaceDestroy(void *obj)
{
	/* nothing yet */
}

int
AG_SpaceLoad(void *obj, AG_Netbuf *buf)
{
	AG_Space *sp = obj;
	Uint32 i, nactors, name;
	void *actor;

	if (AG_ReadVersion(buf, &agSpaceVer, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&sp->lock);
	nactors = AG_ReadUint32(buf);
	for (i = 0; i < nactors; i++) {
		name = AG_ReadUint32(buf);
		if (AG_ObjectFindDep(sp, name, &actor) == -1) {
			goto fail;
		}
		TAILQ_INSERT_TAIL(&sp->actors, AGACTOR(actor), actors);
	}
	pthread_mutex_unlock(&sp->lock);
	return (0);
fail:
	pthread_mutex_unlock(&sp->lock);
	return (-1);
}

int
AG_SpaceSave(void *obj, AG_Netbuf *buf)
{
	AG_Space *sp = obj;
	AG_Actor *actor;
	off_t nactors_offs;
	Uint32 nactors = 0;

	AG_WriteVersion(buf, &agSpaceVer);

	pthread_mutex_lock(&sp->lock);
	nactors_offs = AG_NetbufTell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(actor, &sp->actors, actors) {
		dprintf("actor: %s (%s)\n", AGOBJECT(actor)->name,
		    AGOBJECT(actor)->type);
		AG_WriteUint32(buf, AG_ObjectEncodeName(sp, actor));
		nactors++;
	}
	AG_PwriteUint32(buf, nactors, nactors_offs);
	pthread_mutex_unlock(&sp->lock);
	return (0);
}

/* Map an actor into a space. */
int
AG_SpaceAttach(void *sp_obj, void *obj)
{
	AG_Space *space = sp_obj;
	AG_Actor *ac = obj;

	pthread_mutex_lock(&ac->lock);
		
	AG_ObjectAddDep(space, ac);
	
	if (AGOBJECT_TYPE(space, "map")) {
		AG_Map *m = (AG_Map *)space;
		
		if (ac->g_map.x < 0 || ac->g_map.x >= m->mapw ||
		    ac->g_map.y < 0 || ac->g_map.y >= m->maph)  {
			AG_SetError(_("Illegal coordinates: %s:%d,%d"),
			    AGOBJECT(m)->name, ac->g_map.x, ac->g_map.y);
			goto fail;
		}

		ac->type = AG_ACTOR_MAP;
		ac->parent = space;
		ac->g_map.x0 = ac->g_map.x;
		ac->g_map.y0 = ac->g_map.y;
		ac->g_map.x1 = ac->g_map.x;
		ac->g_map.y1 = ac->g_map.y;
	
		if (AGACTOR_OPS(ac)->map != NULL)
			AGACTOR_OPS(ac)->map(ac, m);
	} else {
		ac->type = AG_ACTOR_NONE;
		ac->parent = NULL;
	}
	pthread_mutex_unlock(&ac->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ac->lock);
	return (-1);
}

void
AG_SpaceDetach(void *sp_obj, void *obj)
{
	AG_Space *space = sp_obj;
	AG_Actor *ac = obj;

	pthread_mutex_lock(&ac->lock);

	AG_ObjectCancelTimeouts(ac, 0);		/* XXX hook? */

	if (AGOBJECT_TYPE(space, "map")) {
		AG_ActorUnmapSprite(ac);
	}
	AG_ObjectDelDep(space, ac);
	ac->parent = NULL;
out:
	pthread_mutex_unlock(&ac->lock);
}

