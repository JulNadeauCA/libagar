/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Dimensioned 2D sketch with geometric constraints.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/text.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/button.h>
#include <agar/gui/menu.h>
#include <agar/gui/units.h>

#include "sk.h"
#include "sk_placement.h"
#include "sk_gui.h"

#include <math.h>
#include <string.h>

const char *skConstraintNames[] = {
	N_("Distance"),
	N_("Incidence"),
	N_("Angle"),
	N_("Perpendicular"),
	N_("Parallel"),
	N_("Tangent"),
};

SK_NodeOps skNodeOps = {
	"Node",
	sizeof(SK_Node),
	0,
	NULL,			/* init */
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* draw */
	NULL,			/* redraw */
	NULL,			/* edit */
	NULL,			/* proximity */
	NULL,			/* delete */
	NULL,			/* move */
	NULL,			/* constrained */
};

SK_NodeOps **skElements = NULL;
Uint         skElementsCnt = 0;

void
SK_RegisterClass(SK_NodeOps *nops)
{
	skElements = Realloc(skElements,(skElementsCnt+1)*sizeof(SK_NodeOps *));
	skElements[skElementsCnt] = nops;
	skElementsCnt++;
}

static int
SK_NodeOfClassGeneral(SK_Node *_Nonnull node, const char *_Nonnull cn)
{
	char cname[SK_TYPE_NAME_MAX], *cp, *c;
	char nname[SK_TYPE_NAME_MAX], *np, *s;

	Strlcpy(cname, cn, sizeof(cname));
	Strlcpy(nname, node->ops->name, sizeof(nname));
	cp = cname;
	np = nname;
	while ((c = AG_Strsep(&cp, ":")) != NULL &&
	       (s = AG_Strsep(&np, ":")) != NULL) {
		if (c[0] == '*' && c[1] == '\0')
			continue;
		if (strcmp(c, s) != 0)
			return (0);
	}
	return (1);
}

/* Evaluate whether a node's class name matches a pattern. */
int
SK_NodeOfClass(void *pNode, const char *cname)
{
	SK_Node *node = pNode;
	const char *c;

#ifdef AG_DEBUG
	if (cname[0] == '*' && cname[1] == '\0')
		AG_FatalError("Use SK_FOREACH_NODE()");
#endif
	for (c = &cname[0]; *c != '\0'; c++) {
		if (c[0] == ':' && c[1] == '*' && c[2] == '\0') {
			if (c == &cname[0]) {
				return (1);
			}
			if (strncmp(node->ops->name, cname, c - &cname[0])
			    == 0) {
				return (1);
			}
		}
	}
	return (SK_NodeOfClassGeneral(node, cname));	/* General case */
}

/*
 * Register the SK classes with the Agar object system, and also register
 * the default SK node classes.
 */
void
SK_InitSubsystem(void)
{
	AG_RegisterNamespace("Agar-SK", "SK_", "http://libagar.org/");

	/* Register our base Agar object classes. */
	AG_RegisterClass(&skClass);

	/* Register our built-in sketch element classes. */
	SK_RegisterClass(&skDummyOps);
	SK_RegisterClass(&skGroupOps);
	SK_RegisterClass(&skPointOps);
	SK_RegisterClass(&skLineOps);
	SK_RegisterClass(&skCircleOps);
	SK_RegisterClass(&skPixmapOps);
	SK_RegisterClass(&skDimensionOps);
	
	/* Initialize GUI facilities for Edit() */
	SK_InitGUI();
}

void
SK_DestroySubsystem(void)
{
	SK_DestroyGUI();
	AG_UnregisterNamespace("Agar-SK");
}

SK *
SK_New(void *parent, const char *name)
{
	char nameGen[AG_OBJECT_NAME_MAX];
	SK *sk;
	
	if (skElementsCnt == 0)
		AG_FatalError("SK is not initialized, use SK_InitSubsystem()");
	
	if (name == NULL) {
		AG_ObjectGenName(parent, &skClass, nameGen, sizeof(nameGen));
	} else {
		if (parent != NULL &&
		    AG_ObjectFindChild(parent, name) != NULL) {
			AG_SetError(_("%s: Existing child object %s"),
			    OBJECT(parent)->name, name);
			return (NULL);
		}
	}

	sk = Malloc(sizeof(SK));
	AG_ObjectInit(sk, &skClass);
	AG_ObjectSetNameS(sk, (name != NULL) ? name : nameGen);

	AG_ObjectAttach(parent, sk);
	return (sk);
}

static void
SK_InitRoot(SK *_Nonnull sk)
{
	SK_Point *pt;

	pt = Malloc(sizeof(SK_Point));
	sk->root = SKNODE(pt);
	SK_PointInit(pt, 0);
	SK_PointSize(pt, 3.0);
	SK_PointColor(pt, M_ColorRGB(1.0, 1.0, 0.0));
	SKNODE(pt)->sk = sk;
	SKNODE(pt)->flags |= SK_NODE_FIXED;
	TAILQ_INSERT_TAIL(&sk->nodes, sk->root, nodes);
}

static void
Init(void *_Nonnull obj)
{
	SK *sk = obj;
	const AG_Unit *un;
	
	OBJECT(sk)->flags |= AG_OBJECT_REOPEN_ONLOAD;

	sk->flags = 0;
	sk->nSolutions = 0;
	AG_MutexInitRecursive(&sk->lock);
	SK_InitCluster(&sk->ctGraph, 0);
	TAILQ_INIT(&sk->nodes);
	TAILQ_INIT(&sk->clusters);
	TAILQ_INIT(&sk->insns);

	if ((un = AG_FindUnit("mm")) == NULL) {
		AG_FatalError(NULL);
	}
	sk->uLen = un;
	sk->status = SK_WELL_CONSTRAINED;
	Strlcpy(sk->statusText, _("New sketch"), sizeof(sk->statusText));

	SK_InitRoot(sk);
}

/* Allocate a new node name. */
Uint
SK_GenNodeName(SK *sk, const char *type)
{
	Uint name = 1;

	while (SK_FindNode(sk, name, type) != NULL) {
		if (++name >= SK_NAME_MAX)
			AG_FatalError("Out of node names");
	}
	return (name);
}

M_Color
SK_NodeColor(void *p, const M_Color *Corig)
{
	SK_Node *node = p;
	M_Color C = *Corig;

	if (node->flags & SK_NODE_MOUSEOVER) {
		C.b = MIN((Corig->b + 1.0)/2.0,1.0);
	}
	if (SKNODE_SELECTED(node)) {
		C.g = MIN((C.g + 1.5)/2.0,1.0);
	}
	return (C);
}

/* Create a new instance of the base node class. */
SK_Node *
SK_NodeNew(void *pNode)
{
	SK_Node *node;

	node = Malloc(sizeof(SK_Node));
	SK_NodeInit(node, &skNodeOps,
	    SK_GenNodeName(SKNODE(pNode)->sk, "Node"),
	    0);
	SK_NodeAttach(pNode, node);
	return (node);
}

void
SK_NodeInit(void *np, const void *ops, Uint handle, Uint flags)
{
	SK_Node *n = np;

	n->ops = (const SK_NodeOps *)ops;
	n->handle = handle;
	AG_Snprintf(n->name, sizeof(n->name), "%s%u", n->ops->name, (Uint)handle);

	n->flags = flags;
	n->sk = NULL;
	n->pNode = NULL;
	n->nRefs = 0;
	n->refNodes = Malloc(sizeof(SK_Node *));
	n->nRefNodes = 0;
	n->cons = Malloc(sizeof(SK_Constraint *));
	n->nCons = 0;
	M_MatIdentity44v(&n->T);
	n->userData = NULL;
	TAILQ_INIT(&n->cnodes);
}

/* Change the name string associated with a node. */
void
SK_NodeSetName(void *p, const char *fmt, ...)
{
	SK_Node *node = p;
	va_list ap;

	va_start(ap, fmt);
	AG_Vsnprintf(node->name, sizeof(node->name), fmt, ap);
	va_end(ap);
}

/* Free a node and detach/free any child nodes. */
static void
SK_FreeNode(SK *_Nonnull sk, SK_Node *_Nonnull node)
{
	SK_Node *cnode, *cnodeNext;

	for (cnode = TAILQ_FIRST(&node->cnodes);
	     cnode != TAILQ_END(&node->cnodes);
	     cnode = cnodeNext) {
		cnodeNext = TAILQ_NEXT(cnode, sknodes);
		TAILQ_REMOVE(&sk->nodes, cnode, nodes);
		SK_FreeNode(sk, cnode);
	}
	if (node->ops->destroy != NULL) {
		node->ops->destroy(node);
	}
	Free(node->refNodes);
	Free(node->cons);
	Free(node);
}

static void
Reset(void *_Nonnull obj)
{
	SK *sk = obj;

	if (sk->root != NULL) {
		SK_FreeNode(sk, sk->root);
		sk->root = NULL;
	}
	TAILQ_INIT(&sk->nodes);
	SK_InitRoot(sk);

	SK_FreeCluster(&sk->ctGraph);
	SK_FreeClusters(sk);
	SK_FreeInsns(sk);
}

static int
SK_NodeSaveData(SK *_Nonnull sk, SK_Node *_Nonnull node, AG_DataSource *_Nonnull buf)
{
	SK_Node *chldNode;

	TAILQ_FOREACH(chldNode, &node->cnodes, sknodes) {
		if (SK_NodeSaveData(sk, chldNode, buf) == -1)
			return (-1);
	}
	if (node->ops->save != NULL &&
	    node->ops->save(sk, node, buf) == -1) {
		AG_SetError("NodeSave: %s", AG_GetError());
		return (-1);
	}
	return (0);
}

static int
SK_SaveNodeGeneric(SK *_Nonnull sk, SK_Node *_Nonnull node, AG_DataSource *_Nonnull buf)
{
	SK_Node *cnode;
	Uint32 ncnodes;
	off_t bsize_offs, ncnodes_offs;

	/* Save generic node information. */
	AG_WriteString(buf, node->ops->name);

	bsize_offs = AG_Tell(buf);
	AG_Seek(buf, sizeof(Uint32), AG_SEEK_CUR);

	AG_WriteUint32(buf, node->handle);
	AG_WriteString(buf, node->name);
	AG_WriteUint16(buf, (Uint16)node->flags);
	M_WriteMatrix44(buf, &node->T);

	/* Save the child nodes recursively. */
	ncnodes_offs = AG_Tell(buf);
	ncnodes = 0;
	AG_Seek(buf, sizeof(Uint32), AG_SEEK_CUR);
	TAILQ_FOREACH(cnode, &node->cnodes, sknodes) {
		if (SK_SaveNodeGeneric(sk, cnode, buf) == -1) {
			return (-1);
		}
		ncnodes++;
	}
	AG_WriteUint32At(buf, ncnodes, ncnodes_offs);

	/* Save the total block size to allow the loader to skip. */
	AG_WriteUint32At(buf, AG_Tell(buf)-bsize_offs, bsize_offs);
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull buf)
{
	SK *sk = obj;
	SK_Constraint *ct;
	Uint32 count;
	off_t offs;
	
	AG_MutexLock(&sk->lock);
	
	AG_WriteUint32(buf, sk->flags);
	AG_WriteString(buf, sk->uLen->key);

	/* Save the generic part of all nodes. */
	if (SK_SaveNodeGeneric(sk, sk->root, buf) == -1)
		goto fail;

	/* Save the data part of all nodes. */
	if (SK_NodeSaveData(sk, sk->root, buf) == -1)
		goto fail;

	/* Save the graph of geometric constraints. */
	offs = AG_Tell(buf);
	count = 0;
	AG_Seek(buf, sizeof(Uint32), AG_SEEK_CUR);
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		AG_WriteUint32(buf, (Uint32)ct->type);
		AG_WriteUint32(buf, (Uint32)ct->uType);
		SK_WriteRef(buf, ct->n1);
		SK_WriteRef(buf, ct->n2);
		switch (ct->type) {
		case SK_DISTANCE:
			M_WriteReal(buf, ct->ct_distance);
			break;
		case SK_ANGLE:
			M_WriteReal(buf, ct->ct_angle);
			break;
		default:
			break;
		}
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
	AG_MutexUnlock(&sk->lock);
	return (0);
fail:
	AG_MutexUnlock(&sk->lock);
	return (-1);
}

/* Load the data part of a node. */
static int
SK_LoadNodeData(SK *_Nonnull sk, SK_Node *_Nonnull node, AG_DataSource *_Nonnull buf)
{
	SK_Node *chldNode;

	TAILQ_FOREACH(chldNode, &node->cnodes, sknodes) {
		if (SK_LoadNodeData(sk, chldNode, buf) == -1)
			return (-1);
	}
	if (node->ops->load != NULL &&
	    node->ops->load(sk, node, buf) == -1) {
		AG_SetError("%s: %s", node->name, AG_GetError());
		return (-1);
	}
	return (0);
}

/* Load the generic part of a node. */
static int
SK_LoadNodeGeneric(SK *_Nonnull sk, SK_Node *_Nonnull *_Nullable rnode,
    AG_DataSource *_Nonnull buf)
{
	char type[SK_TYPE_NAME_MAX];
	SK_Node *node;
	Uint32 bsize, nchildren, j;
	Uint32 handle;
	Uint i;

	/* Load generic node information. */
	AG_CopyString(type, buf, sizeof(type));
	bsize = AG_ReadUint32(buf);
	for (i = 0; i < skElementsCnt; i++) {
		if (strcmp(skElements[i]->name, type) == 0)
			break;
	}
	if (i == skElementsCnt) {
		if (sk->flags & SK_SKIP_UNKNOWN_NODES) {
			fprintf(stderr, "%s: skipping node (%s/%luB)\n",
			    OBJECT(sk)->name, type, (Ulong)bsize);
			AG_Seek(buf, bsize, AG_SEEK_CUR);
			*rnode = NULL;
			return (0);
		} else {
			AG_SetError("Unimplemented node class: %s (%luB)",
			    type, (Ulong)bsize);
			return (-1);
		}
	}
	node = Malloc(skElements[i]->size);
	handle = AG_ReadUint32(buf);
	skElements[i]->init(node, (Uint)handle);
	node->sk = sk;

	AG_CopyString(node->name, buf, sizeof(node->name));
	node->flags = (Uint)AG_ReadUint16(buf);
	M_ReadMatrix44v(buf, &node->T);

	/* Load the child nodes recursively. */
	nchildren = AG_ReadUint32(buf);
	for (j = 0; j < nchildren; j++) {
		SK_Node *cnode;

		if (SK_LoadNodeGeneric(sk, &cnode, buf) == -1) {
			AG_SetError("subnode: %s", AG_GetError());
			return (-1);
		}
		if (cnode != NULL)
			SK_NodeAttach(node, cnode);
	}
	*rnode = node;
	return (0);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull buf, const AG_Version *_Nonnull ver)
{
	char unitKey[128];
	SK *sk = obj;
	Uint32 i, count;
	SK_Constraint *ct;
	
	AG_MutexLock(&sk->lock);
	sk->flags = (Uint)AG_ReadUint32(buf);
	AG_CopyString(unitKey, buf, sizeof(unitKey));
	if ((sk->uLen = AG_FindUnit(unitKey)) == NULL) {
		AG_MutexUnlock(&sk->lock);
		return (-1);
	}

	/* Free the existing root node. */
	if (sk->root != NULL) {
		SK_FreeNode(sk, sk->root);
		sk->root = NULL;
	}
	TAILQ_INIT(&sk->nodes);

	/*
	 * Load the generic part of all nodes. We need to load the data
	 * afterwards to properly resolve interdependencies.
	 */
	if (SK_LoadNodeGeneric(sk, &sk->root, buf) == -1) {
		goto fail;
	}
	TAILQ_INSERT_HEAD(&sk->nodes, sk->root, nodes);

	/* Load the data part of all nodes. */
	if (SK_LoadNodeData(sk, sk->root, buf) == -1)
		goto fail;

	/* Load the geometric constraint data. */
	count = AG_ReadUint32(buf);
	for (i = 0; i < count; i++) {
		ct = AG_Malloc(sizeof(SK_Constraint));
		ct->type = (enum sk_constraint_type)AG_ReadUint32(buf);
		ct->uType = (enum sk_constraint_type)AG_ReadUint32(buf);
		ct->n1 = SK_ReadRef(buf, sk, NULL);
		ct->n2 = SK_ReadRef(buf, sk, NULL);
		switch (ct->type) {
		case SK_DISTANCE:
			ct->ct_distance = M_ReadReal(buf);
			break;
		case SK_ANGLE:
			ct->ct_angle = M_ReadReal(buf);
			break;
		default:
			break;
		}
		SK_NodeAddConstraint(ct->n1, ct);
		SK_NodeAddConstraint(ct->n2, ct);
		TAILQ_INSERT_TAIL(&sk->ctGraph.edges, ct, constraints);
	}
	SK_Update(sk);
	AG_MutexUnlock(&sk->lock);
	return (0);
fail:
	AG_MutexUnlock(&sk->lock);
	AG_ObjectReset(sk);
	return (-1);
}

/*
 * Compute the product of the transform matrices of the given node and its
 * parents in order. T is initialized to identity.
 */
void
SK_GetNodeTransform(void *p, M_Matrix44 *T)
{
	SK_Node *node = p;
	SK_Node *cnode = node;
	TAILQ_HEAD_(sk_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);
	
	M_MatIdentity44v(T);

	/*
	 * Build a list of parent nodes and multiply their matrices in order
	 * (ugly but faster than computing the product of their inverses).
	 */
	while (cnode != NULL) {
		TAILQ_INSERT_HEAD(&rnodes, cnode, rnodes);
		if (cnode->pNode == NULL) {
			break;
		}
		cnode = cnode->pNode;
	}
	TAILQ_FOREACH(cnode, &rnodes, rnodes)
		M_MatMult44v(T, &cnode->T);
}

/*
 * Compute the product of the inverse transform matrices of the given node
 * and its parents.
 */
void
SK_GetNodeTransformInverse(void *p, M_Matrix44 *T)
{
	SK_Node *node = p;
	SK_Node *cnode = node;
	TAILQ_HEAD_(sk_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

	M_MatIdentity44v(T);

	while (cnode != NULL) {
		TAILQ_INSERT_TAIL(&rnodes, cnode, rnodes);
		if (cnode->pNode == NULL) {
			break;
		}
		cnode = cnode->pNode;
	}
	TAILQ_FOREACH(cnode, &rnodes, rnodes) {
		M_Matrix44 Tinv;

		Tinv = M_MatInvert44(cnode->T);
		M_MatMult44v(T, &Tinv);
	}
}

void
SK_NodeRedraw(void *p, SK_View *skv)
{
	SK_Node *node = p;

	if (node->ops->redraw != NULL)
		node->ops->redraw(node, skv);
}

/* Return the orientation vector of a node. */
M_Vector3
SK_NodeDir(void *p)
{
	SK_Node *node = p;
	M_Matrix44 T;
	M_Vector4 v = M_VECTOR4(0.0, 0.0, 1.0, 0.0);
	
	SK_GetNodeTransform(node, &T);
	M_MatMultVector44v(&v, &T);
	return M_VecFromProj3(v);
}

/* Create a new dependency table entry for the given node. */
void
SK_NodeAddReference(void *pNode, void *pOther)
{
	SK_Node *node = pNode;
	SK_Node *other = pOther;

	if (node == other)
		return;
	
	node->refNodes = Realloc(node->refNodes,
	                         (node->nRefNodes+1)*sizeof(SK_Node *));
	node->refNodes[node->nRefNodes++] = other;
	other->nRefs++;
}

/* Remove a dependency table entry. */
void
SK_NodeDelReference(void *pNode, void *pOther)
{
	SK_Node *node = pNode;
	SK_Node *other = pOther;
	Uint i;

	if (node == other)
		return;

	for (i = 0; i < node->nRefNodes; i++) {
		if (node->refNodes[i] != other) {
			continue;
		}
		if (i < node->nRefNodes-1) {
			memmove(&node->refNodes[i], &node->refNodes[i+1],
			    (node->nRefNodes - i - 1)*sizeof(SK_Node *));
		}
		node->nRefNodes--;
		other->nRefs--;
		break;
	}
}

/* Associate a constraint with a node. */
void
SK_NodeAddConstraint(void *pNode, SK_Constraint *ct)
{
	SK_Node *node = pNode;
	Uint i;

	for (i = 0; i < node->nCons; i++) {
		if (node->cons[i] == ct)
			return;
	}
	node->cons = Realloc(node->cons, (node->nCons+1) *
	                                 sizeof(SK_Constraint *));
	node->cons[node->nCons++] = ct;
}

/* Dissociate a constraint from a node. */
void
SK_NodeDelConstraint(void *pNode, SK_Constraint *ct)
{
	SK_Node *node = pNode;
	Uint i;

	for (i = 0; i < node->nCons; i++) {
		if (node->cons[i] != ct) {
			continue;
		}
		if (i < node->nCons-1) {
			memmove(&node->cons[i], &node->cons[i+1],
			    (node->nCons - i - 1)*sizeof(SK_Constraint *));
		}
		node->nCons--;
		break;
	}
}

/* Search a node by handle and class. */
void *
SK_FindNode(SK *sk, Uint handle, const char *type)
{
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->handle == handle &&
		    strcmp(node->ops->name, type) == 0)
			return (node);
	}
	AG_SetError("No such node: %u", (Uint)handle);
	return (NULL);
}

/* Search a node by name only. */
void *
SK_FindNodeByName(SK *sk, const char *name)
{
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (strcmp(node->name, name) == 0)
			return (node);
	}
	AG_SetError("No such node: %s", name);
	return (NULL);
}

/* Return the absolute position of a node. */
M_Vector3
SK_Pos(void *p)
{
	SK_Node *node = p;
	M_Matrix44 T;
	M_Vector4 v = M_VECTOR4(0.0, 0.0, 0.0, 1.0);
	
	SK_GetNodeTransform(node, &T);
	M_MatMultVector44v(&v, &T);
	return M_VECTOR3(v.x, v.y, v.z);
}

/* Return the absolute position of a node. */
M_Vector2
SK_Pos2(void *p)
{
	SK_Node *node = p;
	M_Matrix44 T;
	M_Vector4 v = M_VECTOR4(0.0, 0.0, 0.0, 1.0);
	
	SK_GetNodeTransform(node, &T);
	M_MatMultVector44v(&v, &T);
	return M_VECTOR2(v.x, v.y);
}

/* Attach a node to another node in the sketch. */
void
SK_NodeAttach(void *ppNode, void *pcNode)
{
	SK_Node *pNode = ppNode;
	SK_Node *cNode = pcNode;

	cNode->sk = pNode->sk;
	cNode->pNode = pNode;
	TAILQ_INSERT_TAIL(&pNode->cnodes, cNode, sknodes);
	TAILQ_INSERT_TAIL(&pNode->sk->nodes, cNode, nodes);
}

