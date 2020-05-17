/*	Public domain	*/

#ifndef _AGAR_GUI_COLORS_H_
#define _AGAR_GUI_COLORS_H_
#include <agar/gui/begin.h>

#if AG_MODEL == AG_LARGE		/* LG (48-bit color + 16-bit alpha) */
# define AG_COMPONENT_BITS 16
# define AG_COLOR_LAST  0xffff
# define AG_COLOR_LASTF 65535.0f
# define AG_COLOR_LASTD 65535.0
typedef struct ag_color { Uint16 r,g,b,a; } AG_Color;
typedef struct ag_grayscale { Uint32 v,a; } AG_Grayscale;
typedef Uint16 AG_Component;
typedef Sint16 AG_ComponentOffset;
typedef Uint32 AG_GrayComponent;
typedef Uint64 AG_Pixel;
#elif AG_MODEL == AG_MEDIUM		/* MD (24-bit color + 8-bit alpha) */
# define AG_COMPONENT_BITS 8
# define AG_COLOR_LAST  0xff
# define AG_COLOR_LASTF 255.0f
# define AG_COLOR_LASTD 255.0
typedef struct ag_color { Uint8 r,g,b,a; } AG_Color;
typedef struct ag_grayscale { Uint16 v,a; } AG_Grayscale;
typedef Uint8  AG_Component;
typedef Sint8  AG_ComponentOffset;
typedef Uint16 AG_GrayComponent;
typedef Uint32 AG_Pixel;
#elif AG_MODEL == AG_SMALL		/* SM (12-bit color + 4-bit alpha) */
/*
 * The SMALL memory model requires micro-Agar (in ../micro/).
 */
# error "SMALL mode requires micro-Agar"
#endif

#define AG_COLOR_FIRST 0
#define AG_TRANSPARENT AG_COLOR_FIRST	/* Fully transparent */
#define AG_OPAQUE      AG_COLOR_LAST	/* Fully opaque */

typedef struct ag_color_hsv {
	float h,s,v,a;
} AG_ColorHSV;

#define AG_SATURATION_EPSILON 0.01f
#define AG_VALUE_EPSILON      0.01f

typedef struct ag_color_name {
	const char *name;
	AG_Color c;
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
} AG_ColorName;

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

# define AG_ColorWhite(c) AG_ColorRGBA_16((c),0xffff,0xffff,0xffff,0xffff)
# define AG_ColorBlack(c) AG_ColorRGBA_16((c),0x0000,0x0000,0x0000,0xffff)
# define AG_ColorNone(c)  AG_ColorRGBA_16((c),0x0000,0x0000,0x0000,0x0000)
# define AG_ColorHex(c,v) AG_ColorHex64((c),(v))

# define AG_MapRGB_HSVf(r,g,b,h,s,v) AG_MapRGB16_HSVf((r),(g),(b),(h),(s),(v))
# define AG_MapHSVf_RGB(h,s,v,r,g,b) AG_MapHSVf_RGB16((h),(s),(v),(r),(g),(b))

#else /* !AG_LARGE */

# define AG_4toH(c)      AG_4to8(c)
# define AG_8toH(c)      (c)
# define AG_16toH(c)     AG_16to8(c)
# define AG_Hto4(c)      AG_8to4(c)
# define AG_Hto8(c)      (c)
# define AG_Hto16(c)     AG_8to16(c)

# define AG_ColorWhite(c)  AG_ColorRGBA_8((c),255,255,255,255)
# define AG_ColorBlack(c)  AG_ColorRGBA_8((c),0,0,0,255)
# define AG_ColorNone(c)   AG_ColorRGBA_8((c),0,0,0,0)
# define AG_ColorHex(c,v)  AG_ColorHex32((c),(v))

# define AG_MapRGB_HSVf(r,g,b,h,s,v) AG_MapRGB8_HSVf((r),(g),(b),(h),(s),(v))
# define AG_MapHSVf_RGB(h,s,v,r,g,b) AG_MapHSVf_RGB8((h),(s),(v),(r),(g),(b))

#endif /* !AG_LARGE */

#define AG_ColorRGB(c,r,g,b)     AG_ColorRGB_8((c),(r),(g),(b))
#define AG_ColorRGBA(c,r,g,b,a)  AG_ColorRGBA_8((c),(r),(g),(b),(a))
#define AG_RGB2HSV(r,g,b, h,s,v) AG_MapRGB8_HSVf((r),(g),(b),(h),(s),(v))
#define AG_HSV2RGB(h,s,v, r,g,b) AG_MapHSVf_RGB8((h),(s),(v),(r),(g),(b))

