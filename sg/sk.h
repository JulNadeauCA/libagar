/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_SK_H_
#define _AGAR_SK_H_

#ifndef _AGAR_SG_PUBLIC
#include <config/edition.h>
#endif

#include <agar/sg/sg.h>
#include <agar/sg/sk_view.h>

#include "begin_code.h"

#define SK_TYPE_NAME_MAX 128

struct sk;
struct sk_node;
struct sk_point;
struct ag_widget;

typedef struct sk_node_ops {
	const char *name;
	size_t size;
	Uint flags;
	void (*init)(void *);
	void (*destroy)(void *);
	int (*load)(void *, AG_Netbuf *);
	int (*save)(void *, AG_Netbuf *);
	void (*draw_relative)(void *, SK_View *);
	void (*draw_absolute)(void *, SK_View *);
} SK_NodeOps;

typedef struct sk_node {
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
	SLIST_ENTRY(sk_node) rnodes;	/* Reverse entry (optimization) */
} SK_Node;

typedef struct sk_constraint {
	enum {
		SK_COINCIDENT_PP,	/* Point/point coincident */
		SK_COINCIDENT_PL,	/* Point/line coincident */
		SK_COINCIDENT_PA,	/* Point/arc coincident */
		SK_DISTANCE_PP,		/* Point/point distance */
		SK_DISTANCE_PL,		/* Point/line distance */
		SK_ANGLE_LL,		/* Line/line angle */
		SK_PARALLEL_LL,		/* Line/line parallelism */
		SK_PERPENDICULAR_LL	/* Line/line perpendicularity */
	} type;
	SK_Node *e1;				/* Entity 1 */
	SK_Node *e2;				/* Entity 2 */
	TAILQ_ENTRY(sk_constraint) cgraph;	/* Entry in graph */
} SK_Constraint;

typedef struct sk {
	struct ag_object obj;
	Uint flags;
	AG_Mutex lock;
	struct sk_point *root;			/* Root of transform tree */
	TAILQ_HEAD(,sk_node) nodes;		/* List of entities */
	TAILQ_HEAD(,sk_constraint) cgraph;	/* Constraint graph */
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

#include <agar/sg/sk_dummy.h>
#include <agar/sg/sk_point.h>
#include <agar/sg/sk_line.h>
#include <agar/sg/sk_circle.h>
#include <agar/sg/sk_arc.h>

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
void		 SK_NodeInit(void *, const void *, Uint);
void		*SK_NodeAdd(void *, const SK_NodeOps *, Uint);
void		 SK_NodeAttach(void *, void *);
void		 SK_NodeDetach(void *, void *);
int		 SK_NodeSave(SK *, SK_Node *, AG_Netbuf *);
int		 SK_NodeLoad(SK *, SK_Node **, AG_Netbuf *);
void		 SK_GetNodeTransform(void *, SG_Matrix *);
SG_Vector	 SK_NodeCoords(void *);
SG_Vector	 SK_NodeDir(void *);
void		 SK_NodeAddReference(void *, void *);
#define	SK_Identity(n) SG_MatrixIdentityv(&SKNODE(n)->T)
#define	SK_Translate(n,x,y) SG_MatrixTranslate2(&SKNODE(n)->T,(v).x,(v).y)
#define	SK_Translatev(n,v) SG_MatrixTranslate2(&SKNODE(n)->T,(v)->x,(v)->y)
#define	SK_Translate2(n,x,y) SG_MatrixTranslate2(&SKNODE(n)->T,(x),(y))

__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SK_H_ */
