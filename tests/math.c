/*	Public domain	*/

/*
 * This program tests the Agar math library (ag_math).
 */

#include <agar/config/ag_enable_string.h>

#include "agartest.h"

#include <agar/math/m.h>
#include <agar/math/m_gui.h>

#include <agar/config/have_altivec.h>
#include <agar/config/have_altivec_h.h>
#include <agar/config/inline_altivec.h>
#include <agar/config/have_sse.h>
#include <agar/config/have_sse2.h>
#include <agar/config/have_sse3.h>
#include <agar/config/inline_sse.h>
#include <agar/config/quad_precision.h>
#include <agar/config/double_precision.h>
#include <agar/config/single_precision.h>

#include "config/have_rand48.h"

#include <string.h>
#include <stdlib.h>

#define NREALS 10000
#define NVECTORS 1000
#define NMATRICES 100

typedef struct {
	AG_TestInstance _inherit;
	M_Real r[NREALS];
	M_Vector2 v2[NVECTORS];
	M_Vector3 v3[NVECTORS];
	M_Vector4 v4[NVECTORS];
	M_Matrix44 m44[NMATRICES];
	int curReal, curVec, curMat;
} MyTestInstance;

M_Real realJunk = 0.0;

static __inline__ M_Real
RandomReal(MyTestInstance *ti)
{
	if (ti->curReal+1 >= NREALS) { ti->curReal = 0; }
	return (ti->r[ti->curReal++]);
}
static __inline__ M_Vector3
RandomVector3(MyTestInstance *ti)
{
	if (ti->curVec+1 >= NVECTORS) { ti->curVec = 0; }
	return (ti->v3[ti->curVec++]);
}
static __inline__ M_Vector4
RandomVector4(MyTestInstance *ti)
{
	if (ti->curVec+1 >= NVECTORS) { ti->curVec = 0; }
	return (ti->v4[ti->curVec++]);
}
static __inline__ M_Matrix44
RandomMatrix44(MyTestInstance *ti)
{
	if (ti->curMat+1 >= NMATRICES) { ti->curMat = 0; }
	return (ti->m44[ti->curMat++]);
}

#include "math_vector3.h"
#include "math_matrix44.h"

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;
	int i, j;

	M_InitSubsystem();

	ti->curReal = 0;
	ti->curVec = 0;
	ti->curMat = 0;
	for (i = 0; i < NREALS ; i++) {
#ifdef HAVE_RAND48
		ti->r[i] = (M_Real)drand48();
#else
		ti->r[i] = (M_Real)i;
#endif
	}
	for (i = 0; i < NVECTORS; i++) {
#ifdef HAVE_RAND48
		ti->v2[i] = M_VECTOR2((M_Real)drand48(), (M_Real)drand48());
# ifdef HAVE_SSE
		ti->v3[i] = M_VECTOR3((float)drand48(), (float)drand48(),
		                      (float)drand48());
		ti->v4[i] = M_VECTOR4((float)drand48(), (float)drand48(),
		                      (float)drand48(), (float)drand48());
# else
		ti->v3[i] = M_VECTOR3((M_Real)drand48(), (M_Real)drand48(),
		                      (M_Real)drand48());
		ti->v4[i] = M_VECTOR4((M_Real)drand48(), (M_Real)drand48(),
		                      (M_Real)drand48(), (M_Real)drand48());
# endif
#else
		ti->v2[i] = M_VECTOR2((M_Real)(i+1), (M_Real)(i*i));
# ifdef HAVE_SSE
		ti->v3[i] = M_VECTOR3((float)(i+1), (float)(i*i), (float)(i*i*i));
		ti->v4[i] = M_VECTOR4((float)(i+1), (float)(i*i), (float)(i*i*i),
		                      (float)(i*i*i));
# else
		ti->v3[i] = M_VECTOR3((M_Real)(i+1), (M_Real)(i*i),
		                      (M_Real)(i*i*i));
		ti->v4[i] = M_VECTOR4((M_Real)(i+1), (M_Real)(i*i),
		                      (M_Real)(i*i*i), (M_Real)(i*i*i));
# endif
#endif
	}
	for (i = 0; i < NMATRICES; i++) {
		double rands[16];
#ifdef HAVE_RAND48
		for (j = 0; j < 16; j++)
			rands[j] = drand48();
#else
		for (j = 0; j < 16; j++)
			rands[j] = (double)(i*j + i + i*i + i*i*j + 1);
#endif
		M_MatFromDoubles44(&ti->m44[i], rands);
	}
	return (0);
}

static void
TestComplex(AG_TestInstance *ti)
{
	M_Complex C1, C2, C1mi, C1plusC2, C1multC2;
	M_Real C1mod, C1arg;

	C1.r = 1.0;
	C1.i = 1.1;
	C2.r = 10.0;
	C2.i = 2.2;
	C1mi = M_ComplexMultiplicativeInverse(C1);
	C1mod = M_ComplexModulus(C1);
	C1arg = M_ComplexArg(C1);
	C1plusC2 = M_ComplexAdd(C1, C2);
	C1multC2 = M_ComplexMult(C1, C2);

	TestMsgS(ti, AG_Printf("\tC1=%[C], C2=%[C]", &C1, &C2));
	TestMsgS(ti, AG_Printf("\tC1 mult. inverse = %[C]", &C1mi));
	TestMsgS(ti, AG_Printf("\tC1 modulus = %[R]", &C1mod));
	TestMsgS(ti, AG_Printf("\tC1 argument = %[R]", &C1arg));
	TestMsgS(ti, AG_Printf("\tC1+C2 = %[C]", &C1plusC2));
	TestMsgS(ti, AG_Printf("\tC1*C2 = %[C]", &C1multC2));
}

static void
TestVector(AG_TestInstance *ti)
{
	M_Vector *a, *b, *AplusB, *AsubB, *aNorm, *bNorm, *aLERP, *aPow2, *a2,
	    *b2;
	M_Real aLen, bLen, abDist, AdotB;
	M_Real someReal = 1.234567, someReal2 = 2.345;

	a = M_VecNew(4);
	a->v[0] = 1.1;	a->v[1] = 2.2;	a->v[2] = 3.3;	a->v[3] = 4.4;
	b = M_VecNew(4);
	b->v[0] = 10.0;	b->v[1] = 20.0;	b->v[2] = 30.0;	b->v[3] = 40.0;

	aLen = M_VecLen(a);		bLen = M_VecLen(b);
	a2 = M_VecScale(a, 2.0);	b2 = M_VecScale(b, 2.0);
	AplusB = M_VecAdd(a, b);	AsubB = M_VecSub(a, b);
	aNorm = M_VecNorm(a);		bNorm = M_VecNorm(b);
	aLERP = M_VecLERP(a, b, 0.5);
	aPow2 = M_VecElemPow(a, 2.0);
	abDist = M_VecDistance(a, b);
	AdotB = M_VecDot(a, b);

	TestMsgS(ti, AG_Printf("\tReal=%[R], Real2=%[R]", &someReal, &someReal2));
	TestMsgS(ti, AG_Printf("\ta=%[V], b=%[V]", a, b));
	TestMsgS(ti, AG_Printf("\t2a=%[V], 2b=%[V]", a2, b2));
	TestMsgS(ti, AG_Printf("\tlen(a)=%[R], len(b)=%[R]", &aLen, &bLen));
	TestMsgS(ti, AG_Printf("\tnorm(a)=%[V], norm(b)=%[V]", aNorm, bNorm));
	TestMsgS(ti, AG_Printf("\ta+b=%[V], a-b=%[V]", AplusB, AsubB));
	TestMsgS(ti, AG_Printf("\ta dot b=%[R]", &AdotB));
	TestMsgS(ti, AG_Printf("\tdist(a,b)=%[R]", &abDist));
	TestMsgS(ti, AG_Printf("\tlerp(a,b,1/2)=%[V]", aLERP));
	TestMsgS(ti, AG_Printf("\tpow2(a)=%[V]", aPow2));

	M_VecFree(a);		M_VecFree(b);
	M_VecFree(a2);		M_VecFree(b2);
	M_VecFree(AplusB);	M_VecFree(AsubB);
	M_VecFree(aNorm);	M_VecFree(bNorm);
	M_VecFree(aLERP);
	M_VecFree(aPow2);
}

