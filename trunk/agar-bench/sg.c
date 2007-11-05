/*	$Csoft: surfaceops.c,v 1.3 2005/10/03 07:17:31 vedge Exp $	*/
/*	Public domain	*/

#include <agar/config/have_sse.h>
#include <agar/config/have_sse3.h>
#include "agar-bench.h"
#include <agar/sg.h>
#include <stdlib.h>

#define RANDOMIZE() if (++cur >= nVecs) { cur = 0; }

static SG_Vector *Vecs = NULL, *Vecs2 = NULL;
static SG_Real *Reals = NULL;
static int nVecs = 1000000;
static int cur = 0;
static int foo = 0;

static void InitVectors(void)
{
	int i;

	cur = 0;

	if (Vecs == NULL) {
		fprintf(stderr, "Initializing %d random vectors...", nVecs);
		Vecs = AG_Malloc(nVecs*sizeof(SG_Vector));
		Vecs2 = AG_Malloc(nVecs*sizeof(SG_Vector));
		Reals = AG_Malloc(nVecs*sizeof(SG_Real));
		for (i = 0; i < nVecs; i++) {
			Vecs[i] = VecGet(drand48(), drand48(), drand48());
			Vecs2[i] = VecGet(drand48(), drand48(), drand48());
			Reals[i] = (SG_Real)drand48();
		}
		fprintf(stderr, "ok\n");
	}
}

