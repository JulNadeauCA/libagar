/*
 * Copyright (c) 2009-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Driver for OpenGL graphics via the SDL 1.2 library.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>
#include <agar/gui/sdl.h>

enum ag_sdlgl_out {
	AG_SDLGL_OUT_NONE,		/* No capture */
	AG_SDLGL_OUT_JPEG,		/* Output JPEG files */
	AG_SDLGL_OUT_PNG		/* Output PNG files */
};

typedef struct ag_sdlgl_driver {
	struct ag_driver_sw _inherit;
	SDL_Surface  *s;		/* View surface */
	AG_GL_Context gl;		/* Common OpenGL context data */
	enum ag_sdlgl_out outMode;	/* Output capture mode */
	char		 *outPath;	/* Output capture path */
	Uint		  outFrame;	/* Capture frame# counter */
	Uint		  outLast;	/* Terminate after this many frames */
	Uint8		 *outBuf;	/* Output capture buffer */
	Uint		  outJpegQual;	/* Quality (%) for jpeg output */
	Uint		  outJpegFlags;	/* DCT options */
} AG_DriverSDLGL;

static int nDrivers = 0;			/* Opened driver instances */
static int initedSDL = 0;			/* Used SDL_Init() */
static int initedSDLVideo = 0;			/* Used SDL_INIT_VIDEO */
static AG_EventSink *sglEventSpinner = NULL;	/* Standard event sink */
static AG_EventSink *sglEventEpilogue = NULL;	/* Standard event epilogue */

static void
Init(void *obj)
{
	AG_DriverSDLGL *sgl = obj;

	sgl->s = NULL;
}

/*
 * Generic driver operations
 */

static int
SDLGL_Open(void *obj, const char *spec)
{
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
#if 0
	/* Use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);
	AG_DestroyEventSubsystem();
	AG_InitEventSubsystem(AG_SOFT_TIMERS);
#endif
	/* Initialize this driver instance. */
	if ((drv->mouse = AG_MouseNew(sgl, "SDL mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(sgl, "SDL keyboard")) == NULL) {
		goto fail;
	}
	sgl->outMode = AG_SDLGL_OUT_NONE;
	sgl->outPath = NULL;
	sgl->outFrame = 0;
	sgl->outLast = 0;
	sgl->outBuf = NULL;
	sgl->outJpegQual = 100;
	sgl->outJpegFlags = 0;
	
	/* Configure the window caption */
	if (agProgName != NULL)
		SDL_WM_SetCaption(agProgName, agProgName);

	/*
	 * TODO where AG_SINK_READ capability and pipes are available,
	 * could we create a separate thread running SDL_WaitEvent() and
	 * sending notifications over a pipe, instead of using a spinner?
	 */
	if ((sglEventSpinner = AG_AddEventSpinner(AG_SDL_EventSink, "%p", drv)) == NULL ||
	    (sglEventEpilogue = AG_AddEventEpilogue(AG_SDL_EventEpilogue, NULL)) == NULL)
		goto fail;

	/* Set up event filters for standard AG_EventLoop(). */
	nDrivers = 1;
	return (0);
fail:
	if (sglEventSpinner != NULL) { AG_DelEventSpinner(sglEventSpinner); sglEventSpinner = NULL; }
	if (sglEventEpilogue != NULL) { AG_DelEventEpilogue(sglEventEpilogue); sglEventEpilogue = NULL; }
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
SDLGL_Close(void *obj)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	
	AG_DelEventSpinner(sglEventSpinner); sglEventSpinner = NULL;
	AG_DelEventEpilogue(sglEventEpilogue); sglEventEpilogue = NULL;

	if (drv->gl != NULL)
		AG_GL_DestroyContext(sgl);

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sgl));

	if (dsw->flags & AG_DRIVER_SW_FULLSCREEN) {
		if (SDL_WM_ToggleFullScreen(sgl->s))
			dsw->flags &= ~(AG_DRIVER_SW_FULLSCREEN);
	}
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd); drv->kbd = NULL;

	if (sgl->outBuf != NULL) {
		free(sgl->outBuf);
		sgl->outBuf = NULL;
	}

	nDrivers = 0;
}

static void
SDLGL_BeginRendering(void *obj)
{
	AG_DriverSDLGL *sgl = obj;
	AG_GL_Context *gl = &sgl->gl;

	glPushAttrib(GL_VIEWPORT_BIT|GL_TRANSFORM_BIT|GL_LIGHTING_BIT|
	             GL_ENABLE_BIT);
	
	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		AG_Driver *drv = obj;

		/* Reinitialize Agar's OpenGL context. */
		if (drv->gl != NULL) {
			AG_GL_DestroyContext(drv);
		}
		if (AG_GL_InitContext(drv, gl) == -1) {
			AG_FatalError(NULL);
		}
		AG_GL_SetViewport(gl,
		    AG_RECT(0, 0, AGDRIVER_SW(sgl)->w, AGDRIVER_SW(sgl)->h));
	} else {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}

	gl->clipStates[0] = glIsEnabled(GL_CLIP_PLANE0); glEnable(GL_CLIP_PLANE0);
	gl->clipStates[1] = glIsEnabled(GL_CLIP_PLANE1); glEnable(GL_CLIP_PLANE1);
	gl->clipStates[2] = glIsEnabled(GL_CLIP_PLANE2); glEnable(GL_CLIP_PLANE2);
	gl->clipStates[3] = glIsEnabled(GL_CLIP_PLANE3); glEnable(GL_CLIP_PLANE3);
}

static void
SDLGL_RenderWindow(struct ag_window *win)
{
	AG_WidgetDraw(win);
}

static void
SDLGL_CaptureOutput(AG_DriverSDLGL *sgl)
{
	char path[AG_PATHNAME_MAX];
	AG_DriverSw *dsw = (AG_DriverSw *)sgl;
	AG_Surface *s;

	Snprintf(path, sizeof(path), sgl->outPath, sgl->outFrame);
	glReadPixels(0, 0, dsw->w, dsw->h, GL_RGBA, GL_UNSIGNED_BYTE,
	    sgl->outBuf);

	if (AG_PackedPixelFlip(sgl->outBuf, dsw->h, dsw->w*4) == -1) {
		goto fail_disable;
	}
	s = AG_SurfaceFromPixelsRGBA(sgl->outBuf,
	    dsw->w, dsw->h, 32,
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0);
	if (s == NULL)
		goto fail;

	switch (sgl->outMode) {
	case AG_SDLGL_OUT_JPEG:
		if (AG_SurfaceExportJPEG(s, path, sgl->outJpegQual,
		    sgl->outJpegFlags) == -1) {
			goto fail;
		}
		break;
	case AG_SDLGL_OUT_PNG:
		if (AG_SurfaceExportPNG(s, path, 0) == -1) {
			goto fail;
		}
		break;
	default:
		break;
	}

	if (++sgl->outFrame == sgl->outLast) {
		Verbose("SDLGL: Reached last frame; terminating\n");
		AG_Terminate(0);
	}
	AG_SurfaceFree(s);
	return;
fail:
	AG_SurfaceFree(s);
fail_disable:
	Verbose("SDLGL: %s; disabling capture\n", AG_GetError());
	sgl->outMode = AG_SDLGL_OUT_NONE;
}

static void
SDLGL_EndRendering(void *drv)
{
	AG_DriverSDLGL *sgl = drv;
	AG_GL_Context *gl = &sgl->gl;
	
	/* Render to specified capture output. */
	if (sgl->outMode != AG_SDLGL_OUT_NONE)
		SDLGL_CaptureOutput(sgl);

	glPopAttrib();
	
	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		/*
		 * Restore the OpenGL state exactly to its former state
		 * (all textures are display lists are deleted).
		 */
		AG_GL_DestroyContext(gl);
	} else {
		SDL_GL_SwapBuffers();
		if (gl->clipStates[0])	{ glEnable(GL_CLIP_PLANE0); }
		else			{ glDisable(GL_CLIP_PLANE0); }
		if (gl->clipStates[1])	{ glEnable(GL_CLIP_PLANE1); }
		else			{ glDisable(GL_CLIP_PLANE1); }
		if (gl->clipStates[2])	{ glEnable(GL_CLIP_PLANE2); }
		else			{ glDisable(GL_CLIP_PLANE2); }
		if (gl->clipStates[3])	{ glEnable(GL_CLIP_PLANE3); }
		else			{ glDisable(GL_CLIP_PLANE3); }
	}
}

/*
 * Operations specific to single-display drivers.
 */

