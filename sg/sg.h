/*	Public domain	*/

#ifndef _AGAR_SG_SG_H_
#define _AGAR_SG_SG_H_

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include <agar/gui/surface.h>
#include <agar/gui/opengl.h>

#if defined(_AGAR_SG_INTERNAL) || defined(_USE_SG_GL)
# include <GL/glu.h>
#endif

/* Definitions internal to the SG implementation */
#include <agar/sg/begin.h>

#if defined(_AGAR_SG_INTERNAL) || defined(_USE_SG_GL)
# include <agar/math/gl_macros.h>
#endif

#ifdef _AGAR_SG_INTERNAL
# define NODE SGNODE
# define NODE_OPS SGNODE_OPS
# undef MAX
# define MAX(h,i) ((h) > (i) ? (h) : (i))
# undef MIN
# define MIN(l,o) ((l) < (o) ? (l) : (o))
#endif /* _AGAR_SG_INTERNAL */

struct sg;
struct sg_node;
struct sg_point;
struct sg_camera;
struct sg_light;
struct sg_view;
struct ag_menu_item;
struct ag_widget;
struct sg_widget;

/* Side with respect to some line or plane */
enum sg_side {
	SG_LEFT,
	SG_RIGHT
};

/* Vertex winding */
enum sg_winding {
	SG_CW,
	SG_CCW
};

/* Packed OpenGL vertex structure. */
typedef struct sg_glvertex2 {
	GLdouble x, y;
} SG_GLVertex2;
typedef struct sg_glvertex3 {
	GLdouble x, y, z;
} SG_GLVertex3;

/*
 * Node Action structure. Actions dictate specific transformations to
 * be performed. All transformations must be invertible, and are
 * subject to interpolation.
 */
enum sg_action_type {
	SG_ACTION_NONE,
	SG_ACTION_MOVE,		SG_ACTION_MOVE_BEGIN,	SG_ACTION_MOVE_END,
	SG_ACTION_ZMOVE,	SG_ACTION_ZMOVE_BEGIN,	SG_ACTION_ZMOVE_END,
	SG_ACTION_ROTATE,	SG_ACTION_ROTATE_BEGIN,	SG_ACTION_ROTATE_END,
	SG_ACTION_SCALE,	SG_ACTION_SCALE_BEGIN,	SG_ACTION_SCALE_END,
	SG_ACTION_LAST
};
typedef struct sg_rotation {
	M_Vector3 axis;			/* Axis of rotation */
	M_Real theta;			/* Angle in radians */
	Uint8 _pad[8];
} SG_Rotation;
typedef struct sg_action {
	enum sg_action_type type;
	Uint flags;
#define SG_ACTION_SAVED 0
	AG_KeySym key;			/* Keyboard binding */
	AG_KeyMod keyMod;		/* Key modifier */
	union {
		M_Vector3 move;
		SG_Rotation rotate;
		M_Vector3 scale;
#ifdef _AGAR_SG_INTERNAL
# define act_move   args.move
# define act_rotate args.rotate
# define act_scale  args.scale
#endif
	} args;
	/* For editor */
	M_Matrix44 Torig;		/* Saved node transformation matrix */
	M_Vector3 vOrig;		/* Original intersection point */
	M_Line3 Rorig;			/* Control ray (gesture origin) */
	M_Line3 Rcur;			/* Control ray (gesture current) */
	AG_TAILQ_HEAD_(sg_widget) widgets; /* Active in-scene widgets */
	AG_TAILQ_ENTRY(sg_action) actions;
} SG_Action;

/* SG Node class-specific operations */
typedef struct sg_node_class {
	struct ag_object_class _inherit;   /* AG_ObjectClass -> SG_NodeClass */

	void  (*_Nullable menuInstance)(void *_Nonnull, struct ag_menu_item *_Nonnull,
	                                struct sg_view *_Nonnull);
	void  (*_Nullable menuClass)(struct sg *_Nonnull,
	                             struct ag_menu_item *_Nonnull,
	                             struct sg_view *_Nonnull);
	void  (*_Nullable draw)(void *_Nonnull, struct sg_view *_Nonnull);
	int   (*_Nullable intersect)(void *_Nonnull, M_Geom3, M_GeomSet3 *_Nonnull);
	void *_Nullable (*_Nullable edit)(void *_Nonnull, struct sg_view *_Nullable);
	int   (*_Nullable editor_action)(void *_Nonnull, enum sg_action_type,
	                                 struct sg_action *_Nonnull);
	int   (*_Nullable script_action)(void *_Nonnull,
	                                 struct sg_action *_Nonnull, int);
} SG_NodeClass;

/* SG Node object instance */
typedef struct sg_node {
	struct ag_object _inherit;	/* AG_Object -> SG_Node */
	Uint flags;
#define SG_NODE_SELECTED	0x01	/* Selection flag */
#define SG_NODE_HIDE		0x02	/* Disable rendering */

#ifdef AG_DEBUG
	Uint8 _pad1[12];
#else
	Uint8 _pad1[20];
#endif
	M_Matrix44 T;			/* Transformation from parent */
	struct sg *_Nullable sg;	/* Back pointer to sg */
	AG_TAILQ_ENTRY(sg_node) rnodes;	/* Used for quick inverse traversal */
	AG_TAILQ_ENTRY(sg_node) nodes;	/* For flat list */
	AG_TAILQ_HEAD_(sg_action) actions; /* Enabled node actions */
	Uint32 _pad2;
	Uint32 _pad3;
} SG_Node;

/* Scene graph object */
typedef struct sg {
	struct ag_object _inherit;	/* AG_Object -> SG */
	Uint flags;
#define SG_OVERLAY_WIREFRAME	0x01	/* Overlay scene wireframe */
#define SG_OVERLAY_VERTICES	0x02	/* Draw points at vertices */
#define SG_OVERLAY_VNORMALS	0x04	/* Draw vertex normals */
#define SG_OVERLAY_FNORMALS	0x08	/* Draw facet normals */
#define SG_SKIP_UNKNOWN_NODES	0x10	/* Ignore unknown node types in save
					   (otherwise LoadObject will fail) */
#define SG_NO_DEFAULT_NODES	0x20	/* Don't create default camera/lights */

	Uint32 _pad;
	SG_Node *_Nullable root;	/* Root of graph */

	struct {
		struct sg_camera *_Nullable cam;	/* Default camera */
		struct sg_light  *_Nullable lt[2];	/* Default lights */
	} def;
	AG_TAILQ_HEAD_(sg_node) nodes;	/* Flat list of nodes */
} SG;

#define SG_SELF()    (SG *)( AG_OBJECT(0,"SG:*") )
#define SG_PTR(n)    (SG *)( AG_OBJECT((n),"SG:*") )
#define SG_NAMED(n)  (SG *)( AG_OBJECT_NAMED((n),"SG:*") )

#define SG_NODE_SELF()    SGNODE( AG_OBJECT(0,"SG_Node:*") )
#define SG_NODE_PTR(n)    SGNODE( AG_OBJECT((n),"SG_Node:*") )
#define SG_NODE_NAMED(n)  SGNODE( AG_OBJECT_NAMED((n),"SG_Node:*") )

#define SGNODE(node) ((struct sg_node *)(node))
#define SGNODE_OPS(node) ((struct sg_node_class *)(AGOBJECT(node)->cls))
#define SGNODE_SELECTED(node) (((SG_Node *)(node))->flags & SG_NODE_SELECTED)

/* Iterate over attached nodes in the SG. */
#define SG_FOREACH_NODE(node, sg) \
	for((node) = AG_TAILQ_FIRST(&sg->nodes); \
	    (node) != AG_TAILQ_END(&sg->nodes); \
	    (node) = AG_TAILQ_NEXT(SGNODE(node), nodes))

/* Iterate over all selected nodes in the SG. */
#define SG_FOREACH_NODE_SELECTED(node, sg)			\
	SG_FOREACH_NODE((node),sg)				\
		if (!((node)->flags & SG_NODE_SELECTED)) {	\
			continue;				\
		} else

/* Built-in classes and subsystems */
#include <agar/sg/sg_dummy.h>
#include <agar/sg/sg_program.h>
/* #include <agar/sg/sg_cg_program.h> <- Legacy */
#include <agar/sg/sg_light.h>
#include <agar/sg/sg_camera.h>
#include <agar/sg/sg_texture.h>
#include <agar/sg/sg_palette.h>
#include <agar/sg/sg_geom.h>
#include <agar/sg/sg_widget.h>
#include <agar/sg/sg_point.h>
#include <agar/sg/sg_line.h>
#include <agar/sg/sg_circle.h>
#include <agar/sg/sg_sphere.h>
#include <agar/sg/sg_plane.h>
#include <agar/sg/sg_polygon.h>
#include <agar/sg/sg_triangle.h>
#include <agar/sg/sg_rectangle.h>
#include <agar/sg/sg_object.h>
#include <agar/sg/sg_polyball.h>
#include <agar/sg/sg_polybox.h>
#include <agar/sg/sg_voxel.h>
#include <agar/sg/sg_image.h>
#include <agar/sg/sg_script.h>

__BEGIN_DECLS
extern AG_ObjectClass sgClass;
extern SG_NodeClass sgNodeClass;
extern const char *_Nonnull sgActionNames[];

extern const AG_FileExtMapping sgFileExtMap[];
extern const Uint              sgFileExtCount;

void SG_InitSubsystem(void);
void SG_DestroySubsystem(void);

SG *_Nonnull    SG_New(void *_Nullable, const char *_Nullable, Uint);
void *_Nullable SG_Edit(void *_Nonnull);
void            SG_Clear(SG *_Nonnull);

SG_Node *_Nullable SG_SearchNodes(SG_Node *_Nonnull, const char *_Nonnull);
void *_Nonnull SG_FindNode(SG *_Nonnull, const char *_Nonnull);

int            SG_NodeLoad(SG *_Nonnull, AG_DataSource *_Nonnull, SG_Node *_Nullable);
int            SG_NodeSave(SG *_Nonnull, AG_DataSource *_Nonnull, SG_Node *_Nonnull);
void           SG_NodeDraw(SG *_Nonnull, SG_Node *_Nonnull, struct sg_view *_Nonnull);
void *_Nonnull SG_NodeEdit(void *_Nonnull);
void           SG_GetNodeTransform(void *_Nonnull, M_Matrix44 *_Nonnull);
void           SG_GetNodeTransformInverse(void *_Nonnull, M_Matrix44 *_Nonnull);

void SG_ActionInit(SG_Action *_Nonnull, enum sg_action_type);
void SG_ActionCopy(SG_Action *_Nonnull, const SG_Action *_Nonnull);
void SG_ActionPrint(const SG_Action *_Nonnull, char *_Nonnull, AG_Size);
int  SG_ActionLoad(SG_Action *_Nonnull, AG_DataSource *_Nonnull);
int  SG_ActionSave(SG_Action *_Nonnull, AG_DataSource *_Nonnull);

SG_Action *_Nullable SG_FetchAction(void *_Nonnull, enum sg_action_type);
int  SG_EnableAction(void *_Nonnull, enum sg_action_type);
void SG_DisableAction(void *_Nonnull, enum sg_action_type);

#define SG_Identity(n)		M_MatIdentity44v(&SGNODE(n)->T)

#define SG_Translatev(n,v)	M_MatTranslate44v(&SGNODE(n)->T,(v))
#define SG_Translate(n,x,y,z)	M_MatTranslate44(&SGNODE(n)->T,(x),(y),(z))
#define SG_TranslateX(n,t)	M_MatTranslate44X(&SGNODE(n)->T,(t))
#define SG_TranslateY(n,t)	M_MatTranslate44Y(&SGNODE(n)->T,(t))
#define SG_TranslateZ(n,t)	M_MatTranslate44Z(&SGNODE(n)->T,(t))

#define SG_Scale(n,s)		M_MatUniScale44(&SGNODE(n)->T,(s))

#define SG_Rotatev(n,a,d)	M_MatRotateAxis44(&SGNODE(n)->T,(a),(d))
#define SG_RotateI(n,d)		M_MatRotate44I(&SGNODE(n)->T,(d))
#define SG_RotateJ(n,d)		M_MatRotate44J(&SGNODE(n)->T,(d))
#define SG_RotateK(n,d)		M_MatRotate44K(&SGNODE(n)->T,(d))
#define SG_Rotatevd(n,a,v)	M_MatRotateAxis44(&SGNODE(n)->T,M_Radians(a),(v))
#define SG_RotateId(n,a)	M_MatRotate44I(&SGNODE(n)->T,M_Radians(a))
#define SG_RotateJd(n,a)	M_MatRotate44J(&SGNODE(n)->T,M_Radians(a))
#define SG_RotateKd(n,a)	M_MatRotate44K(&SGNODE(n)->T,M_Radians(a))

#define SG_Orbitv(n,p,a,d)	M_MatOrbitAxis44(&SGNODE(n)->T, \
				M_VecSub3((p),SG_NodePos(n)),(a),(d));
#define SG_Orbitvd(n,p,a,d)	SG_Orbitv((n),(p),(a),M_Radians(d))

/* Return the absolute (world) coordinates of a node. */
static __inline__ M_Vector3
SG_NodePos(void *_Nonnull p)
{
	SG_Node *node = (SG_Node *)p;
	M_Matrix44 T;
	M_Vector4 v = M_VECTOR4(0.0, 0.0, 0.0, 1.0);

	SG_GetNodeTransformInverse(node, &T);
	M_MatMultVector44v(&v, &T);
	return M_VECTOR3(v.x, v.y, v.z);
}

/* Return the absolute (world) direction of a node. */
static __inline__ M_Vector3
SG_NodeDir(void *_Nonnull p)
{
	SG_Node *node = (SG_Node *)p;
	M_Matrix44 T;
	M_Vector4 v = M_VECTOR4(0.0, 0.0, 1.0, 0.0);
	
	SG_GetNodeTransform(node, &T);
	M_MatMultVector44v(&v, &T);
	return M_VECTOR3(v.x, v.y, v.z);
}

/* Return the absolute scaling factor of a node. */
static __inline__ M_Real
SG_NodeSize(void *_Nonnull p)
{
	SG_Node *node = p;
	M_Matrix44 T;
	
	SG_GetNodeTransform(node, &T);
	return (T.m[0][0]);
}

/* Compute the intersection(s) between a node and a geometrical object.  */
static __inline__ int
SG_Intersect(SG_Node *_Nonnull node, M_Geom3 g, M_GeomSet3 *_Nonnull S)
{
	SG_NodeClass *nc = (SG_NodeClass *)AGOBJECT_CLASS(node);

	if (nc->intersect == NULL) {
		return (-1);
	}
	return nc->intersect(node, g, S);
}
__END_DECLS

#include <agar/sg/close.h>
#endif /* _AGAR_SG_SG_H_ */