static void VectorZero3_FPU(void) {
	SG_Vector v1, v2, v3, v4;
	v1 = SG_VectorZero3_FPU();
	v2 = SG_VectorZero3_FPU();
	v3 = SG_VectorZero3_FPU();
	v4 = SG_VectorZero3_FPU();
	v1 = SG_VectorZero3_FPU();
	v2 = SG_VectorZero3_FPU();
	v3 = SG_VectorZero3_FPU();
	v4 = SG_VectorZero3_FPU();
	if (v1.x > v2.x) { foo = 1; }
	if (v2.x > v1.y) { foo = 2; }
	if (v3.x > v2.z) { foo = 3; }
	if (v4.x > v3.x) { foo = 4; }
}
static void VectorGet3_FPU(void) {
	SG_Vector v;
	RANDOMIZE();
	v = SG_VectorGet3_FPU(Reals[cur], Reals[cur], Reals[cur]);
	RANDOMIZE();
	if (v.x > Reals[cur]) { v.z = 1.0; }
	if (v.z > Reals[cur]) { v.x = 1.0; }
}
static void VectorSet3_FPU(void) {
	SG_Vector v;
	RANDOMIZE();
	SG_VectorSet3_FPU(&v, Reals[cur], Reals[cur], Reals[cur]);
	RANDOMIZE();
	if (v.x > Reals[cur]) { v.y = 1.0; }
	if (v.z < Reals[cur]) { v.x = 1.0; }
}
static void VectorCopy3_FPU(void) {
	SG_Vector vD;
	RANDOMIZE();
	SG_VectorCopy3_FPU(&vD, &Vecs[cur]);
	RANDOMIZE();
	if (vD.x > Reals[cur]) { vD.z = 1.0; }
	if (vD.z > Reals[cur]) { vD.x = 1.0; }
}
static void VectorMirror3_FPU(void) {
	SG_Vector vD;
	RANDOMIZE();
	vD = SG_VectorMirror3_FPU(Vecs[cur], 1,1,1);
	RANDOMIZE();
}
static void VectorMirror3p_FPU(void) {
	SG_Vector vD;
	RANDOMIZE();
	vD = SG_VectorMirror3p_FPU(&Vecs[cur], 1,1,1);
	RANDOMIZE();
}
static void VectorLen3_FPU(void) {
	SG_Real len;
	RANDOMIZE();
	len = SG_VectorLen3_FPU(Vecs[cur]);
	RANDOMIZE();
}
static void VectorLen3p_FPU(void) {
	SG_Real len;
	RANDOMIZE();
	len = SG_VectorLen3p_FPU(&Vecs[cur]);
}
static void VectorDot3_FPU(void) {
	SG_Real a, b, c, d, e;
	RANDOMIZE();
	a = SG_VectorDot3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	b = SG_VectorDot3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	c = SG_VectorDot3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	d = SG_VectorDot3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	e = SG_VectorDot3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (a > Reals[cur]) { foo = 1; }
	if (b > Reals[cur]) { foo = 2; }
	if (c > Reals[cur]) { foo = 3; }
	if (d > Reals[cur]) { foo = 4; }
	if (e > Reals[cur]) { foo = 5; }
}
static void VectorDot3p_FPU(void) {
	SG_Real a, b, c, d, e;
	RANDOMIZE();
	a = SG_VectorDot3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	b = SG_VectorDot3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	c = SG_VectorDot3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	d = SG_VectorDot3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	e = SG_VectorDot3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	if (a > Reals[cur]) { foo = 1; }
	if (b > Reals[cur]) { foo = 2; }
	if (c > Reals[cur]) { foo = 3; }
	if (d > Reals[cur]) { foo = 4; }
	if (e > Reals[cur]) { foo = 5; }
}
static void VectorDistance3_FPU(void) {
	SG_Real r;
	RANDOMIZE();
	r = SG_VectorDistance3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (r > Reals[cur]) { foo = 1; }
}
static void VectorDistance3p_FPU(void) {
	SG_Real r;
	RANDOMIZE();
	r = SG_VectorDistance3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	if (r > Reals[cur]) { foo = 1; }
}
static void VectorNorm3_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorNorm3_FPU(Vecs[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorNorm3p_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorNorm3p_FPU(&Vecs[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorNorm3v_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorNorm3v_FPU(&r);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
#if 0
static void VectorCross3_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorCross3_FPU(Vecs[cur], Vecs2[cur]);
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorCross3p_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorCross3p_FPU(&Vecs[cur], &Vecs2[cur]);
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
#endif
static void VectorScale3_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorScale3_FPU(Vecs[cur], Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorScale3p_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorScale3p_FPU(&Vecs[cur], Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorScale3v_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorScale3v_FPU(&r, Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorAdd3_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorAdd3_FPU(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}
static void VectorAdd3p_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorAdd3p_FPU(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}
static void VectorAdd3v_FPU(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorAdd3v_FPU(&r, &Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}

#ifdef HAVE_SSE

static void VectorZero3_SSE(void) {
	SG_Vector v1, v2, v3, v4;
	v1 = SG_VectorZero3_SSE();
	v2 = SG_VectorZero3_SSE();
	v3 = SG_VectorZero3_SSE();
	v4 = SG_VectorZero3_SSE();
	v1 = SG_VectorZero3_SSE();
	v2 = SG_VectorZero3_SSE();
	v3 = SG_VectorZero3_SSE();
	v4 = SG_VectorZero3_SSE();
	if (v1.x > v2.x) { foo = 1; }
	if (v2.x > v1.y) { foo = 2; }
	if (v3.x > v2.z) { foo = 3; }
	if (v4.x > v3.x) { foo = 4; }
}
static void VectorGet3_SSE(void) {
	SG_Vector v;
	RANDOMIZE();
	v = SG_VectorGet3_SSE(Reals[cur], Reals[cur], Reals[cur]);
	RANDOMIZE();
	if (v.x > Reals[cur]) { v.z = 1.0; }
	if (v.z > Reals[cur]) { v.x = 1.0; }
}
#if 0
static void VectorSet3_SSE(void) {
	SG_Vector v;
	RANDOMIZE();
	SG_VectorSet3_SSE(&v, Reals[cur], Reals[cur], Reals[cur]);
	RANDOMIZE();
	if (v.x > Reals[cur]) { v.y = 1.0; }
	if (v.z < Reals[cur]) { v.x = 1.0; }
}
static void VectorCopy3_SSE(void) {
	SG_Vector vD;
	RANDOMIZE();
	SG_VectorCopy3_SSE(&vD, &Vecs[cur]);
	RANDOMIZE();
	if (vD.x > Reals[cur]) { vD.z = 1.0; }
	if (vD.z > Reals[cur]) { vD.x = 1.0; }
}
#endif
static void VectorMirror3_SSE(void) {
	SG_Vector vD;
	RANDOMIZE();
	vD = SG_VectorMirror3_SSE(Vecs[cur], 1,1,1);
	RANDOMIZE();
}
static void VectorMirror3p_SSE(void) {
	SG_Vector vD;
	RANDOMIZE();
	vD = SG_VectorMirror3p_SSE(&Vecs[cur], 1,1,1);
	RANDOMIZE();
}
#if 0
static void VectorLen3_SSE(void) {
	SG_Real len;
	RANDOMIZE();
	len = SG_VectorLen3_SSE(Vecs[cur]);
	RANDOMIZE();
}
static void VectorLen3p_SSE(void) {
	SG_Real len;
	RANDOMIZE();
	len = SG_VectorLen3p_SSE(&Vecs[cur]);
	RANDOMIZE();
}
#endif

#ifdef HAVE_SSE3
static void VectorDot3_SSE3(void) {
	SG_Real a, b, c, d, e;
	RANDOMIZE();
	a = SG_VectorDot3_SSE3(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	b = SG_VectorDot3_SSE3(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	c = SG_VectorDot3_SSE3(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	d = SG_VectorDot3_SSE3(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	e = SG_VectorDot3_SSE3(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (a > Reals[cur]) { foo = 1; }
	if (b > Reals[cur]) { foo = 2; }
	if (c > Reals[cur]) { foo = 3; }
	if (d > Reals[cur]) { foo = 4; }
	if (e > Reals[cur]) { foo = 5; }
}
static void VectorDot3p_SSE3(void) {
	SG_Real a, b, c, d, e;
	RANDOMIZE();
	a = SG_VectorDot3p_SSE3(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	b = SG_VectorDot3p_SSE3(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	c = SG_VectorDot3p_SSE3(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	d = SG_VectorDot3p_SSE3(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	e = SG_VectorDot3p_SSE3(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	if (a > Reals[cur]) { foo = 1; }
	if (b > Reals[cur]) { foo = 2; }
	if (c > Reals[cur]) { foo = 3; }
	if (d > Reals[cur]) { foo = 4; }
	if (e > Reals[cur]) { foo = 5; }
}
#endif /* HAVE_SSE3 */

static void VectorDistance3_SSE(void) {
	SG_Real r;
	RANDOMIZE();
	r = SG_VectorDistance3_SSE(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (r > Reals[cur]) { foo = 1; }
}
static void VectorDistance3p_SSE(void) {
	SG_Real r;
	RANDOMIZE();
	r = SG_VectorDistance3p_SSE(&Vecs[cur], &Vecs2[cur]);
	if (r > Reals[cur]) { foo = 1; }
}
static void VectorNorm3_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorNorm3_SSE(Vecs[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorNorm3p_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorNorm3p_SSE(&Vecs[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
#if 0
static void VectorNorm3v_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorNorm3v_SSE(&r);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorCross3_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorCross3_SSE(Vecs[cur], Vecs2[cur]);
}
static void VectorCross3p_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorCross3p_SSE(&Vecs[cur], &Vecs2[cur]);
}
#endif
static void VectorScale3_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorScale3_SSE(Vecs[cur], Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
static void VectorScale3p_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorScale3p_SSE(&Vecs[cur], Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
#if 0
static void VectorScale3v_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorScale3v_SSE(&r, Reals[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.y > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 1; }
}
#endif
static void VectorAdd3_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorAdd3_SSE(Vecs[cur], Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}
static void VectorAdd3p_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = SG_VectorAdd3p_SSE(&Vecs[cur], &Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}

#if 0
static void VectorAdd3v_SSE(void) {
	SG_Vector r;
	RANDOMIZE();
	r = Vecs[cur];
	SG_VectorAdd3v_SSE(&r, &Vecs2[cur]);
	RANDOMIZE();
	if (r.x > Reals[cur]) { foo = 1; }
	if (r.z > Reals[cur]) { foo = 3; }
	if (r.y > Reals[cur]) { foo = 2; }
}
#endif

#endif /* HAVE_SSE */

static struct testfn_ops testfns[] = {
 { "VectorZero3_FPU()", InitVectors, NULL,	VectorZero3_FPU		},
 { "VectorGet3_FPU()", InitVectors, NULL,	VectorGet3_FPU		},
 { "VectorSet3_FPU()", InitVectors, NULL,	VectorSet3_FPU		},
 { "VectorCopy3_FPU()", InitVectors, NULL,	VectorCopy3_FPU		},
 { "VectorMirror3_FPU()", InitVectors, NULL,	VectorMirror3_FPU	},
 { "VectorMirror3p_FPU()", InitVectors, NULL,	VectorMirror3p_FPU	},
 { "VectorLen3_FPU()", InitVectors, NULL,	VectorLen3_FPU		},
 { "VectorLen3p_FPU()", InitVectors, NULL,	VectorLen3p_FPU		},
 { "VectorDot3_FPU()", InitVectors, NULL,	VectorDot3_FPU		},
 { "VectorDot3p_FPU()", InitVectors, NULL,	VectorDot3p_FPU		},
 { "VectorDistance3_FPU()", InitVectors, NULL,	VectorDistance3_FPU	},
 { "VectorDistance3p_FPU()", InitVectors, NULL,	VectorDistance3p_FPU	},
 { "VectorNorm3_FPU()", InitVectors, NULL,	VectorNorm3_FPU		},
 { "VectorNorm3p_FPU()", InitVectors, NULL,	VectorNorm3p_FPU	},
 { "VectorNorm3v_FPU()", InitVectors, NULL,	VectorNorm3v_FPU	},
#if 0
 { "VectorCross3_FPU()", InitVectors, NULL,	VectorCross3_FPU	},
 { "VectorCross3p_FPU()", InitVectors, NULL,	VectorCross3p_FPU	},
#endif
 { "VectorScale3_FPU()", InitVectors, NULL,	VectorScale3_FPU	},
 { "VectorScale3p_FPU()", InitVectors, NULL,	VectorScale3p_FPU	},
 { "VectorScale3v_FPU()", InitVectors, NULL,	VectorScale3v_FPU	},
 { "VectorAdd3_FPU()", InitVectors, NULL,	VectorAdd3_FPU		},
 { "VectorAdd3p_FPU()", InitVectors, NULL,	VectorAdd3p_FPU		},
 { "VectorAdd3v_FPU()", InitVectors, NULL,	VectorAdd3v_FPU		},
#ifdef HAVE_SSE
 { "VectorZero3_SSE()", InitVectors, NULL,	VectorZero3_SSE	},
 { "VectorGet3_SSE()", InitVectors, NULL,	VectorGet3_SSE		},
#if 0
 { "VectorSet3_SSE()", InitVectors, NULL,	VectorSet3_SSE		},
 { "VectorCopy3_SSE()", InitVectors, NULL,	VectorCopy3_SSE	},
 { "VectorLen3_SSE()", InitVectors, NULL,	VectorLen3_SSE		},
 { "VectorLen3p_SSE()", InitVectors, NULL,	VectorLen3p_SSE	},
#endif
#ifdef HAVE_SSE3
 { "VectorDot3_SSE3()", InitVectors, NULL,	VectorDot3_SSE3		},
 { "VectorDot3p_SSE3()", InitVectors, NULL,	VectorDot3p_SSE3	},
#endif
 { "VectorDistance3_SSE()", InitVectors, NULL,	VectorDistance3_SSE	},
 { "VectorDistance3p_SSE()", InitVectors, NULL, VectorDistance3p_SSE	},
 { "VectorNorm3_SSE()", InitVectors, NULL,	VectorNorm3_SSE	},
 { "VectorNorm3p_SSE()", InitVectors, NULL,	VectorNorm3p_SSE	},
#if 0
 { "VectorNorm3v_SSE()", InitVectors, NULL,	VectorNorm3v_SSE	},
 { "VectorCross3_SSE()", InitVectors, NULL,	VectorCross3_SSE	},
 { "VectorCross3p_SSE()", InitVectors, NULL,	VectorCross3p_SSE	},
#endif
 { "VectorScale3_SSE()", InitVectors, NULL,	VectorScale3_SSE	},
 { "VectorScale3p_SSE()", InitVectors, NULL,	VectorScale3p_SSE	},
#if 0
 { "VectorScale3v_SSE()", InitVectors, NULL,	VectorScale3v_SSE	},
#endif
 { "VectorAdd3_SSE()", InitVectors, NULL,	VectorAdd3_SSE		},
 { "VectorAdd3p_SSE()", InitVectors, NULL,	VectorAdd3p_SSE	},
#if 0
 { "VectorAdd3v_SSE()", InitVectors, NULL,	VectorAdd3v_SSE	},
#endif

#endif /* HAVE_SSE */
};

struct test_ops sg_test = {
	"SG",
	NULL,
	&testfns[0],
	sizeof(testfns) / sizeof(testfns[0]),
	0,
	100, 100000, 100000
};
