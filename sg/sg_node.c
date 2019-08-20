/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Base class for any node in a SG(3) scene graph.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>

#include <string.h>

static void
OnAttach(AG_Event *_Nonnull event)
{
	SG_Node *node = SG_NODE_SELF();
	AG_Object *parent = AG_OBJECT_PTR(1);
	SG *sg = NULL;
	
	if (AG_OfClass(parent, "SG:*")) {
		sg = (SG *)parent;
	} else if (AG_OfClass(parent, "SG_Node:*")) {
		sg = SGNODE(parent)->sg;
	} else {
		AG_FatalError("Invalid SG_Node parent");
	}
	AG_ObjectLock(sg);
	TAILQ_INSERT_TAIL(&sg->nodes, node, nodes);
	node->sg = sg;
	AG_ObjectUnlock(sg);
}

static void
OnDetach(AG_Event *_Nonnull event)
{
	SG_Node *node = SG_NODE_SELF();
	void *parent = AG_OBJECT_PTR(1);
	SG *sg = NULL;
	
	if (AG_OfClass(parent, "SG:*")) {
		sg = (SG *)parent;
	} else if (AG_OfClass(parent, "SG_Node:*")) {
		sg = SGNODE(parent)->sg;
	} else {
		AG_FatalError("Invalid SG_Node parent");
	}
	AG_ObjectLock(sg);
	TAILQ_REMOVE(&sg->nodes, node, nodes);
	node->sg = NULL;
	AG_ObjectUnlock(sg);
}

static void
Init(void *_Nonnull obj)
{
	SG_Node *node = obj;

	node->flags = 0;
	M_MatIdentity44v(&node->T);
	TAILQ_INIT(&node->actions);

	AG_SetEvent(node, "attached", OnAttach, NULL);
	AG_SetEvent(node, "detached", OnDetach, NULL);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Node *node = obj;
	SG_Action *act, *actNext;

	for (act = TAILQ_FIRST(&node->actions);
	     act != TAILQ_END(&node->actions);
	     act = actNext) {
		actNext = TAILQ_NEXT(act, actions);
		Free(act);
	}
	TAILQ_INIT(&node->actions);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	SG_Node *node = obj;

	node->flags = (Uint)AG_ReadUint32(buf);
	M_ReadMatrix44v(buf, &node->T);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SG_Node *node = obj;

	AG_WriteUint32(buf, (Uint32)node->flags);
	M_WriteMatrix44(buf, &node->T);
	return (0);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Node *node = obj;
	AG_Box *box;

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);
	AG_LabelNew(box, 0, _("Node Name: %s"), AGOBJECT(node)->name);
	return (box);
}

