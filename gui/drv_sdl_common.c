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
 * Common code for SDL 1.2 drivers.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl.h>

#define AG_SDL_CLIPPED_PIXEL(s, ax, ay)			\
	((ax) < (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x+(s)->clip_rect.w ||	\
	 (ay) < (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y+(s)->clip_rect.h)

/*
 * Initialize Agar with an existing SDL display. If the display surface has
 * flags SDL_OPENGL / SDL_OPENGLBLIT set, the "sdlgl" driver is selected;
 * otherwise, the "sdlfb" driver is used.
 */
int
AG_InitVideoSDL(void *pDisplay, Uint flags)
{
	SDL_Surface *display = pDisplay;
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL;
	int useGL = 0;
	int i;

	if (AG_InitGUIGlobals() == -1)
		return (-1);
	
	/* Enable OpenGL mode if the surface has SDL_OPENGL set. */
	if ((display->flags & SDL_OPENGL)
#ifdef SDL_OPENGLBLIT
	    || (display->flags & SDL_OPENGLBLIT)
#endif
	    ) {
		if (flags & AG_VIDEO_SDL) {
			AG_SetError("AG_VIDEO_SDL flag requested, but "
			            "display surface has SDL_OPENGL set");
			goto fail;
		}
		useGL = 1;
	} else {
		if (flags & AG_VIDEO_OPENGL) {
			AG_SetError("AG_VIDEO_OPENGL flag requested, but "
			            "display surface is missing SDL_OPENGL");
			goto fail;
		}
	}
	for (i = 0; i < agDriverListSize; i++) {
		dc = agDriverList[i];
		if ((dc->wm == AG_WM_SINGLE) &&
		    (dc->flags & AG_DRIVER_SDL)) {
			if (useGL) {
				if (!(dc->flags & AG_DRIVER_OPENGL))
					continue;
			} else {
				if (dc->flags & AG_DRIVER_OPENGL)
					continue;
			}
			break;
		}
	}
	if (i == agDriverListSize) {
		AG_SetError("No compatible %s driver is available",
		    useGL ? "SDL/OpenGL" : "SDL");
		goto fail;
	}
	dc = agDriverList[i];
	if ((drv = AG_DriverOpen(dc)) == NULL) {
		goto fail;
	}

	/* Open a video display. */
	if (AGDRIVER_SW_CLASS(drv)->openVideoContext(drv, (void *)display,
	    flags) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
	if (drv->videoFmt == NULL)
		AG_FatalError("Driver did not set video format");

	/* Generic Agar-GUI initialization. */
	if (AG_InitGUI(0) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}

	agDriverOps = dc;
	agDriverSw = AGDRIVER_SW(drv);
	return (0);
fail:
	AG_DestroyGUIGlobals();
	return (-1);
}

/*
 * Reattach to a different SDL display surface.
 */
int
AG_SetVideoSurfaceSDL(void *pDisplay)
{
	if (agDriverSw == NULL ||
	    !(agDriverOps->flags & AG_DRIVER_SDL)) {
		AG_SetError("Current driver is not an SDL driver");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(agDriverSw)->setVideoContext(agDriverSw,
	    pDisplay) == -1) {
		return (-1);
	}
	AG_PostResizeDisplay(agDriverSw);
	return (0);
}

/* Return the corresponding Agar PixelFormat structure for a SDL_Surface. */
AG_PixelFormat *
AG_SDL_GetPixelFormat(SDL_Surface *su)
{
	switch (su->format->BytesPerPixel) {
	case 1:
		return AG_PixelFormatIndexed(su->format->BitsPerPixel);
	case 2:
	case 3:
	case 4:
		if (su->format->Amask != 0) {
			return AG_PixelFormatRGBA(su->format->BitsPerPixel,
			                          su->format->Rmask,
			                          su->format->Gmask,
			                          su->format->Bmask,
						  su->format->Amask);
		} else {
			return AG_PixelFormatRGB(su->format->BitsPerPixel,
			                         su->format->Rmask,
			                         su->format->Gmask,
			                         su->format->Bmask);
		}
	default:
		AG_SetError("Unsupported pixel depth (%d bpp)",
		    (int)su->format->BitsPerPixel);
		return (NULL);
	}
}

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given alpha function. No clipping is done. The surface must be locked.
 */
static void
AG_SDL_SurfaceBlendPixel(SDL_Surface *s, Uint8 *pDst, AG_Color Cnew,
    AG_BlendFn fn)
{
	Uint32 px, pxDst;
	AG_Color Cdst;
	Uint8 a;

	AG_PACKEDPIXEL_GET(s->format->BytesPerPixel, pxDst, pDst);
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0)
	if ((s->flags & SDL_SRCCOLORKEY) && (pxDst == s->format->colorkey)) {
		px = SDL_MapRGBA(s->format, Cnew.r, Cnew.g, Cnew.b, Cnew.a);
	 	AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, pDst, px);
	} else 
#endif
	{
		SDL_GetRGBA(pxDst, s->format, &Cdst.r, &Cdst.g, &Cdst.b,
		    &Cdst.a);
		switch (fn) {
		case AG_ALPHA_DST:
			a = Cdst.a;
			break;
		case AG_ALPHA_SRC:
			a = Cnew.a;
			break;
		case AG_ALPHA_ZERO:
			a = 0;
			break;
		case AG_ALPHA_OVERLAY:
			a = (Uint8)((Cdst.a+Cnew.a) > 255) ? 255 :
			            (Cdst.a+Cnew.a);
			break;
		case AG_ALPHA_ONE_MINUS_DST:
			a = 255-Cdst.a;
			break;
		case AG_ALPHA_ONE_MINUS_SRC:
			a = 255-Cnew.a;
			break;
		case AG_ALPHA_ONE:
		default:
			a = 255;
			break;
		}
		px = SDL_MapRGBA(s->format,
		    (((Cnew.r - Cdst.r)*Cnew.a) >> 8) + Cdst.r,
		    (((Cnew.g - Cdst.g)*Cnew.a) >> 8) + Cdst.g,
		    (((Cnew.b - Cdst.b)*Cnew.a) >> 8) + Cdst.b,
		    a);
		AG_PACKEDPIXEL_PUT(s->format->BytesPerPixel, pDst, px);
	}
}

/* Blit an AG_Surface to a target SDL_Surface. */
void
AG_SDL_BlitSurface(const AG_Surface *ss, const AG_Rect *srcRect,
    SDL_Surface *ds, int xDst, int yDst)
{
	AG_Rect sr, dr;
	AG_Color C;
	Uint32 px;
	int x, y;
	Uint8 *pSrc, *pDst;

	/* Compute the effective source and destination rectangles. */
	if (srcRect != NULL) {
		sr = *srcRect;
		if (sr.x < 0) { sr.x = 0; }
		if (sr.y < 0) { sr.y = 0; }
		if (sr.x+sr.w >= ss->w) { sr.w = ss->w - sr.x; }
		if (sr.y+sr.h >= ss->h) { sr.h = ss->h - sr.y; }
	} else {
		sr.x = 0;
		sr.y = 0;
		sr.w = ss->w;
		sr.h = ss->h;
	}
	dr.x = MAX(xDst, ds->clip_rect.x);
	dr.y = MAX(yDst, ds->clip_rect.y);
	dr.w = (dr.x+sr.w > ds->clip_rect.x+ds->clip_rect.w) ?
	        (ds->clip_rect.x+ds->clip_rect.w - dr.x) : sr.w;
	dr.h = (dr.y+sr.h > ds->clip_rect.y+ds->clip_rect.h) ?
	        (ds->clip_rect.y+ds->clip_rect.h - dr.y) : sr.h;

	/* XXX TODO optimized cases */
	/* XXX TODO per-surface alpha */
	if (SDL_MUSTLOCK(ds)) {
		SDL_LockSurface(ds);
	}
	for (y = 0; y < dr.h; y++) {
		pSrc = (Uint8 *)ss->pixels + (sr.y+y)*ss->pitch +
		    sr.x*ss->format->BytesPerPixel;
		pDst = (Uint8 *)ds->pixels + (dr.y+y)*ds->pitch +
		    dr.x*ds->format->BytesPerPixel;
		for (x = 0; x < dr.w; x++) {
			AG_PACKEDPIXEL_GET(ss->format->BytesPerPixel, px, pSrc);
			if ((ss->flags & AG_SRCCOLORKEY) &&
			    (ss->format->colorkey == px)) {
				pSrc += ss->format->BytesPerPixel;
				pDst += ds->format->BytesPerPixel;
				continue;
			}
			C = AG_GetColorRGBA(px, ss->format);
			if ((C.a != AG_ALPHA_OPAQUE) &&
			    (ss->flags & AG_SRCALPHA)) {
				AG_SDL_SurfaceBlendPixel(ds, pDst, C,
				    AG_ALPHA_SRC);
			} else {
				px = SDL_MapRGB(ds->format, C.r, C.g, C.b);
				AG_PACKEDPIXEL_PUT(ds->format->BytesPerPixel,
				    pDst, px);
			}
			pSrc += ss->format->BytesPerPixel;
			pDst += ds->format->BytesPerPixel;
		}
	}
	if (SDL_MUSTLOCK(ds))
		SDL_UnlockSurface(ds);
}

#if 0
#define AG_SDL_GET_PIXEL_COMPONENT(rv, mask, shift, loss)		\
	tmp = (pc & mask) >> shift;					\
	(rv) = (tmp << loss) + (tmp >> (8 - (loss << 1)));

/* Decompose a pixel value to an AG_Color (honor any alpha). */
static __inline__ AG_Color
AG_SDL_GetColorRGBA(Uint32 pc, const SDL_PixelFormat *pf)
{
	AG_Color C;
	Uint tmp;

	if (pf->palette != NULL) {
		SDL_Color sc = pf->palette->colors[(Uint)pc % pf->palette->ncolors];
		C.r = sc.r;
		C.g = sc.g;
		C.b = sc.b;
		return (C);
	}
	AG_SDL_GET_PIXEL_COMPONENT(C.r, pf->Rmask, pf->Rshift, pf->Rloss);
	AG_SDL_GET_PIXEL_COMPONENT(C.g, pf->Gmask, pf->Gshift, pf->Gloss);
	AG_SDL_GET_PIXEL_COMPONENT(C.b, pf->Bmask, pf->Bshift, pf->Bloss);
	if (pf->Amask != 0) {
		AG_SDL_GET_PIXEL_COMPONENT(C.a, pf->Amask, pf->Ashift, pf->Aloss);
	} else {
		C.a = AG_ALPHA_OPAQUE;
	}
	return (C);
}
#endif

/* Convert a SDL_Surface to an AG_Surface. */
AG_Surface *
AG_SDL_ImportSurface(SDL_Surface *ss)
{
	AG_PixelFormat *pf;
	AG_Surface *ds;
	Uint8 *pSrc, *pDst;
	int y;
#if 0
	Uint32 px;
	AG_Color C;
	int x;
#endif

	if ((pf = AG_SDL_GetPixelFormat(ss)) == NULL) {
		return (NULL);
	}
	if (pf->palette != NULL) {
		AG_SetError("Indexed formats not supported");
		AG_PixelFormatFree(pf);
		return (NULL);
	}

	if ((ds = AG_SurfaceNew(AG_SURFACE_PACKED, ss->w, ss->h, pf, 0))
	    == NULL) {
		goto out;
	}
	if (ss->flags & SDL_SRCCOLORKEY) { ds->flags |= AG_SRCCOLORKEY; }
	if (ss->flags & SDL_SRCALPHA) { ds->flags |= AG_SRCALPHA; }
	
	if (SDL_MUSTLOCK(ss)) {
		SDL_LockSurface(ss);
	}
	pSrc = (Uint8 *)ss->pixels;
	pDst = (Uint8 *)ds->pixels;
	for (y = 0; y < ss->h; y++) {
		memcpy(pDst, pSrc, ss->pitch);
		pSrc += ss->pitch;
		pDst += ds->pitch;
	}
#if 0
	for (y = 0; y < ss->h; y++) {
		for (x = 0; x < ss->w; x++) {
			AG_PACKEDPIXEL_GET(ss->format->BytesPerPixel, px, pSrc);
			C = AG_SDL_GetColorRGBA(px, ss->format);
			AG_PUT_PIXEL(ds,pDst,
			    AG_MapColorRGBA(ds->format, C));
			pSrc += ss->format->BytesPerPixel;
			pDst += ds->format->BytesPerPixel;
		}
	}
#endif
	if (SDL_MUSTLOCK(ss))
		SDL_UnlockSurface(ss);

out:
	AG_PixelFormatFree(pf);
	return (ds);
}

/* Convert a SDL surface to Agar surface. */
AG_Surface *
AG_SurfaceFromSDL(void *p)
{
	return AG_SDL_ImportSurface((SDL_Surface *)p);
}

/* Convert an Agar surface to an SDL surface. */
void *
AG_SurfaceExportSDL(const AG_Surface *ss)
{
	Uint32 sdlFlags = SDL_SWSURFACE;
	SDL_Surface *ds;

	if (ss->flags & AG_SRCCOLORKEY) { sdlFlags |= SDL_SRCCOLORKEY; }
	if (ss->flags & AG_SRCALPHA) { sdlFlags |= SDL_SRCALPHA; }
	ds = SDL_CreateRGBSurface(sdlFlags, ss->w, ss->h,
	    ss->format->BitsPerPixel,
	    ss->format->Rmask,
	    ss->format->Gmask,
	    ss->format->Bmask,
	    ss->format->Amask);
	if (ds == NULL) {
		AG_SetError("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}
	AG_SDL_BlitSurface(ss, NULL, ds, 0,0);
	return (void *)(ds);
}

/* Initialize the default cursor. */
int
AG_SDL_InitDefaultCursor(void *obj)
{
	AG_Driver *drv = AGDRIVER(obj);
	AG_Cursor *ac;
	SDL_Cursor *sc;
	
	if ((sc = SDL_GetCursor()) == NULL) {
		AG_SetError("SDL_GetCursor() returned NULL");
		return (-1);
	}
	if ((ac = TryMalloc(sizeof(AG_Cursor))) == NULL) {
		return (-1);
	}
	AG_CursorInit(ac);
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0)
	ac->w = (Uint)sc->area.w;
	ac->h = (Uint)sc->area.h;
	ac->xHot = (int)sc->hot_x;
	ac->yHot = (int)sc->hot_y;
#else
    // TODO
#endif
	ac->p = sc;

	TAILQ_INSERT_HEAD(&drv->cursors, ac, cursors);
	drv->nCursors++;
	return (0);
}

/* Change the cursor. */
int
AG_SDL_SetCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;
	
	if (drv->activeCursor == ac)
		return (0);

	SDL_SetCursor((SDL_Cursor *)ac->p);
	drv->activeCursor = ac;
	return (0);
}

/* Revert to the default cursor. */
void
AG_SDL_UnsetCursor(void *obj)
{
	AG_Driver *drv = obj;
	AG_Cursor *ac0 = TAILQ_FIRST(&drv->cursors);
	
	if (drv->activeCursor == ac0)
		return;
	
	SDL_SetCursor((SDL_Cursor *)ac0->p);
	drv->activeCursor = ac0;
}

/* Configure refresh rate. */
int
AG_SDL_SetRefreshRate(void *obj, int fps)
{
	AG_DriverSw *dsw = obj;

	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	dsw->rNom = 1000/fps;
	return (0);
}

/* Create a cursor. */
AG_Cursor *
AG_SDL_CreateCursor(void *obj, Uint w, Uint h, const Uint8 *data,
    const Uint8 *mask, int xHot, int yHot)
{
	AG_Cursor *ac;
	SDL_Cursor *sc;
	Uint size = w*h;
	
	if ((ac = TryMalloc(sizeof(AG_Cursor))) == NULL) {
		return (NULL);
	}
	if ((ac->data = TryMalloc(size)) == NULL) {
		free(ac);
		return (NULL);
	}
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		free(ac);
		return (NULL);
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;

	sc = SDL_CreateCursor(ac->data, ac->mask,
	    ac->w, ac->h,
	    ac->xHot, ac->yHot);
	if (sc == NULL) {
		AG_SetError("SDL_CreateCursor failed");
		goto fail;
	}
	ac->p = (void *)sc;
	return (ac);
fail:
	free(ac->data);
	free(ac->mask);
	free(ac);
	return (NULL);
}

/* Release a cursor. */
void
AG_SDL_FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;

	if (ac == drv->activeCursor)
		drv->activeCursor = NULL;

	SDL_FreeCursor((SDL_Cursor *)(ac->p));
	free(ac->data);
	free(ac->mask);
	free(ac);
}

