/*	$Csoft: mapedit.c,v 1.128 2002/12/31 01:46:35 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/version.h>
#include <engine/map.h>
#include <engine/world.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "toolbar.h"
#include "fileops.h"

static const struct version mapedit_ver = {
	"agar map editor",
	1, 0
};

static const struct object_ops mapedit_ops = {
	NULL,
	NULL,	/* load */
	NULL	/* save */
};

struct mapedit *curmapedit = NULL;	/* Map editor */

static void	 mapedit_shadow(struct mapedit *, void *);

void
mapedit_init(struct mapedit *med, char *name)
{
	object_init(&med->obj, "map-editor", name, "mapedit",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &mapedit_ops);
	prop_set_int(med, "zoom-minimum", 4);
	prop_set_int(med, "zoom-maximum", 400);
	prop_set_int(med, "zoom-increment", 2);
	prop_set_int(med, "zoom-speed", 60);
	prop_set_bool(med, "tilemap-scroll-x", 0);
	prop_set_bool(med, "tilemap-scroll-y", 1);
	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;
	med->ref.obj = NULL;
	med->ref.offs = 0;
	med->ref.flags = 0;
	med->ref.type = 0;
	med->node.flags = NODE_WALK;
	med->curtool = NULL;

	mapedit_init_toolbar(med);
	
	event_new(med, "attached", mapedit_attached, NULL);
	event_new(med, "detached", mapedit_detached, NULL);
}

/*
 * Construct a list of shadow objects and references.
 * Map editor and world must be locked.
 */
static void
mapedit_shadow(struct mapedit *med, void *parent)
{
	struct world *wo = parent;
	struct object *ob;

	SLIST_FOREACH(ob, &wo->wobjs, wobjs) {
		struct editobj *eob;
		
		if ((ob->flags & OBJECT_ART) == 0 ||
		   (ob->art->nsprites < 1 && ob->art->nanims < 1)) {
			continue;
		}
		if (ob == OBJECT(med)) {
			continue;
		}

		/* Create a shadow structure for this object. */
		eob = emalloc(sizeof(struct editobj));
		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefs);
		eob->nrefs = 0;
		eob->nsprites = ob->art->nsprites;
		eob->nanims = ob->art->nanims;

		dprintf("%s: %d sprites, %d anims\n", ob->name,
		    eob->nsprites, eob->nanims);

		if (med->curobj == NULL && eob->nsprites > 0) {
			med->curobj = eob;
		}

		/* Create shadow references for sprites. */
		if (eob->nsprites > 0) {
			int y;

			for (y = 0; y < eob->nsprites; y++) {
				struct editref *eref;
		
				eref = emalloc(sizeof(struct editref));
				eref->animi = -1;
				eref->spritei = y;
				eref->p = SPRITE(ob, y);
				eref->type = NODEREF_SPRITE;
				SIMPLEQ_INSERT_TAIL(&eob->erefs, eref, erefs);
				eob->nrefs++;
			}
		}
	
		/* Create shadow references for animations. */
		if (eob->nanims > 0) {
			int z;

			for (z = 0; z < eob->nanims; z++) {
				struct editref *eref;

				eref = emalloc(sizeof(struct editref));
				eref->animi = z;
				eref->spritei = -1;
				eref->p = ANIM(ob, z);
				eref->type = NODEREF_ANIM;
				SIMPLEQ_INSERT_TAIL(&eob->erefs, eref, erefs);
				eob->nrefs++;
			}
		}

		/* Insert into the list of shadow objects. */
		TAILQ_INSERT_HEAD(&med->eobjsh, eob, eobjs);
		med->neobjs++;
	}
}

void
mapedit_attached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;

	/* Create the shadow object structures. */
	mapedit_shadow(med, world);

	/* Set up the GUI. */
	window_show(med->toolbar_win);
	window_show(med->objlist_win);

	dprintf("editing %d object(s)\n", med->neobjs);
	
	curmapedit = med;	/* XXX obsolete */
}

void
mapedit_detached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;
	struct editobj *eob, *nexteob;

	/* Deallocate the shadow structures. */
	for (eob = TAILQ_FIRST(&med->eobjsh);
	     eob != TAILQ_END(&med->eobjsh);
	     eob = nexteob) {
		struct editref *eref, *nexteref;

		nexteob = TAILQ_NEXT(eob, eobjs);
		for (eref = SIMPLEQ_FIRST(&eob->erefs);
		     eref != SIMPLEQ_END(&eob->erefs);
		     eref = nexteref) {
			nexteref = SIMPLEQ_NEXT(eref, erefs);
			free(eref);
		}
		free(eob);
	}
	
	curmapedit = NULL;	/* XXX unsafe */
}

