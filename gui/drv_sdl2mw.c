/*
 * Copyright (c) 2022-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Multi-window Driver for OpenGL graphics via SDL2.
 * One GL context is created for each Agar window.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>
#include <agar/gui/sdl2.h>

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/tlist.h>
#include <agar/gui/notebook.h>
#endif

/* #define DEBUG_DISPLAY */

static int nDrivers = 0;                        /* Drivers open */
static int initedSDL = 0;			/* Inited TIMERS and EVENTS */

AG_EventSink *_Nullable sdl2mwEventEpilogue = NULL; /* Event sink epilogue */
AG_EventSink *_Nullable sdl2mwEventSpinner = NULL;  /* For agTimeOps_renderer */

enum ag_sdl2mw_out {
	AG_SDL2MW_OUT_NONE,		/* No capture */
	AG_SDL2MW_OUT_JPEG,		/* Output JPEG files */
	AG_SDL2MW_OUT_PNG		/* Output PNG files */
};

typedef struct ag_sdl2mw_driver {
	struct ag_driver_mw _inherit;	/* AG_Driver -> AG_DriverMw */

	SDL_Window *_Nullable window;	/* SDL window */

	AG_GL_Context gl;		/* Agar OpenGL context data */
	SDL_GLContext glCtx;		/* SDL per-window OpenGL context */

	Uint8 *_Nullable   outBuf;	/* Output capture buffer */
	char *_Nullable    outPath;	/* Output capture path (%N = window name) */
	enum ag_sdl2mw_out outMode;	/* Output capture mode */
	Uint               outFrame;	/* Capture frame# counter */
	Uint               outLast;	/* Terminate after this many frames */
	Uint               outJpegQual;	/* Quality (%) for jpeg output */
	Uint               outJpegFlags;/* DCT options */

	Uint32 _pad;
} AG_DriverSDL2MW;

AG_DriverMwClass agDriverSDL2MW;
#if 0
#define AGDRIVER_IS_SDL2MW(drv) AG_OfClass((drv),"AG_Driver:AG_DriverMw:AG_DriverSDL2MW")
#else
#define AGDRIVER_IS_SDL2MW(drv) (AGDRIVER_CLASS(drv) == (AG_DriverClass *)&agDriverSDL2MW)
#endif

static int SDL2MW_ProcessEvent(void *, AG_DriverEvent *);

static void
Init(void *_Nonnull obj)
{
	AG_DriverMw *dmw = obj;
	AG_DriverSDL2MW *smw = obj;

	/*
	 * Advertise this driver's capability of creating windows at
	 * undefined positions.
	 */
	dmw->flags |= AG_DRIVER_MW_ANYPOS_AVAIL;

	smw->window = NULL;     /* Remains NULL for the root driver instance */

	smw->outBuf = NULL;
	smw->outPath = NULL;
	smw->outMode = AG_SDL2MW_OUT_NONE;
	smw->outFrame = 0;
	smw->outLast = 0;
	smw->outJpegQual = 100;
	smw->outJpegFlags = 0;
}

static int
SDL2MW_EventSink(AG_EventSink *es, AG_Event *event)
{
	AG_DriverEvent dev;
	AG_Driver *drv = AG_DRIVER_PTR(1);

	if (SDL_PollEvent(NULL) != 0) {
		while (AG_SDL2_GetNextEvent(drv, &dev) == 1) {
			if (dev.type != AG_DRIVER_UNKNOWN)
				(void)SDL2MW_ProcessEvent(drv, &dev);
		}
	} else {
		AG_Delay(1);
	}
	return (0);
}

