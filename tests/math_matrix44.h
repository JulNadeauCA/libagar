/*	Public domain	*/
/*
 * Microbenchmarks for comparing the performance of M_Vector3 operations
 * on random vector sets, under different backends.
 */

static __inline__ void
UseMatrix(const M_Matrix44 *M)
{
	if (M->m[0][0] > M->m[3][3]) { realJunk = M->m[1][1]+M->m[2][2]; }
}

static void
MatrixZero44(void *ti)
{
	M_Matrix44 M[10];
	int i;
	for (i = 0; i < 10; i++) { M[i] = M_MatZero44(); }
	for (i = 0; i < 10; i++) { UseMatrix(&M[i]); }
}
static void
MatrixZero44v(void *ti)
{
	M_Matrix44 M[10];
	int i;
	for (i = 0; i < 10; i++) { M_MatZero44v(&M[i]); }
	for (i = 0; i < 10; i++) { UseMatrix(&M[i]); }
}

static void
MatrixIdentity44(void *ti)
{
	M_Matrix44 M[10];
	int i;
	for (i = 0; i < 10; i++) { M[i] = M_MatIdentity44(); }
	for (i = 0; i < 10; i++) { UseMatrix(&M[i]); }
}
static void
MatrixIdentity44v(void *ti)
{
	M_Matrix44 M[10];
	int i;
	for (i = 0; i < 10; i++) { M_MatIdentity44v(&M[i]); }
	for (i = 0; i < 10; i++) { UseMatrix(&M[i]); }
}

static void
MatrixInvert44(void *ti)
{
	M_Matrix44 M[2], Minv[2];
	int i;
	for (i = 0; i < 2; i++) { M[i] = RandomMatrix44(ti); }
	for (i = 0; i < 2; i++) { Minv[i] = M_MatInvert44(M[i]); }
	for (i = 0; i < 2; i++) { UseMatrix(&Minv[i]); }
}

static void
MatrixTranspose44(void *ti)
{
	M_Matrix44 M[6], Mt[6];
	int i;
	for (i = 0; i < 6; i++) { M[i] = RandomMatrix44(ti); }
	for (i = 0; i < 6; i++) { Mt[i] = M_MatTranspose44(M[i]); }
	for (i = 0; i < 6; i++) { UseMatrix(&Mt[i]); }
}
static void
MatrixTranspose44p(void *ti)
{
	M_Matrix44 M[6], Mt[6];
	int i;
	for (i = 0; i < 6; i++) { M[i] = RandomMatrix44(ti); }
	for (i = 0; i < 6; i++) { Mt[i] = M_MatTranspose44p(&M[i]); }
	for (i = 0; i < 6; i++) { UseMatrix(&Mt[i]); }
}
static void
MatrixTranspose44v(void *ti)
{
	M_Matrix44 M[6];
	int i;
	for (i = 0; i < 6; i++) { M[i] = RandomMatrix44(ti); }
	for (i = 0; i < 6; i++) { M_MatTranspose44v(&M[i]); }
	for (i = 0; i < 6; i++) { UseMatrix(&M[i]); }
}

static void
MatrixMult44(void *ti)
{
	M_Matrix44 A[3], B[3], AB[3];
	int i;
	for (i = 0; i < 3; i++) { A[i] = RandomMatrix44(ti); B[i] = RandomMatrix44(ti); }
	for (i = 0; i < 3; i++) { AB[i] = M_MatMult44(A[i], B[i]); }
	for (i = 0; i < 3; i++) { UseMatrix(&AB[i]); }
}
static void
MatrixMult44v(void *ti)
{
	M_Matrix44 A[3], B[3];
	int i;
	for (i = 0; i < 3; i++) { A[i] = RandomMatrix44(ti); B[i] = RandomMatrix44(ti); }
	for (i = 0; i < 3; i++) { M_MatMult44v(&A[i], &B[i]); }
	for (i = 0; i < 3; i++) { UseMatrix(&A[i]); }
}

static void
MatrixCopy44(void *ti)
{
	M_Matrix44 A[6], B[6];
	int i;
	for (i = 0; i < 6; i++) { A[i] = RandomMatrix44(ti); }
	for (i = 0; i < 6; i++) { M_MatCopy44(&B[i], &A[i]); }
	for (i = 0; i < 6; i++) { realJunk = B[i].m[0][0]; }
}

static void
MatrixRotateAxis44(void *ti)
{
	M_Matrix44 A[6];
	M_Vector3 b[6];
	M_Real theta[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		b[i] = RandomVector3(ti);
		theta[i] = RandomReal(ti);
	}
	for (i = 0; i < 6; i++) { M_MatRotateAxis44(&A[i], theta[i], b[i]); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}
static void
MatrixRotate44I(void *ti)
{
	M_Matrix44 A[6];
	M_Real theta[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		theta[i] = RandomReal(ti);
	}
	for (i = 0; i < 6; i++) { M_MatRotate44I(&A[i], theta[i]); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}
static void
MatrixTranslate44(void *ti)
{
	M_Matrix44 A[6];
	M_Vector3 b[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		b[i] = RandomVector3(ti);
	}
	for (i = 0; i < 6; i++) { M_MatTranslate44v(&A[i], b[i]); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}
static void
MatrixTranslateX44(void *ti)
{
	M_Matrix44 A[6];
	M_Real t[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		t[i] = RandomReal(ti);
	}
	for (i = 0; i < 6; i++) { M_MatTranslate44X(&A[i], t[i]); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}
static void
MatrixScale44(void *ti)
{
	M_Matrix44 A[6];
	M_Vector4 b[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		b[i] = RandomVector4(ti);
	}
	for (i = 0; i < 6; i++) { M_MatScale44(&A[i], b[i].x, b[i].y, b[i].z, b[i].w); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}
static void
MatrixUniScale44(void *ti)
{
	M_Matrix44 A[6];
	M_Real s[6];
	int i;
	for (i = 0; i < 6; i++) {
		A[i] = RandomMatrix44(ti);
		s[i] = RandomReal(ti);
	}
	for (i = 0; i < 6; i++) { M_MatUniScale44(&A[i], s[i]); }
	for (i = 0; i < 6; i++) { realJunk = A[i].m[0][0]; }
}

static struct ag_benchmark_fn mathBenchMatrix44Fns[] = {
	{ "Zero44()", 		MatrixZero44		},
	{ "Zero44v()", 		MatrixZero44v		},
	{ "Identity44()", 	MatrixIdentity44	},
	{ "Identity44v()", 	MatrixIdentity44v	},
	{ "Transpose44()", 	MatrixTranspose44	},
	{ "Transpose44p()", 	MatrixTranspose44p	},
	{ "Transpose44v()", 	MatrixTranspose44v	},
	{ "Invert44()", 	MatrixInvert44		},
	{ "Mult44()", 		MatrixMult44		},
	{ "Mult44v()", 		MatrixMult44v		},
	{ "Copy()", 		MatrixCopy44		},
	{ "RotateAxis44()",	MatrixRotateAxis44	},
	{ "Rotate44I()",	MatrixRotate44I		},
	{ "Translate44()",	MatrixTranslate44	},
	{ "TranslateX44()",	MatrixTranslateX44	},
	{ "Scale44()",		MatrixScale44		},
	{ "UniScale44()",	MatrixUniScale44	},
};
struct ag_benchmark mathBenchMatrix44 = {
	"M_Matrix44",
	&mathBenchMatrix44Fns[0],
	sizeof(mathBenchMatrix44Fns) / sizeof(mathBenchMatrix44Fns[0]),
	10, 50000, 10000
};
