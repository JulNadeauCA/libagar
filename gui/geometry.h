/*	Public domain	*/

#ifndef _AGAR_GUI_GEOMETRY_H_
#define _AGAR_GUI_GEOMETRY_H_
#include <agar/gui/begin.h>

/* Point from x,y coordinates */
typedef struct ag_pt {
	int x, y;
} AG_Pt;

/* Rectangle from upper-left coordinates + size */
typedef struct ag_rect {
	int x, y;		/* Upper-left */
	int w, h;		/* Size */
} AG_Rect;

/*
 * Rectangle from upper-left coords + size (& lower-right coords calculated), Or:
 * Rectangle from upper-left coords and lower-right coords (& size calculated).
 */
typedef struct ag_rect2 {
	int x1, y1;		/* Upper-left */
	int w, h;		/* Size */
	int x2, y2;		/* Lower-right */
} AG_Rect2;

/* Clipping rectangle */
typedef struct ag_clip_rect {
	AG_Rect r;		/* Clipping rectangle coordinates */
	double eqns[4][4];	/* Plane equations (calculated) */
} AG_ClipRect;

/* Floating-point texture coordinate */
typedef struct ag_texcoord {
	float x, y;		/* Upper left coordinate */
	float w, h;		/* Rectangle size */
} AG_TexCoord;

__BEGIN_DECLS
void AG_PtInit(AG_Pt *_Nonnull, int,int);

AG_Rect AG_RECT(int,int, int,int);

void AG_RectInit(AG_Rect *_Nonnull, int,int, int,int);
void AG_Rect2Init(AG_Rect2 *_Nonnull, int,int, int,int);
void AG_Rect2ToRect(AG_Rect *_Nonnull, const AG_Rect2 *_Nonnull);
void AG_RectToRect2(AG_Rect2 *_Nonnull, const AG_Rect *_Nonnull);

void AG_RectSize(AG_Rect *_Nonnull, int,int);
void AG_RectSize2(AG_Rect2 *_Nonnull, int,int);
void AG_RectTranslate(AG_Rect *_Nonnull, int,int);
void AG_RectTranslate2(AG_Rect2 *_Nonnull, int,int);

void AG_ReadRect(AG_Rect *_Nonnull, AG_DataSource *_Nonnull);
void AG_ReadRect2(AG_Rect2 *_Nonnull, AG_DataSource *_Nonnull);
void AG_WriteRect(AG_DataSource *_Nonnull, const AG_Rect *_Nonnull);
void AG_WriteRect2(AG_DataSource *_Nonnull, const AG_Rect2 *_Nonnull);

int AG_RectIntersect(AG_Rect *_Nonnull,
                     const AG_Rect *_Nonnull,
                     const AG_Rect *_Nonnull);

int AG_RectIntersect2(AG_Rect2 *_Nonnull,
                      const AG_Rect2 *_Nonnull,
                      const AG_Rect2 *_Nonnull);

int AG_RectCompare(const AG_Rect *_Nonnull,
                   const AG_Rect *_Nonnull)
                  _Pure_Attribute;

int AG_RectCompare2(const AG_Rect2 *_Nonnull,
                    const AG_Rect2 *_Nonnull)
                   _Pure_Attribute;

int AG_RectInside(const AG_Rect *_Nonnull, int,int) _Pure_Attribute;
int AG_RectInside2(const AG_Rect2 *_Nonnull, int,int) _Pure_Attribute;
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GEOMETRY_H_ */
