/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/edition.h>
#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/typesw.h>

#include "sk.h"
#include "sk_constraint.h"

#include <math.h>
#include <string.h>

const AG_ObjectOps skOps = {
	"SK",
	sizeof(SK),
	{ 0,0 },
	SK_Init,
	SK_Reinit,
	SK_Destroy,
	SK_Load,
	SK_Save,
#ifdef EDITION
	SK_Edit
#else
	NULL
#endif
};

const char *skConstraintNames[] = {
	N_("Incident"),
	N_("Distance"),
	N_("Angle"),
	N_("Perpendicular"),
	N_("Parallel"),
	N_("Concentric"),
};

SK_NodeOps **skElements = NULL;
Uint         skElementsCnt = 0;

void
SK_NodeRegister(SK_NodeOps *nops)
{
	skElements = Realloc(skElements,(skElementsCnt+1)*sizeof(SK_NodeOps *));
	skElements[skElementsCnt] = nops;
	skElementsCnt++;
}

static int
SK_NodeOfClassGeneral(SK_Node *node, const char *cn)
{
	char cname[SK_TYPE_NAME_MAX], *cp, *c;
	char nname[SK_TYPE_NAME_MAX], *np, *s;

	strlcpy(cname, cn, sizeof(cname));
	strlcpy(nname, node->ops->name, sizeof(nname));
	cp = cname;
	np = nname;
	while ((c = strsep(&cp, ":")) != NULL &&
	       (s = strsep(&np, ":")) != NULL) {
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

#ifdef DEBUG
	if (cname[0] == '*' && cname[1] == '\0')
		fatal("Use SK_FOREACH_NODE()");
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

/* Register the SK classes with the Agar object system. */
int
SK_InitEngine(void)
{
	AG_RegisterType(&skOps, DRAWING_ICON);

	SK_NodeRegister(&skDummyOps);
	SK_NodeRegister(&skPointOps);
	SK_NodeRegister(&skLineOps);
	SK_NodeRegister(&skCircleOps);

	SK_NodeRegister(&skDimensionOps);
	return (0);
}

void
SK_DestroyEngine(void)
{
}

SK *
SK_New(void *parent, const char *name)
{
	SK *sk;

	sk = Malloc(sizeof(SK), M_SG);
	SK_Init(sk, name);
	AG_ObjectAttach(parent, sk);
	return (sk);
}

static void
SK_InitRoot(SK *sk)
{
	sk->root = Malloc(sizeof(SK_Point), M_SG);
	SK_PointInit(sk->root, 0);
	SK_PointSize(sk->root, 3.0);
	SK_PointColor(sk->root, SG_ColorRGB(100.0, 100.0, 0.0));
	SKNODE(sk->root)->sk = sk;
	TAILQ_INSERT_TAIL(&sk->nodes, SKNODE(sk->root), nodes);
}

#ifdef EDITION
static void
PostEditorLoad(AG_Event *event)
{
	SK *sk = AG_SELF();

	SK_Update(sk);
}
#endif /* EDITION */

void
SK_Init(void *obj, const char *name)
{
	SK *sk = obj;
	
	if (skElementsCnt == 0) {
		if (SK_InitEngine() == -1)
			fatal("SK: %s", AG_GetError());
	}

	AG_ObjectInit(sk, name, &skOps);
	OBJECT(sk)->flags |= AG_OBJECT_REOPEN_ONLOAD;
	sk->flags = 0;
	AG_MutexInitRecursive(&sk->lock);
	SK_InitCluster(&sk->ctGraph, 0);
	TAILQ_INIT(&sk->nodes);
	TAILQ_INIT(&sk->clusters);
	TAILQ_INIT(&sk->insns);
	sk->uLen = AG_FindUnit("mm");

#ifdef EDITION
	AG_SetEvent(sk, "edit-post-load", PostEditorLoad, NULL);
#endif
	SK_InitRoot(sk);
}

void
SK_Update(SK *sk)
{
	SK_Insn *si;

	if (SK_Solve(sk) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Failed to solve sketch: %s"),
		    AG_GetError());
		return;
	}
#if 0
	TAILQ_FOREACH(si, &sk->insns, insns) {
		if (SK_ExecInsn(sk, si) == -1) {
			AG_TextMsg(AG_MSG_ERROR, _("Instruction failed: %s"),
			    AG_GetError());
			return;
		}
	}
#endif
}

/* Allocate a new node name. */
Uint32
SK_GenNodeName(SK *sk, const char *type)
{
	Uint32 name = 1;

	while (SK_FindNode(sk, name, type) != NULL) {
		if (++name >= SK_NAME_MAX)
			fatal("Out of node names");
	}
	return (name);
}

/* Return string representation of a node name. */
char *
SK_NodeName(void *p)
{
	SK_Node *node = p;
	char *s;

	asprintf(&s, "%s%u", node->ops->name, node->name);
	return (s);
}

/* Return string representation of a node name in fixed buffer. */
char *
SK_NodeNameCopy(void *p, char *buf, size_t buf_len)
{
	SK_Node *node = p;

	snprintf(buf, buf_len, "%s%u", node->ops->name, node->name);
	return (buf);
}

SG_Color
SK_NodeColor(void *p, const SG_Color *cOrig)
{
	SK_Node *node = p;
	SG_Color c = *cOrig;

	if (node->flags & SK_NODE_MOUSEOVER) {
		c.b = MIN((cOrig->b + 1.0)/2.0,1.0);
	}
	if (SKNODE_SELECTED(node)) {
		c.g = MIN((c.g + 1.5)/2.0,1.0);
	}
	return c;
}

void
SK_NodeInit(void *np, const void *ops, Uint32 name, Uint flags)
{
	SK_Node *n = np;

	n->name = name;
	n->flags = flags;
	n->ops = (const SK_NodeOps *)ops;
	n->sk = NULL;
	n->pNode = NULL;
	n->nRefs = 0;
	n->refNodes = Malloc(sizeof(SK_Node *), M_SG);
	n->nRefNodes = 0;
	n->cons = Malloc(sizeof(SK_Constraint *), M_SG);
	n->nCons = 0;
	SG_MatrixIdentityv(&n->T);
	TAILQ_INIT(&n->cnodes);
}

/* Free a node and detach/free any child nodes. */
static void
SK_FreeNode(SK *sk, SK_Node *node)
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
	Free(node->refNodes, M_SG);
	Free(node->cons, M_SG);
	Free(node, M_SG);
}