/* Generic edit() operation for node objects. */
void *_Nonnull
SG_NodeEdit(void *obj)
{
	SG_Node *node = obj;
	AG_Window *win;
	void *wEdit;
	AG_ObjectClass **hier;
	int i, nHier;

	if ((win = AG_WindowNew(0)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(node)->name);
	
	if (AG_ObjectGetInheritHier(node, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = 0; i < nHier; i++) {
		SG_NodeClass *nc = (SG_NodeClass *)hier[i];

		if (!AG_ClassIsNamed(hier[i], "SG_Node:*") ||
		    nc->edit == NULL)
			continue;

		wEdit = nc->edit(node, NULL);
		if (wEdit != NULL && AG_OfClass(wEdit, "AG_Widget:*"))
			AG_ObjectAttach(win, wEdit);
	}
	Free(hier);
	return (win);
}

/*
 * Compute the product of the transform matrices of the given node
 * and its parents.
 */
void
SG_GetNodeTransform(void *obj, M_Matrix44 *T)
{
	SG_Node *node = obj;
#ifdef AG_THREADS
	SG *sg = node->sg;
#endif
	SG_Node *chld = node;
	TAILQ_HEAD_(sg_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

	M_MatIdentity44v(T);

	AG_ObjectLock(sg);
	while (chld != NULL) {
		TAILQ_INSERT_TAIL(&rnodes, chld, rnodes);
		if (OBJECT(chld)->parent == NULL ||
		    !AG_OfClass(OBJECT(chld)->parent, "SG_Node:*")) {
			break;
		}
		chld = OBJECT(chld)->parent;
	}
	TAILQ_FOREACH(chld, &rnodes, rnodes) {
		M_MatMult44v(T, &chld->T);
	}
	AG_ObjectUnlock(sg);
}

/*
 * Compute the product of the inverse transform matrices of the given node
 * and its parents.
 */
void
SG_GetNodeTransformInverse(void *obj, M_Matrix44 *T)
{
	SG_Node *node = obj;
	SG_Node *chld = node;
#ifdef AG_THREADS
	SG *sg = node->sg;
#endif
	TAILQ_HEAD_(sg_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

	M_MatIdentity44v(T);

	AG_ObjectLock(sg);
	while (chld != NULL) {
		TAILQ_INSERT_TAIL(&rnodes, chld, rnodes);
		if (OBJECT(chld)->parent == NULL ||
		    !AG_OfClass(OBJECT(chld)->parent, "SG_Node:*")) {
			break;
		}
		chld = OBJECT(chld)->parent;
	}
	TAILQ_FOREACH(chld, &rnodes, rnodes) {
		M_Matrix44 Tinv;

		Tinv = M_MatInvert44(chld->T);
		M_MatMult44v(T, &Tinv);
	}
	AG_ObjectUnlock(sg);
}

/*
 * Graphically render a node and its children.
 * The SG must be locked.
 */
void
SG_NodeDraw(SG *sg, SG_Node *node, SG_View *view)
{
	AG_ObjectClass **hier;
	int i, nHier;
	M_Matrix44 Tsave, T;
	SG_Node *chld;

	GL_FetchMatrixv(GL_MODELVIEW_MATRIX, &Tsave);
	T = M_MatTranspose44p(&node->T);	/* OpenGL is column-major */
	GL_MultMatrixv(&T);

	AG_ObjectLock(node);

	/* Render this node. */
	if (AG_ObjectGetInheritHier(node, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = nHier-1; i >= 0; i--) {
		SG_NodeClass *nc = (SG_NodeClass *)hier[i];

		if (!AG_ClassIsNamed(hier[i], "SG_Node:*") ||
		    nc->draw == NULL) {
			continue;
		}
		nc->draw(node, view);
		break;
	}

	/* Render child nodes. */
	OBJECT_FOREACH_CLASS(chld, node, sg_node, "SG_Node:*") {
		SG_NodeDraw(sg, chld, view);
	}
	AG_ObjectUnlock(node);

	GL_LoadMatrixv(&Tsave);
	Free(hier);
}

/* Save node data (and child nodes) to a data source. */
int
SG_NodeSave(SG *sg, AG_DataSource *ds, SG_Node *node)
{
	off_t countOffs, skipSizeOffs;
	Uint32 count;
	SG_Node *chldNode;

	AG_ObjectLock(node);

	/* Save object metadata */
	AG_WriteString(ds, OBJECT(node)->name);
	if (OBJECT_CLASS(node)->pvt.libs[0] != '\0') {
		char s[AG_OBJECT_TYPE_MAX];
		Strlcpy(s, OBJECT_CLASS(node)->hier, sizeof(s));
		Strlcat(s, "@", sizeof(s));
		Strlcat(s, OBJECT_CLASS(node)->pvt.libs, sizeof(s));
		AG_WriteString(ds, s);
	} else {
		AG_WriteString(ds, OBJECT_CLASS(node)->hier);
	}
	skipSizeOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);

	/* Save object dataset */
	if (AG_ObjectSerialize(node, ds) == -1)
		goto fail;

	/* Save child nodes */
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	count = 0;
	OBJECT_FOREACH_CHILD(chldNode, node, sg_node) {
		if (SG_NodeSave(sg, ds, chldNode) == -1) {
			goto fail;
		}
		count++;
	}

	AG_WriteUint32At(ds, count, countOffs);
	AG_WriteUint32At(ds, AG_Tell(ds)-skipSizeOffs, skipSizeOffs);

	AG_ObjectUnlock(node);
	return (0);
fail:
	AG_ObjectUnlock(node);
	return (-1);
}

/* Load node data (and children) from data source. */
int
SG_NodeLoad(SG *sg, AG_DataSource *ds, SG_Node *parentNode)
{
	Uint count, i;
	char nodeName[AG_OBJECT_NAME_MAX];
	char classSpec[AG_OBJECT_TYPE_MAX];
	AG_ObjectClass *nodeClass;
	/* Uint32 skipSize; */
	SG_Node *node;

	/* Load object metadata */
	AG_CopyString(nodeName, ds, sizeof(nodeName));
	AG_CopyString(classSpec, ds, sizeof(classSpec));
/*	skipSize = AG_ReadUint32(ds); */
	(void)AG_ReadUint32(ds);
	
	/* Allocate node instance; load DSOs as needed. */
	if ((nodeClass = AG_LoadClass(classSpec)) == NULL) {
		return (-1);
	}
	if ((node = AG_TryMalloc(nodeClass->size)) == NULL) {
		return (-1);
	}
	AG_ObjectInit(node, nodeClass);
	AG_ObjectSetNameS(node, nodeName);

	/* Load object dataset */
	if (AG_ObjectUnserialize(node, ds) == -1)
		goto fail;

	/* Attach node to parent. */
	if (parentNode != NULL) {
		AG_ObjectAttach(parentNode, node);
	} else {
		AG_ObjectAttach(sg, node);
		sg->root = node;
	}

	/* Load child nodes */
	count = (Uint)AG_ReadUint32(ds);
	for (i = 0; i < count; i++) {
		if (SG_NodeLoad(sg, ds, node) == -1)
			goto fail_detach;
	}
	return (0);
fail_detach:
	if (node == sg->root) { sg->root = NULL; }
fail:
	AG_ObjectDestroy(node);
	return (-1);
}

SG_NodeClass sgNodeClass = {
	{
		"SG_Node",
		sizeof(SG_Node),
		{ 0,0 },
		Init,
		Reset,
		NULL,		/* destroy */
		Load,
		Save,
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	NULL,			/* draw */
	NULL,			/* intersect */
	Edit
};
