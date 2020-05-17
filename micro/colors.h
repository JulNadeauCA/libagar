/*	Public domain	*/

#ifndef _AGAR_MICRO_COLORS_H_
#define _AGAR_MICRO_COLORS_H_
#include <agar/micro/begin.h>

#define MA_PHASE_BITS     4			/* Phase angle bits */
#define MA_PHASE_FIRST    0
#define MA_PHASE_LAST     15

#define MA_COMPONENT_BITS 4			/* Component value bits */
#define MA_COLOR_FIRST    0
#define MA_COLOR_LAST     15
#define MA_TRANSPARENT    MA_COLOR_FIRST	/* Fully transparent */
#define MA_OPAQUE         MA_COLOR_LAST		/* Fully opaque */

typedef struct ma_color {
	Uint8 r,g,b,a;
} MA_Color;

typedef struct ma_color_offset {
	Sint8 r,g,b,a;
} MA_ColorOffset;

typedef Uint8  MA_Component;
typedef Sint8  MA_ComponentOffset;
typedef Uint16 MA_Pixel;

#define MA_ColorWhite(c) MA_ColorRGBA_16((c),0xffff)
#define MA_ColorBlack(c) MA_ColorRGBA_16((c),0x000f)
#define MA_ColorNone(c)  MA_ColorRGBA_16((c),0x0000)

#define MA_ColorRGB(c,r,g,b)    MA_ColorRGBA_4((c),(r),(g),(b),0xf)
#define MA_ColorRGBA(c,r,g,b,a) MA_ColorRGBA_4((c),(r),(g),(b))

__BEGIN_DECLS
extern MA_ColorOffset maSunkColor, maRaisedColor, maLowColor, maHighColor,
                      maTint, maShade;

void  MA_ColorRGBA_4(MA_Color *_Nonnull, Uint8,Uint8,Uint8,Uint8);
void  MA_ColorRGBA_16(MA_Color *_Nonnull, Uint16);

void  MA_ColorAdd(MA_Color *_Nonnull, const MA_Color *_Nonnull,
                  const MA_ColorOffset *_Nonnull);
void  MA_ColorInterpolate(MA_Color *_Nonnull, const MA_Color *_Nonnull,
                          const MA_Color *_Nonnull, Uint8, Uint8);
Uint8 MA_ColorCompare(const MA_Color *_Nonnull, const MA_Color *_Nonnull)
                     _Pure_Attribute;
void  MA_ColorFromString(MA_Color *_Nonnull, const char *_Nonnull,
                         const MA_Color *_Nullable);
#ifdef AG_SERIALIZATION
void  MA_ReadColor(MA_Color *_Nonnull, AG_DataSource *_Nonnull);
void  MA_WriteColor(AG_DataSource *_Nonnull, const MA_Color *_Nonnull);
#endif
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_COLORS_H_ */
