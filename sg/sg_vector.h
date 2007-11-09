/*	Public domain	*/

/* Operations on vectors in R^2 */
typedef struct sg_vector_ops2 {
	const char *name;

	SG_Vector2 (*Zero)(void);
	SG_Vector2 (*Get)(SG_Real, SG_Real);
	void       (*Set)(SG_Vector2 *, SG_Real, SG_Real);
	void       (*Copy)(SG_Vector2 *, const SG_Vector2 *);
	SG_Vector2 (*Mirror)(SG_Vector2, int, int);
	SG_Vector2 (*Mirrorp)(const SG_Vector2 *, int, int);
	SG_Real    (*Len)(SG_Vector2);
	SG_Real    (*Lenp)(const SG_Vector2 *);
	SG_Real    (*Dot)(SG_Vector2, SG_Vector2);
	SG_Real    (*Dotp)(const SG_Vector2 *, const SG_Vector2 *);
	SG_Real    (*PerpDot)(SG_Vector2, SG_Vector2);
	SG_Real    (*PerpDotp)(const SG_Vector2 *, const SG_Vector2 *);
	SG_Real    (*Distance)(SG_Vector2, SG_Vector2);
	SG_Real    (*Distancep)(const SG_Vector2 *, const SG_Vector2 *);
	SG_Vector2 (*Norm)(SG_Vector2);
	SG_Vector2 (*Normp)(const SG_Vector2 *);
	void       (*Normv)(SG_Vector2 *);
	SG_Vector2 (*Scale)(SG_Vector2, SG_Real);
	SG_Vector2 (*Scalep)(const SG_Vector2 *, SG_Real);
	void       (*Scalev)(SG_Vector2 *, SG_Real);
	SG_Vector2 (*Add)(SG_Vector2, SG_Vector2);
	SG_Vector2 (*Addp)(const SG_Vector2 *, const SG_Vector2 *);
	void       (*Addv)(SG_Vector2 *, const SG_Vector2 *);
	SG_Vector2 (*Addn)(int, ...);
	SG_Vector2 (*Sub)(SG_Vector2, SG_Vector2);
	SG_Vector2 (*Subp)(const SG_Vector2 *, const SG_Vector2 *);
	void       (*Subv)(SG_Vector2 *, const SG_Vector2 *);
	SG_Vector2 (*Subn)(int, ...);
	SG_Vector2 (*Avg)(SG_Vector2, SG_Vector2);
	SG_Vector2 (*Avgp)(const SG_Vector2 *, const SG_Vector2 *);
	SG_Vector2 (*LERP)(SG_Vector2, SG_Vector2, SG_Real);
	SG_Vector2 (*LERPp)(SG_Vector2 *, SG_Vector2 *, SG_Real);
	SG_Vector2 (*ElemPow)(SG_Vector2, SG_Real);
	SG_Real    (*VecAngle)(SG_Vector2, SG_Vector2);
	SG_Vector2 (*Rotate)(SG_Vector2, SG_Real);
	void       (*Rotatev)(SG_Vector2 *, SG_Real);
} SG_VectorOps2;

