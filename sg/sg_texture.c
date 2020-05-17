/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * High-level texture object.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/gui/opengl.h>

/* Create a new texture object. */
SG_Texture *
SG_TextureNew(void *parent, const char *name)
{
	SG_Texture *tex;

	tex = Malloc(sizeof(SG_Texture));
	AG_ObjectInit(tex, &sgTextureClass);
	if (name) {
		AG_ObjectSetNameS(tex, name);
	} else {
		OBJECT(tex)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, tex);
	return (tex);
}

static void
Init(void *_Nonnull obj)
{
	SG_Texture *tex = obj;

	tex->flags = 0;
	tex->w = 0;
	tex->h = 0;
	tex->shininess = 0.0;
	tex->emissive = M_ColorGray(0.5);
	tex->ambient = M_ColorGray(0.2);
	tex->diffuse = M_ColorGray(0.8);
	tex->specular = M_ColorBlack();
	TAILQ_INIT(&tex->progs);
	tex->nProgs = 0;
	tex->surface = NULL;
	tex->nSurfaces = 0;
	TAILQ_INIT(&tex->surfaces);
	TAILQ_INIT(&tex->vtex);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Texture *tex = obj;
	SG_TextureProgram *tp, *tpNext;
	SG_TextureSurface *tsu, *tsuNext;

	for (tp = TAILQ_FIRST(&tex->progs);
	     tp != TAILQ_END(&tex->progs);
	     tp = tpNext) {
		tpNext = TAILQ_NEXT(tp, programs);
		Free(tp);
	}
	TAILQ_INIT(&tex->progs);
	tex->nProgs = 0;

	for (tsu = TAILQ_FIRST(&tex->surfaces);
	     tsu != TAILQ_END(&tex->surfaces);
	     tsu = tsuNext) {
		tsuNext = TAILQ_NEXT(tsu, surfaces);
		if (!(tsu->flags & SG_TEXTURE_SURFACE_NODUP)) {
			AG_SurfaceFree(tsu->su);
		}
		Free(tsu);
	}
	TAILQ_INIT(&tex->surfaces);
	tex->nSurfaces = 0;

	AG_SurfaceFree(tex->surface);
	tex->surface = NULL;
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Texture *tex = obj;
	int i, count;

	tex->flags &= ~(SG_TEXTURE_SAVED);
	tex->flags |= (AG_ReadUint32(ds) & SG_TEXTURE_SAVED);
	tex->w = (Uint)AG_ReadUint32(ds);
	tex->h = (Uint)AG_ReadUint32(ds);
	tex->shininess = M_ReadReal(ds);
	tex->emissive = M_ReadColor(ds);
	tex->ambient = M_ReadColor(ds);
	tex->diffuse = M_ReadColor(ds);
	tex->specular = M_ReadColor(ds);

	/* Load program references. */
	if ((count = (int)AG_ReadUint8(ds)) > SG_TEXTURE_PROGS_MAX) {
		AG_SetError("Bad program count");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		SG_TextureProgram *tp;

		tp = SG_TextureAddProgram(tex, NULL);
		AG_CopyString(tp->progName, ds, sizeof(tp->progName));
	}

	/* Load surfaces. */
	if ((count = (int)AG_ReadUint8(ds)) > SG_TEXTURE_SURFACES_MAX) {
		AG_SetError("Bad surface count");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		AG_Surface *su;
		SG_TextureSurface *tsu;
		Uint flags;
	
		flags = (Uint)AG_ReadUint8(ds);
		if ((su = AG_ReadSurface(ds)) == NULL) {
			AG_SetError("Reading surface %d: %s", i, AG_GetError());
			return (-1);
		}
		if ((tsu = SG_TextureAddSurface(tex, su)) == NULL) {
			AG_SurfaceFree(su);
			return (-1);
		}
		AG_SurfaceFree(su);
		
		tsu->flags &= ~(SG_TEXTURE_SURFACE_SAVED|
		                SG_TEXTURE_SURFACE_NODUP);
		tsu->flags |= flags;
		AG_ReadRect(&tsu->rSrc, ds);
		AG_ReadRect(&tsu->rDst, ds);
	}

	return SG_TextureCompile(tex);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Texture *tex = obj;
	SG_TextureProgram *tp;
	SG_TextureSurface *tsu;

	AG_WriteUint32(ds, tex->flags & SG_TEXTURE_SAVED);
	AG_WriteUint32(ds, (Uint32)tex->w);
	AG_WriteUint32(ds, (Uint32)tex->h);
	M_WriteReal(ds, tex->shininess);
	M_WriteColor(ds, &tex->emissive);
	M_WriteColor(ds, &tex->ambient);
	M_WriteColor(ds, &tex->diffuse);
	M_WriteColor(ds, &tex->specular);

	AG_WriteUint8(ds, (Uint32)tex->nProgs);
	TAILQ_FOREACH(tp, &tex->progs, programs) {
		AG_WriteUint8(ds, (Uint8)tp->flags & SG_TEXTURE_PROGRAM_SAVED);
		AG_WriteString(ds, OBJECT(tp->prog)->name);
	}
	
	AG_WriteUint16(ds, (Uint16)tex->nSurfaces);
	TAILQ_FOREACH(tsu, &tex->surfaces, surfaces) {
		AG_WriteUint8(ds, (Uint8)tsu->flags & SG_TEXTURE_SURFACE_SAVED);
		AG_WriteSurface(ds, tsu->su);
		AG_WriteRect(ds, &tsu->rSrc);
		AG_WriteRect(ds, &tsu->rDst);
	}
	return (0);
}

