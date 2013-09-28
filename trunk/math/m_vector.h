/*	Public domain	*/

#ifdef AG_DEBUG
# define M_VEC_ENTRY_EXISTS(v,i) ((i) >= 0 && (i) < MVECTOR(v)->m)
# define M_ASSERT_MATCHING_VECTORS(a, b, ret) \
	do { \
		if (MVECSIZE(a) != MVECSIZE(b)) { \
			AG_SetError("Incompatible vectors %u != %u", \
			    MVECSIZE(a), MVECSIZE(b)); \
			return (ret); \
		} \
	} while (0)
#else
# define M_VEC_ENTRY_EXISTS(A,i) 1
# define M_ASSERT_MATCHING_VECTORS(a, b, ret)
#endif

/* Operations on vectors in R^n */
typedef struct m_vector_ops {
	const char *name;
	
	/* Basic access functions */
	M_Vector *(*NewVector)(Uint m);
	M_Real   *(*GetElement)(const M_Vector *v, Uint j);
	void      (*SetZero)(M_Vector *v);
	int       (*Resize)(M_Vector *v, Uint m);
	void      (*FreeVector)(M_Vector *v);
	M_Vector *(*Flip)(const M_Vector *v);
	M_Vector *(*Scale)(const M_Vector *v, M_Real c);
	int       (*Scalev)(M_Vector *v, M_Real c);
	M_Vector *(*Add)(const M_Vector *a, const M_Vector *b);
	int       (*Addv)(M_Vector *a, const M_Vector *b);
	M_Vector *(*Sub)(const M_Vector *a, const M_Vector *b);
	int       (*Subv)(M_Vector *a, const M_Vector *b);
	M_Real    (*Len)(const M_Vector *v);
	M_Real    (*Dot)(const M_Vector *a, const M_Vector *b);
	M_Real    (*Distance)(const M_Vector *a, const M_Vector *b);
	M_Vector *(*Norm)(const M_Vector *a);
	M_Vector *(*LERP)(const M_Vector *a, const M_Vector *b, M_Real c);
	M_Vector *(*ElemPow)(const M_Vector *a, M_Real pow);
	int       (*Copy)(M_Vector *x, const M_Vector *y);
	M_Vector *(*Read)(AG_DataSource *ds);
	void      (*Write)(AG_DataSource *ds, const M_Vector *v);
	M_Vector *(*FromReals)(Uint, const M_Real *);
	M_Vector *(*FromFloats)(Uint, const float *);
	M_Vector *(*FromDoubles)(Uint, const double *);
#ifdef AG_HAVE_LONG_DOUBLE
	M_Vector *(*FromLongDoubles)(Uint, const long double *);
#else /* Padding */
	M_Vector *(*FromLongDoubles)(Uint, const double *);
#endif
} M_VectorOps;