/* Operations on vectors in R^3 */
typedef struct sg_vector_ops3 {
	const char *name;

	SG_Vector  (*Zero)(void);
	SG_Vector  (*Get)(SG_Real, SG_Real, SG_Real);
	void       (*Set)(SG_Vector *, SG_Real, SG_Real, SG_Real);
	void       (*Copy)(SG_Vector *, const SG_Vector *);
	SG_Vector  (*Mirror)(SG_Vector, int, int, int);
	SG_Vector  (*Mirrorp)(const SG_Vector *, int, int, int);
	SG_Real	   (*Len)(SG_Vector);
	SG_Real    (*Lenp)(const SG_Vector *);
	SG_Real    (*Dot)(SG_Vector, SG_Vector);
	SG_Real    (*Dotp)(const SG_Vector *, const SG_Vector *);
	SG_Real    (*Distance)(SG_Vector, SG_Vector);
	SG_Real    (*Distancep)(const SG_Vector *, const SG_Vector *);
	SG_Vector  (*Norm)(SG_Vector);
	SG_Vector  (*Normp)(const SG_Vector *);
	void       (*Normv)(SG_Vector *);
	SG_Vector  (*Cross)(SG_Vector, SG_Vector);
	SG_Vector  (*Crossp)(const SG_Vector *, const SG_Vector *);
	SG_Vector  (*NormCross)(SG_Vector, SG_Vector);
	SG_Vector  (*NormCrossp)(const SG_Vector *, const SG_Vector *);
	SG_Vector  (*Scale)(SG_Vector, SG_Real);
	SG_Vector  (*Scalep)(const SG_Vector *, SG_Real);
	void       (*Scalev)(SG_Vector *, SG_Real);
	SG_Vector  (*Add)(SG_Vector, SG_Vector);
	SG_Vector  (*Addp)(const SG_Vector *, const SG_Vector *);
	void	   (*Addv)(SG_Vector *, const SG_Vector *);
	SG_Vector  (*Addn)(int, ...);
	SG_Vector  (*Sub)(SG_Vector, SG_Vector);
	SG_Vector  (*Subp)(const SG_Vector *, const SG_Vector *);
	void	   (*Subv)(SG_Vector *, const SG_Vector *);
	SG_Vector  (*Subn)(int, ...);
	SG_Vector  (*Avg)(SG_Vector, SG_Vector);
	SG_Vector  (*Avgp)(const SG_Vector *, const SG_Vector *);
	SG_Vector  (*LERP)(SG_Vector, SG_Vector, SG_Real);
	SG_Vector  (*LERPp)(SG_Vector *, SG_Vector *, SG_Real);
	SG_Vector  (*ElemPow)(SG_Vector, SG_Real);
	void	   (*VecAngle)(SG_Vector, SG_Vector, SG_Real *, SG_Real *);
	SG_Vector  (*Rotate)(SG_Vector, SG_Real, SG_Vector);
	void       (*Rotatev)(SG_Vector *, SG_Real, SG_Vector);
	SG_Vector  (*RotateQuat)(SG_Vector, SG_Quat);
	SG_Vector  (*RotateI)(SG_Vector, SG_Real);
	SG_Vector  (*RotateJ)(SG_Vector, SG_Real);
	SG_Vector  (*RotateK)(SG_Vector, SG_Real);
} SG_VectorOps3;