static void
TestVector3(AG_TestInstance *ti)
{
	M_Vector3 a = M_VECTOR3(1.1, 2.8, 3.7);
	M_Vector3 b = M_VECTOR3(15.1, 52.8, 35.0);
	M_Real aLen = M_VecLen3(a), bLen = M_VecLen3(b);
	M_Vector3 Anorm = M_VecNorm3(a), Bnorm = M_VecNorm3(b);
	M_Real AdotB = M_VecDot3(a, b);
	M_Vector3 AcrossB = M_VecCross3(a, b);
	M_Vector3 AnormcrossB = M_VecNormCross3(a, b);
	M_Real AdistB = M_VecDistance3(a, b);
	M_Vector3 AavgB = M_VecAvg3(a, b);
	
	TestMsgS(ti, AG_Printf("\ta=%[V3], b=%[V3]", &a, &b));
	TestMsgS(ti, AG_Printf("\tlen(a)=%[R], len(b)=%[R]", &aLen, &bLen));
	TestMsgS(ti, AG_Printf("\tnorm(a)=%[V3], norm(b)=%[V3]", &Anorm, &Bnorm));
	TestMsgS(ti, AG_Printf("\ta dot b=%[R]", &AdotB));
	TestMsgS(ti, AG_Printf("\ta x b=%[V3]", &AcrossB));
	TestMsgS(ti, AG_Printf("\tnorm(a x b)=%[V3]", &AnormcrossB));
	TestMsgS(ti, AG_Printf("\tdist(a,b)=%[R]", &AdistB));
	TestMsgS(ti, AG_Printf("\tavg(a,b)=%[V3]", &AavgB));
}

static void
TestMatrix(AG_TestInstance *ti)
{
	M_Matrix *M;

	M = M_New(5,5);
	M_SetIdentity(M);

	TestMsgS(ti, AG_Printf("\tM=%[M]", M));
	
	M_Free(M);
}

static void
TestMatrix44(AG_TestInstance *ti)
{
	M_Matrix44 A, B, AB, ABinv, ABt, Rot, UniScale;

	M_MatIdentity44v(&A);
	M_MatIdentity44v(&B);
	A.m[0][1] = 2.0;	A.m[1][0] = 3.0;	A.m[0][2] = 4.0;
	A.m[2][0] = 5.0;	A.m[0][3] = 6.0;	A.m[3][0] = 7.0;
	B.m[1][1] = 10.0;	B.m[1][2] = 0.1;	B.m[2][1] = 0.2;
	AB = M_MatMult44(B, A);
	ABinv = M_MatInvert44(AB);
	ABt = M_MatTranspose44(AB);

	Rot = M_MatIdentity44();
	UniScale = M_MatIdentity44();
	M_MatRotateAxis44(&Rot, M_Radians(33.0),
	    M_VecNorm3(M_VECTOR3(11.6, 4.51, 8.5)));
	M_MatUniScale44(&UniScale, 6.66);

	TestMsgS(ti, AG_Printf("\tA=%[M44]", &A));
	TestMsgS(ti, AG_Printf("\tB=%[M44]", &B));
	TestMsgS(ti, AG_Printf("\tAB=%[M44]", &AB));
	TestMsgS(ti, AG_Printf("\tinv(AB)=%[M44]", &ABinv));
	TestMsgS(ti, AG_Printf("\ttranspose(AB)= %[M44]", &ABt));
	TestMsgS(ti, AG_Printf("\tRot=%[M44]", &Rot));
	TestMsgS(ti, AG_Printf("\tUniScale=%[M44]", &UniScale));
}

static int
Test(void *obj)
{
	AG_TestInstance *ti = obj;
	const M_VectorOps3 *prevVecOps3 = mVecOps3;
	const M_MatrixOps44 *prevMatOps44 = mMatOps44;

	TestMsg(ti, "Agar-Math settings:");

	/* Standard Precision */
#if defined(QUAD_PRECISION)
	TestMsg(ti, "\tPrecision: Quad");
#elif defined(DOUBLE_PRECISION)
	TestMsg(ti, "\tPrecision: Double");
#elif defined(SINGLE_PRECISION)
	TestMsg(ti, "\tPrecision: Single");
#endif

	/* AltiVec support */
#if defined(HAVE_ALTIVEC)
	TestMsg(ti, "\tAltiVec support is available");
# if defined(INLINE_ALTIVEC)
	TestMsg(ti, "\tAltiVec extensions are compiled INLINE");
# endif
#endif

	/* SSE support */
#if defined(HAVE_SSE3)
	TestMsg(ti, "\tSSE3 extensions are available");
#elif defined(HAVE_SSE2)
	TestMsg(ti, "\tSSE2 extensions are available");
#elif defined(HAVE_SSE)
	TestMsg(ti, "\tSSE extensions are available");
#endif
#if defined(INLINE_SSE)
	TestMsg(ti, "\tSSE extensions are compiled INLINE");
#endif

	TestMsg(ti, "\tM_Vector engine: %s", mVecOps->name);
	TestMsg(ti, "\tM_Vector2 engine: %s", mVecOps2->name);
	TestMsg(ti, "\tM_Vector3 engine: %s", mVecOps3->name);
	TestMsg(ti, "\tM_Vector4 engine: %s", mVecOps4->name);
	TestMsg(ti, "\tM_Matrix engine: %s", mMatOps->name);
	TestMsg(ti, "\tM_Matrix44 engine: %s", mMatOps44->name);
	TestMsgS(ti, "");
	
	mVecOps3 = &mVecOps3_FPU;
	mMatOps44 = &mMatOps44_FPU;
	TestMsg(ti, "M_Complex Test (FPU):");	TestComplex(ti);
	TestMsg(ti, "M_Vector Test (FPU):");	TestVector(ti);
	TestMsg(ti, "M_Matrix Test (FPU):");	TestMatrix(ti);
	TestMsg(ti, "M_Vector3 Test (FPU):");	TestVector3(ti);
	TestMsg(ti, "M_Matrix44 Test (FPU):");	TestMatrix44(ti);

#if defined(HAVE_SSE)
	mVecOps3 = &mVecOps3_SSE;
	mMatOps44 = &mMatOps44_SSE;
	TestMsgS(ti, "");
	TestMsgS(ti, "M_Complex Test (SSE):");	TestComplex(ti);
	TestMsgS(ti, "M_Vector Test (SSE):");	TestVector(ti);
	TestMsgS(ti, "M_Matrix Test (SSE):");	TestMatrix(ti);
	TestMsgS(ti, "M_Vector3 Test (SSE):");	TestVector3(ti);
	TestMsgS(ti, "M_Matrix44 Test (SSE):");	TestMatrix44(ti);
#endif /* HAVE_SSE */

	mMatOps44 = prevMatOps44;
	mVecOps3 = prevVecOps3;
	return (0);
}

static int
Bench(void *obj)
{
	AG_TestInstance *ti = obj;
	const M_VectorOps3 *prevVecOps3 = mVecOps3;
	const M_MatrixOps44 *prevMatOps44 = mMatOps44;

#if defined(INLINE_SSE)
	TestMsg(ti, "M_Vector3 Microbenchmark (INLINE SSE):");
	TestExecBenchmark(obj, &mathBenchVector3);
#else /* !INLINE_SSE */
	mVecOps3 = &mVecOps3_FPU;
	mMatOps44 = &mMatOps44_FPU;
	TestMsg(ti, "M_Vector3 Microbenchmark (FPU):");
	TestExecBenchmark(obj, &mathBenchVector3);
	TestMsg(ti, "M_Matrix44 Microbenchmark (FPU):");
	TestExecBenchmark(obj, &mathBenchMatrix44);
# ifdef HAVE_SSE
	mVecOps3 = &mVecOps3_SSE;
	mMatOps44 = &mMatOps44_SSE;
	TestMsg(ti, "M_Vector3 Microbenchmark (SSE):");
	TestExecBenchmark(obj, &mathBenchVector3);
	TestMsg(ti, "M_Matrix44 Microbenchmark (SSE):");
	TestExecBenchmark(obj, &mathBenchMatrix44);
# endif
#endif /* !INLINE_SSE */

	mMatOps44 = prevMatOps44;
	mVecOps3 = prevVecOps3;
	return (0);
}

const AG_TestCase mathTest = {
	"math",
	N_("Test the ag_math library"),
	"1.6.0",
	0,
	sizeof(MyTestInstance),
	Init,
	NULL,	/* destroy */
	Test,
	NULL,	/* testGUI */
	Bench
};
