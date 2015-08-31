/*
 * Copyright (c) 2004-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Base vector graphics object.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/primitive.h>
#include <agar/gui/iconmgr.h>
#include <agar/vg/vg.h>
#include <agar/vg/vg_view.h>
#include <agar/vg/icons.h>
#include <agar/vg/icons_data.h>

#include <string.h>

const AG_Version vgVer = { 6, 1 };

int          vgInitedSubsystem = 0;
VG_NodeOps **vgNodeClasses;
Uint         vgNodeClassCount;
int          vgGUI = 1;

extern VG_NodeOps vgPointOps;
extern VG_NodeOps vgLineOps;
extern VG_NodeOps vgPolygonOps;
extern VG_NodeOps vgCircleOps;
extern VG_NodeOps vgArcOps;
extern VG_NodeOps vgTextOps;

VG_NodeOps *vgBuiltinClasses[] = {
	&vgPointOps,
	&vgLineOps,
	&vgPolygonOps,
	&vgCircleOps,
	&vgArcOps,
	&vgTextOps,
	NULL
};

void
VG_InitSubsystem(void)
{
	VG_NodeOps **vnOps;

	if (vgInitedSubsystem)
		return;

	vgNodeClasses = NULL;
	vgNodeClassCount = 0;

	AG_RegisterNamespace("VG", "VG_", "http://libagar.org/");
	if (vgGUI) {
		AG_RegisterClass(&vgViewClass);
	}
	for (vnOps = &vgBuiltinClasses[0]; *vnOps != NULL; vnOps++)
		VG_RegisterClass(*vnOps);

	vgIcon_Init();
	vgInitedSubsystem = 1;
}

void
VG_DestroySubsystem(void)
{
	if (!vgInitedSubsystem)
		return;
	
	if (vgGUI) {
		AG_UnregisterClass(&vgViewClass);
	}
	Free(vgNodeClasses);
	vgNodeClasses = NULL;
	vgNodeClassCount = 0;

	AG_UnregisterNamespace("VG");
	
	vgInitedSubsystem = 0;
}

VG *
VG_New(Uint flags)
{
	VG *vg;

	vg = Malloc(sizeof(VG));
	VG_Init(vg, flags);
	return (vg);
}

void
VG_Init(VG *vg, Uint flags)
{
	VG_Point *ptRoot;

	vg->flags = flags;
	vg->colors = NULL;
	vg->nColors = 0;
	vg->fillColor = VG_GetColorRGB(0,0,0);
	vg->selectionColor = VG_GetColorRGBA(0,200,0,150);
	vg->mouseoverColor = VG_GetColorRGBA(250,250,0,100);
	vg->layers = NULL;
	vg->nLayers = 0;
	TAILQ_INIT(&vg->nodes);
	AG_MutexInitRecursive(&vg->lock);
	
	vg->T = Malloc(sizeof(VG_Matrix));
	vg->nT = 1;
	vg->T[0] = VG_MatrixIdentity();
	
	VG_PushLayer(vg, _("Layer 0"));
	
	ptRoot = VG_PointNew(NULL, VGVECTOR(0.0f,0.0f));
	vg->root = VGNODE(ptRoot);
	vg->root->vg = vg;
	vg->root->handle = 1;
	VG_SetColorRGB(vg->root, 0, 150, 0);
}

/* Delete and free a node (including its children). */
void
VG_NodeDestroy(void *p)
{
	VG_Node *vn = p;
	VG_Node *vnChld, *vnNext;

#ifdef AG_DEBUG
	if (vn->vg != NULL || vn->parent != NULL)
		AG_FatalError("VG_NodeDetach() must precede VG_NodeDestroy()");
#endif

	for (vnChld = TAILQ_FIRST(&vn->cNodes);
	     vnChld != TAILQ_END(&vn->cNodes);
	     vnChld = vnNext) {
		vnNext = TAILQ_NEXT(vnChld, tree);
		TAILQ_REMOVE(&vnChld->vg->nodes, vnChld, list);
		VG_NodeDestroy(vnChld);
	}
	TAILQ_INIT(&vn->cNodes);

	if (vn->ops->destroy != NULL) {
		vn->ops->destroy(vn);
	}
	Free(vn);
}

/* Reinitialize the drawing. */
void
VG_Clear(VG *vg)
{
	VG_ClearNodes(vg);
	VG_ClearColors(vg);
}

/* Reinitialize the tree of entities. */
void
VG_ClearNodes(VG *vg)
{
	VG_Node *vnChld, *vnNext;

	if (vg->root == NULL) {
		return;
	}
	for (vnChld = TAILQ_FIRST(&vg->root->cNodes);
	     vnChld != TAILQ_END(&vg->root->cNodes);
	     vnChld = vnNext) {
		vnNext = TAILQ_NEXT(vnChld, tree);
		VG_NodeDetach(vnChld);
		VG_NodeDestroy(vnChld);
	}
	TAILQ_INIT(&vg->root->cNodes);
	TAILQ_INIT(&vg->nodes);
}

/* Reinitialize the color array. */
void
VG_ClearColors(VG *vg)
{
	if (vg->colors != NULL) {
		Free(vg->colors);
		vg->colors = NULL;
	}
	vg->nColors = 0;
}

void
VG_Destroy(VG *vg)
{
	VG_Clear(vg);
	Free(vg->layers);
	AG_MutexDestroy(&vg->lock);
}

void
VG_RegisterClass(VG_NodeOps *vnOps)
{
	vgNodeClasses = Realloc(vgNodeClasses,
	    (vgNodeClassCount+1)*sizeof(VG_NodeOps *));
	vgNodeClasses[vgNodeClassCount++] = vnOps;
}

void
VG_UnregisterClass(VG_NodeOps *vnOps)
{
	int i;

	for (i = 0; i < vgNodeClassCount; i++) {
		if (vgNodeClasses[i] == vnOps)
			break;
	}
	if (i == vgNodeClassCount) {
		return;
	}
	if (i < vgNodeClassCount-1) {
		memmove(&vgNodeClasses[i], &vgNodeClasses[i+1],
		    (vgNodeClassCount-i-1)*sizeof(VG_NodeOps *));
	}
	vgNodeClassCount--;
}


/* Lookup a node class by name. */
VG_NodeOps *
VG_LookupClass(const char *name)
{
	Uint i;

	for (i = 0; i < vgNodeClassCount; i++) {
		VG_NodeOps *vnOps = vgNodeClasses[i];
		if (strcmp(vnOps->name, name) == 0)
			return (vnOps);
	}
	AG_SetError("Invalid node type: %s", name);
	return (NULL);
}

/* Detach and free the specified node and its children. */
int
VG_Delete(void *pVn)
{
	VG_Node *vn = pVn;
	VG *vg = vn->vg;

#ifdef AG_DEBUG
	if (vg == NULL)
		AG_FatalError("VG_Delete() on unattached node %s%d",
		    vn->ops->name, vn->handle);
#endif
	VG_Lock(vg);
	if (vn->nDeps > 0) {
		AG_SetError("%s%u is in use", vn->ops->name, (Uint)vn->handle);
		goto fail;
	}
	if (vn->ops->deleteNode != NULL) {
		vn->ops->deleteNode(vn);
	}
	VG_NodeDetach(vn);
	VG_NodeDestroy(vn);
	VG_Unlock(vg);
	return (0);
fail:
	VG_Unlock(vg);
	return (-1);
}

static void
MoveNodesRecursively(VG *vgDst, VG_Node *vn)
{
	VG_Node *vnChld;

	VG_FOREACH_CHLD(vnChld, vn, vg_node) {
		MoveNodesRecursively(vgDst, vnChld);
	}
	vn->handle = VG_GenNodeName(vgDst, vn->ops->name);
	TAILQ_REMOVE(&vn->vg->nodes, vn, list);
	vn->vg = vgDst;
	TAILQ_INSERT_TAIL(&vgDst->nodes, vn, list);
}

/*
 * Move the contents of a source VG (to be discarded) to the specified
 * destination VG, under a given node.
 */
void
VG_Merge(void *pVnDst, VG *vgSrc)
{
	VG_Node *vnDst = pVnDst;
	VG_Node *vn = vgSrc->root;

	vn->vg = vnDst->vg;
	vn->parent = vnDst;
	TAILQ_INSERT_TAIL(&vnDst->vg->nodes, vn, list);
	TAILQ_INSERT_TAIL(&vnDst->cNodes, vn, tree);
	MoveNodesRecursively(vnDst->vg, vn);
	vgSrc->root = NULL;
}

