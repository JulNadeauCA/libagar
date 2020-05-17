/*
 * Public domain.
 * Operations on vectors in R^n using standard FPU instructions.
 */
__BEGIN_DECLS
extern const M_VectorOps mVecOps_FPU;

M_Vector *_Nonnull M_ReadVector_FPU(AG_DataSource *_Nonnull);
void               M_WriteVector_FPU(AG_DataSource *_Nonnull,
                                     const M_Vector *_Nonnull);
M_Vector *_Nonnull M_VectorFromReals_FPU(Uint, const M_Real *_Nonnull);
M_Vector *_Nonnull M_VectorFromFloats_FPU(Uint, const float *_Nonnull);
M_Vector *_Nonnull M_VectorFromDoubles_FPU(Uint, const double *_Nonnull);
__END_DECLS
