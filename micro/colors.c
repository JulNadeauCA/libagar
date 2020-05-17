/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/micro/gui.h>
#include <agar/micro/colors.h>
#include <agar/micro/drv.h>

#include <ctype.h>

/* Color offsets used by 3D-style widget primitives. */
MA_ColorOffset maSunkColor   = { -1, -1, -2 };
MA_ColorOffset maRaisedColor = {  1,  1,  2 };
MA_ColorOffset maLowColor    = { -2, -2, -1 };
MA_ColorOffset maHighColor   = {  4,  4,  4 };
MA_ColorOffset maTint        = {  1,  1,  1 };
MA_ColorOffset maShade       = { -1, -1, -1 };

static MA_Component PctRGB(MA_Component, Uint16)
                          _Const_Attribute;

/* Set MA_Color from 4-bit components expanded to 8-bit values. */
void
MA_ColorRGBA_4(MA_Color *c, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
}

/* Set MA_Color from 4-bit components as 16-bit value 0x[RGBA]. */
void
MA_ColorRGBA_16(MA_Color *c, Uint16 rgba)
{
	c->r = (rgba & 0xf000) >> 12;
	c->g = (rgba & 0x0f00) >> 8;
	c->b = (rgba & 0x00f0) >> 4;
	c->a = (rgba & 0x000f);
}

/* Compute the clamped sum of color c and offset offs. */
void
MA_ColorAdd(MA_Color *dst, const MA_Color *c, const MA_ColorOffset *offs)
{
	dst->r = AG_MIN(MA_COLOR_LAST, c->r + offs->r);
	dst->g = AG_MIN(MA_COLOR_LAST, c->g + offs->g);
	dst->b = AG_MIN(MA_COLOR_LAST, c->b + offs->b);
	dst->a = AG_MIN(MA_COLOR_LAST, c->a + offs->a);
}

/* Compute the point in RGB space between c1 and c2 closest to (num/denom). */
void
MA_ColorInterpolate(MA_Color *dst, const MA_Color *c1, const MA_Color *c2,
    Uint8 num, Uint8 denom)
{
	dst->r = c1->r + (c2->r - c1->r)*num/denom;
	dst->g = c1->g + (c2->g - c1->g)*num/denom;
	dst->b = c1->b + (c2->b - c1->b)*num/denom;
	dst->a = c1->a + (c2->a - c1->a)*num/denom;
}

/* Compute the difference between two colors. */
Uint8
MA_ColorCompare(const MA_Color *A, const MA_Color *B)
{
	return !(A->r == B->r &&
	         A->g == B->g &&
	         A->b == B->b &&
	         A->a == B->a);
}

/*
 * Parse a micro-Agar color specification string. Acceptable forms include:
 *
 * 	"r,g,b[,a]"          # Comma-separated 8-bit Red,Green,Blue,Alpha
 * 	"r:g:b[:a]"          # Colon-separated variant
 *      "#rgb"               # 4-bit RGB opaque
 *      "#rgba"              # 4-bit RGB + alpha
 * 	"hsv(h,s,v[,a])"     # 4-bit Hue,Saturation,Value and 4-bit Alpha
 * 
 * Components with terminating '%' character are interpreted as percentage
 * of a parent widget's color pColor.
 */
void
MA_ColorFromString(MA_Color *cOut, const char *s, const MA_Color *pColor)
{
	char buf[36];
	MA_Color cIn;
	Uint16 v[4];
	int isPct[4], isHSV = 0;
 	char *c, *pc;
	int i, argc;

	if (pColor != NULL) {
		cIn = *pColor;
	} else {
		MA_ColorBlack(&cIn);
	}

	Strlcpy(buf, s, sizeof(buf));
	
	for (c = &buf[0]; *c != '\0' && isspace((int)*c); c++) {
		;;
	}
	switch (*c) {
	case 'r':		/* "rgb(r,g,b[,a])" */
		break;
	case 'h':		/* "hsv(h,s,v[,a])" */
		isHSV = 1;
		break;
	case '#': {		/* "#rgb" or "#rgba" */
		Uint16 hexVal;

		memmove(&c[2], &c[1], strlen(&c[1])+1);
		c[0] = '0';
		c[1] = 'x';
		hexVal = (Uint16)strtoul(c, NULL, 16);
		cOut->r = (hexVal >> 8) & 0xf;
		cOut->g = (hexVal >> 4) & 0xf;
		cOut->b = (hexVal)      & 0xf;
		cOut->a = MA_OPAQUE;
		return;
	    }
	}
	if (*c == 'r' || *c == 'h') {
		for (; *c != '\0' && *c != '('; c++)
			;;
		if (*c == '\0' || c[1] == '\0') {
			return;
		}
		pc = &c[1];
	} else {
		pc = &c[0];	/* Just "r,g,b[,a]" */
	}
	for (i=0, argc=0; i < 4; i++) {
		char *tok, *ep;
		if ((tok = AG_Strsep(&pc, ",:/")) == NULL) {
			break;
		}
		v[i] = (Uint16)strtoul(tok, &ep, 10);
		isPct[i] = (*ep == '%');
		argc++;
	}
	if (argc < 3) {
		return;
	}
	if (isHSV) {
		cOut->r = isPct[0] ? PctRGB(cIn.r,v[0]) : v[0];
		cOut->g = isPct[1] ? PctRGB(cIn.g,v[1]) : v[1];
		cOut->b = isPct[2] ? PctRGB(cIn.b,v[2]) : v[2];
		cOut->a = (argc >= 4) ?
		         (isPct[3] ? PctRGB(cIn.a,v[3]) : v[3]) : MA_OPAQUE;
	}
}

static MA_Component _Const_Attribute
PctRGB(MA_Component c, Uint16 v)
{
	Sint16 x = c*v/100;

	if (x < 0)             { return (0); }
	if (x > MA_COLOR_LAST) { return (MA_COLOR_LAST); }

	return (MA_Component)(x);
}
