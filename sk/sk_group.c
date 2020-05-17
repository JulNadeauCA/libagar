/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * This node class defines a group of entities.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Group *
SK_GroupNew(void *pnode)
{
	SK_Group *g;

	g = Malloc(sizeof(SK_Group));
	SK_GroupInit(g, SK_GenNodeName(SKNODE(pnode)->sk, "Group"));
	SK_NodeAttach(pnode, g);
	return (g);
}

void
SK_GroupInit(void *p, Uint name)
{
	SK_Group *g = p;

	SK_NodeInit(g, &skGroupOps, name, 0);
	g->nodes = NULL;
	g->nNodes = 0;
	g->color = M_ColorRGB(0.0, 0.0, 0.0);
}

int
SK_GroupLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Group *g = p;
//	Uint32 i;

	g->color = M_ReadColor(buf);
#if 0
	g->nNodes = (Uint)AG_ReadUint32(buf);
	g->nodes = Realloc(g->nodes, g->nNodes*sizeof(SK_Node *));
	for (i = 0; i < nNodes; i++) {
		g->nodes[i] = SK_ReadRef(buf, sk, NULL);
		if (g->nodes[i] == NULL) {
			AG_SetError("Cannot find group item: %s",
			    AG_GetError());
			return (-1);
		}
	}
#endif
	return (0);
}

int
SK_GroupSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Group *g = p;
//	Uint i;

	M_WriteColor(buf, &g->color);
	AG_WriteUint32(buf, (Uint32)g->nNodes);
#if 0
	for (i = 0; i < g->nNodes; i++)
		SK_WriteRef(buf, g->nodes[i]);
#endif
	return (0);
}

void
SK_GroupEdit(void *p, AG_Widget *box, SK_View *skv)
{
	SK_Group *g = p;
	AG_HSVPal *pal;

	pal = AG_HSVPalNew(box, AG_HSVPAL_EXPAND);
	M_BindReal(pal, "RGBAv", (void *)&g->color);
}

void
SK_GroupColor(SK_Group *line, M_Color c)
{
	line->color = c;
}

SK_NodeOps skGroupOps = {
	"Group",
	sizeof(SK_Group),
	0,
	SK_GroupInit,
	NULL,			/* destroy */
	SK_GroupLoad,
	SK_GroupSave,
	NULL,			/* draw */
	NULL,			/* redraw */
	SK_GroupEdit,
	NULL,			/* proximity */
	NULL,			/* delete */
	NULL,			/* move */
	NULL			/* constrained */
};