static int
SDL2MW_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2MW *smw = obj;

	if (!initedSDL) {
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1) {
			AG_SetError("SDL_Init() failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDL = 1;
	}
#if 0
	/* Use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);
	AG_DestroyEventSubsystem();
	AG_InitEventSubsystem(AG_SOFT_TIMERS);
#endif
	if ((drv->mouse = AG_MouseNew(drv, "SDL2 Mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(drv, "SDL2 Keyboard")) == NULL)
		goto fail;

	/* Driver manages rendering of window background. */
	drv->flags |= AG_DRIVER_WINDOW_BG;

	if (nDrivers == 0) {			/* Root driver instance */
		if (AG_Defined(drv, "noAutoCapture") &&
		    AG_GetInt(drv, "noAutoCapture"))
			SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");

		/* Enable the joystick subsystem if requested. */
		if (AG_Defined(drv, "ctrl")) {
			Uint32 sdlFlags = SDL_INIT_GAMECONTROLLER;

			Debug(drv, "Enabling controller subsystem\n");

			if (AG_Defined(drv, "haptic")) { sdlFlags |= SDL_INIT_HAPTIC; }
			if (SDL_InitSubSystem(sdlFlags) < 0)
				AG_Verbose("SDL_INIT_GAMECONTROLLER: %s\n", SDL_GetError());
		} else if (AG_Defined(drv, "joy")) {
			Uint32 sdlFlags = SDL_INIT_JOYSTICK;
		
			Debug(drv, "Enabling joystick subsystem\n");

			if (AG_Defined(drv, "haptic")) { sdlFlags |= SDL_INIT_HAPTIC; }
			if (SDL_InitSubSystem(sdlFlags) < 0)
				AG_Verbose("SDL_INIT_JOYSTICK: %s\n", SDL_GetError());
		}

		/*
		 * Parse global attributes for capturing to image files
		 * ("out", "outFirst", "outLast", "jpegQual" and "jpegDCT").
		 */
		if (AG_Defined(drv, "out")) {
			char buf[256];
			char *ext;

			AG_GetString(drv, "out", buf, sizeof(buf));
			Debug(drv, "Enabled capture (%s)\n", buf);
			if ((ext = strrchr(buf, '.')) != NULL &&
			    ext[1] != '\0') {
				if (Strcasecmp(&ext[1], "jpeg") == 0 ||
				    Strcasecmp(&ext[1], "jpg") == 0) {
					smw->outMode = AG_SDL2MW_OUT_JPEG;
					if ((smw->outPath = TryStrdup(buf)) == NULL)
						return (-1);
				} else if (Strcasecmp(&ext[1], "png") == 0) {
					smw->outMode = AG_SDL2MW_OUT_PNG;
					if ((smw->outPath = TryStrdup(buf)) == NULL)
						return (-1);
				} else {
					AG_SetError("Invalid out= argument: `%s'", buf);
					return (-1);
				}
				if (AG_Defined(drv, "outFirst")) {
					AG_GetString(drv, "outFirst", buf, sizeof(buf));
					smw->outFrame = atoi(buf);
				} else {
					smw->outFrame = 0;
				}
				if (AG_Defined(drv, "outLast")) {
					AG_GetString(drv, "outLast", buf, sizeof(buf));
					smw->outLast = atoi(buf);
				}
			}
			if (AG_Defined(drv, "jpegQual")) {
				AG_GetString(drv, "jpegQual", buf, sizeof(buf));
				smw->outJpegQual = atoi(buf);
			}
			if (AG_Defined(drv, "jpegDCT")) {
				AG_GetString(drv, "jpegDCT", buf, sizeof(buf));
				if (Strcasecmp(buf, "islow")) {
					smw->outJpegFlags = AG_EXPORT_JPEG_JDCT_ISLOW;
				} else if (Strcasecmp(buf, "ifast")) {
					smw->outJpegFlags = AG_EXPORT_JPEG_JDCT_IFAST;
				} else if (Strcasecmp(buf, "float")) {
					smw->outJpegFlags = AG_EXPORT_JPEG_JDCT_FLOAT;
				}
			}
		}

		/*
		 * Register event sink and epilogue routines for use by the
		 * standard AG_EventLoop() routine. Custom event loops may
		 * use alternate routines.
		 */
		if ((sdl2mwEventSpinner = AG_AddEventSpinner(
		    SDL2MW_EventSink,"%p",drv)) == NULL) {
			goto fail;
		}
		if ((sdl2mwEventEpilogue = AG_AddEventEpilogue(
		    AG_SDL2_EventEpilogue, NULL)) == NULL) {
			AG_DelEventSink(sdl2mwEventSpinner);
			sdl2mwEventSpinner = NULL;
			goto fail;
		}
	}

	nDrivers++;
	return (0);
fail:
	if (drv->kbd != NULL)   { AG_ObjectDelete(drv->kbd);   drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
SDL2MW_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

#ifdef AG_DEBUG
	if (nDrivers == 0) { AG_FatalError("Driver close without open"); }
#endif
	if (drv->kbd) {
		AG_ObjectDetach(drv->kbd);
		AG_ObjectDestroy(drv->kbd);
		drv->kbd = NULL;
	}
	if (drv->mouse) {
		AG_ObjectDetach(drv->mouse);
		AG_ObjectDestroy(drv->mouse);
		drv->mouse = NULL;
	}

	if (--nDrivers == 0) {
		if (sdl2mwEventSpinner != NULL) {
			AG_DelEventSink(sdl2mwEventSpinner);
			sdl2mwEventSpinner = NULL;
		}
		if (AG_Defined(drv, "ctrl")) {
			Uint32 sdlFlags = SDL_INIT_GAMECONTROLLER;

			if (AG_Defined(drv, "haptic")) { sdlFlags |= SDL_INIT_HAPTIC; }
			SDL_QuitSubSystem(sdlFlags);
		} else if (AG_Defined(drv, "joy")) {
			Uint32 sdlFlags = SDL_INIT_JOYSTICK;

			if (AG_Defined(drv, "haptic")) { sdlFlags |= SDL_INIT_HAPTIC; }
			SDL_QuitSubSystem(sdlFlags);
		}
	}
}

static void
SDL2MW_PostMoveCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)drv;
	AG_SizeAlloc wa;
	const int useText = (win->flags & AG_WINDOW_USE_TEXT);

	SDL_GL_MakeCurrent(smw->window, smw->glCtx);

	if (useText) {
		AG_PushTextState();
		AG_TextFont(WIDGET(win)->font);
		AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
		                                [AG_TEXT_COLOR]);
	}

	wa.x = 0;
	wa.y = 0;
	wa.w = a->w;
	wa.h = a->h;
	AG_WidgetSizeAlloc(win, &wa);
	AG_WidgetUpdateCoords(win, 0,0);
	WIDGET(win)->x = a->x;
	WIDGET(win)->y = a->y;

	if (useText)
		AG_PopTextState();

	win->dirty = 1;

	if (agWindowPinnedCount > 0)
		AG_WindowMovePinned(win, a->x - WIDGET(win)->x,
		                         a->y - WIDGET(win)->y);
}