static void
SelectColor(AG_Event *_Nonnull event)
{
	AG_HSVPal *pal = AG_HSVPAL_PTR(1);
	void *color = AG_PTR(2);
	AG_Mutex *lock = AG_PTR(3);

	M_BindRealMp(pal, "RGBAv", color, lock);
}

static void
PollSurfaces(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_Texture *tex = SG_TEXTURE_PTR(1);
	AG_TlistItem *it;
	SG_TextureSurface *tsu;
	int n = 0;

	AG_TlistClear(tl);
	AG_ObjectLock(tex);
	TAILQ_FOREACH(tsu, &tex->surfaces, surfaces) {
		it = AG_TlistAdd(tl, NULL, "Surface %d\n(%dx%dx%d)", n++,
		    tsu->su->w, tsu->su->h,
		    tsu->su->format.BitsPerPixel);
		it->p1 = tsu;
		it->cat = "texture-surface";
		AG_TlistSetIcon(tl, it, tsu->su);
	}
	AG_ObjectUnlock(tex);
	AG_TlistRestore(tl);
}

/* Add a new surface to a texture object. */
SG_TextureSurface *
SG_TextureAddSurface(SG_Texture *tex, const AG_Surface *su)
{
	SG_TextureSurface *tsu;

	if ((tex->nSurfaces+1) > SG_TEXTURE_SURFACES_MAX) {
		AG_SetError("Too many surfaces");
		return (NULL);
	}
	if ((tsu = TryMalloc(sizeof(SG_TextureSurface))) == NULL) {
		return (NULL);
	}
	if ((tsu->su = AG_SurfaceNew(agSurfaceFmt, su->w, su->h, 0)) == NULL) {
		Free(tsu);
		return (NULL);
	}
	AG_SurfaceCopy(tsu->su, su);
	tsu->flags = 0;

	tsu->rSrc.x = 0;
	tsu->rSrc.y = 0;
	tsu->rSrc.w = su->w;
	tsu->rSrc.h = su->h;
	tsu->rDst = tsu->rSrc;

	if (tex->w == 0 || tex->h == 0) {
		tex->w = su->w;
		tex->h = su->h;
	}

	AG_ObjectLock(tex);
	TAILQ_INSERT_TAIL(&tex->surfaces, tsu, surfaces);
	tex->nSurfaces++;
	AG_ObjectUnlock(tex);
	return (tsu);
}

/*
 * Add a new surface to a texture object (NODUP variant).
 * As the surface is referenced directly, it must have been created
 * with AG_SurfaceStdRGBA() and must remain valid.
 */
