/*	Public domain	*/

typedef struct sg_triangle {
	struct sg_geom _inherit;		/* SG_Geom -> SG_Triangle */
} SG_Triangle;

#define SG_TRIANGLE_ISA(o) (((AGOBJECT(o)->cid & 0xffffff00) >> 8)  == 0x7A0408)
#define SGTRIANGLE(n)      ((SG_Triangle *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgTriangleClass;

SG_Triangle *_Nonnull SG_TriangleNew(void *_Nullable, const char *_Nullable,
                                     const M_Triangle3 *_Nullable);

#define SG_TriangleColor(ln,c)     SG_GeomColor(SGGEOM(ln),(c))
#define SG_TriangleWidth(ln,wd)    SG_GeomLineWidth(SGGEOM(ln),(wd))
#define SG_TriangleStipple(ln,f,p) SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS
