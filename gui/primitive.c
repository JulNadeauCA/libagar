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

/* Import inlinables */
#undef AG_INLINE_HEADER
#include "inline_primitive.h"

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
		(((long) ((unsigned long) a ^ (unsigned long) b)) >= 0 )

int
AG_GetLineIntersection(long x1,long y1, long x2,long y2, long x3,long y3,
    long x4,long y4, long *xi,long *yi)
{
	long a1,a2, b1,b2, c1,c2;	/* Coefficients of line eqns. */
	long r1, r2, r3, r4;		/* 'Sign' values */
	long denom, offset, num;	/* Intermediate values */

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

#ifdef HAVE_FLOAT
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
AG_ClipLineCircle(int xc, int yc, int r, int x1, int y1, int x2, int y2,
    int *xi, int *yi)
{
	/* In the case that x1 and x2 are the same, there is a bug that causes
	 * m to be zero, resulting in no intersect being detected. */
	if (x1 == x2) {x1++;}

	/* the derivation here is fairly straightforward -- just calculate
	 * the y = mx+b form of the line segment, and then plug the y found
	 * thus into (x - x꜀)² + (y - y꜀)² = r²
	 *
	 * You should get an order-2 polynomial, the solution for which is
	 * trivially found with the quadratic formula.
	 */
	double m = ((double) y2 - (double) y1) / ((double) x2 - (double) x1);
	double k = -m * (double) x1 + (double) y1  - (double) yc;
	double a = 1 + AG_Square(m);
	double b = -2 * (double) xc + 2 * m * k;
	double c = -1 * (AG_Square((double) r) - AG_Square((double) xc) - AG_Square(k));

	/* no intersection */
	if (! AG_HaveQuadraticSolution(a, b, c)) {return;}

	/* calculate both possible intersections */
	double xs1, ys1, xs2, ys2;
	xs1 = AG_QuadraticPositive(a, b, c);
	xs2 = AG_QuadraticNegative(a, b, c);
	ys1 = m * (xs1 - (double) x1) + (double) y1;
	ys2 = m * (xs2 - (double) x1) + (double) y1;

	/* Pick the one closer to (x1, y1) and return it. If they are equal,
	 * then this docent matter and the answer will still be correct. */
	if (AG_Distance((double) x1, (double) y1, xs1, ys1) <
		AG_Distance((double) x1, (double) y1, xs2, ys2)) {
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

	long x1_d, y1_d, x2_d, y2_d;

	x1_d = (long) x1;
	y1_d = (long) y1;
	x2_d = (long) *x2;
	y2_d = (long) *y2;

	struct {
		long x1;
		long y1;
		long x2;
		long y2;
		long x3;
		long y3;
		long x4;
		long y4;
		long xi;
		long yi;
		int intersect;
	} faces[4] = {
		{x1_d, y1_d, x2_d, y2_d, ax   , ay   , ax   , ay+ah, 0, 0, 0},
		{x1_d, y1_d, x2_d, y2_d, ax   , ay+ah, ax+aw, ay+ah, 0, 0, 0},
		{x1_d, y1_d, x2_d, y2_d, ax+aw, ay+ah, ax+aw, ay   , 0, 0, 0},
		{x1_d, y1_d, x2_d, y2_d, ax+aw, ay   , ax   , ay   , 0, 0, 0},
	};

	unsigned short best = 0;
	double shortest_dist = -1;
	double dist;
	for (unsigned short i = 0 ; i < 4 ; i++) {
		faces[i].intersect = AG_GetLineIntersection(
			  faces[i].x1,
			  faces[i].y1,
			  faces[i].x2,
			  faces[i].y2,
			  faces[i].x3,
			  faces[i].y3,
			  faces[i].x4,
			  faces[i].y4,
			&(faces[i].xi),
			&(faces[i].yi));

		/* don't care about candidates that don't intersect with us */
		if (faces[i].intersect != 1) {continue;}

		dist = sqrt(pow(faces[i].xi-x1, 2) + pow(faces[i].yi-y1, 2));
		if ((shortest_dist == -1) || (dist < shortest_dist)) {
			best = i;
			shortest_dist = dist;
		}
	}

	/* no intersection, so leave x2, y2 as they are */
	if (shortest_dist < 0) { return; }

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
    double theta, const AG_Color *_Nonnull c)
{
	AG_Widget *wid = obj;
	AG_Pt P_start, P_tip,P1, P2;
	double V_dirx, V_diry, V_refx, V_refy, V_per1x, V_per1y, V_per2x, V_per2y;
	double width;


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
	double line_length = AG_Distance((double) P_tip.x,   (double) P_tip.y,
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
	V_refx = (double) length * V_dirx + (double) P_tip.x;
	V_refy = (double) length * V_diry + (double) P_tip.y;

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
#endif /* HAVE_FLOAT */
