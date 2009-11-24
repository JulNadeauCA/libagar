/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Driver for OpenGL graphics via the SDL library.
 */

#include <config/have_sdl.h>
#include <config/have_opengl.h>

#if defined(HAVE_SDL) && defined(HAVE_OPENGL)

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "packedpixel.h"
#include "cursors.h"
#include "perfmon.h"

#include "drv_gl_common.h"
#include "drv_sdl_common.h"

typedef struct ag_sdlgl_driver {
	struct ag_driver_sw _inherit;
	SDL_Surface     *s;		/* View surface */
	Uint             rNom;		/* Nominal refresh rate (ms) */
	int              rCur;		/* Effective refresh rate (ms) */
	int              clipStates[4];	/* Clipping GL state */
	AG_ClipRect     *clipRects;	/* Clipping rectangle stack */
	Uint            nClipRects;
	Uint            *textureGC;	/* Textures queued for deletion */
	Uint            nTextureGC;
	AG_GL_BlendState bs[1];		/* Saved blending states */
	AG_Cursor       *cursorToSet;	/* Set cursor at end of event cycle */
} AG_DriverSDLGL;

static int nDrivers = 0;		/* Opened driver instances */
static int initedSDL = 0;		/* Used SDL_Init() */
static int initedSDLVideo = 0;		/* Used SDL_INIT_VIDEO */

static void
Init(void *obj)
{
	AG_DriverSDLGL *sgl = obj;

	sgl->s = NULL;
	sgl->rNom = 16;
	sgl->rCur = 0;
	sgl->clipRects = NULL;
	sgl->nClipRects = 0;
	memset(sgl->clipStates, 0, sizeof(sgl->clipStates));
	sgl->textureGC = NULL;
	sgl->nTextureGC = 0;
	sgl->cursorToSet = NULL;
}

static void
Destroy(void *obj)
{
	AG_DriverSDLGL *sgl = obj;

	Free(sgl->clipRects);
	Free(sgl->textureGC);
}

/*
 * Generic driver operations
 */

static int
Open(void *obj, const char *spec)
{
	extern const AG_TimeOps agTimeOps_SDL;
	AG_Driver *drv = obj;
	AG_DriverSDLGL *sgl = obj;
	
	if (nDrivers != 0) {
		AG_SetError("Multiple SDL displays are not supported");
		return (-1);
	}

	/* Initialize SDL's video subsystem. */
	if (!initedSDL) {
		if (SDL_Init(0) == -1) {
			AG_SetError("SDL_Init() failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDL = 1;
	}
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			AG_SetError("SDL_INIT_VIDEO failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDLVideo = 1;
	}

	/* We can use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);

	/* Initialize the main mouse and keyboard devices. */
	drv->mouse = AG_MouseNew(sgl, "SDL mouse");
	drv->kbd = AG_KeyboardNew(sgl, "SDL keyboard");

	/* Configure the window caption */
	SDL_WM_SetCaption(agProgName, agProgName);

	nDrivers = 1;
	return (0);
}

static void
Close(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverSDLGL *sgl = obj;

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sgl));

	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDetach(drv->mouse);
	AG_ObjectDestroy(drv->mouse);
	AG_ObjectDetach(drv->kbd);
	AG_ObjectDestroy(drv->kbd);
	drv->mouse = NULL;
	drv->kbd = NULL;

	nDrivers = 0;
}

static int
GetDisplaySize(Uint *w, Uint *h)
{
	const SDL_VideoInfo *vi;

	vi = SDL_GetVideoInfo();
#if SDL_VERSION_ATLEAST(1,2,10)
	*w = (Uint)vi->current_w;
	*h = (Uint)vi->current_h;
#else
	*w = 320;		/* Arbitrary */
	*h = 240;
#endif
	return (0);
}

static void
BeginEventProcessing(void *obj)
{
	/* Nothing to do */
}

/* Test if the given coordinates overlap a window resize control. */
static __inline__ int
MouseOverCtrl(AG_Window *win, int x, int y)
{
	if ((y - WIDGET(win)->y) > (HEIGHT(win) - win->wBorderBot)) {
		int xRel = x - WIDGET(win)->x;
	    	if (xRel < win->wResizeCtrl) {
			return (AG_WINOP_LRESIZE);
		} else if (xRel > (WIDTH(win) - win->wResizeCtrl)) {
			return (AG_WINOP_RRESIZE);
		} else if ((win->flags & AG_WINDOW_NOVRESIZE) == 0) {
			return (AG_WINOP_HRESIZE);
		}
	}
	return (AG_WINOP_NONE);
}

