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
void     AG_WriteRect(AG_DataSource *_Nonnull, AG_Rect);

#define  AG_ReadRect2(ds)    AG_RectToRect2(AG_ReadRect(ds))
#define  AG_WriteRect2(ds,r) AG_WriteRect(ds, AG_Rect2ToRect(r))

/* Return a Point at x,y. */
static __inline__ AG_Pt _Const_Attribute
AG_POINT(int x, int y)
{
	AG_Pt pt;
	pt.x = x;
	pt.y = y;
	return (pt);
}

/* Return a Rect of dimensions w,h at position x,y. */
static __inline__ AG_Rect _Const_Attribute
AG_RECT(int x, int y, int w, int h)
{
	AG_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

/* Return a Rect2 of dimensions w,h at position x,y. */
static __inline__ AG_Rect2 _Const_Attribute
AG_RECT2(int x, int y, int w, int h)
{
	AG_Rect2 r;
	r.x1 = x;
	r.y1 = y;
	r.w = w;
	r.h = h;
	r.x2 = x+w;
	r.y2 = y+h;
	return (r);
}

/* Convert a Rect2 to a Rect. */
static __inline__ AG_Rect _Const_Attribute
AG_Rect2ToRect(AG_Rect2 r2)
{
	AG_Rect r;
	r.x = r2.x1;
	r.y = r2.y1;
	r.w = r2.w;
	r.h = r2.h;
	return (r);
}

/* Resize a Rect. */
static __inline__ void
AG_RectSize(AG_Rect *_Nonnull r, int w, int h)
{
	r->w = w;
	r->h = h;
}

/* Resize a Rect2. */
static __inline__ void
AG_RectSize2(AG_Rect2 *_Nonnull r, int w, int h)
{
	r->w = w;
	r->x2 = r->x1+w;
	r->h = h;
	r->y2 = r->y1+h;
}

/* Translate a Rect. */
static __inline__ void
AG_RectTranslate(AG_Rect *_Nonnull r, int x, int y)
{
	r->x += x;
	r->y += y;
}

/* Translate a Rect2. */
static __inline__ void
AG_RectTranslate2(AG_Rect2 *_Nonnull r, int x, int y)
{
	r->x1 += x;
	r->y1 += y;
	r->x2 += x;
	r->y2 += y;
}

/* Convert a Rect to a Rect2. */
static __inline__ AG_Rect2 _Const_Attribute
AG_RectToRect2(AG_Rect r)
{
	AG_Rect2 r2;
	r2.x1 = r.x;
	r2.y1 = r.y;
	r2.w = r.w;
	r2.h = r.h;
	r2.x2 = r.x+r.w;
	r2.y2 = r.y+r.h;
	return (r2);
}

/* Return the intersection of two Rect's. */
static __inline__ AG_Rect _Const_Attribute
AG_RectIntersect(AG_Rect a, AG_Rect b)
{
	AG_Rect x;
	x.x = AG_MAX(a.x, b.x);
	x.y = AG_MAX(a.y, b.y);
	x.w = AG_MIN((a.x + a.w), (b.x + b.w)) - x.x;
	x.h = AG_MIN((a.y + a.h), (b.y + b.h)) - x.y;
	if (x.w < 0) { x.w = 0; }
	if (x.h < 0) { x.h = 0; }
	return (x);
}

/* Return the intersection of two Rect2's. */
static __inline__ AG_Rect2 _Pure_Attribute
AG_RectIntersect2(const AG_Rect2 *_Nonnull a, const AG_Rect2 *_Nonnull b)
{
	AG_Rect2 rx;

	rx.x1 = AG_MAX(a->x1, b->x1);
	rx.y1 = AG_MAX(a->y1, b->y1);
	rx.w = AG_MIN(a->x2, b->x2) - rx.x1;
	if (rx.w < 0) { rx.w = 0; }
	rx.h = AG_MIN(a->y2, b->y2) - rx.y1;
	if (rx.h < 0) { rx.h = 0; }
	rx.x2 = rx.x1 + rx.w;
	rx.y2 = rx.y1 + rx.h;
	return (rx);
}

/* Test whether two Rect's are the same. */
static __inline__ int _Const_Attribute
AG_RectCompare(AG_Rect a, AG_Rect b)
{
	return !(a.x == b.x &&
	         a.y == b.y &&
		 a.w == b.w &&
		 a.h == b.h);
}

/* Test whether two (valid) Rect2's are the same. */
static __inline__ int _Pure_Attribute
AG_RectCompare2(const AG_Rect2 *_Nonnull a, const AG_Rect2 *_Nonnull b)
{
	return !(a->x1 == b->x1 &&
	         a->y1 == b->y1 &&
		 a->w == b->w &&
		 a->h == b->h);
}

/* Test whether a point intersects a Rect. */
static __inline__ int _Const_Attribute
AG_RectInside(AG_Rect r, int x, int y)
{
	return (x >= r.x &&
	        y >= r.y &&
		x <  r.x + r.w &&
		y <  r.y + r.h); 
}

/* Test whether a point intersects a Rect2. */
static __inline__ int _Pure_Attribute
AG_RectInside2(const AG_Rect2 *_Nonnull r, int x, int y)
{
	return (x >= r->x1 &&
	        y >= r->y1 &&
		x <  r->x2 &&
		y <  r->y2); 
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GEOMETRY_H_ */