/* Detach a node from its parent in the sketch. */
void
SK_NodeDetach(void *ppNode, void *pcNode)
{
	SK_Node *pNode = ppNode;
	SK_Node *cNode = pcNode;
	SK_Node *subnode;
	SK *sk = pNode->sk;
	SK_Constraint *ct;

	while ((subnode = TAILQ_FIRST(&cNode->cnodes)) != NULL) {
		SK_NodeDetach(cNode, subnode);
	}
free_constraints:
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		if (ct->n1 == cNode || ct->n2 == cNode) {
			SK_DelConstraint(&sk->ctGraph, ct);
			goto free_constraints;
		}
	}
	TAILQ_REMOVE(&pNode->cnodes, cNode, sknodes);
	TAILQ_REMOVE(&sk->nodes, cNode, nodes);
	cNode->sk = NULL;
	cNode->pNode = NULL;
}

/* Move a node up in the order of rendering. */
void
SK_NodeMoveUp(void *p)
{
	SK_Node *node = p;
	SK_Node *prev;

	AG_ObjectLock(node->sk);
	if (node != TAILQ_FIRST(&node->pNode->cnodes)) {
		prev = TAILQ_PREV(node, sk_nodeq, sknodes);
		TAILQ_REMOVE(&node->pNode->cnodes, node, sknodes);
		TAILQ_INSERT_BEFORE(prev, node, sknodes);
	}
	AG_ObjectUnlock(node->sk);
}

/* Move a node down in the order of rendering. */
void
SK_NodeMoveDown(void *p)
{
	SK_Node *node = p;
	SK_Node *next = TAILQ_NEXT(node, sknodes);

	AG_ObjectLock(node->sk);
	if (next != NULL) {
		TAILQ_REMOVE(&node->pNode->cnodes, node, sknodes);
		TAILQ_INSERT_AFTER(&node->pNode->cnodes, next, node, sknodes);
	}
	AG_ObjectUnlock(node->sk);
}

/* Move a node to the tail of its parent's child list. */
void
SK_NodeMoveTail(void *p)
{
	SK_Node *n = p;

	AG_ObjectLock(n->sk);
	TAILQ_REMOVE(&n->pNode->cnodes, n, sknodes);
	TAILQ_INSERT_TAIL(&n->pNode->cnodes, n, sknodes);
	AG_ObjectUnlock(n->sk);
}

/* Move a node to the head of its parent's child list. */
void
SK_NodeMoveHead(void *p)
{
	SK_Node *n = p;

	AG_ObjectLock(n->sk);
	TAILQ_REMOVE(&n->pNode->cnodes, n, sknodes);
	TAILQ_INSERT_HEAD(&n->pNode->cnodes, n, sknodes);
	AG_ObjectUnlock(n->sk);
}

/* Move a node to a different parent node. */
void
SK_NodeMoveToParent(void *pNode, void *pParent)
{
	SK_Node *n = pNode;
	SK_Node *pOld = n->pNode;
	SK_Node *pNew = pParent;

	AG_ObjectLock(n->sk);
	TAILQ_REMOVE(&pOld->cnodes, n, sknodes);
	TAILQ_INSERT_TAIL(&pNew->cnodes, n, sknodes);
	AG_ObjectUnlock(n->sk);
}

/*
 * Detach and free a node (and its children) from the sketch. Annotations
 * and constraints referencing the node are automatically deleted as well.
 */
int
SK_NodeDel(void *p)
{
	SK_Node *node = p;
	SK *sk = node->sk;
	SK_Constraint *ct;

	if (node == sk->root) {
		AG_SetError("Cannot delete root node");
		return (-1);
	}
	if (!SK_NodeOfClass(node, "Annot:Dimension:*")) {
		SK_Dimension *dim;
deldims:
		SK_FOREACH_NODE_CLASS(dim, sk, sk_dimension,
		    "Annot:Dimension:*") {
			if (dim->n1 == node || dim->n2 == node) {
				SKNODE(dim)->ops->del(dim);
				goto deldims;
			}
		}
	}
delcts:
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		if (ct->n1 == node || ct->n2 == node) {
			SK_NodeDelConstraint(ct->n1, ct);
			SK_NodeDelConstraint(ct->n2, ct);
			SK_DelConstraint(&sk->ctGraph, ct);
			goto delcts;
		}
	}
	if (node->nRefs > 0) {
		AG_SetError("Node is being referenced");
		return (-1);
	}

	SK_NodeDetach(node->pNode, node);
	SK_FreeNode(sk, node);
	return (0);
}

void
SK_WriteRef(AG_DataSource *buf, void *pNode)
{
	SK_Node *node = pNode;

	AG_WriteString(buf, node->ops->name);
	AG_WriteUint32(buf, (Uint32)node->handle);
}

void *
SK_ReadRef(AG_DataSource *buf, SK *sk, const char *expType)
{
	char rType[SK_TYPE_NAME_MAX];

	AG_CopyString(rType, buf, sizeof(rType));
	if (expType != NULL) {
		if (strcmp(rType, expType) != 0) {
			AG_Verbose("Unexpected reference type: `%s' "
			           "(expecting %s)", rType, expType);
			AG_FatalError("SK_ReadRef");
		}
	}
	return (SK_FindNode(sk, (Uint)AG_ReadUint32(buf), rType));
}

/* Set the distance unit used by this sketch. */
void
SK_SetLengthUnit(SK *sk, const AG_Unit *unit)
{
	AG_MutexLock(&sk->lock);
	sk->uLen = unit;
	AG_MutexUnlock(&sk->lock);
}

/*
 * Perform a proximity query with the given vector against all elements
 * of the given type (or all elements if type is NULL), and return the
 * closest item.
 *
 * The closest point of the closest item is also returned in vC.
 */
void *
SK_ProximitySearch(SK *sk, const char *type, const M_Vector3 *v, M_Vector3 *vC,
    void *nodeIgnore)
{
	SK_Node *node, *nClosest = NULL;
	M_Real rClosest = M_INFINITY, p;
	M_Vector3 vClosest = M_VecGet3(M_INFINITY, M_INFINITY, 0.0);

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node == nodeIgnore ||
		    node->ops->proximity == NULL) {
			continue;
		}
		if (type != NULL &&
		    strcmp(node->ops->name, type) != 0) {
			continue;
		}
		p = node->ops->proximity(node, v, vC);
		if (p < rClosest) {
			rClosest = p;
			nClosest = node;
			vClosest.x = vC->x;
			vClosest.y = vC->y;
		}
	}
	vC->x = vClosest.x;
	vC->y = vClosest.y;
	vC->z = 0.0;
	return (nClosest);
}

