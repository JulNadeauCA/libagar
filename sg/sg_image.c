/*
 * Copyright (c) 2010-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Image surface mapped onto flat complex polygon.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/gui/opengl.h>

#include <string.h>
#include <stdlib.h>

/* Save steps in contour processing to jpeg files. */
/* #define DEBUG_DUMP */
/* #define DEBUG_VERBOSE */

#ifndef SG_IMAGE_PRECISION
#define SG_IMAGE_PRECISION 0.2		/* For intersection test */
#endif

/* Create a new Image node. */
SG_Image *
SG_ImageNew(void *parent, const char *name)
{
	SG_Image *si;

	si = Malloc(sizeof(SG_Image));
	AG_ObjectInit(si, &sgImageClass);
	if (name) {
		AG_ObjectSetNameS(si, name);
	} else {
		OBJECT(si)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, si);
	return (si);
}

/* Create a new Image node and set an initial surface. */
SG_Image *
SG_ImageFromSurface(void *parent, const char *name, const AG_Surface *su,
    M_Real scale)
{
	SG_Image *si;

	if ((si = SG_ImageNew(parent, name)) == NULL) {
		return (NULL);
	}
	if (su->flags & AG_SURFACE_ANIMATED) {
		SG_ImageSetAnim(si, su, scale);
	} else {
		SG_ImageSetSurface(si, su, scale);
	}
	return (si);
}
SG_Image *
SG_ImageFromSurfaceNODUP(void *parent, const char *name, AG_Surface *su,
    M_Real scale)
{
	SG_Image *si;

	if ((si = SG_ImageNew(parent, name)) == NULL) {
		return (NULL);
	}
	if (su->flags & AG_SURFACE_ANIMATED) {
		SG_ImageSetAnimNODUP(si, su, scale);
	} else {
		SG_ImageSetSurfaceNODUP(si, su, scale);
	}
	return (si);
}

static void
TessError(GLenum err)
{
#if defined(DEBUG_VERBOSE)
	printf("GLU tesselator error: %s\n", gluErrorString(err));
#endif
}

static void
TessBegin(GLenum type, void *_Nullable arg)
{
	GL_Begin(type);
}

static void
TessVertex(void *_Nonnull vertexData, void *_Nullable polygonData)
{
	SG_Image *si = polygonData;
	GLdouble *v = vertexData;
	
	GL_TexCoord2((v[0]/si->w + 0.5)*si->tc->w,
	            (-v[1]/si->h + 0.5)*si->tc->h);
	GL_Vertex2(v[0], v[1]);
}

static void
TessEnd(void *_Nullable arg)
{
	GL_End();
}

static void
TessCombine(GLdouble coords[3], GLdouble *_Nullable vdata[4], GLfloat wt[4],
    GLdouble *_Nonnull *_Nonnull out)
{
	GLdouble *v;

	v = Malloc(3*sizeof(GLdouble));
	v[0] = coords[0];
	v[1] = coords[1];
	v[2] = coords[2];
	*out = v;
}

static void
Init(void *_Nonnull obj)
{
	SG_Image *si = obj;

	si->path[0] = '\0';
	si->shape = SG_IMAGE_POLY;
	AG_ColorBlack(&si->color);
	si->flags = 0;
	si->su = NULL;
	si->tc = NULL;
	si->an = NULL;
	si->w = 1.0;
	si->h = 1.0;
	si->tolContour = 5.0;
	si->tolHoles = 1.0;
	si->tessBufs = NULL;
	si->nTessBufs = 0;
	si->holes = NULL;
	si->nHoles = 0;
	M_PolygonInit(&si->contour);
	TAILQ_INIT(&si->vtex);
	TAILQ_INIT(&si->vlist);

	if ((si->to = (void *)gluNewTess()) == NULL) {
		AG_FatalError("gluNewTess");
	}
	gluTessCallback(si->to, GLU_TESS_ERROR,		(void *)TessError);
	gluTessCallback(si->to, GLU_TESS_BEGIN_DATA,	(void *)TessBegin);
	gluTessCallback(si->to, GLU_TESS_VERTEX_DATA,	(void *)TessVertex);
	gluTessCallback(si->to, GLU_TESS_END_DATA,	(void *)TessEnd);
	gluTessCallback(si->to, GLU_TESS_COMBINE,	(void *)TessCombine);

	SG_EnableAction(si, SG_ACTION_MOVE);
	SG_EnableAction(si, SG_ACTION_ZMOVE);
	SG_EnableAction(si, SG_ACTION_ROTATE);
	SG_EnableAction(si, SG_ACTION_SCALE);
}

void
SG_ImageFreeCached(void *obj)
{
	SG_Image *si = obj;
	SG_ViewTexture *vt, *nVt;
	SG_ViewList *vl, *nVl;

	/* Free the cached per-view textures. */
	for (vt = TAILQ_FIRST(&si->vtex);
	     vt != TAILQ_END(&si->vtex);
	     vt = nVt) {
		nVt = TAILQ_NEXT(vt, textures);
		AG_GL_DeleteTexture(WIDGET(vt->sv)->drv, vt->name);
		Free(vt);
	}
	TAILQ_INIT(&si->vtex);
	
	/* Free the cached per-view display lists. */
	for (vl = TAILQ_FIRST(&si->vlist);
	     vl != TAILQ_END(&si->vlist);
	     vl = nVl) {
		nVl = TAILQ_NEXT(vl, lists);
		AG_GL_DeleteList(WIDGET(vl->sv)->drv, vl->name);
		Free(vl);
	}
	TAILQ_INIT(&si->vlist);
}

void
SG_ImageFreeSurfaces(void *obj)
{
	SG_Image *si = obj;

	SG_ImageFreeCached(si);

	if (si->su != NULL) {
		if (!(si->flags & SG_IMAGE_NODUP)) {
			AG_SurfaceFree(si->su);
		}
		si->su = NULL;
	}
	if (si->an != NULL) {
		AG_AnimStateDestroy(&si->ast);
		if (!(si->flags & SG_IMAGE_NODUP_ANIM)) {
			AG_SurfaceFree(si->an);
		}
		si->an = NULL;
	}
}

/* Clear hole detection results. */
static void
FreeHoles(SG_Image *_Nonnull si)
{
	Uint i;

	if (si->nHoles > 0) {
		for (i = 0; i < si->nHoles; i++) {
			M_PolygonFree(&si->holes[i]);
		}
		Free(si->holes);
		si->holes = NULL;
		si->nHoles = 0;
	}
}

static void
FreeTessBufs(SG_Image *_Nonnull si)
{
	Uint i;

	for (i = 0; i < si->nTessBufs; i++) {
		Free(si->tessBufs[i]);
	}
	Free(si->tessBufs);
	si->tessBufs = NULL;
	si->nTessBufs = 0;
}

static void
Reset(void *_Nonnull obj)
{
	SG_Image *si = obj;

	M_PolygonFree(&si->contour);
	FreeHoles(si);
	FreeTessBufs(si);
	SG_ImageFreeSurfaces(si);
}

static void
Destroy(void *_Nonnull obj)
{
	SG_Image *si = obj;

	gluDeleteTess(si->to);
}

static int
LoadFromPath(SG_Image *_Nonnull si, const char *_Nonnull path)
{
	char *ext;
	AG_Surface *an;
		
	if ((ext = strrchr(path, '.')) == NULL) {
		AG_SetError("Invalid path");
		return (-1);
	}
	if (strchr(path, '%') != NULL) {
		if (Strcasecmp(ext, ".png") == 0) {
			if ((an = AG_SurfaceFromPNGs(path, 0, -1,
			     AG_DISPOSE_UNSPECIFIED, 500, 0)) == NULL)
				return (-1);
		} else {
			AG_SetError("Invalid anim path");
			return (-1);
		}
		SG_ImageSetAnim(si, an, 0.01);
		SG_ImageAnimPlay(si);
		AG_SurfaceFree(an);
	} else {
		AG_Surface *su;

		if ((su = AG_SurfaceFromFile(path)) == NULL) {
			return (-1);
		}
		SG_ImageSetSurface(si, su, 0.01);
		AG_SurfaceFree(su);
	}
	return (0);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds,
   const AG_Version *_Nonnull ver)
{
	SG_Image *si = obj;
	
	si->flags &= ~(SG_IMAGE_SAVED);
	si->flags |= (AG_ReadUint32(ds) & SG_IMAGE_SAVED);

	AG_CopyString(si->path, ds, sizeof(si->path));
	si->shape = (enum sg_image_shape)AG_ReadUint8(ds);
	AG_ReadColor(&si->color, ds);
	si->w = M_ReadReal(ds);
	si->h = M_ReadReal(ds);

	if (si->flags & SG_IMAGE_SAVE_SURFACE) {
		/* Surface data follows */
		SG_ImageFreeSurfaces(si);
		si->flags &= ~(SG_IMAGE_NODUP);
		if ((si->su = AG_ReadSurface(ds)) == NULL)
			goto fail;
	} else {
		/* Load surface/animation from specified path */
		if (LoadFromPath(si, si->path) == -1)
			goto fail;
	}
	return (0);
fail:
	si->path[0] = '\0';
	SG_ImageFreeSurfaces(si);
	return (-1);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Image *si = obj;

	AG_WriteUint32(ds, (Uint32)(si->flags & SG_IMAGE_SAVED));
	AG_WriteString(ds, si->path);
	AG_WriteUint8(ds, si->shape);
	AG_WriteColor(ds, &si->color);
	M_WriteReal(ds, si->w);
	M_WriteReal(ds, si->h);

	if (si->flags & SG_IMAGE_SAVE_SURFACE) {
		AG_WriteSurface(ds, si->su);
	}
	return (0);
}

static void
ClearCache(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);

	SG_ImageFreeCached(si);
}

static void
UpdateMode(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);

	SG_ImageSetShape(si, si->shape);
	SG_ImageFreeCached(si);
}

static void
LoadImageFile(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_Surface *su;

	if ((su = AG_SurfaceFromFile(path)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	if (SG_ImageSetSurface(si, su, 0.01) == 0) {
		SG_ImageSetShapeAuto(si);
		Strlcpy(si->path, path, sizeof(si->path));
	}
	AG_SurfaceFree(su);
}

static void
PreviewImage(AG_Event *_Nonnull event)
{
	AG_Widget *box = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_Surface *su;
	AG_Scrollview *sv;
	
	AG_ObjectFreeChildren(box);

	if ((su = AG_SurfaceFromFile(path)) == NULL) {
		AG_LabelNew(box, 0, _("Preview: %s"), AG_GetError());
		return;
	}
	AG_LabelNew(box, 0, _("Depth: %d bpp"), su->format.BitsPerPixel);
	AG_LabelNew(box, 0, _("Size: %dx%d"), su->w, su->h);
	/* TODO: Analyze for SG_Image vectorialization */

	AG_SeparatorNewHoriz(box);

	sv = AG_ScrollviewNew(box, AG_SCROLLVIEW_BY_MOUSE|AG_SCROLLVIEW_EXPAND);
	if (su->w > 320 || su->h > 240) {
		(void)AG_PixmapFromSurfaceScaled(sv, 0, su,
		    MIN(su->w,320),
		    MIN(su->h,240));
	} else {
		(void)AG_PixmapFromSurface(sv, 0, su);
	}
	AG_SurfaceFree(su);
	AG_WidgetUpdate(box);
}
 
static void
LoadImageFileDlg(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);
	AG_Window *win;
	AG_Pane *pa;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Load Image..."));
	pa = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		fd = AG_FileDlgNew(pa->div[0], AG_FILEDLG_EXPAND|
		                               AG_FILEDLG_CLOSEWIN|
					       AG_FILEDLG_MASK_EXT|
					       AG_FILEDLG_MASK_HIDDEN);
		WIDGET(fd)->flags &= ~(AG_WIDGET_FOCUSABLE);
		AG_FileDlgSetDirectoryMRU(fd, "sg-images", ".");
		
		AG_SetEvent(fd, "file-selected",
		    PreviewImage, "%p", pa->div[1]);

		AG_FileDlgAddType(fd, _("Portable Network Graphics"), "*.png",
		    LoadImageFile, "%p", si);
		AG_FileDlgAddType(fd, _("JPEG Image"), "*.jpg,*.jpeg",
		    LoadImageFile, "%p", si);
		AG_FileDlgAddType(fd, _("Windows Bitmap"), "*.bmp",
		    LoadImageFile, "%p", si);
	}

	AG_PaneMoveDividerPct(pa, 60);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 40, 40);
	AG_WindowShow(win);
}

static void
PathEntered(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);

	if (LoadFromPath(si, si->path) == -1)
		AG_TextMsgFromError();
}

static void
InstContour(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);
	SG_Polygon *poly;

	poly = SG_PolygonNew(si, "Contour", &si->contour);
	SG_PolygonColor(poly, M_ColorRGB(0, 100, 100));
	SG_PolygonWidth(poly, 2.0);
}

static void
InstHoles(AG_Event *_Nonnull event)
{
	SG_Image *si = AG_PTR(1);
	SG_Polygon *poly;
	Uint i;

	for (i = 0; i < si->nHoles; i++) {
		poly = SG_PolygonNew(si, "Holes", &si->holes[i]);
		SG_PolygonColor(poly, M_ColorRGB(180, 0, 0));
	}
}

static void *_Nullable
Edit(void *_Nonnull p, SG_View *_Nullable sgv)
{
	SG_Image *si = p;
	AG_Mutex *lock = &OBJECT(si)->lock;
	AG_Numerical *nWidth, *nHeight;
	AG_Checkbox *cb;
	AG_Box *box, *hBox;
	AG_Radio *rad;
	const char *siModes[] = {
		N_("Rectangle"),
		N_("Polygon"),
		NULL
	};
	AG_Textbox *tb;
	AG_Slider *sl;

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(hBox, AG_TEXTBOX_HFILL, _("Image: "));
		AG_TextboxBindASCII(tb, si->path, sizeof(si->path));
		AG_SetEvent(tb, "textbox-return", PathEntered, "%p", si);
		AG_ButtonNewFn(hBox, 0, "...", LoadImageFileDlg, "%p", si);
	}
	
	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("Shape:  "));
		rad = AG_RadioNew(hBox, 0, siModes);
		AG_BindUintMp(rad, "value", (Uint *)&si->shape, lock);
		AG_SetEvent(rad, "radio-changed", UpdateMode, "%p", si);
	}

	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("Contour: "));
		sl = AG_SliderNew(hBox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL);
		AG_BindFloatMp(sl, "value", &si->tolContour, lock);
		AG_SetFloat(sl, "min", 0.01f);
		AG_SetFloat(sl, "max", 20.0f);
		AG_SetFloat(sl, "inc", 0.1f);
		AG_SetEvent(sl, "slider-changed", ClearCache, "%p", si);
	}
	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("    Holes: "));
		sl = AG_SliderNew(hBox, AG_SLIDER_HORIZ, AG_SLIDER_HFILL);
		AG_BindFloatMp(sl, "value", &si->tolHoles, lock);
		AG_SetFloat(sl, "min", 0.01f);
		AG_SetFloat(sl, "max", 20.0f);
		AG_SetFloat(sl, "inc", 0.1f);
		AG_SetEvent(sl, "slider-changed", ClearCache, "%p", si);
	}
	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		AG_LabelNewS(hBox, 0, _("Size: "));
		nWidth = AG_NumericalNew(hBox, 0, NULL, NULL);
		AG_LabelNewS(hBox, 0, "x");
		nHeight = AG_NumericalNew(hBox, 0, NULL, NULL);
		M_BindRealMp(nWidth, "value", &si->w, lock);
		M_BindRealMp(nHeight, "value", &si->h, lock);
		M_SetReal(nWidth, "inc", 0.05);
		M_SetReal(nHeight, "inc", 0.05);
		AG_SetEvent(nWidth, "numerical-changed", UpdateMode, "%p", si);
		AG_SetEvent(nHeight, "numerical-changed", UpdateMode, "%p", si);
	}
	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		cb = AG_CheckboxNew(hBox, 0, _("Billboard"));
		AG_BindFlagMp(cb, "state", &si->flags, SG_IMAGE_BILLBOARD, lock);
		AG_SpacerNewVert(hBox);
		cb = AG_CheckboxNew(hBox, 0, _("Wireframe"));
		AG_BindFlagMp(cb, "state", &si->flags, SG_IMAGE_WIREFRAME, lock);
		AG_SpacerNewVert(hBox);
		cb = AG_CheckboxNew(hBox, 0, _("HoleDet"));
		AG_BindFlagMp(cb, "state", &si->flags, SG_IMAGE_HOLES, lock);
		AG_SetEvent(cb, "checkbox-changed", ClearCache, "%p", si);
	}
	hBox = AG_BoxNewHoriz(box, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hBox, 0, _("Trace Contour"), InstContour, "%p", si);
		AG_ButtonNewFn(hBox, 0, _("Trace Holes"),   InstHoles, "%p", si);
	}
	return (box);
}

