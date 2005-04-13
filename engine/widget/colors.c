/*	$Csoft: colors.c,v 1.5 2005/04/11 12:12:47 vedge Exp $	*/

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

#include "colors.h"

Uint32 colors[LAST_COLOR];
Uint32 colors_border[7];
int colors_border_size = 7;

const char *colors_names[] = {
	N_("Background"),
	N_("Frame"),
	N_("Text"),
	N_("Window background"),
	N_("Window highlight"),
	N_("Window lowlight"),
	N_("Titlebar (focused)"),
	N_("Titlebar (unfocused)"),
	N_("Titlebar caption"),
	N_("Button"),
	N_("Button text"),
	N_("Button (disabled)"),
	N_("Checkbox"),
	N_("Checkbox text"),
	N_("Graph"),
	N_("Graph X-axis"),
	N_("HSV palette circle"),
	N_("HSV palette tiling 1"),
	N_("HSV palette tiling 2"),
	N_("Menu"),
	N_("Menu selection"),
	N_("Menu option"),
	N_("Menu text"),
	N_("Menu separator 1"),
	N_("Menu separator 2"),
	N_("Notebook"),
	N_("Notebook selection"),
	N_("Notebook tab text"),
	N_("Radio selection"),
	N_("Radio overlap"),
	N_("Radio highlight"),
	N_("Radio lowlight"),
	N_("Radio text"),
	N_("Scrollbar"),
	N_("Scrollbar buttons"),
	N_("Separator line 1"),
	N_("Separator line 2"),
	N_("Tableview"),
	N_("Tableview header"),
	N_("Tableview header text"),
	N_("Tableview cell text"),
	N_("Tableview line"),
	N_("Tableview selection"),
	N_("Textbox (read/write)"),
	N_("Textbox (read-only)"),
	N_("Textbox text"),
	N_("Textbox cursor"),
	N_("Tlist text"),
	N_("Tlist"),
	N_("Tlist line"),
	N_("Tlist selection"),
	N_("Mapview grid"),
	N_("Mapview cursor"),
	N_("Mapview tiling 1"),
	N_("Mapview tiling 2"),
	N_("Mapview mouse selection"),
	N_("Mapview effective selection"),
	N_("Tileview tiling 1"),
	N_("Tileview tiling 2"),
	N_("Tileview text background"),
	N_("Tileview text")
};

