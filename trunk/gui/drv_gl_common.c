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
 * Routines common to all OpenGL drivers.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "window.h"
#include "gui_math.h"
#include "text.h"
#include "packedpixel.h"
#include "opengl.h"

/*
 * Initialize GL for rendering of Agar GUI elements. The vp argument
 * specifies the viewport in view coordinates.
 */
void
AG_GL_InitContext(AG_Rect vp)
{
	glViewport(vp.x, vp.y, vp.w, vp.h);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, vp.w, vp.h, 0, -1.0, 1.0);

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
}

/* Generic FillRect() operation for GL drivers. */
void
AG_GL_FillRect(void *obj, AG_Rect r, AG_Color c)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	glColor3ub(c.r, c.g, c.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

/*
 * Surface operations (rendering context)
 */

/* Generic UploadTexture() for GL drivers. */
void
AG_GL_UploadTexture(Uint *rv, AG_Surface *suSrc, AG_TexCoord *tc)
{
	AG_Surface *suTex;
	GLuint texture;
	int w, h;

	/* Convert to the GL_RGBA/GL_UNSIGNED_BYTE format. */
	w = PowOf2i(suSrc->w);
	h = PowOf2i(suSrc->h);
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)suSrc->w / w;
		tc->h = (float)suSrc->h / h;
	}
	suTex = AG_SurfaceRGBA(w,h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	    );
	if (suTex == NULL) {
		AG_FatalError(NULL);
	}
	AG_SurfaceCopy(suTex, suSrc);
	
	/* Upload as an OpenGL texture. */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
#if 0
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, suTex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(suTex);
	*rv = texture;
}

/* Generic UpdateTexture() for GL drivers. */
int
AG_GL_UpdateTexture(Uint texture, AG_Surface *su, AG_TexCoord *tc)
{
	AG_Surface *suTex;
	int w, h;

	/*
	 * Convert to the GL_RGBA/GL_UNSIGNED_BYTE format and adjust for
	 * power-of-two dimensions.
	 * TODO check for GL_ARB_texture_non_power_of_two.
	 */
	w = PowOf2i(su->w);
	h = PowOf2i(su->h);
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)su->w / w;
		tc->h = (float)su->h / h;
	}
	suTex = AG_SurfaceRGBA(w,h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	);
	if (suTex == NULL) {
		return (-1);
	}
	AG_SurfaceCopy(suTex, su);

	/* Upload as an OpenGL texture. */
	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, suTex->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	AG_SurfaceFree(suTex);
	return (0);
}


/* Prepare a widget-bound texture for rendering. */
void
AG_GL_PrepareTexture(void *obj, int s)
{
	AG_Widget *wid = obj;

	if (wid->textures[s] == 0) {
		wid->textures[s] = AG_SurfaceTexture(wid->surfaces[s],
		    &wid->texcoords[s]);
	} else if (wid->surfaceFlags[s] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[s] &= ~(AG_WIDGET_SURFACE_REGEN);
		AG_GL_UpdateTexture(wid->textures[s], wid->surfaces[s],
		    &wid->texcoords[s]);
	}
}

/* Delete a GL texture (thread-safely) */
void
AG_GL_DeleteTexture(void *obj, Uint name)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	dc->deleteTexture(drv, name);
}

