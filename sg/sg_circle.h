/*	Public domain	*/

typedef struct sg_circle {
	struct sg_geom _inherit;	/* SG_Geom -> SG_Circle */
} SG_Circle;

#define SGCIRCLE(n) ((SG_Circle *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgCircleClass;

SG_Circle *_Nonnull SG_CircleNew(void *_Nullable, const char *_Nullable,
                                 const M_Circle3 *_Nullable);

#define    SG_CircleWidth(ln,wd)	SG_GeomLineWidth(SGGEOM(ln),(wd))
#define    SG_CircleColor(ln,c)		SG_GeomLineColor(SGGEOM(ln),(c))
#define    SG_CircleStipple(ln,f,p)	SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS
