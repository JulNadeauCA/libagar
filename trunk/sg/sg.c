/*
 * Copyright (c) 2005-2007 Hypertriton, Inc.
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

#include "sg.h"

#if 0
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

#include <math.h>
#include <string.h>

const AG_ObjectOps sgOps = {
	"SG",
	sizeof(SG),
	{ 1,0 },
	SG_Init,
	SG_Reinit,
	SG_Destroy,
	SG_Load,
	SG_Save,
	SG_Edit
};

SG_NodeOps **sgElements = NULL;
Uint         sgElementsCnt = 0;

void
SG_NodeRegister(SG_NodeOps *nops)
{
	sgElements = Realloc(sgElements,(sgElementsCnt+1)*sizeof(SG_NodeOps *));
	sgElements[sgElementsCnt] = nops;
	sgElementsCnt++;
}

static int
SG_NodeOfClassGen(SG_Node *node, const char *cn)
{
	char cname[SG_CLASS_MAX], *cp, *c;
	char nname[SG_CLASS_MAX], *np, *s;

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
SG_NodeOfClass(SG_Node *node, const char *cname)
{
	const char *c;

#ifdef DEBUG
	if (cname[0] == '*' && cname[1] == '\0')
		fatal("Use SG_FOREACH_NODE()");
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
	return (SG_NodeOfClassGen(node, cname));	/* General case */
}

int
SG_InitEngine(void)
{
	extern AG_ObjectOps sgMaterialOps;

	AG_RegisterType(&sgOps, MAP_ICON);
	AG_RegisterType(&sgMaterialOps, RG_TILING_ICON);

	SG_NodeRegister(&sgDummyOps);
	SG_NodeRegister(&sgPointOps);
	SG_NodeRegister(&sgSphereOps);
	SG_NodeRegister(&sgBoxOps);
	SG_NodeRegister(&sgCameraOps);
	SG_NodeRegister(&sgPlaneObjOps);
	SG_NodeRegister(&sgSolidOps);
	SG_NodeRegister(&sgObjectOps);
#if 0
	AG_AtExitFunc(SG_DestroyEngine);
#endif
	return (0);
}

void
SG_DestroyEngine(void)
{
}

SG *
SG_New(void *parent, const char *name)
{
	SG *sg;

	sg = Malloc(sizeof(SG), M_SG);
	SG_Init(sg, name);
	AG_ObjectAttach(parent, sg);
	return (sg);
}

static void
SG_Attached(AG_Event *event)
{
	SG *sg = AG_SELF();
	SG_Camera *cam;
	SG_Light *lt0;

	cam = SG_CameraNew(sg->root, "Camera0");
	SG_Translate3(cam, 0.0, 0.0, -10.0);
	
	lt0 = SG_LightNew(sg->root, "Light0");
	SG_Translate3(lt0, 20.0, 20.0, 20.0);
}

void
SG_Init(void *obj, const char *name)
{
	SG *sg = obj;
	
	if (sgElementsCnt == 0) {
		if (SG_InitEngine() == -1 ||
		    SK_InitEngine() == -1)
			fatal("SG: %s", AG_GetError());
	}

	AG_ObjectInit(sg, name, &sgOps);
	AGOBJECT(sg)->flags |= AG_OBJECT_REOPEN_ONLOAD;
	sg->flags = 0;
	TAILQ_INIT(&sg->nodes);
	AG_MutexInit(&sg->lock);

	sg->root = Malloc(sizeof(SG_Point), M_SG);
	SG_PointInit(sg->root, "_root");
	SG_PointSize(sg->root, 3.0);
	SG_PointColor(sg->root, SG_ColorRGB(0.0, 255.0, 0.0));
	SGNODE(sg->root)->sg = sg;
	TAILQ_INSERT_TAIL(&sg->nodes, SGNODE(sg->root), nodes);

	sg->tess = gluNewTess();
	AG_SetEvent(sg, "attached", SG_Attached, NULL);

//	if ((sgCtxCg = cgCreateContext()) == NULL)
//		fatal("cgCreateContext failed");
}

void
SG_NodeInit(void *np, const char *name, const void *ops, Uint flags)
{
	SG_Node *n = np;

	strlcpy(n->name, name, sizeof(n->name));
	n->flags = flags;
	n->ops = (const SG_NodeOps *)ops;
	n->sg = NULL;
	n->pNode = NULL;
	SG_MatrixIdentityv(&n->T);
	TAILQ_INIT(&n->cnodes);
}