SG_TextureSurface *
SG_TextureAddSurfaceNODUP(SG_Texture *tex, AG_Surface *su)
{
	SG_TextureSurface *tsu;

	if (!(su->flags & AG_SURFACE_GL_TEXTURE)) {
		AG_SetError("Surface is not a GL texture");
		return (NULL);
	}
	if ((tex->nSurfaces+1) > SG_TEXTURE_SURFACES_MAX) {
		AG_SetError("Too many surfaces");
		return (NULL);
	}
	if ((tsu = TryMalloc(sizeof(SG_TextureSurface))) == NULL) {
		return (NULL);
	}
	tsu->su = su;
	tsu->flags = SG_TEXTURE_SURFACE_NODUP;
	tsu->rSrc.x = 0;
	tsu->rSrc.y = 0;
	tsu->rSrc.w = su->w;
	tsu->rSrc.h = su->h;
	tsu->rDst = tsu->rSrc;

	AG_ObjectLock(tex);
	TAILQ_INSERT_TAIL(&tex->surfaces, tsu, surfaces);
	tex->nSurfaces++;
	AG_ObjectUnlock(tex);
	return (tsu);
}

/* Remove the specified surface from texture object. */
void
SG_TextureDelSurface(SG_Texture *tex, SG_TextureSurface *tsu)
{
	AG_ObjectLock(tex);
	TAILQ_REMOVE(&tex->surfaces, tsu, surfaces);
	tex->nSurfaces--;
	AG_ObjectUnlock(tex);

	AG_SurfaceFree(tsu->su);
	Free(tsu);
}

/* Attach a new fragment shader to a texture object. */
SG_TextureProgram *
SG_TextureAddProgram(SG_Texture *tex, SG_Program *prog)
{
	SG_TextureProgram *tp;

	if ((tex->nProgs+1) > SG_TEXTURE_PROGS_MAX) {
		AG_SetError("Too many programs");
		return (NULL);
	}
	if ((tp = TryMalloc(sizeof(SG_TextureProgram))) == NULL) {
		return (NULL);
	}
	tp->flags = 0;
	tp->progName[0] = '\0';
	tp->prog = prog;

	AG_ObjectLock(tex);
	TAILQ_INSERT_TAIL(&tex->progs, tp, programs);
	tex->nProgs++;
	AG_ObjectUnlock(tex);
	return (tp);
}

/* Detach a fragment shader from a texture object. */
void
SG_TextureDelProgram(SG_Texture *tex, SG_TextureProgram *tp)
{
	AG_ObjectLock(tex);
	TAILQ_REMOVE(&tex->progs, tp, programs);
	tex->nProgs--;
	AG_ObjectUnlock(tex);

	Free(tp);
}

/* Import texture surface from file. */
static void
ImportSurface(AG_Event *_Nonnull event)
{
	SG_Texture *tex = SG_TEXTURE_PTR(1);
	const char *path = AG_STRING(2);
	AG_Surface *s;
	SG_TextureSurface *tsu;

	if ((s = AG_SurfaceFromFile(path)) == NULL) {
		AG_TextMsgFromError();
		return;
	}

	AG_ObjectLock(tex);
	if ((tsu = SG_TextureAddSurface(tex, s)) == NULL) {
		AG_TextMsgFromError();
		AG_SurfaceFree(s);
	}
	SG_TextureCompile(tex);
	AG_ObjectUnlock(tex);
}

static void
PreviewTexture(AG_Event *_Nonnull event)
{
	AG_Pixmap *px = AG_PIXMAP_PTR(1);
	const char *path = AG_STRING(2);
	AG_Surface *suFile, *suScaled = NULL;

	if ((suFile = AG_SurfaceFromFile(path)) == NULL) {
		AG_TextColor(&WCOLOR(px, TEXT_COLOR));
		AG_PixmapReplaceSurface(px, px->n, AG_TextRender(AG_GetError()));
		return;
	}
	if ((suScaled = AG_SurfaceScale(suFile, 196, 128, 0)) == NULL) {
		return;
	}
	AG_PixmapReplaceSurface(px, px->n, suScaled);
	AG_SurfaceFree(suFile);
}

