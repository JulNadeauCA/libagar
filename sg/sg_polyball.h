/*	Public domain	*/

typedef struct sg_polyball {
	struct sg_object _inherit;	/* SG_Object -> SG_Polyball */
	Uint flags;
	int subdiv;			/* Subdivision level */
	Uint8 _pad[8];
} SG_Polyball;

__BEGIN_DECLS
extern SG_NodeClass sgPolyballClass;

SG_Polyball *_Nonnull SG_PolyballNew(void *_Nullable, const char *_Nullable,
                                     const M_Sphere *_Nullable);
void                  SG_PolyballSetSubdiv(SG_Polyball *_Nonnull, int);
__END_DECLS

