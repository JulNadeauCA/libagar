/*	Public domain	*/

/*
 * Comparison order for stable point sort. If the first component is equal
 * up to machine precision, we compare the second and so on.
 */
enum m_point_set_sort_mode2 {
	M_POINT_SET_SORT_XY,
	M_POINT_SET_SORT_YX,
};
enum m_point_set_sort_mode3 {
	M_POINT_SET_SORT_XYZ,
	M_POINT_SET_SORT_XZY,
	M_POINT_SET_SORT_YXZ,
	M_POINT_SET_SORT_YZX,
	M_POINT_SET_SORT_ZXY,
	M_POINT_SET_SORT_ZYX,
};

__BEGIN_DECLS
int  M_PointSetAlloc2(M_PointSet2 *_Nonnull, Uint);
int  M_PointSetAlloc3(M_PointSet3 *_Nonnull, Uint);
int  M_PointSetAlloc2i(M_PointSet2i *_Nonnull, Uint);
int  M_PointSetAlloc3i(M_PointSet3i *_Nonnull, Uint);
int  M_PointSetCopy2(M_PointSet2 *_Nonnull, const M_PointSet2 *_Nonnull);
int  M_PointSetCopy3(M_PointSet3 *_Nonnull, const M_PointSet3 *_Nonnull);
int  M_PointSetCopy2i(M_PointSet2i *_Nonnull, const M_PointSet2i *_Nonnull);
int  M_PointSetCopy3i(M_PointSet3i *_Nonnull, const M_PointSet3i *_Nonnull);

void M_PointSetSort2(M_PointSet2 *_Nonnull, enum m_point_set_sort_mode2);
void M_PointSetSort3(M_PointSet3 *_Nonnull, enum m_point_set_sort_mode3);

/* Initialize a point set. */
static __inline__ void
M_PointSetInit2(M_PointSet2 *_Nonnull S)
{
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetInit3(M_PointSet3 *_Nonnull S)
{
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetInit2i(M_PointSet2i *_Nonnull S, M_Real w, M_Real h)
{
	S->w = w;
	S->h = h;
	S->x = NULL;
	S->y = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetScale2i(M_PointSet2i *_Nonnull S, M_Real w, M_Real h)
{
	S->w = w;
	S->h = h;
}
static __inline__ void
M_PointSetInit3i(M_PointSet3i *_Nonnull S)
{
	S->w = 1.0;
	S->h = 1.0;
	S->d = 1.0;
	S->x = NULL;
	S->y = NULL;
	S->z = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetScale3i(M_PointSet3i *_Nonnull S, M_Real w, M_Real h, M_Real d)
{
	S->w = w;
	S->h = h;
	S->d = d;
}

/* Append a new point to a set. */
static __inline__ int
M_PointSetAdd2(M_PointSet2 *_Nonnull S, M_Vector2 v)
{
	if (S->n+1 > S->nMax &&
	    M_PointSetAlloc2(S, S->n+1) == -1) {
		return (-1);
	}
	S->p[S->n] = v;
	return (S->n++);
}
static __inline__ int
M_PointSetAdd3(M_PointSet3 *_Nonnull S, M_Vector3 v)
{
	if (S->n+1 > S->nMax &&
	    M_PointSetAlloc3(S, S->n+1) == -1) {
		return (-1);
	}
	S->p[S->n] = v;
	return (S->n++);
}
static __inline__ int
M_PointSetAdd2i(M_PointSet2i *_Nonnull S, int x, int y)
{
	if (S->n+1 > S->nMax &&
	    M_PointSetAlloc2i(S, S->n+1) == -1) {
		return (-1);
	}
	S->x[S->n] = x;
	S->y[S->n] = y;
	return (S->n++);
}
static __inline__ int
M_PointSetAdd3i(M_PointSet3i *_Nonnull S, int x, int y, int z)
{
	if (S->n+1 > S->nMax &&
	    M_PointSetAlloc3i(S, S->n+1) == -1) {
		return (-1);
	}
	S->x[S->n] = x;
	S->y[S->n] = y;
	S->z[S->n] = z;
	return (S->n++);
}

/* Free a set. */
static __inline__ void
M_PointSetFree2(M_PointSet2 *_Nonnull S)
{
	AG_Free(S->p);
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetFree3(M_PointSet3 *_Nonnull S)
{
	AG_Free(S->p);
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetFree2i(M_PointSet2i *_Nonnull S)
{
	AG_Free(S->x);
	AG_Free(S->y);
	S->x = NULL;
	S->y = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetFree3i(M_PointSet3i *_Nonnull S)
{
	AG_Free(S->x);
	AG_Free(S->y);
	AG_Free(S->z);
	S->x = NULL;
	S->y = NULL;
	S->z = NULL;
	S->n = 0;
	S->nMax = 0;
}
__END_DECLS
