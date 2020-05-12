/*	Public domain	*/

#ifndef _AGAR_SK_SK_VIEW_H_
#define _AGAR_SK_SK_VIEW_H_
#include <agar/sk/begin.h>

#include <agar/sk/sk_tool.h>

#ifndef SK_VIEW_STATUS_MAX
#define SK_VIEW_STATUS_MAX 128
#endif

struct sk;
struct sk_point;
struct ag_pane;

typedef struct sk_view {
	struct ag_widget _inherit;

	Uint flags;
#define SK_VIEW_HFILL   0x01
#define SK_VIEW_VFILL   0x02
#define SK_VIEW_PANNING 0x04                    /* Panning in progress */
#define SK_VIEW_EXPAND (SK_VIEW_HFILL | SK_VIEW_VFILL)
	Uint32 _pad1;

	struct sk *_Nullable sk;		/* Sketch object */
#if !defined(AG_DEBUG)
	Uint32 _pad2;
	Uint32 _pad3;
#endif
	M_Matrix44 mView;			/* Viewing matrix */
	M_Matrix44 mProj;			/* Projection matrix */
	M_Real wPixel, hPixel;			/* Display pixel ratio */

	AG_Event *_Nullable scale_ev;		/* Scaling/movement event */
	AG_Event *_Nullable keydown_ev;		/* Keypress */
	AG_Event *_Nullable keyup_ev;		/* Key release */
	AG_Event *_Nullable btndown_ev;		/* Mouse button down event */
	AG_Event *_Nullable btnup_ev;		/* Mouse button release event */
	AG_Event *_Nullable motion_ev;		/* Mouse motion event */

	M_Vector3 mouseLast;			/* Last coordinates */

	SK_Tool *_Nullable curtool;		/* Selected tool */
	SK_Tool *_Nullable deftool;		/* Default tool if any */
	AG_PopupMenu *_Nullable popup;		/* Popup menu for context */
	struct ag_pane *_Nullable editPane;	/* Edition pane */
	struct ag_pane *_Nullable viewPane;	/* Visualization pane */
	struct ag_widget *_Nullable editBox;	/* Widget container */
	M_Real rSnap;				/* Snapping radius */
	AG_TAILQ_HEAD_(sk_tool) tools;		/* Sketching tools */
	AG_PopupMenu *_Nullable pmView;
	char status[SK_VIEW_STATUS_MAX];	/* Status text buffer */
} SK_View;

#define SKVIEW(obj)            ((SK_View *)(obj))
#define SKCVIEW(obj)           ((const SK_View *)(obj))
#define SK_VIEW_SELF()          SKVIEW( AG_OBJECT(0,"AG_Widget:SK_View:*") )
#define SK_VIEW_PTR(n)          SKVIEW( AG_OBJECT((n),"AG_Widget:SK_View:*") )
#define SK_VIEW_NAMED(n)        SKVIEW( AG_OBJECT_NAMED((n),"AG_Widget:SK_View:*") )
#define SK_CONST_VIEW_SELF()   SKCVIEW( AG_CONST_OBJECT(0,"AG_Widget:SK_View:*") )
#define SK_CONST_VIEW_PTR(n)   SKCVIEW( AG_CONST_OBJECT((n),"AG_Widget:SK_View:*") )
#define SK_CONST_VIEW_NAMED(n) SKCVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:SK_View:*") )

#define SK_VIEW_X(skv,px) ((M_Real)(px - (AGWIDGET(skv)->w >> 1))) / \
                          ((M_Real)WIDGET(skv)->w/2.0)
#define SK_VIEW_Y(skv,py) ((M_Real)(py - (AGWIDGET(skv)->h >> 1))) / \
                          ((M_Real)WIDGET(skv)->h/2.0)
#define SK_VIEW_X_SNAP(skv,px) (px)
#define SK_VIEW_Y_SNAP(skv,py) (py)
#define SK_VIEW_SCALE_X(skv) (skv)->mView.m[0][0]
#define SK_VIEW_SCALE_Y(skv) (skv)->mView.m[1][1]

__BEGIN_DECLS
extern AG_WidgetClass skViewClass;

SK_View	*_Nonnull SK_ViewNew(void *_Nullable, struct sk *_Nullable, Uint);

void SK_ViewZoom(SK_View *_Nonnull, M_Real);
void SK_ViewSelectTool(SK_View *_Nonnull, SK_Tool *_Nullable, void *_Nullable);

SK_Tool	*_Nullable SK_ViewFindTool(SK_View *_Nonnull, const char *_Nonnull);
SK_Tool	*_Nullable SK_ViewFindToolByOps(SK_View *_Nonnull, const SK_ToolOps *_Nonnull);
SK_Tool	*_Nonnull  SK_ViewRegTool(SK_View *_Nonnull, const SK_ToolOps *_Nonnull,
                                  void *_Nullable);

void SK_ViewSetDefaultTool(SK_View *_Nonnull, SK_Tool *_Nonnull);
void SK_ViewPopupMenu(SK_View *_Nonnull, int,int);
void SK_ViewClearEditPane(SK_View *_Nonnull);
void SK_ViewSetEditPane(SK_View *_Nonnull, struct ag_pane *_Nullable);
void SK_ViewSetViewPane(SK_View *_Nonnull, struct ag_pane *_Nullable);
void SK_ViewResizePanes(SK_View *_Nonnull);

struct sk_point *_Nullable SK_ViewOverPoint(SK_View *_Nonnull, M_Vector3 *_Nonnull,
                                            M_Vector3 *_Nonnull, void *_Nullable);
void *_Nullable SK_ViewGetNodeData(SK_View *_Nonnull, void *_Nonnull);
void            SK_ViewSetNodeData(SK_View *_Nonnull, void *_Nonnull,
                                   void *_Nullable);
__END_DECLS

#include <agar/sk/close.h>
#endif /* _AGAR_SK_SK_VIEW_H_ */