/* Operations on vectors in R^4 */
typedef struct sg_vector_ops4 {
	const char *name;

	SG_Vector4 (*Zero)(void);
	SG_Vector4 (*Get)(SG_Real, SG_Real, SG_Real, SG_Real);
	void       (*Set)(SG_Vector4 *, SG_Real, SG_Real, SG_Real, SG_Real);
	void       (*Copy)(SG_Vector4 *, const SG_Vector4 *);
	SG_Vector4 (*Mirror)(SG_Vector4, int, int, int, int);
	SG_Vector4 (*Mirrorp)(const SG_Vector4 *, int, int, int, int);
	SG_Real    (*Len)(SG_Vector4);
	SG_Real    (*Lenp)(const SG_Vector4 *);
	SG_Real    (*Dot)(SG_Vector4, SG_Vector4);
	SG_Real    (*Dotp)(const SG_Vector4 *, const SG_Vector4 *);
	SG_Real    (*Distance)(SG_Vector4, SG_Vector4);
	SG_Real    (*Distancep)(const SG_Vector4 *, const SG_Vector4 *);
	SG_Vector4 (*Norm)(SG_Vector4);
	SG_Vector4 (*Normp)(const SG_Vector4 *);
	void       (*Normv)(SG_Vector4 *);
	SG_Vector4 (*Cross)(SG_Vector4, SG_Vector4, SG_Vector4);
	SG_Vector4 (*Crossp)(const SG_Vector4 *, const SG_Vector4 *,
	                                         const SG_Vector4 *);
	SG_Vector4 (*NormCross)(SG_Vector4, SG_Vector4, SG_Vector4);
	SG_Vector4 (*NormCrossp)(const SG_Vector4 *, const SG_Vector4 *,
	                                             const SG_Vector4 *);
	SG_Vector4 (*Scale)(SG_Vector4, SG_Real);
	SG_Vector4 (*Scalep)(const SG_Vector4 *, SG_Real);
	void       (*Scalev)(SG_Vector4 *, SG_Real);
	SG_Vector4 (*Add)(SG_Vector4, SG_Vector4);
	SG_Vector4 (*Addp)(const SG_Vector4 *, const SG_Vector4 *);
	void       (*Addv)(SG_Vector4 *, const SG_Vector4 *);
	SG_Vector4 (*Addn)(int, ...);
	SG_Vector4 (*Sub)(SG_Vector4, SG_Vector4);
	SG_Vector4 (*Subp)(const SG_Vector4 *, const SG_Vector4 *);
	void       (*Subv)(SG_Vector4 *, const SG_Vector4 *);
	SG_Vector4 (*Subn)(int, ...);
	SG_Vector4 (*Avg)(SG_Vector4, SG_Vector4);
	SG_Vector4 (*Avgp)(const SG_Vector4 *, const SG_Vector4 *);
	SG_Vector4 (*LERP)(SG_Vector4, SG_Vector4, SG_Real);
	SG_Vector4 (*LERPp)(SG_Vector4 *, SG_Vector4 *, SG_Real);
	SG_Vector4 (*ElemPow)(SG_Vector4, SG_Real);
	void	   (*VecAngle)(SG_Vector4, SG_Vector4, SG_Real *, SG_Real *,
	                       SG_Real *);
	SG_Vector4 (*Rotate)(SG_Vector4, SG_Real, SG_Vector4);
	void       (*Rotatev)(SG_Vector4 *, SG_Real, SG_Vector4);
} SG_VectorOps4;

__BEGIN_DECLS
extern const SG_VectorOps2 *sgVecOps2;
extern const SG_VectorOps3 *sgVecOps3;
extern const SG_VectorOps4 *sgVecOps4;
__END_DECLS

#if defined(_AGAR_INTERNAL)
#include <sg/sg_vector2_fpu.h>
#include <sg/sg_vector3_fpu.h>
#include <sg/sg_vector4_fpu.h>
#include <sg/sg_vector3_sse.h>
#include <sg/sg_vector3_sse3.h>
#else
#include <agar/sg/sg_vector2_fpu.h>
#include <agar/sg/sg_vector3_fpu.h>
#include <agar/sg/sg_vector4_fpu.h>
#include <agar/sg/sg_vector3_sse.h>
#include <agar/sg/sg_vector3_sse3.h>
#endif

__BEGIN_DECLS
void       SG_VectorInitEngine(void);
SG_Vector2 SG_ReadVector2(AG_DataSource *);
SG_Vector  SG_ReadVector (AG_DataSource *);
SG_Vector4 SG_ReadVector4(AG_DataSource *);
void	   SG_ReadVector2v(AG_DataSource *, SG_Vector2 *);
void	   SG_ReadVectorv (AG_DataSource *, SG_Vector  *);
void	   SG_ReadVector4v(AG_DataSource *, SG_Vector4 *);
void	   SG_WriteVector2(AG_DataSource *, SG_Vector2 *);
void	   SG_WriteVector (AG_DataSource *, SG_Vector  *);
void	   SG_WriteVector4(AG_DataSource *, SG_Vector4 *);
SG_Vector2 SG_ReadVectorf2(AG_DataSource *);
SG_Vector  SG_ReadVectorf (AG_DataSource *);
SG_Vector4 SG_ReadVectorf4(AG_DataSource *);
void	   SG_ReadVectorf2v(AG_DataSource *, SG_Vector2 *);
void	   SG_ReadVectorfv (AG_DataSource *, SG_Vector  *);
void	   SG_ReadVectorf4v(AG_DataSource *, SG_Vector4 *);
void	   SG_WriteVectorf2(AG_DataSource *, SG_Vector2 *);
void	   SG_WriteVectorf (AG_DataSource *, SG_Vector *);
void	   SG_WriteVectorf4(AG_DataSource *, SG_Vector4 *);