void
SK_Reinit(void *obj)
{
	SK *sk = obj;

	if (sk->root != NULL) {
		SK_FreeNode(sk, SKNODE(sk->root));
		sk->root = NULL;
	}
	TAILQ_INIT(&sk->nodes);
	SK_InitRoot(sk);

	SK_FreeCluster(&sk->ctGraph);
	SK_FreeClusters(sk);
	SK_FreeInsns(sk);
}

void
SK_Destroy(void *obj)
{
}

static int
SK_NodeSaveData(SK *sk, SK_Node *node, AG_Netbuf *buf)
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
SK_SaveNodeGeneric(SK *sk, SK_Node *node, AG_Netbuf *buf)
{
	SK_Node *cnode;
	Uint32 ncnodes;
	off_t bsize_offs, ncnodes_offs;

	/* Save generic node information. */
	AG_WriteString(buf, node->ops->name);

	bsize_offs = AG_NetbufTell(buf);
	AG_NetbufSeek(buf, sizeof(Uint32), SEEK_CUR);

	AG_WriteUint32(buf, node->name);
	AG_WriteUint16(buf, (Uint16)node->flags);
	SG_WriteMatrix(buf, &node->T);

	/* Save the child nodes recursively. */
	ncnodes_offs = AG_NetbufTell(buf);
	ncnodes = 0;
	AG_NetbufSeek(buf, sizeof(Uint32), SEEK_CUR);
	TAILQ_FOREACH(cnode, &node->cnodes, sknodes) {
		if (SK_SaveNodeGeneric(sk, cnode, buf) == -1) {
			return (-1);
		}
		ncnodes++;
	}
	AG_PwriteUint32(buf, ncnodes, ncnodes_offs);

	/* Save the total block size to allow the loader to skip. */
	AG_PwriteUint32(buf, AG_NetbufTell(buf)-bsize_offs, bsize_offs);
	return (0);
}

