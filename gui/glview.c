/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * OpenGL context widget. No longer needed as of Agar 1.5 (replaced by
 * the USE_OPENGL feature of the base AG_Widget(3) class), but kept for
 * backward compatibility.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS
#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/gui/glview.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/opengl.h>

#include <stdarg.h>

static void Reshape(AG_GLView *);

AG_GLView *
AG_GLViewNew(void *parent, Uint flags)
{
	AG_GLView *glv;

	glv = Malloc(sizeof(AG_GLView));
	AG_ObjectInit(glv, &agGLViewClass);

	if (flags & AG_GLVIEW_HFILL) { WIDGET(glv)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_GLVIEW_VFILL) { WIDGET(glv)->flags |= AG_WIDGET_VFILL; }
	glv->flags |= flags;

	AG_ObjectAttach(parent, glv);
	return (glv);
}

/* Initialize an OpenGL matrix to identity. GL must be locked. */
static void
SetIdentity(GLfloat *_Nonnull M, GLenum which)
{
	glMatrixMode(which);
	glPushMatrix();
	glLoadIdentity();
	glGetFloatv(which, M);
	glPopMatrix();
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_GLView *glv = AG_GLVIEW_SELF();

	if ((WIDGET(glv)->flags & AG_WIDGET_FOCUSABLE) &&
	    !AG_WidgetIsFocused(glv))
		AG_WidgetFocus(glv);
}

static void
OnAttach(AG_Event *_Nonnull event)
{
	AG_Widget *parent = AG_PTR(1);

	AG_OBJECT_ISA(parent, "AG_Widget:*");

	if (parent->drv != NULL &&
	    !(AGDRIVER_CLASS(parent->drv)->flags & AG_DRIVER_OPENGL))
		AG_FatalError("AG_GLView requires a driver with GL support");
}

static void
Init(void *_Nonnull obj)
{
	AG_GLView *glv = obj;

	WIDGET(glv)->flags |= AG_WIDGET_FOCUSABLE;
	
	glv->flags = AG_GLVIEW_INIT_MATRICES;
	glv->wPre = 100;
	glv->hPre = 100;
	memset(&glv->draw_ev, 0, sizeof(AG_Event *) + /* draw_ev */
	                         sizeof(AG_Event *) + /* overlay_ev */
	                         sizeof(AG_Event *) + /* underlay_ev */
	                         sizeof(AG_Event *) + /* scale_ev */
	                         sizeof(AG_Event *) + /* keydown_ev */
	                         sizeof(AG_Event *) + /* keyup_ev */
	                         sizeof(AG_Event *) + /* btndown_ev */
	                         sizeof(AG_Event *) + /* btnup_ev */
	                         sizeof(AG_Event *)); /* motion_ev */
	
	AG_ColorBlack(&glv->bgColor);

	AG_SetEvent(glv, "mouse-button-down", MouseButtonDown, NULL);
	AG_AddEvent(glv, "attached", OnAttach, NULL);
}

/* Set an initial size requisition in pixels. */
void
AG_GLViewSizeHint(AG_GLView *glv, int w, int h)
{
	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	glv->wPre = w;
	glv->hPre = h;
}

/* Register a rendering routine. */
void
AG_GLViewDrawFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->draw_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->draw_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a rendering callback routine (before draw). */
void
AG_GLViewUnderlayFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->underlay_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->underlay_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a rendering callback routine (post-draw). */
void
AG_GLViewOverlayFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->overlay_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->overlay_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a callback routine to run whenever the widget is resized. */
void
AG_GLViewScaleFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->scale_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->scale_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a "key-down" (key pressed) callback routine. */
void
AG_GLViewKeydownFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->keydown_ev = AG_SetEvent(glv, "key-down", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->keydown_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a "key-up" (key released) callback routine. */
void
AG_GLViewKeyupFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->keyup_ev = AG_SetEvent(glv, "key-up", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->keyup_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a "mouse-button-down" callback routine. */
void
AG_GLViewButtondownFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->btndown_ev = AG_SetEvent(glv, "mouse-button-down", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->btndown_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a "mouse-button-up" callback routine. */
void
AG_GLViewButtonupFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->btnup_ev = AG_SetEvent(glv, "mouse-button-up", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->btnup_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

/* Register a "mouse-motion" callback routine. */
void
AG_GLViewMotionFn(void *obj, AG_EventFn fn, const char *fmt, ...)
{
	AG_GLView *glv = obj;

	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	glv->motion_ev = AG_SetEvent(glv, "mouse-motion", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(glv->motion_ev, fmt, ap);
		va_end(ap);
	}

	AG_ObjectUnlock(glv);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_GLView *glv = obj;

	if (a->w < 1 || a->h < 1)
		return (-1);

	glv->flags |= AG_GLVIEW_RESHAPE;
	return (0);
}

void
AG_GLViewSetBgColor(AG_GLView *glv, const AG_Color *c)
{
	AG_OBJECT_ISA(glv, "AG_Widget:AG_GLView:*");
	AG_ObjectLock(glv);

	memcpy(&glv->bgColor, c, sizeof(AG_Color));

	AG_ObjectUnlock(glv);
}

static void
Draw(void *_Nonnull obj)
{
	AG_GLView *glv = obj;
	AG_Driver *drv = WIDGET(glv)->drv;
	Uint hView;
	
	if (glv->flags & AG_GLVIEW_BGFILL) {
		AG_DrawRect(glv, &WIDGET(glv)->r, &glv->bgColor);
	}
	if (glv->underlay_ev != NULL)
		glv->underlay_ev->fn(glv->underlay_ev);

	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);

	if (glv->flags & AG_GLVIEW_INIT_MATRICES) {
		glv->flags &= ~(AG_GLVIEW_INIT_MATRICES);
		SetIdentity(glv->mProjection, GL_PROJECTION);
		SetIdentity(glv->mModelview, GL_MODELVIEW);
		SetIdentity(glv->mTexture, GL_TEXTURE);
	}
	if ((glv->flags & AG_GLVIEW_RESHAPE) ||
	    (WIDGET(glv)->flags & AG_WIDGET_GL_RESHAPE)) {
		glv->flags &= ~(AG_GLVIEW_RESHAPE);
		WIDGET(glv)->flags &= ~(AG_WIDGET_GL_RESHAPE);
		Reshape(glv);
	}

	if (AGDRIVER_SINGLE(drv)) {
		hView = AGDRIVER_SW(drv)->h;
	} else {
		hView = HEIGHT(WIDGET(glv)->window);
	}
	glViewport(WIDGET(glv)->rView.x1,
	           hView - WIDGET(glv)->rView.y2,
	           WIDTH(glv), HEIGHT(glv));
	
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadMatrixf(glv->mTexture);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glv->mProjection);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glv->mModelview);

	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
	
	if (glv->draw_ev != NULL)
		glv->draw_ev->fn(glv->draw_ev);
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	/* restore transform and viewport */
	glPopAttrib();

	if (glv->overlay_ev != NULL) {
		glPushAttrib(GL_TRANSFORM_BIT);
		glv->overlay_ev->fn(glv->overlay_ev);
		glPopAttrib();
	}
}

/*
 * Compute the projection matrix for the context and save it for later.
 * Called automatically when the widget is scaled or moved.
 */
static void
Reshape(AG_GLView *glv)
{
	glMatrixMode(GL_TEXTURE);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);	glPushMatrix(); glLoadIdentity();

	if (glv->scale_ev != NULL) {
		glv->scale_ev->fn(glv->scale_ev);
	}
	glGetFloatv(GL_PROJECTION_MATRIX, glv->mProjection);
	glGetFloatv(GL_MODELVIEW_MATRIX, glv->mModelview);
	glGetFloatv(GL_TEXTURE_MATRIX, glv->mTexture);
	
	glMatrixMode(GL_PROJECTION);	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();
	glMatrixMode(GL_TEXTURE);	glPopMatrix();
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_GLView *glv = obj;

	r->w = glv->wPre;
	r->h = glv->hPre;
}

AG_WidgetClass agGLViewClass = {
	{
		"Agar(Widget:GLView)",
		sizeof(AG_GLView),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* HAVE_OPENGL */
#endif /* AG_WIDGETS */