/* Generic emulated BlitSurface() for GL drivers. */
/* XXX inefficient */
void
AG_GL_BlitSurface(void *obj, AG_Widget *wid, AG_Surface *s, int x, int y)
{
	AG_Driver *drv = obj;
	GLuint texture;
	AG_TexCoord tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	texture = AG_SurfaceTexture(s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2i(x,      y);
		glTexCoord2f(tc.w, tc.y);	glVertex2i(x+s->w, y);
		glTexCoord2f(tc.x, tc.h);	glVertex2i(x,      y+s->h);
		glTexCoord2f(tc.w, tc.h);	glVertex2i(x+s->w, y+s->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Generic BlitSurfaceFrom() for GL drivers. We simply render a rectangle
 * bound to the corresponding texture.
 */
void
AG_GL_BlitSurfaceFrom(void *obj, AG_Widget *wid, AG_Widget *widSrc, int s,
    AG_Rect *r, int x, int y)
{
	AG_Driver *drv = obj;
	AG_Surface *su = widSrc->surfaces[s];
	AG_TexCoord tcTmp, *tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");
	AG_ASSERT_CLASS(widSrc, "AG_Widget:*");

	AG_GL_PrepareTexture(widSrc, s);

	if (r == NULL) {
		tc = &widSrc->texcoords[s];
	} else {
		tc = &tcTmp;
		tcTmp.x = (float)r->x/PowOf2i(r->x);
		tcTmp.y = (float)r->y/PowOf2i(r->y);
		tcTmp.w = (float)r->w/PowOf2i(r->w);
		tcTmp.h = (float)r->h/PowOf2i(r->h);
	}

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, widSrc->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/* Generic BlitSurfaceGL() operation for GL drivers. */
void
AG_GL_BlitSurfaceGL(void *obj, AG_Widget *wid, AG_Surface *s, float w, float h)
{
	AG_Driver *drv = obj;
	GLuint texture;
	AG_TexCoord tc;
	float w2 = w/2.0f;
	float h2 = h/2.0f;

	texture = AG_SurfaceTexture(s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc.w, tc.y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc.x, tc.h);	glVertex2f( w2, -h2);
		glTexCoord2f(tc.w, tc.h);	glVertex2f(-w2, -h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/* Generic BlitSurfaceFromGL() operation for GL drivers. */
void
AG_GL_BlitSurfaceFromGL(void *obj, AG_Widget *wid, int s, float w, float h)
{
	AG_Driver *drv = obj;
	AG_TexCoord *tc;
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	
	AG_GL_PrepareTexture(wid, s);
	tc = &wid->texcoords[s];

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, wid->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc->w, tc->y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc->x, tc->h);	glVertex2f( w2, -h2);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(-w2, -h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/* Generic BlitSurfaceFlippedGL() operation for GL drivers. */
void
AG_GL_BlitSurfaceFlippedGL(void *obj, AG_Widget *wid, int s, float w, float h)
{
	AG_Driver *drv = obj;
	AG_TexCoord *tc;
	
	AG_GL_PrepareTexture(wid, s);
	tc = &wid->texcoords[s];

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[s]);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->w, tc->y);	glVertex2f(0.0, 0.0);
		glTexCoord2f(tc->x, tc->y);	glVertex2f(w,   0.0);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(0.0, h);
		glTexCoord2f(tc->x, tc->h);	glVertex2f(w,   h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Generic BackupSurfaces() operation for GL drivers. We simply copy the
 * textures back to software surfaces, where necessary, using glGetTexImage().
 */
void
AG_GL_BackupSurfaces(void *obj, AG_Widget *wid)
{
	AG_Surface *su;
	GLint w, h;
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->textures[i] == 0 ||
		    wid->surfaces[i] != NULL) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[i]);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
		    &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,
		    &h);

		su = AG_SurfaceRGBA(w, h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
			0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
		);
		if (su == NULL) {
			AG_FatalError("Allocating texture: %s", AG_GetError());
		}
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		    su->pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		wid->surfaces[i] = su;
	}
	glDeleteTextures(wid->nsurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nsurfaces*sizeof(Uint));
	AG_ObjectUnlock(wid);
}

/*
 * Generic RestoreSurfaces() operation for GL drivers. We recreate
 * corresponding textures for all surfaces.
 */
void
AG_GL_RestoreSurfaces(void *obj, AG_Widget *wid)
{
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->surfaces[i] != NULL) {
			wid->textures[i] = AG_SurfaceTexture(wid->surfaces[i],
			    &wid->texcoords[i]);
		} else {
			wid->textures[i] = 0;
		}
	}
	AG_ObjectUnlock(wid);
}

/*
 * Generic RenderToSurface() operation for GL drivers.
 * XXX XXX TODO render to offscreen buffer instead of display!
 */
int
AG_GL_RenderToSurface(void *obj, AG_Widget *wid, AG_Surface **s)
{
	AG_Driver *drv = obj;
	Uint8 *pixels;
	int visiblePrev;

	AG_BeginRendering(drv);
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering(drv);

	if ((pixels = AG_TryMalloc(wid->w*wid->h*4)) == NULL) {
		return (-1);
	}
	if (AGDRIVER_MULTIPLE(drv)) {
		glReadPixels(
		    wid->rView.x1,
		    HEIGHT(AGDRIVER_MW(drv)->win) - wid->rView.y2,
		    wid->w,
		    wid->h,
		    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	} else {
		glReadPixels(
		    wid->rView.x1,
		    agDriverSw->h - wid->rView.y2,
		    wid->w,
		    wid->h,
		    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}
	AG_PackedPixelFlip(pixels, wid->h, wid->w*4);
	*s = AG_SurfaceFromPixelsRGBA(pixels, wid->w, wid->h, 32,
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0);
	if (*s == NULL) {
		Free(pixels);
		return (-1);
	}
	return (0);
}

/*
 * Generic GL rendering operations (rendering context)
 */

void
AG_GL_PutPixel(void *obj, int x, int y, AG_Color C)
{
	glBegin(GL_POINTS);
	glColor3ub(C.r, C.g, C.b);
	glVertex2i(x, y);
	glEnd();
}

void
AG_GL_PutPixel32(void *obj, int x, int y, Uint32 c)
{
	AG_Driver *drv = obj;
	Uint8 r, g, b;

	AG_GetPixelRGB(c, drv->videoFmt, &r,&g,&b);
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2i(x, y);
	glEnd();
}

void
AG_GL_PutPixelRGB(void *obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2i(x, y);
	glEnd();
}

void
AG_GL_BlendPixel(void *obj, int x, int y, AG_Color C, AG_BlendFn fnSrc,
    AG_BlendFn fnDst)
{
	AG_Driver *drv = obj;

	/* XXX inefficient */

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv, fnSrc, fnDst);
	glBegin(GL_POINTS);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2i(x, y);
	glEnd();
	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

void
AG_GL_DrawLine(void *obj, int x1, int y1, int x2, int y2, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

void
AG_GL_DrawLineH(void *obj, int x1, int x2, int y, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x1, y);
	glVertex2s(x2, y);
	glEnd();
}

void
AG_GL_DrawLineV(void *obj, int x, int y1, int y2, AG_Color C)
{
	glBegin(GL_LINES);
	glColor3ub(C.r, C.g, C.b);
	glVertex2s(x, y1);
	glVertex2s(x, y2);
	glEnd();
}

void
AG_GL_DrawLineBlended(void *obj, int x1, int y1, int x2, int y2, AG_Color C,
    AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	if (C.a < 255)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_LINES);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
	
	if (C.a < 255)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

void
AG_GL_DrawArrowUp(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x,		y - h2);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x + h2,	y + h2);
	}
	glEnd();
}

