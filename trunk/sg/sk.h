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
#define SK_STATUS_MAX	 2048

struct sk;
struct sk_node;
struct sk_point;
struct sk_constraint;
struct ag_widget;

typedef enum sk_status {
	SK_INVALID,			/* No solutions */
	SK_WELL_CONSTRAINED,		/* Finite number of solution */
	SK_UNDER_CONSTRAINED,		/* Infinite number of solutions */
	SK_OVER_CONSTRAINED		/* Redundant constraints */
} SK_Status;

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
	void (*redraw)(void *, SK_View *);
	void (*edit)(void *, struct ag_widget *, SK_View *);
	SG_Real (*proximity)(void *, const SG_Vector *, SG_Vector *);
	int (*del)(void *);
	int (*move)(void *, const SG_Vector *, const SG_Vector *);
	SK_Status (*constrained)(void *);
} SK_NodeOps;

typedef struct sk_node {
	char name[SK_NODE_NAME_MAX];
	Uint32 handle;			/* Unique handle for this node class */
	const SK_NodeOps *ops;
	Uint flags;
#define SK_NODE_SELECTED	0x01	/* For editor */
#define SK_NODE_MOUSEOVER	0x02	/* For editor */
#define SK_NODE_MOVED		0x04	/* For editor */
#define SK_NODE_UNCONSTRAINED	0x08	/* Suppress constraints */
#define SK_NODE_FIXED		0x10	/* Treat position as known */
#define SK_NODE_KNOWN		0x20	/* Position found by solver */
#define SK_NODE_CHECKED		0x40	/* Skip constraint check */

	struct sk *sk;			/* Back pointer to sk */
	struct sk_node *pNode;		/* Back pointer to parent node */
	SG_Matrix T;			/* Transformation from parent */
	TAILQ_HEAD(,sk_node) cnodes;	/* Siblings */
	Uint nRefs;			/* Reference count (optimization) */
	struct sk_node **refNodes;	/* References to other nodes */
	Uint nRefNodes;
	struct sk_constraint **cons;	/* Constraint edges (optimization) */
	Uint nCons;
	Uint nEdges;			/* For solver (optimization) */
	TAILQ_ENTRY(sk_node) sknodes;	/* Entry in transformation tree */
	TAILQ_ENTRY(sk_node) nodes;	/* Entry in flat node list */
	TAILQ_ENTRY(sk_node) rnodes;	/* Reverse entry (optimization) */
} SK_Node;

enum sk_constraint_type {
	SK_DISTANCE,
	SK_INCIDENT,		/* Translated to DISTANCE(0) */
	SK_ANGLE,
	SK_PERPENDICULAR,	/* Translated to ANGLE(90deg) */
	SK_PARALLEL,		/* Translated to ANGLE(0deg) */
	SK_TANGENT,
	SK_CONSTRAINT_LAST
#define	SK_CONSTRAINT_ANY SK_CONSTRAINT_LAST
};

