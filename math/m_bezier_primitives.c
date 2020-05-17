/* Copyright (c) Charles A. Daniels <http://cdaniels.net>
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

#include <agar/math/m.h>
#include <agar/math/m_bezier.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>

#include <agar/math/m_bezier_primitives.h>

/* Draw a 2 endpoint 2 control point 2D bezier curve. This is a widget drawing
 * primitive, so it should only be called during Draw().
 *
 * Detail specifies the number of line segments to use.
 * */
void M_DrawBezier2(AG_Widget* widget,
		M_Real x1, M_Real y1,		/* start point */
		M_Real x2, M_Real y2,		/* end point */
		M_Real cx1, M_Real cy1,		/* control point 1 */
		M_Real cx2, M_Real cy2,		/* control point 2 */
		int detail, AG_Color* c
	)
{
	M_PointSet2 points, ctrl;
	M_Vector2 pt, prev, v;
	int k;

	pt.x = 0;
	pt.y = 0;
	prev.x = 0;
	prev.y = 0;

	M_PointSetInit2(&points);
	M_PointSetInit2(&ctrl);
	M_PointSetAlloc2(&points, 4);
	M_PointSetAlloc2(&ctrl, 4);

	v.x = x1;
	v.y = y1;
	M_PointSetAdd2(&points, v);
	v.x = x2;
	v.y = y2;
	M_PointSetAdd2(&points, v);
	v.x = cx1;
	v.y = cy1;
	M_PointSetAdd2(&points, v);
	v.x = cx2;
	v.y = cy2;
	M_PointSetAdd2(&points, v);

	M_BezierForm2(&points, &ctrl);

	for (k = 0 ; k < detail ; k++) {
		prev = pt;
		M_BezierCurve2(&ctrl, ((float) k) / ((float) detail), &pt);
		if (k != 0) {
			AG_DrawLine(widget, prev.x, prev.y, pt.x, pt.y, c);
		}
	}

	M_PointSetFree2(&points);
	M_PointSetFree2(&ctrl);

}

