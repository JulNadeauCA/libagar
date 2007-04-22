/*
 * Copyright (c) 2006-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/core/math.h>

#include "sg.h"
#include "sg_gui.h"

#include <GL/gl.h>
#if 0
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

const AG_ObjectOps sgMaterialOps = {
	"SG_Material",
	sizeof(SG_Material),
	{ 0,0 },
	SG_MaterialInit,
	SG_MaterialReinit,
	SG_MaterialDestroy,
	SG_MaterialLoad,
	SG_MaterialSave,
	SG_MaterialEdit
};

static const char *sgBlendModeNames[] = {
	("One"),
	("Zero"),
	("Source color"),
	("Target color"),
	("One minus source color"),
	("One minus target color"),
	("Source alpha"),
	("Target alpha"),
	("One minus source alpha"),
	("One minus target alpha"),
	NULL
};

SG_Material *
SG_MaterialNew(void *parent, const char *name)
{
	SG_Material *mat;

	mat = Malloc(sizeof(SG_Material), M_OBJECT);
	SG_MaterialInit(mat, name);
	AG_ObjectAttach(parent, mat);
	return (mat);
}

void
SG_MaterialInit(void *obj, const char *name)
{
	SG_Material *mat = obj;

	AG_ObjectInit(mat, name, &sgMaterialOps);
	mat->flags = 0;
	mat->emissive = SG_ColorRGB(0.0, 0.0, 0.0);
	mat->ambient = SG_ColorRGB(0.2, 0.2, 0.2);
	mat->diffuse = SG_ColorRGB(0.8, 0.8, 0.8);
	mat->specular = SG_ColorRGB(0.0, 0.0, 0.0);
	mat->shininess = 0.0;
	mat->blend_src = SG_BLEND_ONE;
	mat->blend_dst = SG_BLEND_ZERO;
	TAILQ_INIT(&mat->textures);
}

void
SG_MaterialReinit(void *obj)
{
	SG_Material *mat = obj;
}

void
SG_MaterialDestroy(void *obj)
{
	SG_Material *mat = obj;
}

int
SG_MaterialLoad(void *obj, AG_Netbuf *buf)
{
	SG_Material *mat = obj;

	if (AG_ReadVersion(buf, sgMaterialOps.type, &sgMaterialOps.ver, NULL)
	    != 0)
		return (-1);

	return (0);
}

int
SG_MaterialSave(void *obj, AG_Netbuf *buf)
{
	SG_Material *mat = obj;

	AG_WriteVersion(buf, sgMaterialOps.type, &sgMaterialOps.ver);
	return (0);
}

static void
SelectColor(AG_Event *event)
{
	AG_HSVPal *pal = AG_PTR(1);
	void *color = AG_PTR(2);

	SG_WidgetBindReal(pal, "RGBAv", color);
}

static void
PollTextures(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	SG_Material *mat = AG_PTR(1);
	AG_TlistItem *it;
	SG_Texture *tex;
	int n = 0;

	AG_TlistClear(tl);
	TAILQ_FOREACH(tex, &mat->textures, textures) {
		it = AG_TlistAdd(tl, NULL, "%d (%ux%u, %ubpp)", n++, tex->su->w,
		    tex->su->h, tex->su->format->BitsPerPixel);
		it->p1 = tex;
		it->cat = "texture";
		AG_TlistSetIcon(tl, it, tex->su);
	}
	AG_TlistRestore(tl);
}

SG_Texture *
SG_TextureFromSurface(SDL_Surface *su)
{
	SG_Texture *tex;

	tex = Malloc(sizeof(SG_Texture), M_SG);
	tex->s = 0;
	tex->t = 0;
	tex->su = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    AG_PowOf2i(su->w),
	    AG_PowOf2i(su->h), 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
	if (tex->su == NULL) {
		AG_SetError("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}
	return (tex);
}

static void
ImportTextureBMP(AG_Event *event)
{
	SG_Material *mat = AG_PTR(1);
	char *path = AG_STRING(2);
	SDL_Surface *bmp;
	SG_Texture *tex;

	if ((bmp = SDL_LoadBMP(path)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
		return;
	}
	if ((tex = SG_TextureFromSurface(bmp)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
		SDL_FreeSurface(bmp);
		return;
	}
	SDL_SetAlpha(bmp, 0, 0);
	SDL_BlitSurface(bmp, NULL, tex->su, NULL);
	SDL_FreeSurface(bmp);
	TAILQ_INSERT_TAIL(&mat->textures, tex, textures);
}

static void
PreviewTexture(AG_Event *event)
{
	AG_FileDlg *fd = AG_SELF();
	AG_Pixmap *pix = AG_PTR(1);
	char *path = AG_STRING(2);
	SDL_Surface *bmp;

	if ((bmp = SDL_LoadBMP(path)) != NULL) {
		AG_PixmapReplaceSurfaceScaled(pix, bmp, 196, 128);
		SDL_FreeSurface(bmp);
	} else {
		AG_PixmapReplaceSurface(pix,
		    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR),
		        SDL_GetError()));
	}
}

static void
ImportTextureDlg(AG_Event *event)
{
	AG_Window *pwin = AG_PTR(1), *win;
	SG_Material *mat = AG_PTR(2);
	AG_FileDlg *fd;
	AG_Box *hBox;
	AG_Pixmap *pix;

	win = AG_WindowNew(AG_WINDOW_MODAL);
	AG_WindowSetCaption(win, _("%s: Import texture"), AGOBJECT(mat)->name);

	hBox = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_EXPAND);
	{
		fd = AG_FileDlgNew(hBox, AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|
		                         AG_FILEDLG_EXPAND);
		AG_FileDlgAddType(fd, _("PC bitmap"), "*.bmp",
		    ImportTextureBMP, "%p", mat);

		pix = AG_PixmapNew(hBox, 0, 196, 128);
		AG_SetEvent(fd, "file-selected", PreviewTexture, "%p", pix);
	}

	AG_WindowShow(win);
}

void *
SG_MaterialEdit(void *obj)
{
	SG_Material *mat = obj;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Material <%s>"), AGOBJECT(mat)->name);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Lighting"), AG_BOX_VERT);
	{
		AG_Toolbar *bar;
		AG_HSVPal *pal;
		
		pal = AG_HSVPalNew(ntab, AG_HSVPAL_EXPAND);
		SG_WidgetBindReal(pal, "RGBAv", (void *)&mat->ambient);
		bar = AG_ToolbarNew(ntab, AG_TOOLBAR_HORIZ, 1,
		    AG_TOOLBAR_HOMOGENOUS|AG_TOOLBAR_STICKY);
		{
			AG_ToolbarButton(bar, _("Ambient"), 1,
			    SelectColor, "%p,%p", pal, &mat->ambient);
			AG_ToolbarButton(bar, _("Diffuse"), 0,
			    SelectColor, "%p,%p", pal, &mat->diffuse);
			AG_ToolbarButton(bar, _("Specular"), 0,
			    SelectColor, "%p,%p", pal, &mat->specular);
			AG_ToolbarButton(bar, _("Emissive"), 0,
			    SelectColor, "%p,%p", pal, &mat->emissive);
		}
		SG_SpinReal(ntab, _("Shininess: "), &mat->shininess);
	}
	ntab = AG_NotebookAddTab(nb, _("Blending"), AG_BOX_HORIZ);
	{
		AG_Radio *rad;
		AG_Box *vbox;

		vbox = AG_BoxNew(ntab, AG_BOX_VERT, 0);
		{
			AG_LabelNewStatic(vbox, _("Source: "));
			rad = AG_RadioNew(vbox, 0, sgBlendModeNames);
			AG_WidgetBindInt(rad, "value", &mat->blend_src);
		}
		vbox = AG_BoxNew(ntab, AG_BOX_VERT, 0);
		{
			AG_LabelNewStatic(vbox, _("Target: "));
			rad = AG_RadioNew(vbox, 0, sgBlendModeNames);
			AG_WidgetBindInt(rad, "value", &mat->blend_dst);
		}
	}
	ntab = AG_NotebookAddTab(nb, _("Textures"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_MenuItem *mi;
		AG_Button *btn;

		tl = AG_TlistNew(ntab, AG_TLIST_EXPAND|AG_TLIST_POLL);
		AG_TlistSetItemHeight(tl, 32);
		AG_TlistPrescale(tl, "XXXXXXXXXXXXXXXXXXXXXXXX (00x00)", 6);
		AG_SetEvent(tl, "tlist-poll", PollTextures, "%p", mat);
		mi = AG_TlistSetPopup(tl, "texture");
		{
			AG_MenuAction(mi, _("Edit texture..."), OBJEDIT_ICON,
			    NULL, "%p,%p,%p", win, mat, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Duplicate texture"), OBJDUP_ICON,
			    NULL, "%p,%p", mat, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Delete texture"), TRASH_ICON,
			    NULL, "%p,%p", mat, tl);
		}
		btn = AG_ButtonNew(ntab, AG_BUTTON_HFILL,
		    _("Import texture..."));
		AG_SetEvent(btn, "button-pushed", ImportTextureDlg,
		    "%p,%p", win, mat);
	}
	ntab = AG_NotebookAddTab(nb, _("Programs"), AG_BOX_VERT);
	{
		
	}
	return (win);
}

void
SG_MaterialBind(SG_Material *mat, SG_View *view)
{
	SG_MaterialColor(GL_FRONT, GL_EMISSION, &mat->emissive);
	SG_MaterialColor(GL_FRONT, GL_AMBIENT, &mat->ambient);
	SG_MaterialColor(GL_FRONT, GL_DIFFUSE, &mat->diffuse);
	SG_MaterialColor(GL_FRONT, GL_SPECULAR, &mat->specular);
	SG_Materialf(GL_FRONT, GL_SHININESS, mat->shininess);
}

void
SG_MaterialUnbind(SG_Material *mat, SG_View *view)
{
}