SG_Vector2 SG_RealvToVector2(const SG_Real *);
SG_Vector  SG_RealvToVector(const SG_Real *);
SG_Vector4 SG_RealvToVector4(const SG_Real *);
SG_Vector2 SG_Vector3to2(SG_Vector);
SG_Vector  SG_Vector2to3(SG_Vector2);
SG_Vector4 SG_Vector3to4(SG_Vector);
__END_DECLS

#define SG_VecI2()	sgVecOps2->Get(1.0,0.0)
#define SG_VecJ2()	sgVecOps2->Get(0.0,1.0)
#define SG_VecI()	sgVecOps3->Get(1.0,0.0,0.0)
#define SG_VecJ()	sgVecOps3->Get(0.0,1.0,0.0)
#define SG_VecK()	sgVecOps3->Get(0.0,0.0,1.0)
#define SG_VecI4()	sgVecOps4->Get(1.0,0.0,0.0,0.0)
#define SG_VecJ4()	sgVecOps4->Get(0.0,1.0,0.0,0.0)
#define SG_VecK4()	sgVecOps4->Get(0.0,0.0,1.0,0.0)
#define SG_VecL4()	sgVecOps4->Get(0.0,0.0,0.0,1.0)

#define SG_VecZero2	sgVecOps2->Zero
#define SG_VecGet2	sgVecOps2->Get
#define SG_VecSet2	sgVecOps2->Set
#define SG_VecCopy2	sgVecOps2->Copy
#define SG_VecMirror2	sgVecOps2->Mirror
#define SG_VecMirror2p	sgVecOps2->Mirrorp
#define SG_VecLen2	sgVecOps2->Len
#define SG_VecLen2p	sgVecOps2->Lenp
#define SG_VecDot2	sgVecOps2->Dot
#define SG_VecDot2p	sgVecOps2->Dotp
#define SG_VecPerpDot2	sgVecOps2->PerpDot
#define SG_VecPerpDot2p	sgVecOps2->PerpDotp
#define SG_VecDistance2	sgVecOps2->Distance
#define SG_VecDistance2p sgVecOps2->Distancep
#define SG_VecNorm2	sgVecOps2->Norm
#define SG_VecNorm2p	sgVecOps2->Normp
#define SG_VecNorm2v	sgVecOps2->Normv
#define SG_VecScale2	sgVecOps2->Scale
#define SG_VecScale2p	sgVecOps2->Scalep
#define SG_VecScale2v	sgVecOps2->Scalev
#define SG_VecAdd2	sgVecOps2->Add
#define SG_VecAdd2p	sgVecOps2->Addp
#define SG_VecAdd2v	sgVecOps2->Addv
#define SG_VecAdd2n	sgVecOps2->Addn
#define SG_VecSub2	sgVecOps2->Sub
#define SG_VecSub2p	sgVecOps2->Subp
#define SG_VecSub2v	sgVecOps2->Subv
#define SG_VecSub2n	sgVecOps2->Subn
#define SG_VecAvg2	sgVecOps2->Avg
#define SG_VecAvg2p	sgVecOps2->Avgp
#define SG_VecLERP2	sgVecOps2->LERP
#define SG_VecLERP2p	sgVecOps2->LERPp
#define SG_VecElemPow2	sgVecOps2->ElemPow
#define SG_VecVecAngle2	sgVecOps2->VecAngle
#define SG_VecRotate2	sgVecOps2->Rotate
#define SG_VecRotate2v	sgVecOps2->Rotatev

#if defined(INLINE_SSE) || defined(INLINE_SSE2) || defined(INLINE_SSE3)

#define SG_VecZero	SG_VectorZero3_SSE
#define SG_VecGet	SG_VectorGet3_SSE
#define SG_VecSet	SG_VectorSet3_FPU
#define SG_VecCopy	SG_VectorCopy3_FPU
#define SG_VecMirror	SG_VectorMirror3_SSE
#define SG_VecMirrorp	SG_VectorMirror3p_SSE
#define SG_VecLen	SG_VectorLen3_FPU
#define SG_VecLenp	SG_VectorLen3p_FPU
#ifdef INLINE_SSE3
# define SG_VecDot	SG_VectorDot3_SSE3
# define SG_VecDotp	SG_VectorDot3p_SSE3
#else
# define SG_VecDot	SG_VectorDot3_FPU
# define SG_VecDotp	SG_VectorDot3p_FPU
#endif
#define SG_VecDistance	SG_VectorDistance3_SSE
#define SG_VecDistancep	SG_VectorDistance3p_SSE
#define SG_VecNorm	SG_VectorNorm3_SSE
#define SG_VecNormp	SG_VectorNorm3p_SSE
#define SG_VecNormv	SG_VectorNorm3v_FPU
#define SG_VecCross	SG_VectorCross3_FPU
#define SG_VecCrossp	SG_VectorCross3p_FPU
#define SG_VecNormCross	SG_VectorNormCross3_FPU
#define SG_VecNormCrossp SG_VectorNormCross3p_FPU
#define SG_VecScale	SG_VectorScale3_SSE
#define SG_VecScalep	SG_VectorScale3p_SSE
#define SG_VecScalev	SG_VectorScale3v_FPU
#define SG_VecAdd	SG_VectorAdd3_SSE
#define SG_VecAddp	SG_VectorAdd3p_SSE
#define SG_VecAddv	SG_VectorAdd3v_FPU
#define SG_VecAddn	SG_VectorAdd3n_SSE
#define SG_VecSub	SG_VectorSub3_SSE
#define SG_VecSubp	SG_VectorSub3p_SSE
#define SG_VecSubv	SG_VectorSub3v_FPU
#define SG_VecSubn	SG_VectorSub3n_SSE
#define SG_VecAvg	SG_VectorAvg3_SSE
#define SG_VecAvgp	SG_VectorAvg3p_SSE
#define SG_VecLERP	SG_VectorLERP3_SSE
#define SG_VecLERPp	SG_VectorLERP3p_SSE
#define SG_VecElemPow	SG_VectorElemPow3_SSE
#define SG_VecVecAngle	SG_VectorVecAngle3_SSE
#define SG_VecRotate	SG_VectorRotate3_SSE
#define SG_VecRotatev	SG_VectorRotate3v_SSE
#define SG_VecRotateQuat SG_VectorRotateQuat3_SSE
#define SG_VecRotateI	SG_VectorRotateI3_SSE
#define SG_VecRotateJ	SG_VectorRotateJ3_SSE
#define SG_VecRotateK	SG_VectorRotateK3_SSE

#else  /* !INLINE_SSE[123] */

#define SG_VecZero	sgVecOps3->Zero
#define SG_VecGet	sgVecOps3->Get
#define SG_VecSet	sgVecOps3->Set
#define SG_VecCopy	sgVecOps3->Copy
#define SG_VecMirror	sgVecOps3->Mirror
#define SG_VecMirrorp	sgVecOps3->Mirrorp
#define SG_VecLen	sgVecOps3->Len
#define SG_VecLenp	sgVecOps3->Lenp
#define SG_VecDot	sgVecOps3->Dot
#define SG_VecDotp	sgVecOps3->Dotp
#define SG_VecDistance	sgVecOps3->Distance
#define SG_VecDistancep	sgVecOps3->Distancep
#define SG_VecNorm	sgVecOps3->Norm
#define SG_VecNormp	sgVecOps3->Normp
#define SG_VecNormv	sgVecOps3->Normv
#define SG_VecCross	sgVecOps3->Cross
#define SG_VecCrossp	sgVecOps3->Crossp
#define SG_VecNormCross	sgVecOps3->NormCross
#define SG_VecNormCrossp sgVecOps3->NormCrossp
#define SG_VecScale	sgVecOps3->Scale
#define SG_VecScalep	sgVecOps3->Scalep
#define SG_VecScalev	sgVecOps3->Scalev
#define SG_VecAdd	sgVecOps3->Add
#define SG_VecAddp	sgVecOps3->Addp
#define SG_VecAddv	sgVecOps3->Addv
#define SG_VecAddn	sgVecOps3->Addn
#define SG_VecSub	sgVecOps3->Sub
#define SG_VecSubp	sgVecOps3->Subp
#define SG_VecSubv	sgVecOps3->Subv
#define SG_VecSubn	sgVecOps3->Subn
#define SG_VecAvg	sgVecOps3->Avg
#define SG_VecAvgp	sgVecOps3->Avgp
#define SG_VecLERP	sgVecOps3->LERP
#define SG_VecLERPp	sgVecOps3->LERPp
#define SG_VecElemPow	sgVecOps3->ElemPow
#define SG_VecVecAngle	sgVecOps3->VecAngle
#define SG_VecRotate	sgVecOps3->Rotate
#define SG_VecRotatev	sgVecOps3->Rotatev
#define SG_VecRotateQuat sgVecOps3->RotateQuat
#define SG_VecRotateI	sgVecOps3->RotateI
#define SG_VecRotateJ	sgVecOps3->RotateJ
#define SG_VecRotateK	sgVecOps3->RotateK

#endif /* INLINE_SSE[123] */

#define SG_VecZero4	sgVecOps4->Zero
#define SG_VecGet4	sgVecOps4->Get
#define SG_VecSet4	sgVecOps4->Set
#define SG_VecCopy4	sgVecOps4->Copy
#define SG_VecMirror4	sgVecOps4->Mirror
#define SG_VecMirror4p	sgVecOps4->Mirrorp
#define SG_VecLen4	sgVecOps4->Len
#define SG_VecLen4p	sgVecOps4->Lenp
#define SG_VecDot4	sgVecOps4->Dot
#define SG_VecDot4p	sgVecOps4->Dotp
#define SG_VecDistance4	sgVecOps4->Distance
#define SG_VecDistance4p sgVecOps4->Distancep
#define SG_VecNorm4	sgVecOps4->Norm
#define SG_VecNorm4p	sgVecOps4->Normp
#define SG_VecNorm4v	sgVecOps4->Norm4
#define SG_VecCross4	sgVecOps4->Cross
#define SG_VecCross4p	sgVecOps4->Crossp
#define SG_VecNormCross4 sgVecOps4->NormCross
#define SG_VecNormCross4p sgVecOps4->NormCrossp
#define SG_VecScale4	sgVecOps4->Scale
#define SG_VecScale4p	sgVecOps4->Scalep
#define SG_VecScale4v	sgVecOps4->Scalev
#define SG_VecAdd4	sgVecOps4->Add
#define SG_VecAdd4p	sgVecOps4->Addp
#define SG_VecAdd4v	sgVecOps4->Addv
#define SG_VecAdd4n	sgVecOps4->Addn
#define SG_VecSub4	sgVecOps4->Sub
#define SG_VecSub4p	sgVecOps4->Subp
#define SG_VecSub4v	sgVecOps4->Subv
#define SG_VecSub4n	sgVecOps4->Subn
#define SG_VecAvg4	sgVecOps4->Avg
#define SG_VecAvg4p	sgVecOps4->Avgp
#define SG_VecLERP4	sgVecOps4->LERP
#define SG_VecLERP4p	sgVecOps4->LERPp
#define SG_VecElemPow4	sgVecOps4->ElemPow
#define SG_VecVecAngle4	sgVecOps4->VecAngle
#define SG_VecRotate4	sgVecOps4->Rotate
#define SG_VecRotate4v	sgVecOps4->Rotatev

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)

