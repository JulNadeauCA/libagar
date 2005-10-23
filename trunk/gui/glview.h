/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GLVIEW_H_
#define _AGAR_WIDGET_GLVIEW_H_

#include <agar/config/have_opengl.h>

#ifdef HAVE_OPENGL

#include <agar/gui/widget.h>
#include "begin_code.h"

typedef struct ag_glview {
	struct ag_widget wid;
	Uint flags;
#define AG_GLVIEW_WFILL		0x01
#define AG_GLVIEW_HFILL		0x02
#define AG_GLVIEW_NOMODELVIEW	0x04	/* Don't preserve modelview matrix */
#define AG_GLVIEW_NOTEXTURE	0x08	/* Don't preserve texture matrix */
#define AG_GLVIEW_NOCOLOR	0x10	/* Don't preserve color matrix */
#define AG_GLVIEW_FOCUS		0x20
#define AG_GLVIEW_EXPAND	(AG_GLVIEW_WFILL|AG_GLVIEW_HFILL)

	AG_Event *draw_ev;			/* Draw callback */
	AG_Event *scale_ev;			/* Scaling/movement event */
	AG_Event *keydown_ev, *keyup_ev;	/* Keyboard events */
	AG_Event *btndown_ev, *btnup_ev;	/* Mouse button events */
	AG_Event *motion_ev;			/* Mouse motion event */

	double mProjection[16];			/* Projection matrix to load */
	double mModelview[16];			/* Modelview matrix to load */
	double mTexture[16];			/* Texture matrix to load */
	double mColor[16];			/* Color matrix to load */
} AG_GLView;

#define AGGLVIEW(p) ((AG_GLView *)(p))

__BEGIN_DECLS
AG_GLView *AG_GLViewNew(void *, Uint);
void	   AG_GLViewInit(AG_GLView *, Uint);
void	   AG_GLViewDestroy(void *);
void	   AG_GLViewDraw(void *);
void	   AG_GLViewScale(void *, int, int);
void	   AG_GLViewReshape(AG_GLView *);
void	   AG_GLViewDrawFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewScaleFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeydownFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeyupFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtondownFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtonupFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewMotionFn(AG_GLView *, AG_EventFn, const char *, ...);
__END_DECLS

#endif /* HAVE_OPENGL */
#include "close_code.h"
#endif /* _AGAR_WIDGET_GLVIEW_H_ */
