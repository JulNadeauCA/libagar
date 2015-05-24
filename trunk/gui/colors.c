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

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/gui.h>
#include <agar/gui/colors.h>
#include <agar/gui/load_color.h>
#include <agar/gui/drv.h>

#include <ctype.h>

const AG_Version agColorSchemeVer = { 1, 0 };

Sint8  agSunkColorShift[3] = { -10, -10, -20 };
Sint8  agRaisedColorShift[3] = {  30,  30,  20 };
Sint8  agHighColorShift[3] = {  40,  40,  40 };
Sint8  agLowColorShift[3] = { -30, -30, -20 };

/*
 * Parse a string color specification string. Acceptable forms include:
 *
 * 	r,g,b[,a]
 * 	r:g:b[:a]
 * 	rgb(r,g,b[,a])
 * 	hsv(h,s,v[,a])
 *      #rrggbb
 * 
 * If components are specified with a terminating "%" sign, they may be
 * interpreted as percent of some parent color.
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
	case 'r':		/* "rgb(r,g,b[,a])" */
		break;
	case 'h':		/* "hsv(h,s,v[,a])" */
		isHSV = 1;
		break;
	case '#':		/* "#rrggbb" */
		{
			Uint32 hexVal;

			memmove(&c[2], &c[1], strlen(&c[1])+1);
			c[0] = '0';
			c[1] = 'x';
			hexVal = (Uint32)strtoul(c, NULL, 16);
			cOut.r = (Uint8)((hexVal >> 16) & 0xff);
			cOut.g = (Uint8)((hexVal >> 8) & 0xff);
			cOut.b = (Uint8)((hexVal) & 0xff);
			return (cOut);
		}
		break;
	}
	if (*c == 'r' || *c == 'h') {
		for (; *c != '\0' && *c != '('; c++)
			;;
		if (*c == '\0' || c[1] == '\0') {
			goto out;
		}
		pc = &c[1];
	} else {
		pc = &c[0];	/* Just "r,g,b[,a]" */
	}
	for (i = 0, argc = 0; i < 4; i++) {
		char *tok, *ep;
		if ((tok = AG_Strsep(&pc, ",:")) == NULL) {
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
