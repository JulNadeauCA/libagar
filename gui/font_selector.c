/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include "font_selector.h"

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "icons.h"
#include "primitive.h"

AG_FontSelector *
AG_FontSelectorNew(void *parent, Uint flags)
{
	AG_FontSelector *fs;

	fs = Malloc(sizeof(AG_FontSelector));
	AG_ObjectInit(fs, &agFontSelectorClass);
	fs->flags |= flags;
	if (flags & AG_FONTSELECTOR_HFILL) { AG_ExpandHoriz(fs); }
	if (flags & AG_FONTSELECTOR_VFILL) { AG_ExpandVert(fs); }
	
	AG_ObjectAttach(parent, fs);
	return (fs);
}

static void
Bound(AG_Event *event)
{
	AG_FontSelector *fs = AG_SELF();
	AG_Variable *b = AG_PTR(1);
	AG_Font **pFont;

	if (strcmp(b->name, "font") != 0)
		return;

	AG_ObjectLock(fs);

	pFont = b->data.p;
	AG_SetPointer(fs, "font", *pFont);
	Strlcpy(fs->curFace, OBJECT(*pFont)->name, sizeof(fs->curFace));
	fs->curSize = (*pFont)->size;
	fs->curStyle = (*pFont)->flags;

	fs->tlFaces->flags |= AG_TLIST_SCROLLTOSEL;
	fs->tlSizes->flags |= AG_TLIST_SCROLLTOSEL;

	AG_ObjectUnlock(fs);
}

static void
UpdateFontSelection(AG_FontSelector *fs)
{
	AG_Variable *bFont;
	AG_Font *font, **pFont;

	font = AG_FetchFont(fs->curFace, fs->curSize, fs->curStyle);
	if (font == NULL) {
		AG_TextError(_("Error opening font: %s"), AG_GetError());
		return;
	}
	bFont = AG_GetVariable(fs, "font", &pFont);
	*pFont = font;
	AG_UnlockVariable(bFont);
}

static void
UpdatePreview(AG_FontSelector *fs)
{
	AG_Variable *bFont;
	AG_Font **pFont;
	AG_Surface *s;
	
	bFont = AG_GetVariable(fs, "font", &pFont);
	AG_PushTextState();

	if (*pFont != NULL) {
		AG_TextFont(*pFont);
	}
	s = AG_TextRender(_("The Quick Brown Fox Jumps Over The Lazy Dog"));
	if (fs->sPreview == -1) {
		fs->sPreview = AG_WidgetMapSurfaceNODUP(fs, s);
	} else {
		AG_WidgetReplaceSurfaceNODUP(fs, fs->sPreview, s);
	}

	AG_PopTextState();
	AG_UnlockVariable(bFont);
}

static void
UpdateFaces(AG_Event *event)
{
	AG_Variable *bFont;
	AG_Font **pFont;
	AG_FontSelector *fs = AG_SELF();
	char fontPath[AG_SEARCHPATH_MAX], *pFontPath = &fontPath[0];
	AG_TlistItem *ti;
	char *s;
	int i;
	const int stdSizes[] = { 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
	                         22,24,26,28,32,48,64 };
	const int nStdSizes = sizeof(stdSizes) / sizeof(stdSizes[0]);
	
	bFont = AG_GetVariable(fs, "font", &pFont);
	AG_PushTextState();

	fs->flags &= ~(AG_FONTSELECTOR_UPDATE);

	for (i = 0; i < agBuiltinFontCount; i++) {
		AG_StaticFont *font = agBuiltinFonts[i];

		ti = AG_TlistAdd(fs->tlFaces, NULL, "_%s", font->name);
		ti->p1 = font;
		if (*pFont != NULL &&
		    strcmp(ti->text, OBJECT(*pFont)->name) == 0)
			ti->selected++;
	}

	AG_CopyCfgString("font-path", fontPath, sizeof(fontPath));
	while ((s = AG_Strsep(&pFontPath, ":")) != NULL) {
		AG_Dir *dir;
		int i;

		if ((dir = AG_OpenDir(s)) == NULL) {
			AG_Verbose(_("Ignoring: %s\n"), AG_GetError());
			continue;
		}
		for (i = 0; i < dir->nents; i++) {
			char path[AG_FILENAME_MAX];
			AG_FileInfo info;
			char *file = dir->ents[i], *pExt;

			if (file[0] == '.' ||
			    (pExt = strrchr(file, '.')) == NULL) {
				continue;
			}
			if (strcmp(pExt, ".ttf") != 0 &&
			    strcmp(pExt, ".TTF") != 0)
				continue;

			Strlcpy(path, s, sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));
			Strlcat(path, file, sizeof(path));

			if (AG_GetFileInfo(path, &info) == -1 ||
			    info.type != AG_FILE_REGULAR) {
				continue;
			}
			ti = AG_TlistAddS(fs->tlFaces, NULL, file);
			if (*pFont != NULL &&
			    strcmp(file, OBJECT(*pFont)->name) == 0)
				ti->selected++;
		}
		AG_CloseDir(dir);
	}

	/* XXX */
	for (i = 0; i < nStdSizes; i++) {
		ti = AG_TlistAdd(fs->tlSizes, NULL, "%d", stdSizes[i]);
		if (*pFont != NULL &&
		    stdSizes[i] == (*pFont)->size)
			ti->selected++;
	}
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Regular"));
	if (*pFont != NULL && (*pFont)->flags == 0) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Italic"));
	if (*pFont != NULL && (*pFont)->flags == AG_FONT_ITALIC) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Bold"));
	if (*pFont != NULL && (*pFont)->flags == AG_FONT_BOLD) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Bold Italic"));
	if (*pFont != NULL && (*pFont)->flags == (AG_FONT_BOLD|AG_FONT_ITALIC)) { ti->selected++; }

	UpdatePreview(fs);

	AG_UnlockVariable(bFont);
}