/* Create a node reference to another node. */
void
VG_AddRef(void *p, void *pRef)
{
	VG_Node *vn = p;

	if (vn->vg != NULL) { VG_Lock(vn->vg); }
	vn->refs = Realloc(vn->refs, (vn->nRefs+1)*sizeof(VG_Node *));
	vn->refs[vn->nRefs++] = VGNODE(pRef);
	VGNODE(pRef)->nDeps++;
	if (vn->vg != NULL) { VG_Unlock(vn->vg); }
}

/* Remove a node reference to another node. */
Uint
VG_DelRef(void *pVn, void *pRef)
{
	VG_Node *vn = pVn;
	Uint newDeps;
	int i;

	if (vn->vg != NULL) { VG_Lock(vn->vg); }
	for (i = 0; i < vn->nRefs; i++) {
		if (vn->refs[i] == VGNODE(pRef))
			break;
	}
	if (i == vn->nRefs) {
		AG_FatalError("No such reference");
	}
	if (i < vn->nRefs-1) {
		memmove(&vn->refs[i], &vn->refs[i+1],
		    (vn->nRefs-i-1)*sizeof(VG_Node *));
	}
	vn->nRefs--;
	newDeps = (--VGNODE(pRef)->nDeps);
	if (vn->vg != NULL) { VG_Unlock(vn->vg); }
	return (newDeps);
}

void VG_SetBackgroundColor(VG *vg, VG_Color c) { vg->fillColor = c; }
void VG_SetSelectionColor(VG *vg, VG_Color c) { vg->selectionColor = c; }
void VG_SetMouseOverColor(VG *vg, VG_Color c) { vg->mouseoverColor = c; }

void
VG_NodeInit(void *p, VG_NodeOps *vnOps)
{
	VG_Node *vn = p;

	vn->ops = vnOps;
	vn->handle = 0;
	vn->sym[0] = '\0';
	vn->flags = 0;
	vn->layer = 0;
	vn->color = VG_GetColorRGB(250,250,250);
	vn->parent = NULL;
	vn->vg = NULL;
	vn->refs = NULL;
	vn->nRefs = 0;
	vn->nDeps = 0;
	vn->T = VG_MatrixIdentity();
	vn->p = NULL;
	TAILQ_INIT(&vn->cNodes);

	if (vn->ops->init != NULL)
		vn->ops->init(vn);
}

/* Generate a unique name for a node of the specified type. */
Uint32
VG_GenNodeName(VG *vg, const char *type)
{
	Uint32 name = 1;

	while (VG_FindNode(vg, name, type) != NULL) {
		if (++name >= VG_HANDLE_MAX)
			AG_FatalError("Out of node names");
	}
	return (name);
}

/*
 * Attach the specified node to a new parent. If the node has no
 * name assigned (handle=0), one is generated.
 */
void
VG_NodeAttach(void *pParent, void *pNode)
{
	VG_Node *vnParent = pParent;
	VG_Node *vn = pNode;
	VG *vg;

	if (vnParent == NULL) {
		vn->parent = NULL;
		vn->vg = NULL;
		return;
	}
	vg = vnParent->vg;

	VG_Lock(vg);
	
	if (vn->handle == 0) {
		vn->handle = VG_GenNodeName(vg, vn->ops->name);
	}
	vn->parent = vnParent;
	TAILQ_INSERT_TAIL(&vnParent->cNodes, vn, tree);
	TAILQ_INSERT_TAIL(&vg->nodes, vn, list);
	vn->vg = vg;
	VG_Unlock(vg);
}

/*
 * Detach the specified node from its current parent. The node itself
 * and all of its children are also detached from the VG. No reference
 * checking is done.
 */
