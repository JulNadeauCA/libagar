/*
 * Copyright (c) 2009-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/primitive.h>

#if AG_MODEL == AG_LARGE
# define GL_Color3uH(r,g,b)   glColor3us((r),(g),(b))
# define GL_Color4uH(r,g,b,a) glColor4us((r),(g),(b),(a))
#else
# define GL_Color3uH(r,g,b)   glColor3ub((r),(g),(b))
# define GL_Color4uH(r,g,b,a) glColor4ub((r),(g),(b),(a))
#endif

/* Expensive debugging of GL context & resource management. */
/* #define DEBUG_GL */

#if defined(AG_DEBUG) && defined(HAVE_GLEXT) && defined(GL_DEBUG_OUTPUT)
static void
AG_GL_DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *message, const void *userParam)
{
	Verbose("%s: (type %x, severity %x)  %s\n",
	    (type == GL_DEBUG_TYPE_ERROR ? "GL(ERROR)": "GL"),
	    type, severity, message);
}
#endif /* AG_DEBUG && HAVE_GLEXT && GL_DEBUG_OUTPUT */

/*
 * Initialize an OpenGL context for Agar GUI rendering.
 *
 * This is usually called initially when creating a window, but AG_DriverSw(3)
 * type drivers may Init or Destroy the context during rendering (i.e., using
 * AG_DRIVER_SW_OVERLAY). Agar must be resilient against GL context loss.
 */
void
AG_GL_InitContext(void *obj, AG_GL_Context *gl)
{
	AG_Driver *drv = obj;
	AG_ClipRect *cr;
	AG_GL_BlendState *bs;
	int y;

#ifdef DEBUG_GL
	Debug(drv, "GL Context Init\n");
#endif
	gl->textureGC = NULL;
	gl->nTextureGC = 0;
	gl->maxTextureGC = 0;
	gl->listGC = NULL;
	gl->nListGC = 0;
	gl->maxListGC = 0;

	for (y = 0; y < 32; y++)
		gl->dither[y] = ((y % 2)==0) ? 0x55555555 : 0xaaaaaaaa;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_CLIP_PLANE0);
	glEnable(GL_CLIP_PLANE1);
	glEnable(GL_CLIP_PLANE2);
	glEnable(GL_CLIP_PLANE3);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDisable(GL_BLEND);
/*	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_LIGHTING);

#if defined(AG_DEBUG) && defined(HAVE_GLEXT) && defined(GL_DEBUG_OUTPUT)
	if (agGLdebugOutput) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(AG_GL_DebugCallback, 0);
	}
#endif
	/* Clipping rectangle stack */
	gl->clipRects = Malloc(sizeof(AG_ClipRect));
	gl->maxClipRects = gl->nClipRects = 1;

	/* Alpha blending state stack */
	gl->blendStates = Malloc(sizeof(AG_GL_BlendState));
	gl->maxBlendStates = gl->nBlendStates = 1;
	bs = &gl->blendStates[0];
	bs->enabled = 0;
	bs->srcFactor = GL_ONE;
	bs->dstFactor = GL_ZERO;

	/* Initialize the first clipping rectangle. */
	cr = &gl->clipRects[0];
	AG_RectInit(&cr->r, 0,0,0,0);
	cr->eqns[0][0] =  1.0;    cr->eqns[0][1] =  0.0;
	cr->eqns[0][2] =  0.0;    cr->eqns[0][3] =  0.0;
	cr->eqns[1][0] =  0.0;    cr->eqns[1][1] =  1.0;
	cr->eqns[1][2] =  0.0;    cr->eqns[1][3] =  0.0;
	cr->eqns[2][0] = -1.0;    cr->eqns[2][1] =  0.0;
	cr->eqns[2][2] =  0.0;    cr->eqns[2][3] =  0.0; /* w */
	cr->eqns[3][0] =  0.0;    cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] =  0.0;    cr->eqns[3][3] =  0.0; /* h */
	
	drv->gl = gl;
}

/*
 * Size or resize an OpenGL context to specified dimensions.
 */
void
AG_GL_SetViewport(AG_GL_Context *gl, const AG_Rect *vp)
{
	AG_ClipRect *cr = &gl->clipRects[0];
	const int w = vp->w;
	const int h = vp->h;

	glViewport(vp->x, vp->y, w,h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -1.0, 1.0);

	cr->r.w = w;
	cr->r.h = h;
	cr->eqns[2][3] = (double)w;
	cr->eqns[3][3] = (double)h;
}

/* Destroy an OpenGL rendering context. */
void
AG_GL_DestroyContext(void *obj)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_Glyph *glyph;
	int i;

#if defined(AG_DEBUG) && defined(GL_DEBUG_OUTPUT)
	if (agGLdebugOutput)
		glDisable(GL_DEBUG_OUTPUT);
#endif

#ifdef DEBUG_GL
	Debug(drv, "GL Context Destroy\n");
#endif
	/* Invalidate any cached glyph renderings. */
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_FOREACH(glyph, &drv->glyphCache[i].glyphs, glyphs) {
			if (glyph->texture != 0) {
#ifdef DEBUG_GL
				Debug(drv, "GL delete glyph #%d\n", glyph->texture);
#endif
				glDeleteTextures(1, (GLuint *)&glyph->texture);
				glyph->texture = 0;
			}
		}
	}
	
	if (gl->nTextureGC > 0) {
#ifdef DEBUG_GL
		for (i = 0; i < gl->nTextureGC; i++)
			Debug(drv, "GL delete texture #%d\n", gl->textureGC[i]);
#endif
		glDeleteTextures(gl->nTextureGC, gl->textureGC);
		gl->nTextureGC = 0;
		free(gl->textureGC);
	}

	if (gl->nListGC > 0) {
		gl->nListGC = 0;
		free(gl->listGC);
	}

	free(gl->clipRects);

	drv->gl = NULL;
}

/*
 * Push a clipping rectangle onto the stack of clipping rectangles.
 *
 * Effectively set GL_CLIP_PLANE[0-3] to the intersection of the new
 * rectangle against the last stack entry.
 */
void
AG_GL_StdPushClipRect(void *obj, const AG_Rect *r)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_ClipRect *cr, *crPrev;

	if (gl->nClipRects+1 > gl->maxClipRects) {
		gl->maxClipRects += 4;
#ifdef DEBUG_GL
		Debug(drv, "GL maxClipRects -> %d\n", gl->maxClipRects);
#endif
		gl->clipRects = Realloc(gl->clipRects, gl->maxClipRects *
		                                       sizeof(AG_ClipRect));
	}
	crPrev = &gl->clipRects[gl->nClipRects-1];
	cr     = &gl->clipRects[gl->nClipRects++];

	AG_RectIntersect(&cr->r, &crPrev->r, r);

	cr->eqns[0][0] =  1.0;  cr->eqns[0][1] =  0.0;
	cr->eqns[0][2] =  0.0;  cr->eqns[0][3] = -(double)(cr->r.x);
	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);

	cr->eqns[1][0] =  0.0;  cr->eqns[1][1] =  1.0;
	cr->eqns[1][2] =  0.0;  cr->eqns[1][3] = -(double)(cr->r.y);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);

	cr->eqns[2][0] = -1.0;  cr->eqns[2][1] =  0.0;
	cr->eqns[2][2] =  0.0;  cr->eqns[2][3] = (double)(cr->r.x + cr->r.w);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);

	cr->eqns[3][0] =  0.0;  cr->eqns[3][1] = -1.0;
	cr->eqns[3][2] =  0.0;  cr->eqns[3][3] = (double)(cr->r.y + cr->r.h);
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
		AG_FatalError("PopClipRect() without Push");
#endif
	cr = &gl->clipRects[(--gl->nClipRects) - 1];

	glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
	glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
	glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
	glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
}

static __inline__ GLenum _Const_Attribute
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
	AG_GL_BlendState *bs;

/*	Debug(obj, "pushBlendingMode(%d,%d)\n", (int)fnSrc, (int)fnDst);  */

	if (gl->nBlendStates+1 > gl->maxBlendStates) {
		gl->maxBlendStates += 16;
#ifdef DEBUG_GL
		Debug(NULL, "maxBlendStates-> %d\n", gl->maxBlendStates);
#endif
		gl->blendStates = Realloc(gl->blendStates, gl->maxBlendStates *
		                                           sizeof(AG_GL_BlendState));
	}
	bs = &gl->blendStates[gl->nBlendStates++];
	bs->enabled = !(fnSrc == AG_ALPHA_ONE && fnDst == AG_ALPHA_ZERO);
	bs->srcFactor = AG_GL_GetBlendingFunc(fnSrc);
	bs->dstFactor = AG_GL_GetBlendingFunc(fnDst);

	if (bs->enabled) {
		glEnable(GL_BLEND);
		glBlendFunc(bs->srcFactor, bs->dstFactor);
	} else {
		glDisable(GL_BLEND);
	}
}
void
AG_GL_StdPopBlendingMode(void *obj)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	AG_GL_BlendState *bs;

#ifdef AG_DEBUG
	if (gl->nBlendStates < 1)
		AG_FatalError("PopBlendingMode() without Push");
