/*	$Csoft: animation.c,v 1.5 2005/08/29 03:29:05 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/loader/surface.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/combo.h>
#include <engine/widget/notebook.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/animview.h>
#include <engine/widget/separator.h>

#include "tileset.h"
#include "tileview.h"

#include <stdarg.h>
	
static const char *insn_names[] = {
	N_("Tile"),
	N_("Displace pixmap"),
	N_("Rotate pixmap")
};

void
RG_AnimInit(RG_Anim *ani, RG_Tileset *ts, const char *name,
    int flags)
{
	strlcpy(ani->name, name, sizeof(ani->name));
	ani->flags = flags;
	ani->w = 0;
	ani->h = 0;
	ani->tileset = ts;
	ani->nrefs = 0;

	ani->insns = Malloc(sizeof(RG_AnimInsn), M_RG);
	ani->ninsns = 0;
	ani->frames = Malloc(sizeof(RG_AnimFrame), M_RG);
	ani->nframes = 0;

	ani->gframe = 0;
}

void
RG_AnimScale(RG_Anim *ani, u_int w, u_int h)
{
	ani->w = w;
	ani->h = h;

	/* TODO scale existing frames */
}

u_int
RG_AnimInsertInsn(RG_Anim *ani, enum rg_anim_insn_type type)
{
	RG_AnimInsn *insn;

	ani->insns = Realloc(ani->insns,
	    (ani->ninsns+1)*sizeof(RG_AnimInsn));
	insn = &ani->insns[ani->ninsns];
	insn->type = type;
	insn->t = NULL;
	insn->px = NULL;
	insn->sk = NULL;
	insn->delay = 100;

	switch (type) {
	case RG_ANIM_TILE:
		insn->in_tile.alpha = SDL_ALPHA_OPAQUE;
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
destroy_insn(RG_AnimInsn *insn)
{
	if (insn->t != NULL)	insn->t->nrefs--;
	if (insn->px != NULL)	insn->px->nrefs--;
	if (insn->sk != NULL)	insn->sk->nrefs--;
}

void
RG_AnimRemoveInsn(RG_Anim *ani, u_int insn)
{
	destroy_insn(&ani->insns[insn]);
	if (insn+1 < ani->ninsns)
		memmove(&ani->insns[insn], &ani->insns[insn+1],
		    (--ani->ninsns)*sizeof(RG_AnimInsn));
}

u_int
RG_AnimInsertFrame(RG_Anim *ani)
{
	RG_Tileset *ts = ani->tileset;
	Uint32 sflags = SDL_SWSURFACE;
	RG_AnimFrame *fr;
	
	if (ani->flags & ANIMATION_SRCCOLORKEY)	sflags |= SDL_SRCCOLORKEY;
	if (ani->flags & ANIMATION_SRCALPHA)	sflags |= SDL_SRCALPHA;

	ani->frames = Realloc(ani->frames,
	    (ani->nframes+1)*sizeof(RG_AnimFrame));
	fr = &ani->frames[ani->nframes];
	fr->su = SDL_CreateRGBSurface(sflags, ani->w, ani->h,
	    ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    ts->fmt->Amask);
	if (fr->su == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	fr->delay = 0;
	fr->name = ani->nframes++;
	return (fr->name);
}

static void
destroy_frame(RG_AnimFrame *fr)
{
	SDL_FreeSurface(fr->su);
}

void
RG_AnimRemoveFrame(RG_Anim *ani, u_int frame)
{
	destroy_frame(&ani->frames[frame]);
	if (frame+1 < ani->nframes)
		memmove(&ani->frames[frame], &ani->frames[frame+1],
		    (--ani->nframes)*sizeof(RG_AnimFrame));
}

static void
destroy_frames(RG_Anim *ani)
{
	u_int i;
	
	for (i = 0; i < ani->nframes; i++) {
		destroy_frame(&ani->frames[i]);
	}
	ani->nframes = 0;
}

void
RG_AnimDestroy(RG_Anim *ani)
{
	u_int i;

	for (i = 0; i < ani->ninsns; i++) {
		destroy_insn(&ani->insns[i]);
	}
	Free(ani->insns, M_RG);
	
	destroy_frames(ani);
	Free(ani->frames, M_RG);
}

int
RG_AnimLoad(RG_Anim *ani, AG_Netbuf *buf)
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
		insn->delay = (u_int)AG_ReadUint32(buf);

		switch (type) {
		case RG_ANIM_TILE:
			AG_CopyString(name, buf, sizeof(name));
			insn->t = RG_TilesetFindTile(ts, name);
			insn->in_tile.alpha = (u_int)AG_ReadUint8(buf);
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
			insn->in_rotPx.x = (u_int)AG_ReadUint16(buf);
			insn->in_rotPx.y = (u_int)AG_ReadUint16(buf);
			insn->in_rotPx.theta = (int)AG_ReadUint8(buf);
			break;
		}
	}

	ani->nframes = (u_int)AG_ReadUint32(buf);
	ani->frames = Realloc(ani->frames,
	    ani->nframes*sizeof(RG_AnimFrame));
	for (i = 0; i < ani->nframes; i++) {
		RG_AnimFrame *fr = &ani->frames[i];

		fr->name = i;
		fr->su = AG_ReadSurface(buf, ts->fmt);
		fr->delay = (u_int)AG_ReadUint32(buf);
	}
	return (0);
}

void
RG_AnimSave(RG_Anim *ani, AG_Netbuf *buf)
{
	u_int i;
	
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
	u_int i;

	destroy_frames(ani);

	for (i = 0; i < ani->ninsns; i++) {
		RG_AnimInsn *insn = &ani->insns[i];
		RG_AnimFrame *fr;

		switch (insn->type) {
		case RG_ANIM_TILE:
			if (insn->t != NULL && insn->t->su != NULL) {
				fr = &ani->frames[RG_AnimInsertFrame(ani)];
				AG_ScaleSurface(insn->t->su, ani->w, ani->h,
				    &fr->su);
				fr->delay = insn->delay;
			}
			break;
		default:
			break;
		}
	}
}

#ifdef EDITION

static void
close_animation(int argc, union evarg *argv)
{
	AG_Window *win = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	RG_Anim *ani = argv[2].p;
	
	pthread_mutex_lock(&ts->lock);
	ani->nrefs--;
	pthread_mutex_unlock(&ts->lock);

	AG_ViewDetach(win);
}

static void
poll_insns(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Anim *ani = argv[1].p;
	RG_Tileset *ts = ani->tileset;
	AG_TlistItem *it;
	u_int i;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);

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
			    _(insn_names[insn->type]), insn->delay);
			break;
		}
		it->p1 = insn;
		it->class = "insn";
	}

	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
