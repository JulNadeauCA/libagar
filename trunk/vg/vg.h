/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include <agar/vg/begin.h>

#define VG_NAME_MAX		128
#define VG_LAYER_NAME_MAX	128
#define VG_STYLE_NAME_MAX	16
#define VG_TYPE_NAME_MAX	32
#define VG_SYM_NAME_MAX		16
#define VG_HANDLE_MAX	 	(0xffffffff-1)

enum vg_alignment {
	VG_ALIGN_TL, VG_ALIGN_TC, VG_ALIGN_TR,
	VG_ALIGN_ML, VG_ALIGN_MC, VG_ALIGN_MR,
	VG_ALIGN_BL, VG_ALIGN_BC, VG_ALIGN_BR
};

typedef struct vg_vertex {
	float x, y;
} VG_Vector;

typedef struct vg_rect {
	float x, y;
	float w, h;
} VG_Rect;

typedef struct vg_color {
	int idx;			/* Index into VG color array */
	Uint8 r, g, b, a;		/* RGBA color value */
} VG_Color;

typedef struct vg_indexed_color {
	char     name[VG_STYLE_NAME_MAX];	/* Identifier */
	VG_Color color;				/* Color value (or reference) */
} VG_IndexedColor;

struct vg;
struct vg_view;
struct vg_node;
struct ag_static_icon;

#include <agar/vg/vg_snap.h>
#include <agar/vg/vg_ortho.h>

typedef struct vg_node_ops {
	const char            *name;
	struct ag_static_icon *icon;
	size_t                 size;

	void  (*init)(void *);
	void  (*destroy)(void *);
	int   (*load)(void *, AG_DataSource *, const AG_Version *);
	void  (*save)(void *, AG_DataSource *);
	void  (*draw)(void *, struct vg_view *);
	void  (*extent)(void *, struct vg_view *, VG_Vector *a,
	                VG_Vector *b);
	float (*pointProximity)(void *, struct vg_view *, VG_Vector *p);
	float (*lineProximity)(void *, struct vg_view *, VG_Vector *p1,
	                       VG_Vector *p2);
	void  (*deleteNode)(void *);
	void  (*moveNode)(void *, VG_Vector vAbs, VG_Vector vRel);
	void  *(*edit)(void *, struct vg_view *);
} VG_NodeOps;

typedef struct vg_layer {
	char     name[VG_LAYER_NAME_MAX];	/* Layer name */
	int      visible;			/* Flag of visibility */
	VG_Color color;				/* Per-layer default color */
	Uint8    alpha;				/* Per-layer alpha value */
} VG_Layer;

typedef struct vg_matrix {
	float m[3][3];
} VG_Matrix;

typedef struct vg_node {
	VG_NodeOps *ops;		/* Node class information */
	Uint32 handle;			/* Instance handle */
	char sym[VG_SYM_NAME_MAX];	/* Symbolic name */

	Uint flags;
#define VG_NODE_NOSAVE		0x01	/* Don't save with drawing */
#define VG_NODE_SELECTED	0x02	/* Selection flag */
#define VG_NODE_MOUSEOVER	0x04	/* Mouse overlap flag */
#define VG_NODE_SAVED_FLAGS	0

	struct vg      *vg;		/* Back pointer to VG */
	struct vg_node *parent;		/* Back pointer to parent node */
	struct vg_node **refs;		/* Referenced nodes */
	Uint            nRefs;		/* Referenced node count */
	Uint            nDeps;		/* Dependency count */

	VG_Color  color;		/* Element color */
	int       layer;		/* Layer index */
	VG_Matrix T;			/* Transformation matrix */
	void     *p;			/* User pointer */

	AG_TAILQ_HEAD_(vg_node) cNodes;	/* Child nodes */
	AG_TAILQ_ENTRY(vg_node) tree;	/* Entry in tree */
	AG_TAILQ_ENTRY(vg_node) list;	/* Entry in global list */
	AG_TAILQ_ENTRY(vg_node) reverse; /* For VG_NodeTransform() */
	AG_TAILQ_ENTRY(vg_node) user;	/* Entry in user list */
} VG_Node;

#define VGNODE(p) ((VG_Node *)(p))