#endif
	bs = &gl->blendStates[(--gl->nBlendStates) - 1];

/*	Debug(obj, "popBlendingMode (n->%d)\n", gl->nBlendStates); */

	if (bs->enabled) {
		glEnable(GL_BLEND);
		glBlendFunc(bs->srcFactor, bs->dstFactor);
	} else {
		glDisable(GL_BLEND);
	}
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
	if (S->format.BitsPerPixel >= 48) {                   /* Deep color */
		return (GL_UNSIGNED_SHORT);
	} else
#endif
	{
		return (GL_UNSIGNED_BYTE);
	}
}

static __inline__ int _Const_Attribute
PowOf2i(int i)
{
	int val = 1;

	while (val < i) { val <<= 1; }
	return (val);
}

/*
 * Create a hardware texture from the given AG_Surface.
 *
 * If needed, convert the surface to the required format.
 * Fill in texture coordinates in tc if not NULL.
 * Return new texture ID in rv.
 */
void
AG_GL_StdUploadTexture(void *obj, Uint *rv, AG_Surface *S, AG_TexCoord *tc)
{
	AG_Surface *GS;
	GLuint texture;
	const int isGLtexture = (S->flags & AG_SURFACE_GL_TEXTURE);
	const int w = (agGLuseNPOT) ? S->w : PowOf2i(S->w);
	const int h = (agGLuseNPOT) ? S->h : PowOf2i(S->h);

	glGenTextures(1, &texture);

	if (isGLtexture && (w == S->w) && (h == S->h)) {  /* POT & compatible */
		GS = S;
#ifdef DEBUG_GL
		Debug(obj, "GL upload (%dx%d) surface %p -> #%u (%dx%d)\n",
		    S->w, S->h, S, texture, w,h);
#endif
	} else {                         /* Need POT adjustment or conversion */
#ifdef DEBUG_GL
		Debug(obj, "GL upload " AGSI_BOLD "converted" AGSI_RST
		    " (%dx%d) surface %p -> #%u (%dx%d)\n",
		    S->w, S->h, S, texture, w,h);
#endif
		GS = AG_SurfaceStdRGBA(w,h);
		AG_SurfaceCopy(GS, S);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)S->w / (float)GS->w;
		tc->h = (float)S->h / (float)GS->h;
	}
	
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, GL_RGBA,
	    AG_GL_SurfaceType(GS), GS->pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (GS != S)
		AG_SurfaceFree(GS);

	*rv = (Uint)texture;
}

/*
 * Replace the contents of an existing texture with those of an AG_Surface.
 *
 * If needed, convert the surface to the required format.
 * Fill in texture coordinates in tc if not NULL.
 */
void
AG_GL_StdUpdateTexture(void *obj, Uint texture, AG_Surface *S, AG_TexCoord *tc)
{
	AG_Surface *GS;
	const int isGLtexture = (S->flags & AG_SURFACE_GL_TEXTURE);
	const int w = (agGLuseNPOT) ? S->w : PowOf2i(S->w);
	const int h = (agGLuseNPOT) ? S->h : PowOf2i(S->h);

	if (isGLtexture && (w == S->w) && (h == S->h)) {  /* POT & compatible */
		GS = S;
#ifdef DEBUG_GL
		Debug(obj, "GL update (%dx%d) surface %p -> #%u (%dx%d)\n",
		    S->w, S->h, S, texture, w,h);
#endif
	} else {                         /* Need POT adjustment or conversion */
#ifdef DEBUG_GL
		Debug(obj, "GL update " AGSI_BOLD "converted" AGSI_RST
		    " (%dx%d) surface %p -> #%u (%dx%d)\n", S->w, S->h,
		    S, texture, w,h);
#endif
		GS = AG_SurfaceStdRGBA(w,h);
		AG_SurfaceCopy(GS, S);
	}
	if (tc != NULL) {
		tc->x = 0.0f;
		tc->y = 0.0f;
		tc->w = (float)S->w / (float)GS->w;
		tc->h = (float)S->h / (float)GS->h;
	}

	glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, GL_RGBA,
	    AG_GL_SurfaceType(GS), GS->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (GS != S)
		AG_SurfaceFree(GS);
}

/*
 * Perform a software image transfer from an AG_Surface to a textured
 * polygon at widget coordinates x,y.
 */
