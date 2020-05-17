/*	Public domain	*/

typedef struct sk_polygon {
	struct sk_node node;
	SK_Line *_Nonnull *_Nullable s;		/* Line boundaries */
	Uint flags;
	Uint n;
	M_Color color;				/* Fill color */
} SK_Polygon;

#define SKPOLYGON(n) ((SK_Polygon *)(n))

__BEGIN_DECLS
extern SK_NodeOps skPolygonOps;

SK_Polygon *_Nonnull SK_PolygonNew(void *_Nonnull);
SK_Polygon *_Nonnull SK_PolygonFromValue(void *_Nonnull, M_Polygon);

void   SK_PolygonInit(void *_Nonnull, Uint);
int    SK_PolygonLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int    SK_PolygonSave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void   SK_PolygonDraw(void *_Nonnull, struct sk_view *_Nonnull);
M_Real SK_PolygonProximity(void *_Nonnull, const M_Vector3 *_Nonnull,
                           M_Vector3 *_Nonnull);
int    SK_PolygonDelete(void *_Nonnull);

void      SK_PolygonWidth(SK_Polygon *_Nonnull, M_Real);
void      SK_PolygonColor(SK_Polygon *_Nonnull, M_Color);
M_Polygon SK_PolygonValue(SK_Polygon *_Nonnull);
Uint      SK_PolygonAddSide(SK_Polygon *_Nonnull, SK_Line *_Nonnull);
__END_DECLS
