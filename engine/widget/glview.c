/*	$Csoft$	*/

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

#include <engine/engine.h>

#ifdef HAVE_OPENGL

#include <engine/config.h>
#include <engine/view.h>

#include <stdarg.h>

#include "glview.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

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

void
AG_GLViewInit(AG_GLView *glv, u_int flags)
{
	AG_WidgetInit(glv, "glview", &agGLViewOps, AG_WIDGET_FOCUSABLE);

	if (!AG_Bool(agConfig, "view.opengl"))
		fatal("widget requires OpenGL mode");

	if (flags & AG_GLVIEW_WFILL) AGWIDGET(glv)->flags |= AG_WIDGET_WFILL;
	if (flags & AG_GLVIEW_HFILL) AGWIDGET(glv)->flags |= AG_WIDGET_HFILL;

	glv->draw_ev = NULL;
	glv->reshape_ev = NULL;
#if 0
	AG_SetEvent(bu, "window-mousebuttonup", button_mousebuttonup, NULL);
	AG_SetEvent(bu, "window-mousebuttondown", button_mousebuttondown, NULL);
	AG_SetEvent(bu, "window-mousemotion", button_mousemotion, NULL);
	AG_SetEvent(bu, "window-keyup", button_keyup, NULL);
	AG_SetEvent(bu, "window-keydown", button_keydown, NULL);
#endif
}

void
AG_GLViewDrawFn(AG_GLView *glv, void (*fn)(int, union evarg *),
    const char *fmtp, ...)
{
	const char *fmt = fmtp;
	va_list ap;

	glv->draw_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmtp);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, glv->draw_ev);
		}
		va_end(ap);
	}
}

void
AG_GLViewReshapeFn(AG_GLView *glv, void (*fn)(int, union evarg *),
    const char *fmtp, ...)
{
	const char *fmt = fmtp;
	va_list ap;

	glv->reshape_ev = AG_SetEvent(glv, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmtp);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, glv->reshape_ev);
		}
		va_end(ap);
	}
}

void
AG_GLViewScale(void *p, int w, int h)
{
	AG_GLView *glv = p;

	if (w == -1 && h == -1) {
		AGWIDGET(glv)->w = 32;	/* XXX */
		AGWIDGET(glv)->h = 32;
		return;
	}
}

void
AG_GLViewDraw(void *p)
{
	AG_GLView *glv = p;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, view->w, view->h, 0, -1.0, 1.0);
	glTranslatef(
	    AGWIDGET(glv)->cx + AGWIDGET(glv)->w/2,
	    AGWIDGET(glv)->cy + AGWIDGET(glv)->h/2, 0);
	glScalef(AGWIDGET(glv)->w, AGWIDGET(glv)->h, 0);

	if (glv->draw_ev != NULL) {
		glv->draw_ev->handler(
		    glv->draw_ev->argc,
		    glv->draw_ev->argv);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, view->w, view->h, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
}

#endif /* HAVE_OPENGL */