static __inline__ void
ClearBackground(AG_DriverSw *dsw)
{
	glClearColor(dsw->bgColor.r/255.0,
	             dsw->bgColor.g/255.0,
		     dsw->bgColor.b/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static int
SDLGL_OpenVideo(void *obj, Uint w, Uint h, int depth, Uint flags)
{
	char buf[256];
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
	
	if (AG_Defined(drv, "out")) {
		char *ext;

		AG_GetString(drv, "out", buf, sizeof(buf));
		if ((ext = strrchr(buf, '.')) != NULL &&
		    ext[1] != '\0') {
			if (Strcasecmp(&ext[1], "jpeg") == 0 ||
			    Strcasecmp(&ext[1], "jpg") == 0) {
				sgl->outMode = AG_SDLGL_OUT_JPEG;
				if ((sgl->outPath = TryStrdup(buf)) == NULL)
					return (-1);
			} else if (Strcasecmp(&ext[1], "png") == 0) {
				sgl->outMode = AG_SDLGL_OUT_PNG;
				if ((sgl->outPath = TryStrdup(buf)) == NULL)
					return (-1);
			} else {
				AG_SetError("Invalid out= argument: `%s'", buf);
				return (-1);
			}
			if (AG_Defined(drv, "outFirst")) {
				AG_GetString(drv, "outFirst", buf, sizeof(buf));
				sgl->outFrame = atoi(buf);
			} else {
				sgl->outFrame = 0;
			}
			if (AG_Defined(drv, "outLast")) {
				AG_GetString(drv, "outLast", buf, sizeof(buf));
				sgl->outLast = atoi(buf);
			}
		}
		if (AG_Defined(drv, "jpegQual")) {
			AG_GetString(drv, "jpegQual", buf, sizeof(buf));
			sgl->outJpegQual = atoi(buf);
		}
		if (AG_Defined(drv, "jpegDCT")) {
			AG_GetString(drv, "jpegDCT", buf, sizeof(buf));
			if (Strcasecmp(buf, "islow")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_ISLOW;
			} else if (Strcasecmp(buf, "ifast")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_IFAST;
			} else if (Strcasecmp(buf, "float")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_FLOAT;
			}
		}
	}
	
	/* Set the video mode. Force hardware palette in 8bpp. */
	AG_SDL_GetPrefDisplaySettings(drv, &w, &h, &depth);
	Verbose(_("SDLGL: Setting mode %ux%u (%d bpp)\n"), w, h, depth);
	newDepth = SDL_VideoModeOK(w, h, depth, sFlags);
	if (newDepth == 8) {
		Verbose(_("SDLGL: Using hardware palette\n"));
		sFlags |= SDL_HWPALETTE;
	}
	if ((sgl->s = SDL_SetVideoMode((int)w, (int)h, newDepth, sFlags))
	    == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s", w, h, newDepth,
		    SDL_GetError());
		return (-1);
	}
	SDL_EnableUNICODE(1);

	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: New display (%dbpp)\n"),
	     (int)drv->videoFmt->BitsPerPixel);
	
	/* Create the cursors. */
	if (AG_SDL_InitDefaultCursor(sgl) == -1 ||
	    AG_InitStockCursors(drv) == -1)
		goto fail;
	
	/* Initialize our OpenGL context and viewport. */
	if (AG_GL_InitContext(sgl, &sgl->gl) == -1) {
		goto fail;
	}
	AG_GL_SetViewport(&sgl->gl, AG_RECT(0, 0, dsw->w, dsw->h));

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY))
		ClearBackground(dsw);

	/* Initialize the output capture buffer. */
	Free(sgl->outBuf);
	if (sgl->outMode != AG_SDLGL_OUT_NONE) {
		if ((sgl->outBuf = AG_TryMalloc(dsw->w*dsw->h*4)) == NULL) {
			Verbose("SDLGL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDLGL_OUT_NONE;
		}
	}
	
	if (flags & AG_VIDEO_FULLSCREEN) {
		if (SDL_WM_ToggleFullScreen(sgl->s))
			dsw->flags |= AG_DRIVER_SW_FULLSCREEN;
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
SDLGL_OpenVideoContext(void *obj, void *ctx, Uint flags)
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
	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: Using existing display (%ux%ux%d bpp)\n"),
	    dsw->w, dsw->h, (int)drv->videoFmt->BitsPerPixel);
	
	/* Initialize our OpenGL context and viewport. */
	if (AG_GL_InitContext(sgl, &sgl->gl) == -1) {
		goto fail;
	}
	AG_GL_SetViewport(&sgl->gl, AG_RECT(0, 0, dsw->w, dsw->h));

	/* Create the cursors. */
	if (AG_SDL_InitDefaultCursor(sgl) == -1 ||
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
SDLGL_CloseVideo(void *obj)
{
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
SDLGL_VideoResize(void *obj, Uint w, Uint h)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	Uint32 sFlags;
	SDL_Surface *su;
	AG_Window *win;

	sFlags = sgl->s->flags & (SDL_SWSURFACE|SDL_HWSURFACE|SDL_ASYNCBLIT|
				  SDL_ANYFORMAT|SDL_HWPALETTE|SDL_DOUBLEBUF|
				  SDL_FULLSCREEN|SDL_OPENGL|SDL_OPENGLBLIT|
				  SDL_RESIZABLE|SDL_NOFRAME);
	                          

	/* Backup all widget surfaces prior to GL context loss. */
	AG_FOREACH_WINDOW(win, sgl) {
		AG_WidgetFreeResourcesGL(win);
	}

	/* Invalidate the font cache. */
	AG_TextClearGlyphCache(drv);
	
	if ((su = SDL_SetVideoMode(w, h, 0, sFlags)) == NULL) {
		AG_SetError("Cannot resize display to %ux%u: %s", w, h,
		    SDL_GetError());
		return (-1);
	}
	sgl->s = su;

	dsw->w = su->w;
	dsw->h = su->h;
	dsw->depth = (Uint)su->format->BitsPerPixel;

	/* Resize the output capture buffer. */
	if (sgl->outBuf != NULL) {
		free(sgl->outBuf);
		if ((sgl->outBuf = AG_TryMalloc(dsw->w*dsw->h*4)) == NULL) {
			Verbose("SDLGL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDLGL_OUT_NONE;
		}
	}

	/* Update the viewport coordinates. */
	AG_GL_SetViewport(&sgl->gl, AG_RECT(0, 0, dsw->w, dsw->h));
	
	/* Regenerate all widget textures. */
	AG_FOREACH_WINDOW(win, sgl) {
		AG_WidgetRegenResourcesGL(win);
		win->dirty = 1;
	}

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY))
		ClearBackground(dsw);

	return (0);
}

static int
SDLGL_VideoCapture(void *obj, AG_Surface **sp)
{
#if 0
	AG_DriverSDLGL *sgl = obj;
	AG_Surface *s;

	if ((s = AG_SurfaceDup(sgl->s)) == NULL) {
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
SDLGL_VideoClear(void *obj, AG_Color c)
{
	ClearBackground(obj);
}

static int
SDLGL_SetVideoContext(void *obj, void *pSurface)
{
	AG_DriverSDLGL *sgl = obj;
	AG_GL_Context *gl = &sgl->gl;
	AG_DriverSw *dsw = obj;
	SDL_Surface *su = pSurface;

	sgl->s = su;
	dsw->w = su->w;
	dsw->h = su->h;
	dsw->depth = (Uint)su->format->BitsPerPixel;

	if (dsw->flags & AG_DRIVER_SW_OVERLAY) {
		AG_ClipRect *cr0 = &gl->clipRects[0];

		/* Just update clipping rectangle 0. */
		cr0->r.w = su->w;
		cr0->r.h = su->h;
	} else {
		AG_GL_SetViewport(gl, AG_RECT(0, 0, su->w, su->h));
	}
	return (0);
}

AG_DriverSwClass agDriverSDLGL = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDLGL",
			sizeof(AG_DriverSDLGL),
			{ 1,5 },
			Init,
			NULL,	/* reinit */
			NULL,	/* destroy */
			NULL,	/* load */
			NULL,	/* save */
			NULL,	/* edit */
		},
		"sdlgl",
		AG_VECTOR,
		AG_WM_SINGLE,
		AG_DRIVER_SDL|AG_DRIVER_OPENGL|AG_DRIVER_TEXTURES,
		SDLGL_Open,
		SDLGL_Close,
		AG_SDL_GetDisplaySize,
		AG_SDL_BeginEventProcessing,
		AG_SDL_PendingEvents,
		AG_SDL_GetNextEvent,
		AG_SDL_ProcessEvent,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDLGL_BeginRendering,
		SDLGL_RenderWindow,
		SDLGL_EndRendering,
		AG_GL_FillRect,
		NULL,				/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		AG_SDL_SetRefreshRate,
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		AG_SDL_CreateCursor,
		AG_SDL_FreeCursor,
		AG_SDL_SetCursor,
		AG_SDL_UnsetCursor,
		AG_SDL_GetCursorVisibility,
		AG_SDL_SetCursorVisibility,
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
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList
	},
	0,
	SDLGL_OpenVideo,
	SDLGL_OpenVideoContext,
	SDLGL_SetVideoContext,
	SDLGL_CloseVideo,
	SDLGL_VideoResize,
	SDLGL_VideoCapture,
	SDLGL_VideoClear
};
