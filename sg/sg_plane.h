/*	Public domain	*/

typedef struct sg_plane {
	struct sg_geom _inherit;	/* SG_Geom -> SG_Plane */
} SG_Plane;

#define SG_PLANE_ISA(o) (((AGOBJECT(o)->cid & 0xffffff00) >> 8) == 0x7A0406)

__BEGIN_DECLS
extern SG_NodeClass sgPlaneClass;

SG_Plane *_Nonnull SG_PlaneNew(void *_Nullable, const char *_Nullable);
__END_DECLS
