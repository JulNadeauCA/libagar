/*	Public domain	*/

#define SG_MATRIX_TINYVAL 1e-12		/* For singularity check */

/* Operations on 4x4 matrices. */
typedef struct sg_matrix_ops44 {
	const char *name;
	
	SG_Matrix (*Zero)(void);
	void      (*Zerov)(SG_Matrix *);
	SG_Matrix (*Identity)(void);
	void      (*Identityv)(SG_Matrix *);
	SG_Matrix (*Transpose)(SG_Matrix);
	SG_Matrix (*Transposep)(const SG_Matrix *);
	void	  (*Transposev)(SG_Matrix *);

	SG_Matrix (*Invert)(SG_Matrix);
	SG_Matrix (*Invertp)(const SG_Matrix *);
	int       (*InvertGaussJordanv)(const SG_Matrix *, SG_Matrix *);
	
	SG_Matrix (*Mult)(SG_Matrix, SG_Matrix);
	void      (*Multv)(SG_Matrix *, const SG_Matrix *);
	void      (*Multpv)(SG_Matrix *, const SG_Matrix *, const SG_Matrix *);
	SG_Vector (*MultVector)(SG_Matrix, SG_Vector);
	SG_Vector (*MultVectorp)(const SG_Matrix *, const SG_Vector *);
	void      (*MultVectorv)(SG_Vector *, const SG_Matrix *);
	SG_Vector4 (*MultVector4)(SG_Matrix, SG_Vector4);
	SG_Vector4 (*MultVector4p)(const SG_Matrix *, const SG_Vector4 *);
	void       (*MultVector4v)(SG_Vector4 *, const SG_Matrix *);

	void      (*Copy)(SG_Matrix *, const SG_Matrix *);
	void      (*ToFloats)(float *, const SG_Matrix *);
	void      (*ToDoubles)(double *, const SG_Matrix *);
	void      (*FromFloats)(SG_Matrix *, const float *);
	void      (*FromDoubles)(SG_Matrix *, const double *);
	void      (*GetDirection)(const SG_Matrix *, SG_Vector *, SG_Vector *,
	                          SG_Vector *);
	void	  (*DiagonalSwapv)(SG_Matrix *);

	void      (*RotateAxis)(SG_Matrix *, SG_Real, SG_Vector);
	void      (*OrbitAxis)(SG_Matrix *, SG_Vector, SG_Vector, SG_Real);
	void      (*RotateEul)(SG_Matrix *, SG_Real, SG_Real, SG_Real);
	void      (*RotateI)(SG_Matrix *, SG_Real);
	void      (*RotateJ)(SG_Matrix *, SG_Real);
	void      (*RotateK)(SG_Matrix *, SG_Real);

	void      (*Translate)(SG_Matrix *, SG_Vector);
	void      (*Translate3)(SG_Matrix *, SG_Real, SG_Real, SG_Real);
	void      (*TranslateX)(SG_Matrix *, SG_Real);
	void      (*TranslateY)(SG_Matrix *, SG_Real);
	void      (*TranslateZ)(SG_Matrix *, SG_Real);

	void      (*Scale)(SG_Matrix *, SG_Real, SG_Real, SG_Real, SG_Real);
	void      (*UniScale)(SG_Matrix *, SG_Real);
} SG_MatrixOps44;

__BEGIN_DECLS
extern const SG_MatrixOps44 *sgMatOps44;
__END_DECLS

#if defined(_AGAR_INTERNAL)
#include <sg/sg_matrix44_fpu.h>
#include <sg/sg_matrix44_sse.h>
#else
#include <agar/sg/sg_matrix44_fpu.h>
#include <agar/sg/sg_matrix44_sse.h>
#endif

__BEGIN_DECLS
void SG_MatrixInitEngine(void);

void SG_MatrixPrint(const SG_Matrix *);
void SG_MatrixGetTranslation(const SG_Matrix *, SG_Vector *);
void SG_MatrixGetRotationXYZ(const SG_Matrix *, SG_Real *, SG_Real *,
                             SG_Real *);

SG_Matrix SG_ReadMatrix(AG_DataSource *);
void      SG_ReadMatrixv(AG_DataSource *, SG_Matrix *);
void      SG_WriteMatrix(AG_DataSource *, SG_Matrix *);

void	  SG_LoadMatrixGL(const SG_Matrix *);
void	  SG_GetMatrixGL(int, SG_Matrix *);
__END_DECLS

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)
#define Mat44 sgMatOps44

#define MatZero			Mat44->Zero
#define MatZerov		Mat44->Zerov
#define MatIdentity		Mat44->Identity
#define MatIdentityv		Mat44->Identityv
#define MatTranspose		Mat44->Transpose
#define MatTransposep		Mat44->Transposep
#define MatTransposev		Mat44->Transposev
#define MatInvertGaussJordanv	Mat44->InvertGaussJordanv
#define MatMultVector		Mat44->MultVector
#define MatMultVectorp		Mat44->MultVectorp
#define MatMultVectorv		Mat44->MultVectorv
#define MatMultVector4		Mat44->MultVector4
#define MatMultVector4p		Mat44->MultVector4p
#define MatMultVector4v		Mat44->MultVector4v
#define MatCopy			Mat44->Copy
#define MatToFloats		Mat44->ToFloats
#define MatToDoubles		Mat44->ToDoubles
#define MatFromFloats		Mat44->FromFloats
#define MatFromDoubles		Mat44->FromDoubles
#define MatGetDirection		Mat44->GetDirection
#define MatDiagonalSwapv	Mat44->DiagonalSwapv
#define MatRotateAxis		Mat44->RotateAxis
#define MatOrbitAxis		Mat44->OrbitAxis
#define MatRotateEul		Mat44->RotateEul
#define MatRotateI		Mat44->RotateI
#define MatRotateJ		Mat44->RotateJ
#define MatRotateK		Mat44->RotateK
#define MatTranslate		Mat44->Translate
#define MatTranslate3		Mat44->Translate3
#define MatTranslateX		Mat44->TranslateX
#define MatTranslateY		Mat44->TranslateY
#define MatTranslateZ		Mat44->TranslateZ
#define MatScale		Mat44->Scale
#define MatUniScale		Mat44->UniScale

#if defined(INLINE_SSE) || defined(INLINE_SSE2) || defined(INLINE_SSE3)

#define MatInvert		SG_MatrixInvert44_SSE
#define MatInvertp		SG_MatrixInvert44p_SSE
#define MatMult			SG_MatrixMult44_SSE
#define MatMultv		SG_MatrixMult44v_SSE
#define MatMultpv		SG_MatrixMult44pv_SSE

#else  /* !INLINE_SSE[123] */

#define MatInvert		Mat44->Invert
#define MatInvertp		Mat44->Invertp
#define MatMult			Mat44->Mult
#define MatMultv		Mat44->Multv
#define MatMultpv		Mat44->Multpv

#endif /* INLINE_SSE[123] */

#endif /* _AGAR_INTERNAL or _USE_AGAR_MATH */
