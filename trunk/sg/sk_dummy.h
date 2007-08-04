/*	Public domain	*/

typedef struct sk_dummy {
	struct sk_node node;
	SG_Real foo;
} SK_Dummy;

#define SK_DUMMY(n) ((SK_Dummy *)(n))

__BEGIN_DECLS
extern SK_NodeOps skDummyOps;

SK_Dummy	*SK_DummyNew(void *);
void		 SK_DummyInit(void *, Uint32);
int		 SK_DummyLoad(SK *, void *, AG_Netbuf *);
int		 SK_DummySave(SK *, void *, AG_Netbuf *);
void		 SK_DummyDraw(void *, SK_View *);
__END_DECLS

