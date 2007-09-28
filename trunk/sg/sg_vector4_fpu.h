/*	Public domain	*/

__BEGIN_DECLS
extern const SG_VectorOps4 sgVecOps4_FPU;

SG_Vector4	SG_VectorZero4_FPU(void);
SG_Vector4	SG_VectorGet4_FPU(SG_Real, SG_Real, SG_Real, SG_Real);
void		SG_VectorSet4_FPU(SG_Vector4 *, SG_Real, SG_Real, SG_Real,
		                  SG_Real);
void		SG_VectorCopy4_FPU(SG_Vector4 *, const SG_Vector4 *);
SG_Vector4	SG_VectorMirror4_FPU(SG_Vector4, int, int, int, int);
SG_Vector4	SG_VectorMirror4p_FPU(const SG_Vector4 *, int, int, int, int);
SG_Real		SG_VectorLen4_FPU(SG_Vector4);
SG_Real		SG_VectorLen4p_FPU(const SG_Vector4 *);
SG_Real		SG_VectorDot4_FPU(SG_Vector4, SG_Vector4);
SG_Real		SG_VectorDot4p_FPU(const SG_Vector4 *, const SG_Vector4 *);
SG_Real		SG_VectorDistance4_FPU(SG_Vector4, SG_Vector4);
SG_Real		SG_VectorDistance4p_FPU(const SG_Vector4 *, const SG_Vector4 *);
SG_Vector4	SG_VectorNorm4_FPU(SG_Vector4);

SG_Vector4	SG_VectorNorm4p_FPU(const SG_Vector4 *);
void		SG_VectorNorm4v_FPU(SG_Vector4 *);
SG_Vector4	SG_VectorCross4_FPU(SG_Vector4, SG_Vector4, SG_Vector4);
SG_Vector4	SG_VectorCross4p_FPU(const SG_Vector4 *, const SG_Vector4 *,
		                     const SG_Vector4 *);
SG_Vector4	SG_VectorNormCross4_FPU(SG_Vector4, SG_Vector4, SG_Vector4);
SG_Vector4	SG_VectorNormCross4p_FPU(const SG_Vector4 *, const SG_Vector4 *,
		                         const SG_Vector4 *);
SG_Vector4	SG_VectorScale4_FPU(SG_Vector4, SG_Real);
SG_Vector4	SG_VectorScale4p_FPU(const SG_Vector4 *, SG_Real);
void		SG_VectorScale4v_FPU(SG_Vector4 *, SG_Real);
SG_Vector4	SG_VectorAdd4_FPU(SG_Vector4, SG_Vector4);
SG_Vector4	SG_VectorAdd4p_FPU(const SG_Vector4 *, const SG_Vector4 *);
void		SG_VectorAdd4v_FPU(SG_Vector4 *, const SG_Vector4 *);
SG_Vector4	SG_VectorAdd4n_FPU(int, ...);
SG_Vector4	SG_VectorSub4_FPU(SG_Vector4, SG_Vector4);
SG_Vector4	SG_VectorSub4p_FPU(const SG_Vector4 *, const SG_Vector4 *);
void		SG_VectorSub4v_FPU(SG_Vector4 *, const SG_Vector4 *);
SG_Vector4	SG_VectorSub4n_FPU(int, ...);
SG_Vector4	SG_VectorAvg4_FPU(SG_Vector4, SG_Vector4);
SG_Vector4	SG_VectorAvg4p_FPU(const SG_Vector4 *, const SG_Vector4 *);
void		SG_VectorVecAngle4_FPU(SG_Vector4, SG_Vector4, SG_Real *,
		                       SG_Real *, SG_Real *);
SG_Vector4	SG_VectorRotate4_FPU(SG_Vector4, SG_Real, SG_Vector4);
void		SG_VectorRotate4v_FPU(SG_Vector4 *, SG_Real, SG_Vector4);
SG_Vector4	SG_VectorLERP4_FPU(SG_Vector4, SG_Vector4, SG_Real);
SG_Vector4	SG_VectorLERP4p_FPU(SG_Vector4 *, SG_Vector4 *, SG_Real);
SG_Vector4	SG_VectorElemPow4_FPU(SG_Vector4, SG_Real);
__END_DECLS