/* Change the cursor if overlapping a resize control. */
static void
SetResizeCursor(AG_Window *win, int x, int y)
{
	AG_Driver *drv = WIDGET(win)->drv;

	switch (MouseOverCtrl(win, x,y)) {
	case AG_WINOP_LRESIZE:
		AG_PushStockCursor(drv, AG_LLDIAG_CURSOR);
		break;
	case AG_WINOP_RRESIZE:
		AG_PushStockCursor(drv, AG_LRDIAG_CURSOR);
		break;
	case AG_WINOP_HRESIZE:
		AG_PushStockCursor(drv, AG_VRESIZE_CURSOR);
		break;
	default:
		break;
	}
}

/*
 * If there is a modal window, request its shutdown if a click is
 * detected outside of its area.
 */
static int
ModalClose(AG_Window *win, int x, int y)
{
	if (!AG_WidgetArea(win, x, y)) {
		AG_PostEvent(NULL, win, "window-modal-close", NULL);
		return (1);
	}
	return (0);
}

/* Process an input device event. */
static int
InputEvent(AG_DriverSDLGL *sgl, SDL_Event *ev)
{
	AG_Driver *drv = AGDRIVER(sgl);
	AG_DriverSw *dsw = AGDRIVER_SW(sgl);
	AG_Window *win;

	sgl->cursorToSet = NULL;
	
	if (dsw->Lmodal->n > 0) {
		win = dsw->Lmodal->v[dsw->Lmodal->n-1].data.p;
		switch (ev->type) {
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (ModalClose(win, ev->button.x, ev->button.y)) {
				return (1);
			}
			break;
		default:
			break;
		}
		goto scan;		/* Skip WM events */
	}

	/* Process WM events */
	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:			/* Focus on window */
		AG_WindowFocusAtPos(dsw, ev->button.x, ev->button.y);
		break;
	case SDL_MOUSEBUTTONUP:				/* Terminate WM op */
		dsw->winop = AG_WINOP_NONE;
		break;
	}

scan:
	/*
	 * Iterate over the visible windows and deliver the appropriate Agar
	 * events.
	 */
	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		AG_ObjectLock(win);

		/* XXX TODO move invisible windows to different tailq! */
		if (!win->visible || (dsw->Lmodal->n > 0 &&
		    win != dsw->Lmodal->v[dsw->Lmodal->n-1].data.p)) {
			AG_ObjectUnlock(win);
			continue;
		}
		switch (ev->type) {
		case SDL_MOUSEMOTION:
			/*
			 * Pass event to the internal WM if a window manager
			 * operation is in progress. 
			 */
			if (dsw->winop != AG_WINOP_NONE) {
				if (dsw->winSelected != win) {
					AG_ObjectUnlock(win);
					continue;
				}
				AG_WM_MouseMotion(dsw, win, ev->motion.xrel,
				    ev->motion.yrel);
			}

			/* Post mouse-motion events to interested widgets. */
			AG_ProcessMouseMotion(win,
			    ev->motion.x, ev->motion.y,
			    ev->motion.xrel, ev->motion.yrel,
			    ev->motion.state);

			/*
			 * Change cursor if overlapping a resize control (as
			 * long as no widget has requested a cursor change).
			 */
			if (sgl->cursorToSet == NULL &&
			    (win->wBorderBot > 0) &&
			    !(win->flags & AG_WINDOW_NORESIZE) &&
			    AG_WidgetArea(win, ev->motion.x, ev->motion.y)) {
				SetResizeCursor(win, ev->motion.x, ev->motion.y);
			}
			if (sgl->cursorToSet == NULL) {
				/*
				 * Prevent widgets in other windows from
				 * changing the cursor.
				 */
				sgl->cursorToSet = &drv->cursors[0];
			}
			break;
		case SDL_MOUSEBUTTONUP:
			/* Terminate active window operations. */
			/* XXX redundant? */
			if (dsw->winop != AG_WINOP_NONE) {
				dsw->winop = AG_WINOP_NONE;
				dsw->winSelected = NULL;
			}
			AG_ProcessMouseButtonUp(win,
			    ev->button.x, ev->button.y,
			    (AG_MouseButton)ev->button.button);
			if (agWindowToFocus != NULL ||
			    !TAILQ_EMPTY(&agWindowDetachQ)) {
				AG_ObjectUnlock(win);
				return (1);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!AG_WidgetArea(win, ev->button.x, ev->button.y)) {
				AG_ObjectUnlock(win);
				continue;
			}
			if (win->wBorderBot > 0 &&
			    !(win->flags & AG_WINDOW_NORESIZE)) {
				dsw->winop = MouseOverCtrl(win,
				    ev->button.x, ev->button.y);
				if (dsw->winop != AG_WINOP_NONE) {
					dsw->winSelected = win;
					AG_ObjectUnlock(win);
					return (1);
				}
			}
			AG_ProcessMouseButtonDown(win,
			    ev->button.x, ev->button.y,
			    (AG_MouseButton)ev->button.button);
			if (agWindowToFocus != NULL ||
			    !TAILQ_EMPTY(&agWindowDetachQ)) {
				AG_ObjectUnlock(win);
				return (1);
			}
			break;
		case SDL_KEYUP:
			if (dsw->winLastKeydown != NULL &&
			    dsw->winLastKeydown != win) {
				/*
				 * Key was initially pressed while another
				 * window was holding focus, ignore.
				 */
				dsw->winLastKeydown = NULL;
				break;
			}
			AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
			    (AG_KeySym)ev->key.keysym.sym,
			    (Uint32)ev->key.keysym.unicode);
			AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
			    (AG_KeySym)ev->key.keysym.sym,
			    (Uint32)ev->key.keysym.unicode);
			break;
		case SDL_KEYDOWN:
			AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
			    (AG_KeySym)ev->key.keysym.sym,
			    (Uint32)ev->key.keysym.unicode);
			AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
			    (AG_KeySym)ev->key.keysym.sym,
			    (Uint32)ev->key.keysym.unicode);
			break;
		}
		AG_ObjectUnlock(win);
	}
	if (sgl->cursorToSet != NULL &&
	    sgl->cursorToSet != drv->activeCursor) {
		SDL_SetCursor((SDL_Cursor *)sgl->cursorToSet->p);
		drv->activeCursor = sgl->cursorToSet;
		sgl->cursorToSet = NULL;
	}
	return (0);
}

static int
ProcessEvents(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	SDL_Event ev;
	int nProcessed = 0;

	while (SDL_PollEvent(&ev) != 0) {
		AG_LockVFS(sgl);
		switch (ev.type) {
		case SDL_MOUSEMOTION:
			AG_MouseMotionUpdate(drv->mouse,
			    ev.motion.x, ev.motion.y);
			nProcessed += InputEvent(sgl, &ev);
			break;
		case SDL_MOUSEBUTTONUP:
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
			    ev.button.button);
			nProcessed += InputEvent(sgl, &ev);
			break;
		case SDL_MOUSEBUTTONDOWN:
			AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
			    ev.button.button);
			if (InputEvent(sgl, &ev) == 0) {
				if (dsw->flags & AG_DRIVER_SW_BGPOPUP &&
				    (ev.button.button == AG_MOUSE_MIDDLE ||
				     ev.button.button == AG_MOUSE_RIGHT)) {
					AG_WM_BackgroundPopupMenu(dsw);
					nProcessed++;
					break;
				}
			} else {
				nProcessed++;
			}
			break;
		case SDL_KEYDOWN:
			AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
			    (AG_KeySym)ev.key.keysym.sym,
			    (Uint32)ev.key.keysym.unicode);
			if (AG_ExecGlobalKeys(
			    (AG_KeySym)ev.key.keysym.sym,
			    (AG_KeyMod)ev.key.keysym.mod) == 0) {
				nProcessed += InputEvent(sgl, &ev);
			} else {
				nProcessed++;
			}
			break;
		case SDL_KEYUP:
			AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
			    (AG_KeySym)ev.key.keysym.sym,
			    (Uint32)ev.key.keysym.unicode);
			nProcessed += InputEvent(sgl, &ev);
			break;
		case SDL_VIDEORESIZE:
			AG_ResizeDisplay(ev.resize.w, ev.resize.h);
			nProcessed++;
			break;
		case SDL_VIDEOEXPOSE:
			nProcessed++;
			break;
		case SDL_QUIT:
			/* FALLTHROUGH */
		case SDL_USEREVENT:
			goto quit;
		}
		if (!TAILQ_EMPTY(&agWindowDetachQ)) {
			AG_FreeDetachedWindows();
		}
		if (agWindowToFocus != NULL) {
			AG_WM_CommitWindowFocus(agWindowToFocus);
			agWindowToFocus = NULL;
		}
		AG_UnlockVFS(sgl);
	}
	return (nProcessed);
quit:
	AG_UnlockVFS(sgl);
	agTerminating = 1;
	return (-1);
}

static void
GenericEventLoop(void *obj)
{
	AG_DriverSDLGL *sgl = obj;
	AG_Window *win;
	Uint32 Tr1, Tr2 = 0;

#ifdef AG_DEBUG
	AG_PerfMonInit();
#endif
	Tr1 = AG_GetTicks();
	for (;;) {
		Tr2 = AG_GetTicks();
		if (Tr2-Tr1 >= sgl->rNom) {
			AG_LockVFS(sgl);
			AG_BeginRendering(AGDRIVER(sgl));
			AG_FOREACH_WINDOW(win, sgl) {
				AG_ObjectLock(win);
				AG_WindowDraw(win);
				AG_ObjectUnlock(win);
			}
			AG_EndRendering(AGDRIVER(sgl));
			AG_UnlockVFS(sgl);

			/* Recalibrate the effective refresh rate. */
			Tr1 = AG_GetTicks();
			sgl->rCur = sgl->rNom - (Tr1-Tr2);
#ifdef AG_DEBUG
			if (agPerfWindow->visible)
				AG_PerfMonUpdate(sgl->rCur);
#endif
			if (sgl->rCur < 1) {
				sgl->rCur = 1;
			}
		} else if (SDL_PollEvent(NULL) != 0) {
			if (ProcessEvents(sgl) != 0) {
				if (agTerminating)
					return;
#ifdef AG_DEBUG
				agEventAvg++;
#endif
			} 
		} else if (AG_TIMEOUTS_QUEUED()) {		/* Safe */
			AG_ProcessTimeouts(Tr2);
		} else if (sgl->rCur > agIdleThresh) {
			AG_Delay(sgl->rCur - agIdleThresh);
#ifdef AG_DEBUG
			agIdleAvg = AG_GetTicks() - Tr2;
		} else {
			agIdleAvg = 0;
		}
#else
		}
#endif
	}
}

static void
EndEventProcessing(void *obj)
{
	/* Nothing to do */
}

static void
BeginRendering(void *obj)
{
	AG_DriverSDLGL *sgl = obj;
	
	glPushAttrib(GL_VIEWPORT_BIT|GL_TRANSFORM_BIT|GL_LIGHTING_BIT|
	             GL_ENABLE_BIT);
	
	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		AG_GL_InitContext(
		    AG_RECT(0, 0, AGDRIVER_SW(sgl)->w, AGDRIVER_SW(sgl)->h));
	} else {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	sgl->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE0);
	sgl->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE1);
	sgl->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE2);
	sgl->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3);
	glEnable(GL_CLIP_PLANE3);
}

static void
RenderWindow(struct ag_window *win)
{
	AG_WidgetDraw(win);
}

static void
EndRendering(void *drv)
{
	AG_DriverSDLGL *sgl = drv;
	Uint i;
	
	if (!(AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY)) {
		SDL_GL_SwapBuffers();
		if (sgl->clipStates[0])	{ glEnable(GL_CLIP_PLANE0); }
		else			{ glDisable(GL_CLIP_PLANE0); }
		if (sgl->clipStates[1])	{ glEnable(GL_CLIP_PLANE1); }
		else			{ glDisable(GL_CLIP_PLANE1); }
		if (sgl->clipStates[2])	{ glEnable(GL_CLIP_PLANE2); }
		else			{ glDisable(GL_CLIP_PLANE2); }
		if (sgl->clipStates[3])	{ glEnable(GL_CLIP_PLANE3); }
		else			{ glDisable(GL_CLIP_PLANE3); }
	}

	glPopAttrib();
	
	/* Remove textures queued for deletion. */
	for (i = 0; i < sgl->nTextureGC; i++) {
		glDeleteTextures(1, (GLuint *)&sgl->textureGC[i]);
	}
	sgl->nTextureGC = 0;
}

static void
DeleteTexture(void *drv, Uint texture)
{
	AG_DriverSDLGL *sgl = drv;

	sgl->textureGC = Realloc(sgl->textureGC,
	    (sgl->nTextureGC+1)*sizeof(Uint));
	sgl->textureGC[sgl->nTextureGC++] = texture;
}

static int
SetRefreshRate(void *obj, int fps)
{
	AG_DriverSDLGL *sgl = obj;

	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	sgl->rNom = 1000/fps;
	sgl->rCur = 0;
	return (0);
}

/*
 * Clipping and blending control (rendering context)
 */

static void
PushClipRect(void *obj, AG_Rect r)
{
	AG_DriverSDLGL *sgl = obj;
	AG_ClipRect *cr, *crPrev;

	sgl->clipRects = Realloc(sgl->clipRects, (sgl->nClipRects+1)*
	                                         sizeof(AG_ClipRect));
	crPrev = &sgl->clipRects[sgl->nClipRects-1];
	cr = &sgl->clipRects[sgl->nClipRects++];

	cr->eqns[0][0] = 1.0;
	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;
	cr->eqns[0][3] = MIN(crPrev->eqns[0][3], -(double)(r.x));
	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	
	cr->eqns[1][0] = 0.0;
	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;
	cr->eqns[1][3] = MIN(crPrev->eqns[1][3], -(double)(r.y));
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
		
	cr->eqns[2][0] = -1.0;
	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;
	cr->eqns[2][3] = MIN(crPrev->eqns[2][3], (double)(r.x+r.w));
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
		
	cr->eqns[3][0] = 0.0;
	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;
	cr->eqns[3][3] = MIN(crPrev->eqns[3][3], (double)(r.y+r.h));
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}