static void
SG_FreeNode(SG *sg, SG_Node *node)
{
	SG_Node *n1, *n2;

	for (n1 = TAILQ_FIRST(&node->cnodes);
	     n1 != TAILQ_END(&node->cnodes);
	     n1 = n2) {
		n2 = TAILQ_NEXT(n1, sgnodes);
		if (n1->ops->destroy != NULL) {
			n1->ops->destroy(n1);
		}
		Free(n1, M_SG);
	}
	if (node->ops->destroy != NULL) {
		node->ops->destroy(node);
	}
	Free(node, M_SG);
}

void
SG_Reinit(void *obj)
{
	SG *sg = obj;

	SG_FreeNode(sg, SGNODE(sg->root));
	TAILQ_INIT(&sg->nodes);
}

void
SG_Destroy(void *obj)
{
	SG *sg = obj;

	gluDeleteTess(sg->tess);
}

int
SG_Save(void *obj, AG_Netbuf *buf)
{
	SG *sg = obj;
	SG_Node *node;
	int rv = 0;
	
	AG_WriteVersion(buf, "SG", &sgOps.ver);
	AG_MutexLock(&sg->lock);
	AG_WriteUint32(buf, sg->flags);
	rv = SG_NodeSave(sg, SGNODE(sg->root), buf);
	AG_MutexUnlock(&sg->lock);
	return (rv);
}

