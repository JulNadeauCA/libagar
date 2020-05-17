/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * These functions implement placement of entities based on their constraints
 * with respect to either single entity or a set of two constrained entities.
 *
 * They are implemented as generic sketch instructions. These instructions are
 * usually going to be generated in the degree-of-freedom analysis stage.
 */ 

#include <agar/core/core.h>

#include "sk.h"
#include "sk_placement.h"

/*
 * Basic Point-Distance-Point placement. We preserve the original angle of
 * the line defined by the two points.
 */
static int
PtFromPtAtDistance(SK_Constraint *_Nonnull ct, void *_Nonnull self,
    void *_Nonnull other)
{
	M_Vector3 p1 = SK_Pos(self);
	M_Vector3 p2 = SK_Pos(other);
	M_Real theta;
	
	M_VecVecAngle3(p1, p2, &theta, NULL);
	SK_Identity(self);
	p2.x -= ct->ct_distance*Cos(theta);
	p2.y -= ct->ct_distance*Sin(theta);
	SK_Translatev(self, &p2);
	return (0);
}

/*
 * Basic Point-Distance-Line placement. We move the point in a perpendicular
 * fashion with respect to its original position.
 */
static int
PtFromLineAtDistance(SK_Constraint *_Nonnull ct, void *_Nonnull self,
    void *_Nonnull other)
{
	SK_Line *L = other;
	M_Vector3 pOrig = SK_Pos(self);
	M_Vector3 p1 = SK_Pos(L->p1);
	M_Vector3 p2 = SK_Pos(L->p2);
	M_Vector3 v, vd, s1, s2;
	M_Real mag, u, theta;

	/*
	 * Skip unnecessary computations if this is an incidence relation
	 * between a line and its endpoints.
	 */
	if (self == L->p1 || self == L->p2) {
		return (0);
	}

	/* Find the closest point on the line. */
	vd = M_VecSub3p(&p2, &p1);
	mag = M_VecDistance3(p2, p1);
	u = ( ((pOrig.x - p1.x)*(p2.x - p1.x)) +
	      ((pOrig.y - p1.y)*(p2.y - p1.y)) ) / (mag*mag);
	
	v = M_VecAdd3(p1, M_VecScale3p(&vd,u));
	if (ct->ct_distance == 0.0) {
		SK_Identity(self);
		SK_Translatev(self, &v);
		return (0);
	}
	M_VecVecAngle3(p1, p2, &theta, NULL);
	theta += M_PI/2.0;
	s1.x = v.x + ct->ct_distance*Cos(theta);
	s1.y = v.y + ct->ct_distance*Sin(theta);
	s2.x = v.x - ct->ct_distance*Cos(theta);
	s2.y = v.y - ct->ct_distance*Sin(theta);
	
	SK_Identity(self);
	if (M_VecDistance3p(&pOrig,&s1) < M_VecDistance3p(&pOrig,&s2)) {
		SK_Translatev(self, &s1);
	} else {
		SK_Translatev(self, &s2);
	}
	return (0);
}

#if 0
static int
PtFromDistantCircle(SK_Constraint *_Nonnull ct, void *_Nonnull self,
    void *_Nonnull other)
{
	SK_Point *p = self;
	SK_Circle *C1 = other;
	M_Vector3 p1 = SK_Pos(C1->p);
	
	SK_Identity(p);
	SK_Translatev(p, &p1);
	SK_Translate2(p, C1->r + ct->ct_distance, 0.0);
	return (0);
}
#endif

/*
 * Basic Line-Distance-Line placement.
 * XXX parallel
 */
static int
LineFromLineAtDistance(SK_Constraint *_Nonnull ct, void *_Nonnull self,
    void *_Nonnull other)
{
	SK_Line *L = self;
	SK_Line *L1 = other;

	SK_MatrixCopy(L->p1, L1->p1);
	SK_MatrixCopy(L->p2, L1->p2);
	SK_Translate2(L->p1, ct->ct_distance, 0.0);
	SK_Translate2(L->p2, ct->ct_distance, 0.0);
	return (0);
}

/*
 * Basic Line-Angle-Line placement.
 * XXX
 */
