/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <gui/gui.h>
#include <gui/load_surface.h>

#include <gui/box.h>
#include <gui/numerical.h>
#include <gui/mspinbutton.h>
#include <gui/checkbox.h>
#include <gui/tlist.h>
#include <gui/label.h>
#include <gui/combo.h>
#include <gui/notebook.h>
#include <gui/pixmap.h>
#include <gui/separator.h>
#include <gui/pane.h>
#include <gui/slider.h>

#include "tileset.h"
#include "tileview.h"
#include "animview.h"

#include <stdarg.h>
#include <string.h>
	
static const char *insnNames[] = {
	N_("Tile"),
	N_("Displace pixmap"),
	N_("Rotate pixmap")
};

void
RG_AnimInit(RG_Anim *ani, RG_Tileset *ts, const char *name, Uint flags)
{
	Strlcpy(ani->name, name, sizeof(ani->name));
	ani->main_id = 0;
	ani->flags = flags;
	ani->w = 0;
	ani->h = 0;
	ani->tileset = ts;
	ani->nrefs = 0;

	ani->insns = Malloc(sizeof(RG_AnimInsn));
	ani->ninsns = 0;
	ani->frames = Malloc(sizeof(RG_AnimFrame));
	ani->nframes = 0;

	ani->gframe = 0;
}

void
RG_AnimScale(RG_Anim *ani, Uint w, Uint h)
{
	ani->w = w;
	ani->h = h;

	/* TODO scale existing frames */
}

Uint
RG_AnimInsertInsn(RG_Anim *ani, enum rg_anim_insn_type type)
{
	RG_AnimInsn *insn;

	ani->insns = Realloc(ani->insns,
	    (ani->ninsns+1)*sizeof(RG_AnimInsn));
	insn = &ani->insns[ani->ninsns];
	insn->type = type;
	insn->t = NULL;
	insn->px = NULL;
/*	insn->sk = NULL; */
	insn->delay = 100;

	switch (type) {
	case RG_ANIM_TILE:
		insn->in_tile.alpha = AG_ALPHA_OPAQUE;
		break;
	case RG_ANIM_DISPX:
		insn->in_disPx.dx = 0;
		insn->in_disPx.dy = 0;
		break;
	case RG_ANIM_ROTPX:
		insn->in_rotPx.x = 0;
		insn->in_rotPx.y = 0;
		insn->in_rotPx.theta = 0;
		break;
	default:
		break;
	}
	return (ani->ninsns++);
}

static void
DestroyInsn(RG_AnimInsn *insn)
{
	if (insn->t != NULL)	insn->t->nrefs--;
	if (insn->px != NULL)	insn->px->nrefs--;
/*	if (insn->sk != NULL)	insn->sk->nrefs--; */
}

void
RG_AnimRemoveInsn(RG_Anim *ani, Uint insn)
{
	DestroyInsn(&ani->insns[insn]);
	if (insn+1 < ani->ninsns) {
		memmove(&ani->insns[insn], &ani->insns[insn+1],
		    (ani->ninsns - 1)*sizeof(RG_AnimInsn));
	}
	ani->ninsns--;
}

Uint
RG_AnimInsertFrame(RG_Anim *ani, AG_Surface *sNew)
{
	RG_Tileset *ts = ani->tileset;
	Uint32 sFlags = 0;
	RG_AnimFrame *fr;

#ifdef AG_DEBUG
	if ((ani->nframes+1) >= RG_ANIMATION_FRAMES_MAX)
		AG_FatalError("%s: Too many frames", ani->name);
#endif

	if (ani->flags & RG_ANIM_SRCCOLORKEY)	sFlags |= AG_SRCCOLORKEY;
	if (ani->flags & RG_ANIM_SRCALPHA)	sFlags |= AG_SRCALPHA;

	ani->frames = Realloc(ani->frames,
	    (ani->nframes+1)*sizeof(RG_AnimFrame));
	fr = &ani->frames[ani->nframes];
	if (sNew != NULL) {
		fr->su = sNew;
	} else {
		if ((fr->su = RG_SurfaceStd(ts, ani->w, ani->h, sFlags))
		    == NULL)
			AG_FatalError(NULL);
	}
	fr->delay = 0;
	fr->name = ani->nframes++;
	return (fr->name);
}