#define VecOps2 sgVecOps2
#define VecOps3 sgVecOps3
#define VecOps4 sgVecOps4

#define VecI2()	sgVecOps2->Get(1.0,0.0)
#define VecJ2()	sgVecOps2->Get(0.0,1.0)
#define VecI()	sgVecOps3->Get(1.0,0.0,0.0)
#define VecJ()	sgVecOps3->Get(0.0,1.0,0.0)
#define VecK()	sgVecOps3->Get(0.0,0.0,1.0)
#define VecI4()	sgVecOps4->Get(1.0,0.0,0.0,0.0)
#define VecJ4()	sgVecOps4->Get(0.0,1.0,0.0,0.0)
#define VecK4()	sgVecOps4->Get(0.0,0.0,1.0,0.0)
#define VecL4()	sgVecOps4->Get(0.0,0.0,0.0,1.0)
#define Vec3to2	SG_Vector3to2
#define Vec2to3	SG_Vector2to3
#define Vec3to4	SG_Vector3to4

#define VecZero2	sgVecOps2->Zero
#define VecGet2		sgVecOps2->Get
#define VecSet2		sgVecOps2->Set
#define VecCopy2	sgVecOps2->Copy
#define VecMirror2	sgVecOps2->Mirror
#define VecMirror2p	sgVecOps2->Mirrorp
#define VecLen2		sgVecOps2->Len
#define VecLen2p	sgVecOps2->Lenp
#define VecDot2		sgVecOps2->Dot
#define VecDot2p	sgVecOps2->Dotp
#define VecPerpDot2	sgVecOps2->PerpDot
#define VecPerpDot2p	sgVecOps2->PerpDotp
#define VecDistance2	sgVecOps2->Distance
#define VecDistance2p	sgVecOps2->Distancep
#define VecNorm2	sgVecOps2->Norm
#define VecNorm2p	sgVecOps2->Normp
#define VecNorm2v	sgVecOps2->Normv
#define VecScale2	sgVecOps2->Scale
#define VecScale2p	sgVecOps2->Scalep
#define VecScale2v	sgVecOps2->Scalev
#define VecAdd2		sgVecOps2->Add
#define VecAdd2p	sgVecOps2->Addp
#define VecAdd2v	sgVecOps2->Addv
#define VecAdd2n	sgVecOps2->Addn
#define VecSub2		sgVecOps2->Sub
#define VecSub2p	sgVecOps2->Subp
#define VecSub2v	sgVecOps2->Subv
#define VecSub2n	sgVecOps2->Subn
#define VecAvg2		sgVecOps2->Avg
#define VecAvg2p	sgVecOps2->Avgp
#define VecLERP2	sgVecOps2->LERP
#define VecLERP2p	sgVecOps2->LERPp
#define VecElemPow2	sgVecOps2->ElemPow
#define VecVecAngle2	sgVecOps2->VecAngle
#define VecRotate2	sgVecOps2->Rotate
#define VecRotate2v	sgVecOps2->Rotatev