static __inline__ void
MaskPixel(SG_Image *_Nonnull si, Uint8 *_Nonnull mask, int x, int y)
{
	mask[y*si->su->w + x] = 1;
}

#if 0
static __inline__ void
UnmaskPixel(SG_Image *_Nonnull si, Uint8 *_Nonnull mask, int x, int y)
{
	mask[y*si->su->w + x] = 0;
}
#endif

static __inline__ int
PixelMasked(SG_Image *_Nonnull si, Uint8 *_Nonnull mask, int x, int y)
{
	if (x < 0 || x >= si->su->w ||
	    y < 0 || y >= si->su->h) {
		return (1);
	}
	return mask[y*si->su->w + x];
}

/*
 * Compute a black and white outline for the boundary of a region of contiguous
 * opaque pixels starting at the given seed pixel.
 */
static AG_Surface *_Nullable
GenOutlineSurface(SG_Image *_Nonnull si, const AG_Surface *_Nonnull su,
    Uint8 *_Nonnull scanMask, int xSeed, int ySeed, Uint *_Nullable nPixels)
{
	M_PointSet2i psBack;		/* Backtrack buffer */
	AG_Surface *suOut;
	int x, y, dx, dy, i;
	AG_Color c;

	if ((suOut = AG_SurfaceNew(agSurfaceFmt, su->w, su->h, 0)) == NULL) {
		return (NULL);
	}
	AG_ColorWhite(&c);
	AG_FillRect(suOut, NULL, &c);

	M_PointSetInit2i(&psBack, 1.0, 1.0);
	M_PointSetAlloc2i(&psBack, 128);
	x = xSeed;
	y = ySeed;
	for (;;) {
loop_s1:
		/* Test the 8 neighboring pixels. */
		for (dx = -1; dx <= 1; dx++) {
			for (dy = -1; dy <= 1; dy++) {
				if ((dx == 0 && dy == 0) ||
				    PixelMasked(si, scanMask, x+dx, y+dy)) {
					continue;
				}
				MaskPixel(si, scanMask, x+dx, y+dy);
				AG_GetColor32(&c,
				    AG_SurfaceGet32(su, x+dx, y+dy),
				    &su->format);
				if (c.a > 0) {
					x += dx;
					y += dy;
					M_PointSetAdd2i(&psBack, x, y);
					goto loop_s1;
				} else {
					AG_SurfacePut32(suOut,
					    x+dx,
					    y+dy,
					    AG_MapPixel32_RGB8(&suOut->format, 0,0,0));
					if (nPixels) { (*nPixels)++; }
				}
			}
		}
		/* Backtrack until done. */
		for (i = psBack.n-1; i >= 0; i--) {
			x = psBack.x[i];
			y = psBack.y[i];
			for (dx = -1; dx <= 1; dx++) {
				for (dy = -1; dy <= 1; dy++) {
					if ((dx == 0 && dy == 0) ||
					    PixelMasked(si, scanMask, x+dx, y+dy)) {
						continue;
					}
					MaskPixel(si, scanMask, x+dx, y+dy);
					AG_GetColor32(&c,
					    AG_SurfaceGet32(su, x+dx, y+dy),
					    &su->format);
					if (c.a > 0) {
						x += dx;
						y += dy;
						goto loop_s1;
					} else {
						AG_SurfacePut32(suOut,
						    x+dx, y+dy,
						    AG_MapPixel32_RGB8(&suOut->format, 0,0,0));
						if (nPixels) { (*nPixels)++; }
					}
				}
			}
		}
		break;
	}

	M_PointSetFree2i(&psBack);
	return (suOut);
}

/*
 * From an outline surface, compute an ordered set of points describing
 * the contour. A random scan starts at the provided seed pixel x,y.
 *
 * Pixels in scanMask are ignored, and all pixels in the bounded region
 * are added to scanMask.
 */
static int
DetectContour(SG_Image *_Nonnull si, const AG_Surface *_Nonnull su,
    Uint8 *_Nonnull scanMask, int xSeed, int ySeed, M_PointSet2i *_Nonnull ps)
{
	AG_Color c;
	int i, dx, dy, x, y;

	M_PointSetInit2i(ps, (M_Real)si->su->w, (M_Real)si->su->h);
	M_PointSetAlloc2i(ps, 128);
	x = xSeed;
	y = ySeed;
loop:
	M_PointSetAdd2i(ps, x, y);
	for (dx = -1; dx <= 1; dx++) {
		for (dy = -1; dy <= 1; dy++) {
			if (dx == 0 && dy == 0) {
				continue;
			}
			if ((x+dx) == xSeed &&
			    (y+dy) == ySeed) {
				/* Back to seed pixel, done. */
				goto done;
			}
			if (PixelMasked(si, scanMask, x+dx, y+dy)) {
				continue;
			}
			MaskPixel(si, scanMask, x+dx, y+dy);
			AG_GetColor32(&c,
			    AG_SurfaceGet32(su, x+dx, y+dy),
			    &su->format);
			if (c.r == 0) {
				x += dx;
				y += dy;
				goto loop;
			}
		}
	}
	/* Backtrack until done. */
	for (i = ps->n-1; i >= 0; i--) {
		x = ps->x[i];
		y = ps->y[i];
		for (dx = -1; dx <= 1; dx++) {
			for (dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if ((x+dx) == xSeed &&
				    (y+dy) == ySeed) {
					/* Back to seed point; done. */
					goto done;
				}
				if (PixelMasked(si, scanMask, x+dx, y+dy)) {
					continue;
				}
				MaskPixel(si, scanMask, x+dx, y+dy);
				AG_GetColor32(&c,
				    AG_SurfaceGet32(su, x+dx, y+dy),
				    &su->format);
				if (c.r == 0) {
					x += dx;
					y += dy;
					goto loop;
				}
			}
		}
	}
	AG_SetError("Open contour (at %d,%d)", x, y);
	goto fail;
done:
	if (ps->n < 4) {
		AG_SetError("Degenerate contour (%u pts) at %d,%d", ps->n, x, y);
		goto fail;
	}
	return (0);
fail:
	M_PointSetFree2i(ps);
	return (-1);
}

/* Distance squared */
static __inline__ M_Real
Distance_2(M_Vector2 u, M_Vector2 v)
{
	M_Vector2 w = M_VecSub2(u,v);
	return M_VecDot2(w,w);
}