int
SK_Save(void *obj, AG_Netbuf *buf)
{
	SK *sk = obj;
	SK_Node *node;
	SK_Constraint *ct;
	Uint32 count;
	off_t offs;
	
	AG_WriteObjectVersion(buf, sk);
	AG_MutexLock(&sk->lock);
	
	AG_WriteUint32(buf, sk->flags);
	AG_WriteString(buf, sk->uLen->key);

	/* Save the generic part of all nodes. */
	if (SK_SaveNodeGeneric(sk, SKNODE(sk->root), buf) == -1)
		goto fail;

	/* Save the data part of all nodes. */
	if (SK_NodeSaveData(sk, SKNODE(sk->root), buf) == -1)
		goto fail;

	/* Save the graph of geometric constraints. */
	offs = AG_NetbufTell(buf);
	count = 0;
	AG_NetbufSeek(buf, sizeof(Uint32), SEEK_CUR);
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		AG_WriteUint32(buf, (Uint32)ct->type);
		SK_WriteRef(buf, ct->n1);
		SK_WriteRef(buf, ct->n2);
		switch (ct->type) {
		case SK_DISTANCE:
			SG_WriteReal(buf, ct->ct_distance);
			break;
		case SK_ANGLE:
			SG_WriteReal(buf, ct->ct_angle);
			break;
		default:
			break;
		}
		count++;
	}
	AG_PwriteUint32(buf, count, offs);
	AG_MutexUnlock(&sk->lock);
	return (0);
fail:
	AG_MutexUnlock(&sk->lock);
	return (-1);
}

/* Load the data part of a node. */
static int
SK_LoadNodeData(SK *sk, SK_Node *node, AG_Netbuf *buf)
{
	char nodeName[SK_NODE_NAME_MAX];
	SK_Node *chldNode;

	TAILQ_FOREACH(chldNode, &node->cnodes, sknodes) {
		if (SK_LoadNodeData(sk, chldNode, buf) == -1)
			return (-1);
	}
	if (node->ops->load != NULL &&
	    node->ops->load(sk, node, buf) == -1) {
		AG_SetError("%s: %s",
		    SK_NodeNameCopy(node, nodeName, sizeof(nodeName)),
		    AG_GetError());
		return (-1);
	}
	return (0);
}

/* Load the generic part of a node. */
static int
SK_LoadNodeGeneric(SK *sk, SK_Node **rnode, AG_Netbuf *buf)
{
	char type[SK_TYPE_NAME_MAX];
	SK_Node *node;
	Uint32 bsize, nchildren, j;
	Uint32 name;
	int i;

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
			AG_NetbufSeek(buf, bsize, SEEK_CUR);
			*rnode = NULL;
			return (0);
		} else {
			AG_SetError("Unimplemented node class: %s (%luB)",
			    type, (Ulong)bsize);
			return (-1);
		}
	}
	name = AG_ReadUint32(buf);
	node = Malloc(skElements[i]->size, M_SG);
	skElements[i]->init(node, name);
	node->flags = (Uint)AG_ReadUint16(buf);
	node->sk = sk;

	SG_ReadMatrixv(buf, &node->T);
	
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

int
SK_Load(void *obj, AG_Netbuf *buf)
{
	char unitKey[AG_UNIT_KEY_MAX];
	SK *sk = obj;
	SK_Node *node;
	Uint32 i, count;
	int rv;
	SK_Constraint *ct;
	
	if (AG_ReadObjectVersion(buf, sk, NULL) == -1)
		return (-1);

	AG_MutexLock(&sk->lock);
	sk->flags = (Uint)AG_ReadUint32(buf);
	AG_CopyString(unitKey, buf, sizeof(unitKey));
	if ((sk->uLen = AG_FindUnit(unitKey)) == NULL) {
		AG_MutexUnlock(&sk->lock);
		return (-1);
	}

	/* Free the existing root node. */
	if (sk->root != NULL) {
		SK_FreeNode(sk, SKNODE(sk->root));
		sk->root = NULL;
	}
	TAILQ_INIT(&sk->nodes);

	/*
	 * Load the generic part of all nodes. We need to load the data
	 * afterwards to properly resolve interdependencies.
	 */
	if (SK_LoadNodeGeneric(sk, (SK_Node **)&sk->root, buf) == -1) {
		goto fail;
	}
	TAILQ_INSERT_HEAD(&sk->nodes, SKNODE(sk->root), nodes);

	/* Load the data part of all nodes. */
	if (SK_LoadNodeData(sk, SKNODE(sk->root), buf) == -1)
		goto fail;

	/* Load the geometric constraint data. */
	count = AG_ReadUint32(buf);
	for (i = 0; i < count; i++) {
		ct = AG_Malloc(sizeof(SK_Constraint), M_SG);
		ct->type = (enum sk_constraint_type)AG_ReadUint32(buf);
		ct->n1 = SK_ReadRef(buf, sk, NULL);
		ct->n2 = SK_ReadRef(buf, sk, NULL);
		switch (ct->type) {
		case SK_DISTANCE:
			ct->ct_distance = SG_ReadReal(buf);
			break;
		case SK_ANGLE:
			ct->ct_angle = SG_ReadReal(buf);
			break;
		default:
			break;
		}
		SK_NodeAddConstraint(ct->n1, ct);
		SK_NodeAddConstraint(ct->n2, ct);
		TAILQ_INSERT_TAIL(&sk->ctGraph.edges, ct, constraints);
	}
	AG_MutexUnlock(&sk->lock);
	return (0);
fail:
	AG_MutexUnlock(&sk->lock);
	SK_Reinit(sk);
	return (-1);
}

