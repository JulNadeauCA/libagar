/*	Public domain	*/

__BEGIN_DECLS
M_Triangle2 M_TriangleFromLines2(M_Line2, M_Line2, M_Line2);
M_Triangle3 M_TriangleFromLines3(M_Line3, M_Line3, M_Line3);
M_Triangle2 M_TriangleFromPts2(M_Vector2, M_Vector2, M_Vector2);
M_Triangle3 M_TriangleFromPts3(M_Vector3, M_Vector3, M_Vector3);

M_Triangle2 M_TriangleRead2(AG_DataSource *);
M_Triangle3 M_TriangleRead3(AG_DataSource *);
void        M_TriangleWrite2(AG_DataSource *, M_Triangle2 *);
void        M_TriangleWrite3(AG_DataSource *, M_Triangle3 *);

int         M_PointInTriangle2(M_Triangle2, M_Vector2);
__END_DECLS