poll_frames(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Anim *ani = argv[1].p;
	RG_Tileset *ts = ani->tileset;
	u_int i;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);
	for (i = 0; i < ani->nframes; i++) {
		RG_AnimFrame *fr = &ani->frames[i];
		AG_TlistItem *it;
		
		it = AG_TlistAdd(tl, NULL, _("Frame %ux%u, %ums"),
		    fr->su->w, fr->su->h, fr->delay);
		it->p1 = fr;
		it->class = "frame";
		AG_TlistSetIcon(tl, it, fr->su);
	}
	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
poll_tiles(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Tileset *ts = argv[1].p;
	AG_TlistItem *it;
	RG_Tile *t;

	AG_TlistClear(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = AG_TlistAdd(tl, NULL, "%s (%ux%u)%s%s", t->name,
		    t->su->w, t->su->h,
		    (t->su->flags & SDL_SRCALPHA) ? " alpha" : "",
		    (t->su->flags & SDL_SRCCOLORKEY) ? " colorkey" : "");
		it->p1 = t;
		AG_TlistSetIcon(tl, it, t->su);
	}

	pthread_mutex_unlock(&ts->lock);
	AG_TlistRestore(tl);
}

static void
select_insn_tile(int argc, union evarg *argv)
{
	RG_Anim *ani = argv[1].p;
	RG_AnimInsn *insn = argv[2].p;
	RG_Tileview *tv = argv[3].p;
	AG_TlistItem *it = argv[4].p;

	if (it != NULL) {
		insn->t = (RG_Tile *)it->p1;
		RG_TileviewSetTile(tv, insn->t);
	}
}

