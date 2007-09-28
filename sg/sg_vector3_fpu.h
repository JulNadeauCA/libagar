/*	Public domain	*/

__BEGIN_DECLS
extern const SG_VectorOps3 sgVecOps3_FPU;

SG_Vector	SG_VectorZero3_FPU(void);
SG_Vector	SG_VectorGet3_FPU(SG_Real, SG_Real, SG_Real);
void		SG_VectorSet3_FPU(SG_Vector *, SG_Real, SG_Real, SG_Real);
void		SG_VectorCopy3_FPU(SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorMirror3_FPU(SG_Vector, int, int, int);
SG_Vector	SG_VectorMirror3p_FPU(const SG_Vector *, int, int, int);
SG_Real		SG_VectorLen3_FPU(SG_Vector);
SG_Real		SG_VectorLen3p_FPU(const SG_Vector *);
SG_Real		SG_VectorDot3_FPU(SG_Vector, SG_Vector);
SG_Real		SG_VectorDot3p_FPU(const SG_Vector *, const SG_Vector *);
SG_Real		SG_VectorDistance3_FPU(SG_Vector, SG_Vector);
SG_Real		SG_VectorDistance3p_FPU(const SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorNorm3_FPU(SG_Vector);
SG_Vector	SG_VectorNorm3p_FPU(const SG_Vector *);
void		SG_VectorNorm3v_FPU(SG_Vector *);
SG_Vector	SG_VectorCross3_FPU(SG_Vector, SG_Vector);
SG_Vector	SG_VectorCross3p_FPU(const SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorNormCross3_FPU(SG_Vector, SG_Vector);
SG_Vector	SG_VectorNormCross3p_FPU(const SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorScale3_FPU(SG_Vector, SG_Real);
SG_Vector	SG_VectorScale3p_FPU(const SG_Vector *, SG_Real);
void		SG_VectorScale3v_FPU(SG_Vector *, SG_Real);
SG_Vector	SG_VectorAdd3_FPU(SG_Vector, SG_Vector);
SG_Vector	SG_VectorAdd3p_FPU(const SG_Vector *, const SG_Vector *);
void		SG_VectorAdd3v_FPU(SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorAdd3n_FPU(int, ...);
SG_Vector	SG_VectorSub3_FPU(SG_Vector, SG_Vector);
SG_Vector	SG_VectorSub3p_FPU(const SG_Vector *, const SG_Vector *);
void		SG_VectorSub3v_FPU(SG_Vector *, const SG_Vector *);
SG_Vector	SG_VectorSub3n_FPU(int, ...);
SG_Vector	SG_VectorAvg3_FPU(SG_Vector, SG_Vector);
SG_Vector	SG_VectorAvg3p_FPU(const SG_Vector *, const SG_Vector *);
void		SG_VectorVecAngle3_FPU(SG_Vector, SG_Vector, SG_Real *,
		                       SG_Real *);
SG_Vector	SG_VectorRotate3_FPU(SG_Vector, SG_Real, SG_Vector);
void		SG_VectorRotate3v_FPU(SG_Vector *, SG_Real, SG_Vector);
SG_Vector	SG_VectorRotateI3_FPU(SG_Vector, SG_Real);
SG_Vector	SG_VectorRotateJ3_FPU(SG_Vector, SG_Real);
SG_Vector	SG_VectorRotateK3_FPU(SG_Vector, SG_Real);
SG_Vector	SG_VectorRotateQuat3_FPU(SG_Vector, SG_Quat);
SG_Vector	SG_VectorLERP3_FPU(SG_Vector, SG_Vector, SG_Real);
SG_Vector	SG_VectorLERP3p_FPU(SG_Vector *, SG_Vector *, SG_Real);
SG_Vector	SG_VectorElemPow3_FPU(SG_Vector, SG_Real);
__END_DECLS
