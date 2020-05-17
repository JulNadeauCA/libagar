/*	Public domain	*/

typedef struct sg_rectangle {
	struct sg_geom _inherit;		/* SG_Geom -> SG_Rectangle */
	M_Rectangle3 R;
} SG_Rectangle;

#define SGRECTANGLE(n) ((SG_Rectangle *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgRectangleClass;

SG_Rectangle *_Nonnull SG_RectangleNew(void *_Nullable, const char *_Nullable,
                                       const M_Rectangle3 *_Nullable);

#define SG_RectangleColor(r,c)     SG_GeomColor(SGGEOM(r),(c))
#define SG_RectangleWidth(r,wd)    SG_GeomLineWidth(SGGEOM(r),(wd))
#define SG_RectangleStipple(r,f,p) SG_GeomLineStipple(SGGEOM(r),(f),(p))
__END_DECLS
