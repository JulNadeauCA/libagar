/*	Public domain	*/

typedef struct sg_plane {
	struct sg_geom _inherit;	/* SG_Geom -> SG_Plane */
} SG_Plane;

__BEGIN_DECLS
extern SG_NodeClass sgPlaneClass;

SG_Plane *_Nonnull SG_PlaneNew(void *_Nullable, const char *_Nullable);
__END_DECLS
