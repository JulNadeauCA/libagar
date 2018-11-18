/*	Public domain	*/

#ifndef _AGAR_GUI_COLORS_H_
#define _AGAR_GUI_COLORS_H_
#include <agar/gui/begin.h>

#if AG_MODEL == AG_LARGE
# define AG_COMPONENT_BITS 16
# define AG_COLOR_LAST  0xffff
# define AG_COLOR_LASTF 65535.0f
# define AG_COLOR_LASTD 65535.0
typedef struct ag_color { Uint16 r,g,b,a; } AG_Color;
typedef struct ag_color_offset { Sint16 r,g,b,a; } AG_ColorOffset;
typedef struct ag_grayscale { Uint32 v,a; } AG_Grayscale;
typedef Uint16 AG_Component;
typedef Sint16 AG_ComponentOffset;
typedef Uint32 AG_GrayComponent;
typedef Uint64 AG_Pixel;
#else
# define AG_COMPONENT_BITS 8
# define AG_COLOR_LAST  0xff
# define AG_COLOR_LASTF 255.0f
# define AG_COLOR_LASTD 255.0
typedef struct ag_color { Uint8 r,g,b,a; } AG_Color;
typedef struct ag_color_offset { Sint8 r,g,b,a; } AG_ColorOffset;
typedef struct ag_grayscale { Uint16 v,a; } AG_Grayscale;
typedef Uint8  AG_Component;
typedef Sint8  AG_ComponentOffset;
typedef Uint16 AG_GrayComponent;
typedef Uint32 AG_Pixel;
#endif /* SMALL or MEDIUM */

#define AG_COLOR_FIRST 0
#define AG_TRANSPARENT AG_COLOR_FIRST	/* Fully transparent */
#define AG_OPAQUE      AG_COLOR_LAST	/* Fully opaque */

typedef struct ag_color_hsv {
#ifdef AG_HAVE_FLOAT
	float h,s,v,a;
#else
	Uint32 h,s,v,a;
#endif
} AG_ColorHSV;

#define AG_4to8(c)    (Uint8)((float)(c)/15.0f * 255.0f)
#define AG_4to16(c)  (Uint16)((float)(c)/15.0f * 65535.0f)
#define AG_4to32(c)  (Uint32)((float)(c)/15.0f * 4294967295.0f)
#define AG_8to4(c)    (Uint8)((float)(c)/255.0f * 15.0f)
#define AG_8to16(c)  (Uint16)((float)(c)/255.0f * 65535.0f)
#define AG_8to32(c)  (Uint32)((float)(c)/255.0f * 4294967295.0f)
#define AG_16to4(c)   (Uint8)((float)(c)/65535.0f * 15.0f)
#define AG_16to8(c)   (Uint8)((float)(c)/65535.0f * 255.0f)
#define AG_16to32(c) (Uint32)((float)(c)/65535.0f * 4294967295.0f)
#define AG_32to4(c)   (Uint8)((float)(c)/4294967295.0f * 15.0f)
#define AG_32to8(c)   (Uint8)((float)(c)/4294967295.0f * 255.0f)
#define AG_32to16(c) (Uint16)((float)(c)/4294967295.0f * 65535.0f)

#if AG_MODEL == AG_LARGE

# define AG_4toH(c)      AG_4to16(c)
# define AG_8toH(c)      AG_8to16(c)
# define AG_16toH(c)     (c)
# define AG_Hto4(c)      AG_16to4(c)
# define AG_Hto8(c)      AG_16to8(c)
# define AG_Hto16(c)     (c)

# define AG_ColorWhite() AG_ColorRGBA_16(0xffff,0xffff,0xffff,0xffff)
# define AG_ColorBlack() AG_ColorRGBA_16(0x0000,0x0000,0x0000,0xffff)
# define AG_ColorHex(v)  AG_ColorHex64(v)

# define AG_MapRGB_HSVf(r,g,b,h,s,v) AG_MapRGB16_HSVf((r),(g),(b),(h),(s),(v))
# define AG_MapHSVf_RGB(h,s,v,r,g,b) AG_MapHSVf_RGB16((h),(s),(v),(r),(g),(b))

#else /* MEDIUM or SMALL */

# define AG_4toH(c)      AG_4to8(c)
# define AG_8toH(c)      (c)
# define AG_16toH(c)     AG_16to8(c)
# define AG_Hto4(c)      AG_8to4(c)
# define AG_Hto8(c)      (c)
# define AG_Hto16(c)     AG_8to16(c)

# define AG_ColorWhite() AG_ColorRGBA_8(255,255,255,255)
# define AG_ColorBlack() AG_ColorRGBA_8(0,0,0,255)
# define AG_ColorHex(v)  AG_ColorHex32(v)

# define AG_MapRGB_HSVf(r,g,b,h,s,v) AG_MapRGB8_HSVf((r),(g),(b),(h),(s),(v))
# define AG_MapHSVf_RGB(h,s,v,r,g,b) AG_MapHSVf_RGB8((h),(s),(v),(r),(g),(b))

#endif /* MEDIUM or SMALL */

#define AG_ColorRGB(r,g,b)    AG_ColorRGB_8((r),(g),(b))
#define AG_ColorRGBA(r,g,b,a) AG_ColorRGBA_8((r),(g),(b),(a))

#define AG_RGB2HSV(r,g,b, h,s,v) AG_MapRGB8_HSVf((r),(g),(b),(h),(s),(v))
#define AG_HSV2RGB(h,s,v, r,g,b) AG_MapHSVf_RGB8((h),(s),(v),(r),(g),(b))

__BEGIN_DECLS
extern AG_ColorOffset agSunkColor, agRaisedColor, agLowColor, agHighColor;

AG_Color AG_ColorFromString(const char *_Nonnull, const AG_Color *_Nullable);
AG_Color AG_ReadColor(AG_DataSource *_Nonnull);
void     AG_WriteColor(AG_DataSource *_Nonnull, AG_Color);

#ifdef AG_HAVE_FLOAT
void  AG_MapRGB8_HSVf(Uint8,Uint8,Uint8,
                      float *_Nonnull, float *_Nonnull, float *_Nonnull);
void  AG_MapRGB16_HSVf(Uint16,Uint16,Uint16,
                       float *_Nonnull, float *_Nonnull, float *_Nonnull);
void  AG_MapHSVf_RGB8(float,float,float,
                      Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull);
void  AG_MapHSVf_RGB16(float,float,float,
                       Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull);
#endif

static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_8(Uint8 v, Uint8 a)
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = AG_8to32(v);
	G.a = AG_8to32(a);
#else
	G.v = AG_8to16(v);
	G.a = AG_8to16(a);
#endif
	return (G);
}
static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_16(Uint16 v, Uint16 a)
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = AG_16to32(v);
	G.a = AG_16to32(a);
#else
	G.v = v;
	G.a = a;
#endif
	return (G);
}
static __inline__ AG_Grayscale _Const_Attribute
AG_Grayscale_32(Uint32 v, Uint32 a)
{
	AG_Grayscale G;
#if AG_MODEL == AG_LARGE
	G.v = v;
	G.a = a;
#else
	G.v = AG_32to16(v);
	G.a = AG_32to16(a);
#endif
	return (G);
}

/* Return AG_Color from 8- or 16-bit RGB (or RGB + alpha) components. */
static __inline__ AG_Color _Const_Attribute
AG_ColorRGB_8(Uint8 r, Uint8 g, Uint8 b)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = AG_8to16(r);
	c.g = AG_8to16(g);
	c.b = AG_8to16(b);
#else
	c.r = r;
	c.g = g;
	c.b = b;
#endif
	c.a = AG_OPAQUE;
	return (c);
}
static __inline__ AG_Color _Const_Attribute
AG_ColorRGBA_8(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = AG_8to16(r);
	c.g = AG_8to16(g);
	c.b = AG_8to16(b);
	c.a = AG_8to16(a);
#else
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
#endif
	return (c);
}
static __inline__ AG_Color _Const_Attribute
AG_ColorRGB_16(Uint16 r, Uint16 g, Uint16 b)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = r;
	c.g = g;
	c.b = b;
#else
	c.r = AG_16to8(r);
	c.g = AG_16to8(g);
	c.b = AG_16to8(b);
