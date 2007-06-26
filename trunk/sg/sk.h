/*	Public domain	*/

#ifndef _AGAR_SK_H_
#define _AGAR_SK_H_

#ifdef _AGAR_INTERNAL
#include <config/edition.h>
#include <sg/sg.h>
#include <sg/sk_view.h>
#include <gui/units.h>
#else
#include <agar/sg/sg.h>
#include <agar/sg/sk_view.h>
#include <agar/gui/units.h>
#endif

#include "begin_code.h"

#define SK_TYPE_NAME_MAX 64
#define SK_NODE_NAME_MAX (SK_TYPE_NAME_MAX+12)
#define SK_NAME_MAX	 (0xffffffff-1)

struct sk;
struct sk_node;
struct sk_point;
struct ag_widget;

typedef struct sk_node_ops {
	const char *name;
	size_t size;
	Uint flags;
	void (*init)(void *, Uint32);
	void (*destroy)(void *);
	int (*load)(struct sk *, void *, AG_Netbuf *);
	int (*save)(struct sk *, void *, AG_Netbuf *);
	void (*draw_relative)(void *, SK_View *);
	void (*draw_absolute)(void *, SK_View *);
	void (*edit)(void *, struct ag_widget *, SK_View *);
} SK_NodeOps;

typedef struct sk_node {
	Uint32 name;			/* Unique node handle */
	const SK_NodeOps *ops;
	Uint flags;
#define SK_NODE_SELECTED	0x01
	struct sk *sk;			/* Back pointer to sk */
	struct sk_node *pNode;		/* Back pointer to parent node */
	SG_Matrix4 T;			/* Transformation from parent */
	TAILQ_HEAD(,sk_node) cnodes;	/* Siblings */
	Uint nrefs;			/* Reference count (optimization) */
	struct sk_node **refnodes;	/* References to other nodes */
	Uint nrefnodes;
	TAILQ_ENTRY(sk_node) sknodes;	/* Entry in transformation tree */
	TAILQ_ENTRY(sk_node) nodes;	/* Entry in flat node list */
	TAILQ_ENTRY(sk_node) rnodes;	/* Reverse entry (optimization) */
} SK_Node;

typedef struct sk_constraint {
	enum sk_constraint_type {
		SK_COINCIDENT,
		SK_PERPENDICULAR,
		SK_PARALLEL,
		SK_DISTANCE,
		SK_ANGLE,
		SK_CONSTRAINT_LAST
	} type;
	union {
		SG_Real dist;		/* DISTANCE value */
		SG_Real angle;		/* ANGLE value (radians) */
	} data;
#ifdef _AGAR_INTERNAL
#define ct_distance data.dist
#define ct_angle data.angle
#endif
	SK_Node *n1;
	SK_Node *n2;
	TAILQ_ENTRY(sk_constraint) constraints;
} SK_Constraint;

typedef struct sk_constraint_graph {
	TAILQ_HEAD(,sk_constraint) edges;
	TAILQ_ENTRY(sk_constraint_graph) clusters;
} SK_ConstraintGraph;
	
typedef struct sk {
	struct ag_object obj;
	Uint flags;
#define SK_SKIP_UNKNOWN_NODES	0x01		/* Ignore unimplemented nodes
						   in load (otherwise fail) */
	AG_Mutex lock;
	const AG_Unit *uLen;			/* Length unit */
	Uint32 last_name;			/* Last nodeid (optimization) */
	struct sk_point *root;			/* Root node */
	TAILQ_HEAD(,sk_node) nodes;		/* Flat node list */
	SK_ConstraintGraph ctGraph;		/* Original constraint graph */
	TAILQ_HEAD(,sk_constraint_graph) ctClusters; /* Clusters (for solver) */
} SK;

#define SKNODE(node) ((SK_Node *)(node))
#define SKNODE_SELECTED(node) (((SK_Node *)(node))->flags & SK_NODE_SELECTED)
#define SK_FOREACH_NODE(node, sk, ntype)				\
	for((node) = (struct ntype *)TAILQ_FIRST(&(sk)->nodes);		\
	    (node) != (struct ntype *)TAILQ_END(&(sk)->nodes);		\
	    (node) = (struct ntype *)TAILQ_NEXT(SKNODE(node),nodes))
#define SK_FOREACH_NODE_CLASS(node, sk, ntype, cn)			\
	SK_FOREACH_NODE(node,sk,ntype)					\
		if (!SK_NodeOfClass(SKNODE(node),(cn))) {		\
			continue;					\
		} else
