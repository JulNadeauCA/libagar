/*	Public domain	*/

__BEGIN_DECLS
M_Triangle2 M_TriangleFromLines2(M_Line2, M_Line2, M_Line2);
M_Triangle3 M_TriangleFromLines3(M_Line3, M_Line3, M_Line3);

M_Triangle2 M_TriangleRead2(AG_DataSource *_Nonnull);
M_Triangle3 M_TriangleRead3(AG_DataSource *_Nonnull);
void        M_TriangleWrite2(AG_DataSource *_Nonnull, M_Triangle2 *_Nonnull);
void        M_TriangleWrite3(AG_DataSource *_Nonnull, M_Triangle3 *_Nonnull);

int         M_PointInTriangle2(M_Triangle2, M_Vector2);

static __inline__ M_Triangle2
M_TriangleFromPts2(M_Vector2 a, M_Vector2 b, M_Vector2 c)
{
	M_Triangle2 T;
	T.a = a;
	T.b = b;
	T.c = c;
	return (T);
}

static __inline__ M_Triangle3
M_TriangleFromPts3(M_Vector3 a, M_Vector3 b, M_Vector3 c)
{
	M_Triangle3 T;
	T.a = a;
	T.b = b;
	T.c = c;
	return (T);
}
__END_DECLS