static void
PopClipRect(void *obj)
{
	AG_DriverSDLGL *sgl = obj;
	AG_ClipRect *cr;
	
#ifdef AG_DEBUG
	if (sgl->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &sgl->clipRects[sgl->nClipRects-2];
	sgl->nClipRects--;

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}

static void
PushBlendingMode(void *drv, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_DriverSDLGL *sgl = drv;

	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
	    &sgl->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &sgl->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &sgl->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &sgl->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(AG_GL_GetBlendingFunc(fnSrc), AG_GL_GetBlendingFunc(fnDst));
}
static void
PopBlendingMode(void *drv)
{
	AG_DriverSDLGL *sgl = drv;

	/* XXX TODO: stack */
	if (sgl->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(sgl->bs[0].srcFactor, sgl->bs[0].dstFactor);
}

/*
 * Cursor operations
 */

static int
SDLGL_CreateCursor(void *obj, AG_Cursor *ac)
{
	SDL_Cursor *sc;

	sc = SDL_CreateCursor(ac->data, ac->mask,
	    ac->w, ac->h,
	    ac->xHot, ac->yHot);
	if (sc == NULL) {
		AG_SetError("SDL_CreateCursor failed");
		return (-1);
	}
	ac->p = (void *)sc;
	return (0);
}

static void
FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;

	if (ac == &drv->cursors[0])
		return;

	SDL_FreeCursor((SDL_Cursor *)(ac->p));
	ac->p = NULL;
}

static int
PushCursor(void *obj, AG_Cursor *ac)
{
	AG_DriverSDLGL *sgl = obj;

	/* XXX TODO stack */
	sgl->cursorToSet = ac;
	return (0);
}

static void
PopCursor(void *obj)
{
	/* Nothing to do */
}

static int
GetCursorVisibility(void *obj)
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

static void
SetCursorVisibility(void *obj, int flag)
{
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
}

/*
 * Rendering operations (rendering context)
 */

/* Initialize the clipping rectangle stack. */
static int
InitClipRects(AG_DriverSDLGL *sgl, int wView, int hView)
{
	AG_ClipRect *cr;
	int i;

	for (i = 0; i < 4; i++)
		sgl->clipStates[i] = 0;

	/* Rectangle 0 always covers the whole view. */
	if ((sgl->clipRects = AG_TryMalloc(sizeof(AG_ClipRect))) == NULL)
		return (-1);

	cr = &sgl->clipRects[0];
	cr->r = AG_RECT(0, 0, wView, hView);

	cr->eqns[0][0] = 1.0;	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;	cr->eqns[0][3] = 0.0;
	cr->eqns[1][0] = 0.0;	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;	cr->eqns[1][3] = 0.0;
	cr->eqns[2][0] = -1.0;	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;	cr->eqns[2][3] = (double)wView;
	cr->eqns[3][0] = 0.0;	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;	cr->eqns[3][3] = (double)hView;
	
	sgl->nClipRects = 1;
	return (0);
}

/* Initialize the default cursor. */
static int
InitDefaultCursor(AG_DriverSDLGL *sgl)
{
	AG_Driver *drv = AGDRIVER(sgl);
	AG_Cursor *ac;
	SDL_Cursor *sc;
	
	if ((sc = SDL_GetCursor()) == NULL) {
		AG_SetError("SDL_GetCursor() returned NULL");
		return (-1);
	}
	if ((drv->cursors = AG_TryMalloc(sizeof(AG_Cursor))) == NULL) {
		return (-1);
	}
	ac = &drv->cursors[0];
	drv->nCursors = 1;
	AG_CursorInit(ac);
	ac->w = (Uint)sc->area.w;
	ac->h = (Uint)sc->area.h;
	ac->xHot = (int)sc->hot_x;
	ac->yHot = (int)sc->hot_y;
	ac->p = sc;
	return (0);
}

/*
 * Single-display specific operations.
 */

static AG_PixelFormat *
GetVideoPixelFormat(SDL_Surface *su)
{
	switch (su->format->BytesPerPixel) {
	case 1:
		return AG_PixelFormatIndexed(su->format->BitsPerPixel);
	case 2:
	case 3:
	case 4:
		return AG_PixelFormatRGB(su->format->BitsPerPixel,
		    su->format->Rmask,
		    su->format->Gmask,
		    su->format->Bmask);
	default:
		AG_SetError("Unsupported pixel depth (%d bpp)",
		    (int)su->format->BitsPerPixel);
		return (NULL);
	}
}

static void
ClearBackground(void)
{
	AG_Color c = agColors[BG_COLOR];

	glClearColor(c.r, c.g, c.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static int
OpenVideo(void *obj, Uint w, Uint h, int depth, Uint flags)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	Uint32 sFlags = SDL_OPENGL;
	int newDepth;

	/* Set the requested display options. */
	if (flags & AG_VIDEO_RESIZABLE) { sFlags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_ANYFORMAT) { sFlags |= SDL_ANYFORMAT; }
	if (flags & AG_VIDEO_HWPALETTE) { sFlags |= SDL_HWPALETTE; }
	if (flags & AG_VIDEO_DOUBLEBUF) { sFlags |= SDL_DOUBLEBUF; }
	if (flags & AG_VIDEO_FULLSCREEN) { sFlags |= SDL_FULLSCREEN; }
	if (flags & AG_VIDEO_NOFRAME) { sFlags |= SDL_NOFRAME; }

	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;

	/* Set the video mode. Force hardware palette in 8bpp. */
	newDepth = SDL_VideoModeOK(w, h, depth, sFlags);
	if (newDepth == 8) {
		Verbose(_("Enabling hardware palette"));
		sFlags |= SDL_HWPALETTE;
	}
	if ((sgl->s = SDL_SetVideoMode((int)w, (int)h, newDepth, sFlags))
	    == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s", w, h, newDepth,
		    SDL_GetError());
		return (-1);
	}
	SDL_EnableUNICODE(1);

	if ((drv->videoFmt = GetVideoPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: New display (%dbpp)\n"),
	     (int)drv->videoFmt->BitsPerPixel);
	
	/* Initialize clipping rectangles. */
	if (InitClipRects(sgl, dsw->w, dsw->h) == -1)
		goto fail;
	
	/* Create the cursors. */
	if (InitDefaultCursor(sgl) == -1 ||
	    AG_InitStockCursors(drv) == -1)
		goto fail;

	/* Initialize the GL viewport. */
	AG_GL_InitContext(
	    AG_RECT(0, 0, AGDRIVER_SW(sgl)->w, AGDRIVER_SW(sgl)->h));

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY)) {
		ClearBackground();
	}
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static int
OpenVideoContext(void *obj, void *ctx, Uint flags)
{
	AG_DriverSDLGL *sgl = obj;
	AG_DriverSw *dsw = obj;
	AG_Driver *drv = obj;
	SDL_Surface *ctxSu = (SDL_Surface *)ctx;

	if (!(ctxSu->flags & SDL_OPENGL)) {
		AG_SetError("Given display surface is not SDL_OPENGL");
		return (-1);
	}

	/* Set the requested display options. */
	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;

	/* Use the given display surface. */
	sgl->s = (SDL_Surface *)ctx;
	if ((drv->videoFmt = GetVideoPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: Using existing display (%dbpp)\n"),
	     (int)drv->videoFmt->BitsPerPixel);

	/* Initialize clipping rectangles. */
	if (InitClipRects(sgl, dsw->w, dsw->h) == -1)
		goto fail;
	
	/* Create the cursors. */
	if (InitDefaultCursor(sgl) == -1 ||
	    AG_InitStockCursors(drv) == -1)
		goto fail;
	
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
CloseVideo(void *obj)
{
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
VideoResize(void *obj, Uint w, Uint h)
{
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	Uint32 sFlags;
	SDL_Surface *su;
	AG_ClipRect *cr0;

	sFlags = sgl->s->flags & (SDL_SWSURFACE|SDL_FULLSCREEN|SDL_HWSURFACE|
	                          SDL_ASYNCBLIT|SDL_HWPALETTE|SDL_RESIZABLE|
	                          SDL_OPENGL);

	if ((su = SDL_SetVideoMode(w, h, 0, sFlags)) == NULL) {
		AG_SetError("Cannot resize display to %ux%u: %s", w, h,
		    SDL_GetError());
		return (-1);
	}
	sgl->s = su;

	dsw->w = su->w;
	dsw->h = su->h;
	dsw->depth = (Uint)su->format->BitsPerPixel;

	/* Update clipping rectangle 0. */
	cr0 = &sgl->clipRects[0];
	cr0->r.w = w;
	cr0->r.h = h;
	
	/* Reinitialize the GL viewport. */
	AG_GL_InitContext(
	    AG_RECT(0, 0, AGDRIVER_SW(sgl)->w, AGDRIVER_SW(sgl)->h));

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY))
		ClearBackground();

	return (0);
}

static int
VideoCapture(void *obj, AG_Surface **sp)
{
#if 0
	AG_DriverSDLGL *sgl = obj;
	AG_Surface *s;

	if ((s = AG_DupSurface(sgl->s)) == NULL) {
		return (-1);
	}
	*sp = s;
	return (0);
#endif
	/* XXX TODO */
	AG_SetError("Operation not implemented");
	return (-1);
}

static void
VideoClear(void *obj, AG_Color c)
{
	glClearColor(c.r, c.g, c.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

AG_DriverSwClass agDriverSDLGL = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDLGL",
			sizeof(AG_DriverSDLGL),
			{ 1,4 },
			Init,
			NULL,	/* reinit */
			Destroy,
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
		},
		"sdlgl",
		AG_VECTOR,
		AG_WM_SINGLE,
		AG_DRIVER_SDL|AG_DRIVER_OPENGL|AG_DRIVER_TEXTURES,
		Open,
		Close,
		GetDisplaySize,
		BeginEventProcessing,
		ProcessEvents,
		GenericEventLoop,
		EndEventProcessing,
		BeginRendering,
		RenderWindow,
		EndRendering,
		AG_GL_FillRect,
		NULL,			/* updateRegion */
		AG_GL_UploadTexture,
		AG_GL_UpdateTexture,
		DeleteTexture,
		SetRefreshRate,
		PushClipRect,
		PopClipRect,
		PushBlendingMode,
		PopBlendingMode,
		SDLGL_CreateCursor, // Name conflicts with wingdi.h!
		FreeCursor,
		PushCursor,
		PopCursor,
		GetCursorVisibility,
		SetCursorVisibility,
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB,
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawArrowUp,
		AG_GL_DrawArrowDown,
		AG_GL_DrawArrowLeft,
		AG_GL_DrawArrowRight,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircle2,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_DrawFrame,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph
	},
	0,
	OpenVideo,
	OpenVideoContext,
	CloseVideo,
	VideoResize,
	VideoCapture,
	VideoClear
};

#endif /* HAVE_SDL and HAVE_OPENGL */