typedef struct vg {
	Uint flags;
#define VG_NO_ANTIALIAS	0x01		/* Disable anti-aliasing */

	AG_Mutex lock;

	VG_IndexedColor *colors;	/* Global color table */
	Uint            nColors;	/* Color count */
	VG_Color        fillColor;	/* Background color */
	VG_Color        selectionColor;	/* Selected item/block color */
	VG_Color        mouseoverColor;	/* Mouse overlap item color */

	VG_Layer *layers;		/* Layer information */
	Uint	 nLayers;		/* Layer count */
	
	VG_Matrix *T;			/* Stack of viewing matrices */
	Uint      nT;

	VG_Node *root;			/* Tree of entities */
	AG_TAILQ_HEAD_(vg_node) nodes;	/* List of entities */
	AG_TAILQ_ENTRY(vg) user;	/* Entry in user list */
} VG;

extern VG_NodeOps **vgNodeClasses;
extern Uint         vgNodeClassCount;

#include <agar/vg/vg_math.h>

#define VG_FOREACH_NODE(node, vg, ntype)				\
	for((node) = (struct ntype *)AG_TAILQ_FIRST(&(vg)->nodes);	\
	    (node) != (struct ntype *)AG_TAILQ_END(&(vg)->nodes);	\
	    (node) = (struct ntype *)AG_TAILQ_NEXT(VGNODE(node),list))
#define VG_FOREACH_NODE_CLASS(node, vg, ntype, cn)			\
	VG_FOREACH_NODE(node,vg,ntype)					\
		if (!VG_NodeIsClass(VGNODE(node),(cn))) {		\
			continue;					\
		} else
#define VG_FOREACH_CHLD(node, pnode, ntype)				\
	for((node) = (struct ntype *)AG_TAILQ_FIRST(&VGNODE(pnode)->cNodes); \
	    (node) != (struct ntype *)AG_TAILQ_END(&VGNODE(pnode)->cNodes); \
	    (node) = (struct ntype *)AG_TAILQ_NEXT(VGNODE(node),tree))
#define VG_FOREACH_CHLD_CLASS(node, pnode, ntype, cn)			\
	VG_FOREACH_CHLD(node,pnode,ntype)				\
		if (!VG_NodeIsClass(VGNODE(node),(cn))) {		\
			continue;					\
		} else

__BEGIN_DECLS
void      VG_InitSubsystem(void);
void      VG_DestroySubsystem(void);

VG       *VG_New(Uint);
void      VG_Init(VG *, Uint);
void      VG_Destroy(VG *);
void      VG_Clear(VG *);
void      VG_ClearNodes(VG *);
void      VG_ClearColors(VG *);
void      VG_Save(VG *, AG_DataSource *);
int       VG_Load(VG *, AG_DataSource *);

VG_NodeOps *VG_LookupClass(const char *);
void        VG_RegisterClass(VG_NodeOps *);
void        VG_UnregisterClass(VG_NodeOps *);

void      VG_NodeInit(void *, VG_NodeOps *);
void      VG_NodeAttach(void *, void *);
void      VG_NodeDetach(void *);
void      VG_NodeDestroy(void *);
int       VG_Delete(void *);
void      VG_Merge(void *, VG *);
void      VG_AddRef(void *, void *);
Uint      VG_DelRef(void *, void *);
void      VG_NodeTransform(void *, VG_Matrix *);
Uint32    VG_GenNodeName(VG *, const char *);

void      VG_SetBackgroundColor(VG *, VG_Color);
void      VG_SetSelectionColor(VG *, VG_Color);
void      VG_SetMouseOverColor(VG *, VG_Color);

VG_Layer *VG_PushLayer(VG *, const char *);
void      VG_PopLayer(VG *);

void	  VG_SetSym(void *, const char *, ...);
void	  VG_SetLayer(void *, int);
void	  VG_SetColorv(void *, const VG_Color *);
void	  VG_SetColorRGB(void *, Uint8, Uint8, Uint8);
void	  VG_SetColorRGBA(void *, Uint8, Uint8, Uint8, Uint8);

VG_Vector VG_ReadVector(AG_DataSource *);
void      VG_WriteVector(AG_DataSource *, const VG_Vector *);
VG_Color  VG_ReadColor(AG_DataSource *);
void      VG_WriteColor(AG_DataSource *, const VG_Color *);
void      VG_WriteRef(AG_DataSource *, void *);
void     *VG_ReadRef(AG_DataSource *, void *, const char *);

void     *VG_PointProximity(struct vg_view *, const char *, const VG_Vector *,
                            VG_Vector *, void *);
void     *VG_PointProximityMax(struct vg_view *, const char *,
                               const VG_Vector *, VG_Vector *, void *, float);
VG_Matrix VG_MatrixInvert(VG_Matrix);

/* Acquire the VG lock. */
static __inline__ void
VG_Lock(VG *vg)
{
	AG_MutexLock(&vg->lock);
}

