/*	Public domain	*/

typedef struct sg_polygon {
	struct sg_geom _inherit;		/* SG_Geom -> SG_Polygon */
	M_Polygon P;
} SG_Polygon;

#define SGPOLYGON(n) ((SG_Polygon *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgPolygonClass;

SG_Polygon *_Nonnull SG_PolygonNew(void *_Nullable, const char *_Nullable,
                                   const M_Polygon *_Nullable);

#define SG_PolygonColor(ln,c)     SG_GeomColor(SGGEOM(ln),(c))
#define SG_PolygonWidth(ln,wd)    SG_GeomLineWidth(SGGEOM(ln),(wd))
#define SG_PolygonStipple(ln,f,p) SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS
