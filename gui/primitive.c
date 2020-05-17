/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>.
 * Copyright (c) 2019 Charles A. Daniels <charles@cdaniels.net>.
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

#include <agar/core/core.h>
#include <agar/gui/primitive.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/opengl.h>

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_primitive.h"

/* No-op provided for symmetry with AG_Draw{Rect,Box,Frame}(). */
void
ag_draw_rect_noop(void *widget, const AG_Rect *rd, const AG_Color *c)
{
	/* nothing */
}

/*
 * Return 0 in the case that the lines ((x1,y1), (x2,y2)) and ((x3,y3),(x4,y4))
 * do not intersect, and 1 in the case that they do. If they do intersect, xi
 * and yi are set to the point of intersection, and otherwise unmodified.
 *
 * Originally written by by Mukesh Prasad in Graphics Gems II by James Arvo in
 * gem I.2, "Intersection of Line Segments".
 *
 * The intuitive background behind what's going on is described well by "Tricks
 * of the Windows Game Programming Gurus" by Andre Lamothe (1999), Chapter 8,
 * pp 413, "Computing the Intersection of Two Lines Using the Point Slope Form"
 *
 * The original code may be retrieved from
 * https://github.com/erich666/GraphicsGems/blob/master/gemsii/xlines.c
 *
 * This method uses integer math since it deals primarily with physical pixels
 * on the screen. If you need a highly accurate version, you should
 * re-implement with floating point. If you came here looking for a floating
 * point line intersection calculator, this one works pretty well:
 * https://stackoverflow.com/a/14795484
 */

/**************************************************************
 *                                                            *
 *    NOTE:  The following macro to determine if two numbers  *
 *    have the same sign, is for 2's complement number        *
 *    representation.  It will need to be modified for other  *
 *    number systems.                                         *
 *                                                            *
 **************************************************************/

#define SAME_SIGNS( a, b )	\
		(((int) ((unsigned int) a ^ (unsigned int) b)) >= 0 )

int
AG_GetLineIntersection(int x1,int y1, int x2,int y2, int x3,int y3,
    int x4,int y4, int *xi,int *yi)
{
	int a1,a2, b1,b2, c1,c2;	/* Coefficients of line eqns. */
	int r1, r2, r3, r4;		/* 'Sign' values */
	int denom, offset, num;	/* Intermediate values */

	/* Compute a1, b1, c1, where line joining points 1 and 2
	 * is "a1 x  +  b1 y  +  c1  =  0".
	 */
	a1 = y2 - y1;
	b1 = x1 - x2;
	c1 = x2*y1 - x1*y2;

	/* Compute r3 and r4.
	 */
	r3 = a1*x3 + b1*y3 + c1;
	r4 = a1*x4 + b1*y4 + c1;

	/* Check signs of r3 and r4.  If both point 3 and point 4 lie on
	 * same side of line 1, the line segments do not intersect.
	 */
	if (r3 != 0 &&
	    r4 != 0 &&
	    SAME_SIGNS(r3, r4))
		return (0);

	/* Compute a2, b2, c2 */
	a2 = y4 - y3;
	b2 = x3 - x4;
	c2 = x4*y3 - x3*y4;

	/* Compute r1 and r2 */
	r1 = a2*x1 + b2*y1 + c2;
	r2 = a2*x2 + b2*y2 + c2;

	/* Check signs of r1 and r2.  If both point 1 and point 2 lie
	 * on same side of second line segment, the line segments do
	 * not intersect.
	 */
	if (r1 != 0 &&
	    r2 != 0 &&
	    SAME_SIGNS(r1, r2))
		return (0);

	/* Line segments intersect: compute intersection point.
	 */
	denom = a1*b2 - a2*b1;
	if (denom == 0)
		return (0);

	/* The denom/2 is to get rounding instead of truncating.  It
	 * is added or subtracted to the numerator, depending upon the
	 * sign of the numerator.
	 */
	offset = denom < 0 ? - denom/2 : denom/2;

	num = b1*c2 - b2*c1;
	*xi = (num < 0 ? num-offset : num+offset) / denom;

	num = a2*c1 - a1*c2;
	*yi = (num < 0 ? num-offset : num+offset ) / denom;

	return (1);
}

#undef SAME_SIGNS

/* If the circle with the center (xc, yc) and radius r intersects with the line
 * (x1, y1), (x2, y2), then xi and yi are updated to reflect the point at which
 * the intersection occurs. In the event of two intersections, xi and yi are
 * set to the intersection point closest to the point (x1, y1).
 *
 * XXX: When using this to draw lines between circles in AG_Graph, some of the
 * results this returns seem a little strange... it seems to have a hard time
 * with horizontal approaches, and in sufficiently close vertical approaches
 * the intersection is detected on the wrong side of the circle. This may
 * be due to floating point precision errors as m becomes very small or very
 * large.
 */
void
AG_ClipLineCircle(int xc, int yc, int r, int x1i, int y1i, int x2, int y2,
    int *xi, int *yi)
{
	/* In the case that x1 and x2 are the same, there is a bug that causes
	 * m to be zero, resulting in no intersect being detected. */
	const double x1 = (x1i == x2) ? (double)(x1i+1) : (double)x1i;
	const double y1 = (double)y1i;

	/* the derivation here is fairly straightforward -- just calculate
	 * the y = mx+b form of the line segment, and then plug the y found
	 * thus into (x - x2)^2 + (y - y2)^2 = r^2
	 *
	 * You should get an order-2 polynomial, the solution for which is
	 * trivially found with the quadratic formula.
	 */
	const double m = ((double) y2 - y1) / ((double) x2 - x1);
	const double k = -m * x1 + y1  - (double) yc;
	const double a = 1 + AG_Square(m);
	const double b = -2 * (double) xc + 2 * m * k;
	const double c = -1 * (AG_Square((double) r) - AG_Square((double) xc) - AG_Square(k));
	double xs1, ys1, xs2, ys2;

	/* no intersection */
	if (! AG_HaveQuadraticSolution(a, b, c)) {return;}

	/* calculate both possible intersections */
	xs1 = AG_QuadraticPositive(a, b, c);
	xs2 = AG_QuadraticNegative(a, b, c);
	ys1 = m * (xs1 - x1) + y1;
	ys2 = m * (xs2 - x1) + y1;

	/* Pick the one closer to (x1, y1) and return it. If they are equal,
	 * then this docent matter and the answer will still be correct. */
	if (AG_Distance(x1,y1, xs1,ys1) <
		AG_Distance(x1,y1, xs2,ys2)) {
		*xi = (int) xs1;
		*yi = (int) ys1;
	} else {
		*xi = (int) xs2;
		*yi = (int) ys2;
	}
}

/*
 * Given a widget object and a line, clip the line such that it ends on the
 * border of the object. In other words, set x2, y2 to the intersect of the
 * line and the object. If the line does not intersect the object, then x2
 * and y2 are unchanged.
 */
void
AG_ClipLine(int ax, int ay, int aw, int ah, int x1, int y1, int *x2, int *y2)
{
	/* We simply compute the intersection with every possible face, and
	 * pick the one which is smallest. It _might_ be faster to decide which
	 * face we are "approaching" from, but I think the edge case handling
	 * logic would add a lot of complexity */

	/*
	 *       a.x, a.y   +------+ a.x + a.w, a.y
	 *                  |      |
	 *                  |      |
	 *                  |      |
	 *                  |      |
	 *   a.x, a.y + a.h +------+ a.x + a.w, a.y + a.h
	*/
	const int x2_d = *x2;
	const int y2_d = *y2;
	struct {
		int x1, y1;
		int x2, y2;
		int x3, y3;
		int x4, y4;
		int xi, yi;
		int intersect;
		Uint32 _pad;
	} faces[4] = {
		{x1, y1, x2_d, y2_d, ax   , ay   , ax   , ay+ah, 0, 0, 0},
		{x1, y1, x2_d, y2_d, ax   , ay+ah, ax+aw, ay+ah, 0, 0, 0},
		{x1, y1, x2_d, y2_d, ax+aw, ay+ah, ax+aw, ay   , 0, 0, 0},
		{x1, y1, x2_d, y2_d, ax+aw, ay   , ax   , ay   , 0, 0, 0},
	};
	double shortest_dist = -1;
	double dist;
	Uint i, best=0;

	for (i = 0 ; i < 4 ; i++) {
		faces[i].intersect = AG_GetLineIntersection(
		    faces[i].x1,    faces[i].y1,
		    faces[i].x2,    faces[i].y2,
		    faces[i].x3,    faces[i].y3,
		    faces[i].x4,    faces[i].y4,
		  &(faces[i].xi), &(faces[i].yi) );

		/* don't care about candidates that don't intersect with us */
		if (faces[i].intersect != 1)
			continue;

		dist = sqrt(pow(faces[i].xi-x1, 2) + pow(faces[i].yi-y1, 2));
		if ((shortest_dist == -1) || (dist < shortest_dist)) {
			best = i;
			shortest_dist = dist;
		}
	}

	/* no intersection, so leave x2, y2 as they are */
	if (shortest_dist < 0)
		return;

	*x2 = (int) faces[best].xi;
	*y2 = (int) faces[best].yi;
}

/*
 * Render an arrowhead. The arrowhead will be parallel to the given line (x1,
 * y1), (x2, y2), and will be drawing pointing to (x2, y2).
 *
 * length refers to the length in pixels of the two sides that intersect at
 * the tip of the arrowhead.
 *
 * theta refers to the angle between the two sides that intersect at the tip of
 * the arrowhead.
 *
 */
void
AG_DrawArrowhead(void *obj, int x1, int y1, int x2, int y2, int length,
    double theta, const AG_Color *c)
{
	AG_Widget *wid = obj;
	AG_Pt P_start, P_tip,P1, P2;
	double V_dirx, V_diry, V_per1x, V_per1y, V_per2x, V_per2y;
	/* double V_refx, V_refy */
	double width;
	double line_length;


	/*         AG_Sqrt( */
	/*         ((double) P_tip.x - (double) P_start.x) * ((double) P_tip.x - (double) P_start.x) + */
	/*         ((double) P_tip.y - (double) P_start.y) * ((double) P_tip.y - (double) P_start.y) */
	/* ); */


	/* start and end of the actual line */
	P_start.x = x1;
	P_start.y = y1;
	P_tip.x   = x2;
	P_tip.y   = y2;

	/* length of overall line, necessary to generate direction reference
	 * unit vector */
	line_length = AG_Distance((double) P_tip.x,   (double) P_tip.y,
	                          (double) P_start.x, (double) P_start.y);

	/* Avoid a divide by zero error later -- we can't draw an error for
	 * a zero-length line anyway.
	 *
	 * XXX: maybe this should trigger an error condition or something?
	 *
	 * XXX: might want to use an epsilon test?
	 * */
	if (line_length == 0)
		return;

	/* direction reference vector -- this intentionally points the
	 * "wrong" way since it makes it convenient to calculate P_ref and
	 * we don't need this for anything else */
	V_dirx = ((double) P_start.x - (double) P_tip.x) / (line_length);
	V_diry = ((double) P_start.y - (double) P_tip.y) / (line_length);

	/* reference vector, this is a point a bit back from the tip that we
	 * use to compute some trig later to find the corner points */
	/* V_refx = (double) length * V_dirx + (double) P_tip.x; */
	/* V_refy = (double) length * V_diry + (double) P_tip.y; */

	/* perpendicular to the direction unit vector */
	V_per1x =      V_diry;
	V_per1y = -1 * V_dirx;
	V_per2x = -1 * V_diry;
	V_per2y =      V_dirx;

	/* distance between the two non-tip vertices */
	width = (int) (2.0 * (Tan(((double) theta) / 4.0 ) * 2.0 * (double) length));

	/* calculate the non-tip corners */
	P1.x = (int) ((double) P_tip.x + (V_dirx * (double) length) + (width / 2.0) * V_per1x);
	P1.y = (int) ((double) P_tip.y + (V_diry * (double) length) + (width / 2.0) * V_per1y);
	P2.x = (int) ((double) P_tip.x + (V_dirx * (double) length) + (width / 2.0) * V_per2x);
	P2.y = (int) ((double) P_tip.y + (V_diry * (double) length) + (width / 2.0) * V_per2y);

	/* wid->drvOps->drawTriangle(wid->drv, &P_tip, &P1, &P2, c); */

	AG_DrawTriangle(wid, &P_tip, &P1, &P2, c);
}

/*
 * Render a vector graphic made from a display list of elements on a fixed
 * size grid of MxN vertices. The drawing is scaled to fit r (which is given
 * in display coordinates).
 *
 * Choose the smallest possible grid for a given drawing to optimize coordinate
 * conversion. Select an odd or even grid to achieve pixel-perfect alignment.
 * Use elemFirst-elemLast to index and reuse a subset of an existing drawing.
 *
 * Elements are straight lines, outline polygons, outline circular arcs,
 * filled polygons and filled circular arcs. Per-element attributes include
 * line thickness, color and endpoint style. Attributes are non-modal.
 *
 *  2 x 2    3 x 3        4 x 4             5 x 5                6 x 6
 * .-----. .-------. .-------------. .-----------------. .-------------------. 
 * | 0 1 | | 0 1 2 | |  0  1  2  3 | |  0  1  2  3  4  | |  0  1  2  3  4  5 |
 * | 2 3 | | 3 4 5 | |  4  5  6  7 | |  5  6  7  8  9  | |  6  7  8  9 10 11 |
 * `-----' | 6 7 8 | |  8  9 10 11 | | 10 11 12 13 14  | | 12 13 14 15 16 17 |
 *         `-------' | 12 13 14 15 | | 15 16 17 18 19  | | 18 19 20 21 22 23 |
 *                   `-------------' | 20 21 22 23 24  | | 24 25 26 27 28 29 |
 *            8 x 8                  `-----------------' | 30 31 32 33 34 35 |
 * .-------------------------.                           `-------------------'
 * |  0  1  2  3  4  5  6  7 |      M x N       [ cases ]
 * |  8  9 10 11 12 13 14 15 |  .-----------.   2x2: precalc (fastest)
 * | 16 17 18 19 20 21 22 23 |  | 0 ....N-1 |   3x3: precalc + shift
 * | 24 25 26 27 28 29 30 31 |  | ......... |   4x4: precalc + 2 shifts
 * | 32 33 34 35 36 37 38 39 |  | ......... |   8x8: stream + shifts
 * | 40 41 42 43 44 45 46 47 |  | ... M*N-1 |   MxN: stream + multiply, divide
 * | 48 49 50 51 52 53 54 55 |  `-----------'   
 * | 56 57 58 59 60 61 62 63 |
 * `-------------------------'
 */
static void DrawVector2x2(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull, int,int, const AG_VectorElement *_Nonnull, int,int);
static void DrawVector3x3(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull, int,int, const AG_VectorElement *_Nonnull, int,int);
static void DrawVector4x4(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull, int,int, const AG_VectorElement *_Nonnull, int,int);
static void DrawVector8x8(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull, int,int, const AG_VectorElement *_Nonnull, int,int);
static void DrawVectorMxN(void *_Nonnull, const AG_Rect *_Nonnull, const AG_Color *_Nonnull, int,int, const AG_VectorElement *_Nonnull, int,int);
void
AG_DrawVector(void *obj, int m, int n, const AG_Rect *r, const AG_Color *pal,
    const AG_VectorElement *elemBase, int elemFirst, int elemLast)
{
	static void (*pf[])(void *_Nonnull, const AG_Rect *, const AG_Color *,
	                    int,int, const AG_VectorElement *, int,int) = {
		DrawVector2x2,
		DrawVector3x3,
		DrawVector4x4,
		DrawVectorMxN,	/* 5x5 */
		DrawVectorMxN,	/* 6x6 */
		DrawVectorMxN,	/* 7x7 */
		DrawVector8x8,
		DrawVectorMxN	/* 9x9 (or MxN) */
	};
#ifdef AG_DEBUG
	if (m < 2 || n < 2) { AG_FatalError("m or n < 2"); }
#endif
	if (m != n || n > 9) {
		n = 9;
	}
	pf[n-2](obj, r, pal, m,n, elemBase,elemFirst,elemLast);
}

