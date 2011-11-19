/*	Public domain	*/

__BEGIN_DECLS
void       M_PolygonInit(M_Polygon *);
void       M_PolygonFree(M_Polygon *);
M_Polygon  M_PolygonRead(AG_DataSource *);
void       M_PolygonWrite(AG_DataSource *, const M_Polygon *);

M_Polygon  M_PolygonFromPts(Uint, const M_Vector2 *);
M_Polygon  M_PolygonFromLines(Uint, const M_Line2 *);

int        M_PolygonAddLine(M_Polygon *, M_Line2);
int        M_PolygonDelVertex(M_Polygon *, int);

int        M_PointInPolygon(const M_Polygon *, M_Vector2);
int        M_PolygonIsConvex(const M_Polygon *);

/*
 * Add a vertex to a polygon.
 * Return vertex index on success, -1 on failure.
 */
static __inline__ int
M_PolygonAddVertex(M_Polygon *P, M_Vector2 v)
{
	M_Vector2 *vNew;

	if ((vNew = AG_TryRealloc(P->v, (P->n+1)*sizeof(M_Vector2))) == NULL) {
		return (-1);
	}
	P->v = vNew;
	P->v[P->n] = v;
	return (P->n++);
}
__END_DECLS