int
SG_NodeSave(SG *sg, SG_Node *node, AG_Netbuf *buf)
{
	SG_Node *cnode;
	Uint32 ncnodes;
	off_t bsize_offs, ncnodes_offs;

	/* Save generic node information. */
	AG_WriteString(buf, node->ops->name);
	AG_WriteString(buf, node->name);
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
	TAILQ_FOREACH(cnode, &node->cnodes, sgnodes) {
		if (SG_NodeSave(sg, cnode, buf) == -1) {
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
SG_Load(void *obj, AG_Netbuf *buf)
{
	SG *sg = obj;
	SG_Node *node;
	int rv;
	
	if (AG_ReadVersion(buf, "SG", &sgOps.ver, NULL) == -1)
		return (-1);

	AG_MutexLock(&sg->lock);
	sg->flags = (Uint)AG_ReadUint32(buf);
	rv = SG_NodeLoad(sg, (SG_Node **)&sg->root, buf);
	AG_MutexUnlock(&sg->lock);
	return ((rv == 0 && sg->root != NULL) ? 0 : -1);
}

int
SG_NodeLoad(SG *sg, SG_Node **rnode, AG_Netbuf *buf)
{
	char name[SG_NODE_NAME_MAX];
	char type[SG_CLASS_MAX];
	SG_NodeOps *nops;
	SG_Node *node;
	Uint32 bsize, ncnodes;
	int i;

	/* Load generic node information. */
	AG_CopyString(type, buf, sizeof(type));
	AG_CopyString(name, buf, sizeof(name));
	bsize = AG_ReadUint32(buf);
	for (i = 0; i < sgElementsCnt; i++) {
		if (strcmp(sgElements[i]->name, type) == 0)
			break;
	}
	if (i == sgElementsCnt) {
		fprintf(stderr, "%s: skipping node %s (%s/%luB)\n",
		    AGOBJECT(sg)->name, name, type, (Ulong)bsize);
		AG_NetbufSeek(buf, bsize, SEEK_CUR);
		*rnode = NULL;
		return (0);
	}
	nops = sgElements[i];
	node = Malloc(nops->size, M_SG);
	nops->init(node, name);
	node->sg = sg;

	node->flags = (Uint)AG_ReadUint16(buf);
	dprintf("%s: type <%s>, len %lu\n", name, type, (Ulong)bsize);
	SG_ReadMatrixv(buf, &node->T);

	/* Load information specific to this type of node. */
	if (node->ops->load != NULL &&
	    node->ops->load(node, buf) == -1) {
		*rnode = NULL;
		return (-1);
	}

	/* Load the child nodes recursively. */
	ncnodes = AG_ReadUint32(buf);
	dprintf("%s: %u child nodes\n", name, ncnodes);
	while (ncnodes-- > 0) {
		SG_Node *cnode;

		if (SG_NodeLoad(sg, &cnode, buf) == -1) {
			*rnode = NULL;
			return (-1);
		}
		if (cnode != NULL) {
			dprintf("%s: attach child %s\n", node->name,
			    cnode->name);
			SG_NodeAttach(node, cnode);
		} else {
			dprintf("%s: child load failure\n", node->name);
		}
	}

	*rnode = node;
	return (0);
}

SG_Node *
SG_SearchNodes(SG_Node *node, const char *name)
{
	SG_Node *cnode, *rnode;

	TAILQ_FOREACH(cnode, &node->cnodes, sgnodes) {
		if (strcmp(cnode->name, name) == 0) {
			return (cnode);
		} else {
			if ((rnode = SG_SearchNodes(cnode, name)) != NULL)
				return (rnode);
		}
	}
	return (NULL);
}

void *
SG_FindNode(SG *sg, const char *name)
{
	if (strcmp(name, SGNODE(sg->root)->name) == 0) {
		return (sg->root);
	} else {
		return ((void *)SG_SearchNodes(SGNODE(sg->root), name));
	}
}

/*
 * Compute the product of the transform matrices of the given node
 * and its parents.
 */
void
SG_GetNodeTransform(void *p, SG_Matrix *T)
{
	SG_Node *node = p;
	SG_Node *cnode = node;
	TAILQ_HEAD(,sg_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

	SG_MatrixIdentityv(T);

	while (cnode != NULL) {
		TAILQ_INSERT_TAIL(&rnodes, cnode, rnodes);
		if (cnode->pNode == NULL) {
			break;
		}
		cnode = cnode->pNode;
	}
	TAILQ_FOREACH(cnode, &rnodes, rnodes)
		SG_MatrixMultv(T, &cnode->T);
}

/*
 * Compute the product of the inverse transform matrices of the given node
 * and its parents.
 */
void
SG_GetNodeTransformInverse(void *p, SG_Matrix *T)
{
	SG_Node *node = p;
	SG_Node *cnode = node;
	TAILQ_HEAD(,sg_node) rnodes = TAILQ_HEAD_INITIALIZER(rnodes);

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

SG_Vector
SG_NodePos(void *p)
{
	SG_Node *node = p;
	SG_Matrix T;
	SG_Vector v = SG_VECTOR(0.0, 0.0, 0.0);
	
	SG_GetNodeTransformInverse(node, &T);
	SG_MatrixMultVectorv(&v, &T);
	return (v);
}

SG_Vector
SG_NodeDir(void *p)
{
	SG_Node *node = p;
	SG_Matrix T;
	SG_Vector v = SG_K;				/* Convention */
	
	SG_GetNodeTransform(node, &T);
	SG_MatrixMultVectorv(&v, &T);
	return (v);
}

void
SG_NodeAttach(void *ppNode, void *pcNode)
{
	SG_Node *pNode = ppNode;
	SG_Node *cNode = pcNode;

	cNode->sg = pNode->sg;
	cNode->pNode = pNode;
	TAILQ_INSERT_TAIL(&pNode->cnodes, cNode, sgnodes);
	if (pNode->sg != NULL) {
		TAILQ_INSERT_TAIL(&pNode->sg->nodes, cNode, nodes);
	}
}

void
SG_NodeDetach(void *ppNode, void *pcNode)
{
	SG_Node *pNode = ppNode;
	SG_Node *cNode = pcNode;

	TAILQ_REMOVE(&pNode->cnodes, cNode, sgnodes);
	if (pNode->sg != NULL) {
		TAILQ_REMOVE(&pNode->sg->nodes, cNode, nodes);
	}
	cNode->sg = NULL;
	cNode->pNode = NULL;
}

void *
SG_NodeAdd(void *pNode, const char *name, const SG_NodeOps *ops, Uint flags)
{
	SG_Node *n;

	n = Malloc(ops->size, M_SG);
	SG_NodeInit(n, name, ops, flags);
	SG_NodeAttach(pNode, n);
	return (n);
}

void
SG_RenderNode(SG *sg, SG_Node *node, SG_View *view)
{
	SG_Matrix Tsave, T;
	SG_Node *cnode;

	SG_GetMatrixGL(GL_MODELVIEW_MATRIX, &Tsave);
	T = SG_MatrixTransposep(&node->T);	/* OpenGL is column-major */
	SG_MultMatrixGL(&T);
	if (node->ops->draw != NULL) {
		node->ops->draw(node, view);
	}
	TAILQ_FOREACH(cnode, &node->cnodes, sgnodes) {
		SG_RenderNode(sg, cnode, view);
	}
	SG_LoadMatrixGL(&Tsave);
}

#endif /* HAVE_OPENGL */