static void
SDL2MW_PostResizeCallback(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)drv;
	AG_SizeAlloc wa;
	AG_Rect rVP;
	const int useText = (win->flags & AG_WINDOW_USE_TEXT);

	SDL_GL_MakeCurrent(smw->window, smw->glCtx);

	if (useText) {
		AG_PushTextState();
		AG_TextFont(WIDGET(win)->font);
		AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
		                                [AG_TEXT_COLOR]);
	}
	
	wa.x = 0;
	wa.y = 0;
	wa.w = a->w;
	wa.h = a->h;
	AG_WidgetSizeAlloc(win, &wa);
	AG_WidgetUpdateCoords(win, 0,0);
	SDL_GetWindowPosition(smw->window, &WIDGET(win)->x, &WIDGET(win)->y);
	win->dirty = 1;

	if (useText)
		AG_PopTextState();

	rVP.x = 0;
	rVP.y = 0;
	rVP.w = WIDTH(win);
	rVP.h = HEIGHT(win);
	AG_GL_SetViewport(&smw->gl, &rVP);
}

static int
SDL2MW_ProcessEvent(void *obj, AG_DriverEvent *dev)
{
	int rv = 0;

	AG_LockVFS(&agDrivers);

	if (AG_SDL2_ProcessEvent_MW(obj, dev) == 0) {
		AG_SizeAlloc a;
		AG_Window *win;

		if ((win = dev->win) == NULL) {
			rv = 0;
			goto out;
		}

		switch (dev->type) {
		case AG_DRIVER_EXPOSE:
			win->dirty = 1;
			break;
		case AG_DRIVER_MOVED:
			a.x = dev->moved.x;
			a.y = dev->moved.y;
			if (a.x != WIDGET(win)->x || a.y != WIDGET(win)->y) {
				a.w = WIDGET(win)->w;
				a.h = WIDGET(win)->h;
				SDL2MW_PostMoveCallback(win, &a);
			}
			rv = 1;
			break;
		case AG_DRIVER_VIDEORESIZE:
			a.x = dev->videoresize.x;
			a.y = dev->videoresize.y;
			a.w = dev->videoresize.w;
			a.h = dev->videoresize.h;
			if (a.w != WIDTH(win) || a.h != HEIGHT(win)) {
				SDL2MW_PostResizeCallback(win, &a);
			}
			rv = 1;
			break;
		default:
			break;
		}
	}
out:
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

static void
SDL2MW_BeginRendering(void *_Nonnull obj)
{
	AG_DriverSDL2MW *smw = obj;

	SDL_GL_MakeCurrent(smw->window, smw->glCtx);
}

static void
SDL2MW_RenderWindow(AG_Window *_Nonnull win)
{
	const AG_Color *cBg = &WCOLOR(win, BG_COLOR);

	AG_PushClipRect(win, &WIDGET(win)->r);

	glClearColor((float)cBg->r / AG_COLOR_LASTF,
	             (float)cBg->g / AG_COLOR_LASTF,
		     (float)cBg->b / AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	AG_WidgetDraw(win);

	AG_PopClipRect(win);
}

static void
SDL2MW_EndRendering(void *_Nonnull obj)
{
	AG_DriverSDL2MW *smw = obj;
	AG_GL_Context *gl = &smw->gl;
	Uint i;

#if 0
	if (smw->outMode != AG_SDL2GL_OUT_NONE)            /* Capture output */
		SDL2MW_CaptureOutput(smw);
#endif
	SDL_GL_SwapWindow(smw->window);

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(gl->nTextureGC, (const GLuint *)gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;
}

/*
 * Window operations
 */

static int
SDL2MW_OpenWindow(AG_Window *_Nonnull win, const AG_Rect *_Nonnull r,
    int depthReq, Uint mwFlags)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;
	AG_Driver *drv = WIDGET(win)->drv;
	Uint32 swFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	SDL_Surface *Swin;
	AG_Rect rVP;

	if (win->flags & AG_WINDOW_NOBORDERS) { swFlags |= SDL_WINDOW_BORDERLESS; }
	if (win->flags & AG_WINDOW_NORESIZE)  { swFlags &= ~(SDL_WINDOW_RESIZABLE); }
	if (win->flags & AG_WINDOW_MINIMIZED) { swFlags |= SDL_WINDOW_MINIMIZED; }
	if (win->flags & AG_WINDOW_MAXIMIZED) { swFlags |= SDL_WINDOW_MAXIMIZED; }
	if (win->flags & AG_WINDOW_KEEPABOVE) { swFlags |= SDL_WINDOW_ALWAYS_ON_TOP; }

	switch (win->wmType) {
	case AG_WINDOW_WM_MENU:
	case AG_WINDOW_WM_POPUP_MENU:
	case AG_WINDOW_WM_DROPDOWN_MENU:
	case AG_WINDOW_WM_COMBO:
		swFlags |= SDL_WINDOW_POPUP_MENU;
		swFlags |= SDL_WINDOW_SKIP_TASKBAR;
		break;
	case AG_WINDOW_WM_UTILITY:
		swFlags |= SDL_WINDOW_UTILITY;
		swFlags |= SDL_WINDOW_SKIP_TASKBAR;
		break;
	case AG_WINDOW_WM_DOCK:
	case AG_WINDOW_WM_TOOLBAR:
	case AG_WINDOW_WM_TOOLTIP:
	case AG_WINDOW_WM_NOTIFICATION:
	case AG_WINDOW_WM_DND:
		swFlags |= SDL_WINDOW_SKIP_TASKBAR;
		break;
	default:
		break;
	}

	if (mwFlags & AG_DRIVER_MW_ANYPOS) {
		smw->window = SDL_CreateWindow(win->caption,
		    SDL_WINDOWPOS_UNDEFINED,
		    SDL_WINDOWPOS_UNDEFINED,
		    r->w,
		    r->h,
		    swFlags);
	} else {
		smw->window = SDL_CreateWindow(win->caption,
		    r->x,
		    r->y,
		    r->w,
		    r->h,
		    swFlags);
	}
	if (smw->window == NULL) {
		AG_SetError("SDL_CreateWindow: %s", SDL_GetError());
		return (-1);
	}

	/* For AG_SDL_GetWindowFromID(). */
	AGDRIVER_MW(drv)->windowID = SDL_GetWindowID(smw->window);

	/* Get the preferred pixel format for the window. */
	Swin = SDL_GetWindowSurface(smw->window);
	drv->videoFmt = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatRGBA(drv->videoFmt,
	    Swin->format->BitsPerPixel,
	    Swin->format->Rmask,
	    Swin->format->Gmask,
	    Swin->format->Bmask,
	    Swin->format->Amask);

	if ((smw->glCtx = SDL_GL_CreateContext(smw->window)) == NULL) {
		AG_SetError("SDL_GL_CreateContext: %s", SDL_GetError());
		SDL_DestroyWindow(smw->window);
		smw->window = NULL;
		return (-1);
	}

#ifdef DEBUG_DISPLAY
	Debug(smw, "New display (%d x %d x %d bpp)\n",
	    Swin->w, Swin->h, Swin->format->BitsPerPixel);
#endif
	/* Create the cursors. */
	AG_SDL2_InitDefaultCursor(smw);
	AG_InitStockCursors(drv);

	/* Initialize our OpenGL context and viewport. */
	AG_GL_InitContext(smw, &smw->gl);
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = Swin->w;
	rVP.h = Swin->h;
	AG_GL_SetViewport(&smw->gl, &rVP);
	
	AG_InitStockCursors(drv);
	return (0);
}

static int
SDL2MW_MapWindow(AG_Window *_Nonnull win)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	if (win->wMin > 0 && win->hMin > 0) {
		SDL_SetWindowMinimumSize(smw->window, win->wMin, win->hMin);
	}
	SDL_ShowWindow(smw->window);
	return (0);
}

static void
SDL2MW_CloseWindow(AG_Window *_Nonnull win)
{
	AG_Driver *drv = WIDGET(win)->drv;
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)drv;

	SDL_GL_MakeCurrent(smw->window, smw->glCtx);

	AG_FreeCursors(drv);

	AG_GL_DestroyContext(smw);
	SDL_GL_DeleteContext(smw->glCtx);

	SDL_DestroyWindow(smw->window);
	smw->window = NULL;

	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
}

