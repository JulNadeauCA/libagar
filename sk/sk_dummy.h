/*	Public domain	*/

typedef struct sk_dummy {
	struct sk_node node;
	M_Real foo;
	Uint8 _pad[8];
} SK_Dummy;

#define SK_DUMMY(n) ((SK_Dummy *)(n))

__BEGIN_DECLS
extern SK_NodeOps skDummyOps;

SK_Dummy *_Nonnull SK_DummyNew(void *_Nonnull);

void SK_DummyInit(void *_Nonnull, Uint);
int  SK_DummyLoad(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
int  SK_DummySave(SK *_Nonnull, void *_Nonnull, AG_DataSource *_Nonnull);
void SK_DummyDraw(void *_Nonnull, struct sk_view *_Nonnull);
__END_DECLS