/* Search a node by name in a sketch. */
/* XXX use a hash table */
SK_Cluster *
SK_FindCluster(SK *sk, Uint name)
{
	SK_Cluster *cl;

	TAILQ_FOREACH(cl, &sk->clusters, clusters) {
		if (cl->name == name)
			return (cl);
	}
	AG_SetError("No such cluster: %u", name);
	return (NULL);
}

/* Allocate a new cluster name. */
Uint
SK_GenClusterName(SK *sk)
{
	Uint name = 1;

	while (SK_FindCluster(sk, name) != NULL) {
		if (++name >= SK_NAME_MAX)
			AG_FatalError("Out of cluster names");
	}
	return (name);
}

void
SK_InitCluster(SK_Cluster *cl, Uint name)
{
	cl->name = name;
	TAILQ_INIT(&cl->edges);
}

void
SK_CopyCluster(const SK_Cluster *clSrc, SK_Cluster *clDst)
{
	SK_Constraint *ct;

	TAILQ_FOREACH(ct, &clSrc->edges, constraints)
		SK_AddConstraintCopy(clDst, ct);
}

void
SK_FreeCluster(SK_Cluster *cl)
{
	SK_Constraint *ct;

	while ((ct = TAILQ_FIRST(&cl->edges)) != NULL) {
		TAILQ_REMOVE(&cl->edges, ct, constraints);
		Free(ct);
	}
}

void
SK_FreeClusters(SK *sk)
{
	SK_Cluster *cl;

	while ((cl = TAILQ_FIRST(&sk->clusters)) != NULL) {
		TAILQ_REMOVE(&sk->clusters, cl, clusters);
		SK_FreeCluster(cl);
		Free(cl);
	}
}

void
SK_FreeInsns(SK *sk)
{
	SK_Insn *si;

	while ((si = TAILQ_FIRST(&sk->insns)) != NULL) {
		TAILQ_REMOVE(&sk->insns, si, insns);
		Free(si);
	}
}

/* Create a new constraint edge in the given constraint graph. */
SK_Constraint *
SK_AddConstraint(SK_Cluster *cl, void *node1, void *node2,
    enum sk_constraint_type type, ...)
{
	SK_Constraint *ct;
	va_list ap;

	if (SK_FindConstraint(cl, SK_CONSTRAINT_ANY, node1, node2) != NULL) {
		AG_SetError(_("Existing constraint; new %s constraint would "
		              "overconstraint sketch."),
			      skConstraintNames[type]);
		return (NULL);
	}
	ct = Malloc(sizeof(SK_Constraint));
	ct->uType = type;
	ct->n1 = node1;
	ct->n2 = node2;

	va_start(ap, type);
	switch (type) {
	case SK_DISTANCE:
		ct->ct_distance = (M_Real)va_arg(ap, double);
		break;
	case SK_ANGLE:
		ct->ct_angle = (M_Real)va_arg(ap, double);
		break;
	default:
		break;
	}
	va_end(ap);

	switch (type) {
	case SK_INCIDENT:
		ct->type = SK_DISTANCE;
		ct->ct_distance = 0.0;
		break;
	case SK_PERPENDICULAR:
		ct->type = SK_ANGLE;
		ct->ct_angle = M_PI/2.0;
		break;
	case SK_PARALLEL:
		ct->type = SK_ANGLE;
		ct->ct_angle = 0.0;
		break;
	default:
		ct->type = ct->uType;
		break;
	}
	TAILQ_INSERT_TAIL(&cl->edges, ct, constraints);
	return (ct);
}

/* Duplicate a constraint. */
SK_Constraint *
SK_DupConstraint(const SK_Constraint *ct1)
{
	SK_Constraint *ct2;

	ct2 = Malloc(sizeof(SK_Constraint));
	ct2->type = ct1->type;
	ct2->uType = ct1->uType;
	ct2->n1 = ct1->n1;
	ct2->n2 = ct1->n2;
	switch (ct1->type) {
	case SK_DISTANCE:
		ct2->ct_distance = ct1->ct_distance;
		break;
	case SK_ANGLE:
		ct2->ct_angle = ct1->ct_angle;
		break;
	default:
		break;
	}
	return (ct2);
}

/* Duplicate a constraint edge. */
SK_Constraint *
SK_AddConstraintCopy(SK_Cluster *clDst, const SK_Constraint *ct)
{
	switch (ct->type) {
	case SK_DISTANCE:
		return SK_AddConstraint(clDst, ct->n1, ct->n2, SK_DISTANCE,
		                        ct->ct_distance);
	case SK_ANGLE:
		return SK_AddConstraint(clDst, ct->n1, ct->n2, SK_ANGLE,
		                        ct->ct_angle);
	default:
		break;
	}
	return SK_AddConstraint(clDst, ct->n1, ct->n2, ct->type);
}

/* Destroy a constraint edge. */
void
SK_DelConstraint(SK_Cluster *cl, SK_Constraint *ct)
{
	TAILQ_REMOVE(&cl->edges, ct, constraints);
	Free(ct);
}

/* Destroy a constraint edge matching the argument. */
int
SK_DelSimilarConstraint(SK_Cluster *cl, const SK_Constraint *ctRef)
{
	SK_Constraint *ct;

	if ((ct = SK_FindSimilarConstraint(cl, ctRef)) == NULL) {
		AG_SetError("No matching constraint");
		return (-1);
	}
	SK_DelConstraint(cl, ct);
	return (0);
}