static int
SDL2MW_UnmapWindow(AG_Window *_Nonnull win)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_HideWindow(smw->window);
	return (0);
}

static int
SDL2MW_RaiseWindow(AG_Window *_Nonnull win)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_RaiseWindow(smw->window);
	return (0);
}

static int
SDL2MW_LowerWindow(AG_Window *_Nonnull win)
{
	AG_SetErrorS("Not implemented");
	return (-1);
}

static int
SDL2MW_ReparentWindow(AG_Window *_Nonnull win, AG_Window *_Nonnull winParent,
    int x, int y)
{
	AG_SetErrorS("Not implemented");
	return (-1);
}

static int
SDL2MW_GetInputFocus(AG_Window *_Nonnull *_Nonnull rv)
{
	AG_SetErrorS("Not implemented");
	return (-1);
}

static int
SDL2MW_SetInputFocus(AG_Window *_Nonnull win)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	if (SDL_SetWindowInputFocus(smw->window) != 0) {
		AG_SetError("SDL_SetWindowInputFocus: %s", SDL_GetError());
		return (-1);
	}
	return (0);
}

#if 0
static void
SDL2MW_FreeWidgetResources(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		SDL2MW_FreeWidgetResources(chld);
	}
	AG_WidgetFreeResourcesGL(wid);
}