static void
FreeFrame(RG_AnimFrame *fr)
{
	AG_SurfaceFree(fr->su);
}

void
RG_AnimRemoveFrame(RG_Anim *ani, Uint frame)
{
	FreeFrame(&ani->frames[frame]);
	if (frame+1 < ani->nframes)
		memmove(&ani->frames[frame], &ani->frames[frame+1],
		    (--ani->nframes)*sizeof(RG_AnimFrame));
}

static void
FreeFrames(RG_Anim *ani)
{
	Uint i;
	
	for (i = 0; i < ani->nframes; i++) {
		FreeFrame(&ani->frames[i]);
	}
	ani->nframes = 0;
}

void
RG_AnimDestroy(RG_Anim *ani)
{
	Uint i;

	for (i = 0; i < ani->ninsns; i++) {
		DestroyInsn(&ani->insns[i]);
	}
	Free(ani->insns);
	FreeFrames(ani);
	Free(ani->frames);
}

int
RG_AnimLoad(RG_Anim *ani, AG_DataSource *buf)
{
	RG_Tileset *ts = ani->tileset;
	Uint32 i, ninsns;
	
	ani->w = AG_ReadUint16(buf);
	ani->h = AG_ReadUint16(buf);

	ninsns = AG_ReadUint32(buf);
	for (i = 0; i < ninsns; i++) {
		char name[RG_TILESET_NAME_MAX];
		enum rg_anim_insn_type type;
		RG_AnimInsn *insn;
		
		type = (enum rg_anim_insn_type)AG_ReadUint16(buf);
		insn = &ani->insns[RG_AnimInsertInsn(ani, type)];
		insn->delay = (Uint)AG_ReadUint32(buf);

		switch (type) {
		case RG_ANIM_TILE:
			AG_CopyString(name, buf, sizeof(name));
			insn->t = RG_TilesetFindTile(ts, name);
			insn->in_tile.alpha = (Uint)AG_ReadUint8(buf);
			break;
		case RG_ANIM_DISPX:
			AG_CopyString(name, buf, sizeof(name));
			insn->px = RG_TilesetFindPixmap(ts, name);
			insn->in_disPx.dx = (int)AG_ReadSint16(buf);
			insn->in_disPx.dy = (int)AG_ReadSint16(buf);
			break;
		case RG_ANIM_ROTPX:
			AG_CopyString(name, buf, sizeof(name));
			insn->px = RG_TilesetFindPixmap(ts, name);
			insn->in_rotPx.x = (Uint)AG_ReadUint16(buf);
			insn->in_rotPx.y = (Uint)AG_ReadUint16(buf);
			insn->in_rotPx.theta = (int)AG_ReadUint8(buf);
			break;
		}
	}

	ani->nframes = (Uint)AG_ReadUint32(buf);
	ani->frames = Realloc(ani->frames,
	    ani->nframes*sizeof(RG_AnimFrame));
	for (i = 0; i < ani->nframes; i++) {
		RG_AnimFrame *fr = &ani->frames[i];

		fr->name = i;
		fr->su = AG_ReadSurface(buf, ts->fmt);
		fr->delay = (Uint)AG_ReadUint32(buf);
	}
	return (0);
}

void
RG_AnimSave(RG_Anim *ani, AG_DataSource *buf)
{
	Uint i;
	
	AG_WriteUint16(buf, ani->w);
	AG_WriteUint16(buf, ani->h);
	
	AG_WriteUint32(buf, ani->ninsns);
	for (i = 0; i < ani->ninsns; i++) {
		RG_AnimInsn *insn = &ani->insns[i];
		
		AG_WriteUint16(buf, (Uint16)insn->type);
		AG_WriteUint32(buf, (Uint32)insn->delay);
		switch (insn->type) {
		case RG_ANIM_TILE:
			AG_WriteString(buf, insn->t->name);
			AG_WriteUint8(buf, (Uint8)insn->in_tile.alpha);
			break;
		case RG_ANIM_DISPX:
			AG_WriteString(buf, insn->px->name);
			AG_WriteSint16(buf, (Sint16)insn->in_disPx.dx);
			AG_WriteSint16(buf, (Sint16)insn->in_disPx.dy);
			break;
		case RG_ANIM_ROTPX:
			AG_WriteString(buf, insn->px->name);
			AG_WriteUint16(buf, (Uint16)insn->in_rotPx.x);
			AG_WriteUint16(buf, (Uint16)insn->in_rotPx.y);
			AG_WriteUint8(buf, (Uint8)insn->in_rotPx.theta);
			break;
		}
	}

	AG_WriteUint32(buf, ani->nframes);
	for (i = 0; i < ani->nframes; i++) {
		RG_AnimFrame *fr = &ani->frames[i];

		AG_WriteSurface(buf, fr->su);
		AG_WriteUint32(buf, (Uint32)fr->delay);
	}
}