static void
open_insn(RG_Anim *ani, RG_AnimInsn *insn, AG_Box *box)
{
	AG_Widget *child;
	AG_Spinbutton *sb;
	AG_MSpinbutton *msb;
	AG_Checkbox *cb;
	AG_Combo *com;
	RG_Tileview *tv;

	AGOBJECT_FOREACH_CHILD(child, box, ag_widget) {
		AG_ObjectDetach(child);
		AG_ObjectDestroy(child);
		Free(child, M_OBJECT);
	}

	switch (insn->type) {
	case RG_ANIM_TILE:
		tv = Malloc(sizeof(RG_Tileview), M_OBJECT);
		RG_TileviewInit(tv, ani->tileset, 0);

		com = AG_ComboNew(box, AG_COMBO_POLL, _("Tile: "));
		AG_SetEvent(com, "combo-selected", select_insn_tile,
		    "%p,%p,%p", ani, insn, tv);
		AG_SetEvent(com->list, "tlist-poll", poll_tiles,
		    "%p", ani->tileset);
		if (insn->t != NULL) {
			RG_TileviewSetTile(tv, insn->t);
			AG_ComboSelectPointer(com, insn->t);
		}

		AG_LabelNew(box, AG_LABEL_STATIC, _("Preview:"));
		AG_ObjectAttach(box, tv);
		
		sb = AG_SpinbuttonNew(box, _("Alpha: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT,
		    &insn->in_tile.alpha);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SpinbuttonSetMax(sb, 255);
		break;
	case RG_ANIM_DISPX:
		msb = AG_MSpinbuttonNew(box, ",", _("Displacement: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_INT, &insn->in_disPx.dx);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_INT, &insn->in_disPx.dy);
		break;
	case RG_ANIM_ROTPX:
		msb = AG_MSpinbuttonNew(box, ",", _("Center of rotation: "));
		AG_WidgetBind(msb, "xvalue", AG_WIDGET_UINT, &insn->in_rotPx.x);
		AG_WidgetBind(msb, "yvalue", AG_WIDGET_UINT, &insn->in_rotPx.y);
		
		sb = AG_SpinbuttonNew(box, _("Angle of rotation: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT,
		    &insn->in_rotPx.theta);
		break;
	default:
		break;
	}

	sb = AG_SpinbuttonNew(box, _("Delay (ms): "));
	AG_WidgetBind(sb, "value", AG_WIDGET_UINT, &insn->delay);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SpinbuttonSetIncrement(sb, 50);

	AG_WINDOW_UPDATE(AG_WidgetParentWindow(box));
}

static void
open_frame(RG_Anim *ani, RG_AnimFrame *fr, AG_Box *box)
{
	AG_Widget *child;
	AG_Spinbutton *sb;
	AG_Bitmap *bmp;

	AGOBJECT_FOREACH_CHILD(child, box, ag_widget) {
		AG_ObjectDetach(child);
		AG_ObjectDestroy(child);
		Free(child, M_OBJECT);
	}
	
	ani->gframe = fr->name;

	bmp = AG_BitmapNew(box);
	AG_BitmapSetSurface(bmp, AG_DupSurface(fr->su));
	
	sb = AG_SpinbuttonNew(box, _("Delay (ms): "));
	AG_WidgetBind(sb, "value", AG_WIDGET_UINT, &fr->delay);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SpinbuttonSetIncrement(sb, 50);

	AG_WINDOW_UPDATE(AG_WidgetParentWindow(box));
}

static void
select_insn(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Anim *ani = argv[1].p;
	AG_Box *box = argv[2].p;
	AG_TlistItem *it, *eit;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	open_insn(ani, (RG_AnimInsn *)it->p1, box);
}

static void
select_frame(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	RG_Anim *ani = argv[1].p;
	AG_Box *box = argv[2].p;
	AG_TlistItem *it;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	open_frame(ani, (RG_AnimFrame *)it->p1, box);
}

static void
insert_insn(int argc, union evarg *argv)
{
	RG_Anim *ani = argv[1].p;
	enum rg_anim_insn_type type = (enum rg_anim_insn_type)argv[2].i;
	AG_Box *box = argv[3].p;
	AG_Tlist *tl = argv[4].p;
	AG_TlistItem *it;
	RG_AnimInsn *insn;
	
	insn = &ani->insns[RG_AnimInsertInsn(ani, type)];
	open_insn(ani, insn, box);
	AG_TlistSelectPtr(tl, insn);
}

static void
recompile_anim(int argc, union evarg *argv)
{
	RG_Anim *ani = argv[1].p;
	
	RG_AnimGenerate(ani);
}

static void
preview_anim(int argc, union evarg *argv)
{
	RG_Anim *ani = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_Window *win;
	RG_Animview *av;

	if ((win = AG_WindowNew(AG_WINDOW_DETACH, "anim-prev-%s:%s",
	    AGOBJECT(ani->tileset)->name, ani->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "%s", ani->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_CENTER, 1);
	AG_WindowAttach(pwin, win);

	av = RG_AnimviewNew(win);
	RG_AnimviewSetAnimation(av, ani);
	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);
	AG_LabelNew(win, AG_LABEL_POLLED, " %u/%u", &av->frame, &ani->nframes);

	AG_WindowShow(win);
}

AG_Window *
RG_AnimEdit(RG_Anim *ani)
{
	RG_Tileset *ts = ani->tileset;
	AG_Window *win;
	AG_Box *box_h, *box_v, *box_data;
	AG_Textbox *tb;
	AG_Tlist *tl;
	AG_Button *btn;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Menu *me;
	AG_MenuItem *mi;
	int i;

	if ((win = AG_WindowNew(AG_WINDOW_DETACH, "animation-%s:%s",
	    AGOBJECT(ts)->name, ani->name)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Animation: %s"), ani->name);
	AG_SetEvent(win, "window-close", close_animation, "%p,%p", ts, ani);
	
	me = AG_MenuNew(win);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	nt = AG_NotebookAddTab(nb, _("Instructions"), AG_BOX_VERT);
	{
		box_h = AG_BoxNew(nt, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HFILL);
		{
			box_v = AG_BoxNew(box_h, AG_BOX_VERT, AG_BOX_HFILL);
			tl = AG_TlistNew(box_v, AG_TLIST_POLL);
			AG_SetEvent(tl, "tlist-poll", poll_insns, "%p", ani);

			box_data = AG_BoxNew(box_h, AG_BOX_VERT,
			    AG_BOX_WFILL|AG_BOX_HFILL);
			AG_SetEvent(tl, "tlist-dblclick", select_insn, "%p,%p",
			    ani, box_data);
		}

		mi = AG_MenuAddItem(me, _("Instructions"));
		for (i = 0; i < RG_ANIM_LAST; i++)
			AG_MenuAction(mi, _(insn_names[i]), -1, insert_insn,
			    "%p,%i,%p,%p", ani, i, box_data, tl);
	}
	
	nt = AG_NotebookAddTab(nb, _("Frames"), AG_BOX_VERT);
	{
		box_h = AG_BoxNew(nt, AG_BOX_HORIZ, AG_BOX_WFILL|AG_BOX_HFILL);
		{
			box_v = AG_BoxNew(box_h, AG_BOX_VERT, AG_BOX_HFILL);
			tl = AG_TlistNew(box_v, AG_TLIST_POLL);
			AG_SetEvent(tl, "tlist-poll", poll_frames, "%p", ani);

			box_data = AG_BoxNew(box_h, AG_BOX_VERT,
			    AG_BOX_WFILL|AG_BOX_HFILL);
			AG_SetEvent(tl, "tlist-dblclick", select_frame, "%p,%p",
			    ani, box_data);
		}
	}
	
	mi = AG_MenuAddItem(me, _("Animation"));
	{
		AG_MenuAction(mi, _("Recompile"), -1, recompile_anim,
		    "%p", ani);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Preview..."), -1, preview_anim,
		    "%p,%p", ani, win);
	}
	
	AG_WindowScale(win, -1, -1);
	AG_WindowSetGeometry(win,
	    agView->w/4, agView->h/4,
	    agView->w/2, agView->h/2);

	RG_AnimGenerate(ani);
	ani->nrefs++;
	return (win);
}
#endif /* EDITION */