static __inline__ void
DrawVectorFixed(void *_Nonnull obj, const AG_Pt *_Nonnull vtx, Uint nVtx,
    const AG_Color *_Nonnull pal,
    const AG_VectorElement *_Nonnull elemBase, Uint elemFirst, Uint elemLast)
{
	AG_Widget *wid = obj;
	int a,b, i;

#ifdef AG_DEBUG
	AG_OBJECT_ISA(wid, "AG_Widget:*");

	if (elemFirst > elemLast)
		AG_FatalError("elemFirst>elemLast");
#endif
	for (i = elemFirst; i < elemLast; i++) {
		const AG_VectorElement *elem = &elemBase[i];

		switch (elem->type) {
		case AG_VE_LINE:
			if ((a = elem->a) >= 0 && a < nVtx &&
			    (b = elem->b) >= 0 && b < nVtx) {
				AG_DrawLine(wid,
				    vtx[a].x, vtx[a].y,
				    vtx[b].x, vtx[b].y,
				    &pal[elem->color]);
			}
			break;
		case AG_VE_POLYGON:
			if ((a = elem->a) >= 0 &&
			    (b = elem->b) >= 0 &&
			    (b > a)) {
				const int nPts = b - a;
				AG_Pt *pts = Malloc(nPts * sizeof(AG_Pt));
				const int *poly = elem->p;
				int j;

				for (j = 0; j < nPts; j++) {
					const int vi = poly[a+j];
					pts[j].x = wid->rView.x1 + vtx[vi].x;
					pts[j].y = wid->rView.y1 + vtx[vi].y;
				}
				AG_DrawPolygon(wid, pts, nPts, &pal[elem->color]);
				free(pts);
			}
			break;
		case AG_VE_CIRCLE:
			if ((a = elem->a) >= 0 && a < nVtx) {
				if (elem->flags & AG_VE_FILLED) {
					AG_DrawCircleFilled(wid,
					    vtx[a].x, vtx[a].y,
					    elem->b,
					    &pal[elem->color]);
				} else {
					AG_DrawCircle(wid,
					    vtx[a].x, vtx[a].y,
					    elem->b,
					    &pal[elem->color]);
				}
			}
			break;
		case AG_VE_ARC1:
		case AG_VE_ARC2:
		case AG_VE_ARC3:
		case AG_VE_ARC4:
			break;
		case AG_VE_POINT:
		default:
			if ((a = elem->a) >= 0 && a < nVtx) {
				AG_DrawLineH(wid,
				    vtx[a].x-4, vtx[a].x-4,
				    vtx[a].y,
				    &pal[elem->color]);
				AG_DrawLineV(wid,
				    vtx[a].x,
				    vtx[a].y-4, vtx[a].y+4,
				    &pal[elem->color]);
			}
			break;
		}
	}
}

static void
DrawVector2x2(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull pal, int m, int n,
    const AG_VectorElement *_Nonnull elemBase, int elemFirst, int elemLast)
{
	const int x = r->x + 1;
	const int y = r->y + 1;
	const int w = r->w - 3;
	const int h = r->h - 3;
	AG_Pt vtx[4];

	vtx[0].x = x;	vtx[0].y = y;
	vtx[1].x = x+w;	vtx[1].y = y;
	vtx[2].x = x;	vtx[2].y = y+h;
	vtx[3].x = x+w;	vtx[3].y = y+h;

	if (w > 0 && h > 0)
		DrawVectorFixed(obj, vtx,4, pal, elemBase,elemFirst,elemLast);
}

static void
DrawVector3x3(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull pal, int m, int n,
    const AG_VectorElement *_Nonnull elemBase, int elemFirst, int elemLast)
{
	AG_Pt vtx[9];
	const int x = r->x;
	const int y = r->y;
	int w = r->w, w_2 = (w >> 1) + ((w & 1) ? 1 : 0);
	int h = r->h, h_2 = (h >> 1);

	vtx[0].x = x;		vtx[0].y = y;
	vtx[1].x = x+w_2;	vtx[1].y = y;
	vtx[2].x = x+w;		vtx[2].y = y;
	vtx[3].x = x;		vtx[3].y = y+h_2;
	vtx[4].x = x+w_2;	vtx[4].y = y+h_2;
	vtx[5].x = x+w;		vtx[5].y = y+h_2;
	vtx[6].x = x;		vtx[6].y = y+h;
	vtx[7].x = x+w_2;	vtx[7].y = y+h;
	vtx[8].x = x+w;		vtx[8].y = y+h;

	if (w > 0 && h > 0)
		DrawVectorFixed(obj, vtx,9, pal, elemBase,elemFirst,elemLast);
}

static void
DrawVector4x4(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull pal, int m, int n,
    const AG_VectorElement *_Nonnull elemBase, int elemFirst, int elemLast)
{
	const int x = r->x + 1;
	const int y = r->y + 1;
	const int w = r->w - 3, w_4 = w>>2, w_4x2 = w_4<<1;
	const int h = r->h - 3, h_4 = h>>2, h_4x2 = h_4<<1;
	AG_Pt vtx[16];

	vtx[0].x = x;		vtx[0].y = y;
	vtx[1].x = x+w_4;	vtx[1].y = y;
	vtx[2].x = x+w_4x2;	vtx[2].y = y;
	vtx[3].x = x+w;		vtx[3].y = y;
	vtx[4].x = x;		vtx[4].y = y+h_4;
	vtx[5].x = x+w_4;	vtx[5].y = y+h_4;
	vtx[6].x = x+w_4x2;	vtx[6].y = y+h_4;
	vtx[7].x = x+w;		vtx[7].y = y+h_4;
	vtx[8].x = x;		vtx[8].y = y+h_4x2;
	vtx[9].x = x+w_4;	vtx[9].y = y+h_4x2;
	vtx[10].x = x+w_4x2;	vtx[10].y = y+h_4x2;
	vtx[11].x = x+w;	vtx[11].y = y+h_4x2;
	vtx[12].x = x;		vtx[12].y = y+h;
	vtx[13].x = x+w_4;	vtx[13].y = y+h;
	vtx[14].x = x+w_4x2;	vtx[14].y = y+h;
	vtx[15].x = x+w;	vtx[15].y = y+h;

	if (w > 0 && h > 0)
		DrawVectorFixed(obj, vtx,16, pal, elemBase,elemFirst,elemLast);
}

static void
DrawVector8x8(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull pal, int m, int n,
    const AG_VectorElement *_Nonnull elemBase, int elemFirst, int elemLast)
{
	const int w = r->w;
	const int h = r->h;
	int a,b, i;

#ifdef AG_DEBUG
	AG_OBJECT_ISA(obj, "AG_Widget:*");

	if (elemFirst < 0 || elemFirst > elemLast)
		AG_FatalError("elemFirst/elemLast");
#endif
	for (i = elemFirst; i < elemLast; i++) {
		const AG_VectorElement *elem = &elemBase[i];

		switch (elem->type) {
		case AG_VE_LINE:
			a = elem->a >> 3;
			b = elem->b >> 3;
			AG_DrawLine(obj,
			    r->x + a*w, r->y + a*h,
			    r->x + b*w, r->y + b*h,
			    &pal[elem->color]);
			break;
		case AG_VE_POLYGON:
			if ((a = elem->a) >= 0 && (b = elem->b) >= 0 &&
			    (b > a)) {
				const int nPts = b - a;
				AG_Pt *pts = Malloc(nPts * sizeof(AG_Pt));
				const int *poly = elem->p;
				int j;

				for (j=0; j < nPts; j++) {
					pts[j].x = r->x + (poly[a+j] >> 3)*w;
					pts[j].y = r->y + (poly[a+j] >> 3)*h;
				}
				AG_DrawPolygon(obj, pts, nPts, &pal[elem->color]);
				free(pts);
			}
			break;
		case AG_VE_CIRCLE:
			a = elem->a >> 3;
			if (elem->flags & AG_VE_FILLED) {
				AG_DrawCircleFilled(obj,
				    r->x + a*w,
				    r->y + a*h,
				    elem->b, &pal[elem->color]);
			} else {
				AG_DrawCircle(obj,
				    r->x + a*w,
				    r->y + a*h,
				    elem->b, &pal[elem->color]);
			}
			break;
		case AG_VE_ARC1:
		case AG_VE_ARC2:
		case AG_VE_ARC3:
		case AG_VE_ARC4:
			break;
		case AG_VE_POINT:
		default:
			a = elem->a >> 3;
			AG_DrawLineH(obj,
			    r->x + a*w - 4,
			    r->x + a*w - 4,
			    r->y + a*h,
			    &pal[elem->color]);
			AG_DrawLineV(obj,
			    r->x + a*w,
			    r->y + a*h - 4,
			    r->y + a*h + 4,
			    &pal[elem->color]);
			break;
		}
	}
}

