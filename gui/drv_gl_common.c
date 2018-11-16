/*
 * Copyright (c) 2009-2018 Julien Nadeau Carriere <vedge@hypertriton.com>
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

#if AG_MODEL == AG_LARGE
# define GL_Color3uH(r,g,b)   glColor3us((r),(g),(b))
# define GL_Color4uH(r,g,b,a) glColor4us((r),(g),(b),(a))
#else
# define GL_Color3uH(r,g,b)   glColor3ub((r),(g),(b))
# define GL_Color4uH(r,g,b,a) glColor4ub((r),(g),(b),(a))
#endif

/*
 * Initialize an OpenGL context for Agar GUI rendering.
 *
 * This is usually called initially when creating a window, but AG_DriverSw(3)
 * type drivers may Init or Destroy the context during rendering (i.e., using
 * AG_DRIVER_SW_OVERLAY). Agar must be resilient against GL context loss.
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

/* Push/pop clipping rectangle, setting GL_CLIP_PLANE[0-3]. */
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

static __inline__ GLenum
AG_GL_GetBlendingFunc(AG_AlphaFn fn)
{
	switch (fn) {
	case AG_ALPHA_ONE:		return (GL_ONE);
	case AG_ALPHA_ZERO:		return (GL_ZERO);
	case AG_ALPHA_SRC:		return (GL_SRC_ALPHA);
	case AG_ALPHA_DST:		return (GL_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_DST:	return (GL_ONE_MINUS_DST_ALPHA);
	case AG_ALPHA_ONE_MINUS_SRC:	return (GL_ONE_MINUS_SRC_ALPHA);
	case AG_ALPHA_OVERLAY:		return (GL_ONE);	/* XXX */
	default:			return (GL_ONE);
	}
}

/* Push/pop alpha blending mode. Set GL_BLEND and GL_BLEND_{SRC,DST}. */
void
AG_GL_StdPushBlendingMode(void *obj, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
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
	if (fnSrc == AG_ALPHA_OVERLAY || fnDst == AG_ALPHA_OVERLAY) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glBlendFunc(AG_GL_GetBlendingFunc(fnSrc),
		            AG_GL_GetBlendingFunc(fnDst));
	}
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

/* Delete a texture by name */
void
AG_GL_StdDeleteTexture(void *obj, Uint texture)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;

	gl->textureGC = Realloc(gl->textureGC, (gl->nTextureGC+1)*sizeof(Uint));
	gl->textureGC[gl->nTextureGC++] = texture;
}

/* Delete a display list by name */
void
AG_GL_StdDeleteList(void *obj, Uint list)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;

	gl->listGC = Realloc(gl->listGC, (gl->nListGC+1)*sizeof(Uint));
	gl->listGC[gl->nListGC++] = list;
}

/* Return the corresponding OpenGL type argument for an AG_Surface. */
static __inline__ GLenum
AG_GL_SurfaceType(const AG_Surface *S)
{
#if AG_MODEL == AG_LARGE
	if (S->format.BitsPerPixel == 64) {
		return (GL_UNSIGNED_SHORT);
	} else
#endif
	{
		return (GL_UNSIGNED_BYTE);
	}
}

/*
 * Create a hardware texture from the given AG_Surface.
 *
 * If needed, convert the surface to the required format.
 * Fill in texture coordinates in tc if not NULL.
 * Return new texture ID in rv.
 */
void
AG_GL_StdUploadTexture(void *obj, Uint *rv, AG_Surface *s, AG_TexCoord *tc)
{
	AG_Surface *sGL;
	GLuint texture;

	if (s->flags & AG_SURFACE_GL_TEXTURE) {
		sGL = s;
	} else {
		sGL = AG_SurfaceStdGL(s->w, s->h);
		AG_SurfaceCopy(sGL, s);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)s->w / (float)sGL->w;
		tc->h = (float)s->h / (float)sGL->h;
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
	    sGL->w, sGL->h, 0,
	    GL_RGBA,
	    AG_GL_SurfaceType(sGL),
	    sGL->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!(s->flags & AG_SURFACE_GL_TEXTURE)) {
		AG_SurfaceFree(sGL);
	}
	*rv = texture;
}

