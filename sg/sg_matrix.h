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

#define SG_MatZero		sgMatOps44->Zero
#define SG_MatZerov		sgMatOps44->Zerov
#define SG_MatIdentity		sgMatOps44->Identity
#define SG_MatIdentityv		sgMatOps44->Identityv
#define SG_MatTranspose		sgMatOps44->Transpose
#define SG_MatTransposep	sgMatOps44->Transposep
#define SG_MatTransposev	sgMatOps44->Transposev
#define SG_MatInvertGaussJordanv sgMatOps44->InvertGaussJordanv
#define SG_MatMultVector	sgMatOps44->MultVector
#define SG_MatMultVectorp	sgMatOps44->MultVectorp
#define SG_MatMultVectorv	sgMatOps44->MultVectorv
#define SG_MatMultVector4	sgMatOps44->MultVector4
#define SG_MatMultVector4p	sgMatOps44->MultVector4p
#define SG_MatMultVector4v	sgMatOps44->MultVector4v
#define SG_MatCopy		sgMatOps44->Copy
#define SG_MatToFloats		sgMatOps44->ToFloats
#define SG_MatToDoubles		sgMatOps44->ToDoubles
#define SG_MatFromFloats	sgMatOps44->FromFloats
#define SG_MatFromDoubles	sgMatOps44->FromDoubles
#define SG_MatGetDirection	sgMatOps44->GetDirection
#define SG_MatDiagonalSwapv	sgMatOps44->DiagonalSwapv
#define SG_MatRotateAxis	sgMatOps44->RotateAxis
#define SG_MatOrbitAxis		sgMatOps44->OrbitAxis
#define SG_MatRotateEul		sgMatOps44->RotateEul
#define SG_MatRotateI		sgMatOps44->RotateI
#define SG_MatRotateJ		sgMatOps44->RotateJ
#define SG_MatRotateK		sgMatOps44->RotateK
#define SG_MatTranslate		sgMatOps44->Translate
#define SG_MatTranslate3	sgMatOps44->Translate3
#define SG_MatTranslateX	sgMatOps44->TranslateX
#define SG_MatTranslateY	sgMatOps44->TranslateY
#define SG_MatTranslateZ	sgMatOps44->TranslateZ
#define SG_MatScale		sgMatOps44->Scale
#define SG_MatUniScale		sgMatOps44->UniScale

#if defined(INLINE_SSE) || defined(INLINE_SSE2) || defined(INLINE_SSE3)

#define SG_MatInvert		SG_MatrixInvert44_SSE
#define SG_MatInvertp		SG_MatrixInvert44p_SSE
#define SG_MatMult		SG_MatrixMult44_SSE
#define SG_MatMultv		SG_MatrixMult44v_SSE
#define SG_MatMultpv		SG_MatrixMult44pv_SSE

#else  /* !INLINE_SSE[123] */

#define SG_MatInvert		sgMatOps44->Invert
#define SG_MatInvertp		sgMatOps44->Invertp
#define SG_MatMult		sgMatOps44->Mult
#define SG_MatMultv		sgMatOps44->Multv
#define SG_MatMultpv		sgMatOps44->Multpv

#endif /* INLINE_SSE[123] */

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)
#define Mat44			sgMatOps44
#define MatZero			SG_MatZero
#define MatZerov		SG_MatZerov
#define MatIdentity		SG_MatIdentity
#define MatIdentityv		SG_MatIdentityv
#define MatTranspose		SG_MatTranspose
#define MatTransposep		SG_MatTransposep
#define MatTransposev		SG_MatTransposev
#define MatInvertGaussJordanv	SG_MatInvertGaussJordanv
#define MatMultVector		SG_MatMultVector
#define MatMultVectorp		SG_MatMultVectorp
#define MatMultVectorv		SG_MatMultVectorv
#define MatMultVector4		SG_MatMultVector4
#define MatMultVector4p		SG_MatMultVector4p
#define MatMultVector4v		SG_MatMultVector4v
#define MatCopy			SG_MatCopy
#define MatToFloats		SG_MatToFloats
#define MatToDoubles		SG_MatToDoubles
#define MatFromFloats		SG_MatFromFloats
#define MatFromDoubles		SG_MatFromDoubles
#define MatGetDirection		SG_MatGetDirection
#define MatDiagonalSwapv	SG_MatDiagonalSwapv
#define MatRotateAxis		SG_MatRotateAxis
#define MatOrbitAxis		SG_MatOrbitAxis
#define MatRotateEul		SG_MatRotateEul
#define MatRotateI		SG_MatRotateI
#define MatRotateJ		SG_MatRotateJ
#define MatRotateK		SG_MatRotateK
#define MatTranslate		SG_MatTranslate
#define MatTranslate3		SG_MatTranslate3
#define MatTranslateX		SG_MatTranslateX
#define MatTranslateY		SG_MatTranslateY
#define MatTranslateZ		SG_MatTranslateZ
#define MatScale		SG_MatScale
#define MatUniScale		SG_MatUniScale
#define MatInvert		SG_MatInvert
#define MatInvertp		SG_MatInvertp
#define MatMult			SG_MatMult
#define MatMultv		SG_MatMultv
#define MatMultpv		SG_MatMultpv
#endif /* _AGAR_INTERNAL or _USE_AGAR_MATH */