#define SK_FOREACH_SUBNODE(node, pnode, ntype)				\
	for((node) = (struct ntype *)TAILQ_FIRST(&(pnode)->nodes);	\
	    (node) != (struct ntype *)TAILQ_END(&(pnode)->cnodes);	\
	    (node) = (struct ntype *)TAILQ_NEXT(SKNODE(node),sknodes))
#define SK_FOREACH_SUBNODE_CLASS(node, pnode, ntype, cn)		\
	SK_FOREACH_SUBNODE(node,pnode,ntype)				\
		if (!SK_NodeOfClass(SKNODE(node),(cn))) {		\
			continue;					\
		} else

extern SK_NodeOps **skElements;
extern Uint         skElementsCnt;
extern const char *skConstraintNames[];

#ifdef _AGAR_INTERNAL
#include <sg/sk_dummy.h>
#include <sg/sk_point.h>
#include <sg/sk_line.h>
#include <sg/sk_circle.h>
#include <sg/sk_arc.h>
#else
#include <agar/sg/sk_dummy.h>
#include <agar/sg/sk_point.h>
#include <agar/sg/sk_line.h>
#include <agar/sg/sk_circle.h>
#include <agar/sg/sk_arc.h>
#endif

__BEGIN_DECLS
int	 SK_InitEngine(void);
void	 SK_DestroyEngine(void);

SK	*SK_New(void *, const char *);
void	 SK_Init(void *, const char *);
void	 SK_Reinit(void *);
void	 SK_Destroy(void *);
int	 SK_Save(void *, AG_Netbuf *);
int	 SK_Load(void *, AG_Netbuf *);
void	*SK_Edit(void *);

void	 	 SK_RenderNode(SK *, SK_Node *, SK_View *);
__inline__ void	 SK_RenderAbsolute(SK *, SK_View *);

void		 SK_NodeRegister(SK_NodeOps *);
__inline__ int	 SK_NodeOfClass(SK_Node *, const char *);
void		 SK_NodeInit(void *, const void *, Uint32, Uint);
void		*SK_NodeAdd(void *, const SK_NodeOps *, Uint32, Uint);
void		 SK_NodeAttach(void *, void *);
void		 SK_NodeDetach(void *, void *);
int		 SK_NodeLoadGeneric(SK *, SK_Node **, AG_Netbuf *);
void		 SK_GetNodeTransform(void *, SG_Matrix *);
void		 SK_GetNodeTransformInverse(void *, SG_Matrix *);
SG_Vector	 SK_NodeCoords(void *);
SG_Vector	 SK_NodeDir(void *);
void		 SK_NodeAddReference(void *, void *);
void		*SK_FindNode(SK *, Uint32);
void		*SK_FindNodeOfType(SK *, const char *, Uint32);
Uint32		 SK_GenName(SK *);
char		*SK_NodeName(void *);
char		*SK_NodeNameCopy(void *, char *, size_t);
void		*SK_ReadRef(AG_Netbuf *, SK *, const char *);
void		 SK_WriteRef(AG_Netbuf *, void *);
void		 SK_SetLengthUnit(SK *, const AG_Unit *);

int		 SK_Solve(SK *);
void		 SK_FreeConstraintClusters(SK *);
void		 SK_InitConstraintGraph(SK_ConstraintGraph *);
void		 SK_FreeConstraintGraph(SK_ConstraintGraph *);
void		 SK_CopyConstraintGraph(const SK_ConstraintGraph *,
		                        SK_ConstraintGraph *);
SK_Constraint	*SK_AddConstraint(SK_ConstraintGraph *, void *, void *,
		                  enum sk_constraint_type, ...);
SK_Constraint	*SK_AddConstraintCopy(SK_ConstraintGraph *,
		                      const SK_Constraint *);
void		 SK_DelConstraint(SK_ConstraintGraph *, SK_Constraint *);

#define	SK_Identity(n) SG_MatrixIdentityv(&SKNODE(n)->T)
#define	SK_Translate(n,x,y) SG_MatrixTranslate2(&SKNODE(n)->T,(v).x,(v).y)
#define	SK_Translatev(n,v) SG_MatrixTranslate2(&SKNODE(n)->T,(v)->x,(v)->y)
#define	SK_Translate2(n,x,y) SG_MatrixTranslate2(&SKNODE(n)->T,(x),(y))
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SK_H_ */
