/*	Public domain	*/

#ifndef _AGAR_WIDGET_GLVIEW_H_
#define _AGAR_WIDGET_GLVIEW_H_

#include <agar/gui/widget.h>
#include <agar/config/have_opengl.h>

#ifdef HAVE_OPENGL
#include <agar/gui/begin.h>

typedef struct ag_glview {
	struct ag_widget wid;
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

	int	  wPre, hPre;			/* Initial geometry */

	AG_Event *draw_ev;			/* Draw callback */
	AG_Event *overlay_ev;			/* Graphics overlay callback */
	AG_Event *underlay_ev;			/* Graphics underlay callback */
	AG_Event *scale_ev;			/* Scaling/movement event */
	AG_Event *keydown_ev, *keyup_ev;	/* Keyboard events */
	AG_Event *btndown_ev, *btnup_ev;	/* Mouse button events */
	AG_Event *motion_ev;			/* Mouse motion event */

	float mProjection[16];			/* Projection matrix to load */
	float mModelview[16];			/* Modelview matrix to load */
	float mTexture[16];			/* Texture matrix to load */

	AG_Color bgColor;			/* Background color */
} AG_GLView;

#define AGGLVIEW(p) ((AG_GLView *)(p))

__BEGIN_DECLS
extern AG_WidgetClass agGLViewClass;

AG_GLView *AG_GLViewNew(void *, Uint);
void	   AG_GLViewDraw(void *);
void	   AG_GLViewSizeRequest(void *, AG_SizeReq *);
int	   AG_GLViewSizeAllocate(void *, const AG_SizeAlloc *);

void       AG_GLViewSetBgColor(AG_GLView *, AG_Color);
void	   AG_GLViewSizeHint(AG_GLView *, int, int);
#define	   AG_GLViewPrescale AG_GLViewSizeHint
void	   AG_GLViewReshape(AG_GLView *);
void	   AG_GLViewDrawFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewUnderlayFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewOverlayFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewScaleFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeydownFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeyupFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtondownFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtonupFn(void *, AG_EventFn, const char *, ...);
void	   AG_GLViewMotionFn(void *, AG_EventFn, const char *, ...);
__END_DECLS

#include <agar/gui/close.h>
#endif /* HAVE_OPENGL */
#endif /* _AGAR_WIDGET_GLVIEW_H_ */
