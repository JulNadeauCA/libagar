/*
 * Copyright (c) 2005-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/drv.h>
#include <agar/gui/stylesheet.h>
#include <agar/gui/gui_math.h>

#include <ctype.h>

/* Color offsets used by 3D-style widget primitives. */
#if AG_MODEL == AG_LARGE
AG_ColorOffset agSunkColor   = { -2500, -2500, -5100 };
AG_ColorOffset agRaisedColor = {  7700,  7700,  5100 };
AG_ColorOffset agLowColor    = { -7700, -7700, -5100 };
AG_ColorOffset agHighColor   = { 10000, 10000, 10000 };
#else
AG_ColorOffset agSunkColor   = { -10, -10, -20 };
AG_ColorOffset agRaisedColor = {  10,  10,  20 };
AG_ColorOffset agLowColor    = { -20, -20, -10 };
AG_ColorOffset agHighColor   = {  40,  40,  40 };
#endif

#ifdef HAVE_FLOAT

static __inline__ AG_Component
PctRGB(AG_Component c, double v)
{
	double x = ((double)c)*v/100.0;

	if (x <= 0.0)            { return (0); }
	if (x >= AG_COLOR_LASTD) { return (AG_COLOR_LAST); }
	return (AG_Component)(x);
}
static __inline__ float
PctHSV(float c, double v)
{
	float x = c*((float)v)/100.0f;

	if (x < 0.0f) { return (0.0f); }
	if (x > 1.0f) { return (1.0f); }
	return (x);
}

#else /* !HAVE_FLOAT */

static __inline__ Uint32
PctRGB(AG_Component c, Uint32 v)
{
	double x = ((double)c)*v/100.0;

	if (x <= 0.0)            { return (0); }
	if (x >= AG_COLOR_LASTD) { return (AG_COLOR_LAST); }
	return (AG_Component)(x);
}

#endif /* HAVE_FLOAT */

/*
 * Parse an Agar color specification string. Acceptable forms include:
 *
 * 	"r,g,b[,a]"          # Comma-separated 8-bit Red,Green,Blue,Alpha
 * 	"r:g:b[:a]"          # Colon-separated variant
 * 	"rgb(r,g,b[,a])"     # The "rgb()" may be omitted
 *      "#rrggbb"            # 8-bit RGB opaque
 *      "#rrrrggggbbbbaaaa"  # 16-bit RGBA (exact if AG_MODEL==AG_LARGE)
 *
 * If floating-point support is available, HSV form is also available:
 *
 * 	"hsv(h,s,v[,a])"     # Float Hue,Saturation,Value and 8-bit Alpha
 * 
 * Components with terminating '%' character are interpreted as percentage
 * of a parent color pColor.
 */
AG_Color
AG_ColorFromString(const char *s, const AG_Color *pColor)
{
	char buf[AG_STYLE_VALUE_MAX];
	AG_Color cIn = (pColor != NULL) ? *pColor : AG_ColorRGB(0,0,0);
 	AG_Color cOut = cIn;
#ifdef HAVE_FLOAT
	double v[4];
#else
	Uint32 v[4];
#endif
	int isPct[4], isHSV = 0;
 	char *c, *pc;
	int i, argc;

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
	case '#': {		/* "#rrggbb" */
		Uint32 hexVal;

		memmove(&c[2], &c[1], strlen(&c[1])+1);
		c[0] = '0';
		c[1] = 'x';
		hexVal = (Uint32)strtoul(c, NULL, 16);
		cOut.r = AG_8toH((hexVal >> 16) & 0xff);
		cOut.g = AG_8toH((hexVal >> 8)  & 0xff);
		cOut.b = AG_8toH((hexVal)       & 0xff);
		cOut.a = AG_OPAQUE;
		return (cOut);
	    } /* case */
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
	for (i=0, argc=0; i<4; i++) {
		char *tok, *ep;
		if ((tok = AG_Strsep(&pc, ",:")) == NULL) {
			break;
		}
#ifdef HAVE_FLOAT
		v[i] = strtod(tok, &ep);
#else
		v[i] = (Uint32)strtoul(tok, &ep, 10);
#endif
		isPct[i] = (*ep == '%');
		argc++;
	}
	if (argc < 3) {
		goto out;
	}
	if (isHSV) {
#ifdef HAVE_FLOAT
		float hue, sat, val;

		AG_Color2HSV(cIn, &hue, &sat, &val);
		hue = isPct[0] ? PctHSV(hue, v[0]) : v[0];
		sat = isPct[1] ? PctHSV(sat, v[1]) : v[1];
		val = isPct[2] ? PctHSV(val, v[2]) : v[2];
		AG_HSV2Color(hue, sat, val, &cOut);
		cOut.a = (argc >= 4) ?
		         (isPct[3] ? PctHSV(cIn.a,v[3]) : AG_8toH(v[3])) :
			 AG_OPAQUE;
#endif /* HAVE_FLOAT */
	} else {
		cOut.r = isPct[0] ? PctRGB(cIn.r,v[0]) : AG_8toH(v[0]);
		cOut.g = isPct[1] ? PctRGB(cIn.g,v[1]) : AG_8toH(v[1]);
		cOut.b = isPct[2] ? PctRGB(cIn.b,v[2]) : AG_8toH(v[2]);
		cOut.a = (argc >= 4) ?
		         (isPct[3] ? PctRGB(cIn.a,v[3]) : AG_8toH(v[3])) :
			 AG_OPAQUE;
	}
out:
	return (cOut);
}

