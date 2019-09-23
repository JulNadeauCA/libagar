/*	Public domain	*/

#ifndef _AGAR_MICRO_GEOMETRY_H_
#define _AGAR_MICRO_GEOMETRY_H_
#include <agar/micro/begin.h>

/* Point from x,y coordinates */
typedef struct ma_pt {
	Uint8 x, y;
} MA_Pt;

/* Rectangle from upper-left coordinates + size */
typedef struct ma_rect {
	Uint8 x, y;		/* Upper-left */
	Uint8 w, h;		/* Size */
} MA_Rect;

/*
 * Rectangle from upper-left coords + size (& lower-right coords calculated), Or:
 * Rectangle from upper-left coords and lower-right coords (& size calculated).
 */
typedef struct ma_rect2 {
	Uint8 x1, y1;		/* Upper-left */
	Uint8 w, h;		/* Size */
	Uint8 x2, y2;		/* Lower-right */
} MA_Rect2;

__BEGIN_DECLS
Uint8 MA_RectIntersect(MA_Rect *_Nonnull,
                       const MA_Rect *_Nonnull,
                       const MA_Rect *_Nonnull);

Uint8 MA_RectIntersect2(MA_Rect2 *_Nonnull,
                        const MA_Rect2 *_Nonnull,
                        const MA_Rect2 *_Nonnull);

Uint8 MA_RectCompare(const MA_Rect *_Nonnull,
                     const MA_Rect *_Nonnull)
                    _Pure_Attribute;

Uint8 MA_RectCompare2(const MA_Rect2 *_Nonnull,
                      const MA_Rect2 *_Nonnull)
                     _Pure_Attribute;

Uint8 MA_RectInside(const MA_Rect *_Nonnull, Uint8,Uint8) _Pure_Attribute;
Uint8 MA_RectInside2(const MA_Rect2 *_Nonnull, Uint8,Uint8) _Pure_Attribute;
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_GEOMETRY_H_ */