/* Douglas-Peucker simplification routine. */
static void
SimplifyContourDP(SG_Image *_Nonnull si, M_Vector2 *_Nonnull vt, int j, int k,
    int *_Nonnull mk, M_Real tol)
{
	int i, iMax = j;
	float d2Max, tol_2;
	M_Line2 S;
	M_Vector2 Sp0, Sp1;
	double cu;

	if (k <= j+1)
		return;

	d2Max = 0.0;
	tol_2 = tol*tol;
	S = M_LineFromPts2(vt[j], vt[k]);
	cu = M_VecDot2(S.d, S.d);
	M_LineToPts2(S, &Sp0, &Sp1);

	for (i = j+1; i < k; i++) {
		M_Vector2 w;
		M_Vector2 Pb;	/* Base of perpendicular from vt[1] to S */
		double b, cw, dv2;

		/* Compute distance squared */
		w = M_VecSub2(vt[i], Sp0);
		cw = M_VecDot2(w, S.d);
		if (cw <= 0.0) {
			dv2 = Distance_2(vt[i], Sp0);
		} else if (cu <= cw) {
			dv2 = Distance_2(vt[i], Sp1);
		} else {
			b = cw/cu;
			Pb = M_VecAdd2(Sp0, M_VecScale2(S.d, b));
			dv2 = Distance_2(vt[i], Pb);
		}
		/* Test with current max distance squared */
		if (dv2 <= d2Max) {
			continue;
		}
		/* vt[i] is a new max vertex */
		iMax = i;
		d2Max = dv2;
	}
	if (d2Max > tol_2) {		/* Error worse than tolerance */
		/*
		 * Split polyline at farthest vertex from S and recursively
		 * simplify the two subpolylines at vt[iMax].
		 */
		mk[iMax] = 1;
		SimplifyContourDP(si, vt, j, iMax, mk, tol);	/* at vt[j..iMax] */
		SimplifyContourDP(si, vt, iMax, k, mk, tol);	/* at vt[iMax..k] */
	}
}

/*
 * Convert the specified M_PointSet2i contour to a Douglas-Peucker
 * simplified Polygon (in real coordinates).
 */
static int
ContourToPoly(SG_Image *_Nonnull si, const M_PointSet2i *_Nonnull ps,
    M_Polygon *_Nonnull Pout, M_Real tol)
{
	int i, k, pv;
	float tol_2 = tol*tol;
	M_Vector2 *vt = NULL;				/* Vertex buffer */
	int *mk = NULL;					/* Marker buffer */
	M_Vector2 a, b;

	if ((vt = TryMalloc(ps->n*sizeof(M_Vector2))) == NULL)
		goto fail;
	if ((mk = TryMalloc(ps->n*sizeof(int))) == NULL) {
		goto fail;
	}
	memset(mk, 0, ps->n*sizeof(int));

	/*
	 * Trivial vertex reduction within tolerance of prior
	 * vertex cluster.
	 */
	vt[0].x = (M_Real)ps->x[0];			/* Start at first */
	vt[0].y = (M_Real)ps->y[0];
	for (i=1, k=1, pv=0;
	     i < ps->n;
	     i++) {
		a.x = (M_Real)ps->x[i];
		a.y = (M_Real)ps->y[i];
		b.x = (M_Real)ps->x[pv];
		b.y = (M_Real)ps->y[pv];
		if (Distance_2(a,b) < tol_2) {
			continue;
		}
		vt[k++] = a;
		pv = i;
	}
	if (pv < ps->n-1) {				/* Finish at last */
		a.x = (M_Real)ps->x[ps->n-1];
		a.y = (M_Real)ps->y[ps->n-1];
		vt[k++] = a;
	}

	/* Apply Douglas-Peucker polyline simplification */
	mk[0] = 1;
	mk[k-1] = 1;
	SimplifyContourDP(si, vt, 0, k-1, mk, tol);

	/* Copy marked vertices to the output set. */
#ifdef DEBUG_VERBOSE
	printf("%s: Douglas-Peucker reduction %d points -> %d points\n",
	    OBJECT(si)->name, ps->n, k);
#endif
	M_PolygonInit(Pout);
	for (i = 0; i < k; i++) {
		if (!mk[i]) {
			continue;
		}
		a.x = (vt[i].x/ps->w - 0.5)*si->w;
		a.y = (0.5 - vt[i].y/ps->h)*si->h;
		M_PolygonAddVertex(Pout, a);
	}
	if (Pout->n < 3) {
		M_PolygonFree(Pout);
		goto fail;
	}
	Free(vt);
	Free(mk);
	return (0);
fail:
	Free(vt);
	Free(mk);
	return (-1);
}

#ifdef DEBUG_DUMP
static __inline__ void
PutPixelDebug(AG_Surface *_Nonnull su, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	int i, j;

	for (i = -1; i < 1; i++) {
		for (j = -1; j < 1; j++) {
			AG_SurfacePut32(su, x+i, y+j,
			    AG_MapPixel32_RGB8(su->format, r,g,b));
		}
	}
}
static void
DumpPointSet(SG_Image *_Nonnull si, const M_PointSet2i *_Nonnull ps,
    const char *_Nonnull path)
{
	AG_Color c;
	AG_Surface *su;
	Uint i;

	su = AG_SurfaceNew(AG_SURFACE_PACKED, si->su->w, si->su->h,
	    &si->su.format, 0);
	AG_ColorWhite(&c);
	AG_FillRect(su, NULL, &c);
	for (i = 0; i < ps->n; i++) {
		AG_PUT_PIXEL2(su, ps->x[i], ps->y[i],
		    AG_MapPixelRGB(su->format, 0,0,0));
	}
	AG_SurfaceExportJPEG(su, path, 100, 0);
	AG_SurfaceFree(su);
}
#endif /* DEBUG_DUMP */

/*
 * Compute the holes inside of a contour. The hole outlines are returned as
 * polygons into the "holes" array. The su argument is the outline surface.
 */