/* Retrieve cursor visibility status. */
int
AG_SDL_GetCursorVisibility(void *obj)
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

/* Set cursor visibility. */
void
AG_SDL_SetCursorVisibility(void *obj, int flag)
{
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
}

/* Return the desktop display size in pixels. */
int
AG_SDL_GetDisplaySize(Uint *w, Uint *h)
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

/* Apply default display settings where 0 is specified. */
void
AG_SDL_GetPrefDisplaySettings(void *obj, Uint *w, Uint *h, int *depth)
{
	char buf[16];
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	Uint wDisp, hDisp;

	if (*w == 0 || *h == 0) {
		AG_SDL_GetDisplaySize(&wDisp, &hDisp);
	}
	if (*w == 0) {
		if (AG_Defined(drv, "width")) {
			AG_GetString(drv, "width", buf, sizeof(buf));
			*w = (buf[0] != 'a') ? atoi(buf) :
			                       (Uint)((float)wDisp*2.0/3.0);
		} else {
			*w = (Uint)((float)wDisp*2.0/3.0);
		}
	}
	if (*h == 0) {
		if (AG_Defined(drv, "height")) {
			AG_GetString(drv, "height", buf, sizeof(buf));
			*h = (buf[0] != 'a') ? atoi(buf) :
			                       (Uint)((float)hDisp*2.0/3.0);
		} else {
			*h = (Uint)((float)hDisp*2.0/3.0);
		}
	}
	if (*depth == 0) {
		if (AG_Defined(drv, "depth")) {
			AG_GetString(drv, "depth", buf, sizeof(buf));
			*depth = (buf[0] == 'a') ? 32 : atoi(buf);
		} else {
			*depth = 32;
		}
	}
	if (AG_Defined(drv, "fpsMax")) {
		float v;
		char *ep;

		AG_GetString(drv, "fpsMax", buf, sizeof(buf));
		v = (float)strtod(buf, &ep);
		if (*ep == '\0')
			dsw->rNom = (Uint)(1000.0/v);
	}
	if (AG_Defined(drv, "bgColor")) {
		dsw->bgColor = AG_ColorFromString(AG_GetStringP(drv,"bgColor"), NULL);
	}
	if (AG_Defined(drv, "bgPopup"))
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;
}

