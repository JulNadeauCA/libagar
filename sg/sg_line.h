/*	Public domain	*/

typedef struct sg_line {
	struct sg_geom _inherit;	/* SG_Geom -> SG_Line */
	M_Vector3 d;			/* Direction vector */
	M_Real t;			/* Line length */
	Uint8 _pad[8];
} SG_Line;

#define SGLINE(n) ((SG_Line *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgLineClass;

SG_Line *_Nonnull SG_LineNew(void *_Nullable, const char *_Nullable,
                             const M_Line3 *_Nullable);

#define  SG_LineColor(ln,c)	SG_GeomColor(SGGEOM(ln),(c))
#define  SG_LineWidth(ln,wd)	SG_GeomLineWidth(SGGEOM(ln),(wd))
#define  SG_LineStipple(ln,f,p)	SG_GeomLineStipple(SGGEOM(ln),(f),(p))
__END_DECLS