static void
RegenWidgetResources(AG_Widget *_Nonnull wid)
{
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		RegenWidgetResources(chld);
	}
	AG_WidgetRegenResourcesGL(wid);
}
#endif

static void
SDL2MW_PreResizeCallback(AG_Window *_Nonnull win)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_GL_MakeCurrent(smw->window, smw->glCtx);
}

static int
SDL2MW_MoveWindow(AG_Window *_Nonnull win, int dx, int dy)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;
	
	SDL_SetWindowPosition(smw->window, dx, dy);
	return (0);
}

static int
SDL2MW_ResizeWindow(AG_Window *_Nonnull win, Uint w, Uint h)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_SetWindowSize(smw->window, (int)w, (int)h);
	return (0);
}

static int
SDL2MW_MoveResizeWindow(AG_Window *_Nonnull win, AG_SizeAlloc *_Nonnull a)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;
/*	int w, h; */

	SDL_SetWindowSize(smw->window, a->w, a->h);
#if 0
	SDL_GetWindowSize(smw->window, &w, &h);
	if (w != a->w || h != a->h) {
		SDL_SetErrorS("SDL_SetWindowSize failed");
		return (-1);
	}
#endif
	SDL_SetWindowPosition(smw->window, a->x, a->y);
	return (0);
}