/* Standard beginEventProcessing() method for SDL drivers. */
void
AG_SDL_BeginEventProcessing(void *obj)
{
	/* Nothing to do */
}

/* Standard pendingEvents() method for SDL drivers. */
int
AG_SDL_PendingEvents(void *obj)
{
	return (SDL_PollEvent(NULL) != 0);
}

/* Translate an SDL_Event to an AG_DriverEvent. */
void
AG_SDL_TranslateEvent(void *obj, const SDL_Event *ev, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		AG_MouseMotionUpdate(drv->mouse, ev->motion.x, ev->motion.y);

		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->win = NULL;
		dev->data.motion.x = ev->motion.x;
		dev->data.motion.y = ev->motion.y;
		break;
	case SDL_MOUSEBUTTONUP:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
		    ev->button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
		    ev->button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_KEYDOWN:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
		    (AG_KeySym)ev->key.keysym.sym,
		    (Uint32)ev->key.keysym.unicode);
	
		dev->type = AG_DRIVER_KEY_DOWN;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev->key.keysym.unicode;
		break;
	case SDL_KEYUP:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
		    (AG_KeySym)ev->key.keysym.sym,
		    (Uint32)ev->key.keysym.unicode);

		dev->type = AG_DRIVER_KEY_UP;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		dev->data.key.ucs = (Uint32)ev->key.keysym.unicode;
		break;
	case SDL_VIDEORESIZE:
		dev->type = AG_DRIVER_VIDEORESIZE;
		dev->win = NULL;
		dev->data.videoresize.x = 0;
		dev->data.videoresize.y = 0;
		dev->data.videoresize.w = (int)ev->resize.w;
		dev->data.videoresize.h = (int)ev->resize.h;
		break;
	case SDL_VIDEOEXPOSE:
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = NULL;
		break;
	case SDL_QUIT:
	case SDL_USEREVENT:
		dev->type = AG_DRIVER_CLOSE;
		dev->win = NULL;
		break;
	default:
		dev->type = AG_DRIVER_UNKNOWN;
		dev->win = NULL;
		break;
	}
}