__BEGIN_DECLS
extern AG_ColorName agColorNames[];

void AG_ColorFromString(AG_Color *_Nonnull, const char *_Nonnull,
                        const AG_Color *_Nullable);

void AG_ReadColor(AG_Color *_Nonnull, AG_DataSource *_Nonnull);
void AG_WriteColor(AG_DataSource *_Nonnull, const AG_Color *_Nonnull);

void  AG_MapRGB8_HSVf(Uint8,Uint8,Uint8,
                      float *_Nonnull, float *_Nonnull, float *_Nonnull);
void  AG_MapRGB16_HSVf(Uint16,Uint16,Uint16,
                       float *_Nonnull, float *_Nonnull, float *_Nonnull);
void  AG_MapHSVf_RGB8(float,float,float,
                      Uint8 *_Nonnull, Uint8 *_Nonnull, Uint8 *_Nonnull);
void  AG_MapHSVf_RGB16(float,float,float,
                       Uint16 *_Nonnull, Uint16 *_Nonnull, Uint16 *_Nonnull);

#ifdef AG_INLINE_SURFACE
# define AG_INLINE_HEADER
# include <agar/gui/inline_colors.h>
#else /* !AG_INLINE_SURFACE */

AG_Grayscale ag_grayscale_8(Uint8, Uint8);
AG_Grayscale ag_grayscale_16(Uint16, Uint16);
AG_Grayscale ag_grayscale_32(Uint32, Uint32);
void ag_color_rgb_8(AG_Color *_Nonnull, Uint8, Uint8, Uint8);
void ag_color_rgba_8(AG_Color *_Nonnull, Uint8,Uint8,Uint8, Uint8);
void ag_color_rgb_16(AG_Color *_Nonnull, Uint16, Uint16, Uint16);
void ag_color_rgba_16(AG_Color *_Nonnull, Uint16,Uint16,Uint16, Uint16);
void ag_color_hex_16(AG_Color *_Nonnull, Uint16);
void ag_color_hex_32(AG_Color *_Nonnull, Uint32);
#if AG_MODEL == AG_LARGE
void ag_color_hex_64(AG_Color *_Nonnull, Uint64);
#endif

void ag_color_lighten(AG_Color *_Nonnull, int);
void ag_color_darken(AG_Color *_Nonnull, int);

void ag_color_interpolate(AG_Color *_Nonnull, const AG_Color *_Nonnull,
                          const AG_Color *_Nonnull, int, int);
int  ag_color_compare(const AG_Color *_Nonnull, const AG_Color *_Nonnull)
                     _Pure_Attribute;
void ag_color_2_hsv(const AG_Color *_Nonnull, float *_Nonnull, float *_Nonnull, float *_Nonnull);
void ag_hsv_2_color(float, float, float, AG_Color *_Nonnull);

#define AG_Grayscale8(v,a)         ag_grayscale_8((v),(a))
#define AG_Grayscale16(v,a)        ag_grayscale_16((v),(a))
#define AG_Grayscale32(v,a)        ag_grayscale_32((v),(a))
#define AG_ColorRGB_8(c,r,g,b)     ag_color_rgb_8((c),(r),(g),(b))
#define AG_ColorRGBA_8(c,r,g,b,a)  ag_color_rgba_8((c),(r),(g),(b),(a))
#define AG_ColorRGB_16(c,r,g,b)    ag_color_rgb_16((c),(r),(g),(b))
#define AG_ColorRGBA_16(c,r,g,b,a) ag_color_rgba_16((c),(r),(g),(b),(a))
#define AG_ColorHex16(c,h)         ag_color_hex_16((c),(h))
#define AG_ColorHex32(c,h)         ag_color_hex_32((c),(h))
#define AG_ColorHex64(c,h)         ag_color_hex_64((c),(h))
#define AG_ColorCompare(A,B)       ag_color_compare((A),(B))
#define AG_Color2HSV(c,h,s,v)      ag_color_2_hsv((c),(h),(s),(v))
#define AG_HSV2Color(h,s,v,c)      ag_hsv_2_color((h),(s),(v),(c))

#define AG_ColorDarken(c,shade)              ag_color_darken((c),(shade))
#define AG_ColorLighten(c,shade)             ag_color_lighten((c),(shade))
#define AG_ColorInterpolate(d,c1,c2,num,den) ag_color_interpolate((d),(c1),(c2),(num),(den))

#endif /* !AG_INLINE_SURFACE */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_COLORS_H_ */