#endif
	c.a = AG_OPAQUE;
	return (c);
}
static __inline__ AG_Color _Const_Attribute
AG_ColorRGBA_16(Uint16 r, Uint16 g, Uint16 b, Uint16 a)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = r;
	c.g = g;
	c.b = b;
	c.a = a;
#else
	c.r = AG_16to8(r);
	c.g = AG_16to8(g);
	c.b = AG_16to8(b);
	c.a = AG_16to8(a);
#endif
	return (c);
}

/* Return AG_Color from 4-bit components packed as 0xRGBA. */
static __inline__ AG_Color _Const_Attribute
AG_ColorHex16(Uint16 h)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = AG_4to16((h & 0xf000) >> 3);
	c.g = AG_4to16((h & 0x0f00) >> 2);
	c.b = AG_4to16((h & 0x00f0) >> 1);
	c.a = AG_4to16((h & 0x000f));
#else
	c.r = AG_4to8((h & 0xf000) >> 3);
	c.g = AG_4to8((h & 0x0f00) >> 2);
	c.b = AG_4to8((h & 0x00f0) >> 1);
	c.a = AG_4to8((h & 0x000f));
#endif
	return (c);
}

/* Return AG_Color from 8-bit components packed as 0xRRGGBBAA. */
static __inline__ AG_Color _Const_Attribute
AG_ColorHex32(Uint32 h)
{
	AG_Color c;
#if (AG_MODEL == AG_LARGE)
	c.r = AG_8to16((h & 0xff000000) >> 24);
	c.g = AG_8to16((h & 0x00ff0000) >> 16);
	c.b = AG_8to16((h & 0x0000ff00) >> 8);
	c.a = AG_8to16((h & 0x000000ff));
#else
	c.r = (h & 0xff000000) >> 24;
	c.g = (h & 0x00ff0000) >> 16;
	c.b = (h & 0x0000ff00) >> 8;
	c.a = (h & 0x000000ff);
#endif
	return (c);
}

#if AG_MODEL == AG_LARGE
/* Return AG_Color from 16-bit components packed as 0xRRRRGGGGBBBBAAAA. */
static __inline__ AG_Color _Const_Attribute
AG_ColorHex64(Uint64 h)
{
	AG_Color c;
	c.r = (h & 0xffff000000000000) >> 48;
	c.g = (h & 0x0000ffff00000000) >> 32;
	c.b = (h & 0x00000000ffff0000) >> 16;
	c.a = (h & 0x000000000000ffff);
	return (c);
}
#endif /* AG_LARGE */

/* Component-wise clamped addition */
static __inline__ AG_Color _Const_Attribute
AG_ColorAdd(AG_Color c, AG_ColorOffset offs)
{
	AG_Color out;
	out.r = AG_MIN(AG_COLOR_LAST, c.r + offs.r);
	out.g = AG_MIN(AG_COLOR_LAST, c.g + offs.g);
	out.b = AG_MIN(AG_COLOR_LAST, c.b + offs.b);
	out.a = AG_MIN(AG_COLOR_LAST, c.a + offs.a);
	return (out);
}

/* Compare two colors. */
static __inline__ int _Const_Attribute
AG_ColorCompare(AG_Color A, AG_Color B)
{
	return !(A.r == B.r &&
	         A.g == B.g &&
	         A.b == B.b &&
	         A.a == B.a);
}

#ifdef AG_HAVE_FLOAT
/*
 * Convert between RGB and HSV color representations.
 * TODO make this a _Const_Attribute function returning an AG_ColorHSV.
 */
static __inline__ void
AG_Color2HSV(AG_Color c, float *_Nonnull h, float *_Nonnull s, float *_Nonnull v) {
	AG_MapRGB_HSVf(c.r, c.g, c.b, h,s,v);
}
static __inline__ void
AG_HSV2Color(float h, float s, float v, AG_Color *_Nonnull c) {
	AG_MapHSVf_RGB(h,s,v, &c->r, &c->g, &c->b);
}
#endif /* HAVE_FLOAT */
__END_DECLS


#include <agar/gui/close.h>
#endif /* _AGAR_GUI_COLORS_H_ */
