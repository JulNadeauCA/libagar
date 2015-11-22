/*
 * Copyright (c) 2009-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/text.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/opengl.h>

/*
 * Initialize an OpenGL context for Agar GUI rendering.
 *
 * This routine is usually invoked once when initially creating a window,
 * but single-window drivers such as "sdlgl" may Init/Destroy the context
 * during rendering (i.e., to implement the AG_DRIVER_SW_OVERLAY mode).
 */
int
AG_GL_InitContext(void *obj, AG_GL_Context *gl)
{
	AG_Driver *drv = obj;
	AG_ClipRect *cr;
	
	gl->textureGC = NULL;
	gl->nTextureGC = 0;
	gl->listGC = NULL;
	gl->nListGC = 0;
	memset(&gl->bs, sizeof(AG_GL_BlendState), sizeof(AG_GL_BlendState));
	memset(gl->dither, 0xaa, sizeof(gl->dither));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);

	/* Initialize our clipping rectangles. */
	memset(gl->clipStates, 0, sizeof(gl->clipStates));
	if ((gl->clipRects = TryMalloc(sizeof(AG_ClipRect))) == NULL) {
		return (-1);
	}
	gl->nClipRects = 1;

	/* Initialize the first clipping rectangle. */
	cr = &gl->clipRects[0];
	cr->r = AG_RECT(0,0,0,0);
	cr->eqns[0][0] = 1.0;	cr->eqns[0][1] = 0.0;
	cr->eqns[0][2] = 0.0;	cr->eqns[0][3] = 0.0;
	cr->eqns[1][0] = 0.0;	cr->eqns[1][1] = 1.0;
	cr->eqns[1][2] = 0.0;	cr->eqns[1][3] = 0.0;
	cr->eqns[2][0] = -1.0;	cr->eqns[2][1] = 0.0;
	cr->eqns[2][2] = 0.0;	cr->eqns[2][3] = 0.0; /* w */
	cr->eqns[3][0] = 0.0;	cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] = 0.0;	cr->eqns[3][3] = 0.0; /* h */
	
	drv->gl = gl;
	return (0);
}

/*
 * Size or resize an OpenGL context to specified dimensions.
 */
void
AG_GL_SetViewport(AG_GL_Context *gl, AG_Rect vp)
{
	AG_ClipRect *cr = &gl->clipRects[0];

	/* Set up the view port and projection */
	glViewport(vp.x, vp.y, vp.w, vp.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, vp.w, vp.h, 0, -1.0, 1.0);

	/* Update clipping rectangle 0. */
	cr->r.w = vp.w;
	cr->r.h = vp.h;
	cr->eqns[2][3] = (double)vp.w;
	cr->eqns[3][3] = (double)vp.h;
}

/* Destroy an OpenGL rendering context. */
void
AG_GL_DestroyContext(void *obj)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_Glyph *glyph;
	int i;

	if (gl == NULL)
		return;

	/* Restore the previous clipping rectangle state. */
	if (gl->clipStates[0])	{ glEnable(GL_CLIP_PLANE0); }
	else			{ glDisable(GL_CLIP_PLANE0); }
	if (gl->clipStates[1])	{ glEnable(GL_CLIP_PLANE1); }
	else			{ glDisable(GL_CLIP_PLANE1); }
	if (gl->clipStates[2])	{ glEnable(GL_CLIP_PLANE2); }
	else			{ glDisable(GL_CLIP_PLANE2); }
	if (gl->clipStates[3])	{ glEnable(GL_CLIP_PLANE3); }
	else			{ glDisable(GL_CLIP_PLANE3); }
	
	/* Invalidate any cached glyph renderings. */
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_FOREACH(glyph, &drv->glyphCache[i].glyphs, glyphs) {
			if (glyph->texture != 0) {
				glDeleteTextures(1, (GLuint *)&glyph->texture);
				glyph->texture = 0;
			}
		}
	}
	
	/* Destroy any texture or display list queued for deletion. */
	glDeleteTextures(gl->nTextureGC, gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;
	Free(gl->textureGC);
	Free(gl->listGC);

	free(gl->clipRects);
	gl->clipRects = NULL;
	gl->nClipRects = 0;

	drv->gl = NULL;
}

