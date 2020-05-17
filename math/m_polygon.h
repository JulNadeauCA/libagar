/*	Public domain	*/

__BEGIN_DECLS
void M_PolygonInit(M_Polygon *_Nonnull);
void M_PolygonFree(M_Polygon *_Nonnull);

M_Polygon M_PolygonFromPts(Uint, const M_Vector2 *_Nonnull);
M_Polygon M_PolygonFromLines(Uint, const M_Line2 *_Nonnull);
M_Polygon M_PolygonFromPointSet2(const M_PointSet2 *_Nonnull);
M_Polygon M_PolygonFromPointSet2i(const M_PointSet2i *_Nonnull);

M_PointSet2  M_PolygonToPointSet2(const M_Polygon *_Nonnull);
M_PointSet2i M_PolygonToPointSet2i(const M_Polygon *_Nonnull, M_Real,M_Real);

M_Polygon M_PolygonRead(AG_DataSource *_Nonnull);
void      M_PolygonWrite(AG_DataSource *_Nonnull, const M_Polygon *_Nonnull);

int  M_PolygonCopy(M_Polygon *_Nonnull, const M_Polygon *_Nonnull);
void M_PolygonScale(M_Polygon *_Nonnull, M_Real,M_Real);
void M_PolygonOffset(M_Polygon *_Nonnull, M_Real,M_Real);

int  M_PolygonAddLine(M_Polygon *_Nonnull, M_Line2);
int  M_PolygonAddVertex(M_Polygon *_Nonnull, M_Vector2);
int  M_PolygonDelVertex(M_Polygon *_Nonnull, int);

int  M_PointInPolygon(const M_Polygon *_Nonnull, M_Vector2);
int  M_PolygonIsConvex(const M_Polygon *_Nonnull);
__END_DECLS