void
VG_NodeDetach(void *p)
{
	VG_Node *vn = p;
	VG *vg = vn->vg;
	VG_Node *vnChld, *vnNext;

#ifdef AG_DEBUG
	if (vg == NULL)
		AG_FatalError("VG_NodeDetach() on unattached node");
#endif
	VG_Lock(vg);
	for (vnChld = TAILQ_FIRST(&vn->cNodes);
	     vnChld != TAILQ_END(&vn->cNodes);
	     vnChld = vnNext) {
		vnNext = TAILQ_NEXT(vnChld, tree);
		VG_NodeDetach(vnChld);
	}
	TAILQ_INIT(&vn->cNodes);

	if (vn->parent != NULL) {
		TAILQ_REMOVE(&vn->parent->cNodes, vn, tree);
		vn->parent = NULL;
	}
	TAILQ_REMOVE(&vg->nodes, vn, list);
	vn->vg = NULL;
	VG_Unlock(vg);
}

/* Set the symbolic name of a node. */
void
VG_SetSym(void *pNode, const char *fmt, ...)
{
	VG_Node *vn = pNode;
	va_list args;

	if (vn->vg != NULL) { VG_Lock(vn->vg); }
	va_start(args, fmt);
	Vsnprintf(vn->sym, sizeof(vn->sym), fmt, args);
	va_end(args);
	if (vn->vg != NULL) { VG_Unlock(vn->vg); }
}

void
VG_SetLayer(void *pNode, int layer)
{
	VGNODE(pNode)->layer = layer;
}

void
VG_SetColorv(void *pNode, const VG_Color *c)
{
	VG_Node *vn = pNode;
	vn->color.r = c->r;
	vn->color.g = c->g;
	vn->color.b = c->b;
	vn->color.a = c->a;
}

void
VG_SetColorRGB(void *pNode, Uint8 r, Uint8 g, Uint8 b)
{
	VG_Node *vn = pNode;
	vn->color.r = r;
	vn->color.g = g;
	vn->color.b = b;
}

void
VG_SetColorRGBA(void *pNode, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	VG_Node *vn = pNode;
	vn->color.r = r;
	vn->color.g = g;
	vn->color.b = b;
	vn->color.a = a;
}

/* Push a new layer onto the layer stack. */
VG_Layer *
VG_PushLayer(VG *vg, const char *name)
{
	VG_Layer *vgl;

	VG_Lock(vg);
	vg->layers = Realloc(vg->layers, (vg->nLayers+1) *
	                                 sizeof(VG_Layer));
	vgl = &vg->layers[vg->nLayers];
	vg->nLayers++;
	Strlcpy(vgl->name, name, sizeof(vgl->name));
	vgl->visible = 1;
	vgl->alpha = 255;
	vgl->color = VG_GetColorRGB(255,255,255);
	VG_Unlock(vg);
	return (vgl);
}

/* Pop the highest layer off the layer stack. */
void
VG_PopLayer(VG *vg)
{
	VG_Lock(vg);
	if (--vg->nLayers < 1) {
		vg->nLayers = 1;
	}
	VG_Unlock(vg);
}

static void
SaveMatrix(VG_Matrix *A, AG_DataSource *ds)
{
	int m, n;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 3; n++)
			AG_WriteFloat(ds, A->m[m][n]);
}

static void
LoadMatrix(VG_Matrix *A, AG_DataSource *ds)
{
	int m, n;

	for (m = 0; m < 3; m++)
		for (n = 0; n < 3; n++)
			A->m[m][n] = AG_ReadFloat(ds);
}

static void
SaveNodeGeneric(VG *vg, VG_Node *vn, AG_DataSource *ds)
{
	off_t nNodesOffs;
	Uint32 nNodes = 0;
	VG_Node *vnChld;

	if (vn->flags & VG_NODE_NOSAVE)
		return;

	AG_WriteString(ds, vn->ops->name);
	AG_WriteString(ds, vn->sym);
	AG_WriteUint32(ds, (Uint32)vn->handle);
	AG_WriteUint32(ds, (Uint32)(vn->flags & VG_NODE_SAVED_FLAGS));
	AG_WriteUint32(ds, (Uint32)vn->layer);
	VG_WriteColor(ds, &vn->color);
	SaveMatrix(&vn->T, ds);

	/* Save the entities. */
	nNodesOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	VG_FOREACH_CHLD(vnChld, vn, vg_node) {
		SaveNodeGeneric(vg, vnChld, ds);
		nNodes++;
	}
	AG_WriteUint32At(ds, nNodes, nNodesOffs);
}

static int
SaveNodeData(VG *vg, VG_Node *vn, AG_DataSource *ds)
{
	VG_Node *vnChld;

	VG_FOREACH_CHLD(vnChld, vn, vg_node) {
		if (SaveNodeData(vg, vnChld, ds) == -1)
			return (-1);
	}
	if (vn->ops->save != NULL) {
		vn->ops->save(vn, ds);
	}
	return (0);
}

void
VG_Save(VG *vg, AG_DataSource *ds)
{
	off_t nNodesOffs;
	Uint32 nNodes = 0;
	VG_Node *vn;
	Uint i;

	AG_WriteVersion(ds, "Agar-VG", &vgVer);
	AG_WriteString(ds, "VG");				/* name */

	VG_Lock(vg);

	AG_WriteUint32(ds, (Uint32)vg->flags);
	VG_WriteColor(ds, &vg->fillColor);
	VG_WriteColor(ds, &vg->fillColor);			/* gridColor */
	VG_WriteColor(ds, &vg->selectionColor);
	VG_WriteColor(ds, &vg->mouseoverColor);
	AG_WriteFloat(ds, 0.0f);				/* gridIval */

	/* Save the layer information. */
	AG_WriteUint32(ds, (Uint32)vg->nLayers);
	for (i = 0; i < vg->nLayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_WriteString(ds, layer->name);
		AG_WriteUint8(ds, (Uint8)layer->visible);
		VG_WriteColor(ds, &layer->color);
		AG_WriteUint8(ds, layer->alpha);
	}

	/* Save the color table. */
	AG_WriteUint32(ds, (Uint32)vg->nColors);
	for (i = 0; i < vg->nColors; i++) {
		VG_IndexedColor *vic = &vg->colors[i];
		AG_WriteString(ds, vic->name);
		VG_WriteColor(ds, &vic->color);
	}

	/* Save the entities. */
	nNodesOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	VG_FOREACH_CHLD(vn, vg->root, vg_node) {
		SaveNodeGeneric(vg, vn, ds);
		nNodes++;
	}
	AG_WriteUint32At(ds, nNodes, nNodesOffs);
	SaveNodeData(vg, vg->root, ds);
	VG_Unlock(vg);
}

static int
LoadNodeGeneric(VG *vg, VG_Node *vnParent, AG_DataSource *ds)
{
	char type[VG_TYPE_NAME_MAX];
	VG_Node *vn;
	VG_NodeOps *vnOps;
	Uint32 i, nNodes;
	
	AG_CopyString(type, ds, sizeof(type));
	if ((vnOps = VG_LookupClass(type)) == NULL) {
		return (-1);
	}
	vn = Malloc(vnOps->size);
	VG_NodeInit(vn, vnOps);
	AG_CopyString(vn->sym, ds, sizeof(vn->sym));
	vn->handle = AG_ReadUint32(ds);
	vn->flags = AG_ReadUint32(ds);
	vn->layer = (int)AG_ReadUint32(ds);
	vn->color = VG_ReadColor(ds);
	LoadMatrix(&vn->T, ds);
	VG_NodeAttach(vnParent, vn);

	nNodes = AG_ReadUint32(ds);
	for (i = 0; i < nNodes; i++) {
		if (LoadNodeGeneric(vg, vn, ds) == -1)
			return (-1);
	}
	return (0);
}

static int
LoadNodeData(VG *vg, VG_Node *vn, AG_DataSource *ds, const AG_Version *dsVer)
{
	VG_Node *vnChld;

	VG_FOREACH_CHLD(vnChld, vn, vg_node) {
		if (LoadNodeData(vg, vnChld, ds, dsVer) == -1)
			return (-1);
	}
	if (vn->ops->load != NULL &&
	    vn->ops->load(vn, ds, dsVer) == -1) {
		AG_SetError("%s%u: %s", vn->ops->name, (Uint)vn->handle,
		    AG_GetError());
		return (-1);
	}
	return (0);
}