/* Standard getNextEvent() method for SDL drivers. */
int
AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;
	SDL_Event ev;

	if (SDL_PollEvent(&ev) == 0) {
		return (0);
	}
	AG_SDL_TranslateEvent(drv, &ev, dev);
	return (1);
}

/* Test if the given coordinates overlap a window resize control. */
static __inline__ int
GenericMouseOverCtrl(AG_Window *win, int x, int y)
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

/*
 * Process an input device event.
 * The agDrivers VFS must be locked.
 * TODO: generalize this code to SW drivers.
 */
static int
ProcessInputEvent(AG_Driver *drv, AG_DriverEvent *dev)
{
	AG_DriverSw *dsw = (AG_DriverSw *)drv;
	AG_Window *win, *winTop = NULL;

	if (dev->type == AG_DRIVER_MOUSE_BUTTON_UP) {
		dsw->winop = AG_WINOP_NONE;
		dsw->winSelected = NULL;
	}
	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		AG_ObjectLock(win);

		/* XXX TODO move invisible windows to different tailq! */
		if (!win->visible) {
			AG_ObjectUnlock(win);
			continue;
		}
		switch (dev->type) {
		case AG_DRIVER_MOUSE_MOTION:
			if (dsw->winop != AG_WINOP_NONE) {
				if (dsw->winSelected != win) {
					AG_ObjectUnlock(win);
					continue;
				}
				AG_WM_MouseMotion(dsw, win,
				    drv->mouse->xRel,
				    drv->mouse->yRel);
			}
			AG_ProcessMouseMotion(win,
			    dev->data.motion.x, dev->data.motion.y,
			    drv->mouse->xRel, drv->mouse->yRel,
			    drv->mouse->btnState);
			if (winTop == NULL &&
			    AG_WidgetArea(win, dev->data.motion.x, dev->data.motion.y)) {
				winTop = win;
				AG_MouseCursorUpdate(win,
				    dev->data.motion.x,
				    dev->data.motion.y);
			}
			break;
		case AG_DRIVER_MOUSE_BUTTON_UP:
			AG_ProcessMouseButtonUp(win,
			    dev->data.button.x, dev->data.button.y,
			    dev->data.button.which);
			if (agWindowToFocus != NULL ||
			    !TAILQ_EMPTY(&agWindowDetachQ)) {
				AG_ObjectUnlock(win);
				return (1);
			}
			break;
		case AG_DRIVER_MOUSE_BUTTON_DOWN:
			if (!AG_WidgetArea(win, dev->data.button.x,
			    dev->data.button.y)) {
				AG_ObjectUnlock(win);
				continue;
			}
			if (win != agWindowFocused &&
			    !(win->flags & AG_WINDOW_DENYFOCUS)) {
				agWindowToFocus = win;
			}
			if (win->wBorderBot > 0 &&
			    !(win->flags & AG_WINDOW_NORESIZE)) {
				dsw->winop = GenericMouseOverCtrl(win,
				    dev->data.button.x, dev->data.button.y);
				if (dsw->winop != AG_WINOP_NONE) {
					win->dirty = 1;
					dsw->winSelected = win;
					AG_ObjectUnlock(win);
					return (1);
				}
			}
			AG_ProcessMouseButtonDown(win,
			    dev->data.button.x, dev->data.button.y,
			    dev->data.button.which);
			AG_ObjectUnlock(win);
			return (1);
		case AG_DRIVER_KEY_UP:
			if (dsw->winLastKeydown != NULL &&
			    dsw->winLastKeydown != win) {
				/*
				 * Key was initially pressed while another
				 * window was holding focus, ignore.
				 */
				dsw->winLastKeydown = NULL;
				break;
			}
			AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
			    dev->data.key.ks, dev->data.key.ucs);
			break;
		case AG_DRIVER_KEY_DOWN:
			AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
			    dev->data.key.ks, dev->data.key.ucs);
			break;
		default:
			break;
		}
		AG_ObjectUnlock(win);
	}
	if (dev->type == AG_DRIVER_MOUSE_MOTION &&
	    winTop == NULL) {
		AGDRIVER_CLASS(drv)->unsetCursor(drv);
	}
	return (0);
}