void
colors_init(void)
{
	colors[BG_COLOR] = SDL_MapRGB(vfmt, 0, 0, 0);
	colors[FRAME_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[TEXT_COLOR] = SDL_MapRGB(vfmt, 250, 250, 250);
	colors[WINDOW_BG_COLOR] = SDL_MapRGB(vfmt, 0, 0, 0);
	colors[WINDOW_HI_COLOR] = SDL_MapRGB(vfmt, 90, 90, 85);
	colors[WINDOW_LO_COLOR] = SDL_MapRGB(vfmt, 36, 36, 36);
	colors[TITLEBAR_FOCUSED_COLOR] = SDL_MapRGB(vfmt, 40, 50, 60);
	colors[TITLEBAR_UNFOCUSED_COLOR] = SDL_MapRGB(vfmt, 35, 35, 35);
	colors[TITLEBAR_CAPTION_COLOR] = colors[TEXT_COLOR];
	colors[BUTTON_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[BUTTON_TXT_COLOR] = SDL_MapRGB(vfmt, 240, 240, 240);
	colors[BUTTON_DIS_COLOR] = SDL_MapRGB(vfmt, 110, 110, 110);
	colors[CHECKBOX_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[CHECKBOX_TXT_COLOR] = colors[TEXT_COLOR];
	colors[GRAPH_BG_COLOR] = SDL_MapRGB(vfmt, 50, 50, 50);
	colors[GRAPH_XAXIS_COLOR] = SDL_MapRGB(vfmt, 50, 50, 50);
	colors[HSVPAL_CIRCLE_COLOR] = SDL_MapRGB(vfmt, 0, 0, 0);
	colors[HSVPAL_TILE1_COLOR] = SDL_MapRGB(vfmt, 140, 140, 140);
	colors[HSVPAL_TILE2_COLOR] = SDL_MapRGB(vfmt, 80, 80, 80);
	colors[MENU_UNSEL_COLOR] = SDL_MapRGB(vfmt, 70, 70, 70);
	colors[MENU_SEL_COLOR] = SDL_MapRGB(vfmt, 40, 40, 110);
	colors[MENU_OPTION_COLOR] = SDL_MapRGB(vfmt, 50, 50, 120);
	colors[MENU_TXT_COLOR] = SDL_MapRGB(vfmt, 230, 230, 230);
	colors[MENU_SEP1_COLOR] = SDL_MapRGB(vfmt, 60, 60, 60);
	colors[MENU_SEP2_COLOR] = SDL_MapRGB(vfmt, 120, 120, 120);
	colors[NOTEBOOK_BG_COLOR] = SDL_MapRGB(vfmt, 60, 60, 60);
	colors[NOTEBOOK_SEL_COLOR] = SDL_MapRGB(vfmt, 70, 70, 70);
	colors[NOTEBOOK_TXT_COLOR] = colors[TEXT_COLOR];
	colors[RADIO_SEL_COLOR] = SDL_MapRGB(vfmt, 210, 210, 210);
	colors[RADIO_OVER_COLOR] = SDL_MapRGB(vfmt, 90, 90, 90);
	colors[RADIO_HI_COLOR] = SDL_MapRGB(vfmt, 180, 180, 180);
	colors[RADIO_LO_COLOR] = SDL_MapRGB(vfmt, 80, 80, 80);
	colors[RADIO_TXT_COLOR] = colors[TEXT_COLOR];
	colors[SCROLLBAR_COLOR] = SDL_MapRGB(vfmt, 120, 120, 120);
	colors[SCROLLBAR_BTN_COLOR] = colors[BUTTON_COLOR];
	colors[SEPARATOR_LINE1_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[SEPARATOR_LINE2_COLOR] = SDL_MapRGB(vfmt, 25, 25, 25);
	colors[TABLEVIEW_COLOR] = colors[FRAME_COLOR];
	colors[TABLEVIEW_HEAD_COLOR] = colors[FRAME_COLOR];
	colors[TABLEVIEW_HTXT_COLOR] = colors[TEXT_COLOR];
	colors[TABLEVIEW_CTXT_COLOR] = colors[TEXT_COLOR];
	colors[TABLEVIEW_LINE_COLOR] = SDL_MapRGB(vfmt, 50, 50, 50);
	colors[TABLEVIEW_SEL_COLOR] = SDL_MapRGB(vfmt, 50, 50, 120);
	colors[TEXTBOX_RW_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[TEXTBOX_RO_COLOR] = SDL_MapRGB(vfmt, 40, 40, 40);
	colors[TEXTBOX_TXT_COLOR] = colors[TEXT_COLOR];
	colors[TEXTBOX_CURSOR_COLOR] = SDL_MapRGB(vfmt, 40, 40, 40);
	colors[TLIST_TXT_COLOR] = colors[TEXT_COLOR];
	colors[TLIST_BG_COLOR] = colors[FRAME_COLOR];
	colors[TLIST_LINE_COLOR] = SDL_MapRGB(vfmt, 40, 40, 40);
	colors[TLIST_SEL_COLOR] = SDL_MapRGB(vfmt, 50, 50, 120);
	colors[MAPVIEW_GRID_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[MAPVIEW_CURSOR_COLOR] = SDL_MapRGB(vfmt, 100, 100, 100);
	colors[MAPVIEW_TILE1_COLOR] = SDL_MapRGB(vfmt, 90, 90, 90);
	colors[MAPVIEW_TILE2_COLOR] = SDL_MapRGB(vfmt, 148, 146, 147);
	colors[MAPVIEW_MSEL_COLOR] = SDL_MapRGB(vfmt, 150, 150, 150);
	colors[MAPVIEW_ESEL_COLOR] = SDL_MapRGB(vfmt, 180, 180, 180);
	colors[TILEVIEW_TILE1_COLOR] = SDL_MapRGB(vfmt, 140, 140, 140);
	colors[TILEVIEW_TILE2_COLOR] = SDL_MapRGB(vfmt, 80, 80, 80);
	colors[TILEVIEW_TEXTBG_COLOR] = SDL_MapRGB(vfmt, 0, 0, 0);
	colors[TILEVIEW_TEXT_COLOR] = SDL_MapRGB(vfmt, 240, 240, 240);

	colors_border[0] = SDL_MapRGB(vfmt, 92, 92, 92);
	colors_border[1] = SDL_MapRGB(vfmt, 80, 80, 75);
	colors_border[2] = SDL_MapRGB(vfmt, 85, 85, 80);
	colors_border[3] = SDL_MapRGB(vfmt, 100, 100, 95);
	colors_border[4] = SDL_MapRGB(vfmt, 85, 85, 80);
	colors_border[5] = SDL_MapRGB(vfmt, 80, 80, 75);
	colors_border[6] = SDL_MapRGB(vfmt, 0, 255, 0);
}

void
colors_load(struct netbuf *buf)
{
	int i, ncolors;

	ncolors = (int)read_uint32(buf);
	for (i = 0; i < ncolors; i++)
		colors[i] = read_color(buf, vfmt);

	ncolors = (int)read_uint32(buf);
	for (i = 0; i < ncolors; i++)
		colors_border[i] = read_color(buf, vfmt);
}

void
colors_save(struct netbuf *buf)
{
	int i;

	write_uint32(buf, LAST_COLOR);
	for (i = 0; i < LAST_COLOR; i++) {
		write_color(buf, vfmt, colors[i]);
	}
	write_uint32(buf, (Uint32)colors_border_size);
	for (i = 0; i < LAST_COLOR; i++) {
		write_color(buf, vfmt, colors_border[i]);
	}
}

void
colors_destroy(void)
{
}
