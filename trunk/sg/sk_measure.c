/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Measure tool.
 */

#include <config/edition.h>
#include <config/have_opengl.h>
#if defined(HAVE_OPENGL) && defined(EDITION)

#include <core/core.h>

#include "sk.h"
#include "sk_view.h"
#include "sg_gui.h"

struct sk_measure_tool {
	SK_Tool tool;
	SK_Node	*n1, *n2;
};

static void
ToolInit(void *p)
{
	struct sk_measure_tool *t = p;

	t->n1 = NULL;
	t->n2 = NULL;
}

static int
ToolMouseMotion(void *p, SG_Vector pos, SG_Vector vel, int btn)
{
	struct sk_measure_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SG_Vector vC;
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->flags & SK_NODE_MOUSEOVER) {
			node->flags &= ~(SK_NODE_MOUSEOVER);
			SK_NodeRedraw(node, skv);
		}
	}
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) != NULL &&
	    VecDistancep(&pos, &vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		SK_NodeRedraw(node, skv);
	} else {
		if ((node = SK_ProximitySearch(sk, NULL, &pos, &vC, NULL))
		    != NULL) {
			node->flags |= SK_NODE_MOUSEOVER;
			SK_NodeRedraw(node, skv);
		}
	}
	return (0);
}

static int
ToolMouseButtonDown(void *p, SG_Vector pos, int btn)
{
	struct sk_measure_tool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Node *node;
	SG_Vector vC;
	int ctrlMode = (SDL_GetModState() & KMOD_CTRL);
	
	if (btn != SDL_BUTTON_LEFT)
		return (0);
	
	if (!ctrlMode) {
		TAILQ_FOREACH(node, &sk->nodes, nodes) {
			node->flags &= ~(SK_NODE_SELECTED);
			SK_NodeRedraw(node, skv);
		}
	}
	
	/* Give point proximity more weight than other entities. */
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) == NULL ||
	    VecDistancep(&pos, &vC) >= skv->rSnap) {
		if ((node = SK_ProximitySearch(sk, NULL, &pos, &vC, NULL))
		    == NULL)
			return (0);
	}
	if (ctrlMode && node->flags & SK_NODE_SELECTED) {
		node->flags &= ~SK_NODE_SELECTED;
	} else {
		node->flags |=  SK_NODE_SELECTED;
	}
	SK_NodeRedraw(node, skv);
	return (0);
}

SK_ToolOps skMeasureToolOps = {
	N_("Measure tool"),
	N_("Measure distances and angles"),
	VGLINES_ICON,
	sizeof(struct sk_measure_tool),
	0,
	ToolInit,
	NULL,		/* destroy */
	NULL,		/* edit */
	ToolMouseMotion,
	ToolMouseButtonDown,
	NULL,		/* buttonup */
	NULL,		/* keydown */
	NULL		/* keyup */
};

#endif /* HAVE_OPENGL && EDITION */
