/*	$Csoft: animview.c,v 1.1 2005/03/24 04:02:07 vedge Exp $	*/

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

#include "animview.h"

const struct widget_ops animview_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		widget_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	animview_draw,
	animview_scale
};

struct animview *
animview_new(void *parent)
{
	struct animview *av;

	av = Malloc(sizeof(struct animview), M_OBJECT);
	animview_init(av);
	object_attach(parent, av);
	return (av);
}

static void
do_play(int argc, union evarg *argv)
{
	struct animview *av = argv[1].p;

	timeout_replace(av, &av->timer, 1);
	button_disable(av->btns.play);
	button_enable(av->btns.pause);
	button_enable(av->btns.stop);
}

static void
do_pause(int argc, union evarg *argv)
{
	struct animview *av = argv[1].p;
	
	timeout_del(av, &av->timer);
	button_enable(av->btns.play);
	button_disable(av->btns.pause);
	button_disable(av->btns.stop);
}

static void
do_stop(int argc, union evarg *argv)
{
	struct animview *av = argv[1].p;
	
	timeout_del(av, &av->timer);
	av->frame = 0;
	
	button_enable(av->btns.play);
	button_disable(av->btns.pause);
	button_disable(av->btns.stop);
}

static Uint32
tick(void *p, Uint32 ival, void *arg)
{
	struct animview *av = p;

	if (av->anim->nframes == 0)
		return (0);

	if (++av->frame < av->anim->nframes) {
		struct anim_frame *fr = &av->anim->frames[av->frame];

		return (fr->delay > 0 ? fr->delay*100/av->speed : 1);
	} else {
		struct anim_frame *fr = &av->anim->frames[0];

		av->frame = 0;
		return (fr->delay > 0 ? fr->delay*100/av->speed: 1);
	}
}

static void
close_menu(struct animview *av)
{
	menu_collapse(av->menu, av->menu_item);
	object_destroy(av->menu);
	Free(av->menu, M_OBJECT);
	av->menu = NULL;
	av->menu_win = NULL;
	av->menu_item = NULL;
}

static void
set_speed(int argc, union evarg *argv)
{
	struct animview *av = argv[1].p;
	u_int factor = argv[2].i;

	av->speed = factor;
}

static void
open_menu(struct animview *av, int x, int y)
{
	if (av->menu != NULL)
		close_menu(av);
	
	av->menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(av->menu);
	av->menu_item = av->menu->sel_item = menu_add_item(av->menu, NULL);
	{
		struct AGMenuItem *m_speed;

		menu_action(av->menu_item, _("Play"), ANIM_PLAY_ICON,
		    do_play, "%p", av);
		menu_action(av->menu_item, _("Pause"), ANIM_PAUSE_ICON,
		    do_pause, "%p", av);
		menu_action(av->menu_item, _("Stop"), ANIM_STOP_ICON,
		    do_stop, "%p", av);

		menu_separator(av->menu_item);
		m_speed = menu_action(av->menu_item, _("Playback speed"),
		    -1, NULL, NULL);
		{
			menu_action(m_speed, _("Quadruple"), -1,
			    set_speed, "%p, %i", av, 400);
			menu_action(m_speed, _("Triple"), -1,
			    set_speed, "%p, %i", av, 300);
			menu_action(m_speed, _("Double"), -1,
			    set_speed, "%p, %i", av, 200);
			menu_action(m_speed, _("Normal"), -1,
			    set_speed, "%p, %i", av, 100);
			menu_action(m_speed, _("Half"), -1,
			    set_speed, "%p, %i", av, 50);
			menu_action(m_speed, _("One third"), -1,
			    set_speed, "%p, %i", av, 33);
			menu_action(m_speed, _("One quarter"), -1,
			    set_speed, "%p, %i", av, 25);
		}
	}

	av->menu_win = menu_expand(av->menu, av->menu_item, x, y);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct animview *av = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (button == SDL_BUTTON_RIGHT &&
	    y < WIDGET(av)->h - WIDGET(av->btns.play)->h) {
		open_menu(av, WIDGET(av)->cx+x, WIDGET(av)->cy+y);
	}
}

void
animview_init(struct animview *av)
{
	widget_init(av, "animview", &animview_ops, 0);
	WIDGET(av)->flags |= WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL;
	av->pre_w = 64;
	av->pre_h = 64;
	av->ranim.x = 0;
	av->ranim.y = 0;
	av->ranim.w = av->pre_w;
	av->ranim.h = av->pre_h;
	av->speed = 100;
	av->menu = NULL;
	av->menu_win = NULL;
	timeout_set(&av->timer, tick, av, 0);
	
	av->btns.play = button_new(av, NULL);
	button_set_label(av->btns.play, ICON(ANIM_PLAY_ICON));
	event_new(av->btns.play, "button-pushed", play, "%p", av);
	
	av->btns.pause = button_new(av, NULL);
	button_set_label(av->btns.pause, ICON(ANIM_PAUSE_ICON));
	event_new(av->btns.pause, "button-pushed", pause, "%p", av);
	
	av->btns.stop = button_new(av, NULL);
	button_set_label(av->btns.stop, ICON(ANIM_STOP_ICON));
	event_new(av->btns.stop, "button-pushed", stop, "%p", av);
	
	button_enable(av->btns.play);
	button_disable(av->btns.pause);
	button_disable(av->btns.stop);
	
	event_new(av, "window-mousebuttondown", mousebuttondown, NULL);
}

void
animview_prescale(struct animview *av, int w, int h)
{
	av->pre_w = w;
	av->pre_h = h;
}

void
animview_scale(void *p, int rw, int rh)
{
	struct animview *av = p;
	int bw, bh;
	
	if (rw == -1 && rh == -1) {
		WIDGET_SCALE(av->btns.play, -1, -1);
		WIDGET_SCALE(av->btns.pause, -1, -1);
		WIDGET_SCALE(av->btns.stop, -1, -1);

		WIDGET(av)->w = MAX(av->pre_w, WIDGET(av->btns.play)->w*3);
		WIDGET(av)->h = av->pre_h + WIDGET(av->btns.play)->h;
		return;
	}
	bw = rw/3;
	bh = WIDGET(av->btns.play)->h;

	WIDGET(av->btns.play)->x = 0;
	WIDGET(av->btns.play)->y = rh - bh;
	WIDGET(av->btns.play)->w = bw;

	WIDGET(av->btns.pause)->x = bw;
	WIDGET(av->btns.pause)->y = rh - bh;
	WIDGET(av->btns.pause)->w = bw;
	
	WIDGET(av->btns.stop)->x = bw*2;
	WIDGET(av->btns.stop)->y = rh - bh;
	WIDGET(av->btns.stop)->w = bw;

	av->ranim.x = rw/2 - av->ranim.w/2;
	av->ranim.y = (rh - bh)/2 - av->ranim.h/2;
}

void
animview_draw(void *p)
{
	struct animview *av = p;
	struct anim_frame *fr;

	if (av->anim == NULL || av->anim->nframes < 1) {
		return;
	}
	fr = &av->anim->frames[av->frame];
	if (fr->su != NULL)
		widget_blit(av, fr->su, av->ranim.x, av->ranim.y);
}

void
animview_set_animation(struct animview *av, struct animation *anim)
{
	timeout_del(av, &av->timer);
	av->anim = anim;
	av->frame = 0;
	av->ranim.x = WIDGET(av)->w/2 - anim->w/2;
	av->ranim.y = (WIDGET(av)->h - WIDGET(av->btns.play)->h)/2 - anim->h/2;
	av->ranim.w = anim->w;
	av->ranim.h = anim->h;
}

