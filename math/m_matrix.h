/*	Public domain	*/

/*
 * Operations on m*n matrices (including sparse matrices)
 */
typedef struct m_matrix_ops {
	const char *_Nonnull name;

	M_Real *_Nonnull (*_Nonnull GetElement)(void *_Nonnull, Uint,Uint);
	M_Real           (*_Nonnull Get)(void *_Nonnull, Uint,Uint);
	int              (*_Nonnull Resize)(void *_Nonnull, Uint,Uint);
	void             (*_Nonnull FreeMatrix)(void *_Nonnull);
	void *_Nullable  (*_Nonnull NewMatrix)(Uint,Uint);
	void             (*_Nonnull SetIdentity)(void *_Nonnull);
	void             (*_Nonnull SetZero)(void *_Nonnull);
	void *_Nullable  (*_Nonnull Transpose)(const void *_Nonnull);
	int              (*_Nonnull Copy)(void *_Nonnull, const void *_Nonnull);
	void *_Nullable  (*_Nonnull Dup)(const void *_Nonnull);
	void *_Nonnull   (*_Nonnull Add)(const void *_Nonnull, const void *_Nonnull);
	int              (*_Nonnull Addv)(void *_Nonnull, const void *_Nonnull);
	void *_Nonnull   (*_Nonnull DirectSum)(const void *_Nonnull, const void *_Nonnull);
	void *_Nonnull   (*_Nonnull Mul)(const void *_Nonnull, const void *_Nonnull);
	int              (*_Nonnull Mulv)(const void *_Nonnull, const void *_Nonnull,
	                                  void *_Nonnull);
	void *_Nonnull   (*_Nonnull EntMul)(const void *_Nonnull, const void *_Nonnull);
	int              (*_Nonnull EntMulv)(const void *_Nonnull, const void *_Nonnull,
					     void *_Nonnull);
	int              (*_Nonnull Compare)(const void *_Nonnull, const void *_Nonnull,
					     M_Real *_Nonnull);
	int              (*_Nonnull Trace)(M_Real *_Nonnull, const void *_Nonnull);
	void *_Nonnull   (*_Nonnull Read)(AG_DataSource *_Nonnull);
	void             (*_Nonnull Write)(AG_DataSource *_Nonnull, const void *_Nonnull);
	void             (*_Nonnull ToFloats)(float *_Nonnull, const void *_Nonnull);
	void             (*_Nonnull ToDoubles)(double *_Nonnull, const void *_Nonnull);
	void             (*_Nonnull FromFloats)(void *_Nonnull, const float *_Nonnull);
	void             (*_Nonnull FromDoubles)(void *_Nonnull, const double *_Nonnull);
	void *_Nullable  (*_Nonnull GaussJordan)(const void *_Nonnull, void *_Nonnull);
	int              (*_Nonnull FactorizeLU)(void *_Nonnull);
	void             (*_Nonnull BacksubstLU)(void *_Nonnull, void *_Nonnull);
	void             (*_Nonnull MNAPreorder)(void *_Nonnull);
	void	         (*_Nonnull AddToDiag)(void *_Nonnull, M_Real);
} M_MatrixOps;

/*
 * Operations on 4x4 matrices.
 */
typedef struct m_matrix_ops44 {
	const char *_Nonnull name;
	
	M_Matrix44 (*_Nonnull Zero)(void);
	void       (*_Nonnull Zerov)(M_Matrix44 *_Nonnull);
	M_Matrix44 (*_Nonnull Identity)(void);
	void       (*_Nonnull Identityv)(M_Matrix44 *_Nonnull);
	M_Matrix44 (*_Nonnull Transpose)(M_Matrix44);
	M_Matrix44 (*_Nonnull Transposep)(const M_Matrix44 *_Nonnull);
	void	   (*_Nonnull Transposev)(M_Matrix44 *_Nonnull);

	M_Matrix44 (*_Nonnull Invert)(M_Matrix44);
	int        (*_Nonnull InvertElim)(M_Matrix44, M_Matrix44 *_Nonnull);
	
	M_Matrix44 (*_Nonnull Mult)(M_Matrix44, M_Matrix44);
	void       (*_Nonnull Multv)(M_Matrix44 *_Nonnull, const M_Matrix44 *_Nonnull);
	M_Vector4 (*_Nonnull MultVector)(M_Matrix44, M_Vector4);
	M_Vector4 (*_Nonnull MultVectorp)(const M_Matrix44 *_Nonnull, const M_Vector4 *_Nonnull);
	void      (*_Nonnull MultVectorv)(M_Vector4 *_Nonnull, const M_Matrix44 *_Nonnull);

	void      (*_Nonnull Copy)(M_Matrix44 *_Nonnull, const M_Matrix44 *_Nonnull);
	void      (*_Nonnull ToFloats)(float *_Nonnull, const M_Matrix44 *_Nonnull);
	void      (*_Nonnull ToDoubles)(double *_Nonnull, const M_Matrix44 *_Nonnull);
	void      (*_Nonnull FromFloats)(M_Matrix44 *_Nonnull, const float *_Nonnull);
	void      (*_Nonnull FromDoubles)(M_Matrix44 *_Nonnull, const double *_Nonnull);

	void      (*_Nonnull RotateAxis)(M_Matrix44 *_Nonnull, M_Real, M_Vector3);
	void      (*_Nonnull OrbitAxis)(M_Matrix44 *_Nonnull, M_Vector3, M_Vector3, M_Real);
	void      (*_Nonnull RotateEul)(M_Matrix44 *_Nonnull, M_Real,M_Real,M_Real);
	void      (*_Nonnull RotateI)(M_Matrix44 *_Nonnull, M_Real);
	void      (*_Nonnull RotateJ)(M_Matrix44 *_Nonnull, M_Real);
	void      (*_Nonnull RotateK)(M_Matrix44 *_Nonnull, M_Real);

	void      (*_Nonnull Translatev)(M_Matrix44 *_Nonnull, M_Vector3);
	void      (*_Nonnull Translate)(M_Matrix44 *_Nonnull, M_Real,M_Real,M_Real);
	void      (*_Nonnull TranslateX)(M_Matrix44 *_Nonnull, M_Real);
	void      (*_Nonnull TranslateY)(M_Matrix44 *_Nonnull, M_Real);
	void      (*_Nonnull TranslateZ)(M_Matrix44 *_Nonnull, M_Real);

	void      (*_Nonnull Scale)(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real, M_Real);
	void      (*_Nonnull UniScale)(M_Matrix44 *_Nonnull, M_Real);
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
extern const M_MatrixOps *_Nullable mMatOps;
extern const M_MatrixOps44 *_Nullable mMatOps44;
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
M_Matrix44 M_ReadMatrix44(AG_DataSource *_Nonnull);
void       M_ReadMatrix44v(AG_DataSource *_Nonnull, M_Matrix44 *_Nonnull);
void       M_WriteMatrix44(AG_DataSource *_Nonnull, const M_Matrix44 *_Nonnull);

/* Return 1 if the given matrix is square. */
static __inline__ int
M_IsSquare(const M_Matrix *_Nonnull M)
{
	return (M->m == M->n);
}
static __inline__ void
M_Set(M_Matrix *_Nonnull M, Uint i, Uint j, M_Real val)
{
	M_Real *v = M_GetElement(M, i,j);
	*v = val;
}
__END_DECLS
