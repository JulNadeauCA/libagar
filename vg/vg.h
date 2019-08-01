/*	Public domain	*/

#ifndef _AGAR_VG_H_
#define _AGAR_VG_H_

#include <agar/gui/text.h>

#include <agar/vg/begin.h>

#ifndef VG_NAME_MAX
#define VG_NAME_MAX		128
#endif
#ifndef VG_LAYER_NAME_MAX
#define VG_LAYER_NAME_MAX	128
#endif
#ifndef VG_STYLE_NAME_MAX
#define VG_STYLE_NAME_MAX	16
#endif
#ifndef VG_TYPE_NAME_MAX
#define VG_TYPE_NAME_MAX	32
#endif
#ifndef VG_SYM_NAME_MAX
#define VG_SYM_NAME_MAX		16
#endif
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

typedef struct vg_node_ops {
	const char *_Nonnull name;              /* Display text */
	struct ag_static_icon *_Nullable icon;  /* Display icon */
#ifdef AG_HAVE_64BIT
	Uint64 size;                            /* Instance size */
#else
	Uint size; 
#endif
	void  (*_Nullable init)(void *_Nonnull);
	void  (*_Nullable destroy)(void *_Nonnull);

	int   (*_Nullable load)(void *_Nonnull, AG_DataSource *_Nonnull,
	                        const AG_Version *_Nonnull);
	void  (*_Nullable save)(void *_Nonnull, AG_DataSource *_Nonnull);

	void  (*_Nonnull  draw)(void *_Nonnull, struct vg_view *_Nonnull);
	void  (*_Nullable extent)(void *_Nonnull, struct vg_view *_Nonnull,
	                          VG_Vector *_Nonnull, VG_Vector *_Nonnull);

	float (*_Nullable pointProximity)(void *_Nonnull, struct vg_view *_Nonnull,
					  VG_Vector *_Nonnull);
	float (*_Nullable lineProximity)(void *_Nonnull, struct vg_view *_Nonnull,
	                                 VG_Vector *_Nonnull, VG_Vector *_Nonnull);

	void  (*_Nullable deleteNode)(void *_Nonnull);
	void  (*_Nullable moveNode)(void *_Nonnull, VG_Vector, VG_Vector);

	void *_Nullable (*_Nullable edit)(void *_Nonnull, struct vg_view *_Nonnull);
} VG_NodeOps;

typedef struct vg_layer {
	char     name[VG_LAYER_NAME_MAX];	/* Layer name */
	int      visible;			/* Flag of visibility */
	VG_Color color;				/* Per-layer default color */
	Uint8    alpha;				/* Per-layer alpha value */
	Uint8   _pad[3];
} VG_Layer;

typedef struct vg_matrix {
	float m[3][3];
} VG_Matrix;

typedef struct vg_node {
	VG_NodeOps *_Nonnull ops;	/* Node class information */
	Uint32 handle;			/* Instance handle */
	char sym[VG_SYM_NAME_MAX];	/* Symbolic name */

	Uint flags;
#define VG_NODE_NOSAVE		0x01	/* Don't save with drawing */
#define VG_NODE_SELECTED	0x02	/* Selection flag */
#define VG_NODE_MOUSEOVER	0x04	/* Mouse overlap flag */
#define VG_NODE_SAVED_FLAGS	0

	struct vg      *_Nullable vg;     /* Back pointer to VG */
	struct vg_node *_Nullable parent; /* Back pointer to parent node */

	struct vg_node *_Nullable *_Nonnull refs;   /* Referenced nodes */
	Uint                               nRefs;   /* Referenced node count */

	Uint nDeps;			/* Dependency count */

	VG_Color  color;		/* Element color */
	int       layer;		/* Layer index */
	VG_Matrix T;			/* Transformation matrix */

	void *_Nullable p;		/* User pointer */

	AG_TAILQ_HEAD_(vg_node) cNodes;	/* Child nodes */
	AG_TAILQ_ENTRY(vg_node) tree;	/* Entry in tree */
	AG_TAILQ_ENTRY(vg_node) list;	/* Entry in global list */
	AG_TAILQ_ENTRY(vg_node) reverse; /* For VG_NodeTransform() */
	AG_TAILQ_ENTRY(vg_node) user;	/* Entry in user list */
} VG_Node;

#define VGNODE(p) ((VG_Node *)(p))

typedef struct vg {
	struct ag_object _inherit;		/* AG_Object -> VG */
	Uint flags;
#define VG_NO_ANTIALIAS	0x01			/* Disable anti-aliasing */
	Uint                      nColors;	/* Color count */
	VG_IndexedColor *_Nullable colors;	/* Global color table */
	VG_Color               fillColor;	/* Background color */
	VG_Color          selectionColor;	/* Selected item/block color */
	VG_Color          mouseoverColor;	/* Mouse overlap item color */

	VG_Layer *_Nullable layers;		/* Stack of layers */
	Uint	           nLayers;		/* Layer count */

	Uint               nT;			/* Matrix count */
	VG_Matrix *_Nonnull T;			/* Stack of matrices */

	VG_Node *_Nullable root;		/* Tree of entities */
	AG_TAILQ_HEAD_(vg_node) nodes;		/* List of entities */
	AG_TAILQ_ENTRY(vg) user;		/* Entry in user list */
} VG;

extern VG_NodeOps *_Nullable *_Nonnull vgNodeClasses;
extern Uint                            vgNodeClassCount;

extern int vgGUI;

#include <agar/vg/vg_math.h>

#define VG_NodeIsClass(p,pat) (strcmp(VGNODE(p)->ops->name, (pat)) == 0)

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
extern AG_ObjectClass vgClass;

extern const AG_FileExtMapping vgFileExtMap[];
extern const Uint              vgFileExtCount;

void VG_InitSubsystem(void);
void VG_DestroySubsystem(void);

VG *_Nonnull VG_New(Uint) _Warn_Unused_Result;

void VG_Clear(VG *_Nonnull);
void VG_ClearNodes(VG *_Nonnull);
void VG_ClearColors(VG *_Nonnull);

VG_NodeOps *_Nullable VG_LookupClass(const char *_Nonnull)
                                    _Warn_Unused_Result;

void VG_RegisterClass(VG_NodeOps *_Nonnull);
void VG_UnregisterClass(VG_NodeOps *_Nonnull);

void   VG_NodeInit(void *_Nonnull, VG_NodeOps *_Nonnull);
void   VG_NodeAttach(void *_Nullable, void *_Nonnull);
void   VG_NodeDetach(void *_Nonnull);
void   VG_NodeDestroy(void *_Nonnull);
int    VG_Delete(void *_Nonnull);
void   VG_Merge(void *_Nonnull, VG *_Nonnull);
void   VG_AddRef(void *_Nonnull, void *_Nonnull);
Uint   VG_DelRef(void *_Nonnull, void *_Nonnull);
void   VG_NodeTransform(void *_Nonnull, VG_Matrix *_Nonnull);
Uint32 VG_GenNodeName(VG *_Nonnull, const char *_Nonnull)
                     _Warn_Unused_Result;

void VG_SetBackgroundColor(VG *_Nonnull, VG_Color);
void VG_SetSelectionColor(VG *_Nonnull, VG_Color);
void VG_SetMouseOverColor(VG *_Nonnull, VG_Color);

VG_Layer *_Nonnull VG_PushLayer(VG *_Nonnull, const char *_Nonnull);
void               VG_PopLayer(VG *_Nonnull);

void VG_SetSym(void *_Nonnull, const char *_Nonnull, ...)
	      FORMAT_ATTRIBUTE(printf,2,3);
                   
void VG_SetLayer(void *_Nonnull, int);
void VG_SetColorv(void *_Nonnull, const VG_Color *_Nonnull);
void VG_SetColorRGB(void *_Nonnull, Uint8, Uint8, Uint8);
void VG_SetColorRGBA(void *_Nonnull, Uint8, Uint8, Uint8, Uint8);

VG_Vector VG_ReadVector(AG_DataSource *_Nonnull);
void      VG_WriteVector(AG_DataSource *_Nonnull, const VG_Vector *_Nonnull);

VG_Color  VG_ReadColor(AG_DataSource *_Nonnull);
void      VG_WriteColor(AG_DataSource *_Nonnull, const VG_Color *_Nonnull);

void            VG_WriteRef(AG_DataSource *_Nonnull, void *_Nonnull);
void *_Nullable VG_ReadRef(AG_DataSource *_Nonnull, void *_Nonnull,
                           const char *_Nullable);

void *_Nullable VG_PointProximity(struct vg_view *_Nonnull, const char *_Nullable,
                                  const VG_Vector *_Nonnull, VG_Vector *_Nullable,
                                  void *_Nullable);

void *_Nullable VG_PointProximityMax(struct vg_view *_Nonnull, const char *_Nullable,
                                     const VG_Vector *_Nonnull, VG_Vector *_Nullable,
                                     void *_Nullable, float);

VG_Matrix VG_MatrixInvert(VG_Matrix);
VG_Color  VG_GetColorRGB(Uint8, Uint8, Uint8);
VG_Color  VG_GetColorRGBA(Uint8, Uint8, Uint8, Uint8);
AG_Color  VG_MapColorRGB(VG_Color);
AG_Color  VG_MapColorRGBA(VG_Color);
void      VG_BlendColors(VG_Color *_Nonnull, VG_Color);

void *_Nullable VG_FindNodeSym(VG *_Nonnull, const char *_Nonnull);
void *_Nullable VG_FindNode(VG *_Nonnull, Uint32, const char *_Nonnull);

void VG_PushMatrix(VG *_Nonnull);
void VG_PopMatrix(VG *_Nonnull);
void VG_LoadIdentity(void *_Nonnull);
void VG_SetPositionInParent(void *_Nonnull, VG_Vector);
void VG_Translate(void *_Nonnull, VG_Vector);
void VG_Scale(void *_Nonnull, float);
void VG_Rotate(void *_Nonnull, float);
void VG_FlipVert(void *_Nonnull);
void VG_FlipHoriz(void *_Nonnull);

void VG_Select(void *_Nonnull);
void VG_Unselect(void *_Nonnull);
void VG_SelectAll(VG *_Nonnull);
void VG_UnselectAll(VG *_Nonnull);

VG_Vector VG_Pos(void *_Nonnull);
void      VG_SetPosition(void *_Nonnull, VG_Vector);
__END_DECLS

#include <agar/vg/vg_point.h>
#include <agar/vg/vg_line.h>
#include <agar/vg/vg_circle.h>
#include <agar/vg/vg_arc.h>
#include <agar/vg/vg_text.h>
#include <agar/vg/vg_polygon.h>

#include <agar/vg/close.h>
#endif /* _AGAR_VG_H_ */