static void
SelectedFace(AG_Event *event)
{
	AG_FontSelector *fs = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);

	Strlcpy(fs->curFace, it->text, sizeof(fs->curFace));
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
SelectedStyle(AG_Event *event)
{
	AG_FontSelector *fs = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	Uint flags = 0;

	if (!strcmp(it->text, _("Italic")))
		flags |= AG_FONT_ITALIC;
	if (!strcmp(it->text, _("Bold")))
		flags |= AG_FONT_BOLD;
	if (!strcmp(it->text, _("Bold Italic")))
		flags |= (AG_FONT_BOLD|AG_FONT_ITALIC);

	fs->curStyle = flags;
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
SelectedSize(AG_Event *event)
{
	AG_FontSelector *fs = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);

	fs->curSize = atoi(it->text);
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
Init(void *obj)
{
	AG_FontSelector *fs = obj;
	
	fs->flags = AG_FONTSELECTOR_UPDATE;

	fs->hPane = AG_PaneNewHoriz(fs, AG_PANE_EXPAND);
	fs->tlFaces = AG_TlistNew(fs->hPane->div[0], AG_TLIST_EXPAND);
	fs->hPane2 = AG_PaneNewHoriz(fs->hPane->div[1], AG_PANE_EXPAND);
	fs->tlStyles = AG_TlistNew(fs->hPane2->div[0], AG_TLIST_EXPAND);
	fs->sizeBox = AG_BoxNewVert(fs->hPane2->div[1], AG_BOX_EXPAND);
	fs->tlSizes = AG_TlistNew(fs->sizeBox, AG_TLIST_EXPAND);

	fs->font = NULL;
	fs->curFace[0] = '\0';
	fs->curStyle = 0;
	fs->curSize = 0;
	fs->rPreview = AG_RECT(0,0,0,64);
	fs->sPreview = -1;

	AG_TlistSizeHint(fs->tlFaces, "XXXXXXXXXXXXXXX", 8);
	AG_TlistSizeHint(fs->tlStyles, "XXXXXXXXX", 8);
	AG_TlistSizeHint(fs->tlSizes, "100", 8);
	
	AG_BindPointer(fs, "font", (void *)&fs->font);
	
	AG_SetEvent(fs, "bound", Bound, NULL);
	AG_AddEvent(fs, "widget-shown", UpdateFaces, NULL);
	AG_SetEvent(fs->tlFaces, "tlist-selected", SelectedFace, "%p", fs);
	AG_SetEvent(fs->tlStyles, "tlist-selected", SelectedStyle, "%p", fs);
	AG_SetEvent(fs->tlSizes, "tlist-selected", SelectedSize, "%p", fs);
	
#ifdef AG_DEBUG
	AG_BindUint(fs, "flags", &fs->flags);
	/* AG_BindString(fs, "curFace", fs->curFace, sizeof(fs->curFace)); */
	AG_BindUint(fs, "curStyle", &fs->curStyle);
	AG_BindInt(fs, "curSize", &fs->curSize);
	AG_BindInt(fs, "sPreview", &fs->sPreview);
#endif
}

static void
Draw(void *obj)
{
	AG_FontSelector *fs = obj;
	AG_Widget *chld;

	WIDGET_FOREACH_CHILD(chld, obj)
		AG_WidgetDraw(chld);
	
	AG_DrawBox(fs, fs->rPreview, -1, agColors[FIXED_BOX_COLOR]);
	if (fs->sPreview != -1) {
		AG_Surface *su = WSURFACE(fs,fs->sPreview);

		AG_WidgetBlitSurface(fs, fs->sPreview,
		    fs->rPreview.x + fs->rPreview.w/2 - su->w/2,
		    fs->rPreview.y + fs->rPreview.h/2 - su->h/2);
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_FontSelector *fs = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(fs->hPane, &rChld);
	r->w = rChld.w;
	r->h = rChld.h + fs->rPreview.h;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_FontSelector *fs = obj;
	AG_SizeAlloc aChld;

	/* Size horizontal pane */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w;
	aChld.h = a->h - fs->rPreview.h;
	AG_WidgetSizeAlloc(fs->hPane, &aChld);

	fs->rPreview.x = 0;
	fs->rPreview.y = a->h - fs->rPreview.h;
	fs->rPreview.w = a->w;
	return (0);
}

AG_WidgetClass agFontSelectorClass = {
	{
		"Agar(Widget:FontSelector)",
		sizeof(AG_FontSelector),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
