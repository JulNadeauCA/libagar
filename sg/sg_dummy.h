/*	Public domain	*/

typedef struct sg_dummy {
	struct sg_node node;
	M_Real foo;
	Uint8 _pad[8];
} SG_Dummy;

__BEGIN_DECLS
extern SG_NodeClass sgDummyClass;
SG_Dummy *_Nonnull SG_DummyNew(void *_Nullable, const char *_Nullable);
__END_DECLS

