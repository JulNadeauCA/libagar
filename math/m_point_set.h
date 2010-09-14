/*	Public domain	*/

/*
 * Comparison order for stable point sort. If the first component is equal
 * up to machine precision, we compare the second and so on.
 */
enum m_point_sort_mode2 {
	M_POINT_SORT_XY,
	M_POINT_SORT_YX,
};
enum m_point_sort_mode3 {
	M_POINT_SORT_XYZ,
	M_POINT_SORT_XZY,
	M_POINT_SORT_YXZ,
	M_POINT_SORT_YZX,
	M_POINT_SORT_ZXY,
	M_POINT_SORT_ZYX,
};

__BEGIN_DECLS
void         M_PointSetAlloc2(M_PointSet2 *, Uint);
void         M_PointSetAlloc3(M_PointSet3 *, Uint);
void         M_PointSetAlloc2i(M_PointSet2i *, Uint);
void         M_PointSetAlloc3i(M_PointSet3i *, Uint);
M_PointSet2  M_PointSetDup2(M_PointSet2 *);
M_PointSet3  M_PointSetDup3(M_PointSet3 *);
M_PointSet2i M_PointSetDup2i(M_PointSet2i *);
M_PointSet3i M_PointSetDup3i(M_PointSet3i *);
void         M_SortPoints2(M_PointSet2 *, enum m_point_sort_mode2);
void         M_SortPoints3(M_PointSet3 *, enum m_point_sort_mode3);

/* Initialize a point set. */
static __inline__ void
M_PointSetInit2(M_PointSet2 *S)
{
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetInit3(M_PointSet3 *S)
{
	S->p = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetInit2i(M_PointSet2i *S)
{
	S->p = NULL;
	S->x = NULL;
	S->y = NULL;
	S->n = 0;
	S->nMax = 0;
}
static __inline__ void
M_PointSetInit3i(M_PointSet3i *S)
{
	S->p = NULL;
	S->x = NULL;
	S->y = NULL;
	S->z = NULL;
	S->n = 0;
	S->nMax = 0;
}

/* Append a new point to a set. */
static __inline__ void
M_PointSetAdd2(M_PointSet2 *S, M_Vector2 v)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc2(S, S->n+1); }
	S->p[S->n++] = v;
}
static __inline__ void
M_PointSetAdd3(M_PointSet3 *S, M_Vector3 v)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc3(S, S->n+1); }
	S->p[S->n++] = v;
}
static __inline__ void
M_PointSetAdd2i(M_PointSet2i *S, M_Vector2 v, int x, int y)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc2i(S, S->n+1); }
	S->p[S->n] = v;
	S->x[S->n] = x;
	S->y[S->n] = y;
	S->n++;
}
static __inline__ void
M_PointSetAdd3i(M_PointSet3i *S, M_Vector3 v, int x, int y, int z)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc3i(S, S->n+1); }
	S->p[S->n] = v;
	S->x[S->n] = x;
	S->y[S->n] = y;
	S->z[S->n] = z;
	S->n++;
}
static __inline__ void
M_PointSetAdd2iReal(M_PointSet2i *S, M_Vector2 v)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc2i(S, S->n+1); }
	S->p[S->n] = v;
	S->x[S->n] = (int)v.x;
	S->y[S->n] = (int)v.y;
	S->n++;
}
static __inline__ void
M_PointSetAdd2iInt(M_PointSet2i *S, int x, int y)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc2i(S, S->n+1); }
	S->p[S->n].x = (M_Real)x;
	S->p[S->n].y = (M_Real)y;
	S->x[S->n] = x;
	S->y[S->n] = y;
	S->n++;
}
static __inline__ void
M_PointSetAdd3iReal(M_PointSet3i *S, M_Vector3 v)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc3i(S, S->n+1); }
	S->p[S->n] = v;
	S->x[S->n] = (int)v.x;
	S->y[S->n] = (int)v.y;
	S->z[S->n] = (int)v.z;
	S->n++;
}
static __inline__ void
M_PointSetAdd3iInt(M_PointSet3i *S, int x, int y, int z)
{
	if (S->n+1 > S->nMax) { M_PointSetAlloc3i(S, S->n+1); }
	S->p[S->n].x = (M_Real)x;
	S->p[S->n].y = (M_Real)y;
	S->p[S->n].z = (M_Real)z;
	S->x[S->n] = x;
	S->y[S->n] = y;
	S->z[S->n] = z;
	S->n++;
}

/* Free a set. */
static __inline__ void
M_PointSetFree2(M_PointSet2 *S)
{
	AG_Free(S->p);
	M_PointSetInit2(S);
}
static __inline__ void
M_PointSetFree3(M_PointSet3 *S)
{
	AG_Free(S->p);
	M_PointSetInit3(S);
}
static __inline__ void
M_PointSetFree2i(M_PointSet2i *S)
{
	AG_Free(S->p);
	AG_Free(S->x);
	AG_Free(S->y);
	M_PointSetInit2i(S);
}
static __inline__ void
M_PointSetFree3i(M_PointSet3i *S)
{
	AG_Free(S->p);
	AG_Free(S->x);
	AG_Free(S->y);
	AG_Free(S->z);
	M_PointSetInit3i(S);
}
__END_DECLS