void
AG_GL_BlitSurface(void *obj, AG_Widget *wid, AG_Surface *S, int x, int y)
{
	AG_Driver *drv = obj;
	AG_TexCoord tc;
	GLuint texture;
	
	AG_OBJECT_ISA(obj, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AGDRIVER_CLASS(drv)->uploadTexture(drv, &texture, S, &tc);

	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_POLYGON);
	{
		const int w = S->w;
		const int h = S->h;

		glTexCoord2f(tc.x, tc.y);  glVertex2i(x,   y);
		glTexCoord2f(tc.w, tc.y);  glVertex2i(x+w, y);
		glTexCoord2f(tc.w, tc.h);  glVertex2i(x+w, y+h);
		glTexCoord2f(tc.x, tc.h);  glVertex2i(x,   y+h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texture);
}

static __inline__ void
PrepareTexture(AG_Driver *_Nonnull drv, AG_Widget *_Nonnull wid, int name)
{
	if (wid->textures[name] == 0) {
		AGDRIVER_CLASS(drv)->uploadTexture(drv, &wid->textures[name],
		    wid->surfaces[name], &wid->texcoords[name]);
	} else if (wid->surfaceFlags[name] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[name] &= ~(AG_WIDGET_SURFACE_REGEN);

		AGDRIVER_CLASS(drv)->updateTexture(drv, wid->textures[name],
		    wid->surfaces[name], &wid->texcoords[name]);
	}
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
	const AG_Surface *S = wid->surfaces[name];
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	PrepareTexture(drv, wid, name);

	glBindTexture(GL_TEXTURE_2D, wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		const int x2 = x + S->w;
		const int y2 = y + S->h;
		AG_TexCoord tc;

		if (r != NULL) {
			tc.x = (float)r->x / PowOf2i(r->x);
			tc.y = (float)r->y / PowOf2i(r->y);
			tc.w = (float)r->w / PowOf2i(r->w);
			tc.h = (float)r->h / PowOf2i(r->h);
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
}

/*
 * Perform a software image transfer from an AG_Surface to a textured
 * rectangle of explicit size w,h in GL coordinates.
 */
void
AG_GL_BlitSurfaceGL(void *obj, AG_Widget *wid, AG_Surface *S, float w, float h)
{
	AG_Driver *drv = obj;
	AG_TexCoord tc;
	GLuint name;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AGDRIVER_CLASS(drv)->uploadTexture(drv, &name, S, &tc);
	glBindTexture(GL_TEXTURE_2D, name);
	glBegin(GL_POLYGON);
	{
		const float w_2 = w / 2.0f;
		const float h_2 = h / 2.0f;

		glTexCoord2f(tc.x, tc.y);  glVertex2f( w_2,  h_2);
		glTexCoord2f(tc.w, tc.y);  glVertex2f(-w_2,  h_2);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(-w_2, -h_2);
		glTexCoord2f(tc.x, tc.h);  glVertex2f( w_2, -h_2);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &name);
}

/*
 * Perform an accelerated image transfer from a mapped surface wid:s,
 * to a textured rectangle of explicit size w,h in GL coordinates.
 */
void
AG_GL_BlitSurfaceFromGL(void *obj, AG_Widget *wid, int name, float w, float h)
{
	AG_Driver *drv = obj;
	
	AG_OBJECT_ISA(obj, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	
	PrepareTexture(drv, wid, name);

	glBindTexture(GL_TEXTURE_2D, wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		const AG_TexCoord tc = wid->texcoords[name];
		const float w_2 = w / 2.0f;
		const float h_2 = h / 2.0f;

		glTexCoord2f(tc.x, tc.y);  glVertex2f( w_2,  h_2);
		glTexCoord2f(tc.w, tc.y);  glVertex2f(-w_2,  h_2);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(-w_2, -h_2);
		glTexCoord2f(tc.x, tc.h);  glVertex2f( w_2, -h_2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 * Perform an accelerated image transfer from a mapped surface wid:s,
 * to a textured rectangle of explicit size w,h in (flipped) GL coordinates.
 */
void
AG_GL_BlitSurfaceFlippedGL(void *obj, AG_Widget *wid, int name, float w, float h)
{
	AG_Driver *drv = obj;
	
	AG_OBJECT_ISA(obj, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");
	
	PrepareTexture(drv, wid, name);

	glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[name]);
	glBegin(GL_POLYGON);
	{
		const AG_TexCoord tc = wid->texcoords[name];

		glTexCoord2f(tc.w, tc.y);  glVertex2f(0.0f, 0.0f);
		glTexCoord2f(tc.x, tc.y);  glVertex2f(w,    0.0f);
		glTexCoord2f(tc.x, tc.h);  glVertex2f(w,    h);
		glTexCoord2f(tc.w, tc.h);  glVertex2f(0.0f, h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 * Back up every mapped GL texture to a software surface, so that textures
 * can be restored automatically following a possible loss of GL context.
 */
void
AG_GL_BackupSurfaces(void *obj, AG_Widget *wid)
{
	AG_Surface *S;
	GLint w, h;
	Uint i;
	
	AG_OBJECT_ISA(obj, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nSurfaces; i++)  {
		if (wid->textures[i] == 0 || wid->surfaces[i] != NULL)
			continue;

		glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[i]);

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

		S = AG_SurfaceStdRGBA(w, h);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		    S->pixels);

		glBindTexture(GL_TEXTURE_2D, 0);

		wid->surfaces[i] = S;
	}
	glDeleteTextures(wid->nSurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nSurfaces*sizeof(Uint));
	AG_ObjectUnlock(wid);
}

/* Re-upload previously saved textures following loss of GL context. */
void
AG_GL_RestoreSurfaces(void *obj, AG_Widget *wid)
{
	AG_Driver *drv = obj;
	Uint i;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AG_ObjectLock(wid);

	for (i = 0; i < wid->nSurfaces; i++)  {
		if (wid->surfaces[i] != NULL) {
			AGDRIVER_CLASS(drv)->uploadTexture(drv,
			    &wid->textures[i], wid->surfaces[i],
			    &wid->texcoords[i]);
		} else {
			wid->textures[i] = 0;
		}
	}

	AG_ObjectUnlock(wid);
}

/*
 * Render the specified widget to an AG_Surface.
 * TODO render to offscreen buffer instead of display where possible.
 */
int
AG_GL_RenderToSurface(void *obj, AG_Widget *wid, AG_Surface **S)
{
	AG_Driver *drv = obj;
	Uint8 *pixels;
	const int w = wid->w;
	const int h = wid->h;
	int visiblePrev;
	
	AG_OBJECT_ISA(drv, "AG_Driver:*");
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	AG_BeginRendering(drv);
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering(drv);

	if ((pixels = AG_TryMalloc(w*h*4)) == NULL) {
		return (-1);
	}
	if (AGDRIVER_MULTIPLE(drv)) {
		glReadPixels(wid->rView.x1,
		    HEIGHT(AGDRIVER_MW(drv)->win) - wid->rView.y2,
		    w, h,
		    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	} else {
		glReadPixels(wid->rView.x1,
		    agDriverSw->h - wid->rView.y2,
		    w, h,
		    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}

	if (AG_PackedPixelFlip(pixels, wid->h, w*4) == -1)
		goto fail;

	*S = AG_SurfaceFromPixelsRGBA(pixels, w, h, 32,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    0);

	if (*S == NULL) {
		goto fail;
	}
	return (0);
fail:
	Free(pixels);
	return (-1);
}

/* Put pixel of color c at x,y. */
void
AG_GL_PutPixel(void *obj, int x, int y, const AG_Color *c)
{
	glBegin(GL_POINTS);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x-1, y);
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
AG_GL_BlendPixel(void *obj, int x, int y, const AG_Color *c, AG_AlphaFn fnSrc,
    AG_AlphaFn fnDst)
{
	AG_Driver *drv = obj;

	AGDRIVER_CLASS(drv)->pushBlendingMode(drv, fnSrc, fnDst);

	glBegin(GL_POINTS);
	GL_Color4uH(c->r, c->g, c->b, c->a);
	glVertex2i(x,y);
	glEnd();

	AGDRIVER_CLASS(drv)->popBlendingMode(drv);
}

/* Draw a solid line from (x1,y1) to (x2,y2) */
void
AG_GL_DrawLine(void *obj, int x1, int y1, int x2, int y2, const AG_Color *c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
}

/* Draw a solid horizontal line from (x1,y) to (x2,y). */
void
AG_GL_DrawLineH(void *obj, int x1, int x2, int y, const AG_Color *c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1-1, y);
	glVertex2i(x2, y);
	glEnd();
}

/* Draw a solid vertical line from (x,y1) to (x,y2). */
void
AG_GL_DrawLineV(void *obj, int x, int y1, int y2, const AG_Color *c)
{
	glBegin(GL_LINES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x, y1);
	glVertex2i(x, y2);
	glEnd();
}

/* Draw a alpha-blended line from (x1,y1) to (x2,y2). */
void
AG_GL_DrawLineBlended(void *obj, int x1, int y1, int x2, int y2,
    const AG_Color *c, AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	if (c->a < AG_OPAQUE)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_LINES);
	GL_Color4uH(c->r, c->g, c->b, c->a);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();

	if (c->a < AG_OPAQUE)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

/*
 * Draw a line from (x1,y1) to (x2,y2).
 * Handle widths > 0.
 */
void
AG_GL_DrawLineW(void *obj, int x1, int y1, int x2, int y2, const AG_Color *c,
    float width)
{
	float widthSaved;

	glGetFloatv(GL_LINE_WIDTH, &widthSaved);
	if (widthSaved != width) { glLineWidth(width); }
 
	glBegin(GL_LINES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	
	if (widthSaved != width) { glLineWidth(widthSaved); }
}

/*
 * Draw a line from (x1,y1) to (x2,y2).
 * Handle widths > 0 and 16-bit stipple pattern.
 */
void
AG_GL_DrawLineW_Sti16(void *obj, int x1, int y1, int x2, int y2,
    const AG_Color *c, float width, Uint16 stipple)
{
	float widthSaved;
	int stipSaved;
	
	glGetFloatv(GL_LINE_WIDTH, &widthSaved);
	if (widthSaved != width) { glLineWidth(width); }

	stipSaved = glIsEnabled(GL_LINE_STIPPLE);
	if (!stipSaved) { glEnable(GL_LINE_STIPPLE); }
	
	glBegin(GL_LINES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y2);
	glEnd();
	
	if (!stipSaved) { glDisable(GL_LINE_STIPPLE); }
	if (widthSaved != width) { glLineWidth(widthSaved); }
}

/* Draw a solid triangle from 3 points. */
void
AG_GL_DrawTriangle(void *obj, const AG_Pt *v1, const AG_Pt *v2, const AG_Pt *v3,
    const AG_Color *c)
{
	glBegin(GL_TRIANGLES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(v1->x, v1->y);
	glVertex2i(v2->x, v2->y);
	glVertex2i(v3->x, v3->y);
	glEnd();
}

/* Draw a solid polygon from n points. */
void
AG_GL_DrawPolygon(void *obj, const AG_Pt *pts, Uint nPts, const AG_Color *c)
{
	Uint i;

	glBegin(GL_POLYGON);
	GL_Color3uH(c->r, c->g, c->b);
	for (i = 0; i < nPts; i++) {
		glVertex2i(pts[i].x,
		           pts[i].y);
	}
	glEnd();
}

/* Draw a stippled polygon from n points with a 32x32-bit stipple pattern. */
void
AG_GL_DrawPolygon_Sti32(void *obj, const AG_Pt *pts, Uint nPts,
    const AG_Color *c, const Uint8 *stipple)
{
	Uint8 stipplePrev[32*4];
	Uint i;

	glGetPolygonStipple(stipplePrev);
	glPolygonStipple(stipple);

	glBegin(GL_POLYGON);
	GL_Color3uH(c->r, c->g, c->b);
	for (i = 0; i < nPts; i++) {
		glVertex2i(pts[i].x, pts[i].y);
	}
	glEnd();

	glPolygonStipple(stipplePrev);
}

static void
AG_GL_DrawArrow_Up(int x, int y, int h, const AG_Color *c)
{
	int h_2 = h >> 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x - 1,       y - h_2);
	glVertex2i(x - h_2 - 1, y - h_2 + h + 1);
	glVertex2i(x + h_2 - 1, y - h_2 + h + 1);
	glEnd();
}

static void
AG_GL_DrawArrow_Right(int x, int y, int h, const AG_Color *c)
{
	int h_2 = (h >> 1);
	int x1 = x - h_2 - 1;
	int x2 = x1 + h + 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x2, y);
	glVertex2i(x1, y - h_2);
	glVertex2i(x1, y + h_2);
	glEnd();
}

static void
AG_GL_DrawArrow_Down(int x, int y, int h, const AG_Color *c)
{
	int h_2 = (h >> 1);
	int x1 = x - 1;
	int y1 = y - h_2;
	int y2 = y1 + h + 1;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1,       y2);
	glVertex2i(x1 + h_2, y1);
	glVertex2i(x1 - h_2, y1);
	glEnd();
}

static void
AG_GL_DrawArrow_Left(int x, int y, int h, const AG_Color *c)
{
	int h_2 = (h >> 1);
	int x1 = x - h_2 - 1;
	int x2 = x1 + h;

	glBegin(GL_TRIANGLES);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x1, y);
	glVertex2i(x2, y+h_2);
	glVertex2i(x2, y-h_2);
	glEnd();
}

/* Draw an arrow of height h at (x,y), rotated by a given angle. */
void
AG_GL_DrawArrow(void *obj, Uint8 angle, int x0, int y0, int h, const AG_Color *c)
{
	static void (*pf[])(int,int, int, const AG_Color *) = {
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
AG_GL_FillRect(void *obj, const AG_Rect *r, const AG_Color *c)
{
	int x = r->x;
	int y = r->y;
	int x2 = x + r->w - 1;
	int y2 = y + r->h - 1;
	
	glBegin(GL_POLYGON);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x,  y);
	glVertex2i(x2, y);
	glVertex2i(x2, y2);
	glVertex2i(x,  y2);
	glEnd();
}

/* Solid rectangle fill with color c + dithering. */
void
AG_GL_DrawRectDithered(void *obj, const AG_Rect *r, const AG_Color *c)
{
	AG_Driver *drv = obj;
	AG_GL_Context *gl = drv->gl;
	const int stipplePrev = glIsEnabled(GL_POLYGON_STIPPLE);

	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);

	glPolygonStipple((GLubyte *)gl->dither);
	AG_GL_DrawRectFilled(obj, r, c);

	glPopAttrib();
	if (!stipplePrev) { glDisable(GL_POLYGON_STIPPLE); }
}

/* Draw a box with all rounded corners. */
void
AG_GL_DrawBoxRounded(void *obj, const AG_Rect *r, int z, int radius,
    const AG_Color *c1, const AG_Color *c2, const AG_Color *c3)
{
	float rad = (float)radius, dia = 2.0f*rad;
	float t, i, nFull = rad*4.0f, nQuart = nFull/4.0f;
	float w, h;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r->x + rad,
	             (float)r->y + rad, 0.0f);

 	w = (float)r->w;
	h = (float)r->h;

	glBegin(GL_POLYGON);
	GL_Color3uH(c1->r, c1->g, c1->b);
	{
		for (i = 0.0f; i <= nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           -rad*Sin(t));
		}

		t = 2.0f*AG_PI*nQuart;
		glVertex2f((w - dia + rad*Cos(t/nFull)),
		           -rad*Sin(t/nFull));

		for (i = nQuart-1.0f; i >= 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t),
			                    -rad*Sin(t));
		}

		for (i = 0.0f; i <= nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t),
			           h - dia + rad*Sin(t));
		}

		t = 2.0f*AG_PI*nQuart;
		glVertex2f(0, h - rad);

		for (i = nQuart-1.0f; i >= 1.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           h - dia + rad*Sin(t));
		}
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	{
		GL_Color3uH(c2->r, c2->g, c2->b);
		for (i = 0.0f; i <= nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           -rad*Sin(t));
		}

		t = 2.0f*AG_PI*nQuart;
		glVertex2f((w - dia + rad*Cos(t/nFull)),
		           -rad*Sin(t/nFull));

		GL_Color3uH(c3->r, c3->g, c3->b);
		for (i = nQuart-1.0f; i >= 0.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t),
			           -rad*Sin(t));
		}
		for (i = 0.0f; i <= nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t),
			           h - dia + rad*Sin(t));
		}
		
		t = 2.0f*AG_PI*nQuart;
		glVertex2f(0, h - rad - 1);

		for (i = nQuart-1.0f; i >= 1.0f; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t),
			           h - dia + rad*Sin(t));
		}
	}
	glEnd();

	glPopMatrix();
}

