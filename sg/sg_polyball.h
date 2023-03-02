/*	Public domain	*/

typedef struct sg_polyball {
	struct sg_object _inherit;	/* SG_Object -> SG_Polyball */
	Uint flags;
	int subdiv;			/* Subdivision level */
	Uint8 _pad[8];
} SG_Polyball;

#define SG_POLYBALL_ISA(o) (((AGOBJECT(o)->cid & 0xffffff00) >> 8) == 0x7A0501)
#define SGPOLYBALL(o)      ((SG_Polyball *)(o))
#define SGCPOLYBALL(o)     ((const SG_Polyball *)(o))

__BEGIN_DECLS
extern SG_NodeClass sgPolyballClass;

SG_Polyball *_Nonnull SG_PolyballNew(void *_Nullable, const char *_Nullable,
                                     const M_Sphere *_Nullable);
void                  SG_PolyballSetSubdiv(SG_Polyball *_Nonnull, int);
__END_DECLS