static int
LineFromLineAtAngle(SK_Constraint *_Nonnull ct, void *_Nonnull pSelf,
    void *_Nonnull pFixed)
{
	SK_Line *self = pSelf;
	SK_Line *fixed = pFixed;
	M_Vector3 vShd, vSelf, vFixed;
	M_Real len, theta;
	SK_Point *p;

	theta = ct->ct_angle;

	if (self->p1 == fixed->p1 || self->p1 == fixed->p2) {
		vShd = SK_Pos(self->p1);
		vSelf = SK_Pos(self->p2);
		vFixed = (self->p1==fixed->p1) ? SK_Pos(fixed->p2) :
		                                 SK_Pos(fixed->p1);
		p = self->p2;
//		theta = -ct->ct_angle;
	} else if (self->p2 == fixed->p1 || self->p2 == fixed->p2) {
		vShd = SK_Pos(self->p2);
		vSelf = SK_Pos(self->p1);
		vFixed = (self->p2==fixed->p1) ? SK_Pos(fixed->p2) :
		                                 SK_Pos(fixed->p1);
		p = self->p1;
//		theta = ct->ct_angle;
	} else {
		AG_SetError("No shared point between lines");
		return (-1);
	}
	M_VecSub3v(&vSelf, &vShd);
	M_VecSub3v(&vFixed, &vShd);
	len = M_VecLen3p(&vSelf);
	M_VecNorm3v(&vFixed);
	(void)M_VecSub3(vFixed, M_VecNorm3(vSelf));
	SK_Identity(p);
	SK_Translate2(p,
	    vShd.x + Cos(theta)*vFixed.x*len - Sin(theta)*vFixed.y*len,
	    vShd.y + Sin(theta)*vFixed.x*len + Cos(theta)*vFixed.y*len);
	return (0);
}

/*
 * Compute the position of a point relative to two fixed points from
 * distance/incidence constraints. This is a system of two quadratic
 * equations describing the intersection of two circles:
 *
 *     x^2 + y^2 = c^2
 * (x-a)^2 + y^2 = b^2
 *
 * If there are multiple solutions for a, we pick whichever minimizes
 * the displacement from the original point position.
 */
static int
PtFromPtPt(void *_Nonnull self, SK_Constraint *_Nonnull ct1, void *_Nonnull n1,
    SK_Constraint *_Nonnull ct2, void *_Nonnull n2)
{
	SK *sk = SKNODE(self)->sk;
	M_Vector3 pOrig = SK_Pos(self);
	M_Vector3 p1 = SK_Pos(n1);
	M_Vector3 p2 = SK_Pos(n2);
	M_Real d1 = ct1->ct_distance;
	M_Real d2 = ct2->ct_distance;
	M_Real d12 = M_VecDistance3p(&p1,&p2);
	M_Real a, h, b;
	M_Vector3 p, s1, s2;

	if (ct1->type != SK_DISTANCE || ct2->type != SK_DISTANCE) {
		AG_SetError("Expect Distance with %s and %s",
		    SKNODE(n1)->name, SKNODE(n2)->name);
		return (-1);
	}
	if (d12 > (d1+d2)) {
		AG_SetError("%s and %s are too far apart to satisfy "
		            "constraint: %.02f > (%.02f+%.02f)",
			    SKNODE(n1)->name, SKNODE(n2)->name, d12, d1, d2);
		return (-1);
	}
	if (d12 < Fabs(d1-d2)) {
		AG_SetError("%s and %s are too close to satisfy "
		            "constraint: %.02f < |%.02f-%.02f|",
			    SKNODE(n1)->name, SKNODE(n2)->name, d12, d1, d2);
		return (-1);
	}

	a = (d1*d1 - d2*d2 + d12*d12) / (2.0*d12);
	h = Sqrt(d1*d1 - a*a);
	p = M_VecLERP3p(&p1, &p2, a/d12);
	b = h/d12;

	s1.x = p.x - b*(p2.y - p1.y);
	s1.y = p.y + b*(p2.x - p1.x);
	s1.z = 0.0;
	s2.x = p.x + b*(p2.y - p1.y);
	s2.y = p.y - b*(p2.x - p1.x);
	s2.z = 0.0;

	SK_Identity(self);
	if (M_VecDistance3p(&s1,&s2) == 0.0) {
		SK_Translatev(self, &s1);
		sk->nSolutions++;
	} else {
		if (M_VecDistance3p(&pOrig,&s1) < M_VecDistance3p(&pOrig,&s2)) {
			SK_Translatev(self, &s1);
		} else {
			SK_Translatev(self, &s2);
		}
		sk->nSolutions += 2;
	}
	return (0);
}

/*
 * Place point at distance d1 from a point and distance d2 from a line.
 * We solve a system of quadratic equations describing the intersection of a
 * line with a circle:
 *
 * x^2 + (y - d1)^2 = d2
 *                y = d2
 *
 * If there are 2 solutions, we pick whichever minimizes displacement from
 * the original position.
 *
 * TODO OFFSET LINE
 */
static int
PtFromPtLine(void *_Nonnull self, SK_Constraint *_Nonnull ct1, void *_Nonnull n1,
    SK_Constraint *_Nonnull ct2, void *_Nonnull n2)
{
	SK *sk = SKNODE(self)->sk;
	M_Vector3 p = SK_Pos(n1);
	SK_Line *L = n2;
	M_Vector3 p1 = SK_Pos(L->p1);
	M_Vector3 p2 = SK_Pos(L->p2);
	M_Real a, b, c, det;
	M_Vector3 s[2];
	int nSolns = 0;

	if (ct1->type != SK_DISTANCE || ct2->type != SK_DISTANCE) {
		AG_SetError("Expect Distance with %s and %s",
		    SKNODE(n1)->name, SKNODE(n2)->name);
		return (-1);
	}

	a = (p2.x - p1.x)*(p2.x - p1.x) +
	    (p2.y - p1.y)*(p2.y - p1.y);
	b = 2.0*( (p2.x - p1.x)*(p1.x - p.x) +
	          (p2.y - p1.y)*(p1.y - p.y) );
	c = p.x*p.x + p.y*p.y +
	    p1.x*p1.x + p1.y*p1.y -
	    2.0*(p.x*p1.x + p.y*p1.y) -
	    (ct1->ct_distance*ct1->ct_distance);
	det = b*b - 4.0*a*c;

	if (det < 0.0) {
		AG_SetError("%s,%s are too far apart to satisfy "
		            "constraint (det=%f)",
			    SKNODE(n1)->name, SKNODE(n2)->name, det);
		return (-1);
	} else if (det == 0.0) {
		/* TODO! */
		AG_SetError("(P,L)->P: Tangent");
		return (-1);
	} else {
		M_Real e = Sqrt(det);
		M_Real u1 = (-b + e) / (2.0*a);
		M_Real u2 = (-b - e) / (2.0*a);

		if ((u1 < 0.0 || u1 > 1.0) &&
		    (u2 < 0.0 || u2 > 1.0)) {
			if ((u1 < 0.0 && u2 < 0.0) ||
			    (u1 > 1.0 && u2 > 1.0)) {
				AG_SetError(
				    "%s,%s are too far apart to "
				    "satisfy constraint (%f/%f)",
				    SKNODE(n1)->name, SKNODE(n2)->name,
				    u1, u2);
				return (-1);
			} else {
				if (u1 >= 0.0 && u1 <= 1.0) {
					s[nSolns++] = M_VecLERP3p(&p1,&p2,u1);
				}
				if (u2 >= 0.0 && u2 <= 1.0) {
					s[nSolns++] = M_VecLERP3p(&p1,&p2,u2);
				}
			}
		} else {
			if (u1 >= 0.0 && u1 <= 1.0)
				s[nSolns++] = M_VecLERP3p(&p1,&p2,u1);
			if (u2 >= 0.0 && u2 <= 1.0)
				s[nSolns++] = M_VecLERP3p(&p1,&p2,u2);
		}
	}
	
	SK_Identity(self);
	if (nSolns == 2) {
		M_Vector3 pOrig = SK_Pos(self);
		if (M_VecDistance3p(&pOrig,&s[0]) <
		    M_VecDistance3p(&pOrig,&s[1])) {
			SK_Translatev(self, &s[0]);
		} else {
			SK_Translatev(self, &s[1]);
		}
	} else if (nSolns == 1) {
		SK_Translatev(self, &s[0]);
	} else {
		AG_SetError("(P,L)->P: No solutions");
		return (-1);
	}
	sk->nSolutions += nSolns;
	return (0);
}

/*
 * Compute the position of a point p from two known lines L1 and L2 at
 * angle a at the shared endpoint. This is a system of two linear equations
 * describing the intersection of two lines at distances d1 and d2 from L1
 * and L2:
 *
 * n2 = [-sin(a); cos(a)]
 * r2 = d1*cos(a) - d2
 *
 * Where L2 = (n2,r2).
 *
 * TODO OFFSET LINES
 */