static void
ImportSurfaceDlg(AG_Event *_Nonnull event)
{
	SG_Texture *tex = SG_TEXTURE_PTR(2);
	AG_Window *win;
	AG_FileDlg *fd;
	AG_Box *hBox;
	AG_Pixmap *px;

	if ((win = AG_WindowNew(AG_WINDOW_MODAL)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("%s: Import surface"), OBJECT(tex)->name);

	hBox = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_EXPAND);
	{
		fd = AG_FileDlgNewMRU(hBox, "sg-textures",
		    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);

		AG_FileDlgAddType(fd, _("JPEG Image"), "*.jpg,*.jpeg",
		    ImportSurface, "%p", tex);
		AG_FileDlgAddType(fd, _("Portable Network Graphics"), "*.png",
		    ImportSurface, "%p", tex);
		AG_FileDlgAddType(fd, _("PC bitmap"), "*.bmp",
		    ImportSurface, "%p", tex);

		px = AG_PixmapNew(hBox, 0, 196, 128);
		AG_SetEvent(fd, "file-selected", PreviewTexture, "%p", px);
	}

	AG_WindowShow(win);
}

static void
AttachProgram(AG_Event *_Nonnull event)
{
	AG_ObjectSelector *os = AG_OBJECTSELECTOR_PTR(1);
	SG_Texture *tex = SG_TEXTURE_PTR(2);
	SG_Program *prog = AG_GetPointer(os,"object");
	SG_TextureProgram *tp;

	AG_ObjectLock(tex);
	TAILQ_FOREACH(tp, &tex->progs, programs) {
		if (tp->prog == prog)
			break;
	}
	if (tp == NULL) {
		SG_TextureAddProgram(tex, prog);
	}
	AG_ObjectUnlock(tex);
}