int
VG_Load(VG *vg, AG_DataSource *ds)
{
	char name[VG_NAME_MAX];
	AG_Version dsVer;
	Uint32 i, nColors, nNodes;

	if (AG_ReadVersion(ds, "Agar-VG", &vgVer, &dsVer) != 0) {
		return (-1);
	}
	AG_CopyString(name, ds, sizeof(name));		/* Ignore */
	
	VG_Lock(vg);
	vg->flags = AG_ReadUint32(ds);
	vg->fillColor = VG_ReadColor(ds);
	(void)VG_ReadColor(ds);				/* gridColor */
	vg->selectionColor = VG_ReadColor(ds);
	vg->mouseoverColor = VG_ReadColor(ds);
	(void)AG_ReadFloat(ds);				/* gridIval */

	/* Read the layer information. */
	vg->nLayers = (Uint)AG_ReadUint32(ds);
	vg->layers = Realloc(vg->layers, vg->nLayers*sizeof(VG_Layer));
	for (i = 0; i < vg->nLayers; i++) {
		VG_Layer *layer = &vg->layers[i];

		AG_CopyString(layer->name, ds, sizeof(layer->name));
		layer->visible = (int)AG_ReadUint8(ds);
		layer->color = VG_ReadColor(ds);
		layer->alpha = AG_ReadUint8(ds);
	}

	/* Read the color table. */
	nColors = AG_ReadUint32(ds);
	vg->colors = Malloc(nColors*sizeof(VG_IndexedColor *));
	vg->nColors = nColors;
	for (i = 0; i < nColors; i++) {
		VG_IndexedColor *vic = &vg->colors[i];
		AG_CopyString(vic->name, ds, sizeof(vic->name));
		vic->color = VG_ReadColor(ds);
	}

	/* Read the entities. */
	VG_ClearNodes(vg);
	nNodes = AG_ReadUint32(ds);
	for (i = 0; i < nNodes; i++) {
		if (LoadNodeGeneric(vg, vg->root, ds) == -1)
			goto fail;
	}
	if (LoadNodeData(vg, vg->root, ds, &dsVer) == -1) {
		goto fail;
	}
	VG_Unlock(vg);
	return (0);
fail:
	VG_Unlock(vg);
	return (-1);
}

void
VG_WriteVector(AG_DataSource *ds, const VG_Vector *vtx)
{
	AG_WriteFloat(ds, vtx->x);
	AG_WriteFloat(ds, vtx->y);
}

void
VG_WriteColor(AG_DataSource *ds, const VG_Color *c)
{
	AG_WriteUint8(ds, c->r);
	AG_WriteUint8(ds, c->g);
	AG_WriteUint8(ds, c->b);
	AG_WriteUint8(ds, c->a);
}

VG_Vector
VG_ReadVector(AG_DataSource *ds)
{
	VG_Vector v;

	v.x = AG_ReadFloat(ds);
	v.y = AG_ReadFloat(ds);
	return (v);
}

VG_Color
VG_ReadColor(AG_DataSource *ds)
{
	VG_Color c;

	c.r = AG_ReadUint8(ds);
	c.g = AG_ReadUint8(ds);
	c.b = AG_ReadUint8(ds);
	c.a = AG_ReadUint8(ds);
	c.idx = -1;
	return (c);
}

/* Serialize a node->node reference. */
void
VG_WriteRef(AG_DataSource *ds, void *p)
{
	VG_Node *vn = p;

	AG_WriteString(ds, vn->ops->name);
	AG_WriteUint32(ds, vn->handle);
}

/* Deserialize a node->node reference. */
void *
VG_ReadRef(AG_DataSource *ds, void *pNode, const char *expType)
{
	VG_Node *vn = pNode;
	char rType[VG_TYPE_NAME_MAX];
	Uint32 handle;
	void *vnFound;

	AG_CopyString(rType, ds, sizeof(rType));
	handle = AG_ReadUint32(ds);

	if (expType != NULL) {
		if (strcmp(rType, expType) != 0) {
			AG_SetError("Unexpected reference type: %s "
			            "(expecting %s)", rType, expType);
			return (NULL);
		}
	}
	if ((vnFound = VG_FindNode(vn->vg, handle, rType)) == NULL) {
		AG_SetError("Reference to unexisting item: %s%u", rType,
		    (Uint)handle);
		return (NULL);
	}
	VG_AddRef(vn, vnFound);
	return (vnFound);
}

