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

#ifdef HAVE_OPENGL

#include <core/config.h>

#include "glview.h"

#include <stdarg.h>

AG_GLView *
AG_GLViewNew(void *parent, Uint flags)
{
	AG_GLView *glv;

	glv = Malloc(sizeof(AG_GLView));
	AG_GLViewInit(glv, flags);
	AG_ObjectAttach(parent, glv);
	if (flags & AG_GLVIEW_FOCUS) {
		AG_WidgetFocus(glv);
	}
	return (glv);
}

/* Initialize an OpenGL matrix to identity. GL must be locked. */
static void
SetIdentity(GLdouble *M, GLenum which)
{
	glMatrixMode(which);
	glPushMatrix();
	glLoadIdentity();
	glGetDoublev(which, M);
	glPopMatrix();
}

static void
WidgetMoved(AG_Event *event)
{
	AG_GLViewReshape(AG_SELF());
}

static void
mousebuttondown(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();

	AG_WidgetFocus(glv);
}

void
AG_GLViewInit(AG_GLView *glv, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_GLVIEW_HFILL) wflags |= AG_WIDGET_HFILL;
	if (flags & AG_GLVIEW_VFILL) wflags |= AG_WIDGET_VFILL;

	AG_WidgetInit(glv, &agGLViewOps, wflags);

	if (!AG_Bool(agConfig, "view.opengl"))
		fatal("widget requires OpenGL mode");

	glv->wPre = 64;
	glv->hPre = 64;

	glv->draw_ev = NULL;
	glv->overlay_ev = NULL;
	glv->scale_ev = NULL;
	glv->keydown_ev = NULL;
	glv->btndown_ev = NULL;
	glv->keyup_ev = NULL;
	glv->btnup_ev = NULL;
	glv->motion_ev = NULL;

	AG_LockGL();
	SetIdentity(glv->mProjection, GL_PROJECTION);
	SetIdentity(glv->mModelview, GL_MODELVIEW);
	SetIdentity(glv->mTexture, GL_TEXTURE);
	AG_UnlockGL();

	AG_SetEvent(glv, "widget-moved", WidgetMoved, NULL);
	AG_SetEvent(glv, "window-mousebuttondown", mousebuttondown, NULL);
}

void
AG_GLViewSizeHint(AG_GLView *glv, int w, int h)
{
	glv->wPre = w;
	glv->hPre = h;
}

void
AG_GLViewDrawFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->draw_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->draw_ev, fmt);
}

void
AG_GLViewOverlayFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->overlay_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->overlay_ev, fmt);
}

void
AG_GLViewScaleFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->scale_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->scale_ev, fmt);
}

void
AG_GLViewKeydownFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->keydown_ev = AG_SetEvent(glv, "window-keydown", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keydown_ev, fmt);
}

void
AG_GLViewKeyupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->keyup_ev = AG_SetEvent(glv, "window-keyup", fn, NULL);
	AG_EVENT_GET_ARGS(glv->keyup_ev, fmt);
}

void
AG_GLViewButtondownFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->btndown_ev = AG_SetEvent(glv, "window-mousebuttondown", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btndown_ev, fmt);
}

void
AG_GLViewButtonupFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->btnup_ev = AG_SetEvent(glv, "window-mousebuttonup", fn, NULL);
	AG_EVENT_GET_ARGS(glv->btnup_ev, fmt);
}

void
AG_GLViewMotionFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->motion_ev = AG_SetEvent(glv, "window-mousemotion", fn, NULL);
	AG_EVENT_GET_ARGS(glv->motion_ev, fmt);
}

/*
 * Compute the projection matrix for the context and save it for later.
 * Called automatically when the widget is scaled or moved.
 */
void
AG_GLViewReshape(AG_GLView *glv)
{
	AG_LockGL();

	glMatrixMode(GL_TEXTURE);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	glPushMatrix();	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	if (glv->scale_ev != NULL) {
		glv->scale_ev->handler(glv->scale_ev);
	}
	glGetDoublev(GL_PROJECTION_MATRIX, glv->mProjection);
	glGetDoublev(GL_MODELVIEW_MATRIX, glv->mModelview);
	glGetDoublev(GL_TEXTURE_MATRIX, glv->mTexture);
	
	glMatrixMode(GL_PROJECTION);	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();
	glMatrixMode(GL_TEXTURE);	glPopMatrix();

	AG_UnlockGL();
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

	AG_GLViewReshape(glv);
	return (0);
}

void
AG_GLViewDraw(void *p)
{
	AG_GLView *glv = p;
	
	glViewport(WIDGET(glv)->cx, agView->h - WIDGET(glv)->cy2,
	           WIDGET(glv)->w, WIDGET(glv)->h);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadMatrixd(glv->mTexture);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(glv->mProjection);
		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(glv->mModelview);
	
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

const AG_WidgetOps agGLViewOps = {
	{
		"AG_Widget:AG_GLView",
		sizeof(AG_GLView),
		{ 0,0 },
		NULL,		/* init */
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
