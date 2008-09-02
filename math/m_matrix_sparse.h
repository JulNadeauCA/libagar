/*
 * Public domain.
 * Operations on m*n matrices (SPARSE version).
 */
typedef struct m_matrix_sp {
	struct m_matrix _inherit;
	char *d; /* data to be used by SPARSE */
} M_MatrixSP;

__BEGIN_DECLS
extern const M_MatrixOps mMatOps_SP;

M_Real *M_GetElement_SP(void *pM, Uint i, Uint j);
M_Real  M_GetValue_SP(void *pM, Uint i, Uint j);
M_Real  M_Get_SP(void *, Uint, Uint);
int     M_MatrixResize_SP(void *pA, Uint m, Uint n);
void    M_MatrixFree_SP(void *pA);
void   *M_MatrixNew_SP(Uint m, Uint n);
void    M_MatrixPrint_SP(void *pA);
void    M_MatrixSetZero_SP(void *pA);
int     M_FactorizeLU_SP(void *pA);
void    M_BacksubstLU_SP(void *pA, void *pV);
void    M_MNAPreorder_SP(void *pA);
void    M_AddToDiag_SP(void *pA, M_Real g);
void   *M_MatrixRead_SP(AG_DataSource *buf);
void    M_MatrixWrite_SP(AG_DataSource *buf, const void *pA);
void    M_MatrixToFloats_SP(float *fv, const void *pA);
void    M_MatrixToDoubles_SP(double *dv, const void *pA);
void    M_MatrixFromFloats_SP(void *pA, const float *fv);
void    M_MatrixFromDoubles_SP(void *pA, const double *fv);
__END_DECLS