/* Release the VG lock. */
static __inline__ void
VG_Unlock(VG *vg)
{
	AG_MutexUnlock(&vg->lock);
}

/* Evaluate whether a node belongs to a class. */
static __inline__ int
VG_NodeIsClass(void *p, const char *name)
{
	return (strcmp(VGNODE(p)->ops->name, name) == 0);
}

/* Return the VG_Color representing a RGB triplet. */
static __inline__ VG_Color
VG_GetColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	VG_Color vc;
	vc.r = r;
	vc.g = g;
	vc.b = b;
	vc.a = 255;
	vc.idx = -1;
	return (vc);
}

/* Return the VG_Color from RGBA components. */
static __inline__ VG_Color
VG_GetColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	VG_Color vc;
	vc.r = r;
	vc.g = g;
	vc.b = b;
	vc.a = a;
	vc.idx = -1;
	return (vc);
}

/* Convert a VG_Color to opaque AG_Color */
static __inline__ AG_Color
VG_MapColorRGB(VG_Color vc)
{
	return AG_ColorRGB(vc.r, vc.g, vc.b);
}

/* Convert a VG_Color to AG_Color */
static __inline__ AG_Color
VG_MapColorRGBA(VG_Color vc)
{
	return AG_ColorRGBA(vc.r, vc.g, vc.b, vc.a);
}

/* Alpha-blend colors cDst and cSrc and return in cDst. */
static __inline__ void
VG_BlendColors(VG_Color *cDst, VG_Color cSrc)
{
	cDst->r = (((cSrc.r - cDst->r)*cSrc.a) >> 8) + cDst->r;
	cDst->g = (((cSrc.g - cDst->g)*cSrc.a) >> 8) + cDst->g;
	cDst->b = (((cSrc.b - cDst->b)*cSrc.a) >> 8) + cDst->b;
	cDst->a = (cDst->a+cSrc.a >= 255) ? 255 : (cDst->a+cSrc.a);
}

/* Search a node by symbol. */
static __inline__ void *
VG_FindNodeSym(VG *vg, const char *sym)
{
	VG_Node *vn;

	AG_TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (strcmp(vn->sym, sym) == 0)
			return (vn);
	}
	return (NULL);
}

/* Search a node by handle and class. Used for loading datafiles. */
static __inline__ void *
VG_FindNode(VG *vg, Uint32 handle, const char *type)
{
	VG_Node *vn;

	AG_TAILQ_FOREACH(vn, &vg->nodes, list) {
		if (vn->handle == handle &&
		    strcmp(vn->ops->name, type) == 0)
			return (vn);
	}
	return (NULL);
}

/* Push the transformation matrix stack. */
static __inline__ void
VG_PushMatrix(VG *vg)
{
	vg->T = (VG_Matrix *)AG_Realloc(vg->T, (vg->nT+1)*sizeof(VG_Matrix));
	memcpy(&vg->T[vg->nT], &vg->T[vg->nT-1], sizeof(VG_Matrix));
	vg->nT++;
}

/* Pop the transformation matrix stack. */
static __inline__ void
VG_PopMatrix(VG *vg)
{
#ifdef AG_DEBUG
	if (vg->nT == 1) { AG_FatalError("VG_PopMatrix"); }
#endif
	vg->nT--;
}

/* Load identity matrix for the given node. */
static __inline__ void
VG_LoadIdentity(void *pNode)
{
	VG_Node *vn = (VG_Node *)pNode;
	
	vn->T.m[0][0] = 1.0f;	vn->T.m[0][1] = 0.0f;	vn->T.m[0][2] = 0.0f;
	vn->T.m[1][0] = 0.0f;	vn->T.m[1][1] = 1.0f;	vn->T.m[1][2] = 0.0f;
	vn->T.m[2][0] = 0.0f;	vn->T.m[2][1] = 0.0f;	vn->T.m[2][2] = 1.0f;
}

/* Set the position of the given node relative to its parent. */
static __inline__ void
VG_SetPositionInParent(void *pNode, VG_Vector v)
{
	VG_Node *vn = (VG_Node *)pNode;
	
	vn->T.m[0][2] = v.x;
	vn->T.m[1][2] = v.y;
}

/* Translate the given node. */
static __inline__ void
VG_Translate(void *pNode, VG_Vector v)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Matrix T;
	
	T.m[0][0] = 1.0f;	T.m[0][1] = 0.0f;	T.m[0][2] = v.x;
	T.m[1][0] = 0.0f;	T.m[1][1] = 1.0f;	T.m[1][2] = v.y;
	T.m[2][0] = 0.0f;	T.m[2][1] = 0.0f;	T.m[2][2] = 1.0f;

	VG_MultMatrix(&vn->T, &T);
}