/*
 * Compute the product of the transform matrices of the given node and its
 * parents in order. T is initialized to identity.
 */
void
SK_GetNodeTransform(void *p, SG_Matrix *T)
{
	SK_Node *node = p;
	SK_Node *cnode = node;
	TAILQ_HEAD(,sk_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

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
	SG_MatrixIdentityv(T);
	TAILQ_FOREACH(cnode, &rnodes, rnodes)
		SG_MatrixMultv(T, &cnode->T);
}

/*
 * Compute the product of the inverse transform matrices of the given node
 * and its parents.
 */
void
SK_GetNodeTransformInverse(void *p, SG_Matrix *T)
{
	SK_Node *node = p;
	SK_Node *cnode = node;
	TAILQ_HEAD(,sk_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

	SG_MatrixIdentityv(T);

	while (cnode != NULL) {
		TAILQ_INSERT_TAIL(&rnodes, cnode, rnodes);
		if (cnode->pNode == NULL) {
			break;
		}
		cnode = cnode->pNode;
	}
	TAILQ_FOREACH(cnode, &rnodes, rnodes) {
		SG_Matrix Tinv;

		Tinv = SG_MatrixInvertCramerp(&cnode->T);
		SG_MatrixMultv(T, &Tinv);
	}
}

/* Return the absolute position of a node. */
SG_Vector
SK_NodeCoords(void *p)
{
	SK_Node *node = p;
	SG_Matrix T;
	SG_Vector v = SG_0;
	
	SK_GetNodeTransform(node, &T);
	SG_MatrixMultVectorv(&v, &T);
	return (v);
}

void
SK_NodeRedraw(void *p, SK_View *skv)
{
	SK_Node *node = p;

	if (node->ops->redraw != NULL)
		node->ops->redraw(node, skv);
}

/* Return the orientation vector of a node. */
SG_Vector
SK_NodeDir(void *p)
{
	SK_Node *node = p;
	SG_Matrix T;
	SG_Vector v = SG_K;				/* Convention */
	
	SK_GetNodeTransform(node, &T);
	SG_MatrixMultVectorv(&v, &T);
	return (v);
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
	int i;

	if (node == other)
		return;

	for (i = 0; i < node->nRefNodes; i++) {
		if (node->refNodes[i] != other) {
			continue;
		}
		if (i+1 < node->nRefNodes) {
			memmove(&node->refNodes[i],
			        &node->refNodes[i+1],
				(node->nRefNodes-1)*sizeof(SK_Node *));
		}
		node->nRefNodes--;
		other->nRefs--;
		break;
	}
}

void
SK_NodeAddConstraint(void *pNode, SK_Constraint *ct)
{
	SK_Node *node = pNode;
	int i;

	for (i = 0; i < node->nCons; i++) {
		if (node->cons[i] == ct)
			return;
	}
	node->cons = Realloc(node->cons, (node->nCons+1) *
	                                 sizeof(SK_Constraint *));
	node->cons[node->nCons++] = ct;
}

void
SK_NodeDelConstraint(void *pNode, SK_Constraint *ct)
{
	SK_Node *node = pNode;
	int i;

	for (i = 0; i < node->nCons; i++) {
		if (node->cons[i] != ct) {
			continue;
		}
		if (i+1 < node->nCons) {
			memmove(&node->cons[i],
			        &node->cons[i+1],
				(node->nCons-1)*sizeof(SK_Constraint *));
		}
		node->nCons--;
		break;
	}
}

/* Search a node by name in a sketch. */
/* XXX use a hash table */
void *
SK_FindNode(SK *sk, Uint32 name, const char *type)
{
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->name == name &&
		    strcmp(node->ops->name, type) == 0)
			return (node);
	}
	AG_SetError("No such node: %u", name);
	return (NULL);
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
	SK_Node *subnode, *subnodeNext;
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

/* Create a new node instance in the sketch. */
void *
SK_NodeAdd(void *pNode, const SK_NodeOps *ops, Uint32 name, Uint flags)
{
	SK_Node *n;

	n = Malloc(ops->size, M_SG);
	SK_NodeInit(n, ops, 0, flags);
	SK_NodeAttach(pNode, n);
	return (n);
}

/* Detach and free a node (and its children) from the sketch. */
int
SK_NodeDel(void *p)
{
	SK_Node *node = p;
	SK *sk = node->sk;

	if (node == SKNODE(sk->root)) {
		AG_SetError("Cannot delete root node");
		return (-1);
	}
	if (node->nRefs > 0) {
		AG_SetError("Node is being referenced");
		return (-1);
	}
	SK_NodeDetach(node->pNode, node);
	SK_FreeNode(sk, node);
	return (0);
}

/* Render a graphical node to the display (with transformations). */
void
SK_RenderNode(SK *sk, SK_Node *node, SK_View *view)
{
	SG_Matrix Tsave;
	SG_Matrix Tt;
	SK_Node *cnode;

	SG_GetMatrixGL(GL_MODELVIEW_MATRIX, &Tsave);
	Tt = SG_MatrixTransposep(&node->T);
	SG_MultMatrixGL(&Tt);
	if (node->ops->draw_relative != NULL) {
		node->ops->draw_relative(node, view);
	}
	TAILQ_FOREACH(cnode, &node->cnodes, sknodes) {
		SK_RenderNode(sk, cnode, view);
	}
	SG_LoadMatrixGL(&Tsave);
}

/* Render a graphical node to the display (without transformations). */
void
SK_RenderAbsolute(SK *sk, SK_View *view)
{
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->ops->draw_absolute != NULL)
			node->ops->draw_absolute(node, view);
	}
}

