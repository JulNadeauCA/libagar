/*	Public domain	*/

#ifndef _AGAR_GUI_COLORS_H_
#define _AGAR_GUI_COLORS_H_
#include <agar/gui/begin.h>

/* Color */
typedef struct ag_color {
	Uint8 r, g, b, a;
} AG_Color;

__BEGIN_DECLS
extern Sint8 agSunkColorShift[3];
extern Sint8 agRaisedColorShift[3];
extern Sint8 agHighColorShift[3];
extern Sint8 agLowColorShift[3];

/* Return a Color structure for given RGB components. */
static __inline__ AG_Color
AG_ColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = 255;
	return (c);
}

/* Return a Color structure for given RGBA components. */
static __inline__ AG_Color
AG_ColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_Color c;
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
	return (c);
}

/* Return a Color structure from 0xRRGGBBAA format. */
static __inline__ AG_Color
AG_ColorHex(Uint32 h)
{
	AG_Color c;

	c.r = (h&0xff000000) >> 24;
	c.g = (h&0x00ff0000) >> 16;
	c.b = (h&0x0000ff00) >> 8;
	c.a = (h&0x000000ff);
	return (c);
}

/* Compare two colors. */
static __inline__ int
AG_ColorCompare(AG_Color c1, AG_Color c2)
{
	return (c1.r == c2.r &&
	        c1.g == c2.g &&
		c1.b == c2.b &&
		c1.a == c2.a) ? 0 : 1;
}

AG_Color AG_ColorFromString(const char *, const AG_Color *);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_COLORS_H_ */
