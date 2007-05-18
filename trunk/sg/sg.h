/*	$Csoft: vg.h,v 1.41 2005/09/27 00:25:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SG_H_
#define _AGAR_SG_H_

#ifndef _AGAR_SG_PUBLIC
#include <config/edition.h>
#endif
#include <agar/config/_mk_have_sys_types_h.h>

#if 0
#ifdef _MK_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifndef _MK_HAVE_UNSIGNED_TYPEDEFS
#define _MK_HAVE_UNSIGNED_TYPEDEFS
typedef unsigned int Uint;
typedef unsigned char Uchar;
typedef unsigned long Ulong;
#endif
#endif

#include "begin_code.h"

#include <agar/sg/sg_math.h>
#include <agar/sg/sg_vector.h>
#include <agar/sg/sg_matrix.h>
#include <agar/sg/sg_quat.h>
#include <agar/sg/sg_spherical.h>
#include <agar/sg/sg_plane.h>

typedef struct sg_color {
	SG_Real r, g, b, a;
} SG_Color;

#include "close_code.h"

#include <agar/sg/sk.h>
#include <agar/sg/sg_view.h>

#include "begin_code.h"

#define SG_NODE_NAME_MAX 32
#define SG_CLASS_MAX 128

struct sg;
struct sg_node;
struct sg_point;
struct sg_camera;
struct ag_menu_item;
struct ag_widget;

enum sg_side {
	SG_LEFT,
	SG_RIGHT
};
enum sg_winding {
	SG_CW,
	SG_CCW
};

typedef struct sg_node_ops {
	const char *name;
	size_t size;
	Uint flags;
	void (*init)(void *, const char *);
	void (*destroy)(void *);
	int (*load)(void *, AG_Netbuf *);
	int (*save)(void *, AG_Netbuf *);
	void (*edit)(void *, struct ag_widget *, SG_View *);
	void (*menuInstance)(void *, struct ag_menu_item *, SG_View *);
	void (*menuClass)(struct sg *, struct ag_menu_item *, SG_View *);
	void (*draw)(void *, SG_View *);
} SG_NodeOps;

typedef struct sg_node {
	const SG_NodeOps *ops;
	char name[SG_NODE_NAME_MAX];
	Uint flags;
#define SG_NODE_SELECTED	0x01
	struct sg *sg;			/* Back pointer to sg */
	struct sg_node *pNode;		/* Back pointer to parent node */
	SG_Matrix4 T;			/* Transformation from parent */
	TAILQ_HEAD(,sg_node) cnodes;	/* Siblings */
	TAILQ_ENTRY(sg_node) sgnodes;	/* Entry in parent list */
	TAILQ_ENTRY(sg_node) nodes;	/* Entry in flat list */
	TAILQ_ENTRY(sg_node) rnodes;	/* Used for quick inverse traversal */
} SG_Node;

typedef struct sg {
	struct ag_object obj;
	Uint flags;
#define SG_OVERLAY_WIREFRAME	0x01	/* Overlay scene wireframe */
#define SG_OVERLAY_VERTICES	0x02	/* Draw points at vertices */
#define SG_OVERLAY_VNORMALS	0x04	/* Draw vertex normals */
#define SG_OVERLAY_FNORMALS	0x08	/* Draw facet normals */
	AG_Mutex lock;
	void *tess;			/* GLU tesselator object */
	struct sg_point *root;		/* Root of graph */
//	CGcontext cgCtx;		/* Context for CG programs */
	TAILQ_HEAD(,sg_node) nodes;	/* Flat list of nodes */
} SG;

#define SGNODE(node) ((SG_Node *)(node))
#define SGNODE_SELECTED(node) (((SG_Node *)(node))->flags & SG_NODE_SELECTED)
#define SG_FOREACH_NODE(node, sg, ntype)				\
	for((node) = (struct ntype *)TAILQ_FIRST(&(sg)->nodes);		\
	    (node) != (struct ntype *)TAILQ_END(&(sg)->nodes);		\
	    (node) = (struct ntype *)TAILQ_NEXT(SGNODE(node),nodes))
#define SG_FOREACH_NODE_CLASS(node, sg, ntype, cn)			\
	SG_FOREACH_NODE(node,sg,ntype)					\
		if (!SG_NodeOfClass(SGNODE(node),(cn))) {		\
			continue;					\
		} else
#define SG_FOREACH_SUBNODE(node, pnode, ntype)				\
	for((node) = (struct ntype *)TAILQ_FIRST(&(pnode)->nodes);	\
	    (node) != (struct ntype *)TAILQ_END(&(pnode)->cnodes);	\
	    (node) = (struct ntype *)TAILQ_NEXT(SGNODE(node),sgnodes))
#define SG_FOREACH_SUBNODE_CLASS(node, pnode, ntype, cn)		\
	SG_FOREACH_SUBNODE(node,pnode,ntype)				\
		if (!SG_NodeOfClass(SGNODE(node),(cn))) {		\
			continue;					\
		} else

extern SG_NodeOps **sgElements;
extern Uint         sgElementsCnt;

#include <agar/sg/sg_dummy.h>
#include <agar/sg/sg_light.h>
#include <agar/sg/sg_camera.h>
#include <agar/sg/sg_primitive.h>

#include <agar/sg/sg_material.h>
#include <agar/sg/sg_point.h>
#include <agar/sg/sg_planeobj.h>
#include <agar/sg/sg_object.h>
#include <agar/sg/sg_solid.h>
#include <agar/sg/sg_sphere.h>
#include <agar/sg/sg_box.h>
#include <agar/sg/sg_voxel.h>

__BEGIN_DECLS
int	 SG_InitEngine(void);
void	 SG_DestroyEngine(void);

SG	*SG_New(void *, const char *);
void	 SG_Init(void *, const char *);
void	 SG_Reinit(void *);
void	 SG_Destroy(void *);
int	 SG_Save(void *, AG_Netbuf *);
int	 SG_Load(void *, AG_Netbuf *);
void	*SG_Edit(void *);
void	 SG_RenderNode(SG *, SG_Node *, SG_View *);

void		 SG_NodeRegister(SG_NodeOps *);
__inline__ int	 SG_NodeOfClass(SG_Node *, const char *);
void		 SG_NodeInit(void *, const char *, const void *, Uint);
void		*SG_NodeAdd(void *, const char *, const SG_NodeOps *, Uint);
void		 SG_NodeAttach(void *, void *);
void		 SG_NodeDetach(void *, void *);
int		 SG_NodeSave(SG *, SG_Node *, AG_Netbuf *);
int		 SG_NodeLoad(SG *, SG_Node **, AG_Netbuf *);
SG_Node 	*SG_SearchNodes(SG_Node *, const char *);
void		*SG_FindNode(SG *, const char *);
void		 SG_GetNodeTransform(void *, SG_Matrix *);
void		 SG_GetNodeTransformInverse(void *, SG_Matrix *);
SG_Vector	 SG_NodePos(void *);
SG_Vector	 SG_NodeDir(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SG_H_ */
