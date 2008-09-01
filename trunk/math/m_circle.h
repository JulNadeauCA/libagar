/*	Public domain	*/

__BEGIN_DECLS
M_Circle2	M_CircleRead2(AG_DataSource *);
M_Circle3	M_CircleRead3(AG_DataSource *);
void		M_CircleWrite2(AG_DataSource *, M_Circle2 *);
void		M_CircleWrite3(AG_DataSource *, M_Circle3 *);

M_Circle2	M_CircleFromPt(M_Vector2, M_Real);
M_Real		M_CirclePointDistance2(M_Circle2, M_Vector2);
M_GeomSet2	M_IntersectCircleCircle2(M_Circle2, M_Circle2);
M_GeomSet2	M_IntersectCircleLine2(M_Circle2, M_Line2);
__END_DECLS
