/*	Public domain	*/

#ifndef _AGAR_WIDGET_GLVIEW_H_
#define _AGAR_WIDGET_GLVIEW_H_

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL
#include <agar/gui/widget.h>
#include <agar/gui/begin.h>

typedef struct ag_glview {
	struct ag_widget wid;		/* AG_Widget -> AG_GLView */
	Uint flags;
#define AG_GLVIEW_HFILL		0x01
#define AG_GLVIEW_VFILL		0x02
#define AG_GLVIEW_NOMODELVIEW	0x04	/* Don't preserve modelview matrix */
#define AG_GLVIEW_NOTEXTURE	0x08	/* Don't preserve texture matrix */
#define AG_GLVIEW_NOCOLOR	0x10	/* Don't preserve color matrix */
#define AG_GLVIEW_INIT_MATRICES	0x20	/* For initialization */
#define AG_GLVIEW_RESHAPE	0x40	/* Matrices have changed */
#define AG_GLVIEW_BGFILL	0x80	/* Fill background */
#define AG_GLVIEW_EXPAND	(AG_GLVIEW_HFILL|AG_GLVIEW_VFILL)

	int wPre, hPre;			/* Initial geometry */
	Uint32 _pad1;
	AG_Event *_Nullable draw_ev;		/* Draw callback */
	AG_Event *_Nullable overlay_ev;		/* Graphics overlay callback */
	AG_Event *_Nullable underlay_ev;	/* Graphics underlay callback */
	AG_Event *_Nullable scale_ev;		/* Scaling/movement event */
	AG_Event *_Nullable keydown_ev;		/* Keyboard key pressed */
	AG_Event *_Nullable keyup_ev;		/* Keyboard key released */
	AG_Event *_Nullable btndown_ev;		/* Mouse button pressed */
	AG_Event *_Nullable btnup_ev;		/* Mouse button released */
	AG_Event *_Nullable motion_ev;		/* Mouse cursor moved */

	float mProjection[16];			/* Projection matrix to load */
	float mModelview[16];			/* Modelview matrix to load */
	float mTexture[16];			/* Texture matrix to load */

	AG_Color bgColor;			/* Background color */

#if AG_MODEL == AG_MEDIUM
	Uint32 _pad2;
#endif
} AG_GLView;

#define AGGLVIEW(obj)            ((AG_GLView *)(obj))
#define AGCGLVIEW(obj)           ((const AG_GLView *)(obj))
#define AG_GLVIEW_SELF()          AGGLVIEW( AG_OBJECT(0,"AG_Widget:AG_GLView:*") )
#define AG_GLVIEW_PTR(n)          AGGLVIEW( AG_OBJECT((n),"AG_Widget:AG_GLView:*") )
#define AG_GLVIEW_NAMED(n)        AGGLVIEW( AG_OBJECT_NAMED((n),"AG_Widget:AG_GLView:*") )
#define AG_CONST_GLVIEW_SELF()   AGCGLVIEW( AG_CONST_OBJECT(0,"AG_Widget:AG_GLView:*") )
#define AG_CONST_GLVIEW_PTR(n)   AGCGLVIEW( AG_CONST_OBJECT((n),"AG_Widget:AG_GLView:*") )
#define AG_CONST_GLVIEW_NAMED(n) AGCGLVIEW( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_GLView:*") )

__BEGIN_DECLS
extern AG_WidgetClass agGLViewClass;

AG_GLView *_Nonnull AG_GLViewNew(void *_Nullable, Uint);

void AG_GLViewSetBgColor(AG_GLView *_Nonnull, const AG_Color *_Nonnull);
void AG_GLViewSizeHint(AG_GLView *_Nonnull, int,int);
void AG_GLViewDrawFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewUnderlayFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewOverlayFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewScaleFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewKeydownFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewKeyupFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewButtondownFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewButtonupFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);
void AG_GLViewMotionFn(void *_Nonnull, _Nonnull AG_EventFn, const char *_Nullable, ...);

#ifndef AG_LEGACY
#define AG_GLViewPrescale(glv,w,h) AG_GLViewSizeHint((glv),(w),(h))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_OPENGL */
#endif /* _AGAR_WIDGET_GLVIEW_H_ */