#define VecZero		sgVecOps3->Zero
#define VecGet		sgVecOps3->Get
#define VecSet		sgVecOps3->Set
#define VecCopy		sgVecOps3->Copy
#define VecMirror	sgVecOps3->Mirror
#define VecMirrorp	sgVecOps3->Mirrorp
#define VecLen		sgVecOps3->Len
#define VecLenp		sgVecOps3->Lenp
#define VecDot		sgVecOps3->Dot
#define VecDotp		sgVecOps3->Dotp
#define VecDistance	sgVecOps3->Distance
#define VecDistancep	sgVecOps3->Distancep
#define VecNorm		sgVecOps3->Norm
#define VecNormp	sgVecOps3->Normp
#define VecNormv	sgVecOps3->Normv
#define VecCross	sgVecOps3->Cross
#define VecCrossp	sgVecOps3->Crossp
#define VecNormCross	sgVecOps3->NormCross
#define VecNormCrossp	sgVecOps3->NormCrossp
#define VecScale	sgVecOps3->Scale
#define VecScalep	sgVecOps3->Scalep
#define VecScalev	sgVecOps3->Scalev
#define VecAdd		sgVecOps3->Add
#define VecAddp		sgVecOps3->Addp
#define VecAddv		sgVecOps3->Addv
#define VecAddn		sgVecOps3->Addn
#define VecSub		sgVecOps3->Sub
#define VecSubp		sgVecOps3->Subp
#define VecSubv		sgVecOps3->Subv
#define VecSubn		sgVecOps3->Subn
#define VecAvg		sgVecOps3->Avg
#define VecAvgp		sgVecOps3->Avgp
#define VecLERP		sgVecOps3->LERP
#define VecLERPp	sgVecOps3->LERPp
#define VecElemPow	sgVecOps3->ElemPow
#define VecVecAngle	sgVecOps3->VecAngle
#define VecRotate	sgVecOps3->Rotate
#define VecRotatev	sgVecOps3->Rotatev
#define VecRotateQuat	sgVecOps3->RotateQuat
#define VecRotateI	sgVecOps3->RotateI
#define VecRotateJ	sgVecOps3->RotateJ
#define VecRotateK	sgVecOps3->RotateK

#define VecZero4	sgVecOps4->Zero
#define VecGet4		sgVecOps4->Get
#define VecSet4		sgVecOps4->Set
#define VecCopy4	sgVecOps4->Copy
#define VecMirror4	sgVecOps4->Mirror
#define VecMirror4p	sgVecOps4->Mirrorp
#define VecLen4		sgVecOps4->Len
#define VecLen4p	sgVecOps4->Lenp
#define VecDot4		sgVecOps4->Dot
#define VecDot4p	sgVecOps4->Dotp
#define VecDistance4	sgVecOps4->Distance
#define VecDistance4p	sgVecOps4->Distancep
#define VecNorm4	sgVecOps4->Norm
#define VecNorm4p	sgVecOps4->Normp
#define VecNorm4v	sgVecOps4->Norm4
#define VecCross4	sgVecOps4->Cross
#define VecCross4p	sgVecOps4->Crossp
#define VecNormCross4	sgVecOps4->NormCross
#define VecNormCross4p	sgVecOps4->NormCrossp
#define VecScale4	sgVecOps4->Scale
#define VecScale4p	sgVecOps4->Scalep
#define VecScale4v	sgVecOps4->Scalev
#define VecAdd4		sgVecOps4->Add
#define VecAdd4p	sgVecOps4->Addp
#define VecAdd4v	sgVecOps4->Addv
#define VecAdd4n	sgVecOps4->Addn
#define VecSub4		sgVecOps4->Sub
#define VecSub4p	sgVecOps4->Subp
#define VecSub4v	sgVecOps4->Subv
#define VecSub4n	sgVecOps4->Subn
#define VecAvg4		sgVecOps4->Avg
#define VecAvg4p	sgVecOps4->Avgp
#define VecLERP4	sgVecOps4->LERP
#define VecLERP4p	sgVecOps4->LERPp
#define VecElemPow4	sgVecOps4->ElemPow
#define VecVecAngle4	sgVecOps4->VecAngle
#define VecRotate4	sgVecOps4->Rotate
#define VecRotate4v	sgVecOps4->Rotatev

#endif /* _AGAR_INTERNAL or _USE_AGAR_MATH */