/* Draw a box with top rounded corners. */
void
AG_GL_DrawBoxRoundedTop(void *obj, const AG_Rect *r, int z, int radius,
    const AG_Color *c1, const AG_Color *c2, const AG_Color *c3)
{
	float rad = (float)radius, dia = rad*2.0f;
	float t, w,h;
	const int nFull = radius << 2;
	const int nQuart = nFull >> 2;
	int i;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslatef((float)r->x + rad,
	             (float)r->y + rad, 0.0f);
	w = (float)r->w;
	h = (float)r->h;

	glBegin(GL_POLYGON);
	{
		GL_Color3uH(c1->r, c1->g, c1->b);
		glVertex2f(-rad, h - rad);
		for (i = 0; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-rad*Cos(t), -rad*Sin(t));
		}
		glVertex2f(0.0f, -rad);
		for (i = nQuart; i > 0; i--) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t), -rad*Sin(t));
		}
		glVertex2f(w-rad, h-rad);
	}
	glEnd();
	
	glBegin(GL_LINE_STRIP);
	{
		AG_Color cx;

		GL_Color3uH(c2->r, c2->g, c2->b);
		glVertex2f(-rad, h-rad);
		for (i = 0; i < nQuart; i++) {
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(-(float)rad*Cos(t), -(float)rad*Sin(t));
		}

		glVertex2f(0.0f, -rad);
		t = (2.0f*AG_PI*nQuart)/nFull;
		glVertex2f(w - dia + rad*Cos(t), -rad*Sin(t));
		
		for (i = nQuart-1; i > 0; i--) {
			AG_ColorInterpolate(&cx, c3, c2, i,nQuart);
			GL_Color3uH(cx.r, cx.g, cx.b);
			t = (2.0f*AG_PI*i)/nFull;
			glVertex2f(w - dia + rad*Cos(t), -rad*Sin(t));
		}
		glVertex2f(w-rad, h-rad);
	}
	glEnd();
	glPopMatrix();
}

/* Draw (an approximation of) a circle of radius r centered at x,y. */
void
AG_GL_DrawCircle(void *obj, int x, int y, int r, const AG_Color *c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_LINE_LOOP);
	GL_Color3uH(c->r, c->g, c->b);
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
AG_GL_DrawCircleFilled(void *obj, int x, int y, int r, const AG_Color *c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_POLYGON);
	GL_Color3uH(c->r, c->g, c->b);
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
AG_GL_DrawCircle2(void *obj, int x, int y, int r, const AG_Color *c)
{
	float i, nEdges = r*2;
	float R = (float)r;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)x, (float)y, 0.0f);
	
	glBegin(GL_LINE_LOOP);
	GL_Color3uH(c->r, c->g, c->b);
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
AG_GL_DrawRectFilled(void *obj, const AG_Rect *r, const AG_Color *c)
{
	int x = r->x;
	int y = r->y;
	int x2 = x + r->w - 1;
	int y2 = y + r->h - 1;
	
	glBegin(GL_POLYGON);
	GL_Color3uH(c->r, c->g, c->b);
	glVertex2i(x, y);
	glVertex2i(x2, y);
	glVertex2i(x2, y2);
	glVertex2i(x, y2);
	glEnd();
}

/* Draw a rectangle r filled with color c (blend according to c's alpha). */
void
AG_GL_DrawRectBlended(void *obj, const AG_Rect *r, const AG_Color *c,
    AG_AlphaFn fnSrc, AG_AlphaFn fnDst)
{
	int x1 = r->x;
	int y1 = r->y;
	int x2 = x1 + r->w - 1;
	int y2 = y1 + r->h - 1;

	if (c->a < AG_OPAQUE)
		AGDRIVER_CLASS(obj)->pushBlendingMode(obj, fnSrc, fnDst);

	glBegin(GL_POLYGON);
	GL_Color4uH(c->r, c->g, c->b, c->a);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();

	if (c->a < AG_OPAQUE)
		AGDRIVER_CLASS(obj)->popBlendingMode(obj);
}