void
SK_WriteRef(AG_Netbuf *buf, void *pNode)
{
	SK_Node *node = pNode;

	AG_WriteString(buf, node->ops->name);
	AG_WriteUint32(buf, node->name);
}

void *
SK_ReadRef(AG_Netbuf *buf, SK *sk, const char *expType)
{
	char rType[SK_TYPE_NAME_MAX];

	AG_CopyString(rType, buf, sizeof(rType));
	if (expType != NULL) {
		if (strcmp(rType, expType) != 0) {
			fatal("Unexpected reference type: `%s' (expecting %s)",
			    rType, expType);
		}
	}
	return (SK_FindNode(sk, AG_ReadUint32(buf), rType));
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
SK_ProximitySearch(SK *sk, const char *type, SG_Vector *v, SG_Vector *vC,
    void *nodeIgnore)
{
	SK_Node **nodes, *node, *nClosest = NULL;
	SG_Real rClosest = HUGE_VAL, p;
	SG_Vector vClosest = SG_VECTOR2(HUGE_VAL,HUGE_VAL);
	Uint nNodes;

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
SK_FindCluster(SK *sk, Uint32 name)
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
			fatal("Out of cluster names");
	}
	return (name);
}

void
SK_InitCluster(SK_Cluster *cl, Uint32 name)
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
		Free(ct, M_SG);
	}
}

void
SK_FreeClusters(SK *sk)
{
	SK_Cluster *cl;

	while ((cl = TAILQ_FIRST(&sk->clusters)) != NULL) {
		TAILQ_REMOVE(&sk->clusters, cl, clusters);
		SK_FreeCluster(cl);
		Free(cl, M_SG);
	}
}

