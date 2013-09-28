/*	Public domain	*/

/*
 * Operations on m*n matrices (including sparse matrices)
 */
typedef struct m_matrix_ops {
	const char *name;

	M_Real *(*GetElement)(void *M, Uint i, Uint j);
	M_Real  (*Get)(void *M, Uint i, Uint j);
	int     (*Resize)(void *M, Uint m, Uint n);
	void    (*FreeMatrix)(void *M);
	void   *(*NewMatrix)(Uint m, Uint n);
	
	void    (*SetIdentity)(void *M);
	void    (*SetZero)(void *M);
	void   *(*Transpose)(const void *M);
	int     (*Copy)(void *dst, const void *src);
	void   *(*Dup)(const void *A);
	void   *(*Add)(const void *A, const void *B);
	int     (*Addv)(void *A, const void *B);
	void   *(*DirectSum)(const void *A, const void *B);
	void   *(*Mul)(const void *A, const void *B);
	int     (*Mulv)(const void *A, const void *B, void *AB);
	void   *(*EntMul)(const void *A, const void *B);
	int     (*EntMulv)(const void *A, const void *B, void *AB);
	int     (*Compare)(const void *A, const void *B, M_Real *d);
	int     (*Trace)(M_Real *t, const void *A);

	void   *(*Read)(AG_DataSource *ds);
	void    (*Write)(AG_DataSource *ds, const void *M);
	void    (*ToFloats)(float *fv, const void *M);
	void    (*ToDoubles)(double *dv, const void *M);
	void    (*FromFloats)(void *M, const float *fv);
	void    (*FromDoubles)(void *M, const double *dv);

	void   *(*GaussJordan)(const void *A, void *b);
	int     (*FactorizeLU)(void *A);
	void    (*BacksubstLU)(void *A, void *b);
	void    (*MNAPreorder)(void *A);
	void	(*AddToDiag)(void *A, M_Real g);
} M_MatrixOps;

/*
 * Operations on 4x4 matrices.
 */
typedef struct m_matrix_ops44 {
	const char *name;
	
	M_Matrix44 (*Zero)(void);
	void       (*Zerov)(M_Matrix44 *A);
	M_Matrix44 (*Identity)(void);
	void       (*Identityv)(M_Matrix44 *A);
	M_Matrix44 (*Transpose)(M_Matrix44 A);
	M_Matrix44 (*Transposep)(const M_Matrix44 *A);
	void	   (*Transposev)(M_Matrix44 *A);

	M_Matrix44 (*Invert)(M_Matrix44 A);
	int        (*InvertElim)(M_Matrix44 A, M_Matrix44 *Ainv);
	
	M_Matrix44 (*Mult)(M_Matrix44 A, M_Matrix44 B);
	void       (*Multv)(M_Matrix44 *A, const M_Matrix44 *B);
	M_Vector4 (*MultVector)(M_Matrix44 A, M_Vector4 x);
	M_Vector4 (*MultVectorp)(const M_Matrix44 *A, const M_Vector4 *x);
	void      (*MultVectorv)(M_Vector4 *x, const M_Matrix44 *A);

	void      (*Copy)(M_Matrix44 *dst, const M_Matrix44 *src);
	void      (*ToFloats)(float *flts, const M_Matrix44 *A);
	void      (*ToDoubles)(double *dbls, const M_Matrix44 *A);
	void      (*FromFloats)(M_Matrix44 *A, const float *flts);
	void      (*FromDoubles)(M_Matrix44 *A, const double *dbls);

	void      (*RotateAxis)(M_Matrix44 *A, M_Real theta, M_Vector3 axis);
	void      (*OrbitAxis)(M_Matrix44 *A, M_Vector3 center, M_Vector3 axis,
	                       M_Real theta);
	void      (*RotateEul)(M_Matrix44 *A, M_Real pitch, M_Real roll,
	                       M_Real yaw);
	void      (*RotateI)(M_Matrix44 *A, M_Real theta);
	void      (*RotateJ)(M_Matrix44 *A, M_Real theta);
	void      (*RotateK)(M_Matrix44 *A, M_Real theta);

	void      (*Translatev)(M_Matrix44 *A, M_Vector3 v);
	void      (*Translate)(M_Matrix44 *A, M_Real x, M_Real y, M_Real z);
	void      (*TranslateX)(M_Matrix44 *A, M_Real c);
	void      (*TranslateY)(M_Matrix44 *A, M_Real c);
	void      (*TranslateZ)(M_Matrix44 *A, M_Real c);

	void      (*Scale)(M_Matrix44 *A, M_Real x, M_Real y, M_Real z,
	                   M_Real w);
	void      (*UniScale)(M_Matrix44 *A, M_Real c);
} M_MatrixOps44;

/* Debug macros */
#ifdef AG_DEBUG
# define M_ENTRY_EXISTS(A,i,j) \
	((i) >= 0 && (i) < (A)->m && (j) >= 0 && (j) < (A)->n)
# define M_ASSERT_COMPAT_MATRICES(A, B, ret) \
	do { \
		if (MROWS(A) != MROWS(B) || \
		    MCOLS(A) != MCOLS(B)) { \
			AG_SetError("Incompatible matrices"); \
			return (ret); \
		} \
	} while (0)
# define M_ASSERT_MULTIPLIABLE_MATRICES(A, B, ret) \
	do { \
		if (MROWS(A) != MROWS(B) || \
		    MCOLS(A) != MCOLS(B)) { \
			AG_SetError("Incompatible matrices"); \
			return (ret); \
		} \
	} while (0)
