/*	$Csoft: vg.h,v 1.41 2005/09/27 00:25:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_SG_H_
#define _AGAR_SG_H_

#include "begin_code.h"

#define SG_NAME_MAX 128
#define SG_DOUBLE_PRECISION

struct sg;
struct sg_node;

#include <agar/sg/sg_math.h>
#include <agar/sg/sg_vector.h>
#include <agar/sg/sg_matrix.h>
#include <agar/sg/sg_quaternion.h>
#include <agar/sg/sg_camera.h>

enum sg_node_type {
	SG_NODE_
};

typedef struct sg_node_ops {
	enum sg_node_type type;
	const char *name;
	void (*init)(struct sg *, struct sg_node *);
	void (*destroy)(struct sg *, struct sg_node *);
	void (*draw)(struct sg *, struct sg_node *);
} SG_NodeOps;

typedef struct sg_node {
	const SG_NodeOps *ops;
	u_int flags;
	SG_Matrix4 mTrans;		/* Transformation matrix */

	TAILQ_HEAD(,sg_node) cnodes;	/* Child nodes */
	TAILQ_ENTRY(sg_node) nodes;	/* Entry in parent's list */
} SG_Node;

typedef struct sg {
	struct ag_object obj;
	u_int flags;

	AG_Mutex lock;
	SG_Node *root;			/* Parent node */
} SG;

__BEGIN_DECLS
SG	*SG_New(void *, const char *);
void	 SG_Init(void *, const char *);
void	 SG_Reinit(void *);
void	 SG_Destroy(void *);
int	 SG_Save(void *, AG_Netbuf *);
int	 SG_Load(void *, AG_Netbuf *);
void	*SG_Edit(void *);
SG_Node	*SG_AddNode(SG_Node *, const SG_NodeOps *, u_int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_SG_H_ */
