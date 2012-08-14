/*
 * Public domain.
 * Operations on vectors in R^3 using standard FPU instructions.
 */

#include <core/core.h>
#include "m.h"

const M_VectorOps3 mVecOps3_FPU = {
	"scalar",
	M_VectorZero3_FPU,
	M_VectorGet3_FPU,
	M_VectorSet3_FPU,
	M_VectorCopy3_FPU,
	M_VectorMirror3_FPU,
	M_VectorMirror3p_FPU,
	M_VectorLen3_FPU,
	M_VectorLen3p_FPU,
	M_VectorDot3_FPU,
	M_VectorDot3p_FPU,
	M_VectorDistance3_FPU,
	M_VectorDistance3p_FPU,
	M_VectorNorm3_FPU,
	M_VectorNorm3p_FPU,
	M_VectorNorm3v_FPU,
	M_VectorCross3_FPU,
	M_VectorCross3p_FPU,
	M_VectorNormCross3_FPU,
	M_VectorNormCross3p_FPU,
	M_VectorScale3_FPU,
	M_VectorScale3p_FPU,
	M_VectorScale3v_FPU,
	M_VectorAdd3_FPU,
	M_VectorAdd3p_FPU,
	M_VectorAdd3v_FPU,
	M_VectorSum3_FPU,
	M_VectorSub3_FPU,
	M_VectorSub3p_FPU,
	M_VectorSub3v_FPU,
	M_VectorAvg3_FPU,
	M_VectorAvg3p_FPU,
	M_VectorLERP3_FPU,
	M_VectorLERP3p_FPU,
	M_VectorElemPow3_FPU,
	M_VectorVecAngle3_FPU,
	M_VectorRotate3_FPU,
	M_VectorRotate3v_FPU,
	M_VectorRotateQuat3_FPU,
	M_VectorRotateI3_FPU,
	M_VectorRotateJ3_FPU,
	M_VectorRotateK3_FPU,
};

M_Vector3
M_VectorSum3_FPU(const M_Vector3 *va, Uint count)
{
	M_Vector3 v;
	Uint i;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	for (i = 0; i < count; i++) {
		v.x += va[i].x;
		v.y += va[i].y;
		v.z += va[i].z;
	}
	return (v);
}

M_Vector3
M_VectorRotate3_FPU(M_Vector3 v, M_Real theta, M_Vector3 n)
{
	M_Vector3 r = v;

	M_VectorRotate3v_FPU(&r, theta, n);
	return (r);
}

void
M_VectorRotate3v_FPU(M_Vector3 *v, M_Real theta, M_Vector3 n)
{
	M_Real s = Sin(theta);
	M_Real c = Cos(theta);
	M_Real t = 1.0 - c;
	M_Matrix44 R;

	R.m[0][0] = t*n.x*n.x + c;
	R.m[0][1] = t*n.x*n.y + s*n.z;
	R.m[0][2] = t*n.x*n.z - s*n.y;
	R.m[0][3] = 0.0;
	R.m[1][0] = t*n.x*n.y - s*n.z;
	R.m[1][1] = t*n.y*n.y + c;
	R.m[1][2] = t*n.y*n.z + s*n.x;
	R.m[1][3] = 0.0;
	R.m[2][0] = t*n.x*n.z + s*n.y;
	R.m[2][1] = t*n.y*n.z - s*n.x;
	R.m[2][2] = t*n.z*n.z + c;
	R.m[2][3] = 0.0;
	R.m[3][0] = 0.0;
	R.m[3][1] = 0.0;
	R.m[3][2] = 0.0;
	R.m[3][3] = 1.0;
	M_MatMult44Vector3v(v, &R);
}

M_Vector3
M_VectorRotateQuat3_FPU(M_Vector3 V, M_Quaternion Q)
{
	M_Matrix44 R;

	M_QuaternionToMatrix44(&R, &Q);
	return M_MatMult44Vector3p(&R, &V);
}