/* Prepare for rendering an AG_Text(3) glyph. */
void
AG_GL_UpdateGlyph(void *obj, AG_Glyph *gl)
{
	AG_Driver *drv = (AG_Driver *)obj;

	AG_OBJECT_ISA(drv, "AG_Driver:*");

	AGDRIVER_CLASS(drv)->uploadTexture(drv,
	    &gl->texture, gl->su, &gl->texcoords);
}

/* Render an AG_Text(3) glyph at x,y. */
void
AG_GL_DrawGlyph(void *obj, const AG_Glyph *G, int x, int y)
{
	glBindTexture(GL_TEXTURE_2D, G->texture);
	glBegin(GL_POLYGON);
	{
		const AG_TexCoord tc = G->texcoords;
		const int w = G->su->w;
		const int h = G->su->h;

		glTexCoord2f(tc.x, tc.y);  glVertex2i(x,   y);
		glTexCoord2f(tc.w, tc.y);  glVertex2i(x+w, y);
		glTexCoord2f(tc.w, tc.h);  glVertex2i(x+w, y+h);
		glTexCoord2f(tc.x, tc.h);  glVertex2i(x,   y+h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Upload a texture. */
void
AG_GL_UploadTexture(void *obj, Uint *name, AG_Surface *S, AG_TexCoord *tc)
{
	AG_Driver *drv = obj;

	AGDRIVER_CLASS(drv)->uploadTexture(drv, name, S, tc);
}

/* Update the contents of an existing GL texture. */
void
AG_GL_UpdateTexture(void *obj, Uint name, AG_Surface *S, AG_TexCoord *tc)
{
	AG_Driver *drv = obj;

	AGDRIVER_CLASS(drv)->updateTexture(drv, name, S, tc);
}

/* Queue a GL texture for deletion. */
void
AG_GL_DeleteTexture(void *_Nonnull obj, Uint name)
{
	AG_Driver *drv = obj;

	AGDRIVER_CLASS(drv)->deleteTexture(drv, name);
}

/* Queue a GL display list for deletion. */
void
AG_GL_DeleteList(void *_Nonnull obj, Uint name)
{
	AG_Driver *drv = obj;
	AG_DriverClass *dc = AGDRIVER_CLASS(drv);

	if (dc->deleteList != NULL)
		dc->deleteList(drv, name);
}
