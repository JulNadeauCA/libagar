/*	Public domain	*/

typedef struct sg_polybox {
	struct sg_object _inherit;	/* SG_Object -> SG_Polybox */
} SG_Polybox;

#define SG_POLYBOX_ISA(o) (((AGOBJECT(o)->cid & 0xffffff00) >> 8) == 0x7A0502)
#define SGPOLYBOX(o)      ((SG_Polybox *)(o))
#define SGCPOLYBOX(o)     ((const SG_Polybox *)(o))

__BEGIN_DECLS
extern SG_NodeClass sgPolyboxClass;
SG_Polybox *_Nonnull SG_PolyboxNew(void *_Nullable, const char *_Nullable);
__END_DECLS
