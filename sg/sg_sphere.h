/*	Public domain	*/

typedef struct sg_sphere {
	struct sg_geom _inherit;	/* SG_Geom -> SG_Sphere */
	M_Real d;			/* Sphere diameter */
	Uint8 _pad[8];
} SG_Sphere;

#define SGSPHERE(n) ((SG_Sphere *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgSphereClass;

SG_Sphere *_Nonnull SG_SphereNew(void *_Nullable, const char *_Nullable,
                                 const M_Sphere *_Nullable);

#define  SG_SphereColor(ln,c)		SG_GeomColor(SGGEOM(ln),(c))
#define  SG_SphereWidth(ln,wd)		SG_GeomLineWidth(SGGEOM(ln),(wd))
#define  SG_SphereStipple(ln,f,p)	SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS
