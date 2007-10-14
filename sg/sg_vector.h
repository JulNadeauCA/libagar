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
SG_Vector2 SG_ReadVector2(AG_Netbuf *);
SG_Vector  SG_ReadVector (AG_Netbuf *);
SG_Vector4 SG_ReadVector4(AG_Netbuf *);
void	   SG_ReadVector2v(AG_Netbuf *, SG_Vector2 *);
void	   SG_ReadVectorv (AG_Netbuf *, SG_Vector  *);
void	   SG_ReadVector4v(AG_Netbuf *, SG_Vector4 *);
void	   SG_WriteVector2(AG_Netbuf *, SG_Vector2 *);
void	   SG_WriteVector (AG_Netbuf *, SG_Vector  *);
void	   SG_WriteVector4(AG_Netbuf *, SG_Vector4 *);
SG_Vector2 SG_ReadVectorf2(AG_Netbuf *);
SG_Vector  SG_ReadVectorf (AG_Netbuf *);
SG_Vector4 SG_ReadVectorf4(AG_Netbuf *);
void	   SG_ReadVectorf2v(AG_Netbuf *, SG_Vector2 *);
void	   SG_ReadVectorfv (AG_Netbuf *, SG_Vector  *);
void	   SG_ReadVectorf4v(AG_Netbuf *, SG_Vector4 *);
void	   SG_WriteVectorf2(AG_Netbuf *, SG_Vector2 *);
void	   SG_WriteVectorf (AG_Netbuf *, SG_Vector *);
void	   SG_WriteVectorf4(AG_Netbuf *, SG_Vector4 *);

SG_Vector2 SG_RealvToVector2(const SG_Real *);
SG_Vector  SG_RealvToVector(const SG_Real *);
SG_Vector4 SG_RealvToVector4(const SG_Real *);
SG_Vector2 SG_Vector3to2(SG_Vector);
SG_Vector  SG_Vector2to3(SG_Vector2);
SG_Vector4 SG_Vector3to4(SG_Vector);
__END_DECLS

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)
#define Vec2 sgVecOps2
#define Vec3 sgVecOps3
#define Vec4 sgVecOps4

#define VecI2()		Vec2->Get(1.0,0.0)
#define VecJ2()		Vec2->Get(0.0,1.0)
#define VecI()		Vec3->Get(1.0,0.0,0.0)
#define VecJ()		Vec3->Get(0.0,1.0,0.0)
#define VecK()		Vec3->Get(0.0,0.0,1.0)
#define VecI4()		Vec4->Get(1.0,0.0,0.0,0.0)
#define VecJ4()		Vec4->Get(0.0,1.0,0.0,0.0)
#define VecK4()		Vec4->Get(0.0,0.0,1.0,0.0)
#define VecL4()		Vec4->Get(0.0,0.0,0.0,1.0)
#define Vec3to2		SG_Vector3to2
#define Vec2to3		SG_Vector2to3
#define Vec3to4		SG_Vector3to4

#define VecZero2	Vec2->Zero
#define VecGet2		Vec2->Get
#define VecSet2		Vec2->Set
#define VecCopy2	Vec2->Copy
#define VecMirror2	Vec2->Mirror
#define VecMirror2p	Vec2->Mirrorp
#define VecLen2		Vec2->Len
#define VecLen2p	Vec2->Lenp
#define VecDot2		Vec2->Dot
#define VecDot2p	Vec2->Dotp
#define VecPerpDot2	Vec2->PerpDot
#define VecPerpDot2p	Vec2->PerpDotp
#define VecDistance2	Vec2->Distance
#define VecDistance2p	Vec2->Distancep
#define VecNorm2	Vec2->Norm
#define VecNorm2p	Vec2->Normp
#define VecNorm2v	Vec2->Normv
#define VecScale2	Vec2->Scale
#define VecScale2p	Vec2->Scalep
#define VecScale2v	Vec2->Scalev
#define VecAdd2		Vec2->Add
#define VecAdd2p	Vec2->Addp
#define VecAdd2v	Vec2->Addv
#define VecAdd2n	Vec2->Addn
#define VecSub2		Vec2->Sub
#define VecSub2p	Vec2->Subp
#define VecSub2v	Vec2->Subv
#define VecSub2n	Vec2->Subn
#define VecAvg2		Vec2->Avg
#define VecAvg2p	Vec2->Avgp
#define VecLERP2	Vec2->LERP
#define VecLERP2p	Vec2->LERPp
#define VecElemPow2	Vec2->ElemPow
#define VecVecAngle2	Vec2->VecAngle
#define VecRotate2	Vec2->Rotate
#define VecRotate2v	Vec2->Rotatev

#if defined(INLINE_SSE) || defined(INLINE_SSE2) || defined(INLINE_SSE3)

#define VecZero		SG_VectorZero3_SSE
#define VecGet		SG_VectorGet3_SSE
#define VecSet		SG_VectorSet3_FPU
#define VecCopy		SG_VectorCopy3_FPU
#define VecMirror	SG_VectorMirror3_SSE
#define VecMirrorp	SG_VectorMirror3p_SSE
#define VecLen		SG_VectorLen3_FPU
#define VecLenp		SG_VectorLen3p_FPU
#ifdef INLINE_SSE3
# define VecDot		SG_VectorDot3_SSE3
# define VecDotp	SG_VectorDot3p_SSE3
#else
# define VecDot		SG_VectorDot3_FPU
# define VecDotp	SG_VectorDot3p_FPU
#endif
#define VecDistance	SG_VectorDistance3_SSE
#define VecDistancep	SG_VectorDistance3p_SSE
#define VecNorm		SG_VectorNorm3_SSE
#define VecNormp	SG_VectorNorm3p_SSE
#define VecNormv	SG_VectorNorm3v_FPU
#define VecCross	SG_VectorCross3_FPU
#define VecCrossp	SG_VectorCross3p_FPU
#define VecNormCross	SG_VectorNormCross3_FPU
#define VecNormCrossp	SG_VectorNormCross3p_FPU
#define VecScale	SG_VectorScale3_SSE
#define VecScalep	SG_VectorScale3p_SSE
#define VecScalev	SG_VectorScale3v_FPU
#define VecAdd		SG_VectorAdd3_SSE
#define VecAddp		SG_VectorAdd3p_SSE
#define VecAddv		SG_VectorAdd3v_FPU
#define VecAddn		SG_VectorAdd3n_SSE
#define VecSub		SG_VectorSub3_SSE
#define VecSubp		SG_VectorSub3p_SSE
#define VecSubv		SG_VectorSub3v_FPU
#define VecSubn		SG_VectorSub3n_SSE
#define VecAvg		SG_VectorAvg3_SSE
#define VecAvgp		SG_VectorAvg3p_SSE
#define VecLERP		SG_VectorLERP3_SSE
#define VecLERPp	SG_VectorLERP3p_SSE
#define VecElemPow	SG_VectorElemPow3_SSE
#define VecVecAngle	SG_VectorVecAngle3_SSE
#define VecRotate	SG_VectorRotate3_SSE
#define VecRotatev	SG_VectorRotate3v_SSE
#define VecRotateQuat	SG_VectorRotateQuat3_SSE
#define VecRotateI	SG_VectorRotateI3_SSE
#define VecRotateJ	SG_VectorRotateJ3_SSE
#define VecRotateK	SG_VectorRotateK3_SSE

#else  /* !INLINE_SSE[123] */

#define VecZero		Vec3->Zero
#define VecGet		Vec3->Get
#define VecSet		Vec3->Set
#define VecCopy		Vec3->Copy
#define VecMirror	Vec3->Mirror
#define VecMirrorp	Vec3->Mirrorp
#define VecLen		Vec3->Len
#define VecLenp		Vec3->Lenp
#define VecDot		Vec3->Dot
#define VecDotp		Vec3->Dotp
#define VecDistance	Vec3->Distance
#define VecDistancep	Vec3->Distancep
#define VecNorm		Vec3->Norm
#define VecNormp	Vec3->Normp
#define VecNormv	Vec3->Normv
#define VecCross	Vec3->Cross
#define VecCrossp	Vec3->Crossp
#define VecNormCross	Vec3->NormCross
#define VecNormCrossp	Vec3->NormCrossp
#define VecScale	Vec3->Scale
#define VecScalep	Vec3->Scalep
#define VecScalev	Vec3->Scalev
#define VecAdd		Vec3->Add
#define VecAddp		Vec3->Addp
#define VecAddv		Vec3->Addv
#define VecAddn		Vec3->Addn
#define VecSub		Vec3->Sub
#define VecSubp		Vec3->Subp
#define VecSubv		Vec3->Subv
#define VecSubn		Vec3->Subn
#define VecAvg		Vec3->Avg
#define VecAvgp		Vec3->Avgp
#define VecLERP		Vec3->LERP
#define VecLERPp	Vec3->LERPp
#define VecElemPow	Vec3->ElemPow
#define VecVecAngle	Vec3->VecAngle
#define VecRotate	Vec3->Rotate
#define VecRotatev	Vec3->Rotatev
#define VecRotateQuat	Vec3->RotateQuat
#define VecRotateI	Vec3->RotateI
#define VecRotateJ	Vec3->RotateJ
#define VecRotateK	Vec3->RotateK

#endif /* INLINE_SSE[123] */

#define VecZero4	Vec4->Zero
#define VecGet4		Vec4->Get
#define VecSet4		Vec4->Set
#define VecCopy4	Vec4->Copy
#define VecMirror4	Vec4->Mirror
#define VecMirror4p	Vec4->Mirrorp
#define VecLen4		Vec4->Len
#define VecLen4p	Vec4->Lenp
#define VecDot4		Vec4->Dot
#define VecDot4p	Vec4->Dotp
#define VecDistance4	Vec4->Distance
#define VecDistance4p	Vec4->Distancep
#define VecNorm4	Vec4->Norm
#define VecNorm4p	Vec4->Normp
#define VecNorm4v	Vec4->Norm4
#define VecCross4	Vec4->Cross
#define VecCross4p	Vec4->Crossp
#define VecNormCross4	Vec4->NormCross
#define VecNormCross4p	Vec4->NormCrossp
#define VecScale4	Vec4->Scale
#define VecScale4p	Vec4->Scalep
#define VecScale4v	Vec4->Scalev
#define VecAdd4		Vec4->Add
#define VecAdd4p	Vec4->Addp
#define VecAdd4v	Vec4->Addv
#define VecAdd4n	Vec4->Addn
#define VecSub4		Vec4->Sub
#define VecSub4p	Vec4->Subp
#define VecSub4v	Vec4->Subv
#define VecSub4n	Vec4->Subn
#define VecAvg4		Vec4->Avg
#define VecAvg4p	Vec4->Avgp
#define VecLERP4	Vec4->LERP
#define VecLERP4p	Vec4->LERPp
#define VecElemPow4	Vec4->ElemPow
#define VecVecAngle4	Vec4->VecAngle
#define VecRotate4	Vec4->Rotate
#define VecRotate4v	Vec4->Rotatev

#endif /* _AGAR_INTERNAL or _USE_AGAR_MATH */
