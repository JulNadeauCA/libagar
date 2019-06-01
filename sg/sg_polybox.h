/*	Public domain	*/

typedef struct sg_polybox {
	struct sg_object _inherit;
} SG_Polybox;

__BEGIN_DECLS
extern SG_NodeClass sgPolyboxClass;
SG_Polybox *_Nonnull SG_PolyboxNew(void *_Nullable, const char *_Nullable);
__END_DECLS