/* Return the element closest to the given point. */
void *
VG_PointProximity(VG_View *vv, const char *type, const VG_Vector *vPt,
    VG_Vector *vC, void *ignoreNode)
{
	VG *vg = vv->vg;
	VG_Node *vn, *vnClosest = NULL;
	float distClosest = AG_FLT_MAX, p;
	VG_Vector v, vClosest = VGVECTOR(AG_FLT_MAX,AG_FLT_MAX);

	VG_FOREACH_NODE(vn, vg, vg_node) {
		if (vn == ignoreNode ||
		    vn->ops->pointProximity == NULL) {
			continue;
		}
		if (type != NULL &&
		    strcmp(vn->ops->name, type) != 0) {
			continue;
		}
		v = *vPt;
		p = vn->ops->pointProximity(vn, vv, &v);
		if (p < distClosest) {
			distClosest = p;
			vnClosest = vn;
			vClosest = v;
		}
	}
	if (vC != NULL) {
		*vC = vClosest;
	}
	return (vnClosest);
}

/*
 * Return the element closest to the given point, ignoring all elements
 * beyond a specified distance.
 */
void *
VG_PointProximityMax(VG_View *vv, const char *type, const VG_Vector *vPt,
    VG_Vector *vC, void *ignoreNode, float distMax)
{
	VG *vg = vv->vg;
	VG_Node *vn, *vnClosest = NULL;
	float distClosest = AG_FLT_MAX, p;
	VG_Vector v, vClosest = VGVECTOR(AG_FLT_MAX,AG_FLT_MAX);

	VG_FOREACH_NODE(vn, vg, vg_node) {
		if (vn == ignoreNode ||
		    vn->ops->pointProximity == NULL) {
			continue;
		}
		if (type != NULL &&
		    strcmp(vn->ops->name, type) != 0) {
			continue;
		}
		v = *vPt;
		p = vn->ops->pointProximity(vn, vv, &v);
		if (p < distMax && p < distClosest) {
			distClosest = p;
			vnClosest = vn;
			vClosest = v;
		}
	}
	if (vC != NULL) {
		*vC = vClosest;
	}
	return (vnClosest);
}

/*
 * Compute the product of the transform matrices of the given node and its
 * parents in order. T is initialized to identity.
 */
void
VG_NodeTransform(void *p, VG_Matrix *T)
{
	VG_Node *node = p;
	VG_Node *cNode = node;
	TAILQ_HEAD_(vg_node) rNodes = TAILQ_HEAD_INITIALIZER(rNodes);

	while (cNode != NULL) {
		TAILQ_INSERT_HEAD(&rNodes, cNode, reverse);
		if (cNode->parent == NULL) {
			break;
		}
		cNode = cNode->parent;
	}
	*T = VG_MatrixIdentity();
	TAILQ_FOREACH(cNode, &rNodes, reverse)
		VG_MultMatrix(T, &cNode->T);
}

/* Compute the inverse of a VG transformation matrix. */
VG_Matrix
VG_MatrixInvert(VG_Matrix A)
{
	VG_Matrix B;
	float det, detInv;
	int i, j;

	B.m[0][0] = A.m[1][1]*A.m[2][2] - A.m[1][2]*A.m[2][1];
	B.m[0][1] = A.m[0][2]*A.m[2][1] - A.m[0][1]*A.m[2][2];
	B.m[0][2] = A.m[0][1]*A.m[1][2] - A.m[0][2]*A.m[1][1];
	B.m[1][0] = A.m[1][2]*A.m[2][0] - A.m[1][0]*A.m[2][2];
	B.m[1][1] = A.m[0][0]*A.m[2][2] - A.m[0][2]*A.m[2][0];
	B.m[1][2] = A.m[0][2]*A.m[1][0] - A.m[0][0]*A.m[1][2];
	B.m[2][0] = A.m[1][0]*A.m[2][1] - A.m[1][1]*A.m[2][0];
	B.m[2][1] = A.m[0][1]*A.m[2][0] - A.m[0][0]*A.m[2][1];
	B.m[2][2] = A.m[0][0]*A.m[1][1] - A.m[0][1]*A.m[1][0];

	det = A.m[0][0]*B.m[0][0] +
	      A.m[0][1]*B.m[1][0] +
	      A.m[0][2]*B.m[2][0];
	if (Fabs(det) <= 1e-6f)
		AG_FatalError("Singular matrix");

	detInv = 1.0/det;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			B.m[i][j] *= detInv;
	}
	return (B);
}

