/*	Public domain	*/
/*
 * Microbenchmarks for comparing the performance of M_Vector3 operations
 * on random vector sets, under different backends.
 */

static void
VectorZero3(void *ti)
{
	M_Vector3 v[4];
	int i;

	for (i = 0; i < 4; i++) {
		v[i] = M_VecZero3();
		if (i > 1) {
			if (v[i].x > v[i-1].x) { v[i].x = v[i-1].x; }
			if (v[i].y > v[i-1].y) { v[i].y = v[i-1].y; }
			if (v[i].y > v[i-1].z) { v[i].z = v[i-1].z; }
		}
	}
}
static void
VectorGet3(void *ti)
{
	M_Real f[3];
	M_Vector3 v;

	f[0] = RandomReal(ti);
	f[1] = RandomReal(ti);
	f[2] = RandomReal(ti);
	v = M_VecGet3(f[0], f[1], f[2]);
	if (v.x > RandomReal(ti)) { realJunk = v.x; }
}
static void
VectorSet3(void *ti)
{
	M_Real f[3];
	M_Vector3 v;
	
	f[0] = RandomReal(ti);
	f[1] = RandomReal(ti);
	f[2] = RandomReal(ti);
	M_VecSet3(&v, f[0], f[1], f[2]);
	if (v.x > RandomReal(ti)) { realJunk = v.x; }
}
static void
VectorCopy3(void *ti)
{
	M_Vector3 rOrig = RandomVector3(ti);
	M_Vector3 vDup;

	M_VecCopy3(&vDup, &rOrig);
	if (vDup.x > RandomReal(ti)) { realJunk = vDup.x; }
}
static void
VectorFlip3(void *ti)
{
	M_Vector3 vMir;

	vMir = M_VecFlip3(RandomVector3(ti));
	if (vMir.x > RandomReal(ti)) { realJunk = vMir.x; }
}
static void
VectorLen3(void *ti)
{
	M_Real len1, len2;

	len1 = M_VecLen3(RandomVector3(ti));
	len2 = M_VecLen3(RandomVector3(ti));
	if (len1 > len2) { realJunk = len1+len2; }
}
static void
VectorLen3p(void *ti)
{
	M_Vector3 v = RandomVector3(ti);
	M_Real len1, len2;

	len1 = M_VecLen3p(&v);
	len2 = M_VecLen3p(&v);
	if (len1 > len2) { realJunk = len1+len2; }
}
static void
VectorDot3(void *ti)
{
	M_Vector3 v[8];
	M_Real dots[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	dots[0] = M_VecDot3(v[0], v[1]);
	dots[1] = M_VecDot3(v[2], v[1]);
	dots[2] = M_VecDot3(v[2], v[3]);
	dots[5] = M_VecDot3(v[7], v[6]);
	dots[4] = M_VecDot3(v[5], v[6]);
	dots[3] = M_VecDot3(v[4], v[3]);
	for (i = 0; i < 6; i++) { realJunk += dots[i]; }
}
static void
VectorDot3p(void *ti)
{
	M_Vector3 v[8];
	M_Real dots[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	dots[0] = M_VecDot3p(&v[0], &v[1]);
	dots[1] = M_VecDot3p(&v[2], &v[1]);
	dots[2] = M_VecDot3p(&v[2], &v[3]);
	dots[5] = M_VecDot3p(&v[7], &v[6]);
	dots[4] = M_VecDot3p(&v[5], &v[6]);
	dots[3] = M_VecDot3p(&v[4], &v[3]);
	for (i = 0; i < 6; i++) { realJunk += dots[i]; }
}
static void
VectorDistance3(void *ti)
{
	M_Vector3 v[8];
	M_Real dots[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	dots[0] = M_VecDistance3(v[0], v[1]);
	dots[1] = M_VecDistance3(v[2], v[1]);
	dots[2] = M_VecDistance3(v[2], v[3]);
	dots[5] = M_VecDistance3(v[7], v[6]);
	dots[4] = M_VecDistance3(v[5], v[6]);
	dots[3] = M_VecDistance3(v[4], v[3]);
	for (i = 0; i < 6; i++) { realJunk += dots[i]; }
}
static void
VectorDistance3p(void *ti)
{
	M_Vector3 v[8];
	M_Real dots[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	dots[0] = M_VecDistance3p(&v[0], &v[1]);
	dots[1] = M_VecDistance3p(&v[2], &v[1]);
	dots[2] = M_VecDistance3p(&v[2], &v[3]);
	dots[5] = M_VecDistance3p(&v[7], &v[6]);
	dots[4] = M_VecDistance3p(&v[5], &v[6]);
	dots[3] = M_VecDistance3p(&v[4], &v[3]);
	for (i = 0; i < 6; i++) { realJunk += dots[i]; }
}
static void
VectorNorm3(void *ti)
{
	M_Vector3 v[6], vNorm[6];
	int i;

	for (i = 0; i < 6; i++) { v[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { vNorm[i] = M_VecNorm3(v[i]); }
	for (i = 0; i < 6; i++) { realJunk += vNorm[i].x; }
}
static void
VectorNorm3p(void *ti)
{
	M_Vector3 v[6], vNorm[6];
	int i;

	for (i = 0; i < 6; i++) { v[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { vNorm[i] = M_VecNorm3p(&v[i]); }
	for (i = 0; i < 6; i++) { realJunk += vNorm[i].x; }
}
static void
VectorNorm3v(void *ti)
{
	M_Vector3 v[6];
	int i;

	for (i = 0; i < 6; i++) { v[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { M_VecNorm3v(&v[i]); }
}
static void
VectorCross3(void *ti)
{
	M_Vector3 v[8], x[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	x[0] = M_VecCross3(v[0], v[1]);
	x[1] = M_VecCross3(v[2], v[1]);
	x[2] = M_VecCross3(v[2], v[3]);
	x[5] = M_VecCross3(v[7], v[6]);
	x[4] = M_VecCross3(v[5], v[6]);
	x[3] = M_VecCross3(v[4], v[3]);
	for (i = 0; i < 6; i++) { realJunk += x[i].x; }
}
static void
VectorCross3p(void *ti)
{
	M_Vector3 v[8], x[6];
	int i;

	for (i = 0; i < 8; i++) { v[i] = RandomVector3(ti); }
	x[0] = M_VecCross3p(&v[0], &v[1]);
	x[1] = M_VecCross3p(&v[2], &v[1]);
	x[2] = M_VecCross3p(&v[2], &v[3]);
	x[5] = M_VecCross3p(&v[7], &v[6]);
	x[4] = M_VecCross3p(&v[5], &v[6]);
	x[3] = M_VecCross3p(&v[4], &v[3]);
	for (i = 0; i < 6; i++) { realJunk += x[i].x; }
}

static void
VectorNormCross3(void *ti)
{
	M_Vector3 v[4], x[3];
	int i;

	for (i = 0; i < 4; i++) { v[i] = RandomVector3(ti); }
	x[0] = M_VecNormCross3(v[0], v[1]);
	x[1] = M_VecNormCross3(v[1], v[2]);
	x[2] = M_VecNormCross3(v[2], v[3]);
	for (i = 0; i < 3; i++) { realJunk += x[i].x; }
}
static void
VectorNormCross3p(void *ti)
{
	M_Vector3 v[4], x[3];
	int i;

	for (i = 0; i < 4; i++) { v[i] = RandomVector3(ti); }
	x[0] = M_VecNormCross3p(&v[0], &v[1]);
	x[1] = M_VecNormCross3p(&v[1], &v[2]);
	x[2] = M_VecNormCross3p(&v[2], &v[3]);
	for (i = 0; i < 3; i++) { realJunk += x[i].x; }
}

static void
VectorScale3(void *ti)
{
	M_Vector3 a[6], b[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { b[i] = M_VecScale3(a[i], RandomReal(ti)); }
	for (i = 0; i < 6; i++) { realJunk += b[i].x; }
}
static void
VectorScale3p(void *ti)
{
	M_Vector3 a[6], b[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { b[i] = M_VecScale3p(&a[i], RandomReal(ti)); }
	for (i = 0; i < 6; i++) { realJunk += b[i].x; }
}
static void
VectorScale3v(void *ti)
{
	M_Vector3 v[6];
	int i;

	for (i = 0; i < 6; i++) { v[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { M_VecScale3v(&v[i], RandomReal(ti)); }
	for (i = 0; i < 6; i++) { realJunk += v[i].x; }
}
static void
VectorAdd3(void *ti)
{
	M_Vector3 a[6], b[6], c[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { c[i] = M_VecAdd3(a[i], b[i]); }
	for (i = 0; i < 6; i++) { realJunk += c[i].x; }
}
static void
VectorAdd3p(void *ti)
{
	M_Vector3 a[6], b[6], c[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { c[i] = M_VecAdd3p(&a[i], &b[i]); }
	for (i = 0; i < 6; i++) { realJunk += c[i].x; }
}
static void
VectorAdd3v(void *ti)
{
	M_Vector3 a[6], b[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { M_VecAdd3v(&a[i], &b[i]); }
	for (i = 0; i < 6; i++) { realJunk += a[i].x; }
}

static void
VectorAvg3(void *ti)
{
	M_Vector3 a[6], b[6], c[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { c[i] = M_VecAvg3(a[i], b[i]); }
	for (i = 0; i < 6; i++) { realJunk += c[i].x; }
}

static void
VectorAvg3p(void *ti)
{
	M_Vector3 a[6], b[6], c[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { c[i] = M_VecAvg3p(&a[i], &b[i]); }
	for (i = 0; i < 6; i++) { realJunk += c[i].x; }
}

static void
VectorLERP3(void *ti)
{
	M_Vector3 a[6], b[6], c[6];
	int i;

	for (i = 0; i < 6; i++) { a[i] = RandomVector3(ti); b[i] = RandomVector3(ti); }
	for (i = 0; i < 6; i++) { c[i] = M_VecLERP3(a[i], b[i], RandomReal(ti)); }
	for (i = 0; i < 6; i++) { realJunk += c[i].x; }
}

static void
VectorSum3(void *ti)
{
	M_Vector3 v[12], vOut;
	int i;

	for (i = 0; i < 12; i++) { v[i] = RandomVector3(ti); }
	vOut = M_VecSum3(v, 12);
	for (i = 0; i < 12; i++) { realJunk += vOut.x; }
}

static struct ag_benchmark_fn mathBenchVector3Fns[] = {
	{ "Zero3()", 		VectorZero3		},
	{ "Get3()",		VectorGet3		},
	{ "Set3()", 		VectorSet3		},
	{ "Copy3()",		VectorCopy3		},
	{ "Flip3()",		VectorFlip3		},
	{ "Len3()", 		VectorLen3		},
	{ "Len3p()", 		VectorLen3p		},
	{ "Dot()", 		VectorDot3		},
	{ "Dot3p()",		VectorDot3p		},
	{ "Distance3()", 	VectorDistance3		},
	{ "Distance3p()", 	VectorDistance3p	},
	{ "Norm3()", 		VectorNorm3		},
	{ "Norm3p()", 		VectorNorm3p		},
	{ "Norm3v()", 		VectorNorm3v		},
	{ "Cross3()", 		VectorCross3		},
	{ "Cross3p()",	 	VectorCross3p		},
	{ "NormCross3()", 	VectorNormCross3	},
	{ "NormCross3p()",	VectorNormCross3p	},
	{ "Scale3()", 		VectorScale3		},
	{ "Scale3p()",	 	VectorScale3p		},
	{ "Scale3v()",	 	VectorScale3v		},
	{ "Add3()", 		VectorAdd3		},
	{ "Add3p()", 		VectorAdd3p		},
	{ "Add3v()", 		VectorAdd3v		},
	{ "Avg3()", 		VectorAvg3		},
	{ "Avg3p()", 		VectorAvg3p		},
	{ "LERP3()", 		VectorLERP3		},
	{ "Sum3()", 		VectorSum3		},
};
struct ag_benchmark mathBenchVector3 = {
	"M_Vector3",
	&mathBenchVector3Fns[0],
	sizeof(mathBenchVector3Fns) / sizeof(mathBenchVector3Fns[0]),
	20, 10000, 100000
};