/*
 * HSV (Hue/Saturation/Value) to RGB conversion.
 */
#ifdef HAVE_FLOAT
/*
 * Map 8-bit RGB components to single-precision Hue/Saturation/Value.
 * TODO make this a const function returning an AG_ColorHSV.
 */
void
AG_MapRGB8_HSVf(Uint8 r, Uint8 g, Uint8 b,
    float *_Nonnull h, float *_Nonnull s, float *_Nonnull v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/255.0f;
	vG = (float)g/255.0f;
	vB = (float)b/255.0f;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {					/* Gray */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaG = ((vMax - vG)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaB = ((vMax - vB)/6.0f + deltaMax/2.0f) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0f;
		} else if (vG == vMax) {
			*h = 120.0f + (deltaR - deltaB)*360.0f;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0f + (deltaG - deltaR)*360.0f;	/* 2/3 */
		}

		if (*h < 0.0f)   (*h)++;
		if (*h > 360.0f) (*h)--;
	}
}

# if AG_MODEL == AG_LARGE
/*
 * Map 16-bit RGB components to single-precision Hue/Saturation/Value.
 * TODO make this a const function returning an AG_ColorHSV.
 */
void
AG_MapRGB16_HSVf(Uint16 r, Uint16 g, Uint16 b,
    float *_Nonnull h, float *_Nonnull s, float *_Nonnull v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/65535.0f;
	vG = (float)g/65535.0f;
	vB = (float)b/65535.0f;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {					/* Gray */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaG = ((vMax - vG)/6.0f + deltaMax/2.0f) / deltaMax;
		deltaB = ((vMax - vB)/6.0f + deltaMax/2.0f) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0f;
		} else if (vG == vMax) {
			*h = 120.0f + (deltaR - deltaB)*360.0f;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0f + (deltaG - deltaR)*360.0f;	/* 2/3 */
		}

		if (*h < 0.0f)   (*h)++;
		if (*h > 360.0f) (*h)--;
	}
}
# endif /* AG_LARGE */

/* Map single-precision Hue/Saturation/Value to 8-bit RGB components. */
void
AG_MapHSVf_RGB8(float h, float s, float v,
    Uint8 *_Nonnull r, Uint8 *_Nonnull g, Uint8 *_Nonnull b)
{
	float vR, vG, vB, hv, var[3];
	int iv;

	if (s == 0.0) {
		*r = (Uint8)(v * 255.0f);
		*g = (Uint8)(v * 255.0f);
		*b = (Uint8)(v * 255.0f);
		return;
	}
	
	hv = h/60.0f;
	iv = Floor(hv);
	var[0] = v * (1.0f - s);
	var[1] = v * (1.0f - s*(hv - iv));
	var[2] = v * (1.0f - s*(1.0f - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = (Uint8)(vR * 255.0f);
	*g = (Uint8)(vG * 255.0f);
	*b = (Uint8)(vB * 255.0f);
}

# if AG_MODEL == AG_LARGE
/*
 * Map single-precision Hue/Saturation/Value to 16-bit RGB components.
 */
void
AG_MapHSVf_RGB16(float h, float s, float v,
    Uint16 *_Nonnull r, Uint16 *_Nonnull g, Uint16 *_Nonnull b)
{
	float vR, vG, vB, hv, var[3];
	int iv;

	if (s == 0.0) {
		*r = (Uint16)(v * 65535.0f);
		*g = (Uint16)(v * 65535.0f);
		*b = (Uint16)(v * 65535.0f);
		return;
	}
	
	hv = h/60.0F;
	iv = Floor(hv);
	var[0] = v * (1.0F - s);
	var[1] = v * (1.0F - s*(hv - iv));
	var[2] = v * (1.0F - s*(1.0F - (hv - iv)));

	switch (iv) {
	case 0:		vR = v;		vG = var[2];	vB = var[0];	break;
	case 1:		vR = var[1];	vG = v;		vB = var[0];	break;
	case 2:		vR = var[0];	vG = v;		vB = var[2];	break;
	case 3:		vR = var[0];	vG = var[1];	vB = v;		break;
	case 4:		vR = var[2];	vG = var[0];	vB = v;		break;
	default:	vR = v;		vG = var[0];	vB = var[1];	break;
	}
	
	*r = (Uint16)(vR * 65535.0f);
	*g = (Uint16)(vG * 65535.0f);
	*b = (Uint16)(vB * 65535.0f);
}
# endif /* AG_LARGE */
#endif /* HAVE_FLOAT */