/* Operations on vectors in R^2 */
typedef struct m_vector_ops2 {
	const char *name;

	M_Vector2 (*Zero)(void);
	M_Vector2 (*Get)(M_Real, M_Real);
	void      (*Set)(M_Vector2 *, M_Real, M_Real);
	void      (*Copy)(M_Vector2 *, const M_Vector2 *);
	M_Vector2 (*Flip)(M_Vector2);
	M_Real    (*Len)(M_Vector2);
	M_Real    (*Lenp)(const M_Vector2 *);
	M_Real    (*Dot)(M_Vector2, M_Vector2);
	M_Real    (*Dotp)(const M_Vector2 *, const M_Vector2 *);
	M_Real    (*PerpDot)(M_Vector2, M_Vector2);
	M_Real    (*PerpDotp)(const M_Vector2 *, const M_Vector2 *);
	M_Real    (*Distance)(M_Vector2, M_Vector2);
	M_Real    (*Distancep)(const M_Vector2 *, const M_Vector2 *);
	M_Vector2 (*Norm)(M_Vector2);
	M_Vector2 (*Normp)(const M_Vector2 *);
	void      (*Normv)(M_Vector2 *);
	M_Vector2 (*Scale)(M_Vector2, M_Real);
	M_Vector2 (*Scalep)(const M_Vector2 *, M_Real);
	void      (*Scalev)(M_Vector2 *, M_Real);
	M_Vector2 (*Add)(M_Vector2, M_Vector2);
	M_Vector2 (*Addp)(const M_Vector2 *, const M_Vector2 *);
	void      (*Addv)(M_Vector2 *, const M_Vector2 *);
	M_Vector2 (*Sum)(const M_Vector2 *, Uint);
	M_Vector2 (*Sub)(M_Vector2, M_Vector2);
	M_Vector2 (*Subp)(const M_Vector2 *, const M_Vector2 *);
	void      (*Subv)(M_Vector2 *, const M_Vector2 *);
	M_Vector2 (*Avg)(M_Vector2, M_Vector2);
	M_Vector2 (*Avgp)(const M_Vector2 *, const M_Vector2 *);
	M_Vector2 (*LERP)(M_Vector2, M_Vector2, M_Real);
	M_Vector2 (*LERPp)(M_Vector2 *, M_Vector2 *, M_Real);
	M_Vector2 (*ElemPow)(M_Vector2, M_Real);
	M_Real    (*VecAngle)(M_Vector2, M_Vector2);
} M_VectorOps2;

/* Operations on vectors in R^3 */
typedef struct m_vector_ops3 {
	const char *name;

	M_Vector3 (*Zero)(void);
	M_Vector3 (*Get)(M_Real, M_Real, M_Real);
	void      (*Set)(M_Vector3 *, M_Real, M_Real, M_Real);
	void      (*Copy)(M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*Flip)(M_Vector3);
	M_Real	  (*Len)(M_Vector3);
	M_Real    (*Lenp)(const M_Vector3 *);
	M_Real    (*Dot)(M_Vector3, M_Vector3);
	M_Real    (*Dotp)(const M_Vector3 *, const M_Vector3 *);
	M_Real    (*Distance)(M_Vector3, M_Vector3);
	M_Real    (*Distancep)(const M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*Norm)(M_Vector3);
	M_Vector3 (*Normp)(const M_Vector3 *);
	void      (*Normv)(M_Vector3 *);
	M_Vector3 (*Cross)(M_Vector3, M_Vector3);
	M_Vector3 (*Crossp)(const M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*NormCross)(M_Vector3, M_Vector3);
	M_Vector3 (*NormCrossp)(const M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*Scale)(M_Vector3, M_Real);
	M_Vector3 (*Scalep)(const M_Vector3 *, M_Real);
	void      (*Scalev)(M_Vector3 *, M_Real);
	M_Vector3 (*Add)(M_Vector3, M_Vector3);
	M_Vector3 (*Addp)(const M_Vector3 *, const M_Vector3 *);
	void	  (*Addv)(M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*Sum)(const M_Vector3 *, Uint);
	M_Vector3 (*Sub)(M_Vector3, M_Vector3);
	M_Vector3 (*Subp)(const M_Vector3 *, const M_Vector3 *);
	void	  (*Subv)(M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*Avg)(M_Vector3, M_Vector3);
	M_Vector3 (*Avgp)(const M_Vector3 *, const M_Vector3 *);
	M_Vector3 (*LERP)(M_Vector3, M_Vector3, M_Real);
	M_Vector3 (*LERPp)(M_Vector3 *, M_Vector3 *, M_Real);
	M_Vector3 (*ElemPow)(M_Vector3, M_Real);
	void	  (*VecAngle)(M_Vector3, M_Vector3, M_Real *, M_Real *);
} M_VectorOps3;

/* Operations on vectors in R^4 */
typedef struct m_vector_ops4 {
	const char *name;

	M_Vector4 (*Zero)(void);
	M_Vector4 (*Get)(M_Real, M_Real, M_Real, M_Real);
	void      (*Set)(M_Vector4 *, M_Real, M_Real, M_Real, M_Real);
	void      (*Copy)(M_Vector4 *, const M_Vector4 *);
	M_Vector4 (*Flip)(M_Vector4);
	M_Real    (*Len)(M_Vector4);
	M_Real    (*Lenp)(const M_Vector4 *);
	M_Real    (*Dot)(M_Vector4, M_Vector4);
	M_Real    (*Dotp)(const M_Vector4 *, const M_Vector4 *);
	M_Real    (*Distance)(M_Vector4, M_Vector4);
	M_Real    (*Distancep)(const M_Vector4 *, const M_Vector4 *);
	M_Vector4 (*Norm)(M_Vector4);
	M_Vector4 (*Normp)(const M_Vector4 *);
	void      (*Normv)(M_Vector4 *);
	M_Vector4 (*Scale)(M_Vector4, M_Real);
	M_Vector4 (*Scalep)(const M_Vector4 *, M_Real);
	void      (*Scalev)(M_Vector4 *, M_Real);
	M_Vector4 (*Add)(M_Vector4, M_Vector4);
	M_Vector4 (*Addp)(const M_Vector4 *, const M_Vector4 *);
	void      (*Addv)(M_Vector4 *, const M_Vector4 *);
	M_Vector4 (*Sum)(const M_Vector4 *, Uint);
	M_Vector4 (*Sub)(M_Vector4, M_Vector4);
	M_Vector4 (*Subp)(const M_Vector4 *, const M_Vector4 *);
	void      (*Subv)(M_Vector4 *, const M_Vector4 *);
	M_Vector4 (*Avg)(M_Vector4, M_Vector4);
	M_Vector4 (*Avgp)(const M_Vector4 *, const M_Vector4 *);
	M_Vector4 (*LERP)(M_Vector4, M_Vector4, M_Real);
	M_Vector4 (*LERPp)(M_Vector4 *, M_Vector4 *, M_Real);
	M_Vector4 (*ElemPow)(M_Vector4, M_Real);
	void      (*VecAngle)(M_Vector4, M_Vector4, M_Real *, M_Real *,
	                      M_Real *);
} M_VectorOps4;

__BEGIN_DECLS
extern const M_VectorOps2 *mVecOps2;
extern const M_VectorOps3 *mVecOps3;
extern const M_VectorOps4 *mVecOps4;
extern const M_VectorOps *mVecOps;

static __inline__ void
M_VectorInit(M_Vector *v, Uint m)
{
	v->m = m;
}

/*
 * Given some vector in projective space, return its Euclidean vector,
 * if it exists -- otherwise generate a division by zero.
 */
static __inline__ M_Vector2
M_VecFromProj2(M_Vector3 Pv)
{
	M_Vector2 v;
	v.x = Pv.x/Pv.z;
	v.y = Pv.y/Pv.z;
	return (v);
}
static __inline__ M_Vector3
M_VecFromProj3(M_Vector4 Pv)
{
	M_Vector3 v;
	v.x = Pv.x/Pv.w;
	v.y = Pv.y/Pv.w;
	v.z = Pv.z/Pv.w;
	return (v);
}

/*
 * Convert an Euclidean vector to a vector in projective space.
 */
static __inline__ M_Vector3
M_VecToProj2(M_Vector2 v, M_Real z)
{
	M_Vector3 Pv;
	Pv.x = v.x;
	Pv.y = v.y;
	Pv.z = z;
	return (Pv);
}
static __inline__ M_Vector4
M_VecToProj3(M_Vector3 v, M_Real w)
{
	M_Vector4 Pv;
	Pv.x = v.x;
	Pv.y = v.y;
	Pv.z = v.z;
	Pv.w = w;
	return (Pv);
}
__END_DECLS