static int
SDL2MW_SetBorderWidth(AG_Window *_Nonnull win, Uint width)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_SetWindowBordered(smw->window, (width > 0) ? SDL_TRUE : SDL_FALSE);
	return (0);
}

static int
SDL2MW_SetWindowCaption(AG_Window *_Nonnull win, const char *_Nonnull s)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;

	SDL_SetWindowTitle(smw->window, s);
	return (0);
}

static void
SDL2MW_SetWindowMinSize(AG_Window *_Nonnull win, int w, int h)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;
	
	SDL_SetWindowMinimumSize(smw->window, w,h);
}

static void
SDL2MW_SetWindowMaxSize(AG_Window *_Nonnull win, int w, int h)
{
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)WIDGET(win)->drv;
	
	SDL_SetWindowMaximumSize(smw->window, w,h);
}

static void
SDL2MW_SetMouseAutoCapture(void *_Nonnull obj, int state)
{
	if (state == 0) {
		SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
	} else if (state == -1) {
		SDL_ResetHint(SDL_HINT_MOUSE_AUTO_CAPTURE);
	} else {
		SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "1");
	}
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)

static void
PollGLContext(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_DriverSDL2MW *smw = (AG_DriverSDL2MW *)AG_PTR(1);
	const AG_GL_Context *ctx = &smw->gl;
	AG_TlistItem *it;
	Uint i;

	if (!AG_OBJECT_VALID(smw) ||
	    !AG_OfClass(smw, "AG_Driver:AG_DriverMw:AG_DriverSDL2MW:*")) {
		if (WIDGET(tl)->window != NULL) {
			AG_ObjectDetach(WIDGET(tl)->window);
			return;
		}
		return;
	}

	AG_TlistBegin(tl);

	for (i = 0; i < ctx->nClipRects; i++) {
		AG_ClipRect *cr = &ctx->clipRects[i];

		it = AG_TlistAdd(tl, NULL,
		    _("Clipping Rectangle #" AGSI_BOLD "%d" AGSI_RST
		      " (%dx%d) at [" AGSI_BOLD "%d,%d" AGSI_BOLD "]"),
		    i, cr->r.x, cr->r.y, cr->r.w, cr->r.h);
		it->p1 = cr;
	}

	for (i = 0; i < ctx->nBlendStates; i++) {
		AG_GL_BlendState *bs = &ctx->blendStates[i];

		it = AG_TlistAdd(tl, NULL,
		    _("Blending State #" AGSI_BOLD "%d" AGSI_RST
		      " (" AGSI_BOLD "%s" AGSI_RST ", SrcFac=%d, DstFac=%d)"),
		    i, bs->enabled ? _("Enabled") : _("Disabled"),
		    bs->srcFactor, bs->dstFactor);
		it->p1 = bs;
	}

	for (i = 0; i < ctx->nTextureGC; i++) {
		it = AG_TlistAdd(tl, NULL,
		    _("Texture Delete #" AGSI_BOLD "%d" AGSI_RST " (Texture=%u)"),
		    i, ctx->textureGC[i]);
		it->p1 = &ctx->textureGC[i];
	}

	for (i = 0; i < ctx->nListGC; i++) {
		it = AG_TlistAdd(tl, NULL,
		    _("List Delete #" AGSI_BOLD "%d" AGSI_RST " (List=%u)"),
		    i, ctx->listGC[i]);
		it->p1 = &ctx->listGC[i];
	}

	AG_TlistEnd(tl);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	AG_DriverSDL2MW *smw = obj;
	AG_Window *win;
	AG_Label *lbl;
	AG_Tlist *tl;
	AG_Keyboard *kbd = AGDRIVER(smw)->kbd;
	AG_Mouse *mouse = AGDRIVER(smw)->mouse;
	AG_Notebook *nb;
	AG_NotebookTab *nt;

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetPosition(win, AG_WINDOW_BL, 0);

	lbl = AG_LabelNew(win, 0, _("SDL2MW Driver: %s"), OBJECT(smw)->name);
	AG_SetFontFamily(lbl, "league-spartan");
	AG_SetFontSize(lbl, "150%");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);

	if (OBJECT_CLASS(kbd)->edit != NULL) {
		nt = AG_NotebookAdd(nb, _("Keyboard"), AG_BOX_VERT);
		AG_ObjectAttach(nt, OBJECT_CLASS(kbd)->edit(kbd));
	}
	if (OBJECT_CLASS(mouse)->edit != NULL) {
		nt = AG_NotebookAdd(nb, _("Mouse"), AG_BOX_VERT);
		AG_ObjectAttach(nt, OBJECT_CLASS(mouse)->edit(mouse));
	}
	nt = AG_NotebookAdd(nb, _("OpenGL"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0, _("Pushed GL States:"));
		tl = AG_TlistNewPolled(nt, AG_TLIST_EXPAND, PollGLContext,"%p",smw);
		AG_SetFontSize(tl, "80%");
		AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 4);
	}