/*
 * Replace the contents of an existing texture with those of an AG_Surface.
 *
 * If needed, convert the surface to the required format.
 * Fill in texture coordinates in tc if not NULL.
 * Return 0 on success or -1 on failure.
 */
int
AG_GL_StdUpdateTexture(void *obj, Uint texture, AG_Surface *s, AG_TexCoord *tc)
{
	AG_Surface *sGL;

	if (s->flags & AG_SURFACE_GL_TEXTURE) {
		sGL = s;
	} else {
		sGL = AG_SurfaceStdGL(s->w, s->h);
		AG_SurfaceCopy(sGL, s);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)sGL->w / (float)s->w;
		tc->h = (float)sGL->h / (float)s->h;
	}

	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
	    sGL->w, sGL->h, 0,
	    GL_RGBA,
	    AG_GL_SurfaceType(sGL),
	    sGL->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (!(s->flags & AG_SURFACE_GL_TEXTURE)) {
		AG_SurfaceFree(sGL);
	}
	return (0);
}

/*
 * Perform a software image transfer from an AG_Surface to a textured
 * polygon at widget coordinates x,y.
 */
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
		int w = s->w;
		int h = s->h;

		glTexCoord2f(tc.x, tc.y);  glVertex2i(x,   y);
		glTexCoord2f(tc.w, tc.y);  glVertex2i(x+w, y);
		glTexCoord2f(tc.w, tc.h);  glVertex2i(x+w, y+h);
		glTexCoord2f(tc.x, tc.h);  glVertex2i(x,   y+h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Perform an accelerated image transfer from a mapped surface (by name)
 * to target widget coordinates x,y.
 */
void
AG_GL_BlitSurfaceFrom(void *obj, AG_Widget *wid, int name, const AG_Rect *r,
    int x, int y)
{
	AG_Driver *drv = obj;
	AG_Surface *s = wid->surfaces[name];
	AG_TexCoord tc;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	/* XXX move this call past pushBlendingMode? */
	AG_GL_PrepareTexture(wid, name);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);

	glBindTexture(GL_TEXTURE_2D, wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		int x2 = x + s->w;
		int y2 = y + s->h;

		if (r != NULL) {
			tc.x = (float)r->x/PowOf2i(r->x); /* XXX */
			tc.y = (float)r->y/PowOf2i(r->y);
			tc.w = (float)r->w/PowOf2i(r->w);
			tc.h = (float)r->h/PowOf2i(r->h);
		} else {
			tc = wid->texcoords[name];
		}
		glTexCoord2f(tc.x, tc.y);  glVertex2i(x,  y);
		glTexCoord2f(tc.w, tc.y);  glVertex2i(x2, y);
		glTexCoord2f(tc.w, tc.h);  glVertex2i(x2, y2);
		glTexCoord2f(tc.x, tc.h);  glVertex2i(x,  y2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Perform a software image transfer from an AG_Surface to a textured
 * rectangle of explicit size w,h in GL coordinates.
 */
void
AG_GL_BlitSurfaceGL(void *obj, AG_Widget *wid, AG_Surface *s, float w, float h)
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
		float w_2 = w/2.0f;
		float h_2 = h/2.0f;

		glTexCoord2f(tc.x, tc.y);  glVertex2f( w_2,  h_2);
		glTexCoord2f(tc.w, tc.y);  glVertex2f(-w_2,  h_2);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(-w_2, -h_2);
		glTexCoord2f(tc.x, tc.h);  glVertex2f( w_2, -h_2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Perform an accelerated image transfer from a mapped surface wid:s,
 * to a textured rectangle of explicit size w,h in GL coordinates.
 */
void
AG_GL_BlitSurfaceFromGL(void *obj, AG_Widget *wid, int name, float w, float h)
{
	AG_Driver *drv = obj;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");
	
	/* XXX move this call past pushBlendingMode? */
	AG_GL_PrepareTexture(wid, name);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		AG_TexCoord tc = wid->texcoords[name];
		float w_2 = w/2.0f;
		float h_2 = h/2.0f;

		glTexCoord2f(tc.x, tc.y);  glVertex2f( w_2,  h_2);
		glTexCoord2f(tc.w, tc.y);  glVertex2f(-w_2,  h_2);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(-w_2, -h_2);
		glTexCoord2f(tc.x, tc.h);  glVertex2f( w_2, -h_2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Perform an accelerated image transfer from a mapped surface wid:s,
 * to a textured rectangle of explicit size w,h in (flipped) GL coordinates.
 */
void
AG_GL_BlitSurfaceFlippedGL(void *obj, AG_Widget *wid, int name, float w, float h)
{
	AG_Driver *drv = obj;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");
	
	/* XXX move this call past pushBlendingMode? */
	AG_GL_PrepareTexture(wid, name);

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv,
	    AG_ALPHA_SRC,
	    AG_ALPHA_ONE_MINUS_SRC);
	
	glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		AG_TexCoord tc = wid->texcoords[name];

		glTexCoord2f(tc.w, tc.y);  glVertex2f(0.0f, 0.0f);
		glTexCoord2f(tc.x, tc.y);  glVertex2f(w,    0.0f);
		glTexCoord2f(tc.x, tc.h);  glVertex2f(w,    h);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(0.0f, h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/*
 * Back up every mapped GL texture to a software surface, so that textures
 * can be restored automatically following a possible loss of GL context.
 */
void
AG_GL_BackupSurfaces(void *obj, AG_Widget *wid)
{
	AG_Surface *s;
	GLint w, h;
	Uint i;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nSurfaces; i++)  {
		if (wid->textures[i] == 0 ||
		    wid->surfaces[i] != NULL) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[i]);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		s = AG_SurfaceStdGL(w, h);
		glGetTexImage(GL_TEXTURE_2D, 0,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    s->pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		wid->surfaces[i] = s;
	}
	glDeleteTextures(wid->nSurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nSurfaces*sizeof(Uint));
	AG_ObjectUnlock(wid);
}

/* Re-upload previously saved textures following loss of GL context. */
void
AG_GL_RestoreSurfaces(void *obj, AG_Widget *wid)
{
	Uint i;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nSurfaces; i++)  {
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
 * Render the specified widget to an AG_Surface.
 * XXX TODO render to offscreen buffer instead of display
 * XXX TODO handle 16-bit depth
 */
int
AG_GL_RenderToSurface(void *obj, AG_Widget *wid, AG_Surface **s)
{
	AG_Driver *drv = obj;
	Uint8 *pixels;
	int visiblePrev;
	
	AG_ASSERT_CLASS(obj, "AG_Driver:*");
	AG_ASSERT_CLASS(wid, "AG_Widget:*");

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

/* Put pixel of color c at x,y. */
void
AG_GL_PutPixel(void *obj, int x, int y, AG_Color c)
{
	glBegin(GL_POINTS);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(x, y);
	glEnd();
}

/* Put a 32-bit pixel (in videoFmt) px at x,y. */
void
AG_GL_PutPixel32(void *obj, int x, int y, Uint32 px)
{
	AG_Driver *drv = obj;
	Uint8 r,g,b;

	AG_GetColor32_RGB8(px, drv->videoFmt, &r,&g,&b);
	glBegin(GL_POINTS);
	glColor3ub(r,g,b);
	glVertex2i(x,y);
	glEnd();
}

/* Put a pixel of color r,g,b (as 8-bit components) at x,y. */
void
AG_GL_PutPixelRGB8(void *obj, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	glBegin(GL_POINTS);
	glColor3ub(r,g,b);
	glVertex2i(x,y);
	glEnd();
}

#if AG_MODEL == AG_LARGE

/* Put a 64-bit pixel (in videoFmt) px at x,y. */
void
AG_GL_PutPixel64(void *obj, int x, int y, Uint64 px)
{
	AG_Driver *drv = obj;
	Uint16 r,g,b;

	AG_GetColor64_RGB16(px, drv->videoFmt, &r,&g,&b);
	glBegin(GL_POINTS);
	glColor3us(r,g,b);
	glVertex2i(x,y);
	glEnd();
}

/* Put a pixel of color r,g,b (as 16-bit components) at x,y. */
void
AG_GL_PutPixelRGB16(void *obj, int x, int y, Uint16 r, Uint16 g, Uint16 b)
{
	glBegin(GL_POINTS);
	glColor3us(r,g,b);
	glVertex2i(x,y);
	glEnd();
}

#endif /* AG_LARGE */

/*
 * Blend the pixel at x,y against color c, given the specified source and
 * destination blending factors.
 */
void
AG_GL_BlendPixel(void *obj, int x, int y, AG_Color c, AG_AlphaFn fnSrc,
    AG_AlphaFn fnDst)
{
	AG_Driver *drv = obj;

	/* XXX use own blending routine here to avoid blending moe push/pop? */

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv, fnSrc, fnDst);

	glBegin(GL_POINTS);
	GL_Color4uH(c.r, c.g, c.b, c.a);
	glVertex2i(x,y);
	glEnd();

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/* Draw a solid line from (x1,y1) to (x2,y2) */
void
AG_GL_DrawLine(void *obj, int x1, int y1, int x2, int y2, AG_Color c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

/* Draw a solid horizontal line from (x1,y) to (x2,y). */
void
AG_GL_DrawLineH(void *obj, int x1, int x2, int y, AG_Color c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2s(x1, y);
	glVertex2s(x2, y);
	glEnd();
}

/* Draw a solid vertical line from (x,y1) to (x,y2). */
void
AG_GL_DrawLineV(void *obj, int x, int y1, int y2, AG_Color c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2s(x, y1);
	glVertex2s(x, y2);
	glEnd();
}

/* Draw a alpha-blended line from (x1,y1) to (x2,y2). */
void
AG_GL_DrawLineBlended(void *obj, int x1, int y1, int x2, int y2, AG_Color c,
    AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	if (c.a < AG_ALPHA_OPAQUE)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_LINES);
	GL_Color4uH(c.r, c.g, c.b, c.a);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
	
	if (c.a < AG_ALPHA_OPAQUE)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

void
AG_GL_DrawTriangle(void *_Nonnull obj, AG_Pt v1, AG_Pt v2, AG_Pt v3, AG_Color c)
{
	glBegin(GL_TRIANGLES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(v1.x, v1.y);
	glVertex2i(v2.x, v2.y);
	glVertex2i(v3.x, v3.y);
	glEnd();
}

static void
AG_GL_DrawArrow_Up(int x, int y, int h, AG_Color c)
{
	int h_2 = h >> 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(x - 1,       y - h_2);
	glVertex2i(x - h_2 - 1, y - h_2 + h + 1);
	glVertex2i(x + h_2 - 1, y - h_2 + h + 1);
	glEnd();
}

static void
AG_GL_DrawArrow_Right(int x, int y, int h, AG_Color c)
{
	int h_2 = (h >> 1);
	int x1 = x - h_2 - 1;
	int x2 = x1 + h + 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(x2, y);
	glVertex2i(x1, y-h_2);
	glVertex2i(x1, y+h_2);
	glEnd();
}

static void
AG_GL_DrawArrow_Down(int x, int y, int h, AG_Color c)
{
	int h_2 = (h >> 1);
	int x1 = x - 1;
	int y1 = y - h_2;
	int y2 = y1 + h + 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(x1,     y2);
	glVertex2i(x1+h_2, y1);
	glVertex2i(x1-h_2, y1);
	glEnd();
}

static void
AG_GL_DrawArrow_Left(int x, int y, int h, AG_Color c)
{
	int h_2 = (h >> 1);
	int x1 = x - h_2 - 1;
	int x2 = x1 + h;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(x1, y);
	glVertex2i(x2, y+h_2);
	glVertex2i(x2, y-h_2);
	glEnd();
}

/* Draw an arrow of height h at (x,y), rotated by a given angle. */
void
AG_GL_DrawArrow(void *obj, Uint8 angle, int x0, int y0, int h, AG_Color c)
{
	static void (*pf[])(int,int, int, AG_Color) = {
		AG_GL_DrawArrow_Up,
		AG_GL_DrawArrow_Right,
		AG_GL_DrawArrow_Down,
		AG_GL_DrawArrow_Left,
	};
#ifdef AG_DEBUG
	if (angle >= 4) { AG_FatalError("Bad angle"); }
#endif
	pf[angle](x0,y0, h, c);
}

/* Solid rectangle fill with color c. */
void
AG_GL_FillRect(void *obj, AG_Rect r, AG_Color c)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	GL_Color3uH(c.r, c.g, c.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

/* Solid rectangle fill with color c + dithering. */
void
AG_GL_DrawRectDithered(void *obj, AG_Rect r, AG_Color c)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	int stipplePrev;
	
	stipplePrev = glIsEnabled(GL_POLYGON_STIPPLE);
	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	glPolygonStipple((GLubyte *)gl->dither);
	AG_GL_DrawRectFilled(obj, r, c);
	glPopAttrib();
	if (!stipplePrev) { glDisable(GL_POLYGON_STIPPLE); }
}

/* Draw a box with all rounded corners. */
void
AG_GL_DrawBoxRounded(void *obj, AG_Rect r, int z, int radius,
    AG_Color c1, AG_Color c2, AG_Color c3)
{
	float rad = (float)radius, rad2 = 2.0f*rad;
	float t, i, nFull = 10.0f, nQuart = nFull/4.0f;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r.x + rad,
	             (float)r.y + rad, 0.0f);

	glBegin(GL_POLYGON);
	GL_Color3uH(c1.r, c1.g, c1.b);
	{
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t), -rad*Sin(t));
		}
		for (i = nQuart-1; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad2 + rad*Cos(t/nFull),
			           -rad*Sin(t/nFull));
		}
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad2 + rad*Cos(t),
			           r.h - rad2 + rad*Sin(t));
		}
		for (i = nQuart; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(- rad*Cos(t),
			           r.h - rad2 + rad*Sin(t));
		}
	}
	glEnd();
	
	glBegin(GL_LINE_STRIP);
	{
		GL_Color3uH(c2.r, c2.g, c2.b);
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           -rad*Sin(t));
		}
		t = 2.0f*AG_PI*nQuart;
		glVertex2f((r.w - rad2 + rad*Cos(t/nFull)),
		           -rad*Sin(t/nFull));

		GL_Color3uH(c3.r, c3.g, c3.b);
		for (i = nQuart-1; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad2 + rad*Cos(t), -rad*Sin(t));
		}
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad2 + rad*Cos(t),
			           r.h - rad2 + rad*Sin(t));
		}
		for (i = nQuart; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           r.h - rad2 + rad*Sin(t));
		}
		GL_Color3uH(c2.r, c2.g, c2.b);
		glVertex2f(-rad, 0.0f);
	}
	glEnd();
	glPopMatrix();
}