void
RG_AnimGenerate(RG_Anim *ani)
{
	Uint i;

	FreeFrames(ani);

	for (i = 0; i < ani->ninsns; i++) {
		RG_AnimInsn *insn = &ani->insns[i];
		RG_AnimFrame *fr;

		switch (insn->type) {
		case RG_ANIM_TILE:
			if (insn->t != NULL && insn->t->su != NULL) {
				fr = &ani->frames[RG_AnimInsertFrame(ani,NULL)];
				if (AG_ScaleSurface(insn->t->su, ani->w, ani->h,
				    &fr->su) == -1) {
					AG_FatalError(NULL);
				}
				fr->delay = insn->delay;
			}
			break;
		default:
			break;
		}
	}
}

static void
EditClose(AG_Event *event)
{
	AG_Window *win = AG_SELF();
#ifdef AG_THREADS
	RG_Tileset *ts = AG_PTR(1);
#endif
	RG_Anim *ani = AG_PTR(2);
	
	AG_MutexLock(&ts->lock);
	ani->nrefs--;
	AG_MutexUnlock(&ts->lock);

	AG_ObjectDetach(win);
}

static void
PollInsns(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Anim *ani = AG_PTR(1);
#ifdef AG_THREADS
	RG_Tileset *ts = ani->tileset;
#endif
	AG_TlistItem *it;
	Uint i;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);

	for (i = 0; i < ani->ninsns; i++) {
		RG_AnimInsn *insn = &ani->insns[i];

		switch (insn->type) {
		case RG_ANIM_TILE:
			it = AG_TlistAdd(tl, NULL, _("[%04u] Tile <%s>"),
			    insn->delay,
			    insn->t != NULL ? insn->t->name : "(null)");
			AG_TlistSetIcon(tl, it,
			    insn->t != NULL ? insn->t->su : NULL);
			break;
		case RG_ANIM_DISPX:
			it = AG_TlistAdd(tl, NULL, _("[%04u] Displace <%s>"),
			    insn->delay,
			    insn->px != NULL ? insn->px->name : "(null)");
			AG_TlistSetIcon(tl, it,
			    insn->px != NULL ? insn->px->su : NULL);
			break;
		case RG_ANIM_ROTPX:
			it = AG_TlistAdd(tl, NULL,
			    _("[%04u] Rotate <%s> %u\xc2\xb0"), insn->delay,
			    insn->px != NULL ? insn->px->name : "(null)",
			    insn->in_rotPx.theta);
			AG_TlistSetIcon(tl, it,
			    insn->px != NULL ? insn->px->su : NULL);
			break;
		default:
			it = AG_TlistAdd(tl, NULL, "[%04u] %s",
			    _(insnNames[insn->type]), insn->delay);
			break;
		}
		it->p1 = insn;
		it->cat = "insn";
	}

	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollFrames(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Anim *ani = AG_PTR(1);
#ifdef AG_THREADS
	RG_Tileset *ts = ani->tileset;
#endif
	Uint i;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);
	for (i = 0; i < ani->nframes; i++) {
		RG_AnimFrame *fr = &ani->frames[i];
		AG_TlistItem *it;
		
		it = AG_TlistAdd(tl, NULL, _("Frame %ux%u, %ums"),
		    fr->su->w, fr->su->h, fr->delay);
		it->p1 = fr;
		it->cat = "frame";
		AG_TlistSetIcon(tl, it, fr->su);
	}
	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
