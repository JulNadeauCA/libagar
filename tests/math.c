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

#include <string.h>
#include <stdlib.h>

static int
Init(void *obj)
{
	M_InitSubsystem();
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

	TestMsgS(ti, "Testing vectors routines:");
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
TestMatrix(AG_TestInstance *ti)
{
	M_Matrix *M;

	M = M_New(5,5);
	M_SetIdentity(M);

	TestMsgS(ti, "Testing matrix routines:");
	TestMsgS(ti, AG_Printf("M=%[M]", M));
	
	M_Free(M);
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
	TestMatrix(ti);

	return (0);
}

const AG_TestCase mathTest = {
	"math",
	N_("Test the ag_math library"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	Init,
	Destroy,
	Test,
	NULL	/* testGUI */
};