void
AG_GL_DrawArrowDown(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x - h2,	y - h2);
		glVertex2i(x + h2,	y - h2);
		glVertex2i(x,		y + h2);
	}
	glEnd();
}

void
AG_GL_DrawArrowLeft(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x - h2,	y);
		glVertex2i(x + h2,	y + h2);
		glVertex2i(x + h2,	y - h2);
	}
	glEnd();
}

void
AG_GL_DrawArrowRight(void *obj, int x, int y, int h, AG_Color C[2])
{
	int h2 = h>>1;

	/* XXX c2 */
	glBegin(GL_POLYGON);
	{
		glColor3ub(C[0].r, C[0].g, C[0].b);
		glVertex2i(x + h2,	y);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x - h2,	y - h2);
	}
	glEnd();
}

void
AG_GL_DrawRectDithered(void *obj, AG_Rect r, AG_Color C)
{
	AG_Driver *drv = obj;
	int stipplePrev;
	
	stipplePrev = glIsEnabled(GL_POLYGON_STIPPLE);
	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	glPolygonStipple((GLubyte *)drv->glStipple);
	AG_GL_DrawRectFilled(obj, r, C);
	glPopAttrib();
	if (!stipplePrev) { glDisable(GL_POLYGON_STIPPLE); }
}

void
AG_GL_DrawBoxRounded(void *obj, AG_Rect r, int z, int radius, AG_Color C[3])
{
	float rad = (float)radius;
	float i, nFull = 10.0, nQuart = nFull/4.0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r.x + rad,
	             (float)r.y + rad,
		     0.0);

	glBegin(GL_POLYGON);
	glColor3ub(C[0].r, C[0].g, C[0].b);
	{
		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(-rad*Cos((2.0*AG_PI*i)/nFull),
			           -rad*Sin((2.0*AG_PI*i)/nFull));
		}
		for (i = nQuart-1; i > 0.0; i--) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			                       - rad*Sin((2.0*AG_PI*i)/nFull));
		}
		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			           r.h - rad*2 + rad*Sin((2.0*AG_PI*i)/nFull));
		}
		for (i = nQuart; i > 0.0; i--) {
			glVertex2f(            - rad*Cos((2.0*AG_PI*i)/nFull),
			           r.h - rad*2 + rad*Sin((2.0*AG_PI*i)/nFull));
		}
	}
	glEnd();
	
	glBegin(GL_LINE_STRIP);
	{
		glColor3ub(C[1].r, C[1].g, C[1].b);
		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(-rad*Cos((2.0*AG_PI*i)/nFull),
			           -rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*nQuart)/nFull),
		                       - rad*Sin((2.0*AG_PI*nQuart)/nFull));

		glColor3ub(C[2].r, C[2].g, C[2].b);
		for (i = nQuart-1; i > 0.0; i--) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			                       - rad*Sin((2.0*AG_PI*i)/nFull));
		}
		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			           r.h - rad*2 + rad*Sin((2.0*AG_PI*i)/nFull));
		}
		for (i = nQuart; i > 0.0; i--) {
			glVertex2f(            - rad*Cos((2.0*AG_PI*i)/nFull),
			           r.h - rad*2 + rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glColor3ub(C[1].r, C[1].g, C[1].b);
		glVertex2f(-rad, 0.0);
	}
	glEnd();

	glPopMatrix();
}

