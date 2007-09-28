/*	Public domain	*/

__BEGIN_DECLS
extern const SG_VectorOps2 sgVecOps2_FPU;

SG_Vector2	SG_VectorZero2_FPU(void);
SG_Vector2	SG_VectorGet2_FPU(SG_Real, SG_Real);
void		SG_VectorSet2_FPU(SG_Vector2 *, SG_Real, SG_Real);
void		SG_VectorCopy2_FPU(SG_Vector2 *, const SG_Vector2 *);
SG_Vector2	SG_VectorMirror2_FPU(SG_Vector2, int, int);
SG_Vector2	SG_VectorMirror2p_FPU(const SG_Vector2 *, int, int);
SG_Real		SG_VectorLen2_FPU(SG_Vector2);
SG_Real		SG_VectorLen2p_FPU(const SG_Vector2 *);
SG_Real		SG_VectorDot2_FPU(SG_Vector2, SG_Vector2);
SG_Real		SG_VectorDot2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
SG_Real		SG_VectorPerpDot2_FPU(SG_Vector2, SG_Vector2);
SG_Real		SG_VectorPerpDot2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
SG_Real		SG_VectorDistance2_FPU(SG_Vector2, SG_Vector2);
SG_Real		SG_VectorDistance2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
SG_Vector2	SG_VectorNorm2_FPU(SG_Vector2);
SG_Vector2	SG_VectorNorm2p_FPU(const SG_Vector2 *);
void		SG_VectorNorm2v_FPU(SG_Vector2 *);
SG_Vector2	SG_VectorScale2_FPU(SG_Vector2, SG_Real);
SG_Vector2	SG_VectorScale2p_FPU(const SG_Vector2 *, SG_Real);
void		SG_VectorScale2v_FPU(SG_Vector2 *, SG_Real);
SG_Vector2	SG_VectorAdd2_FPU(SG_Vector2, SG_Vector2);
SG_Vector2	SG_VectorAdd2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
void		SG_VectorAdd2v_FPU(SG_Vector2 *, const SG_Vector2 *);
SG_Vector2	SG_VectorAdd2n_FPU(int, ...);
SG_Vector2	SG_VectorSub2_FPU(SG_Vector2, SG_Vector2);
SG_Vector2	SG_VectorSub2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
void		SG_VectorSub2v_FPU(SG_Vector2 *, const SG_Vector2 *);
SG_Vector2	SG_VectorSub2n_FPU(int, ...);
SG_Vector2	SG_VectorAvg2_FPU(SG_Vector2, SG_Vector2);
SG_Vector2	SG_VectorAvg2p_FPU(const SG_Vector2 *, const SG_Vector2 *);
SG_Vector2	SG_VectorLERP2_FPU(SG_Vector2, SG_Vector2, SG_Real);
SG_Vector2	SG_VectorLERP2p_FPU(SG_Vector2 *, SG_Vector2 *, SG_Real);
SG_Vector2	SG_VectorElemPow2_FPU(SG_Vector2, SG_Real);
SG_Real		SG_VectorVecAngle2_FPU(SG_Vector2, SG_Vector2);
SG_Vector2	SG_VectorRotate2_FPU(SG_Vector2, SG_Real);
void		SG_VectorRotate2v_FPU(SG_Vector2 *, SG_Real);
__END_DECLS