PollTiles(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);
	AG_TlistItem *it;
	RG_Tile *t;

	AG_TlistClear(tl);
	AG_MutexLock(&ts->lock);

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u)%s%s", t->name,
		    t->su->w, t->su->h,
		    (t->su->flags & AG_SRCALPHA) ? " alpha" : "",
		    (t->su->flags & AG_SRCCOLORKEY) ? " colorkey" : "");
		it->p1 = t;
		AG_TlistSetIcon(tl, it, t->su);
	}

	AG_MutexUnlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
SelectInsnTile(AG_Event *event)
{
	RG_AnimInsn *insn = AG_PTR(2);
	RG_Tileview *tv = AG_PTR(3);
	AG_TlistItem *it = AG_PTR(4);

	if (it != NULL) {
		insn->t = (RG_Tile *)it->p1;
		RG_TileviewSetTile(tv, insn->t);
	}
}

static void
EditInsn(RG_Anim *ani, RG_AnimInsn *insn, AG_Box *box)
{
	AG_MSpinbutton *msb;
	AG_Combo *comTile;
	RG_Tileview *tv;
	AG_Slider *sl;

	AG_ObjectFreeChildren(box);

	switch (insn->type) {
	case RG_ANIM_TILE:
		tv = RG_TileviewNew(NULL, ani->tileset, 0);
		comTile = AG_ComboNew(box, AG_COMBO_POLL|AG_COMBO_HFILL,
		    _("Tile: "));
		AG_SetEvent(comTile, "combo-selected",
		    SelectInsnTile, "%p,%p,%p", ani, insn, tv);
		AG_SetEvent(comTile->list, "tlist-poll",
		    PollTiles, "%p", ani->tileset);
		if (insn->t != NULL) {
			RG_TileviewSetTile(tv, insn->t);
			AG_ComboSelectPointer(comTile, insn->t);
		}

		AG_LabelNewS(box, 0, _("Preview:"));
		AG_ObjectAttach(box, tv);
		AG_SliderNewUintR(box, AG_SLIDER_HORIZ, 0,
		    &insn->in_tile.alpha, 0, 255);

		AG_WidgetFocus(comTile);
		break;
	case RG_ANIM_DISPX:
		msb = AG_MSpinbuttonNew(box, 0, ",", _("Displacement: "));
		AG_BindInt(msb, "xvalue", &insn->in_disPx.dx);
		AG_BindInt(msb, "yvalue", &insn->in_disPx.dy);
		AG_WidgetFocus(msb);
		break;
	case RG_ANIM_ROTPX:
		msb = AG_MSpinbuttonNew(box, 0, ",", _("Center of rotation: "));
		AG_BindUint(msb, "xvalue", &insn->in_rotPx.x);
		AG_BindUint(msb, "yvalue", &insn->in_rotPx.y);
		AG_NumericalNewIntR(box, 0, "deg", _("Angle of rotation: "),
		    &insn->in_rotPx.theta, 0, 360);
		AG_WidgetFocus(msb);
		break;
	default:
		break;
	}

	AG_LabelNewS(box, 0, _("Delay (ms): "));
	sl = AG_SliderNewUintR(box, AG_SLIDER_HORIZ, AG_SLIDER_HFILL,
	    &insn->delay, 0, 10000);
	AG_SliderSetIntIncrement(sl, 50);
	
	AG_WidgetUpdate(box);
}

static void
EditFrame(RG_Anim *ani, RG_AnimFrame *fr, AG_Box *box)
{
	AG_Numerical *num;
	AG_Pixmap *pix;

	AG_ObjectFreeChildren(box);

	ani->gframe = fr->name;

	pix = AG_PixmapFromSurfaceCopy(box, 0, AG_SurfaceDup(fr->su));

	num = AG_NumericalNewUintR(box, AG_NUMERICAL_HFILL, "ms", _("Delay: "),
	    &fr->delay, 0, 10000);
	AG_NumericalSetIncrement(num, 50);

	AG_WidgetUpdate(box);
}

static void
SelectInsn(AG_Event *event)
{
	RG_Anim *ani = AG_PTR(1);
	AG_Box *box = AG_PTR(2);
	AG_TlistItem *ti = AG_PTR(3);
	RG_AnimInsn *insn = ti->p1;

	EditInsn(ani, insn, box);
}