static void
DetectHoles(SG_Image *_Nonnull si, const AG_Surface *_Nonnull su,
    M_PointSet2i *_Nonnull psContour, Uint8 *_Nonnull scanMask,
    int xSeed, int ySeed)
{
	M_PointSet2i psBack;
	M_Polygon *holesNew;
	M_Polygon Phole;
	M_PointSet2i cHole;
	int x, y, dx, dy;
	AG_Color c;
	int i;
#ifdef DEBUG_DUMP
	AG_Surface *suDbg;
	static Uint nDebugCholes = 0;
	char pdump[64];
#endif /* DEBUG_DUMP */

	FreeHoles(si);

#ifdef DEBUG_VERBOSE
	printf("%s: Begin Hole Detection\n", OBJECT(si)->name);
#endif
#ifdef DEBUG_DUMP
	suDbg = AG_SurfaceStdRGB(su->w, su->h);
	AG_ColorBlack(&c);
	AG_FillRect(suDbg, NULL, &c);
#endif

	/* Ignore the transparent pixels along the exterior contour */
	memset(scanMask, 0, su->w*su->h);
	for (i = 0; i < psContour->n; i++) {
		MaskPixel(si, scanMask,
		    psContour->x[i],
		    psContour->y[i]);
#ifdef DEBUG_DUMP
		AG_PUT_PIXEL2(suDbg, psContour->x[i], psContour->y[i],
		    AG_MapPixelRGB(suDbg->format, 0,150,0));
#endif
	}

	/* Allocate the backtrack buffer. */
	M_PointSetInit2i(&psBack, 1.0, 1.0);
	M_PointSetAlloc2i(&psBack, 32);

	/*
	 * Begin with a random search for a seed pixel from a hole
	 * outline.
	 */
	x = xSeed+1;
	y = ySeed+1;
#ifdef DEBUG_DUMP
	PutPixelDebug(suDbg, x,y, 255,255,255);
#endif
	for (;;) {
loop:
		for (dx = -1; dx <= 1; dx++) {
			for (dy = -1; dy <= 1; dy++) {
				if ((dx == 0 && dy == 0) ||
				    (abs(dx) == abs(dy)) ||
				    PixelMasked(si, scanMask, x+dx, y+dy)) {
					continue;
				}
				MaskPixel(si, scanMask, x+dx, y+dy);
#ifdef DEBUG_DUMP
				PutPixelDebug(suDbg, x+dx,y+dy, 80,80,80);
#endif
				if (AG_SurfaceClipped(su, x+dx, y+dy)) {
					c.r = 255;
				} else {
					AG_GetColor32(&c,
					    AG_SurfaceGet32(su, x+dx, y+dy),
					    &su->format);
				}
				if (c.r == 255) {		/* Keep going */
					x += dx;
					y += dy;
					M_PointSetAdd2i(&psBack, x, y);
					goto loop;
				}

				/* Transparent seed pixel found */
#ifdef DEBUG_DUMP
				PutPixelDebug(suDbg, x+dx,y+dy, 255,0,0);
#endif
				if (DetectContour(si, su, scanMask, x+dx, y+dy,
				    &cHole) == 0) {
#ifdef DEBUG_VERBOSE
					printf("%s: Hole @%d,%d: %d-pt path\n",
					    OBJECT(si)->name, x+dx, y+dy, cHole.n);
#endif
					if (ContourToPoly(si, &cHole, &Phole,
					    si->tolHoles) == 0) {
						holesNew = TryRealloc(si->holes,
						    (si->nHoles+1)*sizeof(M_Polygon));
						if (holesNew == NULL) { goto fail; }
						si->holes = holesNew;
						si->holes[si->nHoles++] = Phole;
					}
#ifdef DEBUG_DUMP
					Snprintf(pdump, sizeof(pdump), "/tmp/hole%u.jpg", nDebugCholes++);
					DumpPointSet(si, &cHole, pdump);
#endif
					M_PointSetFree2i(&cHole);
				} else {
#ifdef DEBUG_VERBOSE
					printf("%s: Ignoring hole at %d,%d (%s)\n",
					    OBJECT(si)->name, x+dx, y+dy,
					    AG_GetError());
#endif
					continue;
				}
			}
		}

		/*
		 * Backtrack until all neighboring pixels have been analyzed.
		 */
#ifdef DEBUG_DUMP
		PutPixelDebug(suDbg, x+dx,y+dy, 255,255,0);
#endif
		for (i = 0; i < psBack.n; i++) {
			x = psBack.x[i];
			y = psBack.y[i];
			for (dx = -1; dx <= 1; dx++) {
				for (dy = -1; dy <= 1; dy++) {
					if ((dx == 0 && dy == 0) ||
				    	    (abs(dx) == abs(dy)) ||
					    PixelMasked(si, scanMask, x+dx, y+dy)) {
						continue;
					}
					MaskPixel(si, scanMask, x+dx, y+dy);
#ifdef DEBUG_DUMP
					PutPixelDebug(suDbg, x+dx,y+dy, 60,60,60);
#endif
					if (AG_SurfaceClipped(su, x+dx, y+dy)) {
						c.r = 255;
					} else {
						AG_GetColor32(&c,
						    AG_SurfaceGet32(su,
						    x+dx, y+dy), &su->format);
					}
					if (c.r == 255) {	/* Keep going */
						x += dx;
						y += dy;
						M_PointSetAdd2i(&psBack, x, y);
						goto loop;
					}
				
					/* Transparent seed pixel found */
#ifdef DEBUG_DUMP
					PutPixelDebug(suDbg, x+dx,y+dy, 255,0,0);
#endif
					if ((DetectContour(si, su, scanMask,
					    x+dx, y+dy, &cHole)) == 0) {
#ifdef DEBUG_VERBOSE
						printf("%s: Hole@%d,%d: %d-pt path\n",
						    OBJECT(si)->name, x+dx, y+dy, cHole.n);
#endif
						if (ContourToPoly(si, &cHole,
						    &Phole, si->tolHoles) == 0) {
							holesNew = TryRealloc(si->holes,
							    (si->nHoles+1)*sizeof(M_Polygon));
							if (holesNew == NULL) { goto fail; }
							si->holes = holesNew;
							si->holes[si->nHoles++] = Phole;
						}
#ifdef DEBUG_DUMP
						Snprintf(pdump, sizeof(pdump), "/tmp/hole%u.jpg", nDebugCholes++);
						DumpPointSet(si, &cHole, pdump);
#endif
						M_PointSetFree2i(&cHole);
					} else {
#ifdef DEBUG_VERBOSE
						printf("%s: Ignoring hole at %d,%d (%s)\n",
						    OBJECT(si)->name, x+dx, y+dy,
						    AG_GetError());
#endif
						continue;
					}
				}
			}
		}
		break;
	}
#ifdef DEBUG_DUMP
	AG_SurfaceExportJPEG(suDbg, "/tmp/holedetection.jpg", 100, 0);
	AG_SurfaceFree(suDbg);
#endif
	M_PointSetFree2i(&psBack);
	return;
fail:
	M_PointSetFree2i(&psBack);
	FreeHoles(si);
}

/* Compile the display list for an opaque rectangular image. */
static SG_ViewList *_Nullable
CompileRect(SG_Image *_Nonnull si, SG_View *view)
{
	SG_ViewList *list;

	if ((list = TryMalloc(sizeof(SG_ViewList))) == NULL) {
		return (NULL);
	}
	list->node = si;
	list->sv = view;
	if ((list->name = GL_GenLists(1)) == 0 ||
	    GL_NewList(list->name, GL_COMPILE) == -1) {
		free(list);
		return (NULL);
	}

	/* Render the textured rectangle. */
	GL_Begin(GL_TRIANGLE_STRIP);
	{
		AG_TexCoord *tc = si->tc;
		M_Real w2 = si->w/2, h2 = si->h/2;

		GL_TexCoord2(tc->w, tc->h); GL_Vertex2( w2, -h2);
		GL_TexCoord2(0.0,   tc->h); GL_Vertex2(-w2, -h2);
		GL_TexCoord2(tc->w, 0.0);   GL_Vertex2( w2,  h2);
		GL_TexCoord2(0.0,   0.0);   GL_Vertex2(-w2,  h2);
	}
	GL_End();

	GL_EndList();
	return (list);
}

/* Upload a contour to the tesselator. */
static int
UploadContour(SG_Image *_Nonnull si, const M_Polygon *_Nonnull P, int dir)
{
	SG_GLVertex3 *tb, **tessBufsNew;
	int i;

	/* Allocate a new tesselator buffer. */
	if ((tessBufsNew = TryRealloc(si->tessBufs,
	    (si->nTessBufs+1)*sizeof(SG_GLVertex3 *))) == NULL) {
		return (-1);
	}
	if ((tb = TryMalloc(P->n*sizeof(SG_GLVertex3))) == NULL) {
		Free(tessBufsNew);
		return (-1);
	}
	si->tessBufs = tessBufsNew;
	si->tessBufs[si->nTessBufs++] = tb;

	gluTessBeginContour(si->to);
	if (dir == +1) {
		for (i = 0; i < P->n; i++) {
			tb[i].x = (GLdouble)P->v[i].x;
			tb[i].y = (GLdouble)P->v[i].y;
			tb[i].z = 0.0;
			gluTessVertex(si->to, (GLdouble *)&tb[i],
			    (GLdouble *)&tb[i]);
		}
	} else {
		for (i = (P->n - 1); i >= 0; i--) {
			tb[i].x = (GLdouble)P->v[i].x;
			tb[i].y = (GLdouble)P->v[i].y;
			tb[i].z = 0.0;
			gluTessVertex(si->to, (GLdouble *)&tb[i],
			    (GLdouble *)&tb[i]);
		}
	}
	gluTessEndContour(si->to);
	return (0);
}

/*
 * Compile the display list for a complex polygonal image. Update the
 * image's contour and hole polygons.
 */
