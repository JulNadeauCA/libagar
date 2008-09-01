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

void M_SortPoints2(M_PointSet2 *, enum m_point_sort_mode2);
void M_SortPoints3(M_PointSet3 *, enum m_point_sort_mode3);
void M_PointSetPrint2(M_PointSet2 *);

/*
 * Add a new point to a set.
 */
static __inline__ void
M_PointSetAdd2(M_PointSet2 *S, M_Vector2 v)
{
	S->p = AG_Realloc(S->p, (S->n+1)*sizeof(M_Vector2));
	S->p[S->n++] = v;
}
static __inline__ void
M_PointSetAdd3(M_PointSet3 *S, M_Vector3 v)
{
	S->p = AG_Realloc(S->p, (S->n+1)*sizeof(M_Vector3));
	S->p[S->n++] = v;
}
static __inline__ void
M_PointSetAdd2p(M_PointSet2 *S, const M_Vector2 *p)
{
	S->p = AG_Realloc(S->p, (S->n+1)*sizeof(M_Vector2));
	S->p[S->n++] = *p;
}
static __inline__ void
M_PointSetAdd3p(M_PointSet3 *S, const M_Vector3 *p)
{
	S->p = AG_Realloc(S->p, (S->n+1)*sizeof(M_Vector3));
	S->p[S->n++] = *p;
}

/*
 * Duplicate an entire set.
 */
static __inline__ M_PointSet2
M_PointSetDup2(M_PointSet2 *S1)
{
	M_PointSet2 S2;
	S2.p = AG_Malloc(S1->n*sizeof(M_Vector2));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector2));
	S2.n = S1->n;
	return (S2);
}
static __inline__ M_PointSet3
M_PointSetDup3(M_PointSet3 *S1)
{
	M_PointSet3 S2;
	S2.p = AG_Malloc(S1->n*sizeof(M_Vector3));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector3));
	S2.n = S1->n;
	return (S2);
}

/*
 * Free a set.
 */
static __inline__ void
M_PointSetFree2(M_PointSet2 *S)
{
	AG_Free(S->p);
	S->p = NULL;
	S->n = 0;
}
static __inline__ void
M_PointSetFree3(M_PointSet3 *S)
{
	AG_Free(S->p);
	S->p = NULL;
	S->n = 0;
}
__END_DECLS
