/*
 * Copyright (c) 2005-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include "glview.h"
#include "opengl.h"

#include <stdarg.h>

AG_GLView *
AG_GLViewNew(void *parent, Uint flags)
{
	AG_GLView *glv;

	glv = Malloc(sizeof(AG_GLView));
	AG_ObjectInit(glv, &agGLViewClass);
	glv->flags |= flags;

	if (flags & AG_GLVIEW_HFILL) { AG_ExpandHoriz(glv); }
	if (flags & AG_GLVIEW_VFILL) { AG_ExpandVert(glv); }

	AG_ObjectAttach(parent, glv);
	return (glv);
}

/* Initialize an OpenGL matrix to identity. GL must be locked. */
static void
SetIdentity(GLfloat *M, GLenum which)
{
	glMatrixMode(which);
	glPushMatrix();
	glLoadIdentity();
	glGetFloatv(which, M);
	glPopMatrix();
}

static void
WidgetMoved(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();

	glv->flags |= AG_GLVIEW_RESHAPE;
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();

	AG_WidgetFocus(glv);
}

static void
OnAttach(AG_Event *event)
{
	AG_Widget *parent = AG_SENDER();

	if (!(AGDRIVER_CLASS(parent->drv)->flags & AG_DRIVER_OPENGL))
		AG_FatalError("AG_GLView requires a driver with GL support");
}

static void
Init(void *obj)
{
	AG_GLView *glv = obj;

	WIDGET(glv)->flags |= AG_WIDGET_FOCUSABLE;

	glv->wPre = 100;
	glv->hPre = 100;

	glv->flags = AG_GLVIEW_INIT_MATRICES;
	glv->draw_ev = NULL;
	glv->overlay_ev = NULL;
	glv->scale_ev = NULL;
	glv->keydown_ev = NULL;
	glv->btndown_ev = NULL;
	glv->keyup_ev = NULL;
	glv->btnup_ev = NULL;
	glv->motion_ev = NULL;

	AG_SetEvent(glv, "widget-moved", WidgetMoved, NULL);
	AG_SetEvent(glv, "mouse-button-down", MouseButtonDown, NULL);
	AG_AddEvent(glv, "attached", OnAttach, NULL);

	/* XXX TODO should make this configurable through AG_GLView. */
	AG_RedrawOnTick(glv, 1);

#ifdef AG_DEBUG
	AG_BindUint(glv, "flags", &glv->flags);
	AG_BindInt(glv, "wPre", &glv->wPre);
	AG_BindInt(glv, "hPre", &glv->hPre);
#endif
}

void
AG_GLViewSizeHint(AG_GLView *glv, int w, int h)
{
	AG_ObjectLock(glv);
	glv->wPre = w;
	glv->hPre = h;
	AG_ObjectUnlock(glv);
}

void
AG_GLViewDrawFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->draw_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->draw_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewOverlayFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->overlay_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->overlay_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewScaleFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->scale_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->scale_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewKeydownFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->keydown_ev = AG_SetEvent(glv, "key-down", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keydown_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewKeyupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->keyup_ev = AG_SetEvent(glv, "key-up", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keyup_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewButtondownFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->btndown_ev = AG_SetEvent(glv, "mouse-button-down", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btndown_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewButtonupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->btnup_ev = AG_SetEvent(glv, "mouse-button-up", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btnup_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewMotionFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->motion_ev = AG_SetEvent(glv, "mouse-motion", fn, NULL);
	AG_EVENT_GET_ARGS(glv->motion_ev, fmt);
	AG_ObjectUnlock(glv);
}

/*
 * Compute the projection matrix for the context and save it for later.
 * Called automatically when the widget is scaled or moved.
 */
void
AG_GLViewReshape(AG_GLView *glv)
{
	glMatrixMode(GL_TEXTURE);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	glPushMatrix();	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	if (glv->scale_ev != NULL) {
		glv->scale_ev->handler(glv->scale_ev);
	}
	glGetFloatv(GL_PROJECTION_MATRIX, glv->mProjection);
	glGetFloatv(GL_MODELVIEW_MATRIX, glv->mModelview);
	glGetFloatv(GL_TEXTURE_MATRIX, glv->mTexture);
	
	glMatrixMode(GL_PROJECTION);	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();
	glMatrixMode(GL_TEXTURE);	glPopMatrix();
}

void
AG_GLViewSizeRequest(void *obj, AG_SizeReq *r)
{
	AG_GLView *glv = obj;

	r->w = glv->wPre;
	r->h = glv->hPre;
}

int
AG_GLViewSizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_GLView *glv = obj;

	if (a->w < 1 || a->h < 1)
		return (-1);

	glv->flags |= AG_GLVIEW_RESHAPE;
	return (0);
}

void
AG_GLViewDraw(void *obj)
{
	AG_GLView *glv = obj;
	AG_Driver *drv = WIDGET(glv)->drv;
	GLint vpSave[4];
	Uint hView;

	glGetIntegerv(GL_VIEWPORT, vpSave);

	if (glv->flags & AG_GLVIEW_INIT_MATRICES) {
		glv->flags &= ~(AG_GLVIEW_INIT_MATRICES);
		SetIdentity(glv->mProjection, GL_PROJECTION);
		SetIdentity(glv->mModelview, GL_MODELVIEW);
		SetIdentity(glv->mTexture, GL_TEXTURE);
	}
	if (glv->flags & AG_GLVIEW_RESHAPE) {
		glv->flags &= ~(AG_GLVIEW_RESHAPE);
		AG_GLViewReshape(glv);
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

	glPushAttrib(GL_TRANSFORM_BIT);
	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);

	if (glv->draw_ev != NULL)
		glv->draw_ev->handler(glv->draw_ev);
	
	glPopAttrib();
		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glViewport(vpSave[0], vpSave[1], vpSave[2], vpSave[3]);
	
	if (glv->overlay_ev != NULL)
		glv->overlay_ev->handler(glv->overlay_ev);
}

AG_WidgetClass agGLViewClass = {
	{
		"Agar(Widget:GLView)",
		sizeof(AG_GLView),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GLViewDraw,
	AG_GLViewSizeRequest,
	AG_GLViewSizeAllocate
};

#endif /* HAVE_OPENGL */
