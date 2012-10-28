/*
 * Copyright (c) 2005-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Standard color palette for GUI elements.
 */

#include <core/core.h>
#include <core/config.h>

#include <ctype.h>

#include "gui.h"
#include "colors.h"
#include "load_color.h"
#include "drv.h"

const AG_Version agColorSchemeVer = { 1, 0 };

Sint8  agFocusSunkColorShift[3] =	{ -10, -10, -20 };
Sint8  agFocusRaisedColorShift[3] =	{  30,  30,  20 };
Sint8  agNofocusSunkColorShift[3] =	{ -20, -20, -20 };
Sint8  agNofocusRaisedColorShift[3] =	{  10,  10,  10 };
Sint8  agHighColorShift[3] =		{  40,  40,  40 };
Sint8  agLowColorShift[3] =		{ -30, -30, -20 };

#if 0
/* Initialize the standard palette. */
void
AG_ColorsInit(void)
{
	agColors[BG_COLOR] = AG_ColorRGB(0, 0, 0);
	agColors[FRAME_COLOR] = AG_ColorRGB(125, 125, 125);
	agColors[FRAME_MOUSEOVER_COLOR] = AG_ColorRGB(133, 133, 133);
	agColors[LINE_COLOR] = AG_ColorRGB(55, 55, 55);
	agColors[TEXT_COLOR] = AG_ColorRGB(250, 250, 250);
	agColors[WINDOW_BG_COLOR] = AG_ColorRGB(82, 82, 82);
	agColors[WINDOW_HI_COLOR] = AG_ColorRGB(73, 73, 73);
	agColors[WINDOW_LO_COLOR] = AG_ColorRGB(36, 36, 36);
	agColors[TITLEBAR_FOCUSED_COLOR] = AG_ColorRGB(40, 50, 60);
	agColors[TITLEBAR_UNFOCUSED_COLOR] = AG_ColorRGB(35, 35, 35);
	agColors[TITLEBAR_CAPTION_COLOR] = agColors[TEXT_COLOR];
	agColors[BUTTON_COLOR] = agColors[FRAME_COLOR];
	agColors[BUTTON_MOUSEOVER_COLOR] = agColors[FRAME_MOUSEOVER_COLOR];
	agColors[BUTTON_TXT_COLOR] = AG_ColorRGB(240, 240, 240);
	agColors[DISABLED_COLOR] = AG_ColorRGB(160, 160, 160);
	agColors[CHECKBOX_COLOR] = agColors[FRAME_COLOR];
	agColors[CHECKBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[GRAPH_BG_COLOR] = AG_ColorRGB(50, 50, 50);
	agColors[GRAPH_XAXIS_COLOR] = AG_ColorRGB(50, 50, 50);
	agColors[HSVPAL_CIRCLE_COLOR] = AG_ColorRGB(0, 0, 0);
	agColors[HSVPAL_TILE1_COLOR] = AG_ColorRGB(140, 140, 140);
	agColors[HSVPAL_TILE2_COLOR] = AG_ColorRGB(80, 80, 80);
	agColors[MENU_UNSEL_COLOR] = AG_ColorRGB(70, 70, 70);
	agColors[MENU_SEL_COLOR] = AG_ColorRGB(40, 40, 110);
	agColors[MENU_OPTION_COLOR] = AG_ColorRGB(223, 207, 128);
	agColors[MENU_TXT_COLOR] = AG_ColorRGB(230, 230, 230);
	agColors[MENU_SEP1_COLOR] = AG_ColorRGB(60, 60, 60);
	agColors[MENU_SEP2_COLOR] = AG_ColorRGB(120, 120, 120);
	agColors[NOTEBOOK_BG_COLOR] = AG_ColorRGB(63, 63, 63);
	agColors[NOTEBOOK_SEL_COLOR] = AG_ColorRGB(117, 117, 116);
	agColors[NOTEBOOK_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[RADIO_SEL_COLOR] = AG_ColorRGB(0, 0, 0);
	agColors[RADIO_OVER_COLOR] = AG_ColorRGB(100, 100, 100);
	agColors[RADIO_HI_COLOR] = AG_ColorRGB(180, 180, 180);
	agColors[RADIO_LO_COLOR] = AG_ColorRGB(140, 140, 140);
	agColors[RADIO_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[SCROLLBAR_COLOR] = AG_ColorRGB(120, 120, 120);
	agColors[SCROLLBAR_BTN_COLOR] = agColors[BUTTON_COLOR];
	agColors[SCROLLBAR_BTN_MOUSEOVER_COLOR] = agColors[BUTTON_MOUSEOVER_COLOR];
	agColors[SCROLLBAR_ARR1_COLOR] = AG_ColorRGB(200, 200, 200);
	agColors[SCROLLBAR_ARR2_COLOR] = AG_ColorRGB(158, 158, 158);
	agColors[SEPARATOR_LINE1_COLOR] = AG_ColorRGB(165, 165, 165);
	agColors[SEPARATOR_LINE2_COLOR] = AG_ColorRGB(82, 82, 82);
	agColors[TABLEVIEW_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HEAD_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLEVIEW_HTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_CTXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TABLEVIEW_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[TABLEVIEW_SEL_COLOR] = AG_ColorRGB(50, 50, 120);
	agColors[TEXTBOX_COLOR] = agColors[FRAME_COLOR];
	agColors[TEXTBOX_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TEXTBOX_CURSOR_COLOR] = AG_ColorRGB(251, 255, 197);
	agColors[TLIST_TXT_COLOR] = agColors[TEXT_COLOR];
	agColors[TLIST_BG_COLOR] = agColors[FRAME_COLOR];
	agColors[TLIST_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[TLIST_SEL_COLOR] = AG_ColorRGB(50, 50, 120);
	agColors[MAPVIEW_GRID_COLOR] = AG_ColorRGB(200, 200, 200);
	agColors[MAPVIEW_CURSOR_COLOR] = AG_ColorRGB(100, 100, 100);
	agColors[MAPVIEW_TILE1_COLOR] = AG_ColorRGB(50, 50, 50);
	agColors[MAPVIEW_TILE2_COLOR] = AG_ColorRGB(40, 40, 40);
	agColors[MAPVIEW_MSEL_COLOR] = AG_ColorRGB(150, 150, 150);
	agColors[MAPVIEW_ESEL_COLOR] = AG_ColorRGB(180, 180, 180);
	agColors[TILEVIEW_TILE1_COLOR] = AG_ColorRGB(140, 140, 140);
	agColors[TILEVIEW_TILE2_COLOR] = AG_ColorRGB(80, 80, 80);
	agColors[TILEVIEW_TEXTBG_COLOR] = AG_ColorRGB(0, 0, 0);
	agColors[TILEVIEW_TEXT_COLOR] = AG_ColorRGB(240, 240, 240);
	agColors[TRANSPARENT_COLOR] = AG_ColorRGBA(0, 0, 0, 0);
	agColors[HSVPAL_BAR1_COLOR] = AG_ColorRGBA(0, 0, 0, 0);
	agColors[HSVPAL_BAR2_COLOR] = AG_ColorRGBA(240, 240, 240, 0);
	agColors[PANE_COLOR] = agColors[FRAME_COLOR];
	agColors[PANE_CIRCLE_COLOR] = AG_ColorRGBA(170, 170, 170, 0);
	agColors[MAPVIEW_RSEL_COLOR] = AG_ColorRGB(60, 250, 60);
	agColors[MAPVIEW_ORIGIN_COLOR] = AG_ColorRGB(150, 150, 0);
	agColors[FOCUS_COLOR] = AG_ColorRGB(150, 150, 150);
	agColors[TABLE_COLOR] = agColors[FRAME_COLOR];
	agColors[TABLE_LINE_COLOR] = agColors[LINE_COLOR];
	agColors[FIXED_BG_COLOR] = agColors[FRAME_COLOR];
	agColors[FIXED_BOX_COLOR] = agColors[FRAME_COLOR];
	agColors[TEXT_DISABLED_COLOR] = AG_ColorRGB(170, 170, 170);
	agColors[MENU_TXT_DISABLED_COLOR] = agColors[TEXT_DISABLED_COLOR];
	agColors[SOCKET_COLOR] = agColors[FRAME_COLOR];
	agColors[SOCKET_LABEL_COLOR] = agColors[TEXT_COLOR];
	agColors[SOCKET_HIGHLIGHT_COLOR] = AG_ColorRGB(200, 0, 0);
	agColors[PROGRESS_BAR_COLOR] = AG_ColorRGB(50, 50, 120);
	agColors[WINDOW_BORDER_COLOR] = AG_ColorRGB(100, 100, 100);
	agColors[TEXT_SEL_COLOR] = AG_ColorRGB(0, 0, 100);
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

/* Load the standard palette from file. */
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
		agColors[i] = AG_ReadColor(ds);

	ncolors = (int)AG_ReadUint8(ds);
	for (i = 0; i < ncolors; i++)
		(void)AG_ReadColor(ds);			/* agColorsBorder */

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

/* Save the standard palette to file. */
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
		AG_WriteColor(ds, agColors[i]);
	}
	AG_WriteUint8(ds, 0);				/* agColorsBorder */

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

/* Save the current color scheme to the default file location. */
int
AG_ColorsSaveDefault(void)
{
	char path[AG_PATHNAME_MAX];

	AG_GetString(agConfig, "save-path", path, sizeof(path));
	if (path[0] != '\0') {
		Strlcat(path, AG_PATHSEP, sizeof(path));
		Strlcat(path, "gui-colors.acs", sizeof(path));
		if (AG_ColorsSave(path) == -1)
			return (-1);
	}
	return (0);
}

#define ASSERT_VALID_COLOR(name) \
	do { \
		if (name < 0 || name >= LAST_COLOR) { \
			AG_SetError("No such color: %d", name); \
			return (-1); \
		}; \
	} while (0)
#endif

/*
 * Parse a string color specification of the form "rgb(r,g,b[,a])" or
 * "hsv(h,s,v[,a])". Color components may be specified as literal values
 * or % of some parent color.
 */
static __inline__ double
ColorPct(Uint8 in, double v)
{
	double val;
	val = ((double)in)*v/100.0;
	if (val < 0.0) { return (0); }
	if (val > 255.0) { return (255); }
	return val;
}
AG_Color
AG_ColorFromString(const char *s, const AG_Color *pColor)
{
	char buf[128];
	AG_Color cIn = (pColor != NULL) ? *pColor : AG_ColorRGB(0,0,0);
 	AG_Color cOut = cIn;
	double v[4];
	int isPct[4], isHSV = 0;
 	char *c, *pc;
	int i, argc;

	Strlcpy(buf, s, sizeof(buf));
	
	for (c = &buf[0]; *c != '\0' && isspace(*c); c++) {
		;;
	}
	switch (*c) {
	case 'r':		/* rgb(r,g,b[,a]) */
		break;
	case 'h':		/* hsv(h,s,v[,a]) */
		isHSV = 1;
		break;
	default:
		return (cOut);
	}
	for (; *c != '\0' && *c != '('; c++)
		;;
	if (*c == '\0' || c[1] == '\0') {
		goto out;
	}
	pc = &c[1];
	for (i = 0, argc = 0; i < 4; i++) {
		char *tok, *ep;
		if ((tok = AG_Strsep(&pc, ",")) == NULL) {
			break;
		}
		v[i] = strtod(tok, &ep);
		isPct[i] = (*ep == '%');
		argc++;
	}
	if (argc < 3) {
		goto out;
	}
	if (isHSV) {
		float hue, sat, val;

		AG_RGB2HSV(cIn.r, cIn.g, cIn.b, &hue, &sat, &val);
		hue = isPct[0] ? ColorPct(hue, v[0]) : v[0];
		sat = isPct[1] ? ColorPct(sat, v[1]) : v[1];
		val = isPct[2] ? ColorPct(val, v[2]) : v[2];
		AG_HSV2RGB(hue, sat, val, &cOut.r, &cOut.g, &cOut.b);
		if (argc == 4) {
			cOut.a = isPct[3] ? ColorPct(cIn.a, v[3]) : v[3];
		} else {
			cOut.a = cIn.a;
		}
	} else {
		cOut.r = isPct[0] ? (Uint8)ColorPct(cIn.r, v[0]) : (Uint8)v[0];
		cOut.g = isPct[1] ? (Uint8)ColorPct(cIn.g, v[1]) : (Uint8)v[1];
		cOut.b = isPct[2] ? (Uint8)ColorPct(cIn.b, v[2]) : (Uint8)v[2];
		if (argc == 4) {
			cOut.a = isPct[3] ? (Uint8)ColorPct(cIn.a, v[3]) : (Uint8)v[3];
		} else {
			cOut.a = cIn.a;
		}
	}
out:
	return (cOut);
}
