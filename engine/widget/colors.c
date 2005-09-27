/*	$Csoft: colors.c,v 1.23 2005/09/17 07:34:04 vedge Exp $	*/

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

Uint32 agColors[LAST_COLOR];
Uint32 agColorsBorder[7];
int    agColorsBorderSize = 7;

const char *agColorNames[] = {
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
	N_("Tileview text"),
	N_("Transparent color"),
	N_("HSV Palette bar #1"),
	N_("HSV Palette bar #2"),
	N_("Pane"),
	N_("Pane (circles)"),
	N_("Mapview noderef selection"),
	N_("Mapview origin point"),
	N_("Focus"),
};

void
AG_ColorsInit(void)
{
	agColors[BG_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[FRAME_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[TEXT_COLOR] = SDL_MapRGB(agVideoFmt, 250, 250, 250);
	agColors[WINDOW_BG_COLOR] = SDL_MapRGB(agVideoFmt, 45, 45, 45);
	agColors[WINDOW_HI_COLOR] = SDL_MapRGB(agVideoFmt, 90, 90, 85);
	agColors[WINDOW_LO_COLOR] = SDL_MapRGB(agVideoFmt, 36, 36, 36);
	agColors[TITLEBAR_FOCUSED_COLOR] = SDL_MapRGB(agVideoFmt, 40, 50, 60);
	agColors[TITLEBAR_UNFOCUSED_COLOR] = SDL_MapRGB(agVideoFmt, 35, 35, 35);
	agColors[TITLEBAR_CAPTION_COLOR] = agColors[TEXT_COLOR];
	agColors[BUTTON_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[BUTTON_TXT_COLOR] = SDL_MapRGB(agVideoFmt, 240, 240, 240);
	agColors[BUTTON_DIS_COLOR] = SDL_MapRGB(agVideoFmt, 110, 110, 110);
	agColors[CHECKBOX_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[CHECKBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[GRAPH_BG_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[GRAPH_XAXIS_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[HSVPAL_CIRCLE_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[HSVPAL_TILE1_COLOR] = SDL_MapRGB(agVideoFmt, 140, 140, 140);
	agColors[HSVPAL_TILE2_COLOR] = SDL_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[MENU_UNSEL_COLOR] = SDL_MapRGB(agVideoFmt, 70, 70, 70);
	agColors[MENU_SEL_COLOR] = SDL_MapRGB(agVideoFmt, 40, 40, 110);
	agColors[MENU_OPTION_COLOR] = SDL_MapRGB(agVideoFmt, 223, 207, 128);
	agColors[MENU_TXT_COLOR] = SDL_MapRGB(agVideoFmt, 230, 230, 230);
	agColors[MENU_SEP1_COLOR] = SDL_MapRGB(agVideoFmt, 60, 60, 60);
	agColors[MENU_SEP2_COLOR] = SDL_MapRGB(agVideoFmt, 120, 120, 120);
	agColors[NOTEBOOK_BG_COLOR] = SDL_MapRGB(agVideoFmt, 60, 60, 60);
	agColors[NOTEBOOK_SEL_COLOR] = SDL_MapRGB(agVideoFmt, 70, 70, 70);
	agColors[NOTEBOOK_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[RADIO_SEL_COLOR] = SDL_MapRGB(agVideoFmt, 210, 210, 210);
	agColors[RADIO_OVER_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[RADIO_HI_COLOR] = SDL_MapRGB(agVideoFmt, 180, 180, 180);
	agColors[RADIO_LO_COLOR] = SDL_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[RADIO_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[SCROLLBAR_COLOR] = SDL_MapRGB(agVideoFmt, 120, 120, 120);
	agColors[SCROLLBAR_BTN_COLOR] = agColors[BUTTON_COLOR];
	agColors[SEPARATOR_LINE1_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[SEPARATOR_LINE2_COLOR] = SDL_MapRGB(agVideoFmt, 25, 25, 25);
	agColors[TABLEVIEW_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HEAD_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_CTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_LINE_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[TABLEVIEW_SEL_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 120);
	agColors[TEXTBOX_RW_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[TEXTBOX_RO_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[TEXTBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TEXTBOX_CURSOR_COLOR] = SDL_MapRGB(agVideoFmt, 251, 255, 197);
	agColors[TLIST_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TLIST_BG_COLOR] = agColors[FRAME_COLOR];
	agColors[TLIST_LINE_COLOR] = SDL_MapRGB(agVideoFmt, 40, 40, 40);
	agColors[TLIST_SEL_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 120);
	agColors[MAPVIEW_GRID_COLOR] = SDL_MapRGB(agVideoFmt, 200, 200, 200);
	agColors[MAPVIEW_CURSOR_COLOR] = SDL_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[MAPVIEW_TILE1_COLOR] = SDL_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[MAPVIEW_TILE2_COLOR] = SDL_MapRGB(agVideoFmt, 40, 40, 40);
	agColors[MAPVIEW_MSEL_COLOR] = SDL_MapRGB(agVideoFmt, 150, 150, 150);
	agColors[MAPVIEW_ESEL_COLOR] = SDL_MapRGB(agVideoFmt, 180, 180, 180);
	agColors[TILEVIEW_TILE1_COLOR] = SDL_MapRGB(agVideoFmt, 140, 140, 140);
	agColors[TILEVIEW_TILE2_COLOR] = SDL_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[TILEVIEW_TEXTBG_COLOR] = SDL_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[TILEVIEW_TEXT_COLOR] = SDL_MapRGB(agVideoFmt, 240, 240, 240);
	agColors[TRANSPARENT_COLOR] = SDL_MapRGBA(agVideoFmt, 0, 0, 0, 0);
	agColors[HSVPAL_BAR1_COLOR] = SDL_MapRGBA(agVideoFmt, 0, 0, 0, 0);
	agColors[HSVPAL_BAR2_COLOR] = SDL_MapRGBA(agVideoFmt, 240, 240, 240, 0);
	agColors[PANE_COLOR] = SDL_MapRGBA(agVideoFmt, 100, 100, 100, 0);
	agColors[PANE_CIRCLE_COLOR] = SDL_MapRGBA(agVideoFmt, 170, 170, 170, 0);
	agColors[MAPVIEW_RSEL_COLOR] = SDL_MapRGB(agVideoFmt, 60, 250, 60);
	agColors[MAPVIEW_ORIGIN_COLOR] = SDL_MapRGB(agVideoFmt, 150, 150, 0);
	agColors[FOCUS_COLOR] = SDL_MapRGB(agVideoFmt, 150, 150, 150);

	agColorsBorder[0] = SDL_MapRGB(agVideoFmt, 92, 92, 92);
	agColorsBorder[1] = SDL_MapRGB(agVideoFmt, 80, 80, 75);
	agColorsBorder[2] = SDL_MapRGB(agVideoFmt, 85, 85, 80);
	agColorsBorder[3] = SDL_MapRGB(agVideoFmt, 100, 100, 95);
	agColorsBorder[4] = SDL_MapRGB(agVideoFmt, 85, 85, 80);
	agColorsBorder[5] = SDL_MapRGB(agVideoFmt, 80, 80, 75);
	agColorsBorder[6] = SDL_MapRGB(agVideoFmt, 0, 255, 0);
}

void
AG_ColorsLoad(AG_Netbuf *buf)
{
	int i, ncolors;

	ncolors = (int)AG_ReadUint32(buf);
	for (i = 0; i < ncolors; i++)
		agColors[i] = AG_ReadColor(buf, agVideoFmt);

	ncolors = (int)AG_ReadUint32(buf);
	for (i = 0; i < ncolors; i++)
		agColorsBorder[i] = AG_ReadColor(buf, agVideoFmt);
}

void
AG_ColorsSave(AG_Netbuf *buf)
{
	int i;

	AG_WriteUint32(buf, LAST_COLOR);
	for (i = 0; i < LAST_COLOR; i++) {
		AG_WriteColor(buf, agVideoFmt, agColors[i]);
	}
	AG_WriteUint32(buf, (Uint32)agColorsBorderSize);
	for (i = 0; i < agColorsBorderSize; i++) {
		AG_WriteColor(buf, agVideoFmt, agColorsBorder[i]);
	}
}

void
AG_ColorsDestroy(void)
{
}