/* Draw a box with top rounded corners. */
void
AG_GL_DrawBoxRoundedTop(void *obj, AG_Rect r, int z, int radius,
    AG_Color c1, AG_Color c2, AG_Color c3)
{
	float rad = (float)radius;
	float t, i, nFull = 10.0f, nQuart = nFull/4.0f;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r.x + rad,
	             (float)r.y + rad, 0.0f);

	glBegin(GL_POLYGON);
	GL_Color3uH(c1.r, c1.g, c1.b);
	{
		glVertex2f(-rad, (float)r.h - rad);
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t), -rad*Sin(t));
		}
		glVertex2f(0.0f, -rad);

		for (i = nQuart; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad*2.0f + rad*Cos(t),
			           -rad*Sin(t));
		}
		glVertex2f((float)r.w - rad,
		           (float)r.h - rad);
	}
	glEnd();
	
	glBegin(GL_LINE_STRIP);
	{
		GL_Color3uH(c2.r, c2.g, c2.b);
		glVertex2i(-rad, r.h-rad);
		for (i = 0.0f; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-(float)rad*Cos(t), -(float)rad*Sin(t));
		}
		glVertex2f(0.0f, -rad);
		t = (2.0f*AG_PI*nQuart)/nFull;
		glVertex2f(r.w - rad*2 + rad*Cos(t), -rad*Sin(t));

		GL_Color3uH(c3.r, c3.g, c3.b);
		for (i = nQuart-1; i > 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(r.w - rad*2 + rad*Cos(t), -rad*Sin(t));
		}
		glVertex2f((float)r.w - rad,
		           (float)r.h - rad);
	}
	glEnd();
	glPopMatrix();
}