static int
PtFromLineLine(void *_Nonnull self, SK_Constraint *_Nonnull ct1,
    void *_Nonnull n1, SK_Constraint *_Nonnull ct2, void *_Nonnull n2)
{
	SK *sk = SKNODE(self)->sk;
	SK_Line *L1 = n1;
	SK_Line *L2 = n2;
	M_Vector3 a1 = SK_Pos(L1->p1);
	M_Vector3 a2 = SK_Pos(L1->p2);
	M_Vector3 b1 = SK_Pos(L2->p1);
	M_Vector3 b2 = SK_Pos(L2->p2);
	M_Real uaNum, ubNum, uDiv;

	/* TODO */

	uaNum = (b2.x - b1.x)*(a1.y - b1.y) - (b2.y - b1.y)*(a1.x - b1.x);
	ubNum = (a2.x - a1.x)*(a1.y - b1.y) - (a2.y - a1.y)*(a1.x - b1.x);
	uDiv =  (b2.y - b1.y)*(a2.x - a1.x) - (b2.x - b1.x)*(a2.y - a1.y);

	if (uDiv != 0.0) {
		M_Real ua = uaNum/uDiv;
		M_Real ub = ubNum/uDiv;

		if (ua >= 0.0 && ua <= 1.0 &&
		    ub >= 0.0 && ub <= 1.0) {
			SK_TranslateVec(self,
			    M_VecAdd3(a1, M_VecScale3(M_VecSub3(a2,a1), ua)));
		} else {
			AG_SetError("L,L->p: No intersection");
			return (-1);
		}
	} else {
		/* TODO */
		if (uaNum == 0.0 || ubNum == 0.0) {
			AG_SetError("L,L->p: Lines are coincident");
			return (-1);
		} else {
			AG_SetError("L,L->p: Lines are parallel");
			return (-1);
		}
	}
	sk->nSolutions++;
	return (0);
}

/*
 * Compute the position of a Line based on Distances with respect to two
 * known Points. This case is underconstrained.
 */
static int
LineFromPtPt(void *_Nonnull self, SK_Constraint *_Nonnull ct1,
    void *_Nonnull n1, SK_Constraint *_Nonnull ct2, void *_Nonnull n2)
{
	/* TODO */
	AG_SetError("TODO");
	return (-1);
}

/*
 * Compute the position of a Line based on Distance from a known Point and
 * Angle with respect to a known Line.
 */
static int
LineFromPtLine(void *_Nonnull self, SK_Constraint *_Nonnull ctPoint,
    void *_Nonnull n1, SK_Constraint *_Nonnull ctLine, void *_Nonnull n2)
{
	if (ctPoint->type != SK_DISTANCE) {
		AG_SetError("Expecting Distance constraint with %s",
		    SKNODE(n1)->name);
		return (-1);
	}
	if (ctLine->type != SK_ANGLE) {
		AG_SetError("Expecting Angle constraint with %s",
		    SKNODE(n2)->name);
		return (-1);
	}
	/* TODO */
	AG_SetError("TODO");
	return (-1);
}

static int
LineFromLineLine(void *_Nonnull self, SK_Constraint *_Nonnull ct1,
    void *_Nonnull n1, SK_Constraint *_Nonnull ct2, void *_Nonnull n2)
{
	/* TODO */
	AG_SetError("TODO");
	return (-1);
}

const SK_ConstraintPairFn skConstraintPairFns[] = {
	{ SK_DISTANCE, 0, "Point:*", "Point:*",	PtFromPtAtDistance },
	{ SK_DISTANCE, 0, "Point:*", "Line:*",	PtFromLineAtDistance },
	{ SK_DISTANCE, 0, "Line:*",  "Line:*",	LineFromLineAtDistance },
	{ SK_ANGLE,    0, "Line:*",  "Line:*",	LineFromLineAtAngle },
};
const SK_ConstraintRingFn skConstraintRingFns[] = {
	{
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Point:*", "Point:*", "Point:*",
		PtFromPtPt
	}, {
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Point:*", "Point:*", "Line:*",
		PtFromPtLine
	}, {
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Point:*", "Line:*", "Line:*",
		PtFromLineLine
	}, {
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Line:*", "Point:*", "Point:*",
		LineFromPtPt
	}, {
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Line:*", "Point:*", "Line:*",
		LineFromPtLine
	}, {
		SK_CONSTRAINT_ANY, SK_CONSTRAINT_ANY,
		"Line:*", "Line:*", "Line:*",
		LineFromLineLine
	}
};

const int skConstraintPairFnCount = sizeof(skConstraintPairFns) /
                                    sizeof(skConstraintPairFns[0]);
const int skConstraintRingFnCount = sizeof(skConstraintRingFns) /
                                    sizeof(skConstraintRingFns[0]);
