/*	$Csoft$	*/
/*	Public domain	*/

typedef struct sk_dummy {
	struct sk_node node;
	SG_Real foo;
} SK_Dummy;

__BEGIN_DECLS
extern SK_NodeOps skDummyOps;

SK_Dummy	*SK_DummyNew(void *);
void		 SK_DummyInit(void *);
int		 SK_DummyLoad(void *, AG_Netbuf *);
int		 SK_DummySave(void *, AG_Netbuf *);
void		 SK_DummyDraw(void *, SK_View *);
__END_DECLS