#include <agar/math/m_vector_fpu.h>
#include <agar/math/m_vector2_fpu.h>
#include <agar/math/m_vector3_fpu.h>
#include <agar/math/m_vector4_fpu.h>
#include <agar/math/m_vector3_sse.h>

__BEGIN_DECLS
void       M_VectorInitEngine(void);

M_Vector2  M_ReadVector2(AG_DataSource *);
M_Vector3  M_ReadVector3(AG_DataSource *);
M_Vector4  M_ReadVector4(AG_DataSource *);
int        M_ReadVector2v(AG_DataSource *, M_Vector2 *);
int	   M_ReadVector3v(AG_DataSource *, M_Vector3 *);
int	   M_ReadVector4v(AG_DataSource *, M_Vector4 *);

void	   M_WriteVector2(AG_DataSource *, const M_Vector2 *);
void	   M_WriteVector3(AG_DataSource *, const M_Vector3 *);
void	   M_WriteVector4(AG_DataSource *, const M_Vector4 *);

M_Vector2  M_RealvToVector2(const M_Real *);
M_Vector3  M_RealvToVector3(const M_Real *);
M_Vector4  M_RealvToVector4(const M_Real *);
M_Vector2 *M_VectorDup2(const M_Vector2 *);
M_Vector3 *M_VectorDup3(const M_Vector3 *);
M_Vector4 *M_VectorDup4(const M_Vector4 *);
__END_DECLS

#define M_VECTOR2	M_VecGet2
#define M_VECTOR3	M_VecGet3
#define M_VECTOR4	M_VecGet4

/*
 * Operations on vectors in R^n
 */
#define M_VecNew		mVecOps->NewVector
#define M_VecGetElement         mVecOps->GetElement
#define M_VecResize             mVecOps->Resize
#define M_VecFree		mVecOps->FreeVector
#define M_VecSetZero		mVecOps->SetZero
#define M_VecDup		mVecOps->Dup
#define M_VecCopy		mVecOps->Copy
#define M_VecFlip		mVecOps->Flip
#define M_VecScale		mVecOps->Scale
#define M_VecScalev		mVecOps->Scalev
#define M_VecAdd		mVecOps->Add
#define M_VecAddv		mVecOps->Addv
#define M_VecSub		mVecOps->Sub
#define M_VecSubv		mVecOps->Subv
#define M_VecLen		mVecOps->Len
#define M_VecDot		mVecOps->Dot
#define M_VecDistance		mVecOps->Distance
#define M_VecNorm		mVecOps->Norm
#define M_VecLERP		mVecOps->LERP
#define M_VecElemPow		mVecOps->ElemPow
#define M_ReadVector		mVecOps->Read
#define M_WriteVector		mVecOps->Write
#define M_VecFromReals		mVecOps->FromReals
#define M_VecFromFloats		mVecOps->FromFloats
#define M_VecFromDoubles	mVecOps->FromDoubles
#define M_VecFromLongDoubles	mVecOps->FromLongDoubles

/* Simple wrappers */
#define M_VecGet(v,i) *M_VecGetElement(v, i)
#define M_VecSet(v,i,val) *M_VecGetElement(v, i) = val;

/*
 * Operations on vectors in R^2
 */