/*
 * Standard clipping rectangle operations.
 */
void
AG_GL_StdPushClipRect(void *obj, AG_Rect r)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_ClipRect *cr, *crPrev;

	gl->clipRects = Realloc(gl->clipRects, (gl->nClipRects+1)*
	                                         sizeof(AG_ClipRect));
	crPrev = &gl->clipRects[gl->nClipRects-1];
	cr = &gl->clipRects[gl->nClipRects++];

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
void
AG_GL_StdPopClipRect(void *obj)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_ClipRect *cr;
	
#ifdef AG_DEBUG
	if (gl->nClipRects < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &gl->clipRects[gl->nClipRects-2];
	gl->nClipRects--;

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}

/*
 * Standard blending control operations.
 */
void
AG_GL_StdPushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	
	/* XXX TODO: stack */
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &gl->bs[0].texEnvMode);
	glGetBooleanv(GL_BLEND, &gl->bs[0].enabled);
	glGetIntegerv(GL_BLEND_SRC, &gl->bs[0].srcFactor);
	glGetIntegerv(GL_BLEND_DST, &gl->bs[0].dstFactor);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glBlendFunc(AG_GL_GetBlendingFunc(fnSrc), AG_GL_GetBlendingFunc(fnDst));
}
void
AG_GL_StdPopBlendingMode(void *obj)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	
	/* XXX TODO: stack */
	if (gl->bs[0].enabled) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
	glBlendFunc(gl->bs[0].srcFactor, gl->bs[0].dstFactor);
}

/*
 * Standard texture management operations.
 */
void
AG_GL_StdDeleteTexture(void *obj, Uint texture)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;

	gl->textureGC = Realloc(gl->textureGC, (gl->nTextureGC+1)*sizeof(Uint));
	gl->textureGC[gl->nTextureGC++] = texture;
}
void
AG_GL_StdDeleteList(void *obj, Uint list)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;

	gl->listGC = Realloc(gl->listGC, (gl->nListGC+1)*sizeof(Uint));
	gl->listGC[gl->nListGC++] = list;
}

void
AG_GL_StdUploadTexture(void *obj, Uint *rv, AG_Surface *su, AG_TexCoord *tc)
{
	AG_Surface *gsu;
	GLuint texture;

	if (su->flags & AG_SURFACE_GLTEXTURE) {
		gsu = su;
	} else {
		if ((gsu = AG_SurfaceStdGL(su->w, su->h)) == NULL) {
			AG_FatalError(NULL);
		}
		AG_SurfaceCopy(gsu, su);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)su->w / (float)gsu->w;
		tc->h = (float)su->h / (float)gsu->h;
	}
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	    gsu->w, gsu->h, 0,
	    GL_RGBA,
	    GL_UNSIGNED_BYTE,
	    gsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!(su->flags & AG_SURFACE_GLTEXTURE)) {
		AG_SurfaceFree(gsu);
	}
	*rv = texture;
}

int
AG_GL_StdUpdateTexture(void *obj, Uint texture, AG_Surface *su, AG_TexCoord *tc)
{
	AG_Surface *gsu;

	if (su->flags & AG_SURFACE_GLTEXTURE) {
		gsu = su;
	} else {
		if ((gsu = AG_SurfaceStdGL(su->w, su->h)) == NULL) {
			return (-1);
		}
		AG_SurfaceCopy(gsu, su);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)gsu->w / (float)su->w;
		tc->h = (float)gsu->h / (float)su->h;
	}

	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	    gsu->w, gsu->h, 0,
	    GL_RGBA,
	    GL_UNSIGNED_BYTE,
	    gsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!(su->flags & AG_SURFACE_GLTEXTURE)) {
		AG_SurfaceFree(gsu);
	}
	return (0);
}

