/*
 * Copyright (c) 2013-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Palette object.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/gui/opengl.h>

#include <string.h>
#include <stdlib.h>

/* Create a new palette object. */
SG_Palette *
SG_PaletteNew(void *parent, const char *name)
{
	SG_Palette *pal;

	pal = Malloc(sizeof(SG_Palette));
	AG_ObjectInit(pal, &sgPaletteClass);
	if (name) {
		AG_ObjectSetNameS(pal, name);
	} else {
		OBJECT(pal)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, pal);
	return (pal);
}

SG_Pigment *
SG_PaletteAddPigment(SG_Palette *pal)
{
	SG_Pigment *pig;

	if ((pig = TryMalloc(sizeof(SG_Pigment))) == NULL) {
		return (NULL);
	}
	memset(pig, 0, sizeof(SG_Pigment));

	AG_ObjectLock(pal);
	TAILQ_INSERT_TAIL(&pal->pigments, pig, pigments);
	AG_ObjectUnlock(pal);
	return (pig);
}

void
SG_PaletteDelPigment(SG_Palette *pal, SG_Pigment *pig)
{
	AG_ObjectLock(pal);
	TAILQ_REMOVE(&pal->pigments, pig, pigments);
	AG_ObjectUnlock(pal);

	free(pig);
}

SG_Mixture *
SG_PaletteAddMixture(SG_Palette *pal)
{
	SG_Mixture *mix;

	if ((mix = TryMalloc(sizeof(SG_Mixture))) == NULL) {
		return (NULL);
	}
	memset(mix, 0, sizeof(SG_Mixture));

	AG_ObjectLock(pal);
	TAILQ_INSERT_TAIL(&pal->mixtures, mix, mixtures);
	AG_ObjectUnlock(pal);
	return (mix);
}

void
SG_PaletteDelMixture(SG_Palette *pal, SG_Mixture *mix)
{
	AG_ObjectLock(pal);
	TAILQ_REMOVE(&pal->mixtures, mix, mixtures);
	AG_ObjectUnlock(pal);

	free(mix);
}

static void
Init(void *_Nonnull obj)
{
	SG_Palette *pal = obj;

	pal->flags = 0;
	TAILQ_INIT(&pal->pigments);
	TAILQ_INIT(&pal->mixtures);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Palette *pal = obj;
	SG_Pigment *pig, *pigNext;
	SG_Mixture *mix, *mixNext;

	for (pig = TAILQ_FIRST(&pal->pigments);
	     pig != TAILQ_END(&pal->pigments);
	     pig = pigNext) {
		pigNext = TAILQ_NEXT(pig, pigments);
		free(pig);
	}
	for (mix = TAILQ_FIRST(&pal->mixtures);
	     mix != TAILQ_END(&pal->mixtures);
	     mix = mixNext) {
		mixNext = TAILQ_NEXT(mix, mixtures);
		free(mix);
	}
	
	TAILQ_INIT(&pal->pigments);
	TAILQ_INIT(&pal->mixtures);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Palette *pal = obj;
	Uint i, count;

	pal->flags &= ~(SG_PALETTE_SAVED);
	pal->flags |= (AG_ReadUint32(ds) & SG_PALETTE_SAVED);

	if ((count = (Uint)AG_ReadUint32(ds)) > SG_PALETTE_PIGMENTS_MAX) {
		AG_SetError("Bad count");
		return (-1);
	}
	for (i = 0; i < count; i++) {
		SG_Pigment *pig;

		if ((pig = SG_PaletteAddPigment(pal)) == NULL) {
			return (-1);
		}
		pig->id = (Uint)AG_ReadUint32(ds);
		AG_CopyString(pig->name, ds, sizeof(pig->name));
		AG_CopyString(pig->ciName, ds, sizeof(pig->ciName));
		pig->Tr = M_ReadReal(ds);
		pig->St = M_ReadReal(ds);
		pig->VR = M_ReadReal(ds);
		pig->Gr = M_ReadReal(ds);
		pig->Bl = M_ReadReal(ds);
		pig->Df = M_ReadReal(ds);
		pig->HA = M_ReadReal(ds);
		pig->HS = M_ReadReal(ds);
		pig->LfTint = M_ReadReal(ds);
		pig->LfMass = M_ReadReal(ds);
	}
	return (0);
}

static int
Save(void *_Nonnull obj, AG_DataSource *_Nonnull ds)
{
	SG_Palette *pal = obj;
	SG_Pigment *pig;
	Uint32 count = 0;
	off_t countOffs;

	AG_WriteUint32(ds, pal->flags & SG_PALETTE_SAVED);
	countOffs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);

	TAILQ_FOREACH(pig, &pal->pigments, pigments) {
		AG_WriteUint32(ds, (Uint32)pig->id);
		AG_WriteString(ds, pig->name);
		AG_WriteString(ds, pig->ciName);
		M_WriteReal(ds, pig->Tr);
		M_WriteReal(ds, pig->St);
		M_WriteReal(ds, pig->VR);
		M_WriteReal(ds, pig->Gr);
		M_WriteReal(ds, pig->Bl);
		M_WriteReal(ds, pig->Df);
		M_WriteReal(ds, pig->HA);
		M_WriteReal(ds, pig->HS);
		M_WriteReal(ds, pig->LfTint);
		M_WriteReal(ds, pig->LfMass);
		count++;
	}
	AG_WriteUint32At(ds, count, countOffs);
	return (0);
}