static SG_ViewList *_Nullable
CompilePoly(SG_Image *_Nonnull si, SG_View *_Nonnull view)
{
	AG_Surface *su = si->su, *suOutline = NULL;
	Uint8 *scanMask = NULL, *pMask, *p;
	int i, x, y;
	AG_Color c;
	SG_ViewList *list;
	M_PointSet2i psContour = M_POINT_SET2I_EMPTY;
	Uint nPixels = 0;

	M_PolygonFree(&si->contour);
	FreeHoles(si);
	FreeTessBufs(si);

	/* Allocate return structure. */
	if ((list = TryMalloc(sizeof(SG_ViewList))) == NULL) {
		return (NULL);
	}
	list->node = si;
	list->sv = view;
	
	if ((scanMask = TryMalloc(su->w*su->h)) == NULL) {
		goto fail;
	}
	memset(scanMask, 0, su->w*su->h);

	/*
	 * Scan for a seed pixel from the outer contour.
	 */
	p = (Uint8 *)su->pixels;
	pMask = scanMask;
	for (y = 0; y < su->h; y++) {
		for (x = 0;
		     x < su->w;
		     x++, p += su->format.BytesPerPixel, pMask++) {
			if (*pMask) {
				continue;
			}
			AG_GetColor32(&c, AG_SurfaceGet32_At(su,p), &su->format);
			if (c.a != 0) {
				*pMask = 1;
				break;
			}
		}
		if (x < su->w)
			break;
	}
	if (y == su->h) { 			/* No seed pixel found */
		AG_SetError("No contour found");
		goto fail;
	}

	/*
	 * Generate an outline of the boundary between transparent and
	 * opaque pixels.
	 */
	if ((suOutline = GenOutlineSurface(si, su, scanMask, x, y, &nPixels))
	    == NULL)
		goto fail;
#ifdef DEBUG_DUMP
	AG_SurfaceExportJPEG(suOutline, "/tmp/outline.jpg", 100, 0);
#endif

	/*
	 * Detect the outer contour.
	 */
	memset(scanMask, 0, su->w*su->h);
	if (DetectContour(si, suOutline, scanMask, x, y, &psContour) == -1) {
		goto fail;
	}
#ifdef DEBUG_VERBOSE
	printf("%s: Contour at %d,%d: %d points\n", OBJECT(si)->name, x, y, psContour.n);
#endif
#ifdef DEBUG_DUMP
	DumpPointSet(si, &psContour, "/tmp/contour.jpg");
#endif
	/* Set up for the display list and tesselation. */
	if ((list->name = GL_GenLists(1)) == 0 ||
	    GL_NewList(list->name, GL_COMPILE) == -1) {
		goto fail;
	}
#if 0
	gluTessProperty(si->to, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
	gluTessProperty(si->to, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
#endif

	gluTessBeginPolygon(si->to, si);

	/* Specify the exterior contour. */
	M_PolygonFree(&si->contour);
	if (ContourToPoly(si, &psContour, &si->contour, si->tolContour) == -1 ||
	    UploadContour(si, &si->contour, -1) == -1)
		goto fail;

	/* Specify the interior hole contours. */
	if (si->flags & SG_IMAGE_HOLES) {
		DetectHoles(si, suOutline, &psContour, scanMask,
		    psContour.x[0], psContour.y[0]);
	}
	for (i = 0; i < si->nHoles; i++) {
		M_Polygon *Phole = &si->holes[i];
		SG_GLVertex3 **tessBufsNew;

#ifdef DEBUG_VERBOSE
		printf("%s: Detected Hole: %d pts (buffer #%d)\n",
		    OBJECT(si)->name, Phole->n, si->nTessBufs-1);
#endif
		if ((tessBufsNew = TryRealloc(si->tessBufs,
		    (si->nTessBufs+1)*sizeof(SG_GLVertex3 *))) == NULL) {
			goto fail;
		}
		if ((tessBufsNew[si->nTessBufs] =
		    TryMalloc(Phole->n*sizeof(SG_GLVertex3))) == NULL) {
			free(tessBufsNew);
			goto fail;
		}
		si->tessBufs = tessBufsNew;
		si->nTessBufs++;
		if (UploadContour(si, Phole, +1) == -1)
			goto fail;
#ifdef DEBUG_DUMP
		{
			static Uint nDebugHole = 0;
			char path[64];
			AG_Surface *suDbg;
			int j;

			suDbg = AG_SurfaceStdRGB(su->w, su->h);
			AG_ColorWhite(&c);
			AG_FillRect(suDbg, NULL, &c);
			for (j = 0; j < Phole->n; j++) {
				AG_PUT_PIXEL2(suDbg,
				    (int)Phole->v[j].x,
				    (int)Phole->v[j].y,
				    AG_MapPixelRGB(suDbg->format, 0,0,0));
			}
			Snprintf(path, sizeof(path), "/tmp/hole%u.jpg", nDebugHole++);
			AG_SurfaceExportJPEG(suDbg, path, 100, 0);
			AG_SurfaceFree(suDbg);
		}
#endif /* DEBUG_DUMP */
	}

	gluTessEndPolygon(si->to);
	GL_EndList();

	AG_SurfaceFree(suOutline);
	M_PointSetFree2i(&psContour);
	Free(scanMask);
	return (list);
fail:
#ifdef DEBUG_VERBOSE
	printf("%s: CompilePoly failed: %s\n", OBJECT(si)->name, AG_GetError());
#endif
	if (suOutline != NULL) {
		AG_SurfaceFree(suOutline);
	}
	M_PointSetFree2i(&psContour);
	Free(list);
	Free(scanMask);
	return (NULL);
}

static void
Draw(void *_Nonnull obj, SG_View *_Nonnull sv)
{
	SG_Image *si = obj;
	SG_ViewTexture *vt;
	SG_ViewList *vl;
	int lighting, depthTest;
	int i, j;

	if (si->an != NULL) {
		AG_MutexLock(&si->ast.lock);
	} else if (si->su == NULL) {
		return;
	}

	GL_PushAttrib(GL_POLYGON_BIT|GL_COLOR_BUFFER_BIT);
	
	GL_Enable(GL_ALPHA_TEST);
	GL_Enable(GL_BLEND);
	GL_BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GL_DisableSave(GL_LIGHTING, &lighting);
	GL_PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Fetch / upload texture */
	TAILQ_FOREACH(vt, &si->vtex, textures) {
		if (vt->sv == sv &&
		    (si->an == NULL || vt->frame == si->ast.f))
			break;
	}
	if (vt == NULL) {
		if ((vt = TryMalloc(sizeof(SG_ViewTexture))) == NULL) {
			goto out;
		}
		vt->node = si;
		vt->sv = sv;

		if (si->an != NULL) {
#if 0
			vt->frame = si->ast.f;
			if (si->su != NULL) {
				AG_SurfaceFree(si->su);
			}
			/* XXX TODO */
			if ((si->su = AG_AnimFrameToSurface(si->an, si->ast.f))
			    == NULL) {
				free(vt);
				goto out;
			}
#else
			vt->frame = -1;
#endif
		} else {
			vt->frame = -1;
		}

		AG_GL_UploadTexture(WIDGET(sv)->drv, &vt->name, si->su, &vt->tc);
		TAILQ_INSERT_TAIL(&si->vtex, vt, textures);
	}
	si->tc = &vt->tc;
	
	GL_MatrixMode(GL_TEXTURE);
	GL_PushMatrix();
	GL_LoadIdentity();

	GL_MatrixMode(GL_MODELVIEW);
	GL_PushMatrix();

	/* Camera-dependent billboard projection */
	if (si->flags & SG_IMAGE_BILLBOARD) {
		float M[16];

		glGetFloatv(GL_MODELVIEW_MATRIX, M);
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				if (i == j) {
					M[i*4 + j] = 1.0;
				} else {
					M[i*4 + j] = 0.0;
				}
			}
		}
		glLoadMatrixf(M);
	}

	/* Fetch / generate rectangle or polygon */
	TAILQ_FOREACH(vl, &si->vlist, lists) {
		if (vl->sv == sv &&
		    (si->an == NULL || vl->frame == si->ast.f))
			break;
	}
	if (vl == NULL) {
#ifdef DEBUG_VERBOSE
		printf("%s: Generate (f%d)\n", OBJECT(si)->name,
		    si->an != NULL ? si->ast.f : 0);
#endif
		switch (si->shape) {
		case SG_IMAGE_RECT:
			vl = CompileRect(si, sv);
			break;
		case SG_IMAGE_POLY:
			vl = CompilePoly(si, sv);
			break;
		}
		if (vl == NULL) {
			goto out;
		}
		vl->frame = (si->an != NULL) ? si->ast.f : -1;
		TAILQ_INSERT_TAIL(&si->vlist, vl, lists);
	}

	GL_BindTexture(GL_TEXTURE_2D, vt->name);
	GL_CallList(vl->name);
	GL_BindTexture(GL_TEXTURE_2D, 0);
	
	GL_DisableSave(GL_DEPTH_TEST, &depthTest);

	/* Overlay selection state / wireframe. */
	if (SGNODE(si)->flags & SG_NODE_SELECTED) {
		switch (si->shape) {
		case SG_IMAGE_RECT:
			{
				M_Real w2 = si->w/2;
				M_Real h2 = si->h/2;

				GL_Begin(GL_LINE_LOOP);
				GL_Color3ub(0, 255, 0);
				GL_Vertex2(-w2, -h2);
				GL_Vertex2(+w2, -h2);
				GL_Vertex2(+w2, +h2);
				GL_Vertex2(-w2, +h2);
				GL_End();
			}
			break;
		case SG_IMAGE_POLY:
			GL_Begin(GL_LINE_LOOP);
			GL_Color3ub(0, 255, 0);
			for (i = 0; i < si->contour.n; i++) {
				GL_Vertex2v(&si->contour.v[i]);
			}
			GL_End();
			break;
		}
	}
	if (si->flags & SG_IMAGE_WIREFRAME) {
		GL_PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		GL_Color4ub(0, 255, 0, 50);
		GL_CallList(vl->name);
	} else if ((SGNODE(si)->flags & SG_NODE_SELECTED)) {
		GL_PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		GL_Color4ub(0, 255, 0, 5);
		GL_CallList(vl->name);
	}
	GL_EnableSaved(GL_DEPTH_TEST, depthTest);
out:
	GL_PopMatrix();
	GL_MatrixMode(GL_TEXTURE);
	GL_PopMatrix();
	GL_MatrixMode(GL_MODELVIEW);

	GL_EnableSaved(GL_LIGHTING, lighting);
	GL_PopAttrib();
	
	if (si->an != NULL)
		AG_MutexUnlock(&si->ast.lock);
}

