/*	$Csoft: eraser.c,v 1.43 2004/01/03 04:25:10 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/mapedit/mapedit.h>

static void eraser_effect(void *, struct mapview *, struct map *,
                          struct node *);

const struct tool eraser_tool = {
	N_("Eraser"),
	N_("Remove the highest node reference."),
	MAPEDIT_TOOL_ERASER,
	MAPEDIT_ERASER_CURSOR,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	eraser_effect,
	NULL,			/* cursor */
	NULL			/* mouse */
};

static void
eraser_effect(void *p, struct mapview *mv, struct map *m, struct node *dn)
{
	struct noderef *nref;
	
	TAILQ_FOREACH(nref, &dn->nrefs, nrefs) {
		if (nref->layer != m->cur_layer)
			continue;

		TAILQ_REMOVE(&dn->nrefs, nref, nrefs);
		noderef_destroy(m, nref);
		break;
	}
}