int
SK_CompareConstraints(const SK_Constraint *ct1, const SK_Constraint *ct2)
{
	if (ct1->type == ct2->type &&
	    ((ct1->n1 == ct2->n1 && ct1->n2 == ct2->n2) ||
	     (ct1->n1 == ct2->n2 && ct1->n2 == ct2->n1))) {
#if 1
		switch (ct1->type) {
		case SK_DISTANCE:
			return (int)(ct1->ct_distance - ct2->ct_distance);
		case SK_ANGLE:
			return (int)(ct1->ct_angle - ct2->ct_angle);
		default:
			return (0);
		}
#else
		return (0);
#endif
	}
	return (1);
}

/*
 * Search a constraint graph for any constraint of a given type between
 * two given nodes.
 */
SK_Constraint *
SK_FindConstraint(const SK_Cluster *cl, enum sk_constraint_type type,
    void *n1, void *n2)
{
	SK_Constraint *ct;

	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if ((ct->type == type || type == SK_CONSTRAINT_ANY) &&
		    ((ct->n1 == n1 && ct->n2 == n2) ||
		     (ct->n1 == n2 && ct->n2 == n1)))
			return (ct);
	}
	return (NULL);
}

/* Search a constraint graph for a constraint matching the argument. */
SK_Constraint *
SK_FindSimilarConstraint(const SK_Cluster *cl, const SK_Constraint *ct)
{
	SK_Constraint *ct2;

	TAILQ_FOREACH(ct2, &cl->edges, constraints) {
		if (SK_CompareConstraints(ct2, ct) == 0)
			return (ct2);
	}
	return (NULL);
}

/*
 * Check if the given two nodes share a constraint edge in the given
 * constraint graph.
 */
SK_Constraint *
SK_ConstrainedNodes(const SK_Cluster *cl, const SK_Node *n1,
    const SK_Node *n2)
{
	SK_Constraint *ct;

	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if ((ct->n1 == n1 && ct->n2 == n2) ||
		    (ct->n1 == n2 && ct->n2 == n1))
			return (ct);
	}
	return (NULL);
}

/*
 * Return the number of constraints for the given node, not counting incidence
 * constraints between line segments and their endpoints.
 */
Uint
SK_NodeConstraintCount(const SK_Cluster *cl, void *node)
{
	SK_Node *nOther;
	SK_Constraint *ct;
	Uint count = 0;

	/* TODO use nCons */
	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if (ct->n1 != node && ct->n2 != node) {
			continue;
		}
		nOther = (ct->n1 == node) ? ct->n2 : ct->n1;
		if (ct->type == SK_DISTANCE && ct->ct_distance == 0.0) {
			if (SK_NodeOfClass(node, "Point:*") &&
			    SK_NodeOfClass(nOther, "Line:*")) {
				continue;
			}
			if (SK_NodeOfClass(node, "Line:*") &&
			    SK_NodeOfClass(nOther, "Point:*")) {
				continue;
			}
		}
		count++;
	}
	return (count);
}

/* Evaluate whether the given node is in the given constraint graph. */
/* XXX optimize */
int
SK_NodeInCluster(const SK_Node *node, const SK_Cluster *cl)
{
	SK_Constraint *ct;

	TAILQ_FOREACH(ct, &cl->edges, constraints) {
		if (ct->n1 == node || ct->n2 == node)
			return (1);
	}
	return (0);
}

/*
 * Count the number of edges between a given node and any other node
 * in the given subgraph clSub of graph cl.
 */
Uint
SK_ConstraintsToSubgraph(const SK_Cluster *clOrig, const SK_Node *node,
    const SK_Cluster *clSub, SK_Constraint *rv[2])
{
	SK_Constraint *ct;
	Uint count = 0;

	TAILQ_FOREACH(ct, &clOrig->edges, constraints) {
		if ((ct->n1 == node && SK_NodeInCluster(ct->n2, clSub)) ||
		    (ct->n2 == node && SK_NodeInCluster(ct->n1, clSub))) {
			if (count < 2) {
				rv[count] = ct;
			}
			count++;
		}
	}
	return (count);
}

/* Add a construction step for the construction phase of the solver. */
SK_Insn *
SK_AddInsn(SK *sk, enum sk_insn_type type, ...)
{
	SK_Insn *si;
	va_list ap;

	si = Malloc(sizeof(SK_Insn));
	si->type = type;

	va_start(ap, type);
	switch (type) {
	case SK_COMPOSE_PAIR:
		si->n[0] = va_arg(ap, void *);
		si->n[1] = va_arg(ap, void *);
		si->ct01 = va_arg(ap, void *);
#ifdef AG_DEBUG
		if (si->n[0] == NULL || si->n[1] == NULL || si->ct01 == NULL ||
		    si->ct01->type >= SK_CONSTRAINT_LAST)
			AG_FatalError("Bad args");
#endif
		Debug(sk, "+ COMPOSE_PAIR(%s,%s)\n",
		    si->n[0]->name, si->n[1]->name);
		break;
	case SK_COMPOSE_RING:
		si->n[0] = va_arg(ap, void *);
		si->n[1] = va_arg(ap, void *);
		si->n[2] = va_arg(ap, void *);
		si->ct01 = va_arg(ap, void *);
		si->ct02 = va_arg(ap, void *);
#ifdef AG_DEBUG
		if (si->n[0] == NULL || si->n[1] == NULL || si->n[2] == NULL ||
		    si->ct01 == NULL || si->ct02 == NULL ||
		    si->ct01->type >= SK_CONSTRAINT_LAST ||
		    si->ct02->type >= SK_CONSTRAINT_LAST)
			AG_FatalError("Bad args");
#endif
		Debug(sk, "+ COMPOSE_RING(%s,%s,%s)\n",
		    si->n[0]->name, si->n[1]->name, si->n[2]->name);
		break;
	}
	va_end(ap);

	TAILQ_INSERT_TAIL(&sk->insns, si, insns);
	return (si);
}

