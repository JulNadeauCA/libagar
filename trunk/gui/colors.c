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
#include <core/config.h>

#include "view.h"
#include "colors.h"
#include "opengl.h"

const AG_Version agColorSchemeVer = { 1, 0 };

Uint32 agColors[LAST_COLOR];
Uint32 agColorsBorder[7];
int    agColorsBorderSize = 7;

Sint8  agFocusSunkColorShift[3] =	{ -10, -10, -20 };
Sint8  agFocusRaisedColorShift[3] =	{  30,  30,  20 };
Sint8  agNofocusSunkColorShift[3] =	{ -20, -20, -20 };
Sint8  agNofocusRaisedColorShift[3] =	{  10,  10,  10 };
Sint8  agHighColorShift[3] =		{  40,  40,  40 };
Sint8  agLowColorShift[3] =		{ -30, -30, -20 };

const char *agColorNames[] = {
	N_("Background"),
	N_("Frame"),
	N_("Line"),
	N_("Text"),
	N_("Window background"),
	N_("Window highlight"),
	N_("Window lowlight"),
	N_("Titlebar (focused)"),
	N_("Titlebar (unfocused)"),
	N_("Titlebar caption"),
	N_("Button"),
	N_("Button text"),
	N_("Disabled widget"),
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
	N_("Scrollbar arrow 1"),
	N_("Scrollbar arrow 2"),
	N_("Separator line 1"),
	N_("Separator line 2"),
	N_("Tableview"),
	N_("Tableview header"),
	N_("Tableview header text"),
	N_("Tableview cell text"),
	N_("Tableview line"),
	N_("Tableview selection"),
	N_("Textbox"),
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
	N_("Table"),
	N_("Table lines"),
	N_("Fixed background"),
	N_("Fixed box"),
	N_("Text (disabled)"),
	N_("Menu text (disabled)"),
	N_("Socket"),
	N_("Socket label"),
	N_("Socket highlight"),
	N_("Progress bar"),
};

