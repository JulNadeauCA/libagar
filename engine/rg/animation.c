/*	$Csoft: animation.c,v 1.4 2005/08/29 02:56:44 vedge Exp $	*/

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
animation_init(struct animation *ani, struct tileset *ts, const char *name,
    int flags)
{
	strlcpy(ani->name, name, sizeof(ani->name));
	ani->flags = flags;
	ani->w = 0;
	ani->h = 0;
	ani->tileset = ts;
	ani->nrefs = 0;

	ani->insns = Malloc(sizeof(struct anim_insn), M_RG);
	ani->ninsns = 0;
	ani->frames = Malloc(sizeof(struct anim_frame), M_RG);
	ani->nframes = 0;

	ani->gframe = 0;
}

void
animation_scale(struct animation *ani, u_int w, u_int h)
{
	ani->w = w;
	ani->h = h;

	/* TODO scale existing frames */
}

u_int
anim_insert_insn(struct animation *ani, enum anim_insn_type type)
{
	struct anim_insn *insn;

	ani->insns = Realloc(ani->insns,
	    (ani->ninsns+1)*sizeof(struct anim_insn));
	insn = &ani->insns[ani->ninsns];
	insn->type = type;
	insn->t = NULL;
	insn->px = NULL;
	insn->sk = NULL;
	insn->delay = 100;

	switch (type) {
	case ANIM_TILE:
		insn->in_tile.alpha = SDL_ALPHA_OPAQUE;
		break;
	case ANIM_DISPX:
		insn->in_disPx.dx = 0;
		insn->in_disPx.dy = 0;
		break;
	case ANIM_ROTPX:
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
destroy_insn(struct anim_insn *insn)
{
	if (insn->t != NULL)	insn->t->nrefs--;
	if (insn->px != NULL)	insn->px->nrefs--;
	if (insn->sk != NULL)	insn->sk->nrefs--;
}

void
anim_remove_insn(struct animation *ani, u_int insn)
{
	destroy_insn(&ani->insns[insn]);
	if (insn+1 < ani->ninsns)
		memmove(&ani->insns[insn], &ani->insns[insn+1],
		    (--ani->ninsns)*sizeof(struct anim_insn));
}

u_int
anim_insert_frame(struct animation *ani)
{
	struct tileset *ts = ani->tileset;
	Uint32 sflags = SDL_SWSURFACE;
	struct anim_frame *fr;
	
	if (ani->flags & ANIMATION_SRCCOLORKEY)	sflags |= SDL_SRCCOLORKEY;
	if (ani->flags & ANIMATION_SRCALPHA)	sflags |= SDL_SRCALPHA;

	ani->frames = Realloc(ani->frames,
	    (ani->nframes+1)*sizeof(struct anim_frame));
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
destroy_frame(struct anim_frame *fr)
{
	SDL_FreeSurface(fr->su);
}

void
anim_remove_frame(struct animation *ani, u_int frame)
{
	destroy_frame(&ani->frames[frame]);
	if (frame+1 < ani->nframes)
		memmove(&ani->frames[frame], &ani->frames[frame+1],
		    (--ani->nframes)*sizeof(struct anim_frame));
}

static void
destroy_frames(struct animation *ani)
{
	u_int i;
	
	for (i = 0; i < ani->nframes; i++) {
		destroy_frame(&ani->frames[i]);
	}
	ani->nframes = 0;
}

void
animation_destroy(struct animation *ani)
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
animation_load(struct animation *ani, struct netbuf *buf)
{
	struct tileset *ts = ani->tileset;
	Uint32 i, ninsns;
	
	ani->w = read_uint16(buf);
	ani->h = read_uint16(buf);

	ninsns = read_uint32(buf);
	for (i = 0; i < ninsns; i++) {
		char name[TILESET_NAME_MAX];
		enum anim_insn_type type;
		struct anim_insn *insn;
		
		type = (enum anim_insn_type)read_uint16(buf);
		insn = &ani->insns[anim_insert_insn(ani, type)];
		insn->delay = (u_int)read_uint32(buf);

		switch (type) {
		case ANIM_TILE:
			copy_string(name, buf, sizeof(name));
			insn->t = tileset_find_tile(ts, name);
			insn->in_tile.alpha = (u_int)read_uint8(buf);
			break;
		case ANIM_DISPX:
			copy_string(name, buf, sizeof(name));
			insn->px = tileset_find_pixmap(ts, name);
			insn->in_disPx.dx = (int)read_sint16(buf);
			insn->in_disPx.dy = (int)read_sint16(buf);
			break;
		case ANIM_ROTPX:
			copy_string(name, buf, sizeof(name));
			insn->px = tileset_find_pixmap(ts, name);
			insn->in_rotPx.x = (u_int)read_uint16(buf);
			insn->in_rotPx.y = (u_int)read_uint16(buf);
			insn->in_rotPx.theta = (int)read_uint8(buf);
			break;
		}
	}

	ani->nframes = (u_int)read_uint32(buf);
	ani->frames = Realloc(ani->frames,
	    ani->nframes*sizeof(struct anim_frame));
	for (i = 0; i < ani->nframes; i++) {
		struct anim_frame *fr = &ani->frames[i];

		fr->name = i;
		fr->su = read_surface(buf, ts->fmt);
		fr->delay = (u_int)read_uint32(buf);
	}
	return (0);
}

void
animation_save(struct animation *ani, struct netbuf *buf)
{
	u_int i;
	
	write_uint16(buf, ani->w);
	write_uint16(buf, ani->h);
	
	write_uint32(buf, ani->ninsns);
	for (i = 0; i < ani->ninsns; i++) {
		struct anim_insn *insn = &ani->insns[i];
		
		write_uint16(buf, (Uint16)insn->type);
		write_uint32(buf, (Uint32)insn->delay);
		switch (insn->type) {
		case ANIM_TILE:
			write_string(buf, insn->t->name);
			write_uint8(buf, (Uint8)insn->in_tile.alpha);
			break;
		case ANIM_DISPX:
			write_string(buf, insn->px->name);
			write_sint16(buf, (Sint16)insn->in_disPx.dx);
			write_sint16(buf, (Sint16)insn->in_disPx.dy);
			break;
		case ANIM_ROTPX:
			write_string(buf, insn->px->name);
			write_uint16(buf, (Uint16)insn->in_rotPx.x);
			write_uint16(buf, (Uint16)insn->in_rotPx.y);
			write_uint8(buf, (Uint8)insn->in_rotPx.theta);
			break;
		}
	}

	write_uint32(buf, ani->nframes);
	for (i = 0; i < ani->nframes; i++) {
		struct anim_frame *fr = &ani->frames[i];

		write_surface(buf, fr->su);
		write_uint32(buf, (Uint32)fr->delay);
	}
}

void
animation_generate(struct animation *ani)
{
	u_int i;

	destroy_frames(ani);

	for (i = 0; i < ani->ninsns; i++) {
		struct anim_insn *insn = &ani->insns[i];
		struct anim_frame *fr;

		switch (insn->type) {
		case ANIM_TILE:
			if (insn->t != NULL && insn->t->su != NULL) {
				fr = &ani->frames[anim_insert_frame(ani)];
				view_scale_surface(insn->t->su, ani->w, ani->h,
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
	struct window *win = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct animation *ani = argv[2].p;
	
	pthread_mutex_lock(&ts->lock);
	ani->nrefs--;
	pthread_mutex_unlock(&ts->lock);

	view_detach(win);
}

static void
poll_insns(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct animation *ani = argv[1].p;
	struct tileset *ts = ani->tileset;
	struct tlist_item *it;
	u_int i;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	for (i = 0; i < ani->ninsns; i++) {
		struct anim_insn *insn = &ani->insns[i];

		switch (insn->type) {
		case ANIM_TILE:
			it = tlist_insert(tl, NULL, _("[%04u] Tile <%s>"),
			    insn->delay,
			    insn->t != NULL ? insn->t->name : "(null)");
			tlist_set_icon(tl, it,
			    insn->t != NULL ? insn->t->su : NULL);
			break;
		case ANIM_DISPX:
			it = tlist_insert(tl, NULL, _("[%04u] Displace <%s>"),
			    insn->delay,
			    insn->px != NULL ? insn->px->name : "(null)");
			tlist_set_icon(tl, it,
			    insn->px != NULL ? insn->px->su : NULL);
			break;
		case ANIM_ROTPX:
			it = tlist_insert(tl, NULL,
			    _("[%04u] Rotate <%s> %u\xc2\xb0"), insn->delay,
			    insn->px != NULL ? insn->px->name : "(null)",
			    insn->in_rotPx.theta);
			tlist_set_icon(tl, it,
			    insn->px != NULL ? insn->px->su : NULL);
			break;
		default:
			it = tlist_insert(tl, NULL, "[%04u] %s",
			    _(insn_names[insn->type]), insn->delay);
			break;
		}
		it->p1 = insn;
		it->class = "insn";
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
poll_frames(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct animation *ani = argv[1].p;
	struct tileset *ts = ani->tileset;
	u_int i;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);
	for (i = 0; i < ani->nframes; i++) {
		struct anim_frame *fr = &ani->frames[i];
		struct tlist_item *it;
		
		it = tlist_insert(tl, NULL, _("Frame %ux%u, %ums"),
		    fr->su->w, fr->su->h, fr->delay);
		it->p1 = fr;
		it->class = "frame";
		tlist_set_icon(tl, it, fr->su);
	}
	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
poll_tiles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tlist_item *it;
	struct tile *t;

	tlist_clear_items(tl);
	pthread_mutex_lock(&ts->lock);

	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		it = tlist_insert(tl, NULL, "%s (%ux%u)%s%s", t->name,
		    t->su->w, t->su->h,
		    (t->su->flags & SDL_SRCALPHA) ? " alpha" : "",
		    (t->su->flags & SDL_SRCCOLORKEY) ? " colorkey" : "");
		it->p1 = t;
		tlist_set_icon(tl, it, t->su);
	}

	pthread_mutex_unlock(&ts->lock);
	tlist_restore_selections(tl);
}

static void
select_insn_tile(int argc, union evarg *argv)
{
	struct animation *ani = argv[1].p;
	struct anim_insn *insn = argv[2].p;
	struct tileview *tv = argv[3].p;
	struct tlist_item *it = argv[4].p;

	if (it != NULL) {
		insn->t = (struct tile *)it->p1;
		tileview_set_tile(tv, insn->t);
	}
}

static void
open_insn(struct animation *ani, struct anim_insn *insn, struct box *box)
{
	struct widget *child;
	struct spinbutton *sb;
	struct mspinbutton *msb;
	struct checkbox *cb;
	struct combo *com;
	struct tileview *tv;

	OBJECT_FOREACH_CHILD(child, box, widget) {
		object_detach(child);
		object_destroy(child);
		Free(child, M_OBJECT);
	}

	switch (insn->type) {
	case ANIM_TILE:
		tv = Malloc(sizeof(struct tileview), M_OBJECT);
		tileview_init(tv, ani->tileset, 0);

		com = combo_new(box, COMBO_POLL, _("Tile: "));
		event_new(com, "combo-selected", select_insn_tile,
		    "%p,%p,%p", ani, insn, tv);
		event_new(com->list, "tlist-poll", poll_tiles,
		    "%p", ani->tileset);
		if (insn->t != NULL) {
			tileview_set_tile(tv, insn->t);
			combo_select_pointer(com, insn->t);
		}

		label_new(box, LABEL_STATIC, _("Preview:"));
		object_attach(box, tv);
		
		sb = spinbutton_new(box, _("Alpha: "));
		widget_bind(sb, "value", WIDGET_UINT, &insn->in_tile.alpha);
		spinbutton_set_min(sb, 0);
		spinbutton_set_max(sb, 255);
		break;
	case ANIM_DISPX:
		msb = mspinbutton_new(box, ",", _("Displacement: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &insn->in_disPx.dx);
		widget_bind(msb, "yvalue", WIDGET_INT, &insn->in_disPx.dy);
		break;
	case ANIM_ROTPX:
		msb = mspinbutton_new(box, ",", _("Center of rotation: "));
		widget_bind(msb, "xvalue", WIDGET_UINT, &insn->in_rotPx.x);
		widget_bind(msb, "yvalue", WIDGET_UINT, &insn->in_rotPx.y);
		
		sb = spinbutton_new(box, _("Angle of rotation: "));
		widget_bind(sb, "value", WIDGET_INT, &insn->in_rotPx.theta);
		break;
	default:
		break;
	}

	sb = spinbutton_new(box, _("Delay (ms): "));
	widget_bind(sb, "value", WIDGET_UINT, &insn->delay);
	spinbutton_set_min(sb, 0);
	spinbutton_set_increment(sb, 50);

	WINDOW_UPDATE(widget_parent_window(box));
}

static void
open_frame(struct animation *ani, struct anim_frame *fr, struct box *box)
{
	struct widget *child;
	struct spinbutton *sb;
	struct bitmap *bmp;

	OBJECT_FOREACH_CHILD(child, box, widget) {
		object_detach(child);
		object_destroy(child);
		Free(child, M_OBJECT);
	}
	
	ani->gframe = fr->name;

	bmp = bitmap_new(box);
	bitmap_set_surface(bmp, view_copy_surface(fr->su));
	
	sb = spinbutton_new(box, _("Delay (ms): "));
	widget_bind(sb, "value", WIDGET_UINT, &fr->delay);
	spinbutton_set_min(sb, 0);
	spinbutton_set_increment(sb, 50);

	WINDOW_UPDATE(widget_parent_window(box));
}

static void
select_insn(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct animation *ani = argv[1].p;
	struct box *box = argv[2].p;
	struct tlist_item *it, *eit;

	if ((it = tlist_selected_item(tl)) == NULL)
		return;

	open_insn(ani, (struct anim_insn *)it->p1, box);
}

static void
select_frame(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct animation *ani = argv[1].p;
	struct box *box = argv[2].p;
	struct tlist_item *it;

	if ((it = tlist_selected_item(tl)) == NULL)
		return;

	open_frame(ani, (struct anim_frame *)it->p1, box);
}

static void
insert_insn(int argc, union evarg *argv)
{
	struct animation *ani = argv[1].p;
	enum anim_insn_type type = (enum anim_insn_type)argv[2].i;
	struct box *box = argv[3].p;
	struct tlist *tl = argv[4].p;
	struct tlist_item *it;
	struct anim_insn *insn;
	
	insn = &ani->insns[anim_insert_insn(ani, type)];
	open_insn(ani, insn, box);
	tlist_select_pointer(tl, insn);
}

static void
recompile_anim(int argc, union evarg *argv)
{
	struct animation *ani = argv[1].p;
	
	animation_generate(ani);
}

static void
preview_anim(int argc, union evarg *argv)
{
	struct animation *ani = argv[1].p;
	struct window *pwin = argv[2].p;
	struct window *win;
	struct animview *av;

	if ((win = window_new(WINDOW_DETACH, "anim-prev-%s:%s",
	    OBJECT(ani->tileset)->name, ani->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s", ani->name);
	window_set_position(win, WINDOW_UPPER_CENTER, 1);
	window_attach(pwin, win);

	av = animview_new(win);
	animview_set_animation(av, ani);
	separator_new(win, SEPARATOR_HORIZ);
	label_new(win, LABEL_POLLED, " %u/%u", &av->frame, &ani->nframes);

	window_show(win);
}

struct window *
animation_edit(struct animation *ani)
{
	struct tileset *ts = ani->tileset;
	struct window *win;
	struct box *box_h, *box_v, *box_data;
	struct textbox *tb;
	struct tlist *tl;
	struct button *btn;
	struct notebook *nb;
	struct notebook_tab *nt;
	struct AGMenu *me;
	struct AGMenuItem *mi;
	int i;

	if ((win = window_new(WINDOW_DETACH, "animation-%s:%s",
	    OBJECT(ts)->name, ani->name)) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Animation: %s"), ani->name);
	event_new(win, "window-close", close_animation, "%p,%p", ts, ani);
	
	me = menu_new(win);

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	nt = notebook_add_tab(nb, _("Instructions"), BOX_VERT);
	{
		box_h = box_new(nt, BOX_HORIZ, BOX_WFILL|BOX_HFILL);
		{
			box_v = box_new(box_h, BOX_VERT, BOX_HFILL);
			tl = tlist_new(box_v, TLIST_POLL);
			event_new(tl, "tlist-poll", poll_insns, "%p", ani);

			box_data = box_new(box_h, BOX_VERT,
			    BOX_WFILL|BOX_HFILL);
			event_new(tl, "tlist-dblclick", select_insn, "%p,%p",
			    ani, box_data);
		}

		mi = menu_add_item(me, _("Instructions"));
		for (i = 0; i < ANIM_LAST; i++)
			menu_action(mi, _(insn_names[i]), -1, insert_insn,
			    "%p,%i,%p,%p", ani, i, box_data, tl);
	}
	
	nt = notebook_add_tab(nb, _("Frames"), BOX_VERT);
	{
		box_h = box_new(nt, BOX_HORIZ, BOX_WFILL|BOX_HFILL);
		{
			box_v = box_new(box_h, BOX_VERT, BOX_HFILL);
			tl = tlist_new(box_v, TLIST_POLL);
			event_new(tl, "tlist-poll", poll_frames, "%p", ani);

			box_data = box_new(box_h, BOX_VERT,
			    BOX_WFILL|BOX_HFILL);
			event_new(tl, "tlist-dblclick", select_frame, "%p,%p",
			    ani, box_data);
		}
	}
	
	mi = menu_add_item(me, _("Animation"));
	{
		menu_action(mi, _("Recompile"), -1, recompile_anim, "%p", ani);
		menu_separator(mi);
		menu_action(mi, _("Preview..."), -1, preview_anim, "%p,%p",
		    ani, win);
	}
	
	window_scale(win, -1, -1);
	window_set_geometry(win,
	    view->w/4, view->h/4,
	    view->w/2, view->h/2);

	animation_generate(ani);
	ani->nrefs++;
	return (win);
}
#endif /* EDITION */
