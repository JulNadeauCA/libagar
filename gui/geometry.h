/*	Public domain	*/

#ifndef _AGAR_GUI_GEOMETRY_H_
#define _AGAR_GUI_GEOMETRY_H_
#include <agar/gui/begin.h>

/* Point from x,y coordinates */
typedef struct ag_pt {
	int x, y;
} AG_Pt;

/* Rectangle from upper-left coordinates and size */
typedef struct ag_rect {
	int x, y;
	int w, h;
} AG_Rect;

/* Rectangle from upper-left and lower-right coordinates with cached size */
typedef struct ag_rect2 {
	int x1, y1;
	int w, h;
	int x2, y2;
} AG_Rect2;

typedef struct ag_clip_rect {
	AG_Rect r;		/* Clipping rectangle coordinates */
#ifdef AG_HAVE_FLOAT
	double eqns[4][4];	/* Cached plane equations (GL mode) */
#endif
} AG_ClipRect;

#ifdef AG_HAVE_FLOAT
typedef struct ag_texcoord {
	float x,y, w,h;		/* Texture coordinates (GL mode) */
} AG_TexCoord;
#endif

__BEGIN_DECLS
AG_Rect  AG_ReadRect(AG_DataSource *_Nonnull);
AG_Rect2 AG_ReadRect2(AG_DataSource *_Nonnull);
void     AG_WriteRect(AG_DataSource *_Nonnull, AG_Rect);
void     AG_WriteRect2(AG_DataSource *_Nonnull, AG_Rect2);

AG_Pt    AG_POINT(int,int) _Const_Attribute;
AG_Rect  AG_RECT(int,int, int,int) _Const_Attribute;
AG_Rect2 AG_RECT2(int,int, int,int) _Const_Attribute;
AG_Rect  AG_Rect2ToRect(AG_Rect2) _Const_Attribute;
void     AG_RectSize(AG_Rect *_Nonnull, int,int);
void     AG_RectSize2(AG_Rect2 *_Nonnull, int,int);
void     AG_RectTranslate(AG_Rect *_Nonnull, int,int);
void     AG_RectTranslate2(AG_Rect2 *_Nonnull, int,int);

AG_Rect2 AG_RectToRect2(AG_Rect)
                       _Const_Attribute;
AG_Rect  AG_RectIntersect(AG_Rect, AG_Rect)
                         _Const_Attribute;
AG_Rect2 AG_RectIntersect2(const AG_Rect2 *_Nonnull, const AG_Rect2 *_Nonnull)
                          _Pure_Attribute;
int      AG_RectCompare(AG_Rect, AG_Rect)
                       _Const_Attribute;
int      AG_RectCompare2(const AG_Rect2 *_Nonnull, const AG_Rect2 *_Nonnull)
                        _Pure_Attribute;
int      AG_RectInside(AG_Rect, int,int)
                      _Const_Attribute;
int      AG_RectInside2(const AG_Rect2 *_Nonnull, int,int)
                       _Pure_Attribute;
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GEOMETRY_H_ */
