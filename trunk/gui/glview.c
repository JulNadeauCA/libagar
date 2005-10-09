/*	$Csoft: glview.c,v 1.2 2005/10/06 10:41:50 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <core/view.h>

#include <stdarg.h>

#include "glview.h"

#include <gui/window.h>
#include <gui/primitive.h>

const AG_WidgetOps agGLViewOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_GLViewDraw,
	AG_GLViewScale
};

AG_GLView *
AG_GLViewNew(void *parent, u_int flags)
{
	AG_GLView *glv;

	glv = Malloc(sizeof(AG_GLView), M_OBJECT);
	AG_GLViewInit(glv, flags);
	AG_ObjectAttach(parent, glv);
	return (glv);
}

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
GLViewMoved(AG_Event *event)
{
	AG_GLViewReshape(AG_SELF());
}

static void
GLViewButtondown(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	int button = AG_INT(1);

	dprintf("focus\n");
	AG_WidgetFocus(glv);
}

static void
GLViewKeydown(AG_Event *event)
{
	AG_GLView *glv = AG_SELF();
	int sym = AG_INT(1);
	int mod = AG_INT(2);
	
	if (glv->keydown_ev != NULL) {
		glv->keydown_ev->handler(glv->keydown_ev);
	} else {
		extern const char *agKeySyms[];
		extern const int agnKeySyms;

		if (sym < agnKeySyms)
			dprintf("%s: no binding\n", agKeySyms[sym]);
	}
}

void
AG_GLViewInit(AG_GLView *glv, u_int flags)
{
	AG_WidgetInit(glv, "glview", &agGLViewOps, AG_WIDGET_FOCUSABLE);

	if (!AG_Bool(agConfig, "view.opengl"))
		fatal("widget requires OpenGL mode");

	if (flags & AG_GLVIEW_WFILL) AGWIDGET(glv)->flags |= AG_WIDGET_WFILL;
	if (flags & AG_GLVIEW_HFILL) AGWIDGET(glv)->flags |= AG_WIDGET_HFILL;

	glv->draw_ev = NULL;
	glv->scale_ev = NULL;
	glv->keydown_ev = NULL;
	glv->btndown_ev = NULL;
	glv->keyup_ev = NULL;
	glv->btnup_ev = NULL;
	glv->motion_ev = NULL;

	SetIdentity(glv->mProjection, GL_PROJECTION);
	SetIdentity(glv->mModelview, GL_MODELVIEW);
	SetIdentity(glv->mTexture, GL_TEXTURE);
	SetIdentity(glv->mColor, GL_COLOR);

	AG_SetEvent(glv, "widget-moved", GLViewMoved, NULL);
}

void
AG_GLViewDrawFn(AG_GLView *glv, AG_EventFn fn, const char *fmt, ...)
{
	glv->draw_ev = AG_SetEvent(glv, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(glv->draw_ev, fmt);
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
	glMatrixMode(GL_TEXTURE);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_COLOR);		glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);	glPushMatrix();	glLoadIdentity();
	
	glLoadIdentity();
	if (glv->scale_ev != NULL) {
		glv->scale_ev->handler(glv->draw_ev);
	}
	glGetDoublev(GL_PROJECTION_MATRIX, glv->mProjection);
	glGetDoublev(GL_MODELVIEW_MATRIX, glv->mModelview);
	glGetDoublev(GL_TEXTURE_MATRIX, glv->mTexture);
	glGetDoublev(GL_COLOR_MATRIX, glv->mColor);
	
	glMatrixMode(GL_PROJECTION);	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();
	glMatrixMode(GL_TEXTURE);	glPopMatrix();
	glMatrixMode(GL_COLOR);		glPopMatrix();
}

void
AG_GLViewScale(void *p, int w, int h)
{
	AG_GLView *glv = p;

	if (w == -1 && h == -1) {
		AGWIDGET(glv)->w = 256;		/* XXX */
		AGWIDGET(glv)->h = 256;
		AG_GLViewReshape(glv);
		return;
	}
	AGWIDGET(glv)->w = w;
	AGWIDGET(glv)->h = h;
	AG_GLViewReshape(glv);
}

void
AG_GLViewDraw(void *p)
{
	AG_GLView *glv = p;
	
	glViewport(
	    AGWIDGET(glv)->cx, agView->h - AGWIDGET(glv)->cy2,
	    AGWIDGET(glv)->w, AGWIDGET(glv)->h);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadMatrixd(glv->mTexture);

	glMatrixMode(GL_COLOR);
	glPushMatrix();
	glLoadMatrixd(glv->mColor);
	
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
	
	glMatrixMode(GL_COLOR);
	glPopMatrix();
	
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glViewport(0, 0, agView->w, agView->h);
}

#endif /* HAVE_OPENGL */