void
AG_GL_DrawBoxRoundedTop(void *obj, AG_Rect r, int z, int radius, AG_Color C[3])
{
	float rad = (float)radius;
	float i, nFull = 10.0, nQuart = nFull/4.0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r.x + rad,
	             (float)r.y + rad,
		     0.0);

	glBegin(GL_POLYGON);
	glColor3ub(C[0].r, C[0].g, C[0].b);
	{
		glVertex2f(-rad, (float)r.h - rad);

		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(-rad*Cos((2.0*AG_PI*i)/nFull),
			           -rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glVertex2f(0.0, -rad);

		for (i = nQuart; i > 0.0; i--) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			                       - rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glVertex2f((float)r.w - rad,
		           (float)r.h - rad);
	}
	glEnd();
	
	glBegin(GL_LINE_STRIP);
	{
		glColor3ub(C[1].r, C[1].g, C[1].b);
		glVertex2i(-rad, r.h-rad);
		for (i = 0.0; i < nQuart; i++) {
			glVertex2f(-(float)rad*Cos((2.0*AG_PI*i)/nFull),
			           -(float)rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glVertex2f(0.0, -rad);
		glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*nQuart)/nFull),
		                       - rad*Sin((2.0*AG_PI*nQuart)/nFull));

		glColor3ub(C[2].r, C[2].g, C[2].b);
		for (i = nQuart-1; i > 0.0; i--) {
			glVertex2f(r.w - rad*2 + rad*Cos((2.0*AG_PI*i)/nFull),
			                       - rad*Sin((2.0*AG_PI*i)/nFull));
		}
		glVertex2f((float)r.w - rad,
		           (float)r.h - rad);
	}
	glEnd();

	glPopMatrix();
}

void
AG_GL_DrawCircle(void *obj, int x, int y, int r, AG_Color C)
{
	float i, nEdges = r*2;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0);
	
	glBegin(GL_LINE_LOOP);
	glColor3ub(C.r, C.g, C.b);
	for (i = 0; i < nEdges; i++) {
		glVertex2f((float)r*Cos((2.0*AG_PI*i)/nEdges),
		           (float)r*Sin((2.0*AG_PI*i)/nEdges));
	}
	glEnd();

	glPopMatrix();
}

void
AG_GL_DrawCircle2(void *obj, int x, int y, int r, AG_Color C)
{
	float i, nEdges = r*2;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0);
	
	glBegin(GL_LINE_LOOP);
	glColor3ub(C.r, C.g, C.b);
	for (i = 0; i < nEdges; i++) {
		glVertex2f((float)r*Cos((2.0*AG_PI*i)/nEdges),
		           (float)r*Sin((2.0*AG_PI*i)/nEdges));
		glVertex2f(((float)r + 1.0)*Cos((2.0*AG_PI*i)/nEdges),
		           ((float)r + 1.0)*Sin((2.0*AG_PI*i)/nEdges));
	}
	glEnd();

	glPopMatrix();
}

void
AG_GL_DrawRectFilled(void *obj, AG_Rect r, AG_Color C)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	glColor3ub(C.r, C.g, C.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

void
AG_GL_DrawRectBlended(void *obj, AG_Rect r, AG_Color C, AG_BlendFn fnSrc,
    AG_BlendFn fnDst)
{
	int x1 = r.x;
	int y1 = r.y;
	int x2 = x1+r.w;
	int y2 = y1+r.h;

	if (C.a < 255)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_POLYGON);
	glColor4ub(C.r, C.g, C.b, C.a);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
	
	if (C.a < 255)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

void
AG_GL_UpdateGlyph(void *obj, AG_Glyph *gl)
{
	AG_GL_UploadTexture(&gl->texture, gl->su, &gl->texcoords);
}

void
AG_GL_DrawGlyph(void *obj, const AG_Glyph *gl, int x, int y)
{
	AG_Surface *su = gl->su;
	const AG_TexCoord *tc = &gl->texcoords;

	glBindTexture(GL_TEXTURE_2D, gl->texture);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Delete a GL display list (thread-safely) */
void
AG_GL_DeleteList(void *obj, Uint name)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	dc->deleteList(drv, name);
}

#endif /* HAVE_OPENGL */