static void
PollPigments(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_Palette *pal = AG_PTR(1);
	AG_TlistItem *it;
	SG_Pigment *pig;

	AG_TlistClear(tl);
	AG_ObjectLock(pal);
	TAILQ_FOREACH(pig, &pal->pigments, pigments) {
		it = AG_TlistAdd(tl, NULL, "%s\n(%s)", pig->name, pig->ciName);
		it->p1 = pig;
		it->cat = "palette-pigment";
	}
	AG_ObjectUnlock(pal);
	AG_TlistRestore(tl);
}

static void
AddPigment(AG_Event *_Nonnull event)
{
	SG_Palette *pal = AG_PTR(1);
	SG_Pigment *pig;

	if ((pig = SG_PaletteAddPigment(pal)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	AG_ObjectLock(pal);
	Strlcpy(pig->name, _("New Pigment"), sizeof(pig->name));
	AG_ObjectUnlock(pal);
}

static void
PollMixtures(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	SG_Palette *pal = AG_PTR(1);
	AG_TlistItem *it;
	SG_Mixture *mix;

	AG_TlistClear(tl);
	AG_ObjectLock(pal);
	TAILQ_FOREACH(mix, &pal->mixtures, mixtures) {
		it = AG_TlistAdd(tl, NULL, "%s\nHSV=%f,%f,%f",
		    mix->name, mix->h, mix->s, mix->v);
		it->p1 = mix;
		it->cat = "palette-mixture";
	}
	AG_ObjectUnlock(pal);
	AG_TlistRestore(tl);
}

static void
AddMixture(AG_Event *_Nonnull event)
{
	SG_Palette *pal = AG_PTR(1);
	SG_Mixture *mix;

	if ((mix = SG_PaletteAddMixture(pal)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	AG_ObjectLock(pal);
	Strlcpy(mix->name, _("New Mixture"), sizeof(mix->name));
	AG_ObjectUnlock(pal);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	SG_Palette *pal = obj;
/*	AG_Mutex *lock = &OBJECT(pal)->lock; */
	AG_Window *win;
	AG_Pane *paHoriz;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Palette <%s>"), OBJECT(pal)->name);

	paHoriz = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	(void)SG_PaletteViewNew(paHoriz->div[1], pal, SG_PALETTE_VIEW_EXPAND);

	nb = AG_NotebookNew(paHoriz->div[0], AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Pigments"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_MenuItem *mi;

		tl = AG_TlistNewPolled(ntab, AG_TLIST_EXPAND,
		    PollPigments, "%p", pal);
		AG_TlistSetItemHeight(tl, 64);
		AG_TlistSetIconWidth(tl, 64);
		AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXX", 7);

		mi = AG_TlistSetPopup(tl, "palette-pigment");
		{
			AG_MenuAction(mi, _("Edit Pigment..."), NULL,
			    NULL, "%p,%p,%p", win, pal, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Delete Pigment"), agIconTrash.s,
			    NULL, "%p,%p", pal, tl);
		}
		AG_ButtonNewFn(ntab, 0, _("Add Pigment"),
		    AddPigment, "%p", pal);
	}
	ntab = AG_NotebookAdd(nb, _("Mixtures"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_MenuItem *mi;

		tl = AG_TlistNewPolled(ntab, AG_TLIST_EXPAND,
		    PollMixtures, "%p", pal);
		AG_TlistSetItemHeight(tl, 64);
		AG_TlistSetIconWidth(tl, 64);
		AG_TlistSizeHint(tl, "XXXXXXXXXXXXXXXX\nXXXXXXXXXXXXXXXX", 7);

		mi = AG_TlistSetPopup(tl, "palette-pigment");
		{
			AG_MenuAction(mi, _("Edit Mixture..."), NULL,
			    NULL, "%p,%p,%p", win, pal, tl);
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Delete Mixture"), agIconTrash.s,
			    NULL, "%p,%p", pal, tl);
		}
		AG_ButtonNewFn(ntab, 0, _("Add Mixture"),
		    AddMixture, "%p", pal);
	}

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 50, 50);
	return (win);
}

AG_ObjectClass sgPaletteClass = {
	"SG_Palette",
	sizeof(SG_Palette),
	{ 0,0 },
	Init,
	Reset,
	NULL,			/* destroy */
	Load,
	Save,
	Edit
};
