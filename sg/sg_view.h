/*	Public domain	*/

#ifndef _AGAR_SG_SG_VIEW_H_
#define _AGAR_SG_SG_VIEW_H_
#include <agar/sg/begin.h>

struct sg_camera;

#ifndef SG_VIEW_STATUS_MAX
#define SG_VIEW_STATUS_MAX 124
#endif

/* Managed per-view texture. */
typedef struct sg_view_texture {
	void *_Nonnull node;			/* Pointer to parent node */
	struct sg_view *_Nonnull sv;		/* Pointer to SG_View */
	Uint name;				/* GL texture handle */
	int frame;				/* Animation frame (optional) */
	AG_TexCoord tc;				/* Texture coordinates */
	AG_TAILQ_ENTRY(sg_view_texture) textures;
} SG_ViewTexture;

/* Managed per-view display list. */
typedef struct sg_view_list {
	void *_Nonnull node;			/* Pointer to parent node */
	struct sg_view *_Nonnull sv;		/* Pointer to SG_View */
	Uint name;				/* GL display list handle */
	int frame;				/* Animation frame (optional) */
	AG_TAILQ_ENTRY(sg_view_list) lists;
} SG_ViewList;

typedef struct sg_view_cam_action {
	M_Real incr;			/* Increment */
	Uint32 vMin, vAccel, vMax;	/* Repeat delays */
	Uint8 _pad[12];
	M_Vector3 v;			/* Axis or direction */
} SG_ViewCamAction;

typedef struct sg_view {
	struct ag_widget _inherit;	/* AG_Widget -> SG_View */

	Uint flags;
#define SG_VIEW_HFILL		0x001
#define SG_VIEW_VFILL		0x002
#define SG_VIEW_EXPAND		(SG_VIEW_HFILL|SG_VIEW_VFILL)
#define SG_VIEW_NO_LIGHTING	0x004 /* Disable lighting */
#define SG_VIEW_NO_DEPTH_TEST	0x008 /* Disable Z-buffering */
#define SG_VIEW_UPDATE_PROJ	0x010 /* Update projection at next draw */
#define SG_VIEW_PANNING		0x020 /* Mouse panning in progress */
#define SG_VIEW_CAMERA_STATUS	0x040 /* Display camera status overlay */
#define SG_VIEW_EDIT		0x080 /* Allow edition commands */
#define SG_VIEW_EDIT_STATUS	0x080 /* Display edition status overlay */
#define SG_VIEW_MOVING		0x100 /* Moving object */
#define SG_VIEW_ROTATING	0x200 /* Rotating object */

	Uint transFlags;
#define SG_VIEW_TRANSFADE	0x01	/* Fade-out/fade-in */

	SG *_Nullable sg;		/* Scene graph */
	SG *_Nullable sgTrans;		/* For SG_ViewTransition() */

	AG_Timer toTransFade;		/* Timer for fade */
	float    transProgress;		/* Transition state */
	AG_Color transFadeColor;	/* Color for fade */
	Uint     transDuration;		/* Duration for transition (ms) */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad1;
#endif
	struct sg_camera *_Nullable cam;	/* Current camera */
	struct sg_camera *_Nullable camTrans;	/* Transitioning to camera */
#ifdef AG_DEBUG
	Uint32 _pad2;
	Uint32 _pad3;
#endif
	M_Vector3 rSens;		/* Rotational sensitivity (mouse) */
	M_Vector3 tSens;		/* Translational sensitivity (mouse) */

	AG_Widget *_Nonnull editArea;	/* Edit container widget */
	SG_Node   *_Nonnull editNode;	/* Node being edited */

	SG_ViewCamAction rot[3];	/* Keyboard controlled yaw/pitch/roll */
	SG_ViewCamAction move[3];	/* Keyboard controlled translate */
	AG_Timer toRot, toMove;
	AG_KeySym lastKeyDown;		/* Last pressed key */

	char editStatus[SG_VIEW_STATUS_MAX];	/* Status text */

	AG_PopupMenu *_Nonnull pmView;	/* Popup menu per view */
	AG_PopupMenu *_Nonnull pmNode;	/* Popup menu per node */
	AG_Timer toRefresh;		/* View refresh timer */
} SG_View;

#define SGVIEW(obj)            ((SG_View *)(obj))
#define SGCVIEW(obj)           ((const SG_View *)(obj))
#define SG_VIEW_SELF()          SGVIEW( AG_OBJECT(0,"AG_Widget:SG_View:*") )
#define SG_VIEW_PTR(n)          SGVIEW( AG_OBJECT((n),"AG_Widget:SG_View:*") )
#define SG_VIEW_NAMED(n)        SGVIEW( AG_OBJECT_NAMED((n),"AG_Widget:SG_View:*") )
#define SG_CONST_VIEW_SELF()   SGCVIEW( AG_CONST_OBJECT(0,"AG_Widget:SG_View:*") )
#define SG_CONST_VIEW_PTR(n)   SGCVIEW( AG_CONST_OBJECT((n),"AG_Widget:SG_View:*") )
#define SG_CONST_VIEW_NAMED(n) SGCVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:SG_View:*") )

__BEGIN_DECLS
extern AG_WidgetClass sgViewClass;
extern M_Real sgEyeSeparation;

SG_View	*_Nonnull SG_ViewNew(void *_Nullable, SG *_Nullable, Uint);

int  SG_ViewTransition(SG_View *_Nonnull, SG *_Nonnull, SG_Camera *_Nullable,
                       Uint);
void SG_ViewSetFadeColor(SG_View *_Nonnull, const AG_Color *_Nonnull);
void SG_ViewSetFadeDuration(SG_View *_Nonnull, Uint);
void SG_ViewUnProject(SG_View *_Nonnull, int,int, M_Vector4 *_Nonnull,
                      M_Vector4 *_Nonnull);
void SG_ViewSetCamera(SG_View *_Nonnull, struct sg_camera *_Nullable);
__END_DECLS

#include <agar/sg/close.h>
#endif /* _AGAR_SG_SG_VIEW_H_ */