static void
SelectFrame(AG_Event *event)
{
	RG_Anim *ani = AG_PTR(1);
	AG_Box *box = AG_PTR(2);
	AG_TlistItem *ti = AG_PTR(3);
	RG_AnimFrame *frame = ti->p1;

	EditFrame(ani, frame, box);
}

static void
InsertInsn(AG_Event *event)
{
	RG_Anim *ani = AG_PTR(1);
	enum rg_anim_insn_type type = AG_INT(2);
	AG_Box *box = AG_PTR(3);
	AG_Tlist *tl = AG_PTR(4);
	RG_AnimInsn *insn;
	
	insn = &ani->insns[RG_AnimInsertInsn(ani, type)];
	EditInsn(ani, insn, box);
	AG_TlistSelectPtr(tl, insn);
}

static void
RegenAnim(AG_Event *event)
{
	RG_Anim *ani = AG_PTR(1);
	
	RG_AnimGenerate(ani);
}

static void
PreviewAnim(AG_Event *event)
{
	RG_Anim *ani = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Window *win;
	RG_Animview *av;
	AG_Label *lbl;

	if ((win = AG_WindowNewNamed(0, "rg-anim-prev-%s:%s",
	    OBJECT(ani->tileset)->name, ani->name)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, ani->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_CENTER, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_WindowAttach(pwin, win);

	av = RG_AnimviewNew(win);
	RG_AnimviewSetAnimation(av, ani);
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	lbl = AG_LabelNewPolled(win, 0, " %u/%u", &av->frame, &ani->nframes);
	AG_LabelSizeHint(lbl, 1, " 00000/00000");

	AG_WindowShow(win);
}

AG_Window *
RG_AnimEdit(RG_Anim *ani)
{
	RG_Tileset *ts = ani->tileset;
	AG_Window *win;
	AG_Box *editBox;
	AG_Tlist *tl;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Menu *me;
	AG_MenuItem *mi;
	AG_Pane *hPane;
	int i;

	if ((win = AG_WindowNewNamed(0, "rg-anim-%s:%s", OBJECT(ts)->name,
	    ani->name)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, ani->name);
	AG_SetEvent(win, "window-close", EditClose, "%p,%p", ts, ani);
	
	me = AG_MenuNew(win, AG_MENU_HFILL);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAddTab(nb, _("Instructions"), AG_BOX_VERT);
	hPane = AG_PaneNewHoriz(nt, AG_PANE_EXPAND);
	{
		tl = AG_TlistNew(hPane->div[0], AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_TlistSizeHint(tl, _("[00000] Displace <Tile #000>"), 4);
		AG_SetEvent(tl, "tlist-poll", PollInsns, "%p", ani);

		editBox = AG_BoxNew(hPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
		AG_SetEvent(tl, "tlist-dblclick",
		    SelectInsn, "%p,%p", ani, editBox);

		mi = AG_MenuNode(me->root, _("Instructions"), NULL);
		for (i = 0; i < RG_ANIM_LAST; i++)
			AG_MenuAction(mi, _(insnNames[i]), NULL, InsertInsn,
			    "%p,%i,%p,%p", ani, i, editBox, tl);
	}
	
	nt = AG_NotebookAddTab(nb, _("Frames"), AG_BOX_VERT);
	hPane = AG_PaneNewHoriz(nt, AG_PANE_EXPAND);
	{
		tl = AG_TlistNew(hPane->div[0], AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_TlistSizeHint(tl, _("[00000] Displace <Tile #000>"), 4);
		AG_SetEvent(tl, "tlist-poll", PollFrames, "%p", ani);

		editBox = AG_BoxNew(hPane->div[1], AG_BOX_VERT, AG_BOX_EXPAND);
		AG_SetEvent(tl, "tlist-dblclick",
		    SelectFrame, "%p,%p", ani, editBox);
	}
	
	mi = AG_MenuNode(me->root, _("Animation"), NULL);
	{
		AG_MenuAction(mi, _("Recompile"), NULL,
		    RegenAnim, "%p", ani);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Preview..."), NULL,
		    PreviewAnim, "%p,%p", ani, win);
	}
	
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 70, 50);
	RG_AnimGenerate(ani);
	ani->nrefs++;
	return (win);
}
