/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Generic selection tool for sketch editor.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

typedef struct sk_select_tool {
	SK_Tool tool;
	Uint flags;
#define SELECT_POINTS	0x01
#define SELECT_LINES	0x02
#define SELECT_ARCS	0x04
	int moving;
} SK_SelectTool;

static void
ToolInit(void *_Nonnull p)
{
	SK_SelectTool *t = p;

	t->flags = SELECT_POINTS|SELECT_LINES|SELECT_ARCS;
	t->moving = 0;
}

static int
ToolMouseMotion(void *_Nonnull p, M_Vector3 pos, M_Vector3 vel, int btn)
{
	SK_SelectTool *t = p;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	M_Vector3 vC;
	SK_Node *node, *oNode;
	int update = 0;
	Uint i;

	if (!t->moving)
		return (0);

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->flags & SK_NODE_MOUSEOVER) {
			node->flags &= ~(SK_NODE_MOUSEOVER);
			SK_NodeRedraw(node, skv);
		}
		node->flags &= ~(SK_NODE_MOVED);
	}
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) != NULL &&
	    M_VecDistance3p(&pos, &vC) < skv->rSnap) {
		node->flags |= SK_NODE_MOUSEOVER;
		SK_NodeRedraw(node, skv);
	} else {
		if ((node = SK_ProximitySearch(sk, NULL, &pos, &vC, NULL))
		    != NULL) {
			node->flags |= SK_NODE_MOUSEOVER;
			SK_NodeRedraw(node, skv);
		}
	}

	/* Move selected elements. */
	if (btn & AG_MOUSE_LEFT) {
		TAILQ_FOREACH(node, &sk->nodes, nodes) {
			if (node->flags & (SK_NODE_MOVED|SK_NODE_FIXED|
			                   SK_NODE_KNOWN)) {
				continue;
			}
			if (node->flags & SK_NODE_SELECTED &&
			    node->ops->move != NULL) {
				if (node->ops->move(node, &pos, &vel) == 1) {
					update = 1;
				}
				node->flags |= SK_NODE_MOVED;
				TAILQ_FOREACH(oNode, &sk->nodes, nodes) {
					for (i = 0; i < oNode->nRefNodes; i++) {
						if (oNode->refNodes[i] != node)
							continue;
						oNode->flags |= SK_NODE_MOVED;
					}
				}
			}
		}
	}
	if (update) {
		SK_Update(sk);
	}
	return (0);
}

static int
ToolMouseButtonDown(void *_Nonnull pTool, M_Vector3 pos, int btn)
{
	SK_SelectTool *t = pTool;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Node *node;
	M_Vector3 vC;
	int ctrlMode;
	
	if (btn != AG_MOUSE_LEFT) {
		return (0);
	}
	t->moving = 1;

	if (!(ctrlMode = (AG_GetModState(skv) & AG_KEYMOD_CTRL))) {
		TAILQ_FOREACH(node, &sk->nodes, nodes) {
			node->flags &= ~(SK_NODE_SELECTED);
			SK_NodeRedraw(node, skv);
		}
	}
	
	/* Give point proximity more weight than other entities. */
	if ((node = SK_ProximitySearch(sk, "Point", &pos, &vC, NULL)) == NULL ||
	    M_VecDistance3p(&pos, &vC) >= skv->rSnap) {
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
	return (1);
}

static int
ToolMouseButtonUp(void *_Nonnull pTool, M_Vector3 pos, int btn)
{
	SK_SelectTool *t = pTool;
	
	if (btn == AG_MOUSE_LEFT) {
		t->moving = 0;
	}
	return (0);
}

static void
ToolEdit(void *_Nonnull p, void *_Nonnull box)
{
	SK_SelectTool *t = p;
	static const AG_FlagDescr flagDescr[] = {
	    { SELECT_POINTS,	N_("Select Points"),	1 },
	    { SELECT_LINES,	N_("Select Lines"),	1 },
	    { SELECT_ARCS,	N_("Select Arcs"),	1 },
	    { 0,		NULL,			0 }
	};
	AG_CheckboxSetFromFlags(box, 0, &t->flags, flagDescr);
}

SK_ToolOps skSelectToolOps = {
	N_("Select"),
	N_("Select and move sketch items"),
	NULL,
	sizeof(SK_SelectTool),
	0,
	ToolInit,
	NULL,			/* destroy */
	ToolEdit,
	ToolMouseMotion,
	ToolMouseButtonDown,
	ToolMouseButtonUp,
	NULL,			/* keydown */
	NULL			/* keyup */
};
