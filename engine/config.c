/*	$Csoft	    */

/*
 * Copyright (c) 2002 CubeSoft Communications <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

#include <engine/widget/text.h>
#include <engine/widget/window.h>
#include <engine/widget/widget.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/checkbox.h>

static void	close_button_push(struct button *);
static void	fullscrn_cbox_push(struct checkbox *);

static void
close_button_push(struct button *b)
{
	view_detach(mainview, WIDGET(b)->win);
}

static void
fullscrn_cbox_push(struct checkbox *b)
{
	pthread_mutex_lock(&world->lock);
	view_fullscreen(world->curmap->view,
	    (world->curmap->flags & SDL_FULLSCREEN) ? 0 : 1);
	pthread_mutex_unlock(&world->lock);
}

static void
color_button_push(struct button *b)
{
	text_msg(10, TEXT_SLEEP, "test\n");
}

void
engine_config(void)
{
	char sharetxt[2048];
	struct window *win;
	struct checkbox *fullscr_cbox;
	struct button *close_button, *color_button;
	struct label *sharedir_label;
	struct region *body_reg, *buttons_reg;

	/* Settings window */
	win = window_new("Engine settings", WINDOW_TITLEBAR, WINDOW_GRADIENT,
	    64, 64, 512, 256);
	body_reg = region_new(win, WIDGET_VALIGN, 0, 0, 100, 80, 10);
	buttons_reg = region_new(win, WIDGET_HALIGN, 0, 80, 100, 20, 10);

	sprintf(sharetxt, "Engine: %d objects, showing \"%s\".", world->nobjs,
	   OBJECT(world->curmap)->name);
	sharedir_label = label_new(body_reg, sharetxt, 0);

	fullscr_cbox = checkbox_new(body_reg, "Full-screen mode", 0);
	fullscr_cbox->push = fullscrn_cbox_push;

	color_button = button_new(buttons_reg, "Colors", 0, 128, 32);
	color_button->push = color_button_push;
	
	close_button = button_new(buttons_reg, "Close", 0, 128, 32);
	close_button->push = close_button_push;
}

