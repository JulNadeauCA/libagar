/*	$Csoft: mapedit.c,v 1.111 2002/07/08 08:39:41 vedge Exp $	*/

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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/map.h>
#include <engine/physics.h>

#include <engine/widget/widget.h>
#include <engine/widget/text.h>
#include <engine/widget/window.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "mapedit_offs.h"
#include "command.h"
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

static void	 mapedit_attached(int, union evarg *);
static void	 mapedit_detached(int, union evarg *);
static void	 mapedit_shadow(struct mapedit *, void *);

void
mapedit_init(struct mapedit *med, char *name)
{
	struct region *coords_reg;

	object_init(&med->obj, "map-editor", name, "mapedit", OBJ_ART,
	    &mapedit_ops);
	med->flags = 0;
	TAILQ_INIT(&med->eobjsh);
	med->neobjs = 0;
	med->curobj = NULL;
	med->curoffs = 0;
	med->curflags = 0;

	/* XXX messy */
	mapedit_init_toolbar(med);
	med->coords_win = window_new("Coordinates", WINDOW_SOLID,
	    32, 32, 320, 64, 320, 64);
	coords_reg = region_new(med->coords_win, REGION_HALIGN|REGION_CENTER,
	    0,   0, 100, 100);
	med->coords_label = label_new(coords_reg, "...", 0);
	
	event_new(med, "attached", 0, mapedit_attached, NULL);
	event_new(med, "detached", 0, mapedit_detached, NULL);
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

	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		struct editobj *eob;
		
		if ((ob->flags & OBJ_ART) == 0 ||
		   (ob->art->nsprites < 1 && ob->art->nanims < 1)) {
			dprintf("skipping %s (no art)\n", ob->name);
			continue;
		}
		if (ob == OBJECT(med)) {
			continue;
		}

		/* Create a shadow structure for this object. */
		eob = emalloc(sizeof(struct editobj));
		eob->pobj = ob;
		SIMPLEQ_INIT(&eob->erefsh);
		eob->nrefs = 0;
		eob->nsprites = ob->art->nsprites;
		eob->nanims = ob->art->nanims;

		dprintf("%s: %d sprites, %d anims\n", ob->name,
		    eob->nsprites, eob->nanims);

		if (med->curobj == NULL && eob->nsprites > 0) {
			med->curobj = eob;
			med->curoffs = 0;
			med->curflags = 0;
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
				eref->type = EDITREF_SPRITE;
				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
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
				eref->type = EDITREF_ANIM;
				SIMPLEQ_INSERT_TAIL(&eob->erefsh, eref, erefs);
				eob->nrefs++;
			}
		}

		/* Insert into the list of shadow objects. */
		TAILQ_INSERT_HEAD(&med->eobjsh, eob, eobjs);
		med->neobjs++;
	}
}

static void
mapedit_attached(int argc, union evarg *argv)
{
	struct mapedit *med = argv[0].p;

	/* Create the shadow object structures. */
	mapedit_shadow(med, world);

	/* Set up the GUI. */
	window_show(med->toolbar_win);
	window_show(med->objlist_win);
	window_show(med->tileq_win);

	dprintf("editing %d object(s)\n", med->neobjs);
	
	curmapedit = med;	/* XXX obsolete */
}

static void
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
		for (eref = SIMPLEQ_FIRST(&eob->erefsh);
		     eref != SIMPLEQ_END(&eob->erefsh);
		     eref = nexteref) {
			nexteref = SIMPLEQ_NEXT(eref, erefs);
			free(eref);
		}
		free(eob);
	}
	
	curmapedit = NULL;	/* XXX unsafe */
}