#define M_VecI2()		mVecOps2->Get(1.0,0.0)
#define M_VecJ2()		mVecOps2->Get(0.0,1.0)
#define M_VecZero2		mVecOps2->Zero
#define M_VecGet2		mVecOps2->Get
#define M_VecSet2		mVecOps2->Set
#define M_VecCopy2		mVecOps2->Copy
#define M_VecFlip2		mVecOps2->Flip
#define M_VecLen2		mVecOps2->Len
#define M_VecLen2p		mVecOps2->Lenp
#define M_VecDot2		mVecOps2->Dot
#define M_VecDot2p		mVecOps2->Dotp
#define M_VecPerpDot2		mVecOps2->PerpDot
#define M_VecPerpDot2p		mVecOps2->PerpDotp
#define M_VecDistance2		mVecOps2->Distance
#define M_VecDistance2p 	mVecOps2->Distancep
#define M_VecNorm2		mVecOps2->Norm
#define M_VecNorm2p		mVecOps2->Normp
#define M_VecNorm2v		mVecOps2->Normv
#define M_VecScale2		mVecOps2->Scale
#define M_VecScale2p		mVecOps2->Scalep
#define M_VecScale2v		mVecOps2->Scalev
#define M_VecAdd2		mVecOps2->Add
#define M_VecAdd2p		mVecOps2->Addp
#define M_VecAdd2v		mVecOps2->Addv
#define M_VecSum2		mVecOps2->Sum
#define M_VecSub2		mVecOps2->Sub
#define M_VecSub2p		mVecOps2->Subp
#define M_VecSub2v		mVecOps2->Subv
#define M_VecAvg2		mVecOps2->Avg
#define M_VecAvg2p		mVecOps2->Avgp
#define M_VecLERP2		mVecOps2->LERP
#define M_VecLERP2p		mVecOps2->LERPp
#define M_VecElemPow2		mVecOps2->ElemPow
#define M_VecVecAngle2		mVecOps2->VecAngle

/*
 * Operations on vectors in R^3
 */
#define M_VecI3()		mVecOps3->Get(1.0,0.0,0.0)
#define M_VecJ3()		mVecOps3->Get(0.0,1.0,0.0)
#define M_VecK3()		mVecOps3->Get(0.0,0.0,1.0)
#if defined(INLINE_SSE)
# define M_VecZero3		M_VectorZero3_SSE
# define M_VecGet3		M_VectorGet3_SSE
# define M_VecSet3		M_VectorSet3_SSE
# define M_VecCopy3		M_VectorCopy3_SSE
# define M_VecFlip3		M_VectorFlip3_SSE
# define M_VecLen3		M_VectorLen3_SSE
# define M_VecLen3p		M_VectorLen3p_SSE
# define M_VecDot3		M_VectorDot3_SSE
# define M_VecDot3p		M_VectorDot3p_SSE
# define M_VecDistance3		M_VectorDistance3_SSE
# define M_VecDistance3p	M_VectorDistance3p_SSE
# define M_VecNorm3		M_VectorNorm3_SSE
# define M_VecNorm3p		M_VectorNorm3p_SSE
# define M_VecNorm3v		M_VectorNorm3v_SSE
# define M_VecCross3		M_VectorCross3_SSE
# define M_VecCross3p		M_VectorCross3p_SSE
# define M_VecNormCross3	M_VectorNormCross3_SSE
# define M_VecNormCross3p	M_VectorNormCross3p_SSE
# define M_VecScale3		M_VectorScale3_SSE
# define M_VecScale3p		M_VectorScale3p_SSE
# define M_VecScale3v		M_VectorScale3v_SSE
# define M_VecAdd3		M_VectorAdd3_SSE
# define M_VecAdd3p		M_VectorAdd3p_SSE
# define M_VecAdd3v		M_VectorAdd3v_SSE
# define M_VecSum3		M_VectorSum3_SSE
# define M_VecSub3		M_VectorSub3_SSE
# define M_VecSub3p		M_VectorSub3p_SSE
# define M_VecSub3v		M_VectorSub3v_SSE
# define M_VecAvg3		M_VectorAvg3_SSE
# define M_VecAvg3p		M_VectorAvg3p_SSE
# define M_VecLERP3		M_VectorLERP3_SSE
# define M_VecLERP3p		M_VectorLERP3p_SSE
# define M_VecElemPow3		M_VectorElemPow3_SSE
# define M_VecVecAngle3		M_VectorVecAngle3_SSE
#else  /* !INLINE_SSE */
# define M_VecZero3		mVecOps3->Zero
# define M_VecGet3		mVecOps3->Get
# define M_VecSet3		mVecOps3->Set
# define M_VecCopy3		mVecOps3->Copy
# define M_VecFlip3		mVecOps3->Flip
# define M_VecLen3		mVecOps3->Len
# define M_VecLen3p		mVecOps3->Lenp
# define M_VecDot3		mVecOps3->Dot
# define M_VecDot3p		mVecOps3->Dotp
# define M_VecDistance3		mVecOps3->Distance
# define M_VecDistance3p	mVecOps3->Distancep
# define M_VecNorm3		mVecOps3->Norm
# define M_VecNorm3p		mVecOps3->Normp
# define M_VecNorm3v		mVecOps3->Normv
# define M_VecCross3		mVecOps3->Cross
# define M_VecCross3p		mVecOps3->Crossp
# define M_VecNormCross3	mVecOps3->NormCross
# define M_VecNormCross3p	mVecOps3->NormCrossp
# define M_VecScale3		mVecOps3->Scale
# define M_VecScale3p		mVecOps3->Scalep
# define M_VecScale3v		mVecOps3->Scalev
# define M_VecAdd3		mVecOps3->Add
# define M_VecAdd3p		mVecOps3->Addp
# define M_VecAdd3v		mVecOps3->Addv
# define M_VecSum3		mVecOps3->Sum
# define M_VecSub3		mVecOps3->Sub
# define M_VecSub3p		mVecOps3->Subp
# define M_VecSub3v		mVecOps3->Subv
# define M_VecAvg3		mVecOps3->Avg
# define M_VecAvg3p		mVecOps3->Avgp
# define M_VecLERP3		mVecOps3->LERP
# define M_VecLERP3p		mVecOps3->LERPp
# define M_VecElemPow3		mVecOps3->ElemPow
# define M_VecVecAngle3		mVecOps3->VecAngle
#endif /* INLINE_SSE */