/* Apply uniform scaling to the current viewing matrix. */
static __inline__ void
VG_Scale(void *pNode, float s)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Matrix T;
	
	T.m[0][0] = s;		T.m[0][1] = 0.0f;	T.m[0][2] = 0.0f;
	T.m[1][0] = 0.0f;	T.m[1][1] = s;		T.m[1][2] = 0.0f;
	T.m[2][0] = 0.0f;	T.m[2][1] = 0.0f;	T.m[2][2] = s;

	VG_MultMatrix(&vn->T, &T);
}

/* Apply a rotation to the current viewing matrix. */
static __inline__ void
VG_Rotate(void *pNode, float theta)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Matrix T;
	float rCos = VG_Cos(theta);
	float rSin = VG_Sin(theta);

	T.m[0][0] = +rCos;	T.m[0][1] = -rSin;	T.m[0][2] = 0.0f;
	T.m[1][0] = +rSin;	T.m[1][1] = +rCos;	T.m[1][2] = 0.0f;
	T.m[2][0] = 0.0f;	T.m[2][1] = 0.0f;	T.m[2][2] = 1.0f;

	VG_MultMatrix(&vn->T, &T);
}

/* Reflection about vertical line going through the origin. */
static __inline__ void
VG_FlipVert(void *pNode)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Matrix T;

	T.m[0][0] = 1.0f;	T.m[0][1] = 0.0f;	T.m[0][2] = 0.0f;
	T.m[1][0] = 0.0f;	T.m[1][1] = -1.0f;	T.m[1][2] = 0.0f;
	T.m[2][0] = 0.0f;	T.m[2][1] = 0.0f;	T.m[2][2] = 1.0f;

	VG_MultMatrix(&vn->T, &T);
}

/* Reflection about horizontal line going through the origin. */
static __inline__ void
VG_FlipHoriz(void *pNode)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Matrix T;

	T.m[0][0] = -1.0f;	T.m[0][1] = 0.0f;	T.m[0][2] = 0.0f;
	T.m[1][0] = 0.0f;	T.m[1][1] = 1.0f;	T.m[1][2] = 0.0f;
	T.m[2][0] = 0.0f;	T.m[2][1] = 0.0f;	T.m[2][2] = 1.0f;

	VG_MultMatrix(&vn->T, &T);
}

/* Mark node as selected. */
static __inline__ void
VG_Select(void *pNode)
{
	VGNODE(pNode)->flags |= VG_NODE_SELECTED;
}

/* Remove the selection flag from node. */
static __inline__ void
VG_Unselect(void *pNode)
{
	VGNODE(pNode)->flags |= VG_NODE_SELECTED;
}

/* Mark all nodes selected. */
static __inline__ void
VG_SelectAll(VG *vg)
{
	VG_Node *vn;
	AG_TAILQ_FOREACH(vn, &vg->nodes, list)
		vn->flags |= VG_NODE_SELECTED;
}

/* Remove the selection flag from all nodes. */
static __inline__ void
VG_UnselectAll(VG *vg)
{
	VG_Node *vn;
	AG_TAILQ_FOREACH(vn, &vg->nodes, list)
		vn->flags &= ~(VG_NODE_SELECTED);
}

/* Return the effective position of the given node relative to the VG origin. */
static __inline__ VG_Vector
VG_Pos(void *node)
{
	VG_Matrix T;
	VG_Vector v = { 0.0f, 0.0f };

	VG_NodeTransform(node, &T);
	VG_MultMatrixByVector(&v, &v, &T);
	return (v);
}

/* Set the position of the given node relative to the VG origin. */
static __inline__ void
VG_SetPosition(void *pNode, VG_Vector v)
{
	VG_Node *vn = (VG_Node *)pNode;
	VG_Vector vParent;

	vn->T.m[0][2] = v.x;
	vn->T.m[1][2] = v.y;
	if (vn->parent != NULL) {
		vParent = VG_Pos(vn->parent);
		vn->T.m[0][2] -= vParent.x;
		vn->T.m[1][2] -= vParent.y;
	}
}
__END_DECLS

#include <agar/vg/vg_point.h>
#include <agar/vg/vg_line.h>
#include <agar/vg/vg_circle.h>
#include <agar/vg/vg_arc.h>
#include <agar/vg/vg_text.h>
#include <agar/vg/vg_polygon.h>

#include <agar/vg/close.h>
#endif /* _AGAR_VG_H_ */
