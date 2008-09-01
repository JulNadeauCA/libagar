/*	Public domain	*/

__BEGIN_DECLS
extern const M_VectorOps2 mVecOps2_FPU;

M_Vector2	M_VectorZero2_FPU(void);
M_Vector2	M_VectorGet2_FPU(M_Real, M_Real);
void		M_VectorSet2_FPU(M_Vector2 *, M_Real, M_Real);
void		M_VectorCopy2_FPU(M_Vector2 *, const M_Vector2 *);
M_Vector2	M_VectorMirror2_FPU(M_Vector2, int, int);
M_Vector2	M_VectorMirror2p_FPU(const M_Vector2 *, int, int);
M_Real		M_VectorLen2_FPU(M_Vector2);
M_Real		M_VectorLen2p_FPU(const M_Vector2 *);
M_Real		M_VectorDot2_FPU(M_Vector2, M_Vector2);
M_Real		M_VectorDot2p_FPU(const M_Vector2 *, const M_Vector2 *);
M_Real		M_VectorPerpDot2_FPU(M_Vector2, M_Vector2);
M_Real		M_VectorPerpDot2p_FPU(const M_Vector2 *, const M_Vector2 *);
M_Real		M_VectorDistance2_FPU(M_Vector2, M_Vector2);
M_Real		M_VectorDistance2p_FPU(const M_Vector2 *, const M_Vector2 *);
M_Vector2	M_VectorNorm2_FPU(M_Vector2);
M_Vector2	M_VectorNorm2p_FPU(const M_Vector2 *);
void		M_VectorNorm2v_FPU(M_Vector2 *);
M_Vector2	M_VectorScale2_FPU(M_Vector2, M_Real);
M_Vector2	M_VectorScale2p_FPU(const M_Vector2 *, M_Real);
void		M_VectorScale2v_FPU(M_Vector2 *, M_Real);
M_Vector2	M_VectorAdd2_FPU(M_Vector2, M_Vector2);
M_Vector2	M_VectorAdd2p_FPU(const M_Vector2 *, const M_Vector2 *);
void		M_VectorAdd2v_FPU(M_Vector2 *, const M_Vector2 *);
M_Vector2	M_VectorAdd2n_FPU(int, ...);
M_Vector2	M_VectorSub2_FPU(M_Vector2, M_Vector2);
M_Vector2	M_VectorSub2p_FPU(const M_Vector2 *, const M_Vector2 *);
void		M_VectorSub2v_FPU(M_Vector2 *, const M_Vector2 *);
M_Vector2	M_VectorSub2n_FPU(int, ...);
M_Vector2	M_VectorAvg2_FPU(M_Vector2, M_Vector2);
M_Vector2	M_VectorAvg2p_FPU(const M_Vector2 *, const M_Vector2 *);
M_Vector2	M_VectorLERP2_FPU(M_Vector2, M_Vector2, M_Real);
M_Vector2	M_VectorLERP2p_FPU(M_Vector2 *, M_Vector2 *, M_Real);
M_Vector2	M_VectorElemPow2_FPU(M_Vector2, M_Real);
M_Real		M_VectorVecAngle2_FPU(M_Vector2, M_Vector2);
M_Vector2	M_VectorRotate2_FPU(M_Vector2, M_Real);
void		M_VectorRotate2v_FPU(M_Vector2 *, M_Real);
__END_DECLS
