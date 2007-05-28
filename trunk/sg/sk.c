/*
 * Copyright (c) 2006-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>
#include <agar/core/objmgr.h>
#include <agar/core/typesw.h>
#include <agar/gui/gui.h>

#include "sk.h"

#include <GL/gl.h>
#include <GL/glu.h>

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
SK_NodeOfClassGen(SK_Node *node, const char *cn)
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

int
SK_NodeOfClass(SK_Node *node, const char *cname)
{
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
	return (SK_NodeOfClassGen(node, cname));	/* General case */
}

int
SK_InitEngine(void)
{
	AG_RegisterType(&skOps, DRAWING_ICON);

	SK_NodeRegister(&skDummyOps);
#if 0
	SK_NodeRegister(&skPointOps);
	SK_NodeRegister(&skLineOps);
	SK_NodeRegister(&skCircleOps);
	SK_NodeRegister(&skTextOps);
#endif
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

void
SK_Init(void *obj, const char *name)
{
	SK *sk = obj;
	
	if (skElementsCnt == 0) {
		if (SK_InitEngine() == -1)
			fatal("SK: %s", AG_GetError());
	}

	AG_ObjectInit(sk, name, &skOps);
	AGOBJECT(sk)->flags |= AG_OBJECT_REOPEN_ONLOAD;
	sk->flags = 0;
	TAILQ_INIT(&sk->nodes);
	AG_MutexInit(&sk->lock);

	sk->root = Malloc(sizeof(SK_Point), M_SG);
	SK_PointInit(sk->root);
	SK_PointSize(sk->root, 3.0);
	SK_PointColor(sk->root, SG_ColorRGB(0.0, 255.0, 0.0));
	SKNODE(sk->root)->sk = sk;
	TAILQ_INSERT_TAIL(&sk->nodes, SKNODE(sk->root), nodes);
}

void
SK_NodeInit(void *np, const void *ops, Uint flags)
{
	SK_Node *n = np;

	n->flags = flags;
	n->ops = (const SK_NodeOps *)ops;
	n->sk = NULL;
	n->pNode = NULL;
	n->nrefs = 0;
	n->refnodes = Malloc(sizeof(SK_Node *), M_SG);
	n->nrefnodes = 0;
	SG_MatrixIdentityv(&n->T);
	TAILQ_INIT(&n->cnodes);
}

static void
SK_FreeNode(SK *sk, SK_Node *node)
{
	SK_Node *n1, *n2;

	for (n1 = TAILQ_FIRST(&node->cnodes);
	     n1 != TAILQ_END(&node->cnodes);
	     n1 = n2) {
		n2 = TAILQ_NEXT(n1, sknodes);
		if (n1->ops->destroy != NULL) {
			n1->ops->destroy(n1);
		}
		Free(n1, M_SG);
	}
	TAILQ_INIT(&node->cnodes);
}

void
SK_Reinit(void *obj)
{
	SK *sk = obj;

	SK_FreeNode(sk, SKNODE(sk->root));
	TAILQ_INIT(&sk->nodes);
}

void
SK_Destroy(void *obj)
{
}

int
SK_Save(void *obj, AG_Netbuf *buf)
{
	SK *sk = obj;
	SK_Node *node;
	int rv = 0;
	
	AG_WriteVersion(buf, "SK", &skOps.ver);
	AG_MutexLock(&sk->lock);
	AG_WriteUint32(buf, sk->flags);
	rv = SK_NodeSave(sk, SKNODE(sk->root), buf);
	AG_MutexUnlock(&sk->lock);
	return (rv);
}

int
SK_NodeSave(SK *sk, SK_Node *node, AG_Netbuf *buf)
{
	SK_Node *cnode;
	Uint32 ncnodes;
	off_t bsize_offs, ncnodes_offs;

	/* Save generic node information. */
	AG_WriteString(buf, node->ops->name);
	bsize_offs = AG_NetbufTell(buf);
	AG_NetbufSeek(buf, sizeof(Uint32), SEEK_CUR);

	AG_WriteUint16(buf, (Uint16)node->flags);
	SG_WriteMatrix(buf, &node->T);

	/* Save information specific to this type of node. */
	if (node->ops->save != NULL &&
	    node->ops->save(node, buf) == -1)
		return (-1);

	/* Save the child nodes recursively. */
	ncnodes_offs = AG_NetbufTell(buf);
	ncnodes = 0;
	AG_NetbufSeek(buf, sizeof(Uint32), SEEK_CUR);
	TAILQ_FOREACH(cnode, &node->cnodes, sknodes) {
		if (SK_NodeSave(sk, cnode, buf) == -1) {
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
SK_Load(void *obj, AG_Netbuf *buf)
{
	SK *sk = obj;
	SK_Node *node;
	int rv;
	
	if (AG_ReadVersion(buf, "SK", &skOps.ver, NULL) == -1)
		return (-1);

	AG_MutexLock(&sk->lock);
	sk->flags = (Uint)AG_ReadUint32(buf);
	rv = SK_NodeLoad(sk, (SK_Node **)&sk->root, buf);
	AG_MutexUnlock(&sk->lock);
	return ((rv == 0 && sk->root != NULL) ? 0 : -1);
}

int
SK_NodeLoad(SK *sk, SK_Node **rnode, AG_Netbuf *buf)
{
	char type[SK_TYPE_NAME_MAX];
	SK_NodeOps *nops;
	SK_Node *node;
	Uint32 bsize, ncnodes;
	int i;

	/* Load generic node information. */
	AG_CopyString(type, buf, sizeof(type));
	bsize = AG_ReadUint32(buf);
	for (i = 0; i < skElementsCnt; i++) {
		if (strcmp(skElements[i]->name, type) == 0)
			break;
	}
	if (i == skElementsCnt) {
		fprintf(stderr, "%s: skipping node (%s/%luB)\n",
		    AGOBJECT(sk)->name, type, (Ulong)bsize);
		AG_NetbufSeek(buf, bsize, SEEK_CUR);
		*rnode = NULL;
		return (0);
	}
	nops = skElements[i];
	node = Malloc(nops->size, M_SG);
	nops->init(node);
	node->sk = sk;

	node->flags = (Uint)AG_ReadUint16(buf);
	dprintf("node: type <%s>, len %lu\n", type, (Ulong)bsize);
	SG_ReadMatrixv(buf, &node->T);

	/* Load information specific to this type of node. */
	if (node->ops->load != NULL &&
	    node->ops->load(node, buf) == -1) {
		*rnode = NULL;
		return (-1);
	}

	/* Load the child nodes recursively. */
	ncnodes = AG_ReadUint32(buf);
	dprintf("node: %u child nodes\n", ncnodes);
	while (ncnodes-- > 0) {
		SK_Node *cnode;

		if (SK_NodeLoad(sk, &cnode, buf) == -1) {
			*rnode = NULL;
			return (-1);
		}
		if (cnode != NULL)
			SK_NodeAttach(node, cnode);
	}
	*rnode = node;
	return (0);
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
	SLIST_HEAD(,sk_node) rnodes = SLIST_HEAD_INITIALIZER(&rnodes);

	/*
	 * Build a list of parent nodes and multiply their matrices in order
	 * (ugly but faster than computing the product of their inverses).
	 */
	while (cnode != NULL) {
		SLIST_INSERT_HEAD(&rnodes, cnode, rnodes);
		if (cnode->pNode == NULL) {
			break;
		}
		cnode = cnode->pNode;
	}
	SG_MatrixIdentityv(T);
	SLIST_FOREACH(cnode, &rnodes, rnodes)
		SG_MatrixMultv(T, &cnode->T);
}

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

void
SK_NodeAddReference(void *pNode, void *pOther)
{
	SK_Node *node = pNode;
	SK_Node *other = pOther;

	if (node == other)
		return;
	
	node->refnodes = Realloc(node->refnodes,
	                         (node->nrefnodes+1)*sizeof(SK_Node *));
	node->refnodes[node->nrefnodes++] = other;
	other->nrefs++;
}

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

void
SK_NodeDetach(void *ppNode, void *pcNode)
{
	SK_Node *pNode = ppNode;
	SK_Node *cNode = pcNode;

	TAILQ_REMOVE(&pNode->cnodes, cNode, sknodes);
	TAILQ_REMOVE(&pNode->sk->nodes, cNode, nodes);
	cNode->sk = NULL;
	cNode->pNode = NULL;
}

void *
SK_NodeAdd(void *pNode, const SK_NodeOps *ops, Uint flags)
{
	SK_Node *n;

	n = Malloc(ops->size, M_SG);
	SK_NodeInit(n, ops, flags);
	SK_NodeAttach(pNode, n);
	return (n);
}

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

void
SK_RenderAbsolute(SK *sk, SK_View *view)
{
	SK_Node *node;

	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->ops->draw_absolute != NULL)
			node->ops->draw_absolute(node, view);
	}
}

#endif /* HAVE_OPENGL */
