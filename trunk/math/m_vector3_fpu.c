/*	Public domain	*/
/*
 * Operations on vectors in R^3 using scalar instructions.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

const M_VectorOps3 mVecOps3_FPU = {
	"scalar",
	M_VectorZero3_FPU,
	M_VectorGet3_FPU,
	M_VectorSet3_FPU,
	M_VectorCopy3_FPU,
	M_VectorFlip3_FPU,
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
	M_VectorVecAngle3_FPU
};
