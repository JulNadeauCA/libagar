/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <core/core.h>
#include "sg.h"

const AG_ObjectOps sgOps = {
	SG_Init,
	SG_Reinit,
	SG_Destroy,
	SG_Load,
	SG_Save
};

SG *
SG_New(void *parent, const char *name)
{
	SG *sg;

	sg = Malloc(sizeof(SG), M_SG);
	SG_Init(sg, name);
	AG_ObjectAttach(parent, sg);
	return (sg);
}

void
SG_Init(void *obj, const char *name)
{
	SG *sg = obj;

	AG_ObjectInit(sg, "sg", name, &sgOps);
	sg->flags = 0;
	sg->root = NULL;
	AG_MutexInit(&sg->lock);
}

void
SG_Reinit(void *obj)
{
	SG *sg = obj;
}

void
SG_Destroy(void *obj)
{
	SG *sg = obj;
}

int
SG_Save(void *obj, AG_Netbuf *buf)
{
	SG *sg = obj;

	return (0);
}

int
SG_Load(void *obj, AG_Netbuf *buf)
{
	SG *sg = obj;
	
	return (0);
}

SG_Node *
SG_AddNode(SG_Node *pNode, const SG_NodeOps *ops, Uint flags)
{
	SG_Node *n;

	n = Malloc(sizeof(SG_Node), M_SG);
	n->ops = ops;
	n->flags = flags;
	SG_LoadIdentity4(&n->mTrans);
	TAILQ_INIT(&n->cnodes);
	TAILQ_INSERT_TAIL(&pNode->cnodes, n, nodes);
	return (n);
}

#ifdef EDITION
void *
SG_Edit(void *obj)
{
	SG *sg = obj;
	AG_Window *win;
	
	win = AG_WindowNew(AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, _("Scene Graph \"%s\""), AGOBJECT(sg)->name);

	return (win);
}
#endif /* EDITION */