# define M_ASSERT_SQUARE_MATRIX(A, ret) \
	do { \
		if (MROWS(A) != MCOLS(A)) { \
			AG_SetError("Incompatible matrices"); \
			return (ret); \
		} \
	} while (0)
#else
# define M_ENTRY_EXISTS(A,i,j) 1
# define M_ASSERT_COMPAT_MATRICES(A, B, ret)
# define M_ASSERT_MULTIPLIABLE_MATRICES(A, B, ret)
# define M_ASSERT_SQUARE_MATRIX(A, ret)
#endif

/* Backends */
__BEGIN_DECLS
extern const M_MatrixOps *mMatOps;
extern const M_MatrixOps44 *mMatOps44;
__END_DECLS

#include <agar/math/m_matrix_fpu.h>
#include <agar/math/m_matrix44_fpu.h>
#include <agar/math/m_matrix44_sse.h>
#include <agar/math/m_matrix_sparse.h>

/* Operations on m*n matrices. */
#define M_New			mMatOps->NewMatrix
#define M_Free			mMatOps->FreeMatrix
#define M_Resize		mMatOps->Resize
#define M_Get                   mMatOps->Get
#define M_GetElement            mMatOps->GetElement
#define M_SetIdentity		mMatOps->SetIdentity
#define M_SetZero		mMatOps->SetZero
#define M_Transpose		mMatOps->Transpose
#define M_Copy			mMatOps->Copy
#define M_Dup			mMatOps->Dup
#define M_Add			mMatOps->Add
#define M_Addv			mMatOps->Addv
#define M_DirectSum		mMatOps->DirectSum
#define M_Mul			mMatOps->Mul
#define M_Mulv			mMatOps->Mulv
#define M_EntMul		mMatOps->EntMul
#define M_EntMulv		mMatOps->EntMulv
#define M_Compare		mMatOps->Compare
#define M_Trace			mMatOps->Trace
#define M_ReadMatrix		mMatOps->Read
#define M_WriteMatrix		mMatOps->Write
#define M_ToFloats		mMatOps->ToFloats
#define M_ToDoubles		mMatOps->ToDoubles
#define M_FromFloats		mMatOps->FromFloats
#define M_FromDoubles		mMatOps->FromDoubles
#define M_GaussJordan		mMatOps->GaussJordan
#define M_FactorizeLU		mMatOps->FactorizeLU
#define M_BacksubstLU		mMatOps->BacksubstLU
#define M_MNAPreorder           mMatOps->MNAPreorder
#define M_AddToDiag		mMatOps->AddToDiag

/* Operations on 4x4 matrices. */
#define M_MatZero44		mMatOps44->Zero
#define M_MatZero44v		mMatOps44->Zerov
#define M_MatIdentity44		mMatOps44->Identity
#define M_MatIdentity44v	mMatOps44->Identityv
#define M_MatTranspose44	mMatOps44->Transpose
#define M_MatTranspose44p	mMatOps44->Transposep
#define M_MatTranspose44v	mMatOps44->Transposev
#define M_MatInvertElim44	mMatOps44->InvertElim
#define M_MatMultVector44	mMatOps44->MultVector
#define M_MatMultVector44p	mMatOps44->MultVectorp
#define M_MatMultVector44v	mMatOps44->MultVectorv
#define M_MatCopy44		mMatOps44->Copy
#define M_MatToFloats44		mMatOps44->ToFloats
#define M_MatToDoubles44	mMatOps44->ToDoubles
#define M_MatFromFloats44	mMatOps44->FromFloats
#define M_MatFromDoubles44	mMatOps44->FromDoubles
#define M_MatRotateAxis44	mMatOps44->RotateAxis
#define M_MatOrbitAxis44	mMatOps44->OrbitAxis
#define M_MatRotateEul44	mMatOps44->RotateEul
#define M_MatRotate44I		mMatOps44->RotateI
#define M_MatRotate44J		mMatOps44->RotateJ
#define M_MatRotate44K		mMatOps44->RotateK
#define M_MatTranslate44v	mMatOps44->Translatev
#define M_MatTranslate44	mMatOps44->Translate
#define M_MatTranslate44X	mMatOps44->TranslateX
#define M_MatTranslate44Y	mMatOps44->TranslateY
#define M_MatTranslate44Z	mMatOps44->TranslateZ
#define M_MatScale44		mMatOps44->Scale
#define M_MatUniScale44		mMatOps44->UniScale
#if defined(INLINE_SSE)
# define M_MatInvert44		M_MatrixInvert44_SSE
# define M_MatMult44		M_MatrixMult44_SSE
# define M_MatMult44v		M_MatrixMult44v_SSE
#else  /* !INLINE_SSE */
# define M_MatInvert44		mMatOps44->Invert
# define M_MatMult44		mMatOps44->Mult
# define M_MatMult44v		mMatOps44->Multv
#endif /* INLINE_SSE */

__BEGIN_DECLS
void       M_MatrixInitEngine(void);
M_Matrix44 M_ReadMatrix44(AG_DataSource *);
void       M_ReadMatrix44v(AG_DataSource *, M_Matrix44 *);
void       M_WriteMatrix44(AG_DataSource *, const M_Matrix44 *);

/* Return 1 if the given matrix is square. */
static __inline__ int
M_IsSquare(const M_Matrix *M)
{
	return (M->m == M->n);
}
static __inline__ void
M_Set(M_Matrix *M, Uint i, Uint j, M_Real val)
{
	M_Real *v = M_GetElement(M, i,j);
	*v = val;
}
__END_DECLS
