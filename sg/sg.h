/*	Public domain	*/

#ifndef _AGAR_SG_H_
#define _AGAR_SG_H_
#include "begin_code.h"

#ifdef _AGAR_INTERNAL
#include <sg/sg_math.h>
#include <sg/sg_vector.h>
#include <sg/sg_matrix.h>
#include <sg/sg_quat.h>
#include <sg/sg_geom.h>
#else
#include <agar/sg/sg_math.h>
#include <agar/sg/sg_vector.h>
#include <agar/sg/sg_matrix.h>
#include <agar/sg/sg_quat.h>
#include <agar/sg/sg_geom.h>
#endif

typedef struct sg_color {
	SG_Real r, g, b, a;
} SG_Color;

#include "close_code.h"

#ifdef _AGAR_INTERNAL
#include <sg/sk.h>
#include <sg/sg_view.h>
#else
#include <agar/sg/sk.h>
#include <agar/sg/sg_view.h>
#endif

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
	SG_Matrix T;			/* Transformation from parent */
	TAILQ_HEAD(,sg_node) cnodes;	/* Siblings */
	TAILQ_ENTRY(sg_node) sgnodes;	/* Entry in parent list */
	TAILQ_ENTRY(sg_node) nodes;	/* Entry in global sg list */
	TAILQ_ENTRY(sg_node) rnodes;	/* Used for quick inverse traversal */
} SG_Node;

typedef struct sg {
	struct ag_object obj;
	Uint flags;
#define SG_OVERLAY_WIREFRAME	0x01	/* Overlay scene wireframe */
#define SG_OVERLAY_VERTICES	0x02	/* Draw points at vertices */
#define SG_OVERLAY_VNORMALS	0x04	/* Draw vertex normals */
#define SG_OVERLAY_FNORMALS	0x08	/* Draw facet normals */
#define SG_SKIP_UNKNOWN_NODES	0x10	/* Ignore unknown node types in save
					   (otherwise LoadObject will fail) */
	AG_Mutex lock;
	void *tess;			/* GLU tesselator object */
	SG_Node *root;			/* Root of graph */
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
	for((node) = (struct ntype *)TAILQ_FIRST(&(pnode)->cnodes);	\
	    (node) != (struct ntype *)TAILQ_END(&(pnode)->cnodes);	\
	    (node) = (struct ntype *)TAILQ_NEXT(SGNODE(node),sgnodes))
#define SG_FOREACH_SUBNODE_CLASS(node, pnode, ntype, cn)		\
	SG_FOREACH_SUBNODE(node,pnode,ntype)				\
		if (!SG_NodeOfClass(SGNODE(node),(cn))) {		\
			continue;					\
		} else

extern SG_NodeOps **sgElements;
extern Uint         sgElementsCnt;

#ifdef _AGAR_INTERNAL
#include <sg/sg_dummy.h>
#include <sg/sg_program.h>
#include <sg/sg_cg_program.h>
#include <sg/sg_light.h>
#include <sg/sg_camera.h>
#include <sg/sg_primitive.h>
#include <sg/sg_material.h>
#include <sg/sg_point.h>
#include <sg/sg_planeobj.h>
#include <sg/sg_object.h>
#include <sg/sg_solid.h>
#include <sg/sg_sphere.h>
#include <sg/sg_box.h>
#include <sg/sg_voxel.h>
#else
#include <agar/sg/sg_dummy.h>
#include <agar/sg/sg_program.h>
#include <agar/sg/sg_cg_program.h>
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
#endif

#ifdef _AGAR_INTERNAL
#include <sg/pe/pe.h>
#else
#include <agar/sg/pe/pe.h>
#endif

__BEGIN_DECLS
int	 SG_InitEngine(void);
void	 SG_DestroyEngine(void);

SG	*SG_New(void *, const char *);
void	 SG_AttachDefaultNodes(SG *);
void	 SG_Init(void *, const char *);
void	 SG_Reinit(void *);
void	 SG_Destroy(void *);
int	 SG_Save(void *, AG_Netbuf *);
int	 SG_Load(void *, AG_Netbuf *);
void	*SG_Edit(void *);
void	 SG_RenderNode(SG *, SG_Node *, SG_View *);

void		 SG_RegisterClass(SG_NodeOps *);
int		 SG_NodeOfClassGeneral(SG_Node *, const char *);
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

/*
 * Operations on node transformation matrices
 */
#define SG_Identity(n)		MatIdentityv(&SGNODE(n)->T)
#define SG_Rotatev(n,a,d)	MatRotateAxis(&SGNODE(n)->T,(a),(d))
#define SG_RotateI(n,d)		MatRotateI(&SGNODE(n)->T,(d))
#define SG_RotateJ(n,d)		MatRotateJ(&SGNODE(n)->T,(d))
#define SG_RotateK(n,d)		MatRotateK(&SGNODE(n)->T,(d))
#define SG_Orbitv(n,p,a,d)	MatOrbitAxis(&SGNODE(n)->T, \
				             VecSub((p),SG_NodePos(n)),(a),(d));
#define SG_Orbitvd(n,p,a,d)	SG_Orbitv((n),(p),(a),SG_Radians(d))
#define SG_Rotatevd(n,a,v)	MatRotateAxis(&SGNODE(n)->T,SG_Radians(a),(v))
#define SG_RotateId(n,a)	MatRotateI(&SGNODE(n)->T,SG_Radians(a))
#define SG_RotateJd(n,a)	MatRotateJ(&SGNODE(n)->T,SG_Radians(a))
#define SG_RotateKd(n,a)	MatRotateK(&SGNODE(n)->T,SG_Radians(a))
#define SG_Translatev(n,v)	MatTranslate(&SGNODE(n)->T,(v))
#define SG_Translate3(n,x,y,z)	MatTranslate3(&SGNODE(n)->T,(x),(y),(z))
#define SG_TranslateX(n,t)	MatTranslateX(&SGNODE(n)->T,(t))
#define SG_TranslateY(n,t)	MatTranslateY(&SGNODE(n)->T,(t))
#define SG_TranslateZ(n,t)	MatTranslateZ(&SGNODE(n)->T,(t))
#define SG_Scale(n,x,y,z)	MatScale(&SGNODE(n)->T,(x),(y),(z),1.0)
#define SG_UniScale(n,r)	MatUniScale(&SGNODE(n)->T,(r))

static __inline__ int
SG_NodeOfClass(SG_Node *node, const char *cname)
{
	const char *c;
#ifdef DEBUG
	if (cname[0] == '*' && cname[1] == '\0')
		AG_FatalError("Use SG_FOREACH_NODE()");
#endif
	for (c = &cname[0]; *c != '\0'; c++) {
		if (c[0] == ':' && c[1] == '*' && c[2] == '\0') {
			if (c == &cname[0]) {
				return (1);
			}
			if (strncmp(node->ops->name, cname, c - &cname[0])
			    == 0)
				return (1);
		}
	}
	return (SG_NodeOfClassGeneral(node, cname));	/* General case */
}

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SG_H_ */