static void
DrawVectorMxN(void *_Nonnull obj, const AG_Rect *_Nonnull r,
    const AG_Color *_Nonnull pal, int m, int n,
    const AG_VectorElement *_Nonnull elemBase, int elemFirst, int elemLast)
{
	const int w = r->w;
	const int h = r->h;
	int a,b, i;

#ifdef AG_DEBUG
	AG_OBJECT_ISA(obj, "AG_Widget:*");

	if (elemFirst < 0 || elemFirst > elemLast)
		AG_FatalError("elemFirst/elemLast");
#endif
	for (i = elemFirst; i < elemLast; i++) {
		const AG_VectorElement *elem = &elemBase[i];

		switch (elem->type) {
		case AG_VE_LINE:
			a = elem->a;
			b = elem->b;
			AG_DrawLine(obj,
			    r->x + (int)((double)a/(double)m*(double)w),
			    r->y + (int)((double)a/(double)n*(double)h),
			    r->x + (int)((double)b/(double)m*(double)w),
			    r->y + (int)((double)b/(double)n*(double)h),
			    &pal[elem->color]);
			break;
		case AG_VE_POLYGON:
			if ((a = elem->a) >= 0 &&
			    (b = elem->b) >= 0 &&
			    (b > a)) {
				const int nPts = b - a;
				AG_Pt *pts = Malloc(nPts * sizeof(AG_Pt));
				const int *poly = elem->p;
				int j;

				for (j=0; j < nPts; j++) {
					pts[j].x = r->x + poly[a+j]/m*w;
					pts[j].y = r->y + poly[a+j]/n*h;
				}
				AG_DrawPolygon(obj, pts, nPts, &pal[elem->color]);
				free(pts);
			}
			break;
		case AG_VE_CIRCLE:
			a = elem->a;
			if (elem->flags & AG_VE_FILLED) {
				AG_DrawCircleFilled(obj,
				    r->x + a/m*w,
				    r->y + a/n*h,
				    elem->b, &pal[elem->color]);
			} else {
				AG_DrawCircle(obj,
				    r->x + a/m*w,
				    r->y + a/n*h,
				    elem->b, &pal[elem->color]);
			}
			break;
		case AG_VE_ARC1:
		case AG_VE_ARC2:
		case AG_VE_ARC3:
		case AG_VE_ARC4:
			break;
		case AG_VE_POINT:
		default:
			a = elem->a;
			AG_DrawLineH(obj,
			    r->x + a/m*w - 4,
			    r->x + a/m*w - 4,
			    r->y + a/n*h,
			    &pal[elem->color]);
			AG_DrawLineV(obj,
			    r->x + a/m*w,
			    r->y + a/n*h - 4,
			    r->y + a/n*h + 4,
			    &pal[elem->color]);
			break;
		}
	}
}

/* Called by AG_DrawFrame() to handle non-opaque case. */
void
AG_DrawFrame_Blended(AG_Widget *wid, const AG_Rect *rd, const AG_Color *c1,
    const AG_Color *c2, int x2, int y2)
{
	AG_Driver *drv = wid->drv;
	AG_DriverClass *drvOps = wid->drvOps;

	drvOps->drawLineBlended(drv, rd->x, rd->y, x2,    rd->y, c1, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	drvOps->drawLineBlended(drv, rd->x, rd->y, rd->x, y2,    c1, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	drvOps->drawLineBlended(drv, rd->x, y2,    x2,    y2,    c2, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	drvOps->drawLineBlended(drv, x2,    rd->y, x2,    y2,    c2, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
}