/* Draw (an approximation of) a circle of radius r centered at x,y. */
void
AG_GL_DrawCircle(void *obj, int x, int y, int r, AG_Color c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_LINE_LOOP);
	GL_Color3uH(c.r, c.g, c.b);
	for (i = 0; i < nEdges; i++) {
		float t = (2.0f * AG_PI * i)/nEdges;

		glVertex2f(R * Cos(t),
		           R * Sin(t));
	}
	glEnd();

	glPopMatrix();
}

/* Draw (an approximation of) a filled circle of radius r centered at x,y. */
void
AG_GL_DrawCircleFilled(void *obj, int x, int y, int r, AG_Color c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_POLYGON);
	GL_Color3uH(c.r, c.g, c.b);
	for (i = 0; i < nEdges; i++) {
		float t = (2.0f * AG_PI * i)/nEdges;

		glVertex2f(R * Cos(t),
		           R * Sin(t));
	}
	glEnd();

	glPopMatrix();
}

/* Variant of AG_GL_DrawCircle() */
void
AG_GL_DrawCircle2(void *obj, int x, int y, int r, AG_Color c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_LINE_LOOP);
	GL_Color3uH(c.r, c.g, c.b);
	for (i = 0; i < nEdges; i++) {
		float t = (2.0f * AG_PI * i)/nEdges;

		glVertex2f(R * Cos(t),
		           R * Sin(t));
		glVertex2f((R + 1.0f)*Cos(t),
		           (R + 1.0f)*Sin(t));
	}
	glEnd();

	glPopMatrix();
}