/*
 * Operations on vectors in R^4
 */
#define M_VecI4()		mVecOps4->Get(1.0,0.0,0.0,0.0)
#define M_VecJ4()		mVecOps4->Get(0.0,1.0,0.0,0.0)
#define M_VecK4()		mVecOps4->Get(0.0,0.0,1.0,0.0)
#define M_VecL4()		mVecOps4->Get(0.0,0.0,0.0,1.0)
#define M_VecZero4		mVecOps4->Zero
#define M_VecGet4		mVecOps4->Get
#define M_VecSet4		mVecOps4->Set
#define M_VecCopy4		mVecOps4->Copy
#define M_VecFlip4		mVecOps4->Flip
#define M_VecLen4		mVecOps4->Len
#define M_VecLen4p		mVecOps4->Lenp
#define M_VecDot4		mVecOps4->Dot
#define M_VecDot4p		mVecOps4->Dotp
#define M_VecDistance4		mVecOps4->Distance
#define M_VecDistance4p 	mVecOps4->Distancep
#define M_VecNorm4		mVecOps4->Norm
#define M_VecNorm4p		mVecOps4->Normp
#define M_VecNorm4v		mVecOps4->Norm4
#define M_VecScale4		mVecOps4->Scale
#define M_VecScale4p		mVecOps4->Scalep
#define M_VecScale4v		mVecOps4->Scalev
#define M_VecAdd4		mVecOps4->Add
#define M_VecAdd4p		mVecOps4->Addp
#define M_VecAdd4v		mVecOps4->Addv
#define M_VecSum4		mVecOps4->Sum
#define M_VecSub4		mVecOps4->Sub
#define M_VecSub4p		mVecOps4->Subp
#define M_VecSub4v		mVecOps4->Subv
#define M_VecAvg4		mVecOps4->Avg
#define M_VecAvg4p		mVecOps4->Avgp
#define M_VecLERP4		mVecOps4->LERP
#define M_VecLERP4p		mVecOps4->LERPp
#define M_VecElemPow4		mVecOps4->ElemPow
#define M_VecVecAngle4		mVecOps4->VecAngle

