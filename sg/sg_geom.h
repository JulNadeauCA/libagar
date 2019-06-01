/*	Public domain	*/

typedef struct sg_geom {
	struct sg_node _inherit;
	Uint flags;
#define SG_GEOM_SAVED	0
	M_Real wd;		/* Line Width */
	M_Color c;		/* Line Color */
	int stFactor;		/* Stipple Factor */
	Uint16 stPat;		/* Stipple Pattern */
} SG_Geom;

#define SGGEOM(n) ((SG_Geom *)(n))

__BEGIN_DECLS
extern SG_NodeClass sgGeomClass;

SG_Geom *_Nonnull SG_GeomNew(void *_Nullable, const char *_Nullable, M_Geom3);
void              SG_GeomColor(SG_Geom *_Nonnull, M_Color);
void              SG_GeomLineWidth(SG_Geom *_Nonnull, M_Real);
void              SG_GeomLineStipple(SG_Geom *_Nonnull, int, Uint16);
void              SG_GeomDrawBegin(const SG_Geom *_Nonnull);
#define           SG_GeomDrawEnd(geom) GL_PopAttrib()
__END_DECLS