static void
PollPrograms(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_Texture *tex = SG_TEXTURE_PTR(1);
	SG_TextureProgram  *tp;
	
	AG_TlistBegin(tl);
	AG_ObjectLock(tex);
	TAILQ_FOREACH(tp, &tex->progs, programs) {
		AG_TlistAddPtr(tl, sgIconCgProgram.s, OBJECT(tp)->name,
		    tp->prog);
	}
	AG_ObjectUnlock(tex);
	AG_TlistEnd(tl);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	SG_Texture *tex = obj;
	AG_Mutex *lock = &OBJECT(tex)->lock;
	AG_Window *win;
	AG_Pane *paHoriz;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Numerical *num;
	SG *sgPre;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Texture <%s>"), OBJECT(tex)->name);

	paHoriz = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	{
		SG_Polybox *cube;
/*		SG_Ball *ball ; */
		SG_Camera *cam;
		AG_Combo *com;

		sgPre = SG_New(NULL, "Preview", 0);
		SG_ViewNew(paHoriz->div[1], sgPre, SG_VIEW_EXPAND);

		cube = SG_PolyboxNew(sgPre->root, "Box");
		SG_ObjectSetTexture(cube, tex);
		
//		ball = SG_BallNew(sgPre->root, "Ball");
//		SG_ObjectSetTexture(ball, tex);

		if ((cam = sgPre->def.cam) != NULL) {
			cam->flags |= SG_CAMERA_ROT_I;
			cam->flags |= SG_CAMERA_ROT_J;
			cam->flags |= SG_CAMERA_ROT_K;
		}

		com = AG_ComboNew(paHoriz->div[1], AG_COMBO_HFILL,
		    _("Preview: "));
		AG_TlistAdd(com->list, NULL, "Cube");
		AG_TlistAdd(com->list, NULL, "Ball");

		AG_ComboSelectText(com, "Cube");
	}

	nb = AG_NotebookNew(paHoriz->div[0], AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Surfaces"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_MenuItem *mi;

		tl = AG_TlistNewPolled(ntab, AG_TLIST_EXPAND,
		    PollSurfaces, "%p", tex);
		AG_TlistSetItemHeight(tl, 64);
		AG_TlistSetIconWidth(tl, 64);
		AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXXXXXXXXXX (00x00)", 4);

		mi = AG_TlistSetPopup(tl, "texture-surface");
		{
			/* XXX */
			AG_MenuAction(mi, _("Edit surface..."), NULL,
			    NULL, "%p,%p,%p", win, tex, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Duplicate surface"), NULL,
			    NULL, "%p,%p", tex, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Delete surface"), agIconTrash.s,
			    NULL, "%p,%p", tex, tl);
		}
		AG_ButtonNewFn(ntab, AG_BUTTON_HFILL, _("Import surface..."),
		    ImportSurfaceDlg, "%p,%p", win, tex);
	}

	ntab = AG_NotebookAdd(nb, _("Fixed Color"), AG_BOX_VERT);
	{
		AG_Toolbar *bar;
		AG_HSVPal *pal;
		
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		M_BindRealMp(pal, "RGBAv", (void *)&tex->ambient, lock);

		bar = AG_ToolbarNew(ntab, AG_TOOLBAR_HORIZ, 1,
		    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);
		{
			AG_ToolbarButton(bar, _("Ambient"), 1,
			    SelectColor, "%p,%p,%p", pal, &tex->ambient, lock);
			AG_ToolbarButton(bar, _("Diffuse"), 0,
			    SelectColor, "%p,%p,%p", pal, &tex->diffuse, lock);
			AG_ToolbarButton(bar, _("Specular"), 0,
			    SelectColor, "%p,%p,%p", pal, &tex->specular, lock);
			AG_ToolbarButton(bar, _("Emissive"), 0,
			    SelectColor, "%p,%p,%p", pal, &tex->emissive, lock);
		}

		num = AG_NumericalNew(ntab, 0, NULL, _("Shininess: "));
		M_BindRealMp(num, "value", &tex->shininess, lock);
	}

	ntab = AG_NotebookAdd(nb, _("Shaders"), AG_BOX_VERT);
	{
		AG_ObjectSelector *os;
		AG_Box *hBox;

		hBox = AG_BoxNewHoriz(ntab, AG_BOX_HFILL);
		{
			os = AG_ObjectSelectorNew(hBox, AG_OBJSEL_PAGE_DATA,
			    tex, AGOBJECT(tex)->root,
			    _("Attach Shader Program: "));
			AG_ObjectSelectorMaskType(os, "SG_Program:*");
			AG_ButtonNewFn(hBox, AG_BUTTON_VFILL, _("OK"),
			    AttachProgram, "%p", os, tex);
		}
		AG_TlistNewPolled(ntab, AG_TLIST_EXPAND,
		    PollPrograms, "%p", tex);
	}

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 40, 40);
	return (win);
}

/* Compile the final texture surface from the input surfaces. */
int
SG_TextureCompile(SG_Texture *tex)
{
	SG_TextureSurface *tsu;
	AG_Color c;

	if (tex->surface != NULL) {
		AG_SurfaceFree(tex->surface);
	}
	if ((tex->surface = AG_SurfaceStdRGBA(tex->w, tex->h)) == NULL) {
		return (-1);
	}
	AG_ColorRGB(&c,
	    (AG_Component)(tex->ambient.r * AG_COLOR_LASTD),
	    (AG_Component)(tex->ambient.g * AG_COLOR_LASTD),
	    (AG_Component)(tex->ambient.b * AG_COLOR_LASTD));
	AG_FillRect(tex->surface, NULL, &c);

	TAILQ_FOREACH(tsu, &tex->surfaces, surfaces) {
		if (tsu->flags & SG_TEXTURE_SURFACE_SUPPRESS) {
			continue;
		}
		if (tsu->rDst.w != tsu->su->w ||
		    tsu->rDst.h != tsu->su->h) {
			AG_Surface *suScaled = NULL;

			suScaled = AG_SurfaceScale(tsu->su,
			    tsu->rDst.w,
			    tsu->rDst.h, 0);
			if (suScaled == NULL) {
				return (-1);
			}
			AG_SurfaceBlit(suScaled, &tsu->rSrc,
			    tex->surface,
			    tsu->rDst.x, tsu->rDst.y);
			AG_SurfaceFree(suScaled);
		} else {
			AG_SurfaceBlit(tsu->su, &tsu->rSrc,
			    tex->surface,
			    tsu->rDst.x, tsu->rDst.y);
		}
	}
	return (0);
}