/* Standard processEvent() method for SDL drivers. */
int
AG_SDL_ProcessEvent(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = (AG_Driver *)obj;
	AG_DriverSw *dsw = (AG_DriverSw *)obj;
	int rv = 1;

	AG_LockVFS(&agDrivers);
	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		rv = ProcessInputEvent(drv, dev);
		if (rv == 0 &&
		    (dsw->flags & AG_DRIVER_SW_BGPOPUP) &&
		    (dev->data.button.which == AG_MOUSE_MIDDLE ||
		     dev->data.button.which == AG_MOUSE_RIGHT)) {
			AG_WM_BackgroundPopupMenu(dsw);
			break;
		}
		break;
	case AG_DRIVER_KEY_DOWN:
		if (AG_ExecGlobalKeys(dev->data.key.ks, drv->kbd->modState) == 0) {
			rv = ProcessInputEvent(drv, dev);
		} else {
			rv = 1;
		}
		break;
	case AG_DRIVER_KEY_UP:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_VIDEORESIZE:
		if (AG_ResizeDisplay(dev->data.videoresize.w,
		    dev->data.videoresize.h) == -1) {
			Verbose("ResizeDisplay: %s\n", AG_GetError());
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_Terminate(0);
		break;
	case AG_DRIVER_EXPOSE:
		break;
	default:
		rv = 0;
		break;
	}
	AG_UnlockVFS(&agDrivers);

	return (rv);
}

/*
 * Standard event sink for AG_EventLoop().
 *
 * TODO where AG_SINK_READ capability and pipes are available,
 * could we create a separate thread running SDL_WaitEvent() and
 * sending notifications over a pipe, instead of using a spinner?
 */
int
AG_SDL_EventSink(AG_EventSink *es, AG_Event *event)
{
	AG_DriverEvent dev;
	AG_Driver *drv = AG_PTR(1);
	int rv = 0;

	if (SDL_PollEvent(NULL) != 0) {
		while (AG_SDL_GetNextEvent(drv, &dev) == 1)
			rv = AG_SDL_ProcessEvent(drv, &dev);
	} else {
		AG_Delay(1);
	}
	return (0);
}
int
AG_SDL_EventEpilogue(AG_EventSink *es, AG_Event *event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}