/* Prepare a widget-bound texture for rendering. */
void
AG_GL_PrepareTexture(void *obj, int s)
{
	AG_Widget *wid = obj;
	AG_Driver *drv = wid->drv;

	if (wid->textures[s] == 0) {
		AG_GL_UploadTexture(drv, &wid->textures[s], wid->surfaces[s],
		    &wid->texcoords[s]);
	} else if (wid->surfaceFlags[s] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[s] &= ~(AG_WIDGET_SURFACE_REGEN);
		AG_GL_UpdateTexture(drv, wid->textures[s],
		    wid->surfaces[s], &wid->texcoords[s]);
	}
}

/* Generic emulated BlitSurface() for GL drivers. */
void
AG_GL_BlitSurface(void *obj, AG_Widget *wid, AG_Surface *s, int x, int y)
{
	AG_Driver *drv = obj;
	GLuint texture;
	AG_TexCoord tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	AG_GL_UploadTexture(drv, &texture, s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2i(x,      y);
		glTexCoord2f(tc.w, tc.y);	glVertex2i(x+s->w, y);
		glTexCoord2f(tc.w, tc.h);	glVertex2i(x+s->w, y+s->h);
		glTexCoord2f(tc.x, tc.h);	glVertex2i(x,      y+s->h);
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
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
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

	AG_GL_UploadTexture(drv, &texture, s, &tc);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc.x, tc.y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc.w, tc.y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc.w, tc.h);	glVertex2f(-w2, -h2);
		glTexCoord2f(tc.x, tc.h);	glVertex2f( w2, -h2);
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
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2f( w2,  h2);
		glTexCoord2f(tc->w, tc->y);	glVertex2f(-w2,  h2);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(-w2, -h2);
		glTexCoord2f(tc->x, tc->h);	glVertex2f( w2, -h2);
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
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc->w, tc->y);	glVertex2f(0.0, 0.0);
		glTexCoord2f(tc->x, tc->y);	glVertex2f(w,   0.0);
		glTexCoord2f(tc->x, tc->h);	glVertex2f(w,   h);
		glTexCoord2f(tc->w, tc->h);	glVertex2f(0.0, h);
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

		if ((su = AG_SurfaceStdGL(w, h)) == NULL) {
			AG_FatalError("Backup texture: %s",
			    AG_GetError());
		}
		glGetTexImage(GL_TEXTURE_2D, 0,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
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
			AG_GL_UploadTexture(wid->drv, &wid->textures[i],
			    wid->surfaces[i], &wid->texcoords[i]);
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
	if (AG_PackedPixelFlip(pixels, wid->h, wid->w*4) == -1) {
		goto fail;
	}
	*s = AG_SurfaceFromPixelsRGBA(pixels,
	    wid->w, wid->h,
	    32,
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0);
	if (*s == NULL) {
		goto fail;
	}
	return (0);
fail:
	Free(pixels);
	return (-1);
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

void
AG_GL_DrawRectDithered(void *obj, AG_Rect r, AG_Color C)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	int stipplePrev;
	
	stipplePrev = glIsEnabled(GL_POLYGON_STIPPLE);
	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	glPolygonStipple((GLubyte *)gl->dither);
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
AG_GL_DrawCircleFilled(void *obj, int x, int y, int r, AG_Color C)
{
	float i, nEdges = r*2;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0);
	
	glBegin(GL_POLYGON);
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
	AG_GL_UploadTexture(obj, &gl->texture, gl->su, &gl->texcoords);
}

void
AG_GL_DrawGlyph(void *obj, const AG_Glyph *gl, int x, int y)
{
	AG_Surface *su = gl->su;
	const AG_TexCoord *tc = &gl->texcoords;

	glBindTexture(GL_TEXTURE_2D, gl->texture);
	glBegin(GL_POLYGON);
	{
		glTexCoord2f(tc->x, tc->y);	glVertex2i(x,       y);
		glTexCoord2f(tc->w, tc->y);	glVertex2i(x+su->w, y);
		glTexCoord2f(tc->w, tc->h);	glVertex2i(x+su->w, y+su->h);
		glTexCoord2f(tc->x, tc->h);	glVertex2i(x,       y+su->h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
