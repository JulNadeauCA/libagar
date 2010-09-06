/*	Public domain	*/

__BEGIN_DECLS
void       M_PolygonInit2(M_Polygon2 *);
void       M_PolygonInit3(M_Polygon3 *);
void       M_PolygonFree2(M_Polygon2 *);
void       M_PolygonFree3(M_Polygon3 *);
M_Polygon2 M_PolygonRead2(AG_DataSource *);
M_Polygon3 M_PolygonRead3(AG_DataSource *);
void       M_PolygonWrite2(AG_DataSource *, M_Polygon2 *);
void       M_PolygonWrite3(AG_DataSource *, M_Polygon3 *);

M_Polygon2 M_PolygonFromLines2(Uint, M_Line2 *);
M_Polygon3 M_PolygonFromLines3(Uint, M_Line3 *);

int        M_PolygonAddLine2(M_Polygon2 *, M_Line2);
int        M_PolygonAddLine3(M_Polygon3 *, M_Line3);
int        M_PolygonAddVertex2(M_Polygon2 *, M_Vector2);
int        M_PolygonAddVertex3(M_Polygon3 *, M_Vector3);
int        M_PolygonDelLine2(M_Polygon2 *, int);
int        M_PolygonDelLine3(M_Polygon3 *, int);

int        M_PointInPolygon2(M_Polygon2 *, M_Vector2);
int        M_PolygonIsConvex2(M_Polygon2 *);
int        M_PolygonIsPlanar3(M_Polygon3 *);
__END_DECLS