/*
 * Set up the texture for rendering an object.
 * Must be called from GUI rendering context.
 */
void
SG_TextureBind(SG_Texture *tex, SG_View *view)
{
	AG_Driver *drv = WIDGET(view)->drv;
	SG_TextureProgram *tp;
	SG_ViewTexture *vt;

	AG_ObjectLock(tex);

	GL_MaterialColorv(GL_FRONT, GL_EMISSION, &tex->emissive);
	GL_MaterialColorv(GL_FRONT, GL_AMBIENT, &tex->ambient);
	GL_MaterialColorv(GL_FRONT, GL_DIFFUSE, &tex->diffuse);
	GL_MaterialColorv(GL_FRONT, GL_SPECULAR, &tex->specular);
	GL_Material(GL_FRONT, GL_SHININESS, tex->shininess);
	
	/* Bind texture surfaces */
	if (tex->nSurfaces > 0) {
		if (tex->surface == NULL &&
		    SG_TextureCompile(tex) == -1) {
			goto fail;
		}
		TAILQ_FOREACH(vt, &tex->vtex, textures) {
			if (vt->sv == view)
				break;
		}
		if (vt == NULL) {
			vt = Malloc(sizeof(SG_ViewTexture));
			vt->sv = view;
			AG_GL_UploadTexture(drv, &vt->name, tex->surface, NULL);
			vt->frame = -1;
			TAILQ_INSERT_TAIL(&tex->vtex, vt, textures);
		}

		AGDRIVER_CLASS(drv)->pushBlendingMode(drv, AG_ALPHA_SRC,
		                                           AG_ALPHA_ONE_MINUS_SRC);

		glBindTexture(GL_TEXTURE_2D, vt->name);
	}

	/* Bind associated fragment shaders. */
	TAILQ_FOREACH(tp, &tex->progs, programs) {
		if (tp->flags & SG_TEXTURE_PROGRAM_SUPPRESS) {
			continue;
		}
		if (tp->prog == NULL) {
			AG_Object *parent;
			SG_Program *prog;

			if ((parent = AG_ObjectParent(tex)) != NULL &&
			    (prog = AG_ObjectFindChild(parent, tp->progName)) != NULL) {
				tp->prog = prog;
			} else {
				Verbose("%s: No such program\n", tp->progName);
				continue;
			}
		}
		SG_ProgramBind(tp->prog, view);
	}
	AG_ObjectUnlock(tex);
	return;
fail:
	Verbose("SG_TextureBind: %s\n", AG_GetError());
	AG_ObjectUnlock(tex);
}

/*
 * Cleanup after rendering an object.
 * Must be called from widget draw context.
 */
void
SG_TextureUnbind(SG_Texture *tex, SG_View *view)
{
	AG_Driver *drv = WIDGET(view)->drv;
	SG_ViewTexture *vt;
	SG_TextureProgram *tp;
	
	AG_ObjectLock(tex);

	TAILQ_FOREACH(tp, &tex->progs, programs) {
		if (tp->flags & SG_TEXTURE_PROGRAM_SUPPRESS) {
			continue;
		}
		SG_ProgramUnbind(tp->prog, view);
	}
	if (tex->nSurfaces > 0) {
		TAILQ_FOREACH(vt, &tex->vtex, textures) {
			if (vt->sv == view)
				break;
		}
		if (vt != NULL)
			glBindTexture(GL_TEXTURE_2D, 0);

		AGDRIVER_CLASS(drv)->popBlendingMode(drv);
	}

	AG_ObjectUnlock(tex);
}

AG_ObjectClass sgTextureClass = {
	"SG_Texture",
	sizeof(SG_Texture),
	{ 0,0 },
	Init,
	Reset,
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};