void
AG_ColorsInit(void)
{
	agColors[BG_COLOR] = AG_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[FRAME_COLOR] = AG_MapRGB(agVideoFmt, 125, 125, 125);
	agColors[LINE_COLOR] = AG_MapRGB(agVideoFmt, 55, 55, 55);
	agColors[TEXT_COLOR] = AG_MapRGB(agVideoFmt, 250, 250, 250);
	agColors[WINDOW_BG_COLOR] = AG_MapRGB(agVideoFmt, 82, 82, 82);
	agColors[WINDOW_HI_COLOR] = AG_MapRGB(agVideoFmt, 90, 90, 85);
	agColors[WINDOW_LO_COLOR] = AG_MapRGB(agVideoFmt, 36, 36, 36);
	agColors[TITLEBAR_FOCUSED_COLOR] = AG_MapRGB(agVideoFmt, 40, 50, 60);
	agColors[TITLEBAR_UNFOCUSED_COLOR] = AG_MapRGB(agVideoFmt, 35, 35, 35);
	agColors[TITLEBAR_CAPTION_COLOR] = agColors[TEXT_COLOR];
	agColors[BUTTON_COLOR] = agColors[FRAME_COLOR];
	agColors[BUTTON_TXT_COLOR] = AG_MapRGB(agVideoFmt, 240, 240, 240);
	agColors[DISABLED_COLOR] = AG_MapRGB(agVideoFmt, 160, 160, 160);
	agColors[CHECKBOX_COLOR] = agColors[FRAME_COLOR];
	agColors[CHECKBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[GRAPH_BG_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[GRAPH_XAXIS_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[HSVPAL_CIRCLE_COLOR] = AG_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[HSVPAL_TILE1_COLOR] = AG_MapRGB(agVideoFmt, 140, 140, 140);
	agColors[HSVPAL_TILE2_COLOR] = AG_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[MENU_UNSEL_COLOR] = AG_MapRGB(agVideoFmt, 70, 70, 70);
	agColors[MENU_SEL_COLOR] = AG_MapRGB(agVideoFmt, 40, 40, 110);
	agColors[MENU_OPTION_COLOR] = AG_MapRGB(agVideoFmt, 223, 207, 128);
	agColors[MENU_TXT_COLOR] = AG_MapRGB(agVideoFmt, 230, 230, 230);
	agColors[MENU_SEP1_COLOR] = AG_MapRGB(agVideoFmt, 60, 60, 60);
	agColors[MENU_SEP2_COLOR] = AG_MapRGB(agVideoFmt, 120, 120, 120);
	agColors[NOTEBOOK_BG_COLOR] = AG_MapRGB(agVideoFmt, 63, 63, 63);
	agColors[NOTEBOOK_SEL_COLOR] = AG_MapRGB(agVideoFmt, 117, 117, 116);
	agColors[NOTEBOOK_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[RADIO_SEL_COLOR] = AG_MapRGB(agVideoFmt, 210, 210, 210);
	agColors[RADIO_OVER_COLOR] = AG_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[RADIO_HI_COLOR] = AG_MapRGB(agVideoFmt, 180, 180, 180);
	agColors[RADIO_LO_COLOR] = AG_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[RADIO_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[SCROLLBAR_COLOR] = AG_MapRGB(agVideoFmt, 120, 120, 120);
	agColors[SCROLLBAR_BTN_COLOR] = agColors[BUTTON_COLOR];
	agColors[SCROLLBAR_ARR1_COLOR] = AG_MapRGB(agVideoFmt, 200, 200, 200);
	agColors[SCROLLBAR_ARR2_COLOR] = AG_MapRGB(agVideoFmt, 158, 158, 158);
	agColors[SEPARATOR_LINE1_COLOR] = AG_MapRGB(agVideoFmt, 165, 165, 165);
	agColors[SEPARATOR_LINE2_COLOR] = AG_MapRGB(agVideoFmt, 82, 82, 82);
	agColors[TABLEVIEW_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HEAD_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_CTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[TABLEVIEW_SEL_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 120);
	agColors[TEXTBOX_COLOR] = agColors[FRAME_COLOR];
	agColors[TEXTBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TEXTBOX_CURSOR_COLOR] = AG_MapRGB(agVideoFmt, 251, 255, 197);
	agColors[TLIST_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TLIST_BG_COLOR] = agColors[FRAME_COLOR];
	agColors[TLIST_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[TLIST_SEL_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 120);
	agColors[MAPVIEW_GRID_COLOR] = AG_MapRGB(agVideoFmt, 200, 200, 200);
	agColors[MAPVIEW_CURSOR_COLOR] = AG_MapRGB(agVideoFmt, 100, 100, 100);
	agColors[MAPVIEW_TILE1_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 50);
	agColors[MAPVIEW_TILE2_COLOR] = AG_MapRGB(agVideoFmt, 40, 40, 40);
	agColors[MAPVIEW_MSEL_COLOR] = AG_MapRGB(agVideoFmt, 150, 150, 150);
	agColors[MAPVIEW_ESEL_COLOR] = AG_MapRGB(agVideoFmt, 180, 180, 180);
	agColors[TILEVIEW_TILE1_COLOR] = AG_MapRGB(agVideoFmt, 140, 140, 140);
	agColors[TILEVIEW_TILE2_COLOR] = AG_MapRGB(agVideoFmt, 80, 80, 80);
	agColors[TILEVIEW_TEXTBG_COLOR] = AG_MapRGB(agVideoFmt, 0, 0, 0);
	agColors[TILEVIEW_TEXT_COLOR] = AG_MapRGB(agVideoFmt, 240, 240, 240);
	agColors[TRANSPARENT_COLOR] = AG_MapRGBA(agVideoFmt, 0, 0, 0, 0);
	agColors[HSVPAL_BAR1_COLOR] = AG_MapRGBA(agVideoFmt, 0, 0, 0, 0);
	agColors[HSVPAL_BAR2_COLOR] = AG_MapRGBA(agVideoFmt, 240, 240, 240, 0);
	agColors[PANE_COLOR] = agColors[FRAME_COLOR];
	agColors[PANE_CIRCLE_COLOR] = AG_MapRGBA(agVideoFmt, 170, 170, 170, 0);
	agColors[MAPVIEW_RSEL_COLOR] = AG_MapRGB(agVideoFmt, 60, 250, 60);
	agColors[MAPVIEW_ORIGIN_COLOR] = AG_MapRGB(agVideoFmt, 150, 150, 0);
	agColors[FOCUS_COLOR] = AG_MapRGB(agVideoFmt, 150, 150, 150);
	agColors[TABLE_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLE_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[FIXED_BG_COLOR] = agColors[FRAME_COLOR];
	agColors[FIXED_BOX_COLOR] = agColors[FRAME_COLOR];
	agColors[TEXT_DISABLED_COLOR] = AG_MapRGB(agVideoFmt, 170, 170, 170);
	agColors[MENU_TXT_DISABLED_COLOR] = agColors[TEXT_DISABLED_COLOR];
	agColors[SOCKET_COLOR] = agColors[FRAME_COLOR];
	agColors[SOCKET_LABEL_COLOR] = agColors[TEXT_COLOR];
	agColors[SOCKET_HIGHLIGHT_COLOR] = AG_MapRGB(agVideoFmt, 200, 0, 0);
	agColors[PROGRESS_BAR_COLOR] = AG_MapRGB(agVideoFmt, 50, 50, 120);

	agColorsBorder[0] = AG_MapRGB(agVideoFmt, 92, 92, 92);
	agColorsBorder[1] = AG_MapRGB(agVideoFmt, 80, 80, 75);
	agColorsBorder[2] = AG_MapRGB(agVideoFmt, 85, 85, 80);
	agColorsBorder[3] = AG_MapRGB(agVideoFmt, 100, 100, 95);
	agColorsBorder[4] = AG_MapRGB(agVideoFmt, 85, 85, 80);
	agColorsBorder[5] = AG_MapRGB(agVideoFmt, 80, 80, 75);
	agColorsBorder[6] = AG_MapRGB(agVideoFmt, 0, 255, 0);
}

#define AG_WriteRGBShift(ds,v) do {	\
	AG_WriteSint8((ds), (v)[0]);	\
	AG_WriteSint8((ds), (v)[1]);	\
	AG_WriteSint8((ds), (v)[2]);	\
} while (0)

#define AG_ReadRGBShift(ds,v) do {	\
	(v)[0] = AG_ReadSint8(ds);	\
	(v)[1] = AG_ReadSint8(ds);	\
	(v)[2] = AG_ReadSint8(ds);	\
} while (0)

int
AG_ColorsLoad(const char *file)
{
	AG_DataSource *ds;
	int i, ncolors;

	if ((ds = AG_OpenFile(file, "rb")) == NULL ||
	    AG_ReadVersion(ds,"AG_ColorScheme",&agColorSchemeVer, NULL) == -1)
		return (-1);

	ncolors = (int)AG_ReadUint16(ds);
	for (i = 0; i < ncolors; i++)
		agColors[i] = AG_ReadColor(ds, agVideoFmt);

	ncolors = (int)AG_ReadUint8(ds);
	for (i = 0; i < ncolors; i++)
		agColorsBorder[i] = AG_ReadColor(ds, agVideoFmt);

	AG_ReadUint8(ds);
	AG_ReadRGBShift(ds, agFocusSunkColorShift);
	AG_ReadRGBShift(ds, agFocusRaisedColorShift);
	AG_ReadRGBShift(ds, agNofocusSunkColorShift);
	AG_ReadRGBShift(ds, agNofocusRaisedColorShift);
	AG_ReadRGBShift(ds, agHighColorShift);
	AG_ReadRGBShift(ds, agLowColorShift);

	AG_CloseFile(ds);
	return (0);
}

int
AG_ColorsSave(const char *file)
{
	AG_DataSource *ds;
	int i;
	
	if ((ds = AG_OpenFile(file, "wb")) == NULL) {
		return (-1);
	}
	AG_WriteVersion(ds, "AG_ColorScheme", &agColorSchemeVer);

	AG_WriteUint16(ds, LAST_COLOR);
	for (i = 0; i < LAST_COLOR; i++) {
		AG_WriteColor(ds, agVideoFmt, agColors[i]);
	}
	AG_WriteUint8(ds, (Uint8)agColorsBorderSize);
	for (i = 0; i < agColorsBorderSize; i++) {
		AG_WriteColor(ds, agVideoFmt, agColorsBorder[i]);
	}

	AG_WriteUint8(ds, 6);
	AG_WriteRGBShift(ds, agFocusSunkColorShift);
	AG_WriteRGBShift(ds, agFocusRaisedColorShift);
	AG_WriteRGBShift(ds, agNofocusSunkColorShift);
	AG_WriteRGBShift(ds, agNofocusRaisedColorShift);
	AG_WriteRGBShift(ds, agHighColorShift);
	AG_WriteRGBShift(ds, agLowColorShift);
	
	AG_CloseFile(ds);
	return (0);
}

int
AG_ColorsSaveDefault(void)
{
	char path[AG_PATHNAME_MAX];

	if (AG_GetString(agConfig, "save-path") != NULL) {
		Strlcpy(path, AG_GetString(agConfig,"save-path"), sizeof(path));
		Strlcat(path, AG_PATHSEP, sizeof(path));
		Strlcat(path, "gui-colors.acs", sizeof(path));
		if (AG_ColorsSave(path) == -1)
			return (-1);
	}
	return (0);
}

void
AG_ColorsDestroy(void)
{
}

#define ASSERT_VALID_COLOR(name) \
	do { \
		if (name < 0 || name >= LAST_COLOR) { \
			AG_SetError("No such color: %d", name); \
			return (-1); \
		}; \
	} while (0)

static void
UpdatedColor(int color)
{
	if (color == BG_COLOR) {
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			Uint8 r, g, b;
			AG_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &r,&g,&b);
			glClearColor(r/255.0, g/255.0, b/255.0, 1.0);
		} else
#endif
		{
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			SDL_UpdateRect(agView->v, 0, 0, agView->w, agView->h);
		}
	}
}

int
AG_ColorsSetRGB(int name, Uint8 r, Uint8 g, Uint8 b)
{
	ASSERT_VALID_COLOR(name);
	agColors[name] = SDL_MapRGB(agVideoFmt, r, g, b);
	UpdatedColor(name);
	return (0);
}

int
AG_ColorsSetRGBA(int name, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	ASSERT_VALID_COLOR(name);
	agColors[name] = SDL_MapRGBA(agVideoFmt, r, g, b, a);
	UpdatedColor(name);
	return (0);
}

int
AG_ColorsGetRGB(int name, Uint8 *r, Uint8 *g, Uint8 *b)
{
	ASSERT_VALID_COLOR(name);
	SDL_GetRGB(agColors[name], agVideoFmt, r, g, b);
	return (0);
}

int
AG_ColorsGetRGBA(int name, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	ASSERT_VALID_COLOR(name);
	SDL_GetRGBA(agColors[name], agVideoFmt, r, g, b, a);
	return (0);
}