static int
IntersectLinePlane(M_Line3 ln, M_Vector3 Pnorm, M_Vector3 *_Nonnull X)
{
	M_Vector3 p1, p2, u;
	M_Real d, n, Si;

	M_LineToPts3(ln, &p1, &p2);
	u = M_VecSub3(p2, p1);
	d = M_VecDot3(Pnorm, u);
	n = -M_VecDot3(Pnorm, p1);
	if (Fabs(d) < SG_IMAGE_PRECISION) {
		return (0);
	}
	Si = n/d;
	if (Si < 0.0 || Si > 1.0) {
		return (0);
	}
	*X = M_VecAdd3(ln.p, M_VecScale3p(&u,Si));
	return (1);
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	SG_Image *si = obj;
	M_Geom3 xg;
	M_Vector3 X;

	switch (g.type) {
	case M_LINE:
		if (!IntersectLinePlane(g.g.line, M_VecK3(), &X)) {
			break;
		}
		switch (si->shape) {
		case SG_IMAGE_RECT:
			if (X.x >= -si->w/2 && X.x <= si->w/2 &&
			    X.y >= -si->h/2 && X.y <= si->h/2) {
				if (S != NULL) {
					xg.type = M_POINT;
					xg.g.point = X;
					M_GeomSetAdd3(S, &xg);
				}
				return (1);
			}
			break;
		case SG_IMAGE_POLY:
			if (M_PointInPolygon(&si->contour, M_VECTOR2(X.x,X.y))) {
				if (S != NULL) {
					xg.type = M_POINT;
					xg.g.point = X;
					M_GeomSetAdd3(S, &xg);
				}
				return (1);
			}
			break;
		}
		break;
	default:
		return (-1);
	}
	return (0);
}

static int
EditorAction(void *_Nonnull obj, enum sg_action_type type,
    SG_Action *_Nonnull act)
{
	SG_Image *si = obj;
	M_Vector3 v[3], X;
	SG_Widget *w, *wNext;
		
	if (!IntersectLinePlane(act->Rcur, M_VecK3(), &X)) {
		return (0);
	}
	switch (type) {
	case SG_ACTION_MOVE:
		act->act_move = M_VecSub3(X, act->vOrig);
		SGNODE(si)->T = act->Torig;
		SG_Translatev(si, act->act_move);
		break;
	case SG_ACTION_ZMOVE:
		v[0] = M_VecSub3(X, act->vOrig);
		v[1].x = 0;
		v[1].y = 0;
		v[1].z = v[0].x;
		SGNODE(si)->T = act->Torig;
		SG_Translatev(si, v[1]);
		act->act_move = v[1];
		break;
	case SG_ACTION_ROTATE_BEGIN:
		if ((w = SG_WidgetNew(si, SG_WIDGET_DISC, "rotate")) != NULL) {
			TAILQ_INSERT_TAIL(&act->widgets, w, widgets);
		}
		break;
	case SG_ACTION_ROTATE:
		v[0] = M_VecZero3();
		v[1] = M_VecNorm3(act->vOrig);
		v[2] = M_VecNorm3(X);
		act->act_rotate.axis =
		    M_VecNormCross3(M_VecSub3(v[0],v[1]),
		                    M_VecSub3(v[2],v[1]));
		act->act_rotate.theta = M_Acos(M_VecDot3(v[1], v[2]));
//		vCross = M_VecCross3(v[1], v[2]);
		SGNODE(si)->T = act->Torig;
//		SG_Translatev(si, act->vOrig);
		SG_Rotatev(si, act->act_rotate.theta, act->act_rotate.axis);
//		SG_Translatev(si, M_VecFlip3(act->vOrig));
		break;
	case SG_ACTION_SCALE_BEGIN:
		si->wOrig = si->w;
		si->hOrig = si->h;
		if ((w = SG_WidgetNew(si, SG_WIDGET_DISC, "scale0")) != NULL) {
			TAILQ_INSERT_TAIL(&act->widgets, w, widgets);
			SG_Translate(w, -(si->w / 2.0), -(si->h / 2.0), 0.0);
		}
		if ((w = SG_WidgetNew(si, SG_WIDGET_DISC, "scale1")) != NULL) {
			TAILQ_INSERT_TAIL(&act->widgets, w, widgets);
			SG_Translate(w, +(si->w / 2.0), +(si->h / 2.0), 0.0);
		}
		if ((w = SG_WidgetNew(si, SG_WIDGET_DISC, "scale2")) != NULL) {
			TAILQ_INSERT_TAIL(&act->widgets, w, widgets);
			SG_Translate(w, +(si->w / 2.0), -(si->h / 2.0), 0.0);
		}
		if ((w = SG_WidgetNew(si, SG_WIDGET_DISC, "scale3")) != NULL) {
			TAILQ_INSERT_TAIL(&act->widgets, w, widgets);
			SG_Translate(w, -(si->w / 2.0), +(si->h / 2.0), 0.0);
		}
		break;
	case SG_ACTION_SCALE:
		si->w = si->wOrig*X.x/act->vOrig.x;
		si->h = si->hOrig*X.y/act->vOrig.y;
		act->act_scale.x = (M_Real)si->w - si->wOrig;
		act->act_scale.y = (M_Real)si->h - si->hOrig;
		act->act_scale.z = 1.0;
		SG_ImageFreeCached(si);
		if ((w = AG_ObjectFindChild(si, "scale0")) != NULL) {
			SG_Identity(w);
			SG_Translate(w, -(si->w / 2.0), -(si->h / 2.0), 0.0);
		}
		if ((w = AG_ObjectFindChild(si, "scale1")) != NULL) {
			SG_Identity(w);
			SG_Translate(w, +(si->w / 2.0), +(si->h / 2.0), 0.0);
		}
		if ((w = AG_ObjectFindChild(si, "scale2")) != NULL) {
			SG_Identity(w);
			SG_Translate(w, +(si->w / 2.0), -(si->h / 2.0), 0.0);
		}
		if ((w = AG_ObjectFindChild(si, "scale3")) != NULL) {
			SG_Identity(w);
			SG_Translate(w, -(si->w / 2.0), +(si->h / 2.0), 0.0);
		}
		break;
	case SG_ACTION_MOVE_END:
	case SG_ACTION_ZMOVE_END:
	case SG_ACTION_ROTATE_END:
	case SG_ACTION_SCALE_END:
		for (w = TAILQ_FIRST(&act->widgets);
		     w != TAILQ_END(&act->widgets);
		     w = wNext) {
			wNext = TAILQ_NEXT(w, widgets);
			AG_ObjectDetach(w);
			AG_ObjectDestroy(w);
		}
		break;
	default:
		return (0);
	}
	return (1);
}

