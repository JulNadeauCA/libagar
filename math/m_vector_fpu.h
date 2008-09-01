/*
 * Public domain.
 * Operations on vectors in R^n using standard FPU instructions.
 */

typedef struct m_vector_fpu {
	struct m_vector _inherit;
	M_Real *v;		/* Values */
} M_VectorFPU;

#define MVECTORFPU(v) ((M_VectorFPU *)(v))

__BEGIN_DECLS

static __inline__ void *
M_VectorNew_FPU(Uint m)
{
	M_VectorFPU *v;

	v = AG_Malloc(sizeof(M_VectorFPU));
	M_VectorInit(v, m);
	v->v = (m > 0) ? AG_Malloc(m*sizeof(M_Real)) : NULL;
	return (v);
}

static __inline__ M_Real *
M_VectorGetElement_FPU(const void *pv, Uint i)
{
	const M_VectorFPU *v=pv;
	return &v->v[i];
}

static __inline__ int
M_VectorResize_FPU(void *pv, Uint m)
{
	M_VectorFPU *v=pv;
	M_Real *vNew;

	if (m > 0) {
		if ((vNew = realloc(v->v, m*sizeof(M_Real))) == NULL) {
			AG_SetError("Out of memory");
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
M_VectorSetZero_FPU(void *pv)
{
	M_VectorFPU *v=pv;
	int i;

	for (i = 0; i < MVECSIZE(v); i++)
		v->v[i] = 0.0;
}

static __inline void
M_VectorFree_FPU(void *pv)
{
	M_VectorFPU *v=pv;
	
	AG_Free(v->v);
	AG_Free(v);
}

static __inline__ void *
M_VectorMirror_FPU(const void *pv)
{
	const M_VectorFPU *v=pv;
	M_VectorFPU *vInv;
	Uint i;

	vInv = M_VectorNew_FPU(MVECSIZE(v));
	for (i = 0; i < MVECSIZE(v); i++) {
		vInv->v[i] = -(v->v[i]);
	}
	return (vInv);
}

static __inline__ int
M_VectorMirrorv_FPU(void *pv)
{
	M_VectorFPU *v=pv;
	Uint i;

	for (i = 0; i < MVECSIZE(v); i++) {
		v->v[i] = -(v->v[i]);
	}
	return (0);
}

static __inline__ void *
M_VectorScale_FPU(const void *pv, M_Real c)
{
	const M_VectorFPU *v=pv;
	M_VectorFPU *w;
	Uint i;

	w = M_VectorNew_FPU(MVECSIZE(v));
	for (i = 0; i < MVECSIZE(v); i++) {
		w->v[i] = v->v[i]*c;
	}
	return (w);
}

static __inline__ int
M_VectorScalev_FPU(void *pv, M_Real c)
{
	M_VectorFPU *v = pv;
	Uint i;

	for (i = 0; i < MVECSIZE(v); i++) {
		v->v[i] *= c;
	}
	return (0);
}

static __inline__ void *
M_VectorAdd_FPU(const void *pa, const void *pb)
{
	const M_VectorFPU *a=pa, *b=pb;
	M_VectorFPU *c;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	c = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		c->v[i] = a->v[i] + b->v[i];
	}
	return (c);
}

static __inline__ int
M_VectorAddv_FPU(void *pa, const void *pb)
{
	M_VectorFPU *a=pa;
	const M_VectorFPU *b=pb;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, -1);
	for (i = 0; i < MVECSIZE(a); i++) {
		a->v[i] += b->v[i];
	}
	return (0);
}

static __inline__ void *
M_VectorSub_FPU(const void *pa, const void *pb)
{
	const M_VectorFPU *a=pa, *b=pb;
	M_VectorFPU *c;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	c = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		c->v[i] = a->v[i] - b->v[i];
	}
	return (c);
}

static __inline__ int
M_VectorSubv_FPU(void *pa, const void *pb)
{
	M_VectorFPU *a=pa;
	const M_VectorFPU *b = pb;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, -1);
	for (i = 0; i < MVECSIZE(a); i++) {
		a->v[i] -= b->v[i];
	}
	return (0);
}

static __inline__ M_Real
M_VectorLen_FPU(const void *pv)
{
	const M_VectorFPU *v=pv;
	M_Real dot = 0.0f;
	Uint i;
	
	for (i = 0; i < MVECSIZE(v); i++) {
		dot += v->v[i]*v->v[i];
	}
	return M_Sqrt(dot);
}

static __inline__ M_Real
M_VectorDot_FPU(const void *pa, const void *pb)
{
	const M_VectorFPU *a=pa, *b=pb;
	M_Real dot = 0.0f;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, 0.0);
	for (i = 0; i < MVECSIZE(a); i++) {
		dot += a->v[i]*b->v[i];
	}
	return (dot);
}

static __inline__ M_Real
M_VectorDistance_FPU(const void *pa, const void *pb)
{
	const M_VectorFPU *a=pa, *b=pb;
	return M_VectorLen_FPU( M_VectorSub_FPU(a,b) );
}

static __inline__ void *
M_VectorNorm_FPU(const void *pa)
{
	const M_VectorFPU *a=pa;
	M_VectorFPU *n;
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

static __inline__ void *
M_VectorLERP_FPU(const void *pa, const void *pb, M_Real c)
{
	const M_VectorFPU *a=pa, *b=pb;
	M_VectorFPU *d;
	Uint i;

	M_ASSERT_MATCHING_VECTORS(a, b, NULL);
	d = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		d->v[i] = a->v[i] + (b->v[i] - a->v[i])*c;
	}
	return (d);
}

static __inline__ void *
M_VectorElemPow_FPU(const void *pa, M_Real pow)
{
	const M_VectorFPU *a=pa;
	M_VectorFPU *b;
	Uint i;
	
	b = M_VectorNew_FPU(MVECSIZE(a));
	for (i = 0; i < MVECSIZE(a); i++) {
		b->v[i] = M_Pow(a->v[i], pow);
	}
	return (b);
}

static __inline__ int
M_VectorCopy_FPU(void *px, const void *py)
{
	const M_VectorFPU *y=py;
	M_VectorFPU *x=px;
	
	M_ASSERT_MATCHING_VECTORS(x, y, -1);
	memcpy(x->v, y->v, MVECSIZE(x) * sizeof(M_Real));
	return (0);
}
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps mVecOps_FPU;

void *M_ReadVector_FPU(AG_DataSource *);
void  M_WriteVector_FPU(AG_DataSource *, const void *);
void *M_VectorFromReals_FPU(Uint, const M_Real *);
void *M_VectorFromFloats_FPU(Uint, const float *);
void *M_VectorFromDoubles_FPU(Uint, const double *);
#ifdef HAVE_LONG_DOUBLE
void *M_VectorFromLongDoubles_FPU(Uint, const long double *);
#endif
void  M_VectorPrint_FPU(const void *);
__END_DECLS
