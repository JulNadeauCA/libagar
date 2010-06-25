/*
 * Public domain.
 * Operations on vectors in R^n using standard FPU instructions.
 */

__BEGIN_DECLS

static __inline__ M_Vector *
M_VectorNew_FPU(Uint m)
{
	M_Vector *v;

	v = (M_Vector *)AG_Malloc(sizeof(M_Vector));
	M_VectorInit(v, m);
	v->v = (M_Real *)((m > 0) ? AG_Malloc(m*sizeof(M_Real)) : NULL);
	return (v);
}

static __inline__ M_Real *
M_VectorGetElement_FPU(const M_Vector *v, Uint i)
{
	return &v->v[i];
}

static __inline__ int
M_VectorResize_FPU(M_Vector *pv, Uint m)
{
	M_Vector *v=pv;
	M_Real *vNew;

	if (m > 0) {
		if ((vNew = (M_Real *)AG_TryRealloc(v->v, m*sizeof(M_Real)))
		    == NULL) {
			return (-1);
		}
		v->v = vNew;
	} else {
		AG_Free(v->v);
		v->v = NULL;
	}
	MVECSIZE(v) = m;
	return (0);
}

static __inline__ void
M_VectorSetZero_FPU(M_Vector *v)
{
	Uint i;

	for (i = 0; i < MVECSIZE(v); i++)
		v->v[i] = 0.0;
}

static __inline__ void
M_VectorFree_FPU(M_Vector *v)
{
	AG_Free(v->v);
	AG_Free(v);
}

static __inline__ M_Vector *
M_VectorMirror_FPU(const M_Vector *v)
{
	M_Vector *vInv;
	Uint i;

	vInv = M_VectorNew_FPU(MVECSIZE(v));
	for (i = 0; i < MVECSIZE(v); i++) {
		vInv->v[i] = -(v->v[i]);
	}
	return (vInv);
}

static __inline__ int
M_VectorMirrorv_FPU(M_Vector *v)
{
	Uint i;

	for (i = 0; i < MVECSIZE(v); i++) {
		v->v[i] = -(v->v[i]);
	}
	return (0);
}

static __inline__ M_Vector *
M_VectorScale_FPU(const M_Vector *v, M_Real c)
{
	M_Vector *w;
	Uint i;

	w = M_VectorNew_FPU(MVECSIZE(v));
	for (i = 0; i < MVECSIZE(v); i++) {
		w->v[i] = v->v[i]*c;
	}
	return (w);
}

static __inline__ int
M_VectorScalev_FPU(M_Vector *v, M_Real c)
{
	Uint i;

	for (i = 0; i < MVECSIZE(v); i++) {
		v->v[i] *= c;
	}
	return (0);
}

static __inline__ M_Vector *
M_VectorAdd_FPU(const M_Vector *a, const M_Vector *b)
{
	M_Vector *c;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	c = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		c->v[i] = a->v[i] + b->v[i];
	}
	return (c);
}

static __inline__ int
M_VectorAddv_FPU(M_Vector *a, const M_Vector *b)
{
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, -1);
	for (i = 0; i < MVECSIZE(a); i++) {
		a->v[i] += b->v[i];
	}
	return (0);
}

static __inline__ M_Vector *
M_VectorSub_FPU(const M_Vector *a, const M_Vector *b)
{
	M_Vector *c;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	c = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		c->v[i] = a->v[i] - b->v[i];
	}
	return (c);
}

static __inline__ int
M_VectorSubv_FPU(M_Vector *a, const M_Vector *b)
{
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, -1);
	for (i = 0; i < MVECSIZE(a); i++) {
		a->v[i] -= b->v[i];
	}
	return (0);
}

static __inline__ M_Real
M_VectorLen_FPU(const M_Vector *v)
{
	M_Real dot = 0.0f;
	Uint i;
	
	for (i = 0; i < MVECSIZE(v); i++) {
		dot += v->v[i]*v->v[i];
	}
	return M_Sqrt(dot);
}

static __inline__ M_Real
M_VectorDot_FPU(const M_Vector *a, const M_Vector *b)
{
	M_Real dot = 0.0f;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, 0.0);
	for (i = 0; i < MVECSIZE(a); i++) {
		dot += a->v[i]*b->v[i];
	}
	return (dot);
}

static __inline__ M_Real
M_VectorDistance_FPU(const M_Vector *a, const M_Vector *b)
{
	return M_VectorLen_FPU( M_VectorSub_FPU(a,b) );
}

static __inline__ M_Vector *
M_VectorNorm_FPU(const M_Vector *a)
{
	M_Vector *n;
	M_Real len;
	Uint i;
	
	n = M_VectorNew_FPU(MVECSIZE(a));
	if ((len = M_VectorLen_FPU(a)) == 0.0) {
		for (i = 0; i < MVECSIZE(a); i++)
			n->v[i] = a->v[i];
	} else {
		for (i = 0; i < MVECSIZE(a); i++)
			n->v[i] = a->v[i]/len;
	}
	return (n);
}

static __inline__ M_Vector *
M_VectorLERP_FPU(const M_Vector *a, const M_Vector *b, M_Real c)
{
	M_Vector *d;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	d = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		d->v[i] = a->v[i] + (b->v[i] - a->v[i])*c;
	}
	return (d);
}

static __inline__ M_Vector *
M_VectorElemPow_FPU(const M_Vector *a, M_Real x)
{
	M_Vector *b;
	Uint i;
	
	b = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		b->v[i] = M_Pow(a->v[i], x);
	}
	return (b);
}

static __inline__ int
M_VectorCopy_FPU(M_Vector *x, const M_Vector *y)
{
	M_ASSERT_MATCHING_VECTORS(x, y, -1);
	memcpy(x->v, y->v, MVECSIZE(x) * sizeof(M_Real));
	return (0);
}
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps mVecOps_FPU;

M_Vector *M_ReadVector_FPU(AG_DataSource *);
void      M_WriteVector_FPU(AG_DataSource *, const M_Vector *);
M_Vector *M_VectorFromReals_FPU(Uint, const M_Real *);
M_Vector *M_VectorFromFloats_FPU(Uint, const float *);
M_Vector *M_VectorFromDoubles_FPU(Uint, const double *);
#ifdef HAVE_LONG_DOUBLE
M_Vector *M_VectorFromLongDoubles_FPU(Uint, const long double *);
#endif
void      M_VectorPrint_FPU(const M_Vector *);
__END_DECLS