static int
ScriptAction(void *_Nonnull obj, SG_Action *_Nonnull act, int invert)
{
	SG_Image *si = obj;

	switch (act->type) {
	case SG_ACTION_MOVE:
	case SG_ACTION_ZMOVE:
		SG_Translatev(si,
		    invert ? M_VecFlip3(act->act_move) : act->act_move);
		break;
	case SG_ACTION_ROTATE:
		SG_Rotatev(si,
		    invert ? -act->act_rotate.theta : +act->act_rotate.theta,
		    act->act_rotate.axis);
		break;
	case SG_ACTION_SCALE:
		if (invert) {
			si->w -= act->act_scale.x;
			si->h -= act->act_scale.y;
		} else {
			si->w += act->act_scale.x;
			si->h += act->act_scale.y;
		}
		SG_ImageFreeCached(si);
		break;
	default:
		return (0);
	}
	return (1);
}

/* Copy a surface centered onto a 1-pixel larger surface. */
static void
CopySurfaceOffsetByOne(AG_Surface *_Nonnull ds, const AG_Surface *_Nonnull ss)
{
	const Uint8 *pSrc;
	Uint8 *pDst;
	AG_Color c;
	Uint32 zero;
	int x, y;

	AG_ColorNone(&c);
	zero = AG_MapPixel32(&ds->format, &c);

	pSrc = ss->pixels;
	pDst = ds->pixels;

	for (x = 0; x < ds->w; x++) {
		AG_SurfacePut32_At(ds, pDst, zero);
		pDst += ds->format.BytesPerPixel;
	}
	for (y = 0; y < ss->h; y++) {
		AG_SurfacePut32_At(ds, pDst, zero);
		pDst += ds->format.BytesPerPixel;

		memcpy(pDst, pSrc, ss->pitch);
		pDst += ds->pitch - (ds->format.BytesPerPixel << 1);
		pSrc += ss->pitch;
		
		AG_SurfacePut32_At(ds, pDst, zero);
		pDst += ds->format.BytesPerPixel;
	}
	for (x = 0; x < ds->w; x++) {
		AG_SurfacePut32_At(ds, pDst, zero);
		pDst += ds->format.BytesPerPixel;
	}
}

/*
 * Set the image surface (surface is duplicated).
 * The source surface must be locked.
 */
int
SG_ImageSetSurface(void *obj, const AG_Surface *su, M_Real scale)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);

	if (su->w < 2 || su->h < 2) {
		AG_SetError("Surface too small");
		goto fail;
	}

	SG_ImageFreeSurfaces(si);
	si->flags &= ~(SG_IMAGE_NODUP);
	if ((si->su = AG_SurfaceRGBA(su->w+2, su->h+2,
	    su->format.BitsPerPixel, 0,
	    su->format.Rmask,
	    su->format.Gmask,
	    su->format.Bmask,
	    su->format.Amask)) == NULL) {
		goto fail;
	}

	si->w = (M_Real)su->w*scale;
	si->h = (M_Real)su->h*scale;

	CopySurfaceOffsetByOne(si->su, su);

	AG_ObjectUnlock(si);
	return (0);
fail:
	AG_ObjectUnlock(si);
	return (-1);
}

/* Set the image surface (surface is reused). */
int
SG_ImageSetSurfaceNODUP(void *obj, AG_Surface *su, M_Real scale)
{
	SG_Image *si = obj;
	
	if (su->w < 2 || su->h < 2) {
		AG_SetError("Surface too small");
		return (-1);
	}

	AG_ObjectLock(si);

	SG_ImageFreeSurfaces(si);
	si->flags |= SG_IMAGE_NODUP;
	si->su = su;
	si->w = (M_Real)su->w*scale;
	si->h = (M_Real)su->h*scale;

	AG_ObjectUnlock(si);
	return (0);
}

/*
 * Set the SG_Image's animation (animation is duplicated).
 * The source animation must be locked.
 */
int
SG_ImageSetAnim(void *obj, const AG_Surface *an, M_Real scale)
{
	SG_Image *si = obj;
	
	AG_ObjectLock(si);

	if (an->w < 2 || an->h < 2) {
		AG_SetError("Anim too small");
		goto fail;
	}
	SG_ImageFreeSurfaces(si);
	si->flags &= ~(SG_IMAGE_NODUP_ANIM);
	if ((si->an = AG_SurfaceDup(an)) == NULL) {
		goto fail;
	}
	si->w = (M_Real)an->w*scale;
	si->h = (M_Real)an->h*scale;
	AG_AnimStateInit(&si->ast, si->an);

	AG_ObjectUnlock(si);
	return (0);
fail:
	AG_ObjectUnlock(si);
	return (-1);
}

/* Set the image animation (animation is reused). */
int
SG_ImageSetAnimNODUP(void *obj, AG_Surface *an, M_Real scale)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);

	if (an->w < 2 || an->h < 2) {
		AG_SetError("Anim too small");
		goto fail;
	}
	SG_ImageFreeSurfaces(si);
	si->flags |= SG_IMAGE_NODUP_ANIM;
	si->an = an;
	si->w = (M_Real)an->w*scale;
	si->h = (M_Real)an->h*scale;
	AG_AnimStateInit(&si->ast, si->an);

	AG_ObjectUnlock(si);
	return (0);
fail:
	AG_ObjectUnlock(si);
	return (-1);
}

/* Change the image rendering mode. */
void
SG_ImageSetShape(void *obj, enum sg_image_shape shape)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);
	si->shape = shape;
	AG_ObjectUnlock(si);
}

/* Auto-select the rendering mode based on image contents. */
void
SG_ImageSetShapeAuto(void *obj)
{
	SG_Image *si = obj;
	Uint8 *pDst;
	AG_Color c;
	int x, y;

	AG_ObjectLock(si);
	if (si->su == NULL) {
		goto out;
	}
	si->shape = SG_IMAGE_RECT;
	pDst = si->su->pixels;
	for (y = 0; y < si->su->h; y++) {
		for (x = 0; x < si->su->w; x++) {
			AG_GetColor32(&c, AG_SurfaceGet32_At(si->su,pDst),
			    &si->su->format);
			if (c.a < AG_OPAQUE) {
				si->shape = SG_IMAGE_POLY;
				break;
			}
			pDst += si->su->format.BytesPerPixel;
		}
		if (x < si->su->w)
			break;
	}
out:
	AG_ObjectUnlock(si);
}

/* Toggle the image projection mode. */
void
SG_ImageSetProj(void *obj, int enable)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);
	AG_SETFLAGS(si->flags, SG_IMAGE_BILLBOARD, enable);
	AG_ObjectUnlock(si);
}

void
SG_ImageAnimPlay(void *obj)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);
	if (si->an != NULL) {
		(void)AG_AnimPlay(&si->ast);
	}
	AG_ObjectUnlock(si);
}

void
SG_ImageAnimStop(void *obj)
{
	SG_Image *si = obj;

	AG_ObjectLock(si);
	if (si->an != NULL) {
		(void)AG_AnimStop(&si->ast);
	}
	AG_ObjectUnlock(si);
}

SG_NodeClass sgImageClass = {
	{
		"SG_Node:SG_Image",
		sizeof(SG_Image),
		{ 0,0 },
		Init,
		Reset,
		Destroy,
		Load,
		Save,
		SG_NodeEdit
	},
	NULL,			/* menuInstance */
	NULL,			/* menuClass */
	Draw,
	Intersect,
	Edit,
	EditorAction,
	ScriptAction
};