#if 0
	nt = AG_NotebookAdd(nb, _("WM States"), AG_BOX_VERT);
	{
		AG_PushDisabledState(nt);
		AG_CheckboxNewInt(nt, 0, _("Pointer Is Grabbed"), &smw->ptrIsGrabbed);
		AG_CheckboxNewInt(nt, 0, _("WM Hints Are Set"), &smw->wmHintsSet);
		AG_CheckboxNewInt(nt, 0, _("Clipboard Selection Waiting"), &agClipboardSelectionWaiting);
		AG_PopDisabledState(nt);
	}
#endif
	return (win);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_DriverMwClass agDriverSDL2MW = {
	{
		{
			"AG_Driver:AG_DriverMw:AG_DriverSDL2MW",
			sizeof(AG_DriverSDL2MW),
			{ 1,7 },
			Init,
			NULL,		/* reset */
			NULL,		/* destroy */
			NULL,		/* load */
			NULL,		/* save */
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
			Edit,
#else
			NULL		/* edit */
#endif

		},
		"sdl2mw",
		AG_VECTOR,
		AG_WM_MULTIPLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES | AG_DRIVER_SDL2,
		SDL2MW_Open,
		SDL2MW_Close,
		AG_SDL2_GetDisplaySize,
		AG_SDL2_BeginEventProcessing,
		AG_SDL2_PendingEvents,
		AG_SDL2_GetNextEvent,
		SDL2MW_ProcessEvent,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDL2MW_BeginRendering,
		SDL2MW_RenderWindow,
		SDL2MW_EndRendering,
		AG_GL_FillRect,
		NULL,				/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		NULL,				/* setRefreshRate */
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		AG_SDL2_CreateCursor,
		AG_SDL2_FreeCursor,
		AG_SDL2_SetCursor,
		AG_SDL2_UnsetCursor,
		AG_SDL2_GetCursorVisibility,
		AG_SDL2_SetCursorVisibility,
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
#endif
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		AG_GL_PutPixel64,
		AG_GL_PutPixelRGB16,
#endif
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawLineW,
		AG_GL_DrawLineW_Sti16,
		AG_GL_DrawTriangle,
		AG_GL_DrawPolygon,
		AG_GL_DrawPolygon_Sti32,
		AG_GL_DrawArrow,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList,
		NULL,				/* getClipboardText */
		NULL,				/* setClipboardText */
		SDL2MW_SetMouseAutoCapture
	},
	SDL2MW_OpenWindow,
	SDL2MW_CloseWindow,
	SDL2MW_MapWindow,
	SDL2MW_UnmapWindow,
	SDL2MW_RaiseWindow,
	SDL2MW_LowerWindow,
	SDL2MW_ReparentWindow,
	SDL2MW_GetInputFocus,
	SDL2MW_SetInputFocus,
	SDL2MW_MoveWindow,
	SDL2MW_ResizeWindow,
	SDL2MW_MoveResizeWindow,
	SDL2MW_PreResizeCallback,
	SDL2MW_PostResizeCallback,
	SDL2MW_SetBorderWidth,
	SDL2MW_SetWindowCaption,
	NULL,				/* setTransientFor */
	NULL,				/* setOpacity */
	NULL,				/* tweakAlignment */
	SDL2MW_SetWindowMinSize,
	SDL2MW_SetWindowMaxSize
};