static int
SK_ComposePair(SK *_Nonnull sk, const SK_Insn *_Nonnull insn)
{
	SK_Node *n, *n1;
	SK_Constraint *ct = insn->ct01;
	int i, rv = -1;

	if (SK_FIXED(insn->n[0]) && SK_FIXED(insn->n[1])) {
		AG_SetError("Attempt to place two fixed entities");
		return (-1);
	}
	if (SK_FIXED(insn->n[0])) {
		n =  insn->n[1];
		n1 = insn->n[0];
	} else if (SK_FIXED(insn->n[1])) {
		n =  insn->n[0];
		n1 = insn->n[1];
	} else {
		if (SK_MOVED(insn->n[0])) {
			n =  insn->n[1];
			n1 = insn->n[0];
		} else if (SK_MOVED(insn->n[1])) {
			n =  insn->n[0];
			n1 = insn->n[1];
		} else {
			n =  insn->n[0];
			n1 = insn->n[1];
		}
	}
	
	Debug(sk, "Constructing: %s(%s) => %s\n",
	    n1->name, skConstraintNames[ct->type], n->name);

	for (i = 0; i < skConstraintPairFnCount; i++) {
		const SK_ConstraintPairFn *fn = &skConstraintPairFns[i];

		if (fn->ctType != ct->type) {
			continue;
		}
		if (SK_NodeOfClass(n,  fn->type1) &&
		    SK_NodeOfClass(n1, fn->type2)) {
			rv = fn->fn(ct, (void *)n, (void *)n1);
			break;
		} else
		if (SK_NodeOfClass(n1, fn->type1) &&
		    SK_NodeOfClass(n,  fn->type2)) {
			rv = fn->fn(ct, (void *)n1, (void *)n);
			break;
		}
	}
	if (i == skConstraintPairFnCount) {
		AG_SetError("Illegal case: %s(%s,%s)",
		    skConstraintNames[ct->type], n->ops->name, n1->ops->name);
		return (-1);
	}
	return (rv);
}

static int
SK_ComposeRing(SK *_Nonnull sk, const SK_Insn *_Nonnull insn)
{
	SK_Node *n = insn->n[0];
	SK_Node *n1 = insn->n[1];
	SK_Node *n2 = insn->n[2];
	SK_Constraint *ct1 = insn->ct01;
	SK_Constraint *ct2 = insn->ct02;
	int i, rv = 0;
	
	if (SK_FIXED(n)) {
		AG_SetError("Attempt to place fixed entity");
		return (-1);
	}
	
	Debug(sk, "Constructing: %s,%s(%s,%s) => %s\n",
	    n1->name, n2->name,
	    skConstraintNames[ct1->type],
	    skConstraintNames[ct2->type],
	    n->name);

	for (i = 0; i < skConstraintRingFnCount; i++) {
		const SK_ConstraintRingFn *fn = &skConstraintRingFns[i];

		if ((fn->ctType1 == ct1->type ||
		     fn->ctType1 == SK_CONSTRAINT_ANY) &&
		    (fn->ctType2 == ct2->type ||
		     fn->ctType2 == SK_CONSTRAINT_ANY) &&
		    SK_NodeOfClass(n, fn->type1) &&
		    SK_NodeOfClass(n1, fn->type2) &&
		    SK_NodeOfClass(n2, fn->type3)) {
			rv = fn->fn(n, ct1, n1, ct2, n2);
			if (rv == 0) {
				n->flags |= SK_NODE_KNOWN;
			}
			break;
		} else if
		   ((fn->ctType1 == ct2->type ||
		     fn->ctType1 == SK_CONSTRAINT_ANY) &&
		    (fn->ctType2 == ct1->type ||
		     fn->ctType2 == SK_CONSTRAINT_ANY) &&
		    SK_NodeOfClass(n, fn->type1) &&
		    SK_NodeOfClass(n2, fn->type2) &&
		    SK_NodeOfClass(n1, fn->type3)) {
			rv = fn->fn(n, ct2, n2, ct1, n1);
			if (rv == 0) {
				n->flags |= SK_NODE_KNOWN;
			}
			break;
		}
	}
	if (i == skConstraintRingFnCount) {
		AG_SetError("Illegal case: %s([%s:%s], [%s:%s])",
		    n->ops->name,
		    skConstraintNames[ct1->type],
		    n1->ops->name,
		    skConstraintNames[ct2->type],
		    n2->ops->name);
		return (-1);
	}
	return (rv);
}

int
SK_ExecInsn(SK *sk, const SK_Insn *insn)
{
	switch (insn->type) {
	case SK_COMPOSE_PAIR:
		return SK_ComposePair(sk, insn);
	case SK_COMPOSE_RING:
		return SK_ComposeRing(sk, insn);
	default:
		AG_SetError("Illegal instruction: 0x%x", insn->type);
		return (-1);
	}
}

void
SK_ClearProgramState(SK *sk)
{
	SK_Node *node;

	sk->nSolutions = 0;
	TAILQ_FOREACH(node, &sk->nodes, nodes)
		sk->flags &= ~(SK_NODE_KNOWN);
}

int
SK_ExecProgram(SK *sk)
{
	SK_Insn *si;
	Uint i = 0;

	TAILQ_FOREACH(si, &sk->insns, insns) {
		if (SK_ExecInsn(sk, si) == -1) {
			SK_SetStatus(sk, SK_INVALID, _("Error(%u): %s"),
			    i, AG_GetError());
			return (-1);
		}
		i++;
	}
	return (0);
}

void
SK_SetStatus(SK *sk, SK_Status status, const char *fmt, ...)
{
	va_list ap;

	sk->status = status;
	va_start(ap, fmt);
	AG_Vsnprintf(sk->statusText, sizeof(sk->statusText), fmt, ap);
	va_end(ap);
}

AG_ObjectClass skClass = {
	"SK",
	sizeof(SK),
	{ 0,0 },
	Init,
	Reset,
	NULL,		/* destroy */
	Load,
	Save,
	SK_Edit
};
