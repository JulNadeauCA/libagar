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
#include <core/view.h>

#include "animview.h"

const AG_WidgetOps rgAnimviewOps = {
	{
		"AG_Widget:RG_Animview",
		sizeof(RG_Animview),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	RG_AnimviewDraw,
	RG_AnimviewScale
};

RG_Animview *
RG_AnimviewNew(void *parent)
{
	RG_Animview *av;

	av = Malloc(sizeof(RG_Animview), M_OBJECT);
	RG_AnimviewInit(av);
	AG_ObjectAttach(parent, av);
	return (av);
}

static void
Play(AG_Event *event)
{
	RG_Animview *av = AG_PTR(1);

	AG_ReplaceTimeout(av, &av->timer, 1);
	AG_ButtonDisable(av->btns.play);
	AG_ButtonEnable(av->btns.pause);
	AG_ButtonEnable(av->btns.stop);
}

static void
Pause(AG_Event *event)
{
	RG_Animview *av = AG_PTR(1);
	
	AG_DelTimeout(av, &av->timer);
	AG_ButtonEnable(av->btns.play);
	AG_ButtonDisable(av->btns.pause);
	AG_ButtonDisable(av->btns.stop);
}

static void
Stop(AG_Event *event)
{
	RG_Animview *av = AG_PTR(1);
	
	AG_DelTimeout(av, &av->timer);
	av->frame = 0;
	
	AG_ButtonEnable(av->btns.play);
	AG_ButtonDisable(av->btns.pause);
	AG_ButtonDisable(av->btns.stop);
}

static Uint32
TickFrame(void *p, Uint32 ival, void *arg)
{
	RG_Animview *av = p;

	if (av->anim->nframes == 0)
		return (0);

	if (++av->frame < av->anim->nframes) {
		RG_AnimFrame *fr = &av->anim->frames[av->frame];

		return (fr->delay > 0 ? fr->delay*100/av->speed : 1);
	} else {
		RG_AnimFrame *fr = &av->anim->frames[0];

		av->frame = 0;
		return (fr->delay > 0 ? fr->delay*100/av->speed: 1);
	}
}

static void
CloseMenu(RG_Animview *av)
{
	AG_MenuCollapse(av->menu, av->menu_item);
	AG_ObjectDestroy(av->menu);
	Free(av->menu, M_OBJECT);
	av->menu = NULL;
	av->menu_win = NULL;
	av->menu_item = NULL;
}

static void
SetSpeed(AG_Event *event)
{
	RG_Animview *av = AG_PTR(1);
	Uint factor = AG_INT(2);

	av->speed = factor;
}

static void
OpenMenu(RG_Animview *av, int x, int y)
{
	if (av->menu != NULL)
		CloseMenu(av);
	
	av->menu = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(av->menu, 0);
	av->menu_item = av->menu->sel_item = AG_MenuAddItem(av->menu, NULL);
	{
		AG_MenuItem *m_speed;

		AG_MenuAction(av->menu_item, _("Play"), ANIM_PLAY_ICON,
		    Play, "%p", av);
		AG_MenuAction(av->menu_item, _("Pause"), ANIM_PAUSE_ICON,
		    Pause, "%p", av);
		AG_MenuAction(av->menu_item, _("Stop"), ANIM_STOP_ICON,
		    Stop, "%p", av);

		AG_MenuSeparator(av->menu_item);
		m_speed = AG_MenuAction(av->menu_item, _("Playback speed"),
		    -1, NULL, NULL);
		{
			AG_MenuAction(m_speed, _("Quadruple"), -1,
			    SetSpeed, "%p, %i", av, 400);
			AG_MenuAction(m_speed, _("Triple"), -1,
			    SetSpeed, "%p, %i", av, 300);
			AG_MenuAction(m_speed, _("Double"), -1,
			    SetSpeed, "%p, %i", av, 200);
			AG_MenuAction(m_speed, _("Normal"), -1,
			    SetSpeed, "%p, %i", av, 100);
			AG_MenuAction(m_speed, _("Half"), -1,
			    SetSpeed, "%p, %i", av, 50);
			AG_MenuAction(m_speed, _("One third"), -1,
			    SetSpeed, "%p, %i", av, 33);
			AG_MenuAction(m_speed, _("One quarter"), -1,
			    SetSpeed, "%p, %i", av, 25);
		}
	}

	av->menu_win = AG_MenuExpand(av->menu, av->menu_item, x, y);
}

static void
mousebuttondown(AG_Event *event)
{
	RG_Animview *av = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (button == SDL_BUTTON_RIGHT &&
	    y < AGWIDGET(av)->h - AGWIDGET(av->btns.play)->h) {
		OpenMenu(av, AGWIDGET(av)->cx+x, AGWIDGET(av)->cy+y);
	}
}

void
RG_AnimviewInit(RG_Animview *av)
{
	AG_WidgetInit(av, &rgAnimviewOps, 0);
	AGWIDGET(av)->flags |= AG_WIDGET_CLIPPING|AG_WIDGET_HFILL|
			       AG_WIDGET_VFILL;
	av->pre_w = 64;
	av->pre_h = 64;
	av->ranim.x = 0;
	av->ranim.y = 0;
	av->ranim.w = av->pre_w;
	av->ranim.h = av->pre_h;
	av->speed = 100;
	av->menu = NULL;
	av->menu_win = NULL;
	AG_SetTimeout(&av->timer, TickFrame, av, 0);
	
	av->btns.play = AG_ButtonNew(av, 0, NULL);
	AG_ButtonSetSurface(av->btns.play, AGICON(ANIM_PLAY_ICON));
	AG_SetEvent(av->btns.play, "button-pushed", Play, "%p", av);
	
	av->btns.pause = AG_ButtonNew(av, 0, NULL);
	AG_ButtonSetSurface(av->btns.pause, AGICON(ANIM_PAUSE_ICON));
	AG_SetEvent(av->btns.pause, "button-pushed", Pause, "%p", av);
	
	av->btns.stop = AG_ButtonNew(av, 0, NULL);
	AG_ButtonSetSurface(av->btns.stop, AGICON(ANIM_STOP_ICON));
	AG_SetEvent(av->btns.stop, "button-pushed", Stop, "%p", av);
	
	AG_ButtonEnable(av->btns.play);
	AG_ButtonDisable(av->btns.pause);
	AG_ButtonDisable(av->btns.stop);
	
	AG_SetEvent(av, "window-mousebuttondown", mousebuttondown, NULL);
}

void
RG_AnimviewPrescale(RG_Animview *av, int w, int h)
{
	av->pre_w = w;
	av->pre_h = h;
}

void
RG_AnimviewScale(void *p, int rw, int rh)
{
	RG_Animview *av = p;
	int bw, bh;
	
	if (rw == -1 && rh == -1) {
		AGWIDGET_SCALE(av->btns.play, -1, -1);
		AGWIDGET_SCALE(av->btns.pause, -1, -1);
		AGWIDGET_SCALE(av->btns.stop, -1, -1);

		AGWIDGET(av)->w = MAX(av->pre_w, AGWIDGET(av->btns.play)->w*3);
		AGWIDGET(av)->h = av->pre_h + AGWIDGET(av->btns.play)->h;
		return;
	}
	bw = rw/3;
	bh = AGWIDGET(av->btns.play)->h;

	AGWIDGET(av->btns.play)->x = 0;
	AGWIDGET(av->btns.play)->y = rh - bh;
	AGWIDGET(av->btns.play)->w = bw;

	AGWIDGET(av->btns.pause)->x = bw;
	AGWIDGET(av->btns.pause)->y = rh - bh;
	AGWIDGET(av->btns.pause)->w = bw;
	
	AGWIDGET(av->btns.stop)->x = bw*2;
	AGWIDGET(av->btns.stop)->y = rh - bh;
	AGWIDGET(av->btns.stop)->w = bw;

	av->ranim.x = rw/2 - av->ranim.w/2;
	av->ranim.y = (rh - bh)/2 - av->ranim.h/2;
}

void
RG_AnimviewDraw(void *p)
{
	RG_Animview *av = p;
	RG_AnimFrame *fr;

	if (av->anim == NULL || av->anim->nframes < 1) {
		return;
	}
	fr = &av->anim->frames[av->frame];
	if (fr->su != NULL)
		AG_WidgetBlit(av, fr->su, av->ranim.x, av->ranim.y);
}

void
RG_AnimviewSetAnimation(RG_Animview *av, RG_Anim *anim)
{
	AG_DelTimeout(av, &av->timer);
	av->anim = anim;
	av->frame = 0;
	av->ranim.x = AGWIDGET(av)->w/2 - anim->w/2;
	av->ranim.y = (AGWIDGET(av)->h - AGWIDGET(av->btns.play)->h)/2 -
	    anim->h/2;
	av->ranim.w = anim->w;
	av->ranim.h = anim->h;
}

