/*
 * Public domain.
 * Operations on m*n matrices (SPARSE version).
 */
typedef struct m_matrix_sp {
	struct m_matrix _inherit;	/* M_Matrix(3) -> M_MatrixSP */
	char *_Nonnull d;		/* Data to be used by SPARSE */
} M_MatrixSP;

__BEGIN_DECLS
extern const M_MatrixOps mMatOps_SP;

M_Real *_Nullable M_GetElement_SP(void *_Nonnull, Uint,Uint);

M_Real  M_GetValue_SP(void *_Nonnull, Uint,Uint);
M_Real  M_Get_SP(void *_Nonnull, Uint,Uint);
int     M_MatrixResize_SP(void *_Nonnull, Uint,Uint);
void    M_MatrixFree_SP(void *_Nonnull);

void *_Nonnull M_MatrixNew_SP(Uint,Uint);

void    M_MatrixSetZero_SP(void *_Nonnull);
int     M_FactorizeLU_SP(void *_Nonnull);
void    M_BacksubstLU_SP(void *_Nonnull, void *_Nonnull);
void    M_MNAPreorder_SP(void *_Nonnull);
void    M_AddToDiag_SP(void *_Nonnull, M_Real);

void *_Nonnull M_MatrixRead_SP(AG_DataSource *_Nonnull);
void           M_MatrixWrite_SP(AG_DataSource *_Nonnull, const void *_Nonnull);

void    M_MatrixToFloats_SP(float *_Nonnull, const void *_Nonnull);
void    M_MatrixToDoubles_SP(double *_Nonnull, const void *_Nonnull);
void    M_MatrixFromFloats_SP(void *_Nonnull, const float *_Nonnull);
void    M_MatrixFromDoubles_SP(void *_Nonnull, const double *_Nonnull);
__END_DECLS
