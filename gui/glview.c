/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/config.h>

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
mousebuttondown(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();

	AG_WidgetFocus(glv);
}

static void
Init(void *obj)
{
	AG_GLView *glv = obj;

	WIDGET(glv)->flags |= AG_WIDGET_FOCUSABLE;

	if (!AG_GetBool(agConfig,"view.opengl"))
		AG_FatalError("AG_GLView requires OpenGL");

	glv->wPre = 64;
	glv->hPre = 64;

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
	AG_SetEvent(glv, "window-mousebuttondown", mousebuttondown, NULL);
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
	glv->keydown_ev = AG_SetEvent(glv, "window-keydown", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keydown_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewKeyupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->keyup_ev = AG_SetEvent(glv, "window-keyup", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keyup_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewButtondownFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->btndown_ev = AG_SetEvent(glv, "window-mousebuttondown", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btndown_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewButtonupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->btnup_ev = AG_SetEvent(glv, "window-mousebuttonup", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btnup_ev, fmt);
	AG_ObjectUnlock(glv);
}

void
AG_GLViewMotionFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(glv);
	glv->motion_ev = AG_SetEvent(glv, "window-mousemotion", fn, NULL);
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
AG_GLViewSizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 32;
	r->h = 32;
}

int
AG_GLViewSizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_GLView *glv = p;

	if (a->w < 1 || a->h < 1)
		return (-1);

	glv->flags |= AG_GLVIEW_RESHAPE;
	return (0);
}

void
AG_GLViewDraw(void *p)
{
	AG_GLView *glv = p;

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

	glViewport(WIDGET(glv)->cx, agView->h - WIDGET(glv)->cy2,
	           WIDGET(glv)->w, WIDGET(glv)->h);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadMatrixf(glv->mTexture);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glv->mProjection);
		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glv->mModelview);
	
	if (glv->draw_ev != NULL)
		glv->draw_ev->handler(glv->draw_ev);
		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glViewport(0, 0, agView->w, agView->h);
	
	if (glv->overlay_ev != NULL)
		glv->overlay_ev->handler(glv->overlay_ev);
}

AG_WidgetClass agGLViewClass = {
	{
		"AG_Widget:AG_GLView",
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