void
SK_FreeInsns(SK *sk)
{
	SK_Insn *si;

	while ((si = TAILQ_FIRST(&sk->insns)) != NULL) {
		TAILQ_REMOVE(&sk->insns, si, insns);
		Free(si, M_SG);
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
	ct = Malloc(sizeof(SK_Constraint), M_SG);
	ct->type = type;
	ct->n1 = node1;
	ct->n2 = node2;

	va_start(ap, type);
	switch (type) {
	case SK_DISTANCE:
		ct->ct_distance = (SG_Real)va_arg(ap, double);
		break;
	case SK_ANGLE:
		ct->ct_angle = (SG_Real)va_arg(ap, double);
		break;
	default:
		break;
	}
	va_end(ap);

	TAILQ_INSERT_TAIL(&cl->edges, ct, constraints);
	return (ct);
}

/* Duplicate a constraint. */
SK_Constraint *
SK_DupConstraint(const SK_Constraint *ct1)
{
	SK_Constraint *ct2;

	ct2 = Malloc(sizeof(SK_Constraint), M_SG);
	ct2->type = ct1->type;
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
	Free(ct, M_SG);
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
	TAILQ_REMOVE(&cl->edges, ct, constraints);
	Free(ct, M_SG);
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
			return (ct1->ct_distance - ct2->ct_distance);
		case SK_ANGLE:
			return (ct1->ct_angle - ct2->ct_angle);
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
	SK_Constraint *ct, *ctOrig;
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

	si = Malloc(sizeof(SK_Insn), M_SG);
	si->type = type;

	va_start(ap, type);
	switch (type) {
	case SK_COMPOSE_PAIR:
		si->n[0] = va_arg(ap, void *);
		si->n[1] = va_arg(ap, void *);
		si->ct01 = va_arg(ap, void *);
#ifdef DEBUG
		if (si->n[0] == NULL || si->n[1] == NULL || si->ct01 == NULL ||
		    si->ct01->type >= SK_CONSTRAINT_LAST)
			fatal("Bad args");
#endif
		break;
	case SK_COMPOSE_RING:
		si->n[0] = va_arg(ap, void *);
		si->n[1] = va_arg(ap, void *);
		si->n[2] = va_arg(ap, void *);
		si->ct01 = va_arg(ap, void *);
		si->ct02 = va_arg(ap, void *);
#ifdef DEBUG
		if (si->n[0] == NULL || si->n[1] == NULL || si->n[2] == NULL ||
		    si->ct01 == NULL || si->ct02 == NULL ||
		    si->ct01->type >= SK_CONSTRAINT_LAST ||
		    si->ct02->type >= SK_CONSTRAINT_LAST)
			fatal("Bad args");
#endif
		break;
	}
	va_end(ap);

	TAILQ_INSERT_TAIL(&sk->insns, si, insns);
	return (si);
}

static int
SK_ComposePair(SK *sk, const SK_Insn *insn)
{
	SK_Node *n = insn->n[0];
	SK_Node *n1 = insn->n[1];
	SK_Constraint *ct = insn->ct01;
	SG_Vector v;
	int i, rv = -1;

	dprintf("ComposePair: %s([%s:%s])\n", SK_NodeName(n),
	    skConstraintNames[ct->type], SK_NodeName(n1));

	for (i = 0; i < skConstraintPairFnCount; i++) {
		const SK_ConstraintPairFn *fn = &skConstraintPairFns[i];

		if (fn->ctType == ct->type &&
		    SK_NodeOfClass(n, fn->type1) &&
		    SK_NodeOfClass(n1, fn->type2)) {
			rv = fn->fn(ct, (void *)n, (void *)n1);
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
SK_ComposeRing(SK *sk, const SK_Insn *insn)
{
	SK_Node *n = insn->n[0];
	SK_Node *n1 = insn->n[1];
	SK_Node *n2 = insn->n[2];
	SK_Constraint *ct1 = insn->ct01;
	SK_Constraint *ct2 = insn->ct02;
	int i;
	int rv = 0;

	dprintf("ComposeRing: %s([%s:%s], [%s:%s])\n", SK_NodeName(n),
	    skConstraintNames[ct1->type], SK_NodeName(n1),
	    skConstraintNames[ct2->type], SK_NodeName(n2));
	
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

#endif /* HAVE_OPENGL */
