/*	Public domain	*/

/*
 * This program tests the Agar math library (ag_math).
 */

#include "agartest.h"

#include <agar/math.h>

#include <agar/config/have_altivec.h>
#include <agar/config/have_altivec_h.h>
#include <agar/config/inline_altivec.h>
#include <agar/config/have_sse.h>
#include <agar/config/have_sse2.h>
#include <agar/config/have_sse3.h>
#include <agar/config/inline_sse.h>
#include <agar/config/inline_sse2.h>
#include <agar/config/inline_sse3.h>
#include <agar/config/quad_precision.h>
#include <agar/config/double_precision.h>
#include <agar/config/single_precision.h>

#include "config/have_rand48.h"

#include <string.h>
#include <stdlib.h>

#define RANDOMIZE() if (++cur >= nVecs) { cur = 0; }

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

static __inline__ M_Real
RandomReal(MyTestInstance *ti)
{
	if (ti->curReal+1 >= NREALS) { ti->curReal = 0; }
	return (ti->r[ti->curReal++]);
}
static __inline__ M_Vector2
RandomVector2(MyTestInstance *ti)
{
	if (ti->curVec+1 >= NVECTORS) { ti->curVec = 0; }
	return (ti->v2[ti->curVec++]);
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

#include "math_bench.h"

static int
Init(void *obj)
{
	MyTestInstance *ti = obj;
	int i, j;

	M_InitSubsystem();

	Verbose("Generating random vectors and matrices...\n");
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
	Verbose("Done.\n");
	return (0);
}

static void
Destroy(void *obj)
{
	M_DestroySubsystem();
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

	TestMsgS(ti, "Testing complex number routines:");
	TestMsgS(ti, AG_Printf("C1=%[C], C2=%[C]", &C1, &C2));
	TestMsgS(ti, AG_Printf("C1 mult. inverse = %[C]", &C1mi));
	TestMsgS(ti, AG_Printf("C1 modulus = %[R]", &C1mod));
	TestMsgS(ti, AG_Printf("C1 argument = %[R]", &C1arg));
	TestMsgS(ti, AG_Printf("C1+C2 = %[C]", &C1plusC2));
	TestMsgS(ti, AG_Printf("C1*C2 = %[C]", &C1multC2));
}

static void
TestVector(AG_TestInstance *ti)
{
	M_Vector *a, *b, *AplusB, *AsubB, *aNorm, *aLERP, *aPow2, *a100;
	M_Real aLen, bLen, abDist, AdotB;
	M_Real someReal = 1.234567, someReal2 = 2.345;

	a = M_VecNew(4);
	a->v[0] = 1.1;	a->v[1] = 2.2;	a->v[2] = 3.3;	a->v[3] = 4.4;
	b = M_VecNew(4);
	b->v[0] = 10.0;	b->v[1] = 20.0;	b->v[2] = 30.0;	b->v[3] = 40.0;

	AplusB = M_VecAdd(a, b);
	AsubB = M_VecSub(a, b);
	aNorm = M_VecNorm(a);
	aLERP = M_VecLERP(a, b, 0.5);
	aPow2 = M_VecElemPow(a, 2.0);
	a100 = M_VecScale(a, 100.0);
	aLen = M_VecLen(a);
	bLen = M_VecLen(b);
	abDist = M_VecDistance(a, b);
	AdotB = M_VecDot(a, b);

	TestMsgS(ti, "Testing M_Vector routines:");
	TestMsgS(ti, AG_Printf("Real=%[R], Real2=%[R]", &someReal, &someReal2));
	TestMsgS(ti, AG_Printf("a=%[V], b=%[V]", a, b));
	TestMsgS(ti, AG_Printf("(a*100)=%[V]", a100));
	TestMsgS(ti, AG_Printf("(a+b)=%[V]", AplusB));
	TestMsgS(ti, AG_Printf("(a-b)=%[V]", AsubB));
	TestMsgS(ti, AG_Printf("(a dot b)=%[R]", &AdotB));
	TestMsgS(ti, AG_Printf("len(a)=%[R], len(b)=%[R]", &aLen, &bLen));
	TestMsgS(ti, AG_Printf("dist(a,b)=%[R]", &abDist));
	TestMsgS(ti, AG_Printf("norm(a)=%[V]", aNorm));
	TestMsgS(ti, AG_Printf("lerp(a,b,1/2)=%[V]", aLERP));
	TestMsgS(ti, AG_Printf("pow2(a)=%[V]", aPow2));

	M_VecFree(a); M_VecFree(b);
	M_VecFree(AplusB);
	M_VecFree(aNorm);
	M_VecFree(aLERP);
	M_VecFree(aPow2);
	M_VecFree(a100);
}

static void
TestVector3(AG_TestInstance *ti)
{
	M_Vector3 a = M_VECTOR3(1.1, 2.2, 3.3);
	M_Vector3 b = M_VECTOR3(10.0, 20.0, 30.0);
	M_Real AdotB = M_VecDot3(a, b);
	
	TestMsgS(ti, "Testing M_Vector3 routines:");
	TestMsgS(ti, AG_Printf("a=%[V3], b=%[V3]", &a, &b));
	TestMsgS(ti, AG_Printf("(a dot b)=%[R]", &AdotB));
}

static void
TestMatrix(AG_TestInstance *ti)
{
	M_Matrix *M;

	M = M_New(5,5);
	M_SetIdentity(M);

	TestMsgS(ti, "Testing M_Matrix routines:");
	TestMsgS(ti, AG_Printf("M=%[M]", M));
	
	M_Free(M);
}

static void
TestMatrix44(AG_TestInstance *ti)
{
	M_Matrix44 A, B, BA, BAinv;

	M_MatIdentity44v(&A);
	M_MatIdentity44v(&B);
	A.m[0][1] = 3.33;
	A.m[1][0] = 2.0;
	B.m[1][1] = 10.0;
	BA = M_MatMult44(B, A);
	BAinv = M_MatInvert44(BA);

	TestMsgS(ti, "Testing M_Matrix44 routines:");
	TestMsgS(ti, AG_Printf("A=%[M44]", &A));
	TestMsgS(ti, AG_Printf("B=%[M44]", &B));
	TestMsgS(ti, AG_Printf("BA=%[M44]", &BA));
	TestMsgS(ti, AG_Printf("BA inverse = %[M44]", &BAinv));
}

static int
Test(void *obj)
{
	AG_TestInstance *ti = obj;

	/*
	 * Display build settings
	 */
	TestMsg(ti, "Agar-Math compilation settings:");
#if defined(QUAD_PRECISION)
	TestMsg(ti, "\tPrecision: Quad");
#elif defined(DOUBLE_PRECISION)
	TestMsg(ti, "\tPrecision: Double");
#elif defined(SINGLE_PRECISION)
	TestMsg(ti, "\tPrecision: Single");
#endif
#if defined(HAVE_ALTIVEC)
	TestMsg(ti, "\tAltiVec support is available");
# if defined(INLINE_ALTIVEC)
	TestMsg(ti, "\tAltiVec extensions are compiled inline");
# endif
#endif
#if defined(HAVE_SSE3)
	TestMsg(ti, "\tSSE3 extensions are available");
# if defined(INLINE_SSE3)
	TestMsg(ti, "\tSSE3 extensions are compiled inline");
# endif
#elif defined(HAVE_SSE2)
	TestMsg(ti, "\tSSE2 extensions are available");
# if defined(INLINE_SSE2)
	TestMsg(ti, "\tSSE2 extensions are compiled inline");
# endif
#elif defined(HAVE_SSE)
	TestMsg(ti, "\tSSE extensions are available");
# if defined(INLINE_SSE)
	TestMsg(ti, "\tSSE extensions are compiled inline");
# endif
#endif

	/*
	 * Display runtime settings
	 */
	TestMsg(ti, "Agar-Math runtime settings:");
	TestMsg(ti, "\tM_Vector engine: %s", mVecOps->name);
	TestMsg(ti, "\tM_Vector2 engine: %s", mVecOps2->name);
	TestMsg(ti, "\tM_Vector3 engine: %s", mVecOps3->name);
	TestMsg(ti, "\tM_Vector4 engine: %s", mVecOps4->name);
	TestMsg(ti, "\tM_Matrix engine: %s", mMatOps->name);
	TestMsg(ti, "\tM_Matrix44 engine: %s", mMatOps44->name);

	TestComplex(ti);
	TestVector(ti);
	TestVector3(ti);
	TestMatrix(ti);
	TestMatrix44(ti);

	return (0);
}

static int
Bench(void *obj)
{
	AG_TestInstance *ti = obj;
	const M_VectorOps3 *prevVecOps3 = mVecOps3;

#if defined(INLINE_SSE) || defined(INLINE_SSE2) || defined(INLINE_SSE3)
	TestMsg(ti, "M_Vector3 Microbenchmark (Inline SSE):");
	TestExecBenchmark(obj, &mathBenchVector3);
#else
	TestMsg(ti, "M_Vector3 Microbenchmark (FPU):");
	mVecOps3 = &mVecOps3_FPU;
	TestExecBenchmark(obj, &mathBenchVector3);
# if defined(HAVE_SSE3)
	TestMsg(ti, "M_Vector3 Microbenchmark (SSE3):");
	mVecOps3 = &mVecOps3_SSE3;
	TestExecBenchmark(obj, &mathBenchVector3);
# elif defined(HAVE_SSE2)
	TestMsg(ti, "M_Vector3 Microbenchmark (SSE2):");
	mVecOps3 = &mVecOps3_SSE2;
	TestExecBenchmark(obj, &mathBenchVector3);
# elif defined(HAVE_SSE)
	TestMsg(ti, "M_Vector3 Microbenchmark (SSE):");
	mVecOps3 = &mVecOps3_SSE;
	TestExecBenchmark(obj, &mathBenchVector3);
# endif
#endif /* INLINE_SSE */

	mVecOps3 = prevVecOps3;
	return (0);
}

const AG_TestCase mathTest = {
	"math",
	N_("Test the ag_math library"),
	"1.4.2",
	0,
	sizeof(MyTestInstance),
	Init,
	Destroy,
	Test,
	NULL,	/* testGUI */
	Bench
};