typedef struct sk_constraint {
	enum sk_constraint_type uType;	/* Constraint type (user-provided) */
	enum sk_constraint_type type;	/* Constraint type (translated) */
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

typedef struct sk_cluster {
	Uint32 name;
	TAILQ_HEAD(,sk_constraint) edges;
	TAILQ_ENTRY(sk_cluster) clusters;
} SK_Cluster;

typedef struct sk_insn {
	enum sk_insn_type { 
		SK_COMPOSE_PAIR,	/* Find n2 from n1 */
		SK_COMPOSE_RING		/* Find n3 from n1 and n2, assuming
					   (n1,n2,n3) is a constrained ring */
	} type;
	SK_Node *n[3];			/* Nodes (n0 = unknown) */
	SK_Constraint *ct01, *ct02;	/* Constraints */
	TAILQ_ENTRY(sk_insn) insns;
} SK_Insn;

typedef struct sk {
	struct ag_object obj;
	Uint flags;
#define SK_SKIP_UNKNOWN_NODES	0x01		/* Ignore unimplemented nodes
						   in load (otherwise fail) */
	AG_Mutex lock;
	const AG_Unit *uLen;			/* Length unit */
	SK_Node *root;				/* Root node */
	TAILQ_HEAD(,sk_node) nodes;		/* Flat node list */
	SK_Status status;			/* Constrainedness status */
	char statusText[SK_STATUS_MAX];		/* Status text */
	Uint nSolutions;			/* Total number of solutions
						   (if well-constrained) */

	/* For internal use by constraint solver */
	SK_Cluster ctGraph;			/* Original constraint graph */
	TAILQ_HEAD(,sk_cluster) clusters;	/* Rigid clusters */
	TAILQ_HEAD(,sk_insn) insns;		/* Construction steps */
} SK;

#define SKNODE(node) ((SK_Node *)(node))
#define SKNODE_SELECTED(node) (((SK_Node *)(node))->flags & SK_NODE_SELECTED)
#define SK_MOVED(node) (((SK_Node *)(node))->flags & SK_NODE_MOVED)
#define SK_FIXED(node) (((SK_Node *)(node))->flags & SK_NODE_FIXED)
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
#include <sg/sk_annot.h>
#include <sg/sk_dimension.h>
#else
#include <agar/sg/sk_dummy.h>
#include <agar/sg/sk_point.h>
#include <agar/sg/sk_line.h>
#include <agar/sg/sk_circle.h>
#include <agar/sg/sk_arc.h>
#include <agar/sg/sk_annot.h>
#include <agar/sg/sk_dimension.h>
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
int	 	 SK_NodeOfClass(void *, const char *);
void		 SK_NodeInit(void *, const void *, Uint32, Uint);
void		*SK_NodeAdd(void *, const SK_NodeOps *, Uint32, Uint);
int		 SK_NodeDel(void *);
void		 SK_NodeAttach(void *, void *);
void		 SK_NodeDetach(void *, void *);
int		 SK_NodeLoadGeneric(SK *, SK_Node **, AG_Netbuf *);
void		 SK_GetNodeTransform(void *, SG_Matrix *);
void		 SK_GetNodeTransformInverse(void *, SG_Matrix *);
void		 SK_NodeAddReference(void *, void *);
void		 SK_NodeDelReference(void *, void *);
void		 SK_NodeAddConstraint(void *, SK_Constraint *);
void		 SK_NodeDelConstraint(void *, SK_Constraint *);
Uint32		 SK_GenNodeName(SK *, const char *);
Uint32		 SK_GenClusterName(SK *);
SG_Color	 SK_NodeColor(void *, const SG_Color *);
void		 SK_NodeRedraw(void *, SK_View *);

void		*SK_ReadRef(AG_Netbuf *, SK *, const char *);
void		 SK_WriteRef(AG_Netbuf *, void *);
void		 SK_SetLengthUnit(SK *, const AG_Unit *);
void		*SK_ProximitySearch(SK *, const char *, SG_Vector *,
		                    SG_Vector *, void *);

__inline__ void	SK_Update(SK *);
int		SK_Solve(SK *);
void		SK_FreeClusters(SK *);
void		SK_FreeInsns(SK *);
void		SK_InitCluster(SK_Cluster *, Uint32);
void		SK_FreeCluster(SK_Cluster *);
void		SK_CopyCluster(const SK_Cluster *, SK_Cluster *);
int		SK_NodeInCluster(const SK_Node *, const SK_Cluster *);
SK_Constraint  *SK_AddConstraint(SK_Cluster *, void *, void *,
		                 enum sk_constraint_type, ...);
SK_Constraint  *SK_AddConstraintCopy(SK_Cluster *,
	                             const SK_Constraint *);
SK_Constraint  *SK_DupConstraint(const SK_Constraint *);
void		SK_DelConstraint(SK_Cluster *, SK_Constraint *);
int		SK_DelSimilarConstraint(SK_Cluster *,
		                        const SK_Constraint *);
int		SK_CompareConstraints(const SK_Constraint *,
		                      const SK_Constraint *);
Uint		SK_ConstraintsToSubgraph(const SK_Cluster *, const SK_Node *,
                                         const SK_Cluster *,
					 SK_Constraint *[2]);
SK_Insn	       *SK_AddInsn(SK *, enum sk_insn_type, ...);
int		SK_ExecInsn(SK *, const SK_Insn *);
int		SK_ExecProgram(SK *);
void		SK_SetStatus(SK *, SK_Status, const char *, ...);
void		SK_ClearProgramState(SK *);

__inline__ SG_Vector	  SK_Pos(void *);
__inline__ SG_Vector	  SK_NodeDir(void *);
__inline__ void		 *SK_FindNode(SK *, Uint32, const char *);
__inline__ void		 *SK_FindNodeByName(SK *, const char *);
__inline__ SK_Cluster	 *SK_FindCluster(SK *, Uint32);
__inline__ SK_Constraint *SK_FindConstraint(const SK_Cluster *,
	                                    enum sk_constraint_type, void *,
					    void *);
__inline__ SK_Constraint *SK_FindSimilarConstraint(const SK_Cluster *,
		                                   const SK_Constraint *);
__inline__ SK_Constraint *SK_ConstrainedNodes(const SK_Cluster *,
			                      const SK_Node *, const SK_Node *);

#define	SK_Identity(n)		MatIdentityv(&SKNODE(n)->T)
#define	SK_Translate(n,x,y)	MatTranslate3(&SKNODE(n)->T,(v).x,(v).y,0.0)
#define	SK_Translatev(n,v)	MatTranslate3(&SKNODE(n)->T,(v)->x,(v)->y,0.0)
#define	SK_Translate2(n,x,y)	MatTranslate3(&SKNODE(n)->T,(x),(y),0.0)
#define	SK_Rotatev(n,theta)	MatRotateZv(&SKNODE(n)->T,(theta))
#define	SK_MatrixCopy(nDst,nSrc) MatCopy(&SKNODE(nDst)->T,&SKNODE(nSrc)->T);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SK_H_ */