/* Draw a rectangle r filled with solid color c (ignore any alpha) */
void
AG_GL_DrawRectFilled(void *obj, AG_Rect r, AG_Color c)
{
	int x2 = r.x + r.w - 1;
	int y2 = r.y + r.h - 1;

	glBegin(GL_POLYGON);
	glColor3us(c.r, c.g, c.b);
	glVertex2i(r.x, r.y);
	glVertex2i(x2, r.y);
	glVertex2i(x2, y2);
	glVertex2i(r.x, y2);
	glEnd();
}

/* Draw a rectangle r filled with color c (blend according to c's alpha). */
void
AG_GL_DrawRectBlended(void *obj, AG_Rect r, AG_Color c, AG_AlphaFn fnSrc,
    AG_AlphaFn fnDst)
{
	int x1 = r.x;
	int y1 = r.y;
	int x2 = x1+r.w;
	int y2 = y1+r.h;

	if (c.a < AG_ALPHA_OPAQUE)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_POLYGON);
	glColor4us(c.r, c.g, c.b, c.a);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
	
	if (c.a < AG_ALPHA_OPAQUE)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

/* Prepare for rendering an AG_Text(3) glyph. */
void
AG_GL_UpdateGlyph(void *obj, AG_Glyph *gl)
{
	AG_GL_UploadTexture(obj, &gl->texture, gl->su, &gl->texcoords);
}

/* Render an AG_Text(3) glyph at x,y. */
void
AG_GL_DrawGlyph(void *obj, const AG_Glyph *gl, int x, int y)
{
	AG_Surface *s = gl->su;

	glBindTexture(GL_TEXTURE_2D, gl->texture);
	glBegin(GL_POLYGON);
	{
		const AG_TexCoord tc = gl->texcoords;
		int w = s->w;
		int h = s->h;

		glTexCoord2f(tc.x, tc.y);  glVertex2i(x,   y);
		glTexCoord2f(tc.w, tc.y);  glVertex2i(x+w, y);
		glTexCoord2f(tc.w, tc.h);  glVertex2i(x+w, y+h);
		glTexCoord2f(tc.x, tc.h);  glVertex2i(x,   y+h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}
